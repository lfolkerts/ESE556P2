#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include <assert.h>
#include"parameters.h"
#include "overlapgrid.h"
#include "node.h"

#ifdef DEBUG_VB_OLG //overlap grid 
#include<stdio.h>
// #define DEBUG_VB_OLGWO //work overlap
// #define DEBUG_VB_OLGACC //accept
// #define DEBUG_VB_OLGREJ //rejecti
// #define DEBUG_VB_OLGVERIFY
#endif

//variables unique to this module
int gridOverlap, gridOverlapCpy;
int extraRowWidth, extraRowWidthCpy;
void work_overlap(struct node* insert, int insert_flag);
void inline verify();

void InitOverlapGrid()
{
	gridOverlap = gridOverlapCpy = 0;
	extraRowWidth = extraRowWidthCpy = 0;
}
int GetOverlapCost(){return gridOverlap;}
int GetExtraRowWidthCost(){return extraRowWidth;}

void FillOverlapGrid()
{
	int i, x, y;
	struct  node* insert;
	for(i=0; i<=PadOffset; i++)
	{
		insert = N_Arr[i];	
		if(insert==NULL){continue;}
		if(insert->x==0 && insert->y==0)
		{
			x = rand()%(RowWidth);//-insert->width);
			y = rand()%(NumRows-1);//-insert->y/AvgRowHeight);
			y = GridHdr[y]->coordinate;
		}
		else
		{
			x = insert->x; 
			y = insert->y;
		}
		InsertOverlapNode(insert, x, y);
	}
	for(i=0; i<Modules; i++)
        {
                insert = N_Arr[i];
		if(insert==NULL){continue;}
		Cost(insert, 0, 0);
	}
	AcceptOverlapMove();
	return;
}

/***************************************************************
* This function will insert or remove a node into the proper position
* A bound check should be performed before calling this function
****************************************************************/
int InsertOverlapNode(struct node* insert, int x, int y)
{
	int cost, overlap_org, row_cost_org;
	
	overlap_org = GetOverlapCost();
	row_cost_org = GetExtraRowWidthCost();
        if(y<0){y =0;}

        insert->x = x;
        insert->y = y;
	work_overlap(insert, 1);
	cost  =  CostTimberwolf(insert, x, y,overlap_org, row_cost_org);
	return cost;
}

