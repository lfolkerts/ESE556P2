struct grid_hdr** GridHdr;
struct node *** Grid;
struct node* RemoveStack;
struct node** EmptyNodeList;
int* EmptyNodeCnt;
int NumRows;
int RowWidth;

//variables unique to this module
static char gridLock;
static uint32_t emptyNodeID;
int eNodeListSize;

void InitOverlapGrid()
{
	gridLock = 0;
	gridOverlap = 0;
	extraRowWidth = 0;
}
int getOverlapCost(){return gridOverlap;}
int getExtraRowWidthCost(){return extraRowWidth;}

/***************************************************************
* This function will insert or remove a node into the proper position
* A bound check should be performed before calling this function
****************************************************************/
void InsertOverlapNode(struct node* insert, int x, int y)
{
        if(y<0){y =0;}

        insert->x = x;
        insert->y = y;
	workon_overlap(insert, 1);
}

void work_overlap(struct node* insert, int insert_flag) 
{
	struct node* noverlap, *noverlap_listhead, noverlap_listcheck;
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
			while(noverlap != NULL)
			{
				if(noverlap == insert){noverlap = noverlap->next; continue;}
				//check to see if we already calculated overlap for noverlap
				ignore_flag = 0;
				noverlap_listcheck = noverlap_listhead;
				while(noverlap_listcheck != NULL)
				{
					if(noverlap_listcheck == noverlap) //we will ignore node and continue through while loop
					{	
						ignore_flag = 1;
						//can we remove noverlap from the noverhead list
						noverlapx = (noverlap->x + noverlap->width) / GRID_GRAIN;
        	                	        noverlapx = (noverlap>RowWidth/GRID_GRAIN)?RowWidth+1:noverlapx;
	                                	noverlapy = (noverlap->y + noverlap->height)/AvgRowHeight;
	                	                while(GridHdr[noverlapy]->coordinate > y){noverlapy--;}
        	                	        while(GridHdr[noverlapy]->coordinate <= y){noverlapy++;}
                	                	if(noverlapx == gindexx && noverlapy == gindexy) //last time we will see noverlap - remove from list
                        	        	{
                                	        	if(noverlap->north!= NULL){noverlap->north->south = noverlap->south;}
							if(noverlap->south!=NULL){noverlap->south->north = noverlap->south;}
                                       			if(noverlap = noverlap->listhead){noverlap->listhead = noverlap->south;}
							noverlap->north = NULL;
							noverlap->south = NULL;
                	                	}
						break; //already found noverlap - no need to move through rest of list
					}
				}
				if(ignore_flag == 1) { noverlap = noverlap->e_next; continue; }			
				
				//we have not calculated overlap for noverlap yet - lets do it now
				//calculate x overlap
				//------------------------------------------ insert
					//-------------------- noverlap
				if(insert->x <= noverlap->x && 
					insert->x +insert->width >= noverlap->x && 
					insert->x + insert->width >= noverlap->x + noverlap->width)
				{ xoverlap = noverlap->width;}	
				//--------------------insert
					//----------------------- noverlap
				else if(insert->x <= noverlap->x && 
					insert->x +insert->width >= noverlap->x)
				{xoverlap = insert->x + insert->width - noverlap->x; }
				//------------------------------------------ noverlap
					//-------------------- insert
				if(noverlap->x <= insert->x && 
					noverlap->x +noverlap->width >= insert->x && 
					noverlap->x + noverlap->width >= insert->x + insert->width)
				{ xoverlap = insert->width;}	
				//--------------------noverlap
					//----------------------- insert
				else if(noverlap->x <= insert->x && 
					noverlap->x + noverlap->width >= insert->x)
				{xoverlap = noverlap->x + noverlap->width - insert->x; }
				else{xoverlap = 0;}
			
				//calculate y overlap
				//same code as above, except x->y, width->height
				if(xoverlap==0){yoverlap = 0;}//not really, but save going through the logic
				 //------------------------------------------ insert
                                        //-------------------- noverlap
                                else if(insert->y <= noverlap->y &&
                                        insert->y +insert->height >= noverlap->y &&
                                        insert->y + insert->height >= noverlap->y + noverlap->height)
                                { yoverlap = noverlap->height;}
                                //--------------------insert
                                        //----------------------- noverlap
                                else if(insert->y <= noverlap->y &&
                                        insert->y +insert->height >= noverlap->y)
                                {yoverlap = insert->y + insert->height - noverlap->y; }
                                //------------------------------------------ noverlap
                                        //-------------------- insert
                                if(noverlap->y <= insert->y &&  
                                        noverlap->y +noverlap->height >= insert->y &&  
                                        noverlap->y + noverlap->height >= insert->y + insert->height)
                                { yoverlap = insert->height;}  
                                //--------------------noverlap
                                        //----------------------- insert
                                else if(noverlap->y <= insert->y &&  
                                        noverlap->y + noverlap->height >= insert->y)
                                {yoverlap = noverlap->y + noverlap->height - insert->y; }
                                else{yoverlap = 0;}

				overlap = xoverlap*yoverlap;
				
				//if we will encounter this node again, append it to the noverlap list to ignore
				noverlapx = (noverlap->x + noverlap->width) / GRID_GRAIN;
				noverlapx = (noverlap>RowWidth/GRID_GRAIN)?RowWidth+1:noverlapx;
				noverlapy = (noverlap->y + noverlap->height)/AvgRowHeight;
				while(GridHdr[noverlapy]->coordinate > y){noverlapy--;}
			        while(GridHdr[noverlapy]->coordinate <= y){noverlapy++;}
				if(noverlapx != gindexx || noverlapy != gindexy)
				{
					noverlap->south = noverlap_listhead;
					noverlap_listhead = noverlap;
				}

				if(insert_flag==0){ gridOverlap -= overlap*overlap; } //remove
				else{ gridOverLap += overlap*overlap;} //insert
				
				noverlap = noverlap->e_next;
			}//end while
			//clear noverlap list
			for(noverlap = noverlap_listhead; noverlap!=NULL; noverlap = noverlap_listhed)
			{ 	
				noverlap_listhead = noverlap->south;
				noverlap->north = NULL;
				noverlap->south = NULL;
			}
			//remove/add insert to list of nodes in this grid square
			if(insert_flag == 0) //remove
			{
				if(insert->e_next!=NULL){insert->e_next->e_prev = insert->e_prev;}
				if(insert->e_prev!=NULL){insert->e_prev->e_next = insert->e_next;}
				if(OverlapGrid[gy][gx]==insert){OverlapGrid[gy][gx]=insert->e_next;}
				insert->e_next = NULL;
				insert->e_prev = NULL;
				
			}
			else //insert node into grid square
			{	
				insert->e_next = OverlapGrid[gx][gy];
				insert->e_prev = NULL;
				OverlapGrid[gx][gy] = insert;
			}
		}
		
	}
	//update extra RowWidth && cleanup
	if(insert_flag
	if(insert->x < 0){ extraRowWidth -= insert->x*insert->height; }
	else if(insert->x + insert->width > RowWidth){ extraRowWidth += (insert->x + insert->width - RowWidth)*insert->height;}
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

	InsertOverlapNode(move, x, y);
}

			