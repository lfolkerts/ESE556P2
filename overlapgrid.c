#include <stdio.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include"parameters.h"
#include "overlapgrid.h"
#include "node.h"


struct overlap_node *** OverlapGrid, OverlapGridCpy;
//variables unique to this module
int gridOverlap, gridOverlapCpy;
int extraRowWidth, extraRowWidthCpy;
void work_overlap(struct node* insert, int insert_flag);

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
	for(i=0; i<Modules; i++)
	{
		insert = N_Arr[i];	
		if(insert==NULL){continue;}
		if(insert->x==0 && insert->y==0)
		{
			x = rand()%RowWidth;
			y = rand()%NumRows;
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
		Cost(insert);
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
	struct node* noverlap_listhead, *noverlap_listcheck;
	int gindexy, gindexx, gx, gy, noverlapx, noverlapy;
	int xoverlap, yoverlap, overlap;
	char ignore_flag;

	assert(insert!=NULL);

	//get to starting row
	gindexy = y/AvgRowHeight;
	while(GridHdr[gindexy]->coordinate > insert->y){gindexy--;}
	while(GridHdr[gindexy]->coordinate <= insert->y){gindexy++;}  
	insert->y = GridHdr[gindexy]->coordinate; //allign y to start of row
	gindexx = insert->x/GRID_GRAIN;
	gindexx = (gindexx>RowWidth/GRID_GRAIN)?RowWidth+1:gindexx;

	//insert, update overlap/row width
	for(gindexy; GridHdr[gindexy]->coordinate < y + insert->height; gindexy++)
	{
		for(gindexx; gindex*GRID_GRAIN < x + insert->width; gindexx++)
		{
			gx = (gindexx<0)?RowWidth/GRID_GRAIN+2:gindexx;
			gy = (gindexy<0)?NumRows+2:gindexy;
			noverlap = OverlapGrid[gy][gx];
			noverlap_insert=NULL;
			while(noverlap != NULL)
			{
				if(noverlap->node == insert)
				{
					noverlap_insert = noverlap;
					noverlap = noverlap->next; 
					continue;
				}
				//check to see if we already calculated overlap for noverlap
				ignore_flag = 0;
				noverlap_listcheck = noverlap_listhead;
				while(noverlap_listcheck != NULL)
				{
					if(noverlap_listcheck == noverlap->node) //we will ignore node and continue through while loop
					{	
						ignore_flag = 1; //ignored
						//can we remove noverlap from the noverlap list??
						noverlapx = (noverlap->node->x + noverlap->node->width) / GRID_GRAIN;
        	                	        noverlapx = (noverlapx > RowWidth/GRID_GRAIN)?RowWidth+1:noverlapx;
	                                	noverlapy = (noverlap->node->y + noverlap->node->height)/AvgRowHeight;
	                	                while(GridHdr[noverlapy]->coordinate > noverlap->node->y){noverlapy--;}
        	                	        while(GridHdr[noverlapy]->coordinate <= noverlap->node->y){noverlapy++;}
                	                	if(noverlapx == gindexx && noverlapy == gindexy) //last time we will see noverlap-> node - remove from list
                        	        	{
                                	        	if(noverlap_listcheck->north!= NULL){noverlap_listcheck>north->south = noverlap_listcheck->south;}
							if(noverlap_listcheck->node->south!=NULL){noverlap_listcheck->south->north = noverlap_listcheck->south;}
                                       			if(noverlaplistchek == noverlap_listhead){noverlap_listhead = noverlaplistcheck->node->south;}
							noverlaplistcheck->north = NULL;
							noverlaplistcheck->south = NULL;
                	                	}
						break; //already found noverlap - no need to move through rest of list
					}
				}
				if(ignore_flag == 1) { noverlap = noverlap->next; continue; }			
				
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
                                if(noverlap->y <= insert->y &&  
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
				
				//if we will encounter this node again, append it to the noverlap list to ignore
				noverlapx = (noverlap->node->x + noverlap->node->width) / GRID_GRAIN;
				noverlapx = (noverlapx > RowWidth/GRID_GRAIN)?RowWidth+1:noverlapx;
				noverlapy = (noverlap->node->y + noverlap->node->height)/AvgRowHeight;
				while(GridHdr[noverlapy]->coordinate > noverlap->node->y){noverlapy--;}
			        while(GridHdr[noverlapy]->coordinate <= noverlap->node->y){noverlapy++;}
				if(noverlapx != gindexx || noverlapy != gindexy)
				{
					noverlap->node->north = NULL;
					noverlap->node->south = noverlap_listhead;
					noverlap_listhead = noverlap->node;
				}

				if(insert_flag==0){ gridOverlap -= overlap*overlap; } //remove
				else{ gridOverLap += overlap*overlap;} //insert
				
				noverlap = noverlap->next;
			}//end while
			//clear noverlap list
			for(noverlap->node = noverlap_listhead; noverlap->node!=NULL; noverlap->node = noverlap_listhed)
			{ 	
				noverlap_listhead = noverlap->south;
				noverlap->node->north = NULL;
				noverlap->node->south = NULL;
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
				assert(noverlap_insert==NULL);
				do{noverlap_insert = malloc(sizeof(overlap_node))}while(noverlap_insert==NULL);
				noverlap_insert->next = OverlapGrid[gx][gy];
				noverlap_insert->prev = NULL;
				OverlapGrid[gx][gy] = noverlap_insert;
			}
		}
		
	}
	//update extra RowWidth && cleanup
	if(insert_flag==0)
	{
		if(insert->x < 0){ extraRowWidth += insert->x*insert->height; }
		else if(insert->x + insert->width > RowWidth){ extraRowWidth -= (insert->x + insert->width - RowWidth)*insert->height;}
	}
	else
        {       
                if(insert->x < 0){ extraRowWidth -= insert->x*insert->height; }
                else if(insert->x + insert->width > RowWidth){ extraRowWidth += (insert->x + insert->width - RowWidth)*insert->height;}
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
	struct overlap_node *onode, *onodecpy, *onodecpyprev *onodetmp;
        int i, j;
        int ecnt;
        //copy modules
        for(i=0; i<Modules; i++)
        {
                n = N_Arr[i];
                ncpy = N_ArrCpy[i];
                if(n==NULL){continue;}
                assert(ncpy != NULL);
                CopyNode(n, ncpy);
        }
        //copy grid ptrs
        for(i=0; i<NumRows+2; i++)
        {
                for(j=0; j<RowWidth/GRID_GRAIN+2; j++)
                {
                        onode = OverlapGrid[i][j];
			onodecpy = OverlapGridCpy[i][j];
			while(onode != NULL || onodecpy != NULL)
			{

                        	if(onode==NULL) //remove onodecpy
				{
					if(onodecpy->prev){onodecpy->prev->next = onodecpy->next;}
					else { OverlapGridCpy[i][j] = onodecpy->next; } //head of list
					if(onodecpy->next){onodecpy->next->prev = onodecpy->prev;}
					onodetmp = onodecpy;
					onodecpyprev = onodecpy;
					onodecpy = onodecpy->next;
					free(onodetmp);
					onodetmp=NULL;
					//onode is already NULL
				}
				else if(onodecpy == NULL)
				{
					do{onodecpy = malloc(sizeof(overlap_node));}while(onodecpy==NULL);
					onodecpy->prev = onodecpyprev;
					onodecpy->next = NULL:
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
					
			}
			onodecpyprev = NULL;
                }
        }
}
void RejectOverlapMove()
{
	struct node *n, *ncpy;
	struct overlap_node *onode, *onodecpy, *onodeprev *onodetmp;
        int i, j;
        int ecnt;
        //copy modules
        for(i=0; i<Modules; i++)
        {
                n = N_Arr[i];
                ncpy = N_ArrCpy[i];
                if(n==NULL){continue;}
                assert(ncpy != NULL);
                CopyNode(ncpy, n); //no pointer - can directly copy
        }
        //copy grid ptrs
        for(i=0; i<NumRows+2; i++)
        {
                for(j=0; j<RowWidth/GRID_GRAIN+2; j++)
                {
                        onode = OverlapGrid[i][j];
			onodecpy = OverlapGridCpy[i][j];
			while(onodecpy != NULL || onode != NULL)
			{

                        	if(onodecpy==NULL) //remove onodecpy
				{
					if(onode->prev){onode->prev->next = onode->next;}
					else { OverlapGrid[i][j] = onode->next; } //head of list
					if(onode->next){onode->next->prev = onode->prev;}
					onodetmp = onode;
					onodeprev = onode;
					onode = onode->next;
					free(onodetmp);
					onodetmp=NULL;
					//onodecpy is already NULL
				}
				else if(onode == NULL)
				{
					do{onode = malloc(sizeof(overlap_node));}while(onode==NULL);
					onode->prev = onoderev;
					onode->next = NULL:
					if(onodecpy->node->type=='a'){onode->node = N_Arr[onodecpy->node->index];}
					else if(onodecpy->node->type=='p'){onode->node = N_Arr[onodecpy->node->index + PadOffset];}
					onodeprev = onode;
                                        onode = onode->next; //NULL
					onodecpy = onodecpy->next;
				}	
				else if(onodecpy->node->type != onode->node->type ||
						onodecpy->node->index != onode->node->index) //different nodes
				{
					if(onodecpy->node->type=='a'){onode->node = N_ArrCpy[onodecpy->node->index];}
					else if(onodecpy->node->type=='p'){onode->node = N_ArrCpy[onodecpy->node->index + PadOffset];}
					onodeprev = onode;
					onode = onode->next;
					onodecpy = onodecpy->next;
				}
					
			}
			onodeprev = NULL;
                }
        }
}
