#include <unistd.h> //getopt
#include <getopt.h> //optind
#include <stdio.h>
#include  <fcntl.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include "my_header.h"

#define PARAM param //parameter we are testing
#define GRID_GRANULARITY 64

//#define TIME_REPORT
//#define DEBUG
#ifdef DEBUG
//VERBOSE DEBUGGING
// #define DEBUG_VB_ECHO
// #define DEBUG_VB_GN //generate node
// #define DEBUG_VB_MAIN
// #define DEBUG_VB_FM //fm_algorithm
// #define DEBUG_VB_COST//cost function
#endif

//global variables
struct node ** N_Arr; //indexed copies
struct node **BHead, ** BTail; //bucet list head and tails

int Offset, PadOffset, Modules;
int ACnt, BCnt;
int CMin, CMax;
int TestCost;
//function prototypes
int gernerate_netlist();
int cost(struct node* n);
void shuffle_partition();
void memfree();

static int getint();
static int endline();
static int to_integer(char * c);
static void help();
static void inline request_help();


/*************************** Input Processing/Initial Netlist generation **********************************/
int GenerateNodes(FILE* gno_file)
{
	struct node *n;
	char type;
	
	while( (type = (char) fgetc(gnt_file))!=EOF)
	{
		
		do
		{
			type = (char) fgetc(gnt_file);
			if(type==EOF){return 1;}
			if(type == '#'){ endline(gnt_file);continue;}
		}while(!isalpha(type));

		index_mod = index = GetNextInt(gnt_file);
		index_mod += (type=='p')?PadOffset:0;
	
		do{ n = malloc(sizeof(struct node));} while(n==NULL);

		n->type = type;	
		n->index = index;
		n->locked = 0;
		n->cost = INT_MAX;
		n->birth = NULL;
		n->out_head = NULL;
		n->dir = '\0';	
		n->orientation = '\0';
		n->x  = 0;
		n->y = 0;
		n->width = GetNextInt(gno_file);
		n->height = GetNextInt(gno_file)		

		assert(N_Arr[index_mod]==NULL);
		N_Arr[index_mod] = n;
	
		endline();		
	}
}

