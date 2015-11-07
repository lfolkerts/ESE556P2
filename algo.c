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


#ifdef
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
#ifdef DEBUG
        fprintf(stderr, "%d: Creating initial placement \n", trial, benchmark);
#endif
	InitialPlacementOverlap();
	for(i=0; i<Modules; i++)
	{
		n = NArr[i];
                if(n==NULL){continue;}
		cost(n);
                cost_org += n->cost;
	}
	
#ifdef DEBUG
	fprintf(stderr, "%d: Creating better placement \n", trial, benchmark);
#endif
	sa_algorithm_overlap(Modules); //do algorithm
	FillGrid(); //get rid of overlaps
	sa_algorithm_local(Modules, INIT_TEMPERATURE);
#ifdef DEBUG
	fprintf(stderr, "%d: Analyzing Results\n", trial);
#endif
	cost = 0;
	for(i=0; i<Modules; i++)
	{
		n = NArr[i];
		if(n==NULL){continue;}
		cost += n->cost;
	}
	gettimeofday(&tv_done, NULL);

	//report
#ifdef OUTPUT_CSV
	fprintf(stdout, "%d,%d,%d\n", cost, cost_org - cost, (tv_done.tv_sec - tv_start.tv_sec)*S2US + tv_done.tv_usec - tv_start.tv_usec);
#else
	 fprintf(stdout, "Cost: %d\nImprovement: %d\nTime: %d\n", cost, cost_org - cost, (tv_done.tv_sec - tv_start.tv_sec)*S2US + tv_done.tv_usec - tv_start.tv_usec);
#endif

	//free memory
	memfree();
close_files:

exit:
	return return_code;

}

void sa_algorithm_local(int iterations)
{
        struct node* n;
        struct edge * e;
        struct hyperedge* h;
        double temperature, alpha;
        int iterations;
        double acceptance;
        int i, nindex;
        int org_cost, cost_mod;

        for(temperature = INIT_TEMPERATURE; temperature > FINAL_TEMPERATUE; temperture*=alpha)
        {
                for(i=0; i<iterations; i++)
                {
                        nindex = rand()%(Modules-PadOffset);
                        n = NArr[nindex];
                        org_cost = GetOverlapNodeCost(n);
                        MoveLocal(n);
                        cost_mod = GetOverlapNodeCost() - org_cost;
                        if(cost_mod < 0){ acceptance = exp(-cost_mod/temperature) * RAND_PRECISION;}
                        else{acceptance = 0; } //guarenteed acceptance
                        if(rand()%RAND_PRECISION > acceptance)
                        {
                                AcceptMoveOverlap();
                        }
                        else
                        {
                                RejectMoveOverlap();
                        }
                }
                update_iterations(&iterations);
                update_alpha(&alpha);
        }
}


void sa_algorithm_overlap(int iterations)
{
	struct node* n;
	struct edge * e;
	struct hyperedge* h;
	double temperature, alpha;	
	int iterations;
	double acceptance;
	int i, nindex;
	int org_cost, cost_mod;

	for(temperature = INIT_TEMPERATURE; temperature > FINAL_TEMPERATUE; temperture*=alpha)
	{
		for(i=0; i<iterations; i++)
		{
			nindex = rand()%(Modules-PadOffset);
			n = NArr[nindex];
			org_cost = GetOverlapNodeCost(n);
			MoveOverlapRandom(n);
			cost_mod = GetOverlapNodeCost() - org_cost;
			if(cost_mod < 0){ acceptance = exp(-cost_mod/temperature) * RAND_PRECISION;}
			else{acceptance = 0; } //guarenteed acceptance
			if(rand()%RAND_PRECISION > acceptance)
			{	
				AcceptMove();
			}
			else
			{ 
				RejectMove(); 
			}
		}
		update_iterations(&iterations);
		update_alpha(&alpha);
	}
}

void update_iterations(int* iter)
{
	return;
}
void update_alpha(double *alpha)
{
	static increase=1;
	if(*alpha>=0.95){increase = 0;}
	if(increase==1){*alpha += 0.01;}
	else{*alpha -= 0.01; }
	return;
}
	


