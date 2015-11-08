#include <unistd.h> //getopt
#include <getopt.h> //optind
#include <stdio.h>
#include  <fcntl.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "parameters.h"
#include "generate_data.h"
#include "grid.h"
#include "helper.h"
#include "node.h"
#include "overlapgrid.h"

void sa_algorithm_no_overlap(int iterations);
void sa_algorithm_shift(int iterations);
void sa_algorithm_overlap(int iterations);
void update_iterations(int* iter);
void update_alpha(double *alpha);

/*********** main ***********
 * Overall wrapper function
 * processes argc and argv, opens/closes files
 *
 * returns 0 on success, negative errorcode otherwise
 ****************************/
int main(int argc, char **argv) {

	int return_code = argc;
	struct timeval tv_start, tv_init, tv_done;
	struct node* n;
	//command interpretation
	extern int optind;
	int arg_num; //maintaining control of command line options
	char opt; //command line options
	int i, f_no; //for loop counters
	int cost_org, cost;
	char sf_nodes[SF_L+5], sf_nets[SF_L+4], sf_pl[SF_L+2], sf_scl[SF_L+3], sf_wcl[SF_L+3];
	FILE *f_nodes, *f_nets, *f_pl, *f_scl, *f_wcl;

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
				f_no = atoi(argv[optind]);
				if(f_no > 18 || f_no <= 0){  fprintf(stderr, "Illegal Test Number %d\r\n", f_no);	goto exit; }

			case 'h':
				Help();
				return_code=0;
				goto exit;
				//break;
			case '?':
#ifdef DEBUG
				fprintf(stderr, "Error: unrecognized argument %s\r\n", argv[optind]);
#endif
				RequestHelp();
				return_code = -1;
				goto exit;
		}
	}
	/******************************** Open Files *************************************************/
	sprintf(sf_nodes, "%sibm%02d/ibm%02d.nodes", TEST_DIR, f_no, f_no);
	sprintf(sf_nets, "%sibm%02d/ibm%02d.nets", TEST_DIR, f_no, f_no);
	sprintf(sf_pl, "%sibm%02d/ibm%02d.pl", TEST_DIR, f_no, f_no);
	sprintf(sf_scl, "%sibm%02d/ibm%02d.scl", TEST_DIR, f_no, f_no);
	sprintf(sf_wcl, "%sibm%02d/ibm%02d.wcl", TEST_DIR, f_no, f_no);

	f_nodes = fopen(sf_nodes, "r");
	f_nets = fopen(sf_nets, "r");
	f_pl = fopen(sf_pl, "r");
	f_scl = fopen(sf_scl, "r");
	f_wcl = fopen(sf_wcl, "r");

#ifdef DEBUG_VB_ALGO
	fprintf(stderr, "Opened Files:\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n", sf_nodes, sf_nets, sf_pl, sf_scl, sf_wcl);
#endif
	/************************ Allocate Memory ********************************************************/
	//read from files and generate initial data structures based on info
	GenerateNodes(f_nodes); 
	GenerateNetlist(f_nets);
	GeneratePlacement(f_pl);
	GenerateGrid(f_scl);
	InitOverlapGrid();
	InitGrid();
	InitEmptyNodeList(Modules*9);
	
	//wcl only contains area informaition
#ifdef DEBUG
	fprintf(stderr, "Height %d\t Width %d\tModules %d\tPadOffset %d\n", GridHdr[NumRows]->coordinate, RowWidth, Modules, PadOffset);
#endif
	gettimeofday(&tv_init, NULL);
#ifdef DEBUG
        fprintf(stderr, "Creating initial placement \n");
#endif
	FillOverlapGrid();
	cost_org = 0;
	for(i=0; i<Modules; i++)
        {
                n = N_Arr[i];
                if(n==NULL){continue;}
                cost_org += n->cost;
        }

	
#ifdef DEBUG
	fprintf(stderr, "Creating better placement \n");
