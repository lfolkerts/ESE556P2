#include <stdio.h>
#include  <fcntl.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include <limits.h>
#include"parameters.h"
#include "generate_data.h"
#include "grid.h"
#include "overlapgrid.h"
#include "node.h"
#include "helper.h"

/*************************** Input Processing/Initial Netlist generation **********************************/
int GenerateNodes(FILE* gno_file)
{
	struct node *n, *n_cpy;
	char type;
	int index, index_mod;

	Endline(gno_file);

	//fnodes - need to get number of terminals(pads) and number of total nodes (modules)
        Modules = GetNextInt(gno_file); //number of nodes
        PadOffset = GetNextInt(gno_file); //terminal offset
        do{N_Arr = malloc((Modules+1)*sizeof(struct node*));} while(N_Arr==NULL);
	
	while( (type = (char) fgetc(gno_file))!=EOF)
	{
		
		do
		{
			type = (char) fgetc(gno_file);
			if(type==EOF){return 1;}
			if(type == '#'){ Endline(gno_file);continue;}
		}while(!isalpha(type));

		index_mod = index = GetNextInt(gno_file);
		index_mod += (type=='p')?PadOffset:0;
	
		do{ n = (struct node*) malloc(sizeof(struct node));} while(n==NULL);
		do{ n_cpy = (struct node*)malloc(sizeof(struct node));}while(n_cpy==NULL);
		n->type = type;	
		n->index = index;
		n->locked = 0;
		
		n->cost = INT_MAX;
		n->birth = NULL;
		n->out_head = NULL;
		n->dir = '\0';	
		n->orientation = '\0';
		n->x  = INIT_X;
		n->y = INIT_Y;
		n->width = GetNextInt(gno_file);
		n->height = GetNextInt(gno_file);	

		assert(N_Arr[index_mod]==NULL);
		N_Arr[index_mod] = n;
		N_ArrCpy[index_mod] = n_cpy;
		Endline(gno_file);		
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
	int nets, pins;
	
	Endline(gnt_file);
        //fnets - neet to genertate the netlist graph 
        nets = GetNextInt(gnt_file);
        pins = GetNextInt(gnt_file);

	while((node_degree = GetNextInt(gnt_file))!= INT_MIN)
	{

		o_xflag = 0; p_flag = 0;
		direction = '\0';
		Endline(gnt_file);
		for(i=0; i<node_degree; i++)
		{
			//Get information
			do
			{
				type = (char) fgetc(gnt_file);
				if(type==EOF){return 1;}
				if(type == '#'){ Endline(gnt_file);continue;}	
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

			//fetch/create node
			if((n = N_Arr[index_mod])==NULL){fprintf(stderr, "NULL node unexpected");}
			if(type == 'p'){n->dir = direction;}
	
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
				Endline(gnt_file);
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
				Endline(gnt_file);
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
			assert(o_xflag); //need one output
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

	Endline(gpl_file); //get rid of header
	while(1)
	{
		do
		{
			type = (char) fgetc(gpl_file);
			if(type==EOF){return 1;}
			if(type == '#'){ Endline(gpl_file);continue;}
		}while(!isalpha(type));
		index_mod = GetNextInt(gpl_file);
		index_mod += (type=='p')?PadOffset:0;
	
		n = N_Arr[index_mod];
		assert(n!=NULL);
	
		n->x = GetNextInt(gpl_file);
		n->y = GetNextInt(gpl_file);
		n->orientation = GetOrientation(gpl_file);
	
		Endline(gpl_file);
	}
}

int GenerateGrid(FILE* gg_file)
{
	struct node* n, **np;
	struct grid_hdr* rhdr; //information about the row
	int  coordinate, height, sitewidth, sitespacing, siteorient, sitesymmetry, subroworgin, numsites;
	int i, j;

	Endline(gg_file); //get rid of header
	
	NumRows = GetNextInt(gg_file);
	do{ GridHdr = (struct grid_hdr**) malloc(NumRows * sizeof(struct grid_hdr*)+2);} while(GridHdr==NULL);
	do{ Grid = (struct node ***) malloc(NumRows * sizeof(struct node**))+1;} while(Grid==NULL);
	do{ GridCpy = (struct node ***) malloc(NumRows * sizeof(struct node**))+1;} while(Grid==NULL);
	do{ OverlapGrid = (struct overlap_node***) malloc(NumRows*sizeof(struct overlap_node**)+2);}while(OverlapGrid!=NULL);
	do{ OverlapGridCpy = (struct overlap_node***) malloc(NumRows*sizeof(struct overlap_node**)+2);}while(OverlapGrid!=NULL);
	
	//make rows
	for(i=0; i<NumRows; i++)
	{
		do{ rhdr = (struct grid_hdr*) malloc(sizeof(struct grid_hdr));} while(rhdr==NULL);
		rhdr->coordinate = GetNextInt(gg_file);
		rhdr->height = GetNextInt(gg_file);
		rhdr-> sitewidth = GetNextInt(gg_file);
		rhdr-> sitespacing = GetNextInt(gg_file);
		rhdr -> siteorient = GetOrientation(gg_file);
		rhdr -> sitesymmetry = GetSymmetry(gg_file);
		rhdr -> subroworgin = GetNextInt(gg_file);
		rhdr -> numsites = GetNextInt(gg_file);
		rhdr -> numindexes = (rhdr->numsites)/GRID_GRAIN + 1;		

		GridHdr[i] = rhdr;
		if(i=0){ AvgRowHeight = rhdr->height; RowWidth = (rhdr->numsites)*(rhdr->sitewidth); }
		assert((rhdr->numsites)*(rhdr->sitewidth) == RowWidth); //assum rows are the same width
		//we will allocate an array of linked list for easy access insertion/deletion
		//indexes will represent a pointer to the start of its range of coordinates
		do{ np = (struct node**) malloc((rhdr->numindexes)  *sizeof(struct node*));} while(np==NULL);
		if(i=0){InitEmptyNode(RowWidth);} //initialize Empty Node array with approximate size

		Grid[i] = np;
				
		//cleanup
		n = NULL;
		np = NULL;
		rhdr = NULL;	
		Endline(gg_file);
	}
	
	do{ rhdr = (struct grid_hdr*) malloc(sizeof(struct grid_hdr));} while(rhdr==NULL);
	rhdr->coordinate = GridHdr[i-1]->coordinate + GridHdr[i-1]->height;
	GridHdr[i++] = rhdr;
	do{ rhdr = (struct grid_hdr*) malloc(sizeof(struct grid_hdr));} while(rhdr==NULL);
	rhdr->coordinate = INT_MIN;
	GridHdr[i++] = rhdr;
	AvgRowHeight = GridHdr[NumRows]->coordinate/NumRows;
			
	return 0;
}

void PopulateCopy()
{
	struct node *n, *ncpy;
	int i;
	for(i=0; i<Modules; i++)
	{
		n = N_Arr[i];
		ncpy = N_ArrCpy[i];
		if(n==NULL){continue;}
		assert(ncpy!=NULL);
		CopyNode(n, ncpy);
	}
}
/*
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

;
}*/
