uint32_t emptynodeid;

void InitEmptyNodeList(int listsize)
{
	emptynodeid = 0;

}

void CreateEmptyNode(int x, int y, int height, int width)
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

	while((move = InsertNode(n))!=NULL) //until it is successfully inserted
	{
		MoveNode(move);
	}


}

void remove_node(struct node* remove)
{
	int i, j;
	struct node* nearby, east, west;

	//first remove node - make Grid pointers null
	update_grid_ptr(remove, NULL);

	//combine sorrrounding empty nodes, unlink pointers to remove node (recursive)
	combine_ew(remove->east, remove->west, remove);

	//unlink node from grid
	remove->north = NULL;
	remove->east = NULL;
	remove->west = NULL;
	remove->x = INT_MAX;
	remove->y = INT_MAX
		//next store in removed node stack
		remove->south = RemoveStack;
	RemoveStack = remove;

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
			west->width = west->width + void_area->width + east->width; //extend
			west->north = east->north; //fix pointers for west node
			west->east = east->east; 	
			remove_empty_node(east); //delete
			update_grid_ptr(west, west); //fix grid ptrs
			update_all_boundries(west);// fix corner stitching TO west
			return retnode; //move up one node
		}
		else if(east->y < west->y)
		{
			east->height = west->y - east->y; //shrink
			west->width = west->width + void_area->width + east->width; //expand
			west->north = east->north; //fix pointers for west node
			west->east = east->east;  
			update_grid_ptr(west, west);  //fix grid ptrs
			update_all_boundries(west);// fix corner stitching TO west
			return west->north; //move up one node
		}
		else if(east->y > west->y)
		{
			west->height = east->y - west->y; //shrink
			east->x = west->x; //shift (expand part 1)
			east->width = west->width + void_area->width + east->width; //expand
			east->south = west->south; //fix pointers from east node
			east->west = west->west;
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			return west;
		}
	}
	else if(east->type == 'e') //west is actual node
	{
		if(east->y == west->y)
		{
			east->x = void_area->x; //shift
			east->width = void_area->width + east->width; //extend east
			east->west = west;
			east->south = find_node(east->y + east->height, east->x); //find south (SW corner)
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			return west->north; //move up one node
		}
		else if(east->y > west->y)
		{
			east->x = void_area->x; //shift 
			east->width = void_area->width + east->width; //expand
			east->south = find_node(east->y + east-> height, east->x); 
			east->west = west;
			update_grid_ptr(east, east); //fix grid ptrs
			update_all_boundries(east);// fix corner stitching TO east
			return west; 
		}


	}
	else if(west->type == 'e') //east is an actual node
	{
		if(east->y == west->y)
		{
			west->width = west->width + void_area->width; //extend east
			//north pointer may not be created yet - will wait for higher recursive call to do this
			west->east = east;
			update_grid_ptr(west, west); //fix grid ptrs
			//north - same problem as above comment
			update_boundry(west, OR_S);
			update_boundry(west, OR_E);
			//west - boundry remains the same
			return west->north; //un-updated (stale) north pointer
		}
		else if (east->y > west->y)
		{
			iheight = west->y + west->height - east->y;
			west->height = east->y - west->y; //shrink west verically
			CreateEmptyNode(west->x, east->y, west->width + void_area->width, iheight); //fill in extend node
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
		else if(east->y > west->y)
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
