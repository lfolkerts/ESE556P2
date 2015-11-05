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
	int i, trial, tmp; //for loop counters
	int results[NUM_TRIALS][STAT_MAX], ignore;
	int param;
	char file_no[2];
	char sf_nodes[SF_L+5], sf_nets[SF_L+4], sf_pl[SF_L+2], sf_scl[SF_L+3], sf_wcl[SF_L+3];


	ACnt = BCnt = 0; //count of nodes in each partition
	CMin = 0; //minimum intdex of bucket list
#ifdef SEED_RANDOM
	srand(time(NULL));
#endif
	gettimeofday(&tv_start, NULL);
	param = 0;

	/*Process through options */
	while ((opt = getopt (argc, argv, "ph:")) != EOF)
	{
		switch (opt)
		{
			case 't':
				tmp = atoi(argv[optind-1]);
				if(tmp>18 || tmp<=0){  fprintf(stderr, "Illegal Test Number\r\n", argv[optind]);	goto exit; }

				sf_no[0] = '0' + (tmp/10);
				sf_no[1] = '0' + (tmp%10);

			case 'p': 
				param = atoi(argv[optind-1]);
				break;
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
	strcpy(sf_nodes, "ibm", 3); 	strcpy(sf_nodes[3], sf_no, SF_LEN-5); 	str_cpy(sf_nodes[SF_LEN-2], ".nodes", 2+5);
	strcpy(sf_nets, "ibm", 4);	strcpy(sf_nETS[3], sf_no, SF_LEN-5); 	str_cpy(sf_nets[SF_LEN-2], ".nets", 2+4);
	strcpy(sf_pl, "ibm", 2);  	strcpy(sf_pl[3], sf_no, SF_LEN-5); 	str_cpy(sf_pl[SF_LEN-2], ".pl", 2+2);
	strcpy(sf_scl, "ibm", 3); 	strcpy(sf_scl[3], sf_no, SF_LEN-5); 	str_cpy(sf_scl[SF_LEN-2], ".scl", 2+3);
	strcpy(sf_wcl, "ibm", 3); 	strcpy(sf_wcl[3], sf_no, SF_LEN-5); 	str_cpy(sf_wcl[SF_LEN-2], ".wcl", 2+3);

	f_nodes = fopen(sf_nodes, "r");
	f_nets = fopen(sf_nodes, "r");
	f_pl = fopen(sf_nodes, "r");
	f_pl = fopen(sf_nodes, "r");
	f_wcl = fopen(sf_nodes, "r");


	/************************ Allocate Memory ********************************************************/
	//fnodes - need to get number of terminals(pads) and number of total nodes (modules)
	Modules = get_next_int(f_nodes); //number of nodes
	PadOffset = get_next_int(f_nodes); //terminal offset
	do{N_Arr = malloc((Modules+1)*sizeof(struct node*));} while(N_Arr==NULL);
	generate_node(f_nodes); //store size of each node

	//fnets - neet to genertate the netlist graph 
	nets = get_next_int(f_nets);
	pins = get_next_int(f_nets);
	generate_netlist(f_nets);


	stat_init(STAT_BENCHMARK);
	stat_init(STAT_IMPROVEMENT);
	stat_init(STAT_FINAL);
	stat_init(STAT_TIME);

#ifdef DEBUG
	fprintf(stderr, "Pins %d\t Nets %d\tModules %d\tPadOffset %d\n", pins, nets, Modules, PadOffset);
	fprintf(stderr, "Generating Netlist\n");	
#endif
#ifdef DEBUG
	fprintf(stderr, "Finished Generating netlist\n");
#endif
	gettimeofday(&tv_init, NULL);

#ifdef TIME_REPORT
	fprintf(stdout, "Init Done: %ldus\n", ((tv_init.tv_sec - tv_start.tv_sec)*S2US + tv_init.tv_usec - tv_start.tv_usec));
#endif

	for(trial = 0; trial < NUM_TRIALS; trial++)
	{
#ifdef DEBUG
		fprintf(stderr, "%d: Generating initil partitions\n", trial);
#endif
		shuffle_partition();

		gettimeofday(&tv_init, NULL);
		//generate bucket list; calclcate benchmark
#ifdef DEBUG
		fprintf(stderr, "%d: Generating benchmark\n", trial);
#endif

		TestCost = 0;
		//benchmark = 0;
		for(i=0; i<=Modules; i++)
		{
			if(N_Arr[i]==NULL){continue;}
			/*benchmark +=*/ cost(N_Arr[i]);
		}
		//E_costs = E_diff - E_sim; E_ denotes sum of
		//2*nets*modules = E_diff + E_sim;
		// E_sim = E_diff + E_costs = 2*nets*modules - E_diff ->
		//E_diff*2 = E_costs*modules + 2*nets 
		//E_diff/2 = cut set size 
		//benchmark = (benchmark+2*nets*modules)/4; //cutset size
		benchmark = TestCost/2;

#ifdef DEBUG
		fprintf(stderr, "%d: Creating better partition \n", trial, benchmark);
#endif
		fm_algorithm(); //do algorithm
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

	}

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