int GenerateNetlist(FILE* gnt_file)
{
	struct node *n, *n_head;
	struct edge* e, *e_fost, *e_net;
	struct hyperedge * h, *h_net;

	char type,  direction;
	int i, index_mod, node_degree;	
	char o_xflag, b_flag, p_flag; //output exclusive flag(only first node in set should be an output, unless pad), bidirectional flag, pad flag

	ACnt = BCnt = 0;
	CMin = 0;
	CMax = Offset*2;

	while((node_degree = GetNextInt(gnt_file))!= INT_MIN)
	{

		o_xflag = 0; p_flag = 0;
		direction = '\0';
		endline(gnt_file);
		for(i=0; i<node_degree; i++)
		{
			//Get information
			do
			{
				type = (char) fgetc(gnt_file);
				if(type==EOF){return 1;}
				if(type == '#'){ endline(gnt_file);continue;}	
			}while(!isalpha(type));
			index_mod = GetNextInt(gnt_file);
			if(type=='p')
			{
				index_mod += PadOffset;
				p_flag = 1;
			}
			do{ direction = (char) fgetc(gnt_file);} while(!isalpha(direction));
			direction =  toupper(direction);
			if(direction == 'O'){o_xflag = (i==0)?1:0;} //only first in list should be output	
			if(direction == 'B'){b_flag = 1;}

#endif
			//fetch/create node
			if((n = N_Arr[index_mod])==NULL){fprintf(stderr, "NULL node unexpected");}
			if(type == p){n->pdir = direction;}
	
			if(i==0)
			{
				n_head = n; //new start

				//new output hyperedge - link to parent node
				do{ h_net = malloc(sizeof(struct hyperedge));} while(h_net==NULL);
				h_net->out = n_head;
				h_net->out_head = NULL;
				h_net->next == NULL;

				//insert output hyperedge
				h = n_head->out_head;
				if(h==NULL)
				{
					n_head->out_head = h_net;
				}
				else
				{
					while(h->next!=NULL){ h=h->next;} //advance to end of linked list
					h->next = h_net;
				}		 

				//cleanup
				endline();
				n = NULL;
				e = e_net = NULL;
				h = NULL; //we are done - reset

				//OTHER NODES
			}else
			{	
				//create edge - link to parent edge
				do{ e =  malloc(sizeof(struct edge));} while(e==NULL);
				e->in = n;
				e->next = NULL;
				e->foster = NULL;
				e->parent = h_net;

				//set edge as input to node n
				e_fost = n->birth;
				if(e_fost == NULL)
				{
					n->birth = e; //first input to this node (a new node)	
				}
				else
				{
#ifdef DEBUG_VB_GN
					fprintf(stderr, "\tchild:\n");
#endif
					while(e_fost->foster != NULL)
					{
						e_fost = e_fost->foster; //progress to tail
#ifdef DEBUG_VB_GN
						fprintf(stderr, "\t%c%d\n", e_fost->parent->out->type, e_fost->parent->out->index);
#endif
					}
					e_fost->foster = e; //add input to tail of linked list
				}

				//link hypredge -> edge
				h = h_net; //load hyperedge
				e_net = h->out_head;
				if(e_net==NULL) //first edge - start new inked list
				{
					h->out_head = e; //first edge
				}
				else //move to tail and insert
				{
					while(e_net->next!=NULL){ e_net = e_net->next; }
					e_net->next = e;
				}

				//cleanup
				endline(gnt_file);
				n = NULL;
				h = NULL;
				e = NULL;

			}//end else
		}//end for node_degree
		if(p_flag)
		{
			assert(node_degree==2);
		}
		else
		{
			assert(o_flag); //need one output
			assert(!b_flag); //no bidirectional pins
		}
	}//end while
	return 1;
}//end get input function

int GeneratePlacement(FILE* gpl_file)
{
	struct node* n;
	char type;	
	int index_mod;

	endline(gpl_file); //get rid of header
	while(1)
	{
		do
		{
			type = (char) fgetc(gpl_file);
			(type==EOF){return 1;}
			if(type == '#'){ endline(gpl_file);continue;}
		}while(!isalpha(type));
		index_mod = GetNextInt(gpl_file);
		index_mod += (type=='p')?PadOffset:0;
	
		n = N_Arr[index_mod];
		assert(n!=NULL);
	
		n->x = GetNextInt(gpl_file);
		n->y = GetNextInt(gpl_file);
		n->orientation = GetOrientation();
	
		endline();
	}
}

