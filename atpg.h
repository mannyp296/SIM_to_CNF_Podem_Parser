/**********************************************************************/
/*           automatic test pattern generation                        */
/*           global data structures header file                       */
/*                                                                    */
/*           Author: Tony Hi Keung Ma                                 */
/*           last update : 09/28/1987                                 */
/**********************************************************************/


typedef struct WIRE *wptr;
typedef struct NODE *nptr;

struct WIRE {
    char *name;            /* asciz name of wire */
    nptr *inode;           /* nodes driving this wire */
    nptr *onode;           /* nodes driven by this wire */
    short nin;             /* number of input nodes */
    short nout;            /* number of output nodes */
    int value;             /* logic value of the wire */
    int flag;              /* flag word */
    wptr pnext;            /* general usage link */
    wptr hnext;            /* hash bucket link */
    /* data structure for podem test pattern generation */
    int level;             /* level of the wire */
    short *pi_reach;       /* array of no. of paths reachable from each pi */
    /* data structure for parallel fault simulator */
    int wire_value1;       /* fault-free value */
    int wire_value2;       /* faulty value */
    int fault_flag;        /* flag to indicate fault-injected bit */
    int wlist_index;       /* index into the wlist array */
};

struct NODE {
    char *name;            /* asciz name of node */
    wptr *iwire;           /* wires driving this node */
    wptr *owire;           /* wires driven by this node */
    short nin;             /* number of input wires */
    short nout;            /* number of output wires */
    int type;              /* node type */
    int flag;              /* flag word */
    nptr pnext;            /* general usage link */
    nptr hnext;            /* hash bucket link */
};

#define HASHSIZE 3911

/* types of gate */
#define NOT       1
#define NAND      2
#define AND       3
#define INPUT     4
#define NOR       5
#define OR        6
#define OUTPUT    8
#define XOR      11
#define BUF      17
#define EQV	      0	/* XNOR gate */
#define SCVCC    20

/* possible value for wire flag */
#define SCHEDULED       1
#define ALL_ASSIGNED    2
/*#define INPUT         4*/
/*#define OUTPUT        8*/
#define MARKED         16
#define FAULT_INJECTED 32
#define FAULTY         64
#define CHANGED       128
#define FICTITIOUS    256
#define PSTATE       1024
