#ifndef CFG
#define CFG

#define NUM_STATES 10   // table length
#define OPTIONS 5       // table width

typedef enum {
    EMPTY,
    SHIFT,
    REDUCE,
    ACCEPT,
} Action;

// a cfg rule which can 
typedef struct {
    Action states[NUM_STATES][OPTIONS];// for 
} StateTable;

#endif