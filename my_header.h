#define STAT_BENCHMARK 0
#define STAT_IMPROVEMENT STAT_BENCHMARK+1
#define STAT_FINAL STAT_IMPROVEMENT+1
#define STAT_TIME STAT_FINAL+1
#define STAT_OVERALL STAT_TIME+1
#define STAT_MAX STAT_OVERALL+1

//oreintation characters
#define OR_N 'N'
#define OR_S 'S'
#define OR_E 'E'
#define OR_W 'W'
#define OR_FN 'n'
#define OR_FS 's'
#define OR_FE 'e'
#define OR_FW 'w'


struct node
{
	char type; //padi/terminal (p) or cell (a)  or empty (e)
	int index; //pad or cell #
	int locked; //counter to LOCK_THRESH
	int cost;
	struct edge* birth; //first edge input
	struct hyperedge* out_head; //hyperedge output
	char dir; //direction (pads only- other nodes have more than one pin)
	char orientation;

	int x; //x coordinate
	int y; //y coordinate
	int width;
	int height;

	//corner stitching
	struct node* north;
	struct node* south;
	struct node* east;
	struct node* west;
	
};
struct edge
{
	struct node* in; //the node this hyperedge connects to as an input
	struct hyperedge* parent; //the parent edge of this hyperedge
	struct edge* next; //next out node this net connects to
	struct edge* foster; //next input hyperedge this out node is connected to
};
struct hyperedge
{
	struct node* out; //the node this edge connects to (output signal)
	struct edge* out_head; //head of a linked list of hyperedges this edge expands to
	struct hyperedge* next; //next output the in node is connected to 
};