void work_overlap(struct node* insert, int insert_flag) 
{
	struct overlap_node* noverlap, *noverlap_insert;
	struct node* overlap_listhead, *overlap_listcheck;
	int gindexy, gindexx, gx, gy, noverlapx, noverlapy;
	int xoverlap, yoverlap, overlap;
	char ignore_flag;

	assert(insert!=NULL);
	overlap_listhead = NULL;
#ifdef DEBUG_VB_OLG
	if(insert_flag) { 	fprintf(stderr, "Inserting Overlap %c%d:\t(%5d,%5d)\n", insert->type, insert->index, insert->x, insert->y);}
	else{ 			fprintf(stderr, "Removing  Overlap %c%d:\t(%5d,%5d)\n", insert->type, insert->index, insert->x, insert->y);}
#endif

	//get to starting row
	gindexy = insert->y/AvgRowHeight;
	assert(gindexy>=0);
	while(GridHdr[gindexy]->coordinate <= insert->y){gindexy++;}  
	while(GridHdr[gindexy]->coordinate > insert->y){gindexy--;}
	insert->y = GridHdr[gindexy]->coordinate; //allign y to start of row
	gindexx = insert->x/GRID_GRAIN;
	gindexx = (gindexx>RowWidth/GRID_GRAIN)?RowWidth+1:gindexx;
#ifdef DEBUG_VB_OLGWO
	fprintf(stderr, "\tRow %d GridX %d\n", gindexy, gindexx);
#endif

	//insert, update overlap/row width
	for(gindexy; gindexy < NumRows+2 && GridHdr[gindexy]->coordinate <insert-> y + insert->height; gindexy++)
	{
		for(gindexx; gindexx*GRID_GRAIN < insert->x + insert->width; gindexx++)
		{
			gx = (gindexx<0)?RowWidth/GRID_GRAIN+2:gindexx;
			gx = (gindexx>RowWidth)?RowWidth/GRID_GRAIN+1:gx; 
			gy = (gindexy<0)?NumRows+1:gindexy;
			gy = (gindexy > NumRows)?NumRows:gy; 
#ifdef DEBUG_VB_OLGWO
                                fprintf(stderr, "\tRetrivng (Y%5d,X%5d)\n", gy, gx );
#endif
			noverlap = OverlapGrid[gy][gx];
#ifdef DEBUG_VB_OLGWO
			if(noverlap!=NULL && noverlap->node!=NULL){ fprintf(stderr, "\tRetrieved %c%d\n",noverlap->node->type, noverlap->node->index );}
			else if(noverlap!=NULL) {  fprintf(stderr, "\tRetrieved NULL Node\n" ); }
			else { fprintf(stderr, "\tRetrieved NULL\n" );}
#endif
			noverlap_insert=NULL;

			while(noverlap != NULL)
			{
#ifdef DEBUG_VB_OLGWO
				fprintf(stderr, "\tOverlap %c%d\n",noverlap->node->type, noverlap->node->index );
#endif
				if(noverlap->node == insert)
				{
					noverlap_insert = noverlap;
					noverlap = noverlap->next; 
					continue;
				}
				//check to see if we already calculated overlap for noverlap
				ignore_flag = 0;
				overlap_listcheck = overlap_listhead;
				while(overlap_listcheck != NULL)
				{
					if(overlap_listcheck == noverlap->node) //we will ignore node and continue through while loop
					{
#ifdef DEBUG_VB_OLGWO
                                		fprintf(stderr, "\t\tIgnoring Overlap %c%d\n",noverlap->node->type, noverlap->node->index );
#endif
	
						ignore_flag = 1; //ignored
						//now we must see if we can remove noverlap from the noverlap list 
						noverlapx = (noverlap->node->x + noverlap->node->width) / GRID_GRAIN;
        	                	        noverlapx = (noverlapx > RowWidth/GRID_GRAIN)?RowWidth+1:noverlapx;
	                                	noverlapy = (noverlap->node->y + noverlap->node->height)/AvgRowHeight;
	                	                while(GridHdr[noverlapy]->coordinate > noverlap->node->y){noverlapy--;}
        	                	        while(GridHdr[noverlapy]->coordinate <= noverlap->node->y){noverlapy++;}
                	                	if(noverlapx == gindexx && noverlapy == gindexy) //last time we will see noverlap-> node - remove from list
                        	        	{
                                	        	if(overlap_listcheck->north!= NULL){overlap_listcheck->north->south = overlap_listcheck->south;}
							if(overlap_listcheck->south!=NULL){overlap_listcheck->south->north = overlap_listcheck->south;}
                                       			if(overlap_listcheck == overlap_listhead){overlap_listhead = overlap_listcheck->south;}
							overlap_listcheck->north = NULL;
							overlap_listcheck->south = NULL;
                	                	}
						break; //already found noverlap - no need to move through rest of list
					}
					overlap_listcheck = overlap_listcheck->south;
				}
				if(ignore_flag == 1) { noverlap = noverlap->next; continue; }			
#ifdef DEBUG_VB_OLGWO
			        fprintf(stderr, "\t\tCalculating Overlap %c%d\n",noverlap->node->type, noverlap->node->index );
#endif
			
				//we have not calculated overlap for noverlap yet - lets do it now
				//calculate x overlap
				//------------------------------------------ insert
					//-------------------- noverlap
				if(insert->x <= noverlap->node->x && 
					insert->x +insert->width >= noverlap->node->x && 
					insert->x + insert->width >= noverlap->node->x + noverlap->node->width)
				{ xoverlap = noverlap->node->width;}	
				//--------------------insert
					//----------------------- noverlap
				else if(insert->x <= noverlap->node->x && 
					insert->x +insert->width >= noverlap->node->x)
				{xoverlap = insert->x + insert->width - noverlap->node->x; }
				//------------------------------------------ noverlap
					//-------------------- insert
				if(noverlap->node->x <= insert->x && 
					noverlap->node->x +noverlap->node->width >= insert->x && 
					noverlap->node->x + noverlap->node->width >= insert->x + insert->width)
				{ xoverlap = insert->width;}	
				//--------------------noverlap
					//----------------------- insert
				else if(noverlap->node->x <= insert->x && 
					noverlap->node->x + noverlap->node->width >= insert->x)
				{xoverlap = noverlap->node->x + noverlap->node->width - insert->x; }
				else{xoverlap = 0;}
			
				//calculate y overlap
				//same code as above, except x->y, width->height
				if(xoverlap==0){yoverlap = 0;}//not really, but save going through the logic
				 //------------------------------------------ insert
                                        //-------------------- noverlap
                                else if(insert->y <= noverlap->node->y &&
                                        insert->y +insert->height >= noverlap->node->y &&
                                        insert->y + insert->height >= noverlap->node->y + noverlap->node->height)
                                { yoverlap = noverlap->node->height;}
                                //--------------------insert
                                        //----------------------- noverlap
                                else if(insert->y <= noverlap->node->y &&
                                        insert->y +insert->height >= noverlap->node->y)
                                {yoverlap = insert->y + insert->height - noverlap->node->y; }
                                //------------------------------------------ noverlap
                                        //-------------------- insert
                                if(noverlap->node->y <= insert->y &&  
                                        noverlap->node->y +noverlap->node->height >= insert->y &&  
                                        noverlap->node->y + noverlap->node->height >= insert->y + insert->height)
                                { yoverlap = insert->height;}  
                                //--------------------noverlap
                                        //----------------------- insert
                                else if(noverlap->node->y <= insert->y &&  
                                        noverlap->node->y + noverlap->node->height >= insert->y)
                                {yoverlap = noverlap->node->y + noverlap->node->height - insert->y; }
                                else{yoverlap = 0;}

				overlap = xoverlap*yoverlap;
#ifdef DEBUG_VB_OLGWO
        fprintf(stderr, "\tFinished recalculating overlap\n");
#endif
	
				//if we will encounter this node again, append it to the noverlap list to ignore
				noverlapx = (noverlap->node->x + noverlap->node->width) / GRID_GRAIN;
				noverlapx = (noverlapx > RowWidth/GRID_GRAIN)?RowWidth/GRID_GRAIN+1:noverlapx;
				noverlapy = (noverlap->node->y + noverlap->node->height)/AvgRowHeight;
				if(noverlapy>NumRows){noverlapy = NumRows;}
				else
				{
					while(GridHdr[noverlapy]->coordinate > noverlap->node->y+noverlap->node->height){noverlapy--;}
			        	while(GridHdr[noverlapy]->coordinate <= noverlap->node->y+noverlap->node->height)
					{
						noverlapy++;
						if(noverlapy>NumRows){noverlapy = NumRows; break;}
					}
				}
				if(noverlapx != gindexx || noverlapy != gindexy)
				{
					noverlap->node->north = NULL;
					noverlap->node->south = overlap_listhead;
					overlap_listhead = noverlap->node;
				}

				if(insert_flag==0){ gridOverlap -= overlap*overlap; } //remove
				else{ gridOverlap += overlap*overlap;} //insert
				
				noverlap = noverlap->next;
#ifdef DEBUG_VB_OLGWO
        fprintf(stderr, "\tAdded to overlap list\n");
#endif


			}//end while noverlap!=NULL

			//clear overlap list
			for(overlap_listcheck = overlap_listhead; overlap_listcheck!=NULL; overlap_listcheck = overlap_listhead)
			{ 	
				overlap_listhead = overlap_listcheck->south;
				overlap_listcheck->north = NULL;
				overlap_listcheck->south = NULL;
			}
			//remove/add insert to list of nodes in this grid square
			if(insert_flag == 0) //remove
			{
				assert(noverlap_insert!=NULL);
				if(noverlap_insert->next!=NULL){noverlap_insert->next->prev = noverlap_insert->prev;}
				if(noverlap_insert->prev!=NULL){noverlap_insert->prev->next = noverlap_insert->next;}
				if(OverlapGrid[gy][gx]==noverlap_insert){OverlapGrid[gy][gx]=noverlap_insert->next;}
				noverlap_insert->next = NULL;
				noverlap_insert->prev = NULL;
				
			}
			else //insert node into grid square
			{	
#ifdef DEBUG_VB_OLGWO
			        fprintf(stderr, "\tInserting Row%4d Col%4d\n", gy, gx);
#endif
				assert(noverlap_insert==NULL);
				do{noverlap_insert = malloc(sizeof(struct overlap_node));}while(noverlap_insert==NULL);
				noverlap_insert->next = OverlapGrid[gy][gx];
				noverlap_insert->prev = NULL;
				noverlap_insert->node = insert;
				OverlapGrid[gy][gx] = noverlap_insert;
                        
#ifdef DEBUG_VB_OLGWO
                                fprintf(stderr, "\tInserted Row%4d Col%4d\n", gy, gx);
#endif

			}
		}//end column
	}//end row
	//update extra RowWidth && cleanup
	if(insert_flag==0)
	{
		if(insert->x < 0){ extraRowWidth += insert->x*insert->height; }
		else if(insert->x + insert->width > RowWidth){ extraRowWidth -= (insert->x + insert->width - RowWidth)*insert->height;}
		if(insert->y < 0){ extraRowWidth += insert->y*insert->width; }
		else if(insert->y + insert->width > GridHdr[NumRows]->coordinate)
		{ extraRowWidth -= (insert->y + insert->height - GridHdr[NumRows]->coordinate)*insert->width;}
	}	
	else
        {       
                if(insert->x < 0){ extraRowWidth -= insert->x*insert->height; }
                else if(insert->x + insert->width > RowWidth){ extraRowWidth += (insert->x + insert->width - RowWidth)*insert->height;}
		if(insert->y < 0){ extraRowWidth -= insert->y*insert->width; }
                else if(insert->y + insert->width > GridHdr[NumRows]->coordinate)
                { extraRowWidth += (insert->y + insert->height - GridHdr[NumRows]->coordinate)*insert->width;}

        }
	
	return;
}
int MoveOverlapRandom(struct node* move)
{
	struct node* blocking;
	int x,y,err;
	//remove
	work_overlap(move, 0); //remove node
	//find random spot
	x = rand()%(RowWidth);
	do
	{
		y = rand()%NumRows;	
	}while(GridHdr[y]->coordinate + move->height > GridHdr[NumRows]->coordinate);
	y = GridHdr[y]->coordinate;

	return InsertOverlapNode(move, x, y);
}

