#define TEST_DIR "test/"
#define SF_L 5+7+6+1 //test/ibmXX/ibmXX. and NULL terminator length
#define S2US 1000000 //seconds to microseconds
//#define SEED_RANDOM
#define INIT_X 0
#define INIT_Y 0
#define INIT_TEMPERATURE 300000
#define FINAL_TEMPERATURE 0.1
#define INIT_ALPHA 0.8
#define RAND_PRECISION 100000
#define GRID_GRAIN 64
#define OVERLAP_WEIGHT 5
#define ROW_WIDTH_WEIGHT 1

//#define OUTPUT_CSV
#define TIME_REPORT
#define DEBUG
#ifdef DEBUG
//VERBOSE DEBUGGING
// #define DEBUG_VB_ALGO //main module
// #define DEBUG_VB_GEN //generate module
// #define DEBUG_VB_OLG  //overlap grid module
#endif

