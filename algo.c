#include <unistd.h> //getopt
#include <getopt.h> //optind
#include <stdio.h>
#include  <fcntl.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include "my_header.h"

/* I/O Flags */
//oflaf and sflag constants, for when fcntl is take out
#define OUTFILE_O_FLAGS 578
#define OUTFILE_S_FLAGS 384
//and so on... 

#define PARAM param //parameter we are testing
#define SF_L 6+1 //ibmXX. and NULL terminator length

//Compile Time Parameters/Constatnts
#define S2US 1000000 //seconds to microseconds
//#deinfe SEED_RANDOM
#define OUTPUT_CSV
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
void fm_algorithm();
int gernerate_netlist();
int cost(struct node* n);
void shuffle_partition();
void memfree();

static int getint();
static int endline();
static int to_integer(char * c);
static void help();
static void inline request_help();

/*********** main ***********
 * Overall wrapper function
 * processes argc and argv, opens/closes files
 *
 * returns 0 on success, negative errorcode otherwise
 ****************************/
int main(int argc, char **argv) {

	int return_code = argc;
	int benchmark, final_benchmark;
	struct timeval tv_start, tv_init, tv_done;
	int pins, nets; //first lines of file readings
	//command interpretation
	extern int optind;
	int arg_num; //maintaining control of command line options
	char opt; //command line options
	int i, trial, f_no; //for loop counters
	int results[NUM_TRIALS][STAT_MAX], ignore;
	int param;
	char sf_nodes[SF_L+5], sf_nets[SF_L+4], sf_pl[SF_L+2], sf_scl[SF_L+3], sf_wcl[SF_L+3];


#ifdef SEED_RANDOM
	srand(time(NULL));
#endif
	gettimeofday(&tv_start, NULL);
	f_no = 1;

	/*Process through options */
	while ((opt = getopt (argc, argv, "th:")) != EOF)
	{
		switch (opt)
		{
			case 't':
				f_no = atoi(argv[optind-1]);
				if(f_no>18 || f_no<=0){  fprintf(stderr, "Illegal Test Number\r\n", argv[optind]);	goto exit; }

			case 'h':
				help();
				return_code=0;
				goto exit;
				//break;
			case '?':
#ifdef DEBUG
				fprintf(stderr, "Error: unrecognized argument %s\r\n", argv[optind]);
#endif
				request_help();
				return_code = -1;
				goto exit;
		}
	}
	/******************************** Open Files *************************************************/
	sprintf(sf_nodes, "ibm%2d.nodes", f_no);
	sprintf(sf_nets, "ibm%2d.nets", f_no);
	sprintf(sf_pl, "ibm%2d.pl", f_no);
	sprintf(sf_scl, "ibm%2d.scl", f_no);
	sprintf(sf_wcl, "ibm%2d.wcl", f_no);

	f_nodes = fopen(sf_nodes, "r");
	f_nets = fopen(sf_nets, "r");
	f_pl = fopen(sf_pl, "r");
	f_scl = fopen(sf_scl, "r");
	f_wcl = fopen(sf_wcl, "r");


	/************************ Allocate Memory ********************************************************/
	//fnodes - need to get number of terminals(pads) and number of total nodes (modules)
	Modules = get_next_int(f_nodes); //number of nodes
	PadOffset = get_next_int(f_nodes); //terminal offset
	do{N_Arr = malloc((Modules+1)*sizeof(struct node*));} while(N_Arr==NULL);
	GenerateNodes(f_nodes); //store size of each node

	//fnets - neet to genertate the netlist graph 
	nets = get_next_int(f_nets);
	pins = get_next_int(f_nets);
	GenerateNetlist(f_nets);
	
	GenertatePlacement(f_pl);
	GenerateGrid(f_scl);


#ifdef DEBUG
	fprintf(stderr, "Pins %d\t Nets %d\tModules %d\tPadOffset %d\n", pins, nets, Modules, PadOffset);
#endif
	gettimeofday(&tv_init, NULL);

#ifdef TIME_REPORT
	fprintf(stdout, "Init Done: %ldus\n", ((tv_init.tv_sec - tv_start.tv_sec)*S2US + tv_init.tv_usec - tv_start.tv_usec));
#endif
#ifdef DEBUG
        fprintf(stderr, "%d: Creating initial placement \n", trial, benchmark);
#endif
	for(i=0; i<
		
#ifdef DEBUG
	fprintf(stderr, "%d: Creating better placement \n", trial, benchmark);
#endif
	sa_algorithm(); //do algorithm
#ifdef DEBUG
		fprintf(stderr, "%d: Analyzing Results\n", trial);
#endif


		//analize results
		TestCost=0;
		//final_benchmark = 0;
		for(i=0; i<=Modules; i++)
		{
			if(N_Arr[i]==NULL){continue;}
			/*final_benchmark +=*/ cost(N_Arr[i]);
		}
		//final_benchmark = (final_benchmark+2*nets)/4;
		final_benchmark = TestCost/2;
		gettimeofday(&tv_done, NULL);

		results[trial][STAT_BENCHMARK] = benchmark;
		results[trial][STAT_IMPROVEMENT] = benchmark - final_benchmark;
		results[trial][STAT_FINAL] = final_benchmark;
		results[trial][STAT_TIME] = (tv_done.tv_sec - tv_init.tv_sec)*S2US + tv_done.tv_usec - tv_init.tv_usec;

		stat_track(results[trial][STAT_BENCHMARK], STAT_BENCHMARK);
		stat_track(results[trial][STAT_IMPROVEMENT], STAT_IMPROVEMENT);
		stat_track(results[trial][STAT_FINAL], STAT_FINAL);
		stat_track(results[trial][STAT_TIME] , STAT_TIME);
#ifdef OUTPUT_CSV
		fprintf(stderr, "%d,%d,%d,%d,%d,%d,%d\n", trial, results[trial][STAT_BENCHMARK],results[trial][STAT_IMPROVEMENT], results[trial][STAT_FINAL], results[trial][STAT_TIME], ACnt, BCnt);
#else
		fprintf(stdout, "Trial%d:\n\t%d initial cutsize\n\t%d improvement in cutsize\n\t%d cutsize\n\t%d us\n\n", trial, results[trial][STAT_BENCHMARK],results[trial][STAT_IMPROVEMENT], results[trial][STAT_FINAL], results[trial][STAT_TIME]);
#endif


	//report
	i = stat_report_min(&ignore, STAT_FINAL);
#ifdef OUTPUT_CSV
	fprintf(stdout, "%d,%d,%d,%d\n", results[i][STAT_BENCHMARK], results[i][STAT_IMPROVEMENT],results[i][STAT_FINAL], results[i][STAT_TIME]) ;

#else
	fprintf(stdout, "Best Trial:\n\t%d cutsize  in %d us\n", results[i][STAT_FINAL], results[i][STAT_TIME]) ;
#endif
	i =  stat_report_max(&ignore, STAT_FINAL);
#ifdef OUTPUT_CSV
	fprintf(stdout, "%d,%d,%d,%d\n", stat_report_avg(STAT_BENCHMARK), stat_report_avg(STAT_IMPROVEMENT), stat_report_avg(STAT_FINAL), stat_report_avg(STAT_TIME)); 
#else
	fprintf(stdout, "Worst Trial:\n\t%d cutsize in %d us\n", results[i][STAT_FINAL], results[i][STAT_TIME]) ;
	fprintf(stdout, "Averages of %d trials:\n\t%d improvement in cutsize\n\t%d cutsize\n\t%d us\n", trial, stat_report_avg(STAT_IMPROVEMENT), stat_report_avg(STAT_FINAL), stat_report_avg(STAT_TIME)); 
#endif	

	//free memory
	memfree();
close_files:

exit:
	return return_code;

}


void fm_algorithm()
{
	struct node* n;
	struct edge * e;
	struct hyperedge* h;
	while(CMax > CMAX_OFFSET)
	{
		if((n=BHead[CMax]) == NULL)	{CMax--;  continue;}
		if(n->locked >= LOCK_THRESH){ cost(n); continue; }//this will make sure remove N from the bucket list

#ifdef DEBUG_VB_FM
		fprintf(stderr, "Testing %c%d:", n->type, n->index);
#endif

		if(n->part=='a')
		{
			if(BCnt * MAX_OFFSET > ACnt) //with ACnt --, ACnt/BCnt cannot drop below threshold
			{
				//Here lies an interesting case with many possible actions
				//i.e. Lock Node n; CMax--; move n to tail of bucklist index Cmax
				//we will increment lock the node for our case, which will move node to end of queue and give it one less chance
				n->locked += 1;
				cost(n); //remove from bucket list
				if(rand()%OFFSET_EXCEPTION!=0){continue;}
			}
			//else
			n->part = 'b'; //swap node
			ACnt--; BCnt++;
		}
		else
		{
			if(ACnt * MAX_OFFSET > BCnt)
			{
				//Here lies an interesting case with many possible actions
				//i.e. Lock Node n; CMin++; move n to tail of bucklist index Cmin
				//we will lock the node for our case
				n->locked += 1;
				cost(n); //remove from bucket list
				if(rand()%OFFSET_EXCEPTION!=0){continue;}
			}
			//else
			n->part = 'a'; //swap node
			ACnt++; BCnt--;
		}
#ifdef DEBUG_VB_FM
		fprintf(stderr, "\tMoved to %c improving cost from %d", n->part, n->cost);
#endif
		//We need to recalculate cost of connected nodes mow
		//children first
		h = n->out_head;
		while(h!=NULL)
		{
			e = h->out_head;
			while(e != NULL)
			{
				cost(e->in);
				e = e->next;
			}
			h = h->next;
		} //end children recalculation
		e = NULL;
		h = NULL;
		//now parents
		e = n->birth;
		while(e!=NULL)
		{
			cost(e->parent->out); //parent of this one input
			//we neglect to recalculate the siblings of this node's hyperedge
			//they are not a part of our cost function since they matter in only a rare instance 
			e = e->foster; //next input
		} //end parent recalculation

		n->locked +=1;
		cost(n);
#ifdef DEBUG_VB_FM
		fprintf(stderr, "to %d\n", n->cost);
#endif

		e = NULL;
		h = NULL;
		n = NULL;

	}//end while{ CMax> Offset) loop
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


