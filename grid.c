struct grid_hdr** GridHdr;
struct node *** Grid;
struct node* RemoveStack;
struct node** EmptyNodeList;
int* EmptyNodeCnt;
char GridLock;
uint32_t emptynodeid;

void InitEmptyNodeList(int size)
{
	size = LogTwo(size)+1;
	do{ EmptyNodeList = (struct node**) malloc(size * sizeof(struct node*)); }while(EmptyNodeList==NULL);
	do{ EmptyNodeCnt = (int *) malloc(size*sizeof(int)); } while(EmptyNodeCnt == NULL);
	for(i=0; i<= size; i++)
	{ 
		EmptyNodeList[i] = NULL; 
		EmptyNodeCnt[i] = 0;
	}

	emptynodeid = 0;
	
}

void listinsert_empty_node(struct node * insert)
{
	int index;
	index = LogTwo(insert->width);
	
	if(EmptyNodeList[index]==NULL)
	{
		insert->e_next = insert->e+prev = NULL;
		EmptyNodeList[index]=insert;
	}
	else
	{
		insert-> e_next = EmptyNodeList[index];
		insert-> e_prev = NULL;
		EmptyNodeList[index]->e_prev = insert;
		EmptyNodeList[index] = insert;
	}
	EmptyNodeCnt[index]++;	
}

struct node* CreateEmptyNode(int x, int y, int height, int width)
{
	struct node* n, *move;

	do{n = (struct node*) malloc(sizeof(node));} while(n==NULL); //create node

	n->type  = 'e'; //empty
	n->index = emptynodeid++; //not used for now
	n->locked = 0;
	n->cost = 0;
	n->birth = NULL;
	n->outhead = NULL;
	n->orientation = OR_N;

	n->x = x;
	n->y = y;
	n->width = width;
	n->height = height;
	
	listinsert_empty_node(n); //append empty node to list
	return n;
}

