/****************************************************
* This function will recalculate the cost of node n and modify the costs of surrounding nodes
* It will return the delta cost for node n
***************************************************/
int cost(struct node* n, int org_x, int org_y)
{
	int s_in=0, d_in=0; //same partition in, different partition in
	int s_flag, d_flag; //same flag/different flag for chyperedge children
	int cost, cost_mod, cost_original; //cost and modified cost
	struct node* bucket;
	struct edge *e;
	struct hyperedge* h;

	if(n==NULL) return 0;
	cost_original = n->cost;
	cost = 0;
	//inputs
	e = n->birth;
	while(e!=NULL) //iterate through node inputs to determine where parents are
	{
		//calculate difference
		cost_mod =0;
		//x
		cost_mod += abs(e->parent->out->x - org_x) - abs(e->parent->out->x - n->x); 
		cost += abs(e->parent->out->x - n->x);
		//y
		cost_mod += abs(e->parent->out->y - org_y) - abs(e->parent->out->y - n->y);
                cost += abs(e->parent->out->y - n->y)

		e->parent->out->cost -= cost_mod;
		e = e->foster; //move to next input
	} 

	//outputs
	h = n->out_head;
	while(h != NULL) //through each  output hyperedge of node
	{
		e = h->out_head;
		while(e!=NULL) //through each edge of hyperedge
		{
			//calculate difference
                	cost_mod =0;
        	        //x
	                cost_mod += abs(e->in->x - org_x) - abs(e->in->x - n->x); 
               		cost += abs(e->in->x - n->x);
        	        //y
	               	cost_mod += abs(e->parent->out->y - org_y) - abs(e->parent->out->y - n->y);
        	        cost += abs(e->parent->out->y - n->y)
	
	                e->in->cost -= cost_mod;
			e = e->next;
		}

		h = h->next;
	}
	n->cost = cost;

`	return cost_original - cost;
}


void CopyNode(struct node* original, struct node* copy)
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