void AcceptOverlapMove()
{
        struct node *n, *ncpy;
	struct overlap_node *onode, *onodecpy, *onodecpyprev, *onodetmp;
        int i, j;
        int ecnt;
        //copy modules
#ifdef DEBUG_VB_OLG
	fprintf(stderr, "\tAccepted Move\n"); 
#endif
        for(i=0; i<Modules+1; i++)
        {
                n = N_Arr[i];
                ncpy = N_ArrCpy[i];
                if(n==NULL){continue;}
		assert(ncpy != NULL);
                CopyNode(n, ncpy);
#ifdef DEBUG_VB_OLGACC
                        fprintf(stderr, "\t\tBin Y%5dX%5d\n", i, j);
#endif
        }
#ifdef DEBUG_VB_OLGACC
        fprintf(stderr, "\t\tCopied Nodes\n");
#endif

        //copy grid ptrs
        for(i=0; i<NumRows+2; i++)
        {
                for(j=0; j<RowWidth/GRID_GRAIN+2; j++)
                {
#ifdef DEBUG_VB_OLGACC
		        fprintf(stderr, "\t\tBin Y%5dX%5d\n", i, j);
#endif

                        onode = OverlapGrid[i][j];
			onodecpy = OverlapGridCpy[i][j];
			onodecpyprev = NULL;
			while(onode != NULL || onodecpy != NULL)
			{

                        	if(onode==NULL) //remove onodecpy
				{
					if(onodecpy->prev!=NULL){onodecpy->prev->next = onodecpy->next;}
					else { OverlapGridCpy[i][j] = onodecpy->next; } //head of list
					if(onodecpy->next){onodecpy->next->prev = onodecpy->prev;}
					onodetmp = onodecpy;
					onodecpyprev = onodecpyprev;
					onodecpy = onodecpy->next;
					free(onodetmp);
					onodetmp=NULL;
					//onode is already NULL
				}
				else if(onodecpy == NULL) //add onodecpy
				{
					do{onodecpy = malloc(sizeof(struct overlap_node));}while(onodecpy==NULL);
					if(OverlapGridCpy[i][j]==NULL){ OverlapGridCpy[i][j] = onodecpy; }
					onodecpy->prev = onodecpyprev;
					if(onodecpyprev!=NULL){onodecpyprev->next = onodecpy;}
					onodecpy->next = NULL;
					if(onode->node->type=='a'){onodecpy->node = N_ArrCpy[onode->node->index];}
					else if(onode->node->type=='p'){onodecpy->node = N_ArrCpy[onode->node->index + PadOffset];}
					onodecpyprev = onodecpy;
                                        onodecpy = onodecpy->next; //NULL
					onode = onode->next;
				}	
				else if(onode->node->type != onodecpy->node->type ||
						onode->node->index != onodecpy->node->index) //different nodes
				{
					if(onode->node->type=='a'){onodecpy->node = N_ArrCpy[onode->node->index];}
					else if(onode->node->type=='p'){onodecpy->node = N_ArrCpy[onode->node->index + PadOffset];}
					onodecpyprev = onodecpy;
					onodecpy = onodecpy->next;
					onode = onode->next;
				}
			        else
                                {
                                        onode = onode->next;
                                	onodecpyprev = onodecpy;
				        onodecpy = onodecpy->next;
                                }
			
			}
                }
        }
	verify();
}
void RejectOverlapMove()
{
	struct node *n, *ncpy;
	struct overlap_node *onode, *onodecpy, *onodeprev, *onodetmp;
        int i, j;
        int ecnt;
#ifdef DEBUG_VB_OLG
        fprintf(stderr, "\tRejected Move\n");
#endif

        //copy modules
        for(i=0; i<Modules+1; i++)
        {
                n = N_Arr[i];
                ncpy = N_ArrCpy[i];
                if(n==NULL){continue;}
                assert(ncpy != NULL);
                CopyNode(ncpy, n); //no pointer - can directly copy
        }
#ifdef DEBUG_VB_OLGREJ
        fprintf(stderr, "\t\tCopied Nodes\n");
#endif

        //copy grid ptrs
        for(i=0; i<NumRows+2; i++)
        {
                for(j=0; j<RowWidth/GRID_GRAIN+2; j++)
                {
#ifdef DEBUG_VB_OLGREJ
                        fprintf(stderr, "\t\tBin Y%5dX%5d\n", i, j);
#endif
                        onode = OverlapGrid[i][j];
			onodecpy = OverlapGridCpy[i][j];
			onodeprev = NULL;
			while(onodecpy != NULL || onode != NULL)
			{

                        	if(onodecpy==NULL) //remove onode
				{
#ifdef DEBUG_VB_OLGREJ
            			        fprintf(stderr, "\t\tOnode Deleted\n");
#endif
            
					if(onode->prev!=NULL){onode->prev->next = onode->next;}
					else { OverlapGrid[i][j] = onode->next; } //head of list
					if(onode->next){onode->next->prev = onode->prev;}
					onodetmp = onode;
					onodeprev = onodeprev;
					onode = onode->next;
					free(onodetmp);
					onodetmp=NULL;
					//onodecpy is already NULL
				}
				else if(onode == NULL) //add onode
				{
#ifdef DEBUG_VB_OLGREJ
                                        fprintf(stderr, "\t\tOnodeCpy Deleted\n");
#endif

					do{onode = malloc(sizeof(struct overlap_node));}while(onode==NULL);
					onode->prev = onodeprev;
					if(onodeprev!=NULL){onodeprev->next = onode;}
					onode->next = NULL;
					if(onodecpy->node->type=='a'){onode->node = N_Arr[onodecpy->node->index];}
					else if(onodecpy->node->type=='p'){onode->node = N_Arr[onodecpy->node->index + PadOffset];}
					onodeprev = onode;
                                        onode = onode->next; //NULL
					onodecpy = onodecpy->next;
				}	
				else if(onodecpy->node->type != onode->node->type ||
						onodecpy->node->index != onode->node->index) //different nodes
				{
#ifdef DEBUG_VB_OLGREJ
                                        fprintf(stderr, "\t\tOnode Replaces\n");
#endif

					if(onodecpy->node->type=='a'){onode->node = N_Arr[onodecpy->node->index];}
					else if(onodecpy->node->type=='p'){onode->node = N_Arr[onodecpy->node->index + PadOffset];}
					onodeprev = onode;
					onode = onode->next;
					onodecpy = onodecpy->next;
				}
				else
				{
					onodeprev = onode;
					onode = onode->next;
					onodecpy = onodecpy->next;
				}
					
			}
                }
        }
	verify();
}

int CostTimberwolf(struct node* ncost, int xorg, int yorg, int overlaporg, int row_widthorg)
{
	return Cost(ncost, xorg, yorg) +
		(GetOverlapCost()-overlaporg)*OVERLAP_WEIGHT +
		(GetExtraRowWidthCost() - row_widthorg)* ROW_WIDTH_WEIGHT;
}

void inline verify()
{
#ifdef DEBUG_VB_OLGVERIFY
	int i, j;
	struct overlap_node *onode, *onodecpy;
	for(i=0; i<NumRows+2; i++)
        {
                for(j=0; j<RowWidth/GRID_GRAIN+2; j++)
                {
			onode = OverlapGrid[i][j];
                        onodecpy = OverlapGridCpy[i][j];
			while(onode != NULL || onodecpy !=NULL)
			{
				fprintf(stderr, "\t\tVerify Bin (Y%4d,X%4d)\n", i, j);
				assert(onode != NULL);
				assert(onodecpy != NULL);
				assert(onode->node->index == onodecpy->node->index);
				onode = onode->next;
				onodecpy = onodecpy->next;
			}
		}	
	}

#endif
}