void listremove_empty_node(struct node* remove)
{
	int index;
	assert(remove!=NULL);
	index = LogTwo(remove->width);
	
	if(remove->e_next!=NULL){ remove->e_next->e_prev = remove->e_prev;}
	if(remove->e_prev!=NULL){remove->e_prev->e_next = remove->e_next;}
	else{ EmptyNodeList[index] = remove->e_next; } //e_prev==NULL means it is at top of stack already
	
	EmptyNodeCnt[index]--;
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
			
			GridLock++;
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
			GridLock--;
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
			GridLock++;
			expand->height += replace->height;
			expand->south = replace->south;
			expand->west = replace->west;
			update_grid(expand, expand);
			update_all_boundries(expand);
			listremove_empty_node(replace);
			GridLock--;
		}
		else if(replace->x == x) //same start, but empty node extends further
		{
			//shrink replace width from west to east
			south = find(x + insert->width, replace->y + replace->height); //replace south
			GridLock++;
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
			GridLock--;
		}
		else if(replace->x + replace->width == x + insert->width)	//share same west boundry
		{
			north = find(x -1, replace->y-1); //replace north
			south = find(x, replace->y+replace->height); //expand->south

			GridLock++;
			expand->height += replace->height;
                        expand->south = south;
                        expand->west = replace;
                        
			replace>width = x - replace->x; //shrink
			replace->north = north;
			replace->east = expand;
  
                        update_grid(replace, replace);
                        update_all_boundries(replace);
			
			GridLock--;
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
	GridLock++;
	assert(insert->north == expand->north);  
	insert->east = expand->east //can be different in the case a new node was inserted
	assert(insert->south == expand->south); 
	assert(insert->west == expand->west); //same; new nodes only inserted to the east

	update_grid(insert, insert)
	update_all_boundries(insert);
	GridLock--;
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
	combine_ew(remove->east, remove->west, remove);

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
struct node* combine_ew(struct node* east, struct node* west, struct node* void_area)
{
	struct node* retnode;
	int iheight;

	assert((east->y + east->height) <= (west->y + west->height)); //should not go to far south
	if((east->y + east->height) != (west->y + west->height))
	{  
		west = combine_ew(east->south, west, void_area); 
	} 
	assert(west!=NULL);
	if(east->type=='e' && west->type=='e') //empty
	{
		if(east->y == west->y)
		{
			assert(east->height == west->height);
			retnode = west->north;
			GridLock++;
			west->width = west->width + void_area->width + east->width; //extend
			west->north = east->north; //fix pointers for west node
			west->east = east->east; 	
			remove_empty_node(east); //delete
			update_grid_ptr(west, west); //fix grid ptrs
			update_all_boundries(west);// fix corner stitching TO west
			GridLock--;
			return retnode; //move up one node
		}
		else if(east->y < west->y)
		{
			GridLock++;
			east->height = west->y - east->y; //shrink
			west->width = west->width + void_area->width + east->width; //expand
			west->north = east->north; //fix pointers for west node
			west->east = east->east;  
			update_grid_ptr(west, west);  //fix grid ptrs
			update_all_boundries(west);// fix corner stitching TO west
			GridLock--;
			return west->north; //move up one node
		}
		else //if(east->y > west->y)
		{
			GridLock++;
			west->height = east->y - west->y; //shrink
			east->x = west->x; //shift (expand part 1)
			east->width = west->width + void_area->width + east->width; //expand
			east->south = west->south; //fix pointers from east node
			east->west = west->west;
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			GridLock--;
			return west;
		}
	}
	else if(east->type == 'e') //west is actual node
	{
		if(east->y == west->y)
		{
			GridLock++;
			east->x = void_area->x; //shift
			east->width = void_area->width + east->width; //extend east
			east->west = west;
			east->south = find_node(east->y + east->height, east->x); //find south (SW corner)
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			GridLock--;
			return west->north; //move up one node
		}
		else if(east->y < west->y)
		{
			GridLock++;
			iheight = (east->y + east->height < west->y + west->height) ?
				east->y + east->height - west->y :
				west->height;
			east->height = west->y - east->y; //shrink east
			CreateEmptyNode(void_area->x, west->y, void_area->width + east->width, iheight);
			GridLock--;
			return combine_ew(east, west->north, void_area); 
		}
		else// if(east->y > west->y)
		{
			GridLock++;
			east->x = void_area->x; //shift 
			east->width = void_area->width + east->width; //expand
			east->south = find_node(east->y + east-> height, east->x); 
			east->west = west;
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			GridLock--;
			return west; 
		}


	}
	else if(west->type == 'e') //east is an actual node
	{
		if(east->y == west->y)
		{
			GridLock++;
			west->width = west->width + void_area->width; //extend east
			//north pointer may not be created yet - will wait for higher recursive call to do this
			west->east = east;
			update_grid_ptr(west, west); //fix grid ptrs
			//north - same problem as above comment
			update_boundry(west, OR_S);
			update_boundry(west, OR_E);
			//west - boundry remains the same
			GridLock--;
			return west->north; //un-updated (stale) north pointer
		}
		else if(east->y < west->y)
		{
			GridLock++;
			retnode = west->north;
			west->width = west->width + void_area->width;
			//cannot update north - node may not be existant yet
			west->east = east;
			update_grid_ptr(west, west); //fix grid ptrs
			//north - same problem as above comment
			update_boundry(west, OR_S);
			update_boundry(west, OR_E);
			//west - boundry remains the same
			GridLock--;
			return combine_ew(east, retnode, void_area); //still need work on east, with new west
		}
		else //if (east->y > west->y)
		{
			iheight = west->y + west->height - east->y;
			GridLock++;
			west->height = east->y - west->y; //shrink west verically
			CreateEmptyNode(west->x, east->y, west->width + void_area->width, iheight); //fill in extend node
			GridLock--;
			return west;
		}

	}
	else //actual nodes on both sides
	{
		if(east->y == west->y)
		{
			iheight = (east->height < west->height)? east->height : west->height ;
			CreateEmptyNode(void_area->x, west->y, void_area->width, iheight );
			return west->north; //move up one node
		}
		else if(east->y < west->y)
		{
			iheight = (east->y + east->height < west->y + west->height) ?
				east->y + east->height - west->y :
				west->height;
			CreateEmptyNode(void_area->x, west->y, void_area->width, iheight );
			return combine_ew(east, west->north, void_area); //still need work on east, with new west

		}
		else //if(east->y > west->y)
		{
			iheight = (east->y + east->height < west->y + west->height) ? 
				east->height : 
				west->y + west->height - east->y ;
			CreateEmptyNode(void_area->x, east->y, void_area->width, iheight );
			return west; 
		}
	}
	return NULL;
}

struct node* create_filler_node(struct node* copy)
{
	struct node* filler;
	
	do{filler = (struct node*) malloc(sizeof(node));}while(filler==NULL);
	
	copy_node(copy, filler);
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

	copy_node(copy, expand);
	//shrink expand
	expand->south = find(copy->x, copy->y);
	expand->west = find(copy->x-1, copy->y-1);
        expand->type = 'F';
	expand->height = 0;
        return expand;

}

void copy_node(struct node* original, struct node* copy)
{	
	assert(original != NULL &&  copy != NULL);
	
        copy->type = original->type; 
        copy->index = original->index;
        copy-> locked =  original ->locked; 
        copy->cost = original->cost;
        copy->birth = original->birth;
        copy-> out_head original->outhead; 
	copy->e_next = original->e_next;
	copy-> e_prev = original->e_prev;
        copy->dir = original->dir; 
        copy->orientation = original->orientation;

        copy->x = original->x; //x coordinate
        copy->y = original->y;
        copy->width = original->width;
        copy->height = original->height;

        //corner stitching
        copy->north = original->north;
        copy->south = original->south;
        copy->east = original->east;
        copy->west = original->west;

}

struct node* find(int x, int y)
{
	
	int i,j;
	struct node* start, position;
	assert(GridLock==0);
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
	return;
}
