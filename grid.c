struct grid_hdr** GridHdr;
struct node *** Grid, GridCpy;
struct node** EmptyNodeList, EmptyNodeListCpy;
int NumRows;
int RowWidth;

//variables unique to this module
static char gridLock;
int emptyNodeID, emptyNodeIDCpy;
int eNodeListSize;
int emptyNodeCnt, emptyNodeCntCpy;

void InitGrid()
{`
	gridLock = -1;
}
void FillGrid()
{
	struct node *n, *blocking;
	int i, err;
	gridLock = 0;
	//first make entire grid empty
	n = CreateEmptyNode(0,0,GridHdr[NumRows]->coordinate, RowWidth); //create and empty Node
	n->north = n->south = n->east = n->west = NULL;
	update_grid(n);
	
	for(i=Modules - PadOffset; i>=0; i--)
	{
		n = N_Arr[i];
		if(n==NULL){ continue;}
		while((blocking=InsertNode(n, n->x, n->y, &err))!=NULL)
		{
			if(err !=0){n->x--;}
			MoveLocal(blocking, move);
		}
	}
	
	AcceptMove();//copy to working copy
}
void InitEmptyNodeList(int size)
{
	eNodeListSize = size;
	do{ EmptyNodeList = (struct node**) malloc(eNodeListSize * sizeof(struct node*)); }while(EmptyNodeList==NULL);
	do{ EmptyNodeListCpy = (struct node**) malloc(eNodeListSize * sizeof(struct node*)); }while(EmptyNodeList==NULL);
	for(i=0; i<= size; i++)
	{ 
		EmptyNodeList[i] = NULL; 
	}
	emptyNodeID = 0;
	emptyNodeCnt = 0;
}
int getNewEmptyNodeID()
{
	while(EmptyNodeList[emptyNodeID]!=NULL){emptyNodeID++; }
	emptyNodeCnt = (emptyNodeID>emptyNodeCnt)?emptyNodeID:emptyNodeCnt;
	assert(eNodeListSize>EmptyNodeCnt);
	return emptyNodeID;
}

struct node* CreateEmptyNode(int x, int y, int height, int width)
{
	struct node* n, *move;

	do{n = (struct node*) malloc(sizeof(node));} while(n==NULL); //create node

	n->type  = 'e'; //empty
	n->index = getNewEmptyNodeID(); //not used for now
	n->locked = 0;
	n->cost = 0;
	n->birth = NULL;
	n->outhead = NULL;
	n->orientation = OR_N;

	n->x = x;
	n->y = y;
	n->width = width;
	n->height = height;
	
	EmptyNodeList[n->index] = n;
	return n;
}

void listremove_empty_node(struct node* remove)
{
	int index;
	assert(remove!=NULL);
	emptyNodeID = remove->index;
	EmptyNodeList[remove->index] = NULL;
	free(remove);
}
/***************************************************************
* This function will insert a node in an empty spot
* If another node (non empty) node is present in that spot, it will return a pointer to
* the node in the way
* Upon success, a NULL pointer is returned and err =0
****************************************************************/
struct node * InsertNode(struct node* insert, int x, int y, int* err)
{
	struct node* replace;
	struct node *north, *south, *east, *west;
	int gindexy, gindexy_original, gindexy_stop;

	//get to starting row
	gindexy = y/AvgRowHeight;
	while(GridHdr[gindexy]->coordinate > y){gindexy--;}
	while(GridHdr[gindexy]->coordinate <= y){gindexy++;}  
	gindexy_original = gindexy;
	y= GridHdr[gindexy]->coordinate; //allign y to start of row
	
	//test if space is free
	
	replace = find(x, GridHdr[gindexy]->coordinate);
	do
	{
		assert(replace!=NULL);
		if(replace ->type != 'e'){ return replace;}
		if(replace->east==NULL)
		{
			if(replace->x + replace->width < x + insert->width) //boundry reached
			{
				*err = -1; //error occurred
				return NULL;
			}
		}
		else if(replace->east->x < x + insert->width) //east is non empty node that overlaps with insert
		{	assert(replace->east->type != 'e');
			return replace->east;
		}
	 	//advance ot next node
		replace = find(replace->x, replace->y+replace->height);
                if(replace == NULL){ break; }
                while(GridHdr[gindexy+1]->coordinate < replace->y){ gindexy++; }	
	}while(GridHdr[gindexy]->coordinate <= y + index->height);
	gindexy_stop = gindexy;
	
	/*********************************************************************
	* If we made it this far, we passed the space-empty test and can start inserting the node
	*********************************************************************/
	
	//divide north/south boundries of place we are inserting the node
        for(i=0; i<2; i++)
	{
		gindexy = (i==0)?gindexy_original:gindexy_stop-1; //north boundry or south boundry. based on iteration	
        	replace = find(x, GridHdr[gindexy]->coordinate);
	        if((i==0 && replace->y < y) || //north boundry divide
			(i==1 && replace->y + replace->height  > y + insert->height)) //south boundry divide
        	{
			//insert will be north most
			east = find(replace->x + replace->width, y+insert->height); //replace->east
			west = find(replace->x - 1, y + insert->height - 1); //new_empty_node->west; new_empty_node will be called north
	                north = CreateEmptyNode(replace->x, replace->y, replace->width, y + insert->height - replace->y); //replace->north
			
			gridLock++;
			north->north = replace->north;
			north->east = replace->east;
			north->south = replace;
			north->west = west;

			replace->height = replace->height - north->height;//shrink
			replace->north = north;
			replace->east = east;
		
			update_grid(east, east);
			update_all_boundries(east);
			//update_grid(replace); //dont need since we are just shrinking replace and updated new+empty_nodes ptrs already
			//update_all_boundries(replace);
			gridLock--;
	        }
	}
	
	//lets overlay insert into the grid
	//have insert point to other nodes, but Grid has no clue he is there
	insert->x = x;
	insert->y = y;
	insert->north = find(x + insert->width -1, y-1); 
	insert->east = find(x+insert->width, y);
	insert->south = find(x, y + insert->height);
	insert->west = find(x-1, y+insert->height-1);
	//begin expansion node, which will maintain grid structure as we make way for insert
	expand = create_expansion_node(insert);
	gindexy=gindex_original;
	replace = find(x, GridHdr[gindexy]->coordinate);
	while(GridHdr[gindexy]->coordinate <= y + index->height);
	{
                assert(replace!=NULL);
		if(replace->x == x && replace->width == insert->width) //same x and width
		{
			gridLock++;
			expand->height += replace->height;
			expand->south = replace->south;
			expand->west = replace->west;
			update_grid(expand, expand);
			update_all_boundries(expand);
			listremove_empty_node(replace);
			gridLock--;
		}
		else if(replace->x == x) //same start, but empty node extends further
		{
			//shrink replace width from west to east
			south = find(x + insert->width, replace->y + replace->height); //replace south
			gridLock++;
			//expand expand
			expand->height += replace->height;		
			expand->south = replace->south;
			expand->west = replace->west;
			
			replace->x = x + insert->width;
			replace->width = replace->width - insert->width;
			replace->south = south;
			replace->west = expand;
			
			update_grid(replace, replace);
                        update_all_boundries(replace);
			gridLock--;
		}
		else if(replace->x + replace->width == x + insert->width)	//share same west boundry
		{
			north = find(x -1, replace->y-1); //replace north
			south = find(x, replace->y+replace->height); //expand->south

			gridLock++;
			expand->height += replace->height;
                        expand->south = south;
                        expand->west = replace;
                        
			replace>width = x - replace->x; //shrink
			replace->north = north;
			replace->east = expand;
  
                        update_grid(replace, replace);
                        update_all_boundries(replace);
			
			gridLock--;
		}
		else //free space to the east and west of insert
		{
			//subdivide replace
			new_node = CreateEmptyNode(x+insert->width, y, replace->x + replace->width - x - insert->width, replace->height);
			InsertNode(new_node, new_node->x, new_node->y, &i); //easiest way to insert the node
			continue; //with replace node subdivided, we can insert as in the above else_if case
		}
		
		//move to next node	
		replace = find(replace->x, replace->y+replace->height);
		if(replace == NULL){ break; }
		while(GridHdr[gindexy+1]->coordinate < replace->y){ gindexy++; }
	}
	
	//replace insert with expand
	assert(expand->height == insert->height); //other expand parameters were never changed
	gridLock++;
	assert(insert->north == expand->north);  
	insert->east = expand->east //can be different in the case a new node was inserted
	assert(insert->south == expand->south); 
	assert(insert->west == expand->west); //same; new nodes only inserted to the east

	update_grid(insert, insert)
	update_all_boundries(insert);
	gridLock--;
	*err = 0;
	return NULL;

}

/*******************************************************
* Only empty nodes are freed from memory
* Other nodes are inserted on the Remove Stack to be reinserted into the grid
*******************************************************/
void remove_node(struct node* remove)
{
	int i, j;
	struct node* nearby, east, west;

	assert(remove->type != 'f' && remove->type != 'F');
	//combine sorrrounding empty nodes, unlink pointers to remove node (recursive)
	filler = create_filler_node(remove);
	combine_ew(remove->east, remove->west, filler);
	assert(filler->height == 0);
	//theres a chance the upper replacement node is linked to filler->north
	if(filler->south->north == filler){filler->south->north = filler->north;}
	free(filler);
	
	//unlink node from grid
	remove->north = NULL;
	remove->east = NULL;
	remove->west = NULL;
	remove->x = INT_MAX;
	remove->y = INT_MAX;
	
	//next store in removed node stack
	if(remove->type == 'e'){ listremove_empty_node(remove); }
	else
	{
		remove->south = RemoveStack;
		RemoveStack = remove;
	}
}

/***************************************************************************************** 
 * This recursive method must take advantage of the fact that east comes from the north, and west comes from the south
 * We will recursively move east down southward until it extends it is more south than the west node
 * Then we will combine the nodes
 * We return the west node that is in line with the latitude we are operating at
 **********************************************************************************************/
struct node* combine_ew(struct node* east, struct node* west, struct node* filler)
{
	struct node *retnode, *tnorth, *tsouth, *teast, *twest; //return node, and temporary direction nodes
	int iheight;

	assert((east->y + east->height) <= (west->y + west->height)); //should not go to far south
	if((east->y + east->height) != (west->y + west->height))
	{  
		west = combine_ew(east->south, west, filler); 
	} 
	assert(west!=NULL);
	
	if(east->type=='e' && west->type=='e') //empty
	{
		assert(east->y +east->height == west->y + west->height); //should have same terminating line
		if(east->y == west->y)
		{
			retnode = west->north;
			twest = west->north; //find(west->x-1, west->y-1); //filler->west
			gridLock++;
			//expand west eastward
			west->width = west->width + filler->width + east->width; //extend
			west->north = east->north; //fix pointers for west node
			west->east = east->east; 	
			//remove east
			listremove_empty_node(east); 
			//shrink filler northward
			filler->height -= west->height;
			filler->south = west;
			filler->west = twest;

			update_grid_ptr(west, west); //fix grid ptrs
			update_all_boundries(west);// fix corner stitching TO west
			gridLock--;
		
			return retnode; //move up one node
		}
		else if(east->y < west->y)//east higher than west
		{
			retnode = west->north;
			teast = find(east->x + east->width, west->y); //west->east
			gridLock++;
			//shrink east northward
			east->height = west->y - east->y; //shrink
			east->south = west;
			east->west = filler;		
			//expand west eastward
			west->width = west->width + filler->width + east->width; //expand
			west->north = east; //fix pointers for west node
			west->east = teast;  
			//shrink filler northward			
			filler->height -= west->height;
			filler->south = west;
			filler->west = west->north;
			
			update_grid_ptr(west, west);  //fix grid ptrs
			update_all_boundries(west);// fix corner stitching TO west
			gridLock--;
			return combine_ew(east, retnode, filler); //need to perform operation on east again
		}
		else //if(east->y > west->y) //west higher than east
		{ 
			twest = find(west->x-1, east->y-1); //west->west
			gridLock++;
			//expand east westward
			east->x = west->x; //shift
			east->width = west->width + filler->width + east->width; //expand
			east->south = west->south; //fix pointers from east node
			east->west = west->west;
			//shrink west northward
			west->height = east->y - west->y; //shrink
			west->south = east;
			west->west = twest;
			//shrink filler northward
			filler->height -= east->height;		
			filler->south = east;
			filler->west = west;
	
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			gridLock--;
			return west;
		}
	}
	else if(east->type == 'e') //west is actual node
	{
		if(east->y == west->y)
		{
			gridLock++;
			//expand east westward
			east->x = filler->x; //shift
			east->width += filler->width; //extend east
			east->west = west;
			east->south = filler->south; //find south (SW corner)
			//shrink filler northward
			filler -= east->height;
			filler->west = west->north;
			filler->south = east;
			
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			gridLock--;
			return west->north; //move up one node
		}
		else if(east->y < west->y)//east higher than west
		{
			//create new empty node to fill in node space
			assert(east->height + east->y <= west->height+west->y);
			/*iheight = (east->y + east->height < west->y + west->height) ?
				east->y + east->height - west->y :
				west->height;*/
			tsouth = CreateEmptyNode(filler->x, west->y, filler->width + east->width, east->y + east->height - west->y); //filler->south
			teast = find(east->x+east->width, west->y); //tsouth->east
			gridLock++;
			//fix new_node (tsouth) pointers
			tsouth->north = east;
			tsouth->east = teast;
			tsouth->south = filler->south;
			tsouth->west = west;		
			//shrink east northward
			east->height -= tsouth->height; 
			east->south = tsouth; 
			east->west = filler;
			//shrink filler
			filler->height -= tsouth->height;
			filler->south = tsouth;
			filler->west = west->north;
	
			update_grid(tsouth, tsouth);
			update_all_boundries(tsouth);
			gridLock--;
			return combine_ew(east, west->north, filler); //need to perform operation on east again
		}
		else// if(east->y > west->y) //west higher than east
		{
			
			gridLock++;
			//expand east westward
			east->x = filler->x; //shift 
			east->width = filler->width + east->width; //expand
			east->south = filler->south;
			east->west = west;
			//shrink filler northward
			filler->height -= east->height;
			filler->south = east;
			filler->west = west;

			update_grid(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			gridLock--;
			return west; 
		}


	}
	else if(west->type == 'e') //east is an actual node
	{
		if(east->y == west->y)
		{
			retnode = twest = west->north;
			gridLock++;
			//expand west eastward
			west->width = west->width + filler->width; //extend east
			west->north = filler;
			west->east = east;
			//shrink filler northward
			filler->height -= west->height;
			filler->south = west;
			filler->west = twest;
		
			update_grid(west, west); //fix grid ptrs
			update_all_boundries(west);
			gridLock--;
			return retnode;
		}
		else if(east->y < west->y) //east is higher than west
		{
			retnode = twest = west->north;
			gridLock++;
			//expand west
			west->width = west->width + filler->width;
			west->north = filler;
			west->east = east;
			//shrink filler
			filler->height -= west->height;
			filler->south = west;
			filler->west = twest;	
		
			update_grid(west, west); //fix grid ptrs
			update_all_boundries(west);
			gridLock--;
			return combine_ew(east, retnode, filler); //still need work on east, with new west
		}
		else //if (east->y > west->y) //west higher than east
		{
			iheight = west->y + west->height - east->y;
			//subdivide west; new node to be exrended eastward so that it is south of filler
			tsouth = CreateEmptyNode(west->x, east->y, west->width + filler->width, iheight); //filer->south
			twest = find(west->x -1, tsouth->y-1);
			gridLock++;
			//link in tsouth
			tsouth->north = filler;
			tsouth->east = east;
			tsouth->south = west->south; 
			tsouth->west = west->west;
			//shrink west northward
			west->height -= tsouth->height;
			west->south = tsouth;
			west->west = twest;
			//shrink filler northward
			filler->height -= tsouth->height;
			filler->south = tsouth;
			filler->west =  west;
		
			update_grid(tsouth, tsouth);
			update_all_boundries(tsouth);
			gridLock--;
			return west;
		}

	}
	else //actual nodes on both sides
	{
		if(east->y == west->y)
		{
			iheight = (east->height < west->height)? east->height : west->height ;
			tsouth = CreateEmptyNode(filler->x, west->y, filler->width, iheight); //filler->south
			gridLock++;
			//link in tsouth
                        tsouth->north = filler;
                        tsouth->east = east;
                        tsouth->south = filler->south;
                        tsouth->west = west;
                        //shrink filler northward
                        filler->height -= tsouth->height;
                        filler->south = tsouth;
                        filler->west = west->north;  

                        update_grid(tsouth, tsouth);
                        update_all_boundries(tsouth);
			gridLock--;
			return west->north; //move up one node
		}
		else if(east->y < west->y)//east is higher than west
		{
			iheight = (east->y + east->height < west->y + west->height) ?
				east->y + east->height - west->y :
				west->height;
			retnode = west->north;
			tsouth = CreateEmptyNode(filler->x, west->y, filler->width, iheight ); //filler->south
	        	
			gridLock++;
                        //link in tsouth
                        tsouth->north = filler;
                        tsouth->east = east;
                        tsouth->south = filler->south;
                        tsouth->west = filler->west;
                        //shrink filler northward
                        filler->height -= tsouth->height;
                        filler->south = tsouth;
                        filler->west = west->north;

                        update_grid(tsouth, tsouth);
                        update_all_boundries(tsouth);
			gridLock--;
			return combine_ew(east, retnode, filler); //still need work on east, with new west

		}
		else //if(east->y > west->y) //west is higher than east
		{
			iheight = (east->y + east->height < west->y + west->height) ? 
				east->height : 
				west->y + west->height - east->y ;
			tsouth = CreateEmptyNode(filler->x, east->y, filler->width, iheight ); //filler->south
			
			gridLock++;
                        //link in tsouth
                        tsouth->north = filler;
                        tsouth->east = east;
                        tsouth->south = filler->south;
                        tsouth->west = filler->west;
                        //shrink filler northward
                        filler->height -= tsouth->height;
                        filler->south = tsouth;
                        filler->west = west;

                        update_grid(tsouth, tsouth);
                        update_all_boundries(tsouth);
			return west; 
		}
	}
	return NULL; //error - should not reach here
}
/*
int MoveNode()
{
	int ecum_sum, esize_index, fail_count;
	int node_index;	
	struct node *move, *moveto;

	esize_index = LogTwo(move->width);
	//find cumlative sum
	for(ecum_size=0; esize_index < eNodeListSize; esize_index++){ ecum_size+=EmptyNodeCnt[esize_index];}
	
	//pick random empty node to insert move in
	//nodes are unweighted - each has a same chance of being chosen despite area
	//this allows for smaller fragmentation
	moveto = NULL;
	for(fail_count = 0; moveto!=NULL && fail_count<ecum_size*10; fail_count++) 
	{
		node_index = (rand()%ecum_sum);
		//move to size list
		for(esize_index = eNodeListSize-1; 
			node_index - EmptyNodeCnt[esize_index] > 0; 
			esize_index--)
		{node_index-=EmptyNodeCnt[esize_index];}
		//go to node number in list
		for(moveto=EmptyNodeList[esize_index]; 
			node_index > 0; 
			node_index--)
		{
			assert(moveto!=NULL);
			moveto = moveto->e_next;
		}
		assert(moveto!=NULL);
		if(moveto->width < move->width)
		{
			moveto = NULL;
			fail_count++;
		}	
	}
	if(moveto == NULL){ return -1;} //high chance every node was tried - sorry
	
	
	
	
}*/

int MoveRandom(struct node* move)
{
	struct node* blocking;
	int x,y,err;
	//remove
	remove_node(move);
	//find random spot
	x = rand()%(RowWidth-move->width);
	do
	{
		y = rand()%NumRows;	
	}while(GridHdr[y]->coordinate + move->height > GridHdr[NumRows]->coordinate);
	y = GridHdr[y]->coordinate;

	while((blocking=InsertNode(move, x, y, err))!=NULL)
	{
		MoveLocal(blocking, move); //move any nodes in the way
        }
                        

}

int MoveLocal(struct node* move, struct node* keepout)
{
	struct node* blocking;
	int err;
	int direction;
	
	direction = rand()%4;//NESW
	
	remove_node(move);
	while(1)
		switch(direction)
		{
			case '0': //move north
				while((blocking=InsertNode(move, move->x, keepout->y - move->height, err))!=NULL)
				{
					MoveLocal(blocking, move); 
				}
			break;
			case '1': //move east
			
				while((blocking=InsertNode(move, keepout->x+keepout->width, move->y, err))!=NULL)
				{
				MoveLocal(blocking, move); 
				}
			break;
			case '2': //move south
				while((blocking=InsertNode(move, move->x, keepout->y + keepout->height, err))!=NULL)
				{	
					MoveLocal(blocking, move); 
				}
			break;
			case '3': //move west
				while((blocking=InsertNode(move, keepout->x-move->width, move->y, err))!=NULL)
				{
					MoveLocal(blocking, move); 
				}
			break;
			default:
				err = -1;
				break;
		}
		if(err>=0){ break;} //exit from while loop
		//out of bound or default case
		direction = rand()%4;
	}
	return 0;
			
}
void AcceptMove()
{
	struct node *n, *ncpy;
	int i, j;
	int ecnt;
	//copy modules
	for(i=0; i<Modules; i++)
	{
		n = N_Arr[i];
		ncpy = N_ArrCpy[i];
		if(n==NULL){continue;}
		assert(ncpy != NULL);
		CopyParallelNode(n, ncpy);
	}
	//copy empty nodes
	ecnt = (emptyNodeCnt>emptyNodeCntCpy)?emptyNodeCnt:emptyNodeCntCpy; //iterate through bogger of 2
	for(i=0; i<ecnt; i++)
        {
		n = EmptyNodeList[i];
                ncpy = EmptyNodeListCpy[i];
               	if(n==NULL && ncpy!=NULL)
		{listremove_empty_nodecpy(ncpy); }
		else if(n!=NULL && ncpy==NULL)
		{
			do{ncpy = malloc(sizeof(node))}while(ncpy==NULL);
			CopyParallelNode(n, ncpy);
			EmptyNodeListCpy[ncpy->index] = ncpy;	
		}
		else if(n!=NULL && ncpy!=NULL)
		{CopyParallelNode(n, ncpy);}
        }
	emptyNodeCntCpy = emptyNodeCnt;
	emptyNodeIDCpy = emptyNodeID;
	//copy grid ptrs
	for(i=0; i<NumRows; i++)
	{
		for(j=0; j<RowWidth/GRID_GRAIN; j++)
		{
			n = Grid[i][j];
			if(n->type == 'e'){ GridCpy[i][j] = EmptyNodeListCpy[n->index];}
			else if(n->type == 'a'){ GridCpy[i][j] = N_ArrCpy[n->index];}
			else if(n->type == 'p'){ GridCpy[i][j] = N_ArrCpy[n->index+PadOffest];}
		}
	}
}
	
void RejectMove() //same as accept move except swapped xx<->xxCpy
{
	struct node *n, *ncpy;
	int i, j;
	int ecnt;
	//copy modules
	for(i=0; i<Modules; i++)
	{
		n = N_Arr[i];
		ncpy = N_ArrCpy[i];
		if(ncpy==NULL){continue;}
		assert(n != NULL);
		RestoreParallelNode(ncpy, n);
	}
	//copy empty nodes
	ecnt = (emptyNodeCnt>emptyNodeCntCpy)?emptyNodeCnt:emptyNodeCntCpy; //iterate through bogger of 2
	for(i=0; i<ecnt; i++)
        {
		n = EmptyNodeList[i];
                ncpy = EmptyNodeListCpy[i];
               	if(ncpy==NULL && n!=NULL)
		{listremove_empty_nodecpy(n); }
		else if(ncpy!=NULL && n==NULL)
		{
			do{n = malloc(sizeof(node))}while(n==NULL);
			RestoreParallelNode(ncpy, n);
			EmptyNodeList[n->index] = n;	
		}
		else if(ncpy!=NULL && n!=NULL)
		{RestoreParallelNode(ncpy, n);}
        }
	emptyNodeCnt = emptyNodeCntCpy;
	emptyNodeID = emptyNodeIDCpy;
	//copy grid ptrs
	for(i=0; i<NumRows; i++)
	{
		for(j=0; j<RowWidth/GRID_GRAIN; j++)
		{
			ncpy = GridCpy[i][j];
			if(ncpy->type == 'e'){ Grid[i][j] = EmptyNodeList[ncpy->index];}
			else if(ncpy->type == 'a'){ Grid[i][j] = N_Arr[ncpy->index];}
			else if(ncpy->type == 'p'){ Grid[i][j] = N_Arr[ncpy->index+PadOffest];}
		}
	}
	
}

struct node* create_filler_node(struct node* copy)
{
	struct node* filler;
	
	do{filler = (struct node*) malloc(sizeof(node));}while(filler==NULL);
	
	CopyNode(copy, filler);
	filler->type = 'f';
	//replace filler in grid
	update_grid(filler, filler);
	update_all_boudries(filler); //point all nodes to filler
	return filler;
}
struct node* create_expansion_node(struct node* copy)
{
	struct node* expand;

        do{expand = (struct node*) malloc(sizeof(node));}while(expand==NULL);

	CopyNode(copy, expand);
	//shrink expand
	expand->south = find(copy->x, copy->y);
	expand->west = find(copy->x-1, copy->y-1);
        expand->type = 'F';
	expand->height = 0;
        return expand;

}

struct node* find(int x, int y)
{
	
	int i,j;
	struct node* start, position;
	assert(gridLock==0);
	i = y/AvgRowHeight;
        j = x/GRID_GRAIN;
        while(GridHdr[i]->coordinate > y){i--;}; //move to proper row in case of poor avg height
	while(GridHdr[i+1]->coordinate < y){i++;}
	
	position = Grid[i][j];	

	while(1)
	{
		start = position;
		if(position->y > y){ postion = positon->north;}
		else if(position->y + position+height < y) { position = position->south; }
		if(position->x + position->width < x){ position = position->east;}
		else if(position->x > x){position = position->west;}
		
		if(start == position){break;}
	}
	return postion;
}

void update_grid(struct node* update, struct node* replace)
{
	int i,j;

	assert(update!=NULL && (replace==NULL || replace == update));
	//first update grid ptrs
	i = (update->y)/AvgRowHeight;
	j = (update->x)/GRID_GRAIN;
	while(GridHdr[i]->coordinate > update->y){i--;}; //move to proper row in case of poor avg height

	do
	{
		do
		{
			if
				(	update->y <=  GridHdr[i]->coordinate &&
					(update->y + update->height) > GridHdr[i]->coordinate &&
					update->x <= j*GRID_GRAIN &&
					(update->x + update->width) > j*GRID_GRAIN 
				)
				{
					Grid[i][j]=replace;
				}

			j++;
		}while((j*GRID_GRAIN) < (remove->x + remove->width));
		j = (remove->x)/GRID_GRAIN;
		i++;
	}while(GridHdr[i]->coordinate < (remove->y + remove->height) );

}
void update_all_boundries(struct node* n)
{
	update_boundry(n, OR_N);
	update_boundry(n, OR_S);
	update_boundry(n, OR_E);
	update_boundry(n, OR_W);
}

void update_boundry(struct node* center, char orient)
{
	struct node* border;
	switch(orient)
	{
		case OR_N: //fix south pointer along center's north border
			border = center->north;
			if(border==NULL){return;}
			while(border->x > center->x)
			{
				assert(border->y + border->height == center->y); //should always be centers north border
				border->south = center;
				border = border->west;
			}
			break;
		case OR_S:
			border = center->south;
			if(border==NULL){return;}
			while(border->x < center->x + center->width)
			{
				assert(center->y + center->height == border->y); //should always be centers south border
				border->north = center;
				border = border->east;
			}
			break;
		case OR_E:
			border = center->east;
			if(border==NULL){return;}
			while(border->y < center->y + center->height)
			{
				assert(center->x + center->width == border->x); //should always be centers east border
				border->west = center;
				border = border->south;	
			}
			break;
		case OR_W:
			border = center->west;
			if(border==NULL){return;}
			while(border->y > center->y) //moving north
			{
				assert(border->x + border->width == center->x); //should always be centers west border
				border->east = center;
				border = border->north;
			}
			break;
		default:
			assert(false);
			break;
	}
}