int GenerateGrid(FILE* gg_file)
{
	struct node* n, **np;
	struct row_hdr* rhdr; //information about the row
	int num_rows;
	int  coordinate, height, sitewidth, sitespacing, siteorient, sitesymmetry, subroworgin, numsites;
	int i, j;

	endline(); //get rid of header
	
	num_rows = GetNextInt(gg_file);
	do{ Grid = (struct node ***) malloc(num_rows * sizeof(node**));} while(Grid==NULL);
	do{ GridHdr = (struct row_hdr**) malloc(num_rows * sizeof(row_hdr*));} while(GridHdr==NULL);
	
	//make rows
	for(i=0; i<num_rows; i++)
	{
		do{ rhdr = (struct row_hdr*) malloc(sizeof(row_hdr));} while(rhdr==NULL);
		rhdr->coordinate = GetNextInt(gg_file);
		rhdr->height = GetNextInt(gg_file);
		rhdr-> sitewidth = GetNextInt(gg_file);
		rhdr-> sitespacing = GetNextInt(gg_file);
		rhdr -> siteorient = GetOrientation(gg_file);
		rhdr -> sitesymmetry = get_sym(gg_file);
		rhdr -> subroworgin = GetNextInt(gg_file);
		rhdr -> numsites = GetNextInt(gg_file);
		rhdr -> numindexes = (rhdr->numsites)/GRID_GRANULARITY + 1;		

		GridHdr[i] = rhdr;
		//we will allocate an array of linked list for easy access insertion/deletion
		//indexes will represent a pointer to the start of its range of coordinates
		do{ np = (struct node**) malloc((rhdr->numindexes)  *sizeof(node*));} while(np==NULL);
		if(i=0){InitEmptyNode((rhdr->numsites)*(rhdr->sitewidth));} //initialize Empty Node array with approximate size
		n = CreateEmptyNode(0, rhdr->coordinate, rhdr->height, (rhdr->numsites)*(rhdr->sitewidth)); //create empty node and add it to empty node list
		for(j=0; j<rhdr->numindexes; j++){ np[j] = n;} //place empty nodes in grid

		Grid[i] = np;
				
		//cleanup
		n = NULL;
		np = NULL;
		rhdr = NULL;	
		endline();
	}
	
	//corner stitch
	Grid[0][0]->south = Grid[1][0]; //top only has south
	for(i=1; i<num_rows-1; i++)
	{	
		Grid[i][0]->north = Grid[i-1][0];
		Grid[i][0]->south = Grid[i+1][0];
	}
	Grid[num_rows-1][0]->north = Grid[num_rows-2][0];
	
	return 0;
}

/*********************************************** Cost function ************************************************/
int cost(struct node* n)
{
	int s_in=0, d_in=0; //same partition in, different partition in
	int s_flag, d_flag; //same flag/different flag for chyperedge children
	int cost, cost_mod; //cost and modified cost
	char part;
	struct node* bucket;
	struct edge *e;
	struct hyperedge* h;

	if(n==NULL) return 0;

	part = n->part; //make it a little faster

	//inputs
#ifdef DEBUG_VB_COST
	fprintf(stderr, "II");
#endif
	e = n->birth;
	while(e!=NULL) //iterate through node inputs to determine where parents are
	{
#ifdef DEBUG_VB_COST
		fprintf(stderr, ".");
#endif

		if(e->parent->out->part == part){ s_in++; }
		else { d_in++;};
		e = e->foster; //move to next input
	} 

	//outputs
	h = n->out_head;
#ifdef DEBUG_VB_COST
	fprintf(stderr, "OI");
#endif


	while(h != NULL) //through each  output hyperedge of node
	{
#ifdef DEBUG_VB_COST
		fprintf(stderr, ".");
#endif
		e = h->out_head;
		s_flag = d_flag = 0;
		while(e!=NULL) //through each edge of hyperedge
		{
			if(e->in->part == part){s_flag = 1;}
			else{d_flag = 1;}
			e = e->next;
		}
		s_in += (s_flag)?1:0;
		d_in += d_flag?1:0;	

		h = h->next;
	}

	cost = d_in - s_in;
	TestCost += d_in;

	//get cost_mod for p bucket index
	cost_mod = cost + Offset;
	cost_mod = (cost_mod>2*Offset) ? 2*Offset : cost_mod;
	cost_mod  = (cost_mod<0) ? 0 : cost_mod;
	CMin = (cost_mod < CMin)?cost_mod:CMin;
	CMax = (cost_mod > CMax)?cost_mod:CMax;

#ifdef DEBUG_VB_COST
	fprintf(stderr, "\tCost: %c%d\t->\t%d-%d=%d\t%d\n", n->type, n->index, d_in, s_in, cost, cost_mod);
#endif

	//remove bucket
	/*	if(n->pbucket == NULL) //head or uninitialized
		{
		BHead[n->cost]= n->nbucket; //disconnect head pointer from n
		if(n->nbucket != NULL)	{ n->nbucket->pbucket = NULL;} //unlink next node from n
		else if(BTail[n->cost] == n){BTail[n->cost] = NULL;} //only one node in list - unlink
		}
		else if(n->nbucket == NULL) //tail 
		{
		BTail[n->cost] = n->pbucket; //unlink tail pointer from node
		n->pbucket->nbucket = NULL; //unlink node from n
		}
		else //normal removal
		{
		n->pbucket->nbucket = n->nbucket; //cut node out of double linked list
		n->nbucket->pbucket = n->pbucket;
		}
	 */

	if(BHead[n->cost] == n){ BHead[n->cost] = n->nbucket;} //disconnect head pointer from n
	if(BTail[n->cost]==n) { BTail[n->cost] = n->pbucket;} //unlink tail pointer from n
	if(n->pbucket!=NULL){n->pbucket->nbucket = NULL;} //unlink previous node from n
	if(n->nbucket != NULL)  { n->nbucket->pbucket = NULL;} //unlink next node from n

	//save cost, unlink node and append to new bucket
	n->cost = cost_mod;
	n->nbucket = n->pbucket = NULL;
#ifdef DEBUG_VB_COST
	fprintf(stderr, "Removed");
#endif

	if(n->locked >= LOCK_THRESH){return cost;} //we dont want to add locked nodes to this list
	else if(BHead[cost_mod] == NULL) //no entry at this index yet
	{
		BHead[cost_mod] = BTail[cost_mod] = n;
	}	
	else if(BTail[cost_mod]==NULL) {BHead[cost_mod]=NULL;} //lost track of tail - FATAL
	else{ 
		//append to tail
		BTail[cost_mod]->nbucket = n; //link last node in bucket list to n
		n->pbucket = BTail[cost_mod]; //link n to bucket list
		BTail[cost_mod] = n; //update tail
	}
#ifdef DEBUG_VB_COST
	fprintf(stderr, "...Readded\n");
#endif

	return cost;
}