#endif
	sa_algorithm_overlap(Modules); //do algorithm
	//FillGrid(); //get rid of overlaps
	//sa_algorithm_shift(Modules, INIT_TEMPERATURE);
#ifdef DEBUG
	fprintf(stderr, "Analyzing Results\n");
#endif
	cost = 0;
	for(i=0; i<Modules; i++)
	{
		n = N_Arr[i];
		if(n==NULL){continue;}
		cost += n->cost;
	}
	gettimeofday(&tv_done, NULL);

	//report
#ifdef OUTPUT_CSV
	fprintf(stdout, "%d,%d,%ld\n", cost, cost_org - cost, (tv_done.tv_sec - tv_start.tv_sec)*S2US + tv_done.tv_usec - tv_start.tv_usec);
#else
	 fprintf(stdout, "Cost: %d\nImprovement: %d\nTime: %ld\n", cost, cost_org - cost, (tv_done.tv_sec - tv_start.tv_sec)*S2US + tv_done.tv_usec - tv_start.tv_usec);
#endif

close_files:

exit:
	return return_code;

}

void sa_algorithm_no_overlap(int iterations)
{
        struct node* n;
        struct edge * e;
        struct hyperedge* h;
        double temperature, alpha;
        double acceptance;
        int i, nindex;
        int org_cost, cost_mod;

	alpha = INIT_ALPHA;
        for(temperature = INIT_TEMPERATURE; temperature > FINAL_TEMPERATURE; temperature*=alpha)
        {
                for(i=0; i<iterations; i++)
                {
                        nindex = rand()%(Modules-PadOffset);
                        n = N_Arr[nindex];
                        //org_cost = GetOverlapNodeCost(n);
                        cost_mod = MoveRandom(n);
                        //cost_mod = GetOverlapNodeCost() - org_cost;
                        if(cost_mod < 0){ acceptance = exp(-cost_mod/temperature) * RAND_PRECISION;}
                        else{acceptance = 0; } //guarenteed acceptance
                        if(rand()%RAND_PRECISION > acceptance)
                        {
                                AcceptOverlapMove();
                        }
                        else
                        {
                                RejectOverlapMove();
                        }
                }
                update_iterations(&iterations);
                update_alpha(&alpha);
        }
}


void sa_algorithm_shift(int iterations)
{
        struct node* n;
        struct edge * e;
        struct hyperedge* h;
        double temperature, alpha;
        double acceptance;
        int i, nindex;
        int org_cost, cost_mod;

	alpha = INIT_ALPHA;
        for(temperature = INIT_TEMPERATURE; temperature > FINAL_TEMPERATURE; temperature*=alpha)
        {
                for(i=0; i<iterations; i++)
                {
                        nindex = rand()%(Modules-PadOffset);
                        n = N_Arr[nindex];
                        //org_cost = GetOverlapNodeCost(n);
                        cost_mod = MoveShift(n);
                        //cost_mod = GetOverlapNodeCost() - org_cost;
                        if(cost_mod < 0){ acceptance = exp(-cost_mod/temperature) * RAND_PRECISION;}
                        else{acceptance = 0; } //guarenteed acceptance
                        if(rand()%RAND_PRECISION > acceptance)
                        {
                                AcceptOverlapMove();
                        }
                        else
                        {
                                RejectOverlapMove();
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
	double acceptance;
	int i, nindex;
	int org_cost, cost_mod;
	
	alpha = INIT_ALPHA;
	for(temperature = INIT_TEMPERATURE; temperature > FINAL_TEMPERATURE; temperature*=alpha)
	{
		for(i=0; i<iterations; i++)
		{
			nindex = rand()%(Modules-PadOffset);
			n = N_Arr[nindex];
			cost_mod = MoveOverlapRandom(n);
			if(cost_mod < 0){ acceptance = exp(-cost_mod/temperature) * RAND_PRECISION;}
			else{acceptance = 0; } //guarenteed acceptance
			if(rand()%RAND_PRECISION > acceptance)
			{	
				AcceptOverlapMove();
			}
			else
			{ 
				RejectOverlapMove(); 
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
	