void memfree()
{
	struct node* n;
	struct hyperedge* h, *h_next;
	struct edge* e, *e_next;
	int i;

	for(i=0; i<Offset; i++)
	{
		if((n=N_Arr[i]) == NULL) {continue;}
		//output hyperedges
		h = n->out_head;
		while(h!=NULL)
		{
			h_next = h->next;
			free(h);
			h = h_next;
		}
		//input edges
		e = n->birth;
		while(e!=NULL)
		{
			e_next = e->foster;
			free(e);
			e = e_next;
		}
		free(n);
	}	
	free(N_Arr);	
}

char s[10];
static int GetNextInt(FILE* gni_file)
{
	int ret, i;
	//	while(scanf("%d", &ret)<=0); //scanf f stops working after ~400 integer scans
	while(!isdigit((s[0]=(char)fgetc(gi_file)))) //wait for first digit
	{
		if(s[0] == '#') { endline(gni_file); } //remove file
	}
	for(i=1; i<10 && isdigit(s[i] = (char)fgetc(gni_file)); i++);
	s[i] = '\0'; //null terminate
	return to_integer(s);
}



/************* get_line ********************
 * Gets one line from infile
 *
 * returns length of line on sucess, negative value on error
 **************************************************/
static int  endline(FILE* el_file)
{
	unsigned char c[1];
	int i, err;

	while(1) //avoid end of file and line
	{
		*c = fgetc(el_file);
		if(*c=='\n' || *c == EOF) break;
	}

	return i;
}


/************** to_integer *********************
 * Converts a string to an integer
 *
 * c - numeric string to conver
 *
 * returns the integer
 ***********************************************/
static int to_integer(char * c)
{
	int i, val;
	i= val = 0;
	while(c[val]!='\0' && c[val]<='9' && c[val]>='0')
	{
		i+= (unsigned int) c[val] - '0';
		i*=10;
		val++;
	}
	i/=10;
	return i;
}

/************** help ***************
 * prints out help message
 ***********************************/
static void help()
{
	fprintf(stdout, "request_help");

}

/***********request_help ***********
 * suggests to confused users that the seek help
 ***********************************/
static void inline request_help()
{
	fprintf(stdout,  "Use \"./algo -h\" for help");
}
