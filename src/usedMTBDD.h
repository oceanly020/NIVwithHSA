
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <setjmp.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
// #include "all_MTBDD.h"

/*=== Defined operators for apply calls ===*/
#define bddop_and       0
#define bddop_xor       1
#define bddop_or        2
#define bddop_nand      3
#define bddop_nor       4
#define bddop_imp       5
#define bddop_biimp     6
#define bddop_diff      7
#define bddop_less      8
#define bddop_invimp    9

/* Should *not* be used in bdd_apply calls !!! */
#define bddop_not      10
#define bddop_simplify 11
#define bddop_to_x     12
#define mtbddop_apply  13
#define mtbddop_addr   14
#define mtbddop_delr   15

/*=== User BDD types ===*/
typedef int BDD;

typedef struct s_bddPair {
  BDD *result;
  int last;
  int id;
  struct s_bddPair *next;
} bddPair;

typedef struct s_bddStat {
  long int produced;
  int nodenum;
  int maxnodenum;
  int freenodes;
  int minfreenodes;
  int varnum;
  int cachesize;
  int gbcnum;
} bddStat;

typedef struct s_bddGbcStat {
  int nodes;
  int freenodes;
  long time;
  long sumtime;
  int num;
} bddGbcStat;

typedef struct s_bddCacheStat {
  long unsigned int uniqueAccess;
  long unsigned int uniqueChain;
  long unsigned int uniqueHit;
  long unsigned int uniqueMiss;
  long unsigned int opHit;
  long unsigned int opMiss;
  long unsigned int swapCount;
} bddCacheStat;

#define bdd_relprod(a,b,var) bdd_appex((a),(b),bddop_and,(var))


  /* In file "kernel.c" */

typedef void (*bddinthandler)(int);
typedef void (*bddgbchandler)(int,bddGbcStat*);
typedef void (*bdd2inthandler)(int,int);
typedef int  (*bddsizehandler)(void);
typedef void (*bddfilehandler)(FILE *, int);
typedef void (*bddallsathandler)(char*, int);
   
extern bddinthandler  bdd_error_hook(bddinthandler);
extern bddgbchandler  bdd_gbc_hook(bddgbchandler);
extern bdd2inthandler bdd_resize_hook(bdd2inthandler);
extern bddinthandler  bdd_reorder_hook(bddinthandler);
extern bddfilehandler bdd_file_hook(bddfilehandler);
   
extern int      bdd_init(int, int, int);
extern void     bdd_done(void);
extern int      bdd_setvarnum(int);
extern int      bdd_extvarnum(int);
extern int      bdd_isrunning(void);
extern int      bdd_setmaxnodenum(int);
extern int      bdd_setmaxincrease(int);
extern int      bdd_setminfreenodes(int);
extern int      bdd_getnodenum(void);
extern int      bdd_getallocnum(void);
extern char*    bdd_versionstr(void);
extern int      bdd_versionnum(void);
extern void     bdd_stats(bddStat *);
extern void     bdd_cachestats(bddCacheStat *);
extern void     bdd_fprintstat(FILE *);
extern void     bdd_printstat(void);
extern void     bdd_default_gbchandler(int, bddGbcStat *);
extern void     bdd_default_errhandler(int);
extern const char *bdd_errstring(int);
extern void     bdd_clear_error(void);
#ifndef CPLUSPLUS
extern BDD      bdd_true(void);
extern BDD      bdd_false(void);
#endif
extern int      bdd_varnum(void);
extern BDD      bdd_ithvar(int); //两者区别只是 high 和 low反向，并且保存位置一个为偶数，另一个相应为基数，应该是创建一个必然有另一个？为了计算方便？
extern BDD      bdd_nithvar(int);
extern int      bdd_var(BDD);
extern BDD      bdd_low(BDD);
extern BDD      bdd_high(BDD);
extern int      bdd_varlevel(int);
extern BDD      bdd_addref(BDD);
extern BDD      bdd_delref(BDD);
extern void     bdd_gbc(void);
extern void     bdd_gbc_except_applycache(void);
extern int      bdd_scanset(BDD, int**, int*);
extern BDD      bdd_makeset(int *, int);
extern bddPair* bdd_newpair(void);
extern int      bdd_setpair(bddPair*, int, int);
extern int      bdd_setpairs(bddPair*, int*, int*, int);
extern int      bdd_setbddpair(bddPair*, int, BDD);
extern int      bdd_setbddpairs(bddPair*, int*, BDD*, int);
extern void     bdd_resetpair(bddPair *);
extern void     bdd_freepair(bddPair*);

/* In file "bddop.c" */
extern int      bdd_setcacheratio(int);
extern BDD      bdd_buildcube(int, int, BDD *);
extern BDD      bdd_ibuildcube(int, int, int *);
extern BDD      bdd_not(BDD);
extern BDD      bdd_apply(BDD, BDD, int);
extern BDD      bdd_and(BDD, BDD);//求与，交集
extern BDD      bdd_or(BDD, BDD);
extern BDD      bdd_xor(BDD, BDD);
extern BDD      bdd_imp(BDD, BDD);
extern BDD      bdd_biimp(BDD, BDD);
extern BDD      bdd_ite(BDD, BDD, BDD);
extern BDD      bdd_restrict(BDD, BDD);
extern BDD      bdd_constrain(BDD, BDD);
extern BDD      bdd_replace(BDD, bddPair*);
extern BDD      bdd_compose(BDD, BDD, BDD);
extern BDD      bdd_veccompose(BDD, bddPair*);
extern BDD      bdd_simplify(BDD, BDD);
extern BDD      bdd_exist(BDD, BDD);
extern BDD      bdd_forall(BDD, BDD);
extern BDD      bdd_unique(BDD, BDD);
extern BDD      bdd_appex(BDD, BDD, int, BDD);
extern BDD      bdd_appall(BDD, BDD, int, BDD);
extern BDD      bdd_appuni(BDD, BDD, int, BDD);
extern BDD      bdd_support(BDD);
extern BDD      bdd_satone(BDD);
extern BDD      bdd_satoneset(BDD, BDD, BDD);
extern BDD      bdd_fullsatone(BDD);
extern void     bdd_allsat(BDD r, bddallsathandler handler);
extern double   bdd_satcount(BDD);
extern double   bdd_satcountset(BDD, BDD);
extern double   bdd_satcountln(BDD);
extern double   bdd_satcountlnset(BDD, BDD);
extern int      bdd_nodecount(BDD);
extern int      bdd_anodecount(BDD *, int);
extern int*     bdd_varprofile(BDD);
extern double   bdd_pathcount(BDD);

   
/* In file "bddio.c" */
extern void     bdd_printall(void);
extern void     bdd_fprintall(FILE *);
extern void     bdd_fprinttable(FILE *, BDD);
extern void     bdd_printtable(BDD);
extern void     bdd_fprintset(FILE *, BDD);
extern void     bdd_printset(BDD);
extern int      bdd_fnprintdot(char *, BDD);
extern void     bdd_fprintdot(FILE *, BDD);
extern void     bdd_printdot(BDD);
extern int      bdd_fnsave(char *, BDD);
extern int      bdd_save(FILE *, BDD);
extern int      bdd_fnload(char *, BDD *);
extern int      bdd_load(FILE *ifile, BDD *);

/* In file reorder.c */
extern int      bdd_swapvar(int v1, int v2);
extern void     bdd_default_reohandler(int);
extern void     bdd_reorder(int);
extern int      bdd_reorder_gain(void);
extern bddsizehandler bdd_reorder_probe(bddsizehandler);
extern void     bdd_clrvarblocks(void);
extern int      bdd_addvarblock(BDD, int);
extern int      bdd_intaddvarblock(int, int, int);
extern void     bdd_varblockall(void);
extern bddfilehandler bdd_blockfile_hook(bddfilehandler);
extern int      bdd_autoreorder(int);
extern int      bdd_autoreorder_times(int, int);
extern int      bdd_var2level(int);
extern int      bdd_level2var(int);
extern int      bdd_getreorder_times(void);
extern int      bdd_getreorder_method(void);
extern void     bdd_enable_reorder(void);
extern void     bdd_disable_reorder(void);
extern int      bdd_reorder_verbose(int);
extern void     bdd_setvarorder(int *);
extern void     bdd_printorder(void);
extern void     bdd_fprintorder(FILE *);

/*=== BDD constants ===*/
extern const BDD bddfalse;
extern const BDD bddtrue;

/*=== Reordering algorithms ===*/
#define BDD_REORDER_NONE     0
#define BDD_REORDER_WIN2     1
#define BDD_REORDER_WIN2ITE  2
#define BDD_REORDER_SIFT     3
#define BDD_REORDER_SIFTITE  4
#define BDD_REORDER_WIN3     5
#define BDD_REORDER_WIN3ITE  6
#define BDD_REORDER_RANDOM   7

#define BDD_REORDER_FREE     0
#define BDD_REORDER_FIXED    1

/*=== Error codes ===*/
#define BDD_MEMORY (-1)   /* Out of memory */
#define BDD_VAR (-2)      /* Unknown variable */
#define BDD_RANGE (-3)    /* Variable value out of range (not in domain) */
#define BDD_DEREF (-4)    /* Removing external reference to unknown node */
#define BDD_RUNNING (-5)  /* Called bdd_init() twice whithout bdd_done() */
#define BDD_FILE (-6)     /* Some file operation failed */
#define BDD_FORMAT (-7)   /* Incorrect file format */
#define BDD_ORDER (-8)    /* Vars. not in order for vector based functions */
#define BDD_BREAK (-9)    /* User called break */
#define BDD_VARNUM (-10)  /* Different number of vars. for vector pair */
#define BDD_NODES (-11)   /* Tried to set max. number of nodes to be fewer */
                          /* than there already has been allocated */
#define BDD_OP (-12)      /* Unknown operator */
#define BDD_VARSET (-13)  /* Illegal variable set */
#define BDD_VARBLK (-14)  /* Bad variable block operation */
#define BDD_DECVNUM (-15) /* Trying to decrease the number of variables */
#define BDD_REPLACE (-16) /* Replacing to already existing variables */
#define BDD_NODENUM (-17) /* Number of nodes reached user defined maximum */
#define BDD_ILLBDD (-18)  /* Illegal bdd argument */
#define BDD_SIZE (-19)    /* Illegal size argument */

#define BVEC_SIZE (-20)    /* Mismatch in bitvector size */
#define BVEC_SHIFT (-21)   /* Illegal shift-left/right parameter */
#define BVEC_DIVZERO (-22) /* Division by zero */

#define BDD_ERRNUM 24


/* In file kernel.h */
/*=====================================================================================================*/
/*=== SANITY CHECKS ===*/
   /* Make sure we use at least 32 bit integers */
#if (INT_MAX < 0x7FFFFFFF)
#error The compiler does not support 4 byte integers!
#endif

   /* Sanity check argument and return eventual error code */
#define CHECK(r)\
  if (!bddrunning) return bdd_error(BDD_RUNNING);\
  else if ((r) < 0  ||  (r) >= bddnodesize) return bdd_error(BDD_ILLBDD);\
  else if (r >= 2 && LOW(r) == -1) return bdd_error(BDD_ILLBDD)\

   /* Sanity check argument and return eventually the argument 'a' */
#define CHECKa(r,a)\
  if (!bddrunning) { bdd_error(BDD_RUNNING); return (a); }\
  else if ((r) < 0  ||  (r) >= bddnodesize)\
    { bdd_error(BDD_ILLBDD); return (a); }\
  else if (r >= 2 && LOW(r) == -1)\
    { bdd_error(BDD_ILLBDD); return (a); }

#define CHECKn(r)\
  if (!bddrunning) { bdd_error(BDD_RUNNING); return; }\
  else if ((r) < 0  ||  (r) >= bddnodesize)\
    { bdd_error(BDD_ILLBDD); return; }\
  else if (r >= 2 && LOW(r) == -1)\
    { bdd_error(BDD_ILLBDD); return; }



/*=== SEMI-INTERNAL TYPES ===*/

#define SW_NUM 16
struct rule_record {
  uint32_t sw_idx;
  uint32_t idx;
};
// struct sw_record {
//   uint32_t sw_num = SW_NUM;
//   uint32_t sw;
// };

struct rule_records_arr {
  // uint32_t main_nrules;
  uint32_t nrules;
  struct rule_record rules[0];
};

typedef struct s_BddNode {/* Node table entry */
  unsigned int refcou : 10;
  unsigned int level  : 22;
  int low;
  int high;
  int hash;
  int next;
  // int value;
  // struct rule_records_arr *rule_records;
} BddNode;

typedef struct s_MTBddValue {/* Node table entry */
  int hash;
  int next;
  int test_count;
  BDD original_bdd;
  BDD self_bdd;
  BDD coveredby_bdd;
  BDD covering_bdd;
  struct rule_record *main_rule;
  struct rule_records_arr *rule_records;
} MTBddValue;


// typedef struct s_MTBddValue /* Value table entry */
// {
//    unsigned int refcou : 10;
//    unsigned int level  : 22;
//    // int low;
//    // int high;
//    int hash;
//    int next;
//    struct rule_records_arr *rule_records;
// } MTBddValue;

/*=== KERNEL VARIABLES ===*/
extern int       bddrunning;         /* Flag - package initialized */
extern int       bdderrorcond;       /* Some error condition was met */
extern int       bddnodesize;        /* Number of allocated nodes */
extern int       mtbddvaluesize;        /* Number of allocated Values  for MTBDD*/
extern int       bddmaxnodesize;     /* Maximum allowed number of nodes */
extern int       bddmaxnodeincrease; /* Max. # of nodes used to inc. table */
extern BddNode*  bddnodes;           /* All of the bdd nodes */
extern MTBddValue*  mtbddvalues;           /* All of the bdd nodes */
extern int       mtbddvaluevaluecount;


extern int       bddvarnum;          /* Number of defined BDD variables */
extern int*      bddrefstack;        /* Internal node reference stack */
extern int*      bddrefstacktop;     /* Internal node reference stack top */
extern int*      bddvar2level;
extern int*      bddlevel2var;
extern jmp_buf   bddexception;
extern int       bddreorderdisabled;
extern int       bddresized;
extern bddCacheStat bddcachestats;



/*=== KERNEL DEFINITIONS ===*/
#define VERSION 22

#define MAXVAR 0x1FFFFF
#define MAXREF 0x3FF

/* Reference counting */
#define DECREF(n) if (bddnodes[n].refcou!=MAXREF && bddnodes[n].refcou>0) bddnodes[n].refcou--
#define INCREF(n) if (bddnodes[n].refcou<MAXREF) bddnodes[n].refcou++
#define DECREFp(n) if (n->refcou!=MAXREF && n->refcou>0) n->refcou--
#define INCREFp(n) if (n->refcou<MAXREF) n->refcou++
#define HASREF(n) (bddnodes[n].refcou > 0)

/* Marking BDD nodes */
#define MARKON   0x200000    /* Bit used to mark a node (1) */
#define MARKOFF  0x1FFFFF    /* - unmark */
#define MARKHIDE 0x1FFFFF
#define SETMARK(n)  (bddnodes[n].level |= MARKON)
#define UNMARK(n)   (bddnodes[n].level &= MARKOFF)
#define MARKED(n)   (bddnodes[n].level & MARKON)
#define SETMARKp(p) (node->level |= MARKON)
#define UNMARKp(p)  (node->level &= MARKOFF)
#define MARKEDp(p)  (node->level & MARKON)

/* Hashfunctions */
#define PAIR(a,b)      ((unsigned int)((((unsigned int)a)+((unsigned int)b))*(((unsigned int)a)+((unsigned int)b)+((unsigned int)1))/((unsigned int)2)+((unsigned int)a)))
#define TRIPLE(a,b,c)  ((unsigned int)(PAIR((unsigned int)c,PAIR(a,b))))

/* Inspection of BDD nodes */
#define ISCONST(a) ((a) < 2)
#define ISNONCONST(a) ((a) >= 2)
#define ISONE(a)   ((a) == 1)
#define ISZERO(a)  ((a) == 0)
#define LEVEL(a)   (bddnodes[a].level)
#define LOW(a)     (bddnodes[a].low)
#define HIGH(a)    (bddnodes[a].high)
#define LEVELp(p)   ((p)->level)
#define LOWp(p)     ((p)->low)
#define HIGHp(p)    ((p)->high)

/* Stacking for garbage collector */
#define INITREF    bddrefstacktop = bddrefstack
#define PUSHREF(a) *(bddrefstacktop++) = (a)
#define READREF(a) *(bddrefstacktop-(a))
#define POPREF(a)  bddrefstacktop -= (a)

#define BDDONE 1
#define BDDZERO 0

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC DEFAULT_CLOCK
#endif

#define DEFAULTMAXNODEINC 50000

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define NEW(t,n) ( (t*)malloc(sizeof(t)*(n)) )

/*=== KERNEL PROTOTYPES ======================*/ 
extern int    bdd_error(int);
extern int    bdd_makenode(unsigned int, int, int);
extern struct rule_records_arr *gen_rule_records_arr_fr2spec(struct rule_records_arr *, struct rule_records_arr *, int *);
extern bool   isame_rule_records_arr(struct rule_records_arr *, struct rule_records_arr *);
extern BDD    mtbdd_maketnode_1r(struct rule_records_arr *);
extern BDD    mtbdd_maketnode_fr2spec(BDD, BDD);


extern int    bdd_noderesize(int);
extern void   bdd_checkreorder(void);
extern void   bdd_mark(int);
extern void   bdd_mark_upto(int, int);
extern void   bdd_markcount(int, int*);
extern void   bdd_unmark(int);
extern void   bdd_unmark_upto(int, int);
extern void   bdd_register_pair(bddPair*);
extern int   *fdddec2bin(int, int);

extern int    bdd_operator_init(int);
extern void   bdd_operator_done(void);
extern void   bdd_operator_varresize(void);
extern void   bdd_operator_reset(void);
extern void   bdd_operator_reset_except_applycache(void);

extern void   bdd_pairs_init(void);
extern void   bdd_pairs_done(void);
extern int    bdd_pairs_resize(int,int);
extern void   bdd_pairs_vardown(int);

extern void   bdd_fdd_init(void);
extern void   bdd_fdd_done(void);

extern void   bdd_reorder_init(void);
extern void   bdd_reorder_done(void);
extern int    bdd_reorder_ready(void);
extern void   bdd_reorder_auto(void);
extern int    bdd_reorder_vardown(int);
extern int    bdd_reorder_varup(int);

extern void   bdd_cpp_init(void);

/* In file cache.h */
/*=====================================================================================================*/
typedef struct{
  union {
    double dres;
    int res;
  } r;
  int a,b,c;
} BddCacheData;

typedef struct{
  BddCacheData *table;
  int tablesize;
} BddCache;

extern int  BddCache_init(BddCache *, int);
extern void BddCache_done(BddCache *);
extern int  BddCache_resize(BddCache *, int);
extern void BddCache_reset(BddCache *);

#define BddCache_lookup(cache, hash) (&(cache)->table[hash % (cache)->tablesize])

/* In file prime.h */
/*=====================================================================================================*/
unsigned int bdd_prime_gte(unsigned int src);
unsigned int bdd_prime_lte(unsigned int src);

/* In file bddtree.h */
/*=====================================================================================================*/
typedef struct s_BddTree {
  int first, last;  /* First and last variable in this block */
  int pos;          /* Sifting position */
  int *seq;         /* Sequence of first...last in the current order */
  char fixed;       /* Are the sub-blocks fixed or may they be reordered */
  int id;           /* A sequential id number given by addblock */
  struct s_BddTree *next, *prev;
  struct s_BddTree *nextlevel;
} BddTree;

BddTree *bddtree_new(int);
void     bddtree_del(BddTree *);
BddTree *bddtree_addrange(BddTree *, int, int, int, int);
void     bddtree_print(FILE *, BddTree *, int);

/* In file imatrix.h */
/*=====================================================================================================*/
typedef struct _imatrix {
  char **rows;
  int size;
} imatrix;

extern imatrix* imatrixNew(int);
extern void     imatrixDelete(imatrix*);
extern void     imatrixFPrint(imatrix*,FILE *);
extern void     imatrixPrint(imatrix*);
extern void     imatrixSet(imatrix*,int,int);
extern void     imatrixClr(imatrix*,int,int);
extern int      imatrixDepends(imatrix*,int,int);

/* In file fdd.h */
/*=====================================================================================================*/
extern int  fdd_extdomain(int*, int);
extern int  fdd_overlapdomain(int, int);
extern void fdd_clearall(void);
extern int  fdd_domainnum(void);
extern int  fdd_domainsize(int);
extern int  fdd_varnum(int);
extern int* fdd_vars(int);
extern BDD  fdd_ithvar(int, int);
extern int  fdd_scanvar(BDD, int);
extern int* fdd_scanallvar(BDD);
extern BDD  fdd_ithset(int);
extern BDD  fdd_domain(int);
extern BDD  fdd_equals(int, int);
extern bddfilehandler fdd_file_hook(bddfilehandler);
extern void fdd_printset(BDD);
extern void fdd_fprintset(FILE*, BDD);
extern int  fdd_scanset(BDD, int**, int*);
extern BDD  fdd_makeset(int*, int);
extern int  fdd_intaddvarblock(int, int, int);
extern int  fdd_setpair(bddPair*, int, int);
extern int  fdd_setpairs(bddPair*, int*, int*, int);

/* In file bvec.h */
/*=====================================================================================================*/
typedef struct s_bvec {
  int bitnum;
  BDD *bitvec;
} BVEC;

#ifndef CPLUSPLUS
typedef BVEC bvec;
#endif

/* Prototypes for bvec.c */
extern BVEC bvec_copy(BVEC v);
extern BVEC bvec_true(int bitnum);
extern BVEC bvec_false(int bitnum);
extern BVEC bvec_con(int bitnum, int val);
extern BVEC bvec_var(int bitnum, int offset, int step);
extern BVEC bvec_varfdd(int var);
extern BVEC bvec_varvec(int bitnum, int *var);
extern BVEC bvec_coerce(int bitnum, BVEC v);
extern int  bvec_isconst(BVEC e);   
extern int  bvec_val(BVEC e);   
extern void bvec_free(BVEC v);
extern BVEC bvec_addref(BVEC v);
extern BVEC bvec_delref(BVEC v);
extern BVEC bvec_map1(BVEC a, BDD (*fun)(BDD));
extern BVEC bvec_map2(BVEC a, BVEC b, BDD (*fun)(BDD,BDD));
extern BVEC bvec_map3(BVEC a, BVEC b, BVEC c, BDD (*fun)(BDD,BDD,BDD));
extern BVEC bvec_add(BVEC left, BVEC right);
extern BVEC bvec_sub(BVEC left, BVEC right);
extern BVEC bvec_mulfixed(BVEC e, int c);
extern BVEC bvec_mul(BVEC left, BVEC right);
extern int  bvec_divfixed(BVEC e, int c, BVEC *res, BVEC *rem);
extern int  bvec_div(BVEC left, BVEC right, BVEC *res, BVEC *rem);
extern BVEC bvec_ite(BDD a, BVEC b, BVEC c);
extern BVEC bvec_shlfixed(BVEC e, int pos, BDD c);
extern BVEC bvec_shl(BVEC l, BVEC r, BDD c);
extern BVEC bvec_shrfixed(BVEC e, int pos, BDD c);
extern BVEC bvec_shr(BVEC l, BVEC r, BDD c);
extern BDD  bvec_lth(BVEC left, BVEC right);
extern BDD  bvec_lte(BVEC left, BVEC right);
extern BDD  bvec_gth(BVEC left, BVEC right);
extern BDD  bvec_gte(BVEC left, BVEC right);
extern BDD  bvec_equ(BVEC left, BVEC right);
extern BDD  bvec_neq(BVEC left, BVEC right);

/* In file pairs.c */
/*=====================================================================================================*/
static int      pairsid;            /* Pair identifier */
static bddPair* pairs;              /* List of all replacement pairs in use */
/***************************************************************************/
void bdd_pairs_init(void) {
  pairsid = 0;
  pairs = NULL;
}

void bdd_pairs_done(void) {
  bddPair *p = pairs;
  int n;

  while (p != NULL) {
    bddPair *next = p->next;
    for (n=0 ; n<bddvarnum ; n++)
      bdd_delref( p->result[n] );
    free(p->result);
    free(p);
    p = next;
  }
}

static int update_pairsid(void) {
  pairsid++;

  if (pairsid == (INT_MAX >> 2)) {
    bddPair *p;
    pairsid = 0;
    for (p=pairs ; p!=NULL ; p=p->next)
      p->id = pairsid++;
    bdd_operator_reset();
  }

  return pairsid;
}

void bdd_register_pair(bddPair *p) {
  p->next = pairs;
  pairs = p;
}

void bdd_pairs_vardown(int level) {
  bddPair *p;

  for (p=pairs ; p!=NULL ; p=p->next) {
    int tmp;

    tmp = p->result[level];
    p->result[level] = p->result[level+1];
    p->result[level+1] = tmp;
    
    if (p->last == level)
      p->last++;
  }
}

int bdd_pairs_resize(int oldsize, int newsize) {
  bddPair *p;
  int n;

  for (p=pairs ; p!=NULL ; p=p->next) {
    if ((p->result=(BDD*)realloc(p->result,sizeof(BDD)*newsize)) == NULL)
      return bdd_error(BDD_MEMORY);
    for (n=oldsize ; n<newsize ; n++)
      p->result[n] = bdd_ithvar(bddlevel2var[n]);
  }

  return 0;
}

bddPair *bdd_newpair(void) {
  int n;
  bddPair *p;

  if ((p=(bddPair*)malloc(sizeof(bddPair))) == NULL) {
    bdd_error(BDD_MEMORY);
    return NULL;
  }

  if ((p->result=(BDD*)malloc(sizeof(BDD)*bddvarnum)) == NULL) {
    free(p);
    bdd_error(BDD_MEMORY);
    return NULL;
  }

  for (n=0 ; n<bddvarnum ; n++)
    p->result[n] = bdd_ithvar(bddlevel2var[n]);

  p->id = update_pairsid();
  p->last = -1;

  bdd_register_pair(p);
  return p;
}

int bdd_setpair(bddPair *pair, int oldvar, int newvar) {
  if (pair == NULL)
    return 0;

  if (oldvar < 0  ||  oldvar > bddvarnum-1)
    return bdd_error(BDD_VAR);
  if (newvar < 0  ||  newvar > bddvarnum-1)
    return bdd_error(BDD_VAR);

  bdd_delref( pair->result[bddvar2level[oldvar]] );
  pair->result[bddvar2level[oldvar]] = bdd_ithvar(newvar);
  pair->id = update_pairsid();

  if (bddvar2level[oldvar] > pair->last)
    pair->last = bddvar2level[oldvar];
    
  return 0;
}

int bdd_setbddpair(bddPair *pair, int oldvar, BDD newvar) {
  int oldlevel;

  if (pair == NULL)
    return 0;

  CHECK(newvar);
  if (oldvar < 0  ||  oldvar >= bddvarnum)
    return bdd_error(BDD_VAR);
  oldlevel = bddvar2level[oldvar];
    
  bdd_delref( pair->result[oldlevel] );
  pair->result[oldlevel] = bdd_addref(newvar);
  pair->id = update_pairsid();

  if (oldlevel > pair->last)
    pair->last = oldlevel;
    
  return 0;
}

int bdd_setpairs(bddPair *pair, int *oldvar, int *newvar, int size) {
  int n,e;
  if (pair == NULL)
    return 0;

  for (n=0 ; n<size ; n++)
    if ((e=bdd_setpair(pair, oldvar[n], newvar[n])) < 0)
  return e;

  return 0;
}

int bdd_setbddpairs(bddPair *pair, int *oldvar, BDD *newvar, int size) {
  int n,e;
  if (pair == NULL)
    return 0;

  for (n=0 ; n<size ; n++)
    if ((e=bdd_setbddpair(pair, oldvar[n], newvar[n])) < 0)
  return e;

  return 0;
}

void bdd_freepair(bddPair *p) {
  int n;

  if (p == NULL)
    return;

  if (pairs != p) {
    bddPair *bp = pairs;
    while (bp != NULL  &&  bp->next != p)
      bp = bp->next;

    if (bp != NULL)
      bp->next = p->next;
  }
  else
    pairs = p->next;

  for (n=0 ; n<bddvarnum ; n++)
    bdd_delref( p->result[n] );
  free(p->result);
  free(p);
}

void bdd_resetpair(bddPair *p) {
  int n;

  for (n=0 ; n<bddvarnum ; n++)
    p->result[n] = bdd_ithvar(n);
  p->last = 0;
}

/* In file bddio.c */
/*=====================================================================================================*/
static void bdd_printset_rec(FILE *, int, int *);
static void bdd_fprintdot_rec(FILE*, BDD);
static int  bdd_save_rec(FILE*, int);
static int  bdd_loaddata(FILE *);
static int  loadhash_get(int);
static void loadhash_add(int, int);

static bddfilehandler filehandler;

typedef struct s_LoadHash {
  int key;
  int data;
  int first;
  int next;
} LoadHash;

static LoadHash *lh_table;
static int       lh_freepos;
static int       lh_nodenum;
static int      *loadvar2level;

/*=== PRINTING ====*/
bddfilehandler bdd_file_hook(bddfilehandler handler) {
  bddfilehandler old = filehandler;
  filehandler = handler;
  return old;
}

void bdd_printall(void) {
  bdd_fprintall(stdout);
}

void bdd_fprintall(FILE *ofile) {
  int n;
   
  for (n=0 ; n<bddnodesize ; n++) {
    if (LOW(n) != -1) {
      fprintf(ofile, "[%5d - %2d] ", n, bddnodes[n].refcou);
      if (filehandler)
        filehandler(ofile, bddlevel2var[bddnodes[n].level]);
      else
        fprintf(ofile, "%3d", bddlevel2var[bddnodes[n].level]);

    fprintf(ofile, ": %3d", LOW(n));
    fprintf(ofile, " %3d", HIGH(n));
    fprintf(ofile, "\n");
    }
  }
}

void bdd_printtable(BDD r) {
  bdd_fprinttable(stdout, r);
}

void bdd_fprinttable(FILE *ofile, BDD r) {
  BddNode *node;
  int n;

  fprintf(ofile, "ROOT: %d\n", r);
  if (r < 2)
    return;

  bdd_mark(r);

  for (n=0 ; n<bddnodesize ; n++) {
    if (bddnodes[n].level & MARKON) {
      node = &bddnodes[n];


      (node)->level &= MARKOFF;

      fprintf(ofile, "[%5d] ", n);
      if (filehandler)
        filehandler(ofile, bddlevel2var[(node)->level]);
      else
        fprintf(ofile, "%3d", bddlevel2var[(node)->level]);

      fprintf(ofile, ": %3d", LOWp(node));
      fprintf(ofile, " %3d", HIGHp(node));
      fprintf(ofile, "\n");
    }
  }
}

void bdd_printset(BDD r) {
  bdd_fprintset(stdout, r);
}

void bdd_fprintset(FILE *ofile, BDD r) {
  int *set;

  if (r < 2) {
    fprintf(ofile, "%s", r == 0 ? "F" : "T");
    return;
  }

  if ((set=(int *)malloc(sizeof(int)*bddvarnum)) == NULL) {
    bdd_error(BDD_MEMORY);
    return;
  }

  memset(set, 0, sizeof(int) * bddvarnum);
  bdd_printset_rec(ofile, r, set);
  free(set);
}

static void bdd_printset_rec(FILE *ofile, int r, int *set) {
  int n;
  int first;

  if (r == 0)
    return;
  else
  if (r == 1) {
    fprintf(ofile, "<");
    first = 1;
    for (n=0 ; n<bddvarnum ; n++) {
      if (set[n] > 0) {
        if (!first)
          fprintf(ofile, ", ");
        first = 0;
        if (filehandler)
          filehandler(ofile, bddlevel2var[n]);
        else
          fprintf(ofile, "%d", bddlevel2var[n]);
        fprintf(ofile, ":%d", (set[n]==2 ? 1 : 0));
      }
    }
    fprintf(ofile, ">");
  }
  else {
    set[LEVEL(r)] = 1;
    bdd_printset_rec(ofile, LOW(r), set);
    
    set[LEVEL(r)] = 2;
    bdd_printset_rec(ofile, HIGH(r), set);
    
    set[LEVEL(r)] = 0;
  }
}

void bdd_printdot(BDD r) {
  bdd_fprintdot(stdout, r);
}

int bdd_fnprintdot(char *fname, BDD r) {
  FILE *ofile = fopen(fname, "w");
  if (ofile == NULL)
    return bdd_error(BDD_FILE);
  bdd_fprintdot(ofile, r);
  fclose(ofile);
  return 0;
}

void bdd_fprintdot(FILE* ofile, BDD r) {
  fprintf(ofile, "digraph G {\n");
  fprintf(ofile, "0 [shape=box, label=\"0\", style=filled, shape=box, height=0.3, width=0.3];\n");
  fprintf(ofile, "1 [shape=box, label=\"1\", style=filled, shape=box, height=0.3, width=0.3];\n");

  bdd_fprintdot_rec(ofile, r);

  fprintf(ofile, "}\n");

  bdd_unmark(r);
}

static void bdd_fprintdot_rec(FILE* ofile, BDD r) {
  if (ISCONST(r) || MARKED(r))
    return;

  fprintf(ofile, "%d [label=\"", r);
  if (filehandler)
    filehandler(ofile, bddlevel2var[LEVEL(r)]);
  else
    fprintf(ofile, "%d", bddlevel2var[LEVEL(r)]);
  fprintf(ofile, "\"];\n");

  fprintf(ofile, "%d -> %d [style=dotted];\n", r, LOW(r));
  fprintf(ofile, "%d -> %d [style=filled];\n", r, HIGH(r));

  SETMARK(r);

  bdd_fprintdot_rec(ofile, LOW(r));
  bdd_fprintdot_rec(ofile, HIGH(r));
}

/*=== SAVE ===*/
int bdd_fnsave(char *fname, BDD r) {
  FILE *ofile;
  int ok;

  if ((ofile=fopen(fname,"w")) == NULL)
    return bdd_error(BDD_FILE);

  ok = bdd_save(ofile, r);
  fclose(ofile);
  return ok;
}

int bdd_save(FILE *ofile, BDD r) {
  int err, n=0;

  if (r < 2){//0 1 两个节点，也就是只有一个节点的时候
    fprintf(ofile, "0 0 %d\n", r);
    return 0;
  }

  bdd_markcount(r, &n);
  bdd_unmark(r);
  fprintf(ofile, "%d %d\n", n, bddvarnum);//在文件开头记录总数量n，和变量数目

  for (n=0 ; n<bddvarnum ; n++)
    fprintf(ofile, "%d ", bddvar2level[n]);//把bddvarnum转化为level保存
  fprintf(ofile, "\n");//然后下一行

  err = bdd_save_rec(ofile, r);//保存结构
  bdd_unmark(r);

  return err;
}

static int bdd_save_rec(FILE *ofile, int root) {
  BddNode *node = &bddnodes[root]; 
  int err;

  if (root < 2)
    return 0;

  if ((node)->level & MARKON)
    return 0;
  (node)->level |= MARKON;

  if ((err=bdd_save_rec(ofile, LOWp(node))) < 0)
    return err;
  if ((err=bdd_save_rec(ofile, HIGHp(node))) < 0)
    return err;

  fprintf(ofile, "%d %d %d %d\n",
   root, bddlevel2var[(node)->level & MARKHIDE],
   LOWp(node), HIGHp(node));

  return 0;
}

/*=== LOAD ===*/
int bdd_fnload(char *fname, BDD *root) {
  FILE *ifile;
  int ok;

  if ((ifile=fopen(fname,"r")) == NULL)
    return bdd_error(BDD_FILE);

  ok = bdd_load(ifile, root);
  fclose(ifile);
  return ok;
}

int bdd_load(FILE *ifile, BDD *root) {
  int n, vnum, tmproot;

  if (fscanf(ifile, "%d %d", &lh_nodenum, &vnum) != 2)
    return bdd_error(BDD_FORMAT);

    /* Check for constant true / false */
  if (lh_nodenum==0  &&  vnum==0) {
    fscanf(ifile, "%d", root);
    return 0;
  }

  if ((loadvar2level=(int*)malloc(sizeof(int)*vnum)) == NULL)
    return bdd_error(BDD_MEMORY);
  for (n=0 ; n<vnum ; n++)
    fscanf(ifile, "%d", &loadvar2level[n]);

  if (vnum > bddvarnum)
    bdd_setvarnum(vnum);

  if ((lh_table=(LoadHash*)malloc(lh_nodenum*sizeof(LoadHash))) == NULL)
    return bdd_error(BDD_MEMORY);

  for (n=0 ; n<lh_nodenum ; n++) {
    lh_table[n].first = -1;
    lh_table[n].next = n+1;
  }
  lh_table[lh_nodenum-1].next = -1;
  lh_freepos = 0;

  tmproot = bdd_loaddata(ifile);

  for (n=0 ; n<lh_nodenum ; n++)
    bdd_delref(lh_table[n].data);

  free(lh_table);
  free(loadvar2level);

  *root = 0;
  if (tmproot < 0)
    return tmproot;
  else
    *root = tmproot;

  return 0;
}

static int bdd_loaddata(FILE *ifile) {
  int key,var,low,high,root=0,n;

  for (n=0 ; n<lh_nodenum ; n++) {
    if (fscanf(ifile,"%d %d %d %d", &key, &var, &low, &high) != 4)
      return bdd_error(BDD_FORMAT);

    if (low >= 2)
      low = loadhash_get(low);
    if (high >= 2)
      high = loadhash_get(high);

    if (low<0 || high<0 || var<0)
      return bdd_error(BDD_FORMAT);

    root = bdd_addref( bdd_ite(bdd_ithvar(var), high, low) );

    loadhash_add(key, root);
  }

  return root;
}

static void loadhash_add(int key, int data) {
  int hash = key % lh_nodenum;
  int pos = lh_freepos;

  lh_freepos = lh_table[pos].next;
  lh_table[pos].next = lh_table[hash].first;
  lh_table[hash].first = pos;

  lh_table[pos].key = key;
  lh_table[pos].data = data;
}

static int loadhash_get(int key) {
  int hash = lh_table[key % lh_nodenum].first;

  while (hash != -1  &&  lh_table[hash].key != key)
    hash = lh_table[hash].next;

  if (hash == -1)
    return -1;
  return lh_table[hash].data;
}

/* In file bddop.c */
/*=====================================================================================================*/
/* Hash value modifiers to distinguish between entries in misccache */
#define CACHEID_CONSTRAIN   0x0
#define CACHEID_RESTRICT    0x1
#define CACHEID_SATCOU      0x2
#define CACHEID_SATCOULN    0x3
#define CACHEID_PATHCOU     0x4

/* Hash value modifiers for replace/compose */
#define CACHEID_REPLACE      0x0
#define CACHEID_COMPOSE      0x1
#define CACHEID_VECCOMPOSE   0x2

/* Hash value modifiers for quantification */
#define CACHEID_EXIST        0x0
#define CACHEID_FORALL       0x1
#define CACHEID_UNIQUE       0x2
#define CACHEID_APPEX        0x3
#define CACHEID_APPAL        0x4
#define CACHEID_APPUN        0x5

/* Number of boolean operators */
#define OPERATOR_NUM    11

/* Operator results - entry = left<<1 | right  (left,right in {0,1}) */
static int oprres[OPERATOR_NUM][4] = 
{ {0,0,0,1},  /* and                       ( & )         */
  {0,1,1,0},  /* xor                       ( ^ )         */
  {0,1,1,1},  /* or                        ( | )         */
  {1,1,1,0},  /* nand                                    */
  {1,0,0,0},  /* nor                                     */
  {1,1,0,1},  /* implication               ( >> )        */
  {1,0,0,1},  /* bi-implication                          */
  {0,0,1,0},  /* difference /greater than  ( - ) ( > )   */
  {0,1,0,0},  /* less than                 ( < )         */
  {1,0,1,1},  /* inverse implication       ( << )        */
  {1,1,0,0}   /* not                       ( ! )         */
};


   /* Variables needed for the operators */
static int applyop;                 /* Current operator for apply */
static int appexop;                 /* Current operator for appex */
static int appexid;                 /* Current cache id for appex */
static int quantid;                 /* Current cache id for quantifications */
static int *quantvarset;            /* Current variable set for quant. */
static int quantvarsetID;           /* Current id used in quantvarset */
static int quantlast;               /* Current last variable to be quant. */
static int replaceid;               /* Current cache id for replace */
static int *replacepair;            /* Current replace pair */
static int replacelast;             /* Current last var. level to replace */
static int composelevel;            /* Current variable used for compose */
static int miscid;                  /* Current cache id for other results */
static int *varprofile;             /* Current variable profile */
static int supportID;               /* Current ID (true value) for support */
static int supportMin;              /* Min. used level in support calc. */
static int supportMax;              /* Max. used level in support calc. */
static int* supportSet;             /* The found support set */
static BddCache applycache;         /* Cache for apply results */
static BddCache itecache;           /* Cache for ITE results */
static BddCache quantcache;         /* Cache for exist/forall results */
static BddCache appexcache;         /* Cache for appex/appall results */
static BddCache replacecache;       /* Cache for replace results */
static BddCache misccache;          /* Cache for other results */
static int cacheratio;
static BDD satPolarity;
static int firstReorder;            /* Used instead of local variable in order
				       to avoid compiler warning about 'first'
				       being clobbered by setjmp */

static char*            allsatProfile; /* Variable profile for bdd_allsat() */
static bddallsathandler allsatHandler; /* Callback handler for bdd_allsat() */

extern bddCacheStat bddcachestats;

int       hit_cache_counter;
int       calc_node_counter;
int       test_counter;

   /* Internal prototypes */
static BDD    not_rec(BDD);
static BDD    apply_rec(BDD, BDD);
static BDD    ite_rec(BDD, BDD, BDD);
static int    simplify_rec(BDD, BDD);
static int    quant_rec(int);
static int    appquant_rec(int, int);
static int    restrict_rec(int);
static BDD    constrain_rec(BDD, BDD);
static BDD    replace_rec(BDD);
static BDD    bdd_correctify(int, BDD, BDD);
static BDD    compose_rec(BDD, BDD);
static BDD    veccompose_rec(BDD);
static void   support_rec(int, int*);
static BDD    satone_rec(BDD);
static BDD    satoneset_rec(BDD, BDD);
static int    fullsatone_rec(int);
static void   allsat_rec(BDD r);
static double satcount_rec(int);
static double satcountln_rec(int);
static void   varprofile_rec(int);
static double bdd_pathcount_rec(BDD);
static int    varset2vartable(BDD);
static int    varset2svartable(BDD);


   /* Hashvalues */
#define NOTHASH(r)           (r)
#define APPLYHASH(l,r,op)    (TRIPLE(l,r,op))
#define ITEHASH(f,g,h)       (TRIPLE(f,g,h))
#define RESTRHASH(r,var)     (PAIR(r,var))
#define CONSTRAINHASH(f,c)   (PAIR(f,c))
#define QUANTHASH(r)         (r)
#define REPLACEHASH(r)       (r)
#define VECCOMPOSEHASH(f)    (f)
#define COMPOSEHASH(f,g)     (PAIR(f,g))
#define SATCOUHASH(r)        (r)
#define PATHCOUHASH(r)       (r)
#define APPEXHASH(l,r,op)    (PAIR(l,r))

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#define log1p(a) (log(1.0+a))

#define INVARSET(a) (quantvarset[a] == quantvarsetID) /* unsigned check */
#define INSVARSET(a) (abs(quantvarset[a]) == quantvarsetID) /* signed check */

int bdd_operator_init(int cachesize) {
  if (BddCache_init(&applycache,cachesize) < 0)
    return bdd_error(BDD_MEMORY);

  if (BddCache_init(&itecache,cachesize) < 0)
    return bdd_error(BDD_MEMORY);

  if (BddCache_init(&quantcache,cachesize) < 0)
    return bdd_error(BDD_MEMORY);

  if (BddCache_init(&appexcache,cachesize) < 0)
    return bdd_error(BDD_MEMORY);

  if (BddCache_init(&replacecache,cachesize) < 0)
    return bdd_error(BDD_MEMORY);

  if (BddCache_init(&misccache,cachesize) < 0)
    return bdd_error(BDD_MEMORY);

  quantvarsetID = 0;
  quantvarset = NULL;
  cacheratio = 0;
  supportSet = NULL;

  return 0;
}

void bdd_operator_done(void) {
  if (quantvarset != NULL)
    free(quantvarset);

  BddCache_done(&applycache);
  BddCache_done(&itecache);
  BddCache_done(&quantcache);
  BddCache_done(&appexcache);
  BddCache_done(&replacecache);
  BddCache_done(&misccache);

  if (supportSet != NULL)
   free(supportSet);
}

void bdd_operator_reset(void) {
  BddCache_reset(&applycache);
  BddCache_reset(&itecache);
  BddCache_reset(&quantcache);
  BddCache_reset(&appexcache);
  BddCache_reset(&replacecache);
  BddCache_reset(&misccache);
}

void bdd_operator_reset_except_applycache(void) {
  BddCache_reset(&itecache);
  BddCache_reset(&quantcache);
  BddCache_reset(&appexcache);
  BddCache_reset(&replacecache);
  BddCache_reset(&misccache);
}

void bdd_operator_varresize(void) {
  if (quantvarset != NULL)
    free(quantvarset);

  if ((quantvarset=NEW(int,bddvarnum)) == NULL)
    bdd_error(BDD_MEMORY);

  memset(quantvarset, 0, sizeof(int)*bddvarnum);
  quantvarsetID = 0;
}

static void bdd_operator_noderesize(void) {
  if (cacheratio > 0) {
    int newcachesize = bddnodesize / cacheratio;
    
    BddCache_resize(&applycache, newcachesize);
    BddCache_resize(&itecache, newcachesize);
    BddCache_resize(&quantcache, newcachesize);
    BddCache_resize(&appexcache, newcachesize);
    BddCache_resize(&replacecache, newcachesize);
    BddCache_resize(&misccache, newcachesize);
  }
}

int bdd_setcacheratio(int r) {
  int old = cacheratio;

  if (r <= 0)
    return bdd_error(BDD_RANGE);
  if (bddnodesize == 0)
    return old;

  cacheratio = r;
  bdd_operator_noderesize();
  return old;
}

static void checkresize(void) {
  if (bddresized)
    bdd_operator_noderesize();
  bddresized = 0;
}

/*=== BUILD A CUBE ===*/
BDD bdd_buildcube(int value, int width, BDD *variables) {
  BDD result = BDDONE;
  int z;

  for (z=0 ; z<width ; z++, value>>=1) {
    BDD tmp;
    BDD v;
    
    if (value & 0x1)
      v = bdd_addref( variables[width-z-1] );
    else
      v = bdd_addref( bdd_not(variables[width-z-1]) );

    bdd_addref(result);
    tmp = bdd_apply(result,v,bddop_and);
    bdd_delref(result);
    bdd_delref(v);

    result = tmp;
  }

  return result;
}

BDD bdd_ibuildcube(int value, int width, int *variables) {
  BDD result = BDDONE;
  int z;

  for (z=0 ; z<width ; z++, value>>=1) {
    BDD tmp;
    BDD v;
    
    if (value & 0x1)
      v = bdd_ithvar(variables[width-z-1]);
    else
      v = bdd_nithvar(variables[width-z-1]);

    bdd_addref(result);
    tmp = bdd_apply(result,v,bddop_and);
    bdd_delref(result);

    result = tmp;
  }

  return result;
}

/*=== NOT ===*/
BDD bdd_not(BDD r) {
   BDD res;
   firstReorder = 1;
   CHECKa(r, bddfalse);

 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = not_rec(r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();
      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD not_rec(BDD r) {
  BddCacheData *entry;
  BDD res;
  if (ISZERO(r))
    return BDDONE;
  if (ISONE(r))
    return BDDZERO;

  entry = BddCache_lookup(&applycache, NOTHASH(r));  
  if (entry->a == r  &&  entry->c == bddop_not) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif   
  PUSHREF( not_rec(LOW(r)) );
  PUSHREF( not_rec(HIGH(r)) );
  res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
  POPREF(2);

  entry->a = r;
  entry->c = bddop_not;
  entry->r.res = res;

  return res;
}

/*=== APPLY ===*/
BDD
check_bddfalse(BDD l, BDD r) {
  CHECKa(l, bddfalse);
  CHECKa(r, bddfalse);
  return 0;
}

BDD bdd_apply(BDD l, BDD r, int op) {
  BDD res;
  firstReorder = 1;

  CHECKa(l, bddfalse);
  CHECKa(r, bddfalse);

  if (op<0 || op>bddop_invimp)
  {
    bdd_error(BDD_OP);
    return bddfalse;
  }

  again:
  if (setjmp(bddexception) == 0)
  {
    INITREF;
    applyop = op;
    
    if (!firstReorder)
      bdd_disable_reorder();
    res = apply_rec(l, r);
    if (!firstReorder)
      bdd_enable_reorder();
  }
  else
  {
    bdd_checkreorder();

    if (firstReorder-- == 1)
      goto again;
    res = BDDZERO;  /* avoid warning about res being uninitialized */
  }

  checkresize();
  return res;
}

static BDD apply_rec(BDD l, BDD r) {
  BddCacheData *entry;
  BDD res;

  switch (applyop) {
  case bddop_and:
     if (l == r)
  return l;
     if (ISZERO(l)  ||  ISZERO(r))
  return 0;
     if (ISONE(l))
  return r;
     if (ISONE(r))
  return l;
     break;
  case bddop_or:
     if (l == r)
  return l;
     if (ISONE(l)  ||  ISONE(r))
  return 1;
     if (ISZERO(l))
  return r;
     if (ISZERO(r))
  return l;
     break;
  case bddop_xor:
     if (l == r)
  return 0;
     if (ISZERO(l))
  return r;
     if (ISZERO(r))
  return l;
     break;
  case bddop_nand:
     if (ISZERO(l) || ISZERO(r))
  return 1;
     break;
  case bddop_nor:
     if (ISONE(l)  ||  ISONE(r))
  return 0;
     break;
  case bddop_imp:
    if (ISZERO(l))
  return 1;
    if (ISONE(l))
  return r;
    if (ISONE(r))
  return 1;
    break;
  }

  if (ISCONST(l)  &&  ISCONST(r))
    res = oprres[applyop][l<<1 | r];
  else {
    entry = BddCache_lookup(&applycache, APPLYHASH(l,r,applyop));
    if (entry->a == l  &&  entry->b == r  &&  entry->c == applyop) {
      #ifdef CACHESTATS
      bddcachestats.opHit++;
      #endif
      return entry->r.res;
    }
    #ifdef CACHESTATS
    bddcachestats.opMiss++;
    #endif
    if (LEVEL(l) == LEVEL(r)) {
      PUSHREF( apply_rec(LOW(l), LOW(r)) );
      PUSHREF( apply_rec(HIGH(l), HIGH(r)) );
      res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
    }
    else
    if (LEVEL(l) < LEVEL(r)) {
      PUSHREF( apply_rec(LOW(l), r) );
      PUSHREF( apply_rec(HIGH(l), r) );
      res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
    }
    else {
      PUSHREF( apply_rec(l, LOW(r)) );
      PUSHREF( apply_rec(l, HIGH(r)) );
      res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
    }
    POPREF(2);
    entry->a = l;
    entry->b = r;
    entry->c = applyop;
    entry->r.res = res;
  }
  return res;
}

BDD
bdd_v2x_rec(BDD root, BDD mask) {
  BddCacheData *entry;
  BDD res;
  if ((root < 2) || (mask < 2))
    return root;


  entry = BddCache_lookup(&applycache, APPLYHASH(root,mask,12));
  if (entry->a == root  &&  entry->b == mask  &&  entry->c == 12) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
    #ifdef CACHESTATS
    bddcachestats.opMiss++;
    #endif
  }
  if (LEVEL(root) == LEVEL(mask)) {//root需要变x
    res = apply_rec(bdd_v2x_rec(LOW(root), LOW(mask)), bdd_v2x_rec(HIGH(root), LOW(mask)));
  }
  else if (LEVEL(root) < LEVEL(mask)) {
    // res = bdd_v2x_rec(root, LOW(mask));
    PUSHREF( bdd_v2x_rec(LOW(root), mask));
    PUSHREF( bdd_v2x_rec(HIGH(root), mask));
    res = bdd_makenode(LEVEL(root), READREF(2), READREF(1));
    POPREF(2);
  }
  else {
    res = bdd_v2x_rec(root, LOW(mask));
  }

  entry->a = root;
  entry->b = mask;
  entry->c = 12;
  entry->r.res = res;

  return res;
}

BDD
mtbdd_apply_rec(BDD a, BDD b) { 
  BddCacheData *entry;
  BDD res;
  if (b < 1)
    return a;
  if (a < 1)
    return b;

  // if (LOW(a) == 1 && LOW(b) == 1)
  //   return mtbdd_maketnode_fr2spec(a, b);

  entry = BddCache_lookup(&applycache, APPLYHASH(a,b,13));
  if (entry->a == a  &&  entry->b == b  &&  entry->c == 13) {
    return entry->r.res;
  }
  
  // if (LOW(a) == 1) {
  if (LOW(a) == 1 && LOW(b) == 1)
    res = mtbdd_maketnode_fr2spec(a, b);
  else if (LOW(a) == 1) {
    PUSHREF( mtbdd_apply_rec(a, LOW(b)) );
    PUSHREF( mtbdd_apply_rec(a, HIGH(b)) );
    res = bdd_makenode(LEVEL(b), READREF(2), READREF(1));
  }
  else if(LOW(b) == 1) {
    PUSHREF( mtbdd_apply_rec(LOW(a), b) );
    PUSHREF( mtbdd_apply_rec(HIGH(a), b) );
    res = bdd_makenode(LEVEL(b), READREF(2), READREF(1));
  }
  else{
    if (LEVEL(a) == LEVEL(b)) {
       PUSHREF( mtbdd_apply_rec(LOW(a), LOW(b)));
       PUSHREF( mtbdd_apply_rec(HIGH(a), HIGH(b)));
       res = bdd_makenode(LEVEL(a), READREF(2), READREF(1));
    }
    else if (LEVEL(a) < LEVEL(b)) {
       PUSHREF( mtbdd_apply_rec(LOW(a), b) );
       PUSHREF( mtbdd_apply_rec(HIGH(a), b) );
       res = bdd_makenode(LEVEL(a), READREF(2), READREF(1));
    }
    else {
       PUSHREF( mtbdd_apply_rec(a, LOW(b)) );
       PUSHREF( mtbdd_apply_rec(a, HIGH(b)) );
       res = bdd_makenode(LEVEL(b), READREF(2), READREF(1));
    }
  }

  POPREF(2);

  entry->a = a;
  entry->b = b;
  entry->c = 13;
  entry->r.res = res;

  return res;
}

BDD
mtbdd_apply(BDD a, BDD b) {
  CHECKa(a, bddfalse);
  CHECKa(b, bddfalse);
  return mtbdd_apply_rec(a,b);
}

BDD
mtbdd_apply_addr_rec(BDD l, BDD r) { 
  BddCacheData *entry;
  BDD res;
  if (r < 1)
    return l;
  if (l < 1)
    return r;
  
  // if (LOW(l) == 2 && LOW(r) == 2)
  //   res = mtbdd_maketnode_fr_2tn_add(l, r);
  // if (LOW(a) == 1 && LOW(b) == 1)
  //   return mtbdd_maketnode_fr2spec(a, b);
  if (r == 1 || l == 1)
    bdd_error(BDD_ILLBDD);

  entry = BddCache_lookup(&applycache, APPLYHASH(l,r,14));
  if (entry->a == l  &&  entry->b == r  &&  entry->c == 14) {
    // hit_cache_counter++;
    return entry->r.res;
  }
  
  // if ((LOW(l) == 2) && (LOW(r) == 2))
  //   return mtbdd_maketnode_fr_2tn_add(l, r);

  if (LOW(l) == 2) {
    if ((LOW(r) == 2))
  // if ((LOW(l) == 2) && (LOW(r) == 2)) {
      res = mtbdd_maketnode_fr_2tn_add_simple(l, r);
      // res = mtbdd_maketnode_fr_2tn_newv(l, r);
    else
      res = bdd_makenode(LEVEL(r), mtbdd_apply_addr_rec(l, LOW(r)), mtbdd_apply_addr_rec(l, HIGH(r)));
    
  // else if (LOW(l) == 2) {
    // PUSHREF( mtbdd_apply_addr_rec(l, LOW(r)) );
    // PUSHREF( mtbdd_apply_addr_rec(l, HIGH(r)) );
    // res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
    // res = bdd_makenode(LEVEL(r), mtbdd_apply_addr_rec(l, LOW(r)), mtbdd_apply_addr_rec(l, HIGH(r)));
  }
  else if(LOW(r) == 2) {
    // PUSHREF( mtbdd_apply_addr_rec(LOW(l), r) );
    // PUSHREF( mtbdd_apply_addr_rec(HIGH(l), r) );
    // res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
    res = bdd_makenode(LEVEL(l), mtbdd_apply_addr_rec(LOW(l), r), mtbdd_apply_addr_rec(HIGH(l), r));
  }
  else{
    if (LEVEL(l) == LEVEL(r)) {
      // PUSHREF( mtbdd_apply_addr_rec(LOW(l), LOW(r)));
      // PUSHREF( mtbdd_apply_addr_rec(HIGH(l), HIGH(r)));
      // res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
      res = bdd_makenode(LEVEL(l), mtbdd_apply_addr_rec(LOW(l), LOW(r)), mtbdd_apply_addr_rec(HIGH(l), HIGH(r)));
    }
    else if (LEVEL(l) < LEVEL(r)) {
      // PUSHREF( mtbdd_apply_addr_rec(LOW(l), r) );
      // PUSHREF( mtbdd_apply_addr_rec(HIGH(l), r) );
      // res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
      res = bdd_makenode(LEVEL(l), mtbdd_apply_addr_rec(LOW(l), r), mtbdd_apply_addr_rec(HIGH(l), r));
    }
    else {
      // PUSHREF( mtbdd_apply_addr_rec(l, LOW(r)) );
      // PUSHREF( mtbdd_apply_addr_rec(l, HIGH(r)) );
      // res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
      res = bdd_makenode(LEVEL(r), mtbdd_apply_addr_rec(l, LOW(r)), mtbdd_apply_addr_rec(l, HIGH(r)));
    }
  }

  // POPREF(2);
  entry->a = l;
  entry->b = r;
  entry->c = 14;
  entry->r.res = res;

  return res;
}

BDD
mtbdd_add_r(BDD a, BDD b, uint32_t sign) {
  CHECKa(a, bddfalse);
  CHECKa(b, bddfalse);
  hit_cache_counter = 0;
  calc_node_counter = 0;
  BDD res = mtbdd_apply_addr_rec(a, b);
  if (sign == 1) {
    printf("the hit_cache_counter is %d\n", hit_cache_counter);
    printf("the calc_node_counter is %d\n", calc_node_counter);
    // printf("a has %d nodes\n", bdd_nodecount(a));
    // printf("b has %d nodes\n", bdd_nodecount(b));
    // printf("res has %d nodes\n", bdd_nodecount(res));
  }
  return res;
}

BDD bdd_and(BDD l, BDD r) {
  return bdd_apply(l,r,bddop_and);
}

BDD bdd_or(BDD l, BDD r) {
  return bdd_apply(l,r,bddop_or);
}

BDD bdd_xor(BDD l, BDD r) {
  return bdd_apply(l,r,bddop_xor);
}

BDD bdd_imp(BDD l, BDD r) {
  return bdd_apply(l,r,bddop_imp);
}

BDD bdd_biimp(BDD l, BDD r) {
  return bdd_apply(l,r,bddop_biimp);
}

/*=== ITE ===*/
BDD bdd_ite(BDD f, BDD g, BDD h) {
   BDD res;
   firstReorder = 1;
   
   CHECKa(f, bddfalse);
   CHECKa(g, bddfalse);
   CHECKa(h, bddfalse);

 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = ite_rec(f,g,h);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD ite_rec(BDD f, BDD g, BDD h) {
  BddCacheData *entry;
  BDD res;

  if (ISONE(f))
    return g;
  if (ISZERO(f))
    return h;
  if (g == h)
    return g;
  if (ISONE(g) && ISZERO(h))
    return f;
  if (ISZERO(g) && ISONE(h))
    return not_rec(f);

  entry = BddCache_lookup(&itecache, ITEHASH(f,g,h));
  if (entry->a == f  &&  entry->b == g  &&  entry->c == h) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif
    
  if (LEVEL(f) == LEVEL(g)) {
    if (LEVEL(f) == LEVEL(h)) {
      PUSHREF( ite_rec(LOW(f), LOW(g), LOW(h)) );
      PUSHREF( ite_rec(HIGH(f), HIGH(g), HIGH(h)) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    }
    else
    if (LEVEL(f) < LEVEL(h)) {
      PUSHREF( ite_rec(LOW(f), LOW(g), h) );
      PUSHREF( ite_rec(HIGH(f), HIGH(g), h) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    }
    else {/* f > h */
      PUSHREF( ite_rec(f, g, LOW(h)) );
      PUSHREF( ite_rec(f, g, HIGH(h)) );
      res = bdd_makenode(LEVEL(h), READREF(2), READREF(1));
    }
  }
  else
  if (LEVEL(f) < LEVEL(g)) {
    if (LEVEL(f) == LEVEL(h)) {
      PUSHREF( ite_rec(LOW(f), g, LOW(h)) );
      PUSHREF( ite_rec(HIGH(f), g, HIGH(h)) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    }
    else
    if (LEVEL(f) < LEVEL(h)) {
      PUSHREF( ite_rec(LOW(f), g, h) );
      PUSHREF( ite_rec(HIGH(f), g, h) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    }
    else {/* f > h */
      PUSHREF( ite_rec(f, g, LOW(h)) );
      PUSHREF( ite_rec(f, g, HIGH(h)) );
      res = bdd_makenode(LEVEL(h), READREF(2), READREF(1));
    }
  }
  else {/* f > g */
    if (LEVEL(g) == LEVEL(h)) {
      PUSHREF( ite_rec(f, LOW(g), LOW(h)) );
      PUSHREF( ite_rec(f, HIGH(g), HIGH(h)) );
      res = bdd_makenode(LEVEL(g), READREF(2), READREF(1));
    }
    else
    if (LEVEL(g) < LEVEL(h)) {
      PUSHREF( ite_rec(f, LOW(g), h) );
      PUSHREF( ite_rec(f, HIGH(g), h) );
      res = bdd_makenode(LEVEL(g), READREF(2), READREF(1));
    }
    else {/* g > h */ 
      PUSHREF( ite_rec(f, g, LOW(h)) );
      PUSHREF( ite_rec(f, g, HIGH(h)) );
      res = bdd_makenode(LEVEL(h), READREF(2), READREF(1));
    }
  }

  POPREF(2);

  entry->a = f;
  entry->b = g;
  entry->c = h;
  entry->r.res = res;

  return res;
}

/*=== RESTRICT ===*/
BDD bdd_restrict(BDD r, BDD var) {
   BDD res;
   firstReorder = 1;
   
   CHECKa(r,bddfalse);
   CHECKa(var,bddfalse);
   
   if (var < 2)  /* Empty set */
      return r;
   
 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2svartable(var) < 0)
	 return bddfalse;

      INITREF;
      miscid = (var << 3) | CACHEID_RESTRICT;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = restrict_rec(r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static int restrict_rec(int r) {
  BddCacheData *entry;
  int res;

  if (ISCONST(r)  ||  LEVEL(r) > quantlast)
    return r;

  entry = BddCache_lookup(&misccache, RESTRHASH(r,miscid));
  if (entry->a == r  &&  entry->c == miscid) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  if (INSVARSET(LEVEL(r))) {
    if (quantvarset[LEVEL(r)] > 0)
  res = restrict_rec(HIGH(r));
    else
  res = restrict_rec(LOW(r));
  }
  else {
    PUSHREF( restrict_rec(LOW(r)) );
    PUSHREF( restrict_rec(HIGH(r)) );
    res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
    POPREF(2);
  }

  entry->a = r;
  entry->c = miscid;
  entry->r.res = res;

  return res;
}

/*=== GENERALIZED COFACTOR ===*/
BDD bdd_constrain(BDD f, BDD c) {
   BDD res;
   firstReorder = 1;
   
   CHECKa(f,bddfalse);
   CHECKa(c,bddfalse);
   
 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      miscid = CACHEID_CONSTRAIN;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = constrain_rec(f, c);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD constrain_rec(BDD f, BDD c) {
  BddCacheData *entry;
  BDD res;

  if (ISONE(c))
    return f;
  if (ISCONST(f))
    return f;
  if (c == f)
    return BDDONE;
  if (ISZERO(c))
    return BDDZERO;

  entry = BddCache_lookup(&misccache, CONSTRAINHASH(f,c));
  if (entry->a == f  &&  entry->b == c  &&  entry->c == miscid) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  if (LEVEL(f) == LEVEL(c)) {
    if (ISZERO(LOW(c)))
      res = constrain_rec(HIGH(f), HIGH(c));
    else if (ISZERO(HIGH(c)))
      res = constrain_rec(LOW(f), LOW(c));
    else {
      PUSHREF( constrain_rec(LOW(f), LOW(c)) );
      PUSHREF( constrain_rec(HIGH(f), HIGH(c)) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
      POPREF(2);
    }
  }
  else
  if (LEVEL(f) < LEVEL(c)) {
    PUSHREF( constrain_rec(LOW(f), c) );
    PUSHREF( constrain_rec(HIGH(f), c) );
    res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    POPREF(2);
  }
  else
  {
    if (ISZERO(LOW(c)))
      res = constrain_rec(f, HIGH(c));
    else if (ISZERO(HIGH(c)))
      res = constrain_rec(f, LOW(c));
    else {
      PUSHREF( constrain_rec(f, LOW(c)) );
      PUSHREF( constrain_rec(f, HIGH(c)) );
      res = bdd_makenode(LEVEL(c), READREF(2), READREF(1));
      POPREF(2);
    }
  }

  entry->a = f;
  entry->b = c;
  entry->c = miscid;
  entry->r.res = res;

  return res;
}

/*=== REPLACE ===*/
BDD bdd_replace(BDD r, bddPair *pair)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(r, bddfalse);
   
 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      replacepair = pair->result;
      replacelast = pair->last;
      replaceid = (pair->id << 2) | CACHEID_REPLACE;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = replace_rec(r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD replace_rec(BDD r) {
  BddCacheData *entry;
  BDD res;

  if (ISCONST(r)  ||  LEVEL(r) > replacelast)
    return r;

  entry = BddCache_lookup(&replacecache, REPLACEHASH(r));
  if (entry->a == r  &&  entry->c == replaceid) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  PUSHREF( replace_rec(LOW(r)) );
  PUSHREF( replace_rec(HIGH(r)) );

  res = bdd_correctify(LEVEL(replacepair[LEVEL(r)]), READREF(2), READREF(1));
  POPREF(2);

  entry->a = r;
  entry->c = replaceid;
  entry->r.res = res;

  return res;
}

static BDD bdd_correctify(int level, BDD l, BDD r) {
  BDD res;

  if (level < LEVEL(l)  &&  level < LEVEL(r))
    return bdd_makenode(level, l, r);
  if (level == LEVEL(l)  ||  level == LEVEL(r)) {
    bdd_error(BDD_REPLACE);
    return 0;
  }
  if (LEVEL(l) == LEVEL(r)) {
    PUSHREF( bdd_correctify(level, LOW(l), LOW(r)) );
    PUSHREF( bdd_correctify(level, HIGH(l), HIGH(r)) );
    res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
  }
  else
  if (LEVEL(l) < LEVEL(r)) {
    PUSHREF( bdd_correctify(level, LOW(l), r) );
    PUSHREF( bdd_correctify(level, HIGH(l), r) );
    res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
  }
  else {
    PUSHREF( bdd_correctify(level, l, LOW(r)) );
    PUSHREF( bdd_correctify(level, l, HIGH(r)) );
    res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
  }
  POPREF(2);

  return res; /* FIXME: cache ? */
}

/*=== COMPOSE ===*/
BDD bdd_compose(BDD f, BDD g, int var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(f, bddfalse);
   CHECKa(g, bddfalse);
   if (var < 0 || var >= bddvarnum)
   {
      bdd_error(BDD_VAR);
      return bddfalse;
   }
   
 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      composelevel = bddvar2level[var];
      replaceid = (composelevel << 2) | CACHEID_COMPOSE;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = compose_rec(f, g);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD compose_rec(BDD f, BDD g) {
  BddCacheData *entry;
  BDD res;

  if (LEVEL(f) > composelevel)
    return f;

  entry = BddCache_lookup(&replacecache, COMPOSEHASH(f,g));
  if (entry->a == f  &&  entry->b == g  &&  entry->c == replaceid) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  if (LEVEL(f) < composelevel) {
    if (LEVEL(f) == LEVEL(g)) {
      PUSHREF( compose_rec(LOW(f), LOW(g)) );
      PUSHREF( compose_rec(HIGH(f), HIGH(g)) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    }
    else
    if (LEVEL(f) < LEVEL(g)) {
      PUSHREF( compose_rec(LOW(f), g) );
      PUSHREF( compose_rec(HIGH(f), g) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    }
    else {
      PUSHREF( compose_rec(f, LOW(g)) );
      PUSHREF( compose_rec(f, HIGH(g)) );
      res = bdd_makenode(LEVEL(g), READREF(2), READREF(1));
    }
    POPREF(2);
  }
  else {
    /*if (LEVEL(f) == composelevel) changed 2-nov-98 */
    res = ite_rec(g, HIGH(f), LOW(f));
  }

  entry->a = f;
  entry->b = g;
  entry->c = replaceid;
  entry->r.res = res;

  return res;
}

BDD bdd_veccompose(BDD f, bddPair *pair)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(f, bddfalse);
   
 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      replacepair = pair->result;
      replaceid = (pair->id << 2) | CACHEID_VECCOMPOSE;
      replacelast = pair->last;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = veccompose_rec(f);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD veccompose_rec(BDD f) {
  BddCacheData *entry;
  register BDD res;

  if (LEVEL(f) > replacelast)
    return f;

  entry = BddCache_lookup(&replacecache, VECCOMPOSEHASH(f));
  if (entry->a == f  &&  entry->c == replaceid) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  PUSHREF( veccompose_rec(LOW(f)) );
  PUSHREF( veccompose_rec(HIGH(f)) );
  res = ite_rec(replacepair[LEVEL(f)], READREF(1), READREF(2));
  POPREF(2);

  entry->a = f;
  entry->c = replaceid;
  entry->r.res = res;

  return res;
}

/*=== SIMPLIFY ===*/
BDD bdd_simplify(BDD f, BDD d)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(f, bddfalse);
   CHECKa(d, bddfalse);
   
 again:
   if (setjmp(bddexception) == 0)
   {
      INITREF;
      applyop = bddop_or;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = simplify_rec(f, d);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static BDD simplify_rec(BDD f, BDD d) {
  BddCacheData *entry;
  BDD res;

  if (ISONE(d)  ||  ISCONST(f))
    return f;
  if (d == f)
    return BDDONE;
  if (ISZERO(d))
    return BDDZERO;

  entry = BddCache_lookup(&applycache, APPLYHASH(f,d,bddop_simplify));

  if (entry->a == f  &&  entry->b == d  &&  entry->c == bddop_simplify) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  if (LEVEL(f) == LEVEL(d))
  {
    if (ISZERO(LOW(d)))
      res = simplify_rec(HIGH(f), HIGH(d));
    else
    if (ISZERO(HIGH(d)))
      res = simplify_rec(LOW(f), LOW(d));
    else {
      PUSHREF( simplify_rec(LOW(f),	LOW(d)) );
      PUSHREF( simplify_rec(HIGH(f), HIGH(d)) );
      res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
      POPREF(2);
    }
  }
  else
  if (LEVEL(f) < LEVEL(d)) {
    PUSHREF( simplify_rec(LOW(f), d) );
    PUSHREF( simplify_rec(HIGH(f), d) );
    res = bdd_makenode(LEVEL(f), READREF(2), READREF(1));
    POPREF(2);
  }
  else {/* LEVEL(d) < LEVEL(f) */
    PUSHREF( apply_rec(LOW(d), HIGH(d)) ); /* Exist quant */
    res = simplify_rec(f, READREF(1));
    POPREF(1);
  }

  entry->a = f;
  entry->b = d;
  entry->c = bddop_simplify;
  entry->r.res = res;

  return res;
}

/*=== QUANTIFICATION ===*/
BDD bdd_exist(BDD r, BDD var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(r, bddfalse);
   CHECKa(var, bddfalse);
   
   if (var < 2)  /* Empty set */
      return r;

 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2vartable(var) < 0)
	 return bddfalse;

      INITREF;
      quantid = (var << 3) | CACHEID_EXIST; /* FIXME: range */
      applyop = bddop_or;

      if (!firstReorder)
	 bdd_disable_reorder();
      res = quant_rec(r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

BDD bdd_forall(BDD r, BDD var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(r, bddfalse);
   CHECKa(var, bddfalse);
   
   if (var < 2)  /* Empty set */
      return r;

 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2vartable(var) < 0)
	 return bddfalse;

      INITREF;
      quantid = (var << 3) | CACHEID_FORALL;
      applyop = bddop_and;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = quant_rec(r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

BDD bdd_unique(BDD r, BDD var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(r, bddfalse);
   CHECKa(var, bddfalse);
   
   if (var < 2)  /* Empty set */
      return r;

 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2vartable(var) < 0)
	 return bddfalse;

      INITREF;
      quantid = (var << 3) | CACHEID_UNIQUE;
      applyop = bddop_xor;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = quant_rec(r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static int quant_rec(int r) {
  BddCacheData *entry;
  int res;
  if (r < 2  ||  LEVEL(r) > quantlast)
    return r;

  entry = BddCache_lookup(&quantcache, QUANTHASH(r));
  if (entry->a == r  &&  entry->c == quantid) {
    #ifdef CACHESTATS
    bddcachestats.opHit++;
    #endif
    return entry->r.res;
  }
  #ifdef CACHESTATS
  bddcachestats.opMiss++;
  #endif

  PUSHREF( quant_rec(LOW(r)) );
  PUSHREF( quant_rec(HIGH(r)) );

  if (INVARSET(LEVEL(r)))
    res = apply_rec(READREF(2), READREF(1));
  else
    res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));

  POPREF(2);

  entry->a = r;
  entry->c = quantid;
  entry->r.res = res;

  return res;
}

/*=== APPLY & QUANTIFY ===*/
BDD bdd_appex(BDD l, BDD r, int opr, BDD var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(l, bddfalse);
   CHECKa(r, bddfalse);
   CHECKa(var, bddfalse);
   
   if (opr<0 || opr>bddop_invimp)
   {
      bdd_error(BDD_OP);
      return bddfalse;
   }
   
   if (var < 2)  /* Empty set */
      return bdd_apply(l,r,opr);

 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2vartable(var) < 0)
	 return bddfalse;
   
      INITREF;
      applyop = bddop_or;
      appexop = opr;
      appexid = (var << 5) | (appexop << 1); /* FIXME: range! */
      quantid = (appexid << 3) | CACHEID_APPEX;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = appquant_rec(l, r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }
   
   checkresize();
   return res;
}

BDD bdd_appall(BDD l, BDD r, int opr, BDD var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(l, bddfalse);
   CHECKa(r, bddfalse);
   CHECKa(var, bddfalse);
   
   if (opr<0 || opr>bddop_invimp)
   {
      bdd_error(BDD_OP);
      return bddfalse;
   }
   
   if (var < 2)  /* Empty set */
      return bdd_apply(l,r,opr);

 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2vartable(var) < 0)
	 return bddfalse;

      INITREF;
      applyop = bddop_and;
      appexop = opr;
      appexid = (var << 5) | (appexop << 1) | 1; /* FIXME: range! */
      quantid = (appexid << 3) | CACHEID_APPAL;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = appquant_rec(l, r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

BDD bdd_appuni(BDD l, BDD r, int opr, BDD var)
{
   BDD res;
   firstReorder = 1;
   
   CHECKa(l, bddfalse);
   CHECKa(r, bddfalse);
   CHECKa(var, bddfalse);
   
   if (opr<0 || opr>bddop_invimp)
   {
      bdd_error(BDD_OP);
      return bddfalse;
   }
   
   if (var < 2)  /* Empty set */
      return bdd_apply(l,r,opr);

 again:
   if (setjmp(bddexception) == 0)
   {
      if (varset2vartable(var) < 0)
	 return bddfalse;

      INITREF;
      applyop = bddop_xor;
      appexop = opr;
      appexid = (var << 5) | (appexop << 1) | 1; /* FIXME: range! */
      quantid = (appexid << 3) | CACHEID_APPUN;
      
      if (!firstReorder)
	 bdd_disable_reorder();
      res = appquant_rec(l, r);
      if (!firstReorder)
	 bdd_enable_reorder();
   }
   else
   {
      bdd_checkreorder();

      if (firstReorder-- == 1)
	 goto again;
      res = BDDZERO;  /* avoid warning about res being uninitialized */
   }

   checkresize();
   return res;
}

static int appquant_rec(int l, int r) {
  BddCacheData *entry;
  int res;

  switch (appexop) {
    case bddop_and:
       if (l == 0  ||  r == 0)
    return 0;
       if (l == r)
    return quant_rec(l);
       if (l == 1)
    return quant_rec(r);
       if (r == 1)
    return quant_rec(l);
       break;
    case bddop_or:
       if (l == 1  ||  r == 1)
    return 1;
       if (l == r)
    return quant_rec(l);
       if (l == 0)
    return quant_rec(r);
       if (r == 0)
    return quant_rec(l);
       break;
    case bddop_xor:
       if (l == r)
    return 0;
       if (l == 0)
    return quant_rec(r);
       if (r == 0)
    return quant_rec(l);
       break;
    case bddop_nand:
       if (l == 0  ||  r == 0)
    return 1;
       break;
    case bddop_nor:
       if (l == 1  ||  r == 1)
    return 0;
       break;
  }

  if (ISCONST(l)  &&  ISCONST(r))
    res = oprres[appexop][(l<<1) | r];
  else
  if (LEVEL(l) > quantlast  &&  LEVEL(r) > quantlast) {
    int oldop = applyop;
    applyop = appexop;
    res = apply_rec(l,r);
    applyop = oldop;
  }
  else {
    entry = BddCache_lookup(&appexcache, APPEXHASH(l,r,appexop));
    if (entry->a == l  &&  entry->b == r  &&  entry->c == appexid) {
      #ifdef CACHESTATS
      bddcachestats.opHit++;
      #endif
      return entry->r.res;
    }
    #ifdef CACHESTATS
    bddcachestats.opMiss++;
    #endif

    if (LEVEL(l) == LEVEL(r)) {
      PUSHREF( appquant_rec(LOW(l), LOW(r)) );
      PUSHREF( appquant_rec(HIGH(l), HIGH(r)) );
      if (INVARSET(LEVEL(l)))
        res = apply_rec(READREF(2), READREF(1));
      else
      res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
    }
    else
    if (LEVEL(l) < LEVEL(r)) {
      PUSHREF( appquant_rec(LOW(l), r) );
      PUSHREF( appquant_rec(HIGH(l), r) );
      if (INVARSET(LEVEL(l)))
        res = apply_rec(READREF(2), READREF(1));
      else
        res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
    }
    else {
      PUSHREF( appquant_rec(l, LOW(r)) );
      PUSHREF( appquant_rec(l, HIGH(r)) );
      if (INVARSET(LEVEL(r)))
        res = apply_rec(READREF(2), READREF(1));
      else
        res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
    }

    POPREF(2);
    
    entry->a = l;
    entry->b = r;
    entry->c = appexid;
    entry->r.res = res;
  }

  return res;
}

/*=== SUPPORT ===*/
BDD bdd_support(BDD r) {
  static int  supportSize = 0;
  int n;
  int res=1;
  CHECKa(r, bddfalse);

  if (r < 2)
    return bddfalse;
  /* On-demand allocation of support set */
  if (supportSize < bddvarnum) {
    if ((supportSet=(int*)malloc(bddvarnum*sizeof(int))) == NULL) {
      bdd_error(BDD_MEMORY);
      return bddfalse;
    }
    memset(supportSet, 0, bddvarnum*sizeof(int));
    supportSize = bddvarnum;
    supportID = 0;
  }

  /* Update global variables used to speed up bdd_support()
   * - instead of always memsetting support to zero, we use
   *   a change counter.
   * - and instead of reading the whole array afterwards, we just
   *   look from 'min' to 'max' used BDD variables.
   */
  if (supportID == 0x0FFFFFFF) {
    /* We probably don't get here -- but let's just be sure */
    memset(supportSet, 0, bddvarnum*sizeof(int));
    supportID = 0;
  }
  ++supportID;
  supportMin = LEVEL(r);
  supportMax = supportMin;

  support_rec(r, supportSet);
  bdd_unmark(r);
  bdd_disable_reorder();

  for (n=supportMax ; n>=supportMin ; --n)
    if (supportSet[n] == supportID) {
      register BDD tmp;
      bdd_addref(res);
      tmp = bdd_makenode(n, 0, res);
      bdd_delref(res);
      res = tmp;
    }
  bdd_enable_reorder();
  return res;
}

static void support_rec(int r, int* support) {
  BddNode *node;

  if (r < 2)
    return;

  node = &bddnodes[r];
  if ((node)->level & MARKON  ||  LOWp(node) == -1)
    return;

  support[(node)->level] = supportID;

  if ((node)->level > supportMax)
    supportMax = (node)->level;

  (node)->level |= MARKON;

  support_rec(LOWp(node), support);
  support_rec(HIGHp(node), support);
}

/*=== ONE SATISFYING VARIABLE ASSIGNMENT ===*/
BDD bdd_satone(BDD r) {
  BDD res;

  CHECKa(r, bddfalse);
  if (r < 2)
    return r;

  bdd_disable_reorder();

  INITREF;
  res = satone_rec(r);

  bdd_enable_reorder();

  checkresize();
  return res;
}

static BDD satone_rec(BDD r) {
  if (ISCONST(r))
    return r;

  if (ISZERO(LOW(r))) {
    BDD res = satone_rec(HIGH(r));
    return PUSHREF( bdd_makenode(LEVEL(r), BDDZERO, res) );
  }
  else {
    BDD res = satone_rec(LOW(r));
    return PUSHREF( bdd_makenode(LEVEL(r), res, BDDZERO) );
  }
}

BDD bdd_satoneset(BDD r, BDD var, BDD pol) {
  BDD res;
  CHECKa(r, bddfalse);
  if (ISZERO(r))
    return r;
  if (!ISCONST(pol)) {
    bdd_error(BDD_ILLBDD);
    return bddfalse;
  }

  bdd_disable_reorder();

  INITREF;
  satPolarity = pol;
  res = satoneset_rec(r, var);

  bdd_enable_reorder();

  checkresize();
  return res;
}

static BDD satoneset_rec(BDD r, BDD var) {
  if (ISCONST(r)  &&  ISCONST(var))
    return r;

  if (LEVEL(r) < LEVEL(var)) {
    if (ISZERO(LOW(r))) {
      BDD res = satoneset_rec(HIGH(r), var);
      return PUSHREF( bdd_makenode(LEVEL(r), BDDZERO, res) );
    }
    else {
      BDD res = satoneset_rec(LOW(r), var);
      return PUSHREF( bdd_makenode(LEVEL(r), res, BDDZERO) );
    }
  }
  else if (LEVEL(var) < LEVEL(r)) {
    BDD res = satoneset_rec(r, HIGH(var));
    if (satPolarity == BDDONE)
      return PUSHREF( bdd_makenode(LEVEL(var), BDDZERO, res) );
    else
      return PUSHREF( bdd_makenode(LEVEL(var), res, BDDZERO) );
  }
  else {/* LEVEL(r) == LEVEL(var) */
    if (ISZERO(LOW(r))) {
      BDD res = satoneset_rec(HIGH(r), HIGH(var));
      return PUSHREF( bdd_makenode(LEVEL(r), BDDZERO, res) );
    }
    else {
      BDD res = satoneset_rec(LOW(r), HIGH(var));
      return PUSHREF( bdd_makenode(LEVEL(r), res, BDDZERO) );
    }
  }
}

/*=== EXACTLY ONE SATISFYING VARIABLE ASSIGNMENT ===*/
BDD bdd_fullsatone(BDD r) {
  BDD res;
  int v;

  CHECKa(r, bddfalse);
  if (r == 0)
    return 0;

  bdd_disable_reorder();

  INITREF;
  res = fullsatone_rec(r);

  for (v=LEVEL(r)-1 ; v>=0 ; v--) {
    res = PUSHREF( bdd_makenode(v, res, 0) );
  }

  bdd_enable_reorder();

  checkresize();
  return res;
}

static int fullsatone_rec(int r) {
  if (r < 2)
    return r;

  if (LOW(r) != 0) {
    int res = fullsatone_rec(LOW(r));
    int v;
    for (v=LEVEL(LOW(r))-1 ; v>LEVEL(r) ; v--) {
      res = PUSHREF( bdd_makenode(v, res, 0) );
    }
    return PUSHREF( bdd_makenode(LEVEL(r), res, 0) );
  }
  else {
    int res = fullsatone_rec(HIGH(r));
    int v;
    
    for (v=LEVEL(HIGH(r))-1 ; v>LEVEL(r) ; v--) {
      res = PUSHREF( bdd_makenode(v, res, 0) );
    }

    return PUSHREF( bdd_makenode(LEVEL(r), 0, res) );
  }
}

/*=== ALL SATISFYING VARIABLE ASSIGNMENTS ===*/
void bdd_allsat(BDD r, bddallsathandler handler)
{
   int v;
  
   CHECKn(r);

   if ((allsatProfile=(char*)malloc(bddvarnum)) == NULL)
   {
      bdd_error(BDD_MEMORY);
      return;
   }

   for (v=LEVEL(r)-1 ; v>=0 ; --v)
     allsatProfile[bddlevel2var[v]] = -1;
   
   allsatHandler = handler;
   INITREF;
   
   allsat_rec(r);

   free(allsatProfile);
}

static void allsat_rec(BDD r)
{
   if (ISONE(r))
   {
      allsatHandler(allsatProfile, bddvarnum);
      return;
   }
  
   if (ISZERO(r))
      return;
   
   if (!ISZERO(LOW(r)))
   {
      int v;

      allsatProfile[bddlevel2var[LEVEL(r)]] = 0;
	 
      for (v=LEVEL(LOW(r))-1 ; v>LEVEL(r) ; --v)
      {
	 allsatProfile[bddlevel2var[v]] = -1;
      }
      
      allsat_rec(LOW(r));
   }
   
   if (!ISZERO(HIGH(r)))
   {
      int v;

      allsatProfile[bddlevel2var[LEVEL(r)]] = 1;
	 
      for (v=LEVEL(HIGH(r))-1 ; v>LEVEL(r) ; --v)
      {
	 allsatProfile[bddlevel2var[v]] = -1;
      }
      
      allsat_rec(HIGH(r));
   }
}

/*=== COUNT NUMBER OF SATISFYING ASSIGNMENT ===*/
double bdd_satcount(BDD r) {
  double size=1;

  CHECKa(r, 0.0);

  miscid = CACHEID_SATCOU;
  size = pow(2.0, (double)LEVEL(r));

  return size * satcount_rec(r);
}

double bdd_satcountset(BDD r, BDD varset) {
  double unused = bddvarnum;
  BDD n;

  if (ISCONST(varset)  ||  ISZERO(r)) /* empty set */
    return 0.0;

  for (n=varset ; !ISCONST(n) ; n=HIGH(n))
    unused--;

  unused = bdd_satcount(r) / pow(2.0,unused);

  return unused >= 1.0 ? unused : 1.0;
}

static double satcount_rec(int root) {
  BddCacheData *entry;
  BddNode *node;
  double size, s;

  if (root < 2)
    return root;

  entry = BddCache_lookup(&misccache, SATCOUHASH(root));
  if (entry->a == root  &&  entry->c == miscid)
    return entry->r.dres;

  node = &bddnodes[root];
  size = 0;
  s = 1;

  s *= pow(2.0, (float)(bddnodes[LOWp(node)].level - (node)->level - 1));
  size += s * satcount_rec(LOWp(node));

  s = 1;
  s *= pow(2.0, (float)(bddnodes[HIGHp(node)].level - (node)->level - 1));
  size += s * satcount_rec(HIGHp(node));

  entry->a = root;
  entry->c = miscid;
  entry->r.dres = size;

  return size;
}

double bdd_satcountln(BDD r) {
  double size;

  CHECKa(r, 0.0);

  miscid = CACHEID_SATCOULN;
  size = satcountln_rec(r);

  if (size >= 0.0)
    size += LEVEL(r);

  return size;
}

double bdd_satcountlnset(BDD r, BDD varset) {
  double unused = bddvarnum;
  BDD n;

  if (ISCONST(varset)) /* empty set */
    return 0.0;

  for (n=varset ; !ISCONST(n) ; n=HIGH(n))
    unused--;

  unused = bdd_satcountln(r) - unused;

  return unused >= 0.0 ? unused : 0.0;
}

static double satcountln_rec(int root) {
  BddCacheData *entry;
  BddNode *node;
  double size, s1,s2;

  if (root == 0)
    return -1.0;
  if (root == 1)
    return 0.0;

  entry = BddCache_lookup(&misccache, SATCOUHASH(root));
  if (entry->a == root  &&  entry->c == miscid)
    return entry->r.dres;

  node = &bddnodes[root];

  s1 = satcountln_rec(LOWp(node));
  if (s1 >= 0.0)
    s1 += bddnodes[LOWp(node)].level - (node)->level - 1;

  s2 = satcountln_rec(HIGHp(node));
  if (s2 >= 0.0)
    s2 += bddnodes[HIGHp(node)].level - (node)->level - 1;

  if (s1 < 0.0)
    size = s2;
  else if (s2 < 0.0)
    size = s1;
  else if (s1 < s2)
    size = s2 + log1p(pow(2.0,s1-s2)) / M_LN2;
  else
    size = s1 + log1p(pow(2.0,s2-s1)) / M_LN2;

  entry->a = root;
  entry->c = miscid;
  entry->r.dres = size;

  return size;
}

/*=== COUNT NUMBER OF ALLOCATED NODES ===*/
int bdd_nodecount(BDD r) {
  int num=0;
  test_counter = 0;
  CHECK(r);

  bdd_markcount(r, &num);
  printf("the valuenode has %d\n", test_counter);
  bdd_unmark(r);

  return num;
}

int bdd_anodecount(BDD *r, int num) {
  int n;
  int cou=0;

  for (n=0 ; n<num ; n++)
    bdd_markcount(r[n], &cou);

  for (n=0 ; n<num ; n++)
    bdd_unmark(r[n]);

  return cou;
}

/*=== NODE PROFILE ===*/
int *bdd_varprofile(BDD r) {
  CHECKa(r, NULL);

  if ((varprofile=(int*)malloc(sizeof(int)*bddvarnum)) == NULL) {
    bdd_error(BDD_MEMORY);
    return NULL;
  }

  memset(varprofile, 0, sizeof(int)*bddvarnum);
  varprofile_rec(r);
  bdd_unmark(r);
  return varprofile;
}

static void varprofile_rec(int r) {
  BddNode *node;

  if (r < 2)
    return;

  node = &bddnodes[r];
  if ((node)->level & MARKON)
    return;

  varprofile[bddlevel2var[(node)->level]]++;
  (node)->level |= MARKON;

  varprofile_rec(LOWp(node));
  varprofile_rec(HIGHp(node));
}

/*=== COUNT NUMBER OF PATHS ===*/
double bdd_pathcount(BDD r) {
  CHECKa(r, 0.0);

  miscid = CACHEID_PATHCOU;

  return bdd_pathcount_rec(r);
}

static double bdd_pathcount_rec(BDD r) {
  BddCacheData *entry;
  double size;

  if (ISZERO(r))
    return 0.0;
  if (ISONE(r))
    return 1.0;

  entry = BddCache_lookup(&misccache, PATHCOUHASH(r));
  if (entry->a == r  &&  entry->c == miscid)
    return entry->r.dres;

  size = bdd_pathcount_rec(LOW(r)) + bdd_pathcount_rec(HIGH(r));

  entry->a = r;
  entry->c = miscid;
  entry->r.dres = size;

  return size;
}

static int varset2vartable(BDD r) {
  BDD n;

  if (r < 2)
    return bdd_error(BDD_VARSET);

  quantvarsetID++;

  if (quantvarsetID == INT_MAX) {
    memset(quantvarset, 0, sizeof(int)*bddvarnum);
    quantvarsetID = 1;
  }

  for (n=r ; n > 1 ; n=HIGH(n)) {
    quantvarset[bddnodes[n].level] = quantvarsetID;
    quantlast = bddnodes[n].level;
  }

  return 0;
}

static int varset2svartable(BDD r) {
  BDD n;

  if (r < 2)
    return bdd_error(BDD_VARSET);

  quantvarsetID++;

  if (quantvarsetID == INT_MAX/2) {
    memset(quantvarset, 0, sizeof(int)*bddvarnum);
    quantvarsetID = 1;
  }

  for (n=r ; !ISCONST(n) ; ) {
    if (ISZERO(LOW(n))) {
      quantvarset[bddnodes[n].level] = quantvarsetID;
      n = HIGH(n);
    }
    else {
      quantvarset[bddnodes[n].level] = -quantvarsetID;
      n = LOW(n);
    }
    quantlast = bddnodes[n].level;
  }

  return 0;
}

/* In file reorder.c */
/*=====================================================================================================*/
/* Change macros to reflect the above idea */
#define VAR(n) (bddnodes[n].level)
#define VARp(p) (p->level)

   /* Avoid these - they are misleading! */
#undef LEVEL
#undef LEVELp

#define __USERESIZE /* FIXME */

   /* Current auto reord. method and number of automatic reorderings left */
static int bddreordermethod;
static int bddreordertimes;

   /* Flag for disabling reordering temporarily */
static int reorderdisabled;

   /* Store for the variable relationships */
static BddTree *vartree;
static int blockid;

   /* Store for the ref.cou. of the external roots */
static int *extroots;
static int extrootsize;

/* Level data */
typedef struct _levelData {
   int start;    /* Start of this sub-table (entry in "bddnodes") */
   int size;     /* Size of this sub-table */
   int maxsize;  /* Max. allowed size of sub-table */
   int nodenum;  /* Number of nodes in this level */
} levelData;

static levelData *levels; /* Indexed by variable! */

   /* Interaction matrix */
static imatrix *iactmtx;

   /* Reordering information for the user */
static int verbose;
static bddinthandler reorder_handler;
static bddfilehandler reorder_filehandler;
static bddsizehandler reorder_nodenum;

   /* Number of live nodes before and after a reordering session */
static int usednum_before;
static int usednum_after;
	    
   /* Kernel variables needed for reordering */
extern int bddfreepos;
extern int mtbddfreepos;
extern int bddfreenum;
extern int mtbddfreepos;
extern long int bddproduced;

/* Flag telling us when a node table resize is done */
static int resizedInMakenode;

/* New node hashing function for use with reordering */
#define NODEHASH(var,l,h) ((PAIR((l),(h))%levels[var].size)+levels[var].start)

/* Reordering prototypes */
static void blockdown(BddTree *);
static void addref_rec(int, char *);
static void reorder_gbc();
static void reorder_setLevellookup(void);
static int  reorder_makenode(int, int, int);
static int  reorder_varup(int);
static int  reorder_vardown(int);
static int  reorder_init(void);
static void reorder_done(void);

#define random(a) (rand() % (a))

   /* For sorting the blocks according to some specific size value */
typedef struct s_sizePair {
  int val;
  BddTree *block;
} sizePair;

void bdd_reorder_init(void) {
  reorderdisabled = 0;
  vartree = NULL;

  bdd_clrvarblocks();
  bdd_reorder_hook(bdd_default_reohandler);
  bdd_reorder_verbose(0);
  bdd_autoreorder_times(BDD_REORDER_NONE, 0);
  reorder_nodenum = bdd_getnodenum;
  usednum_before = usednum_after = 0;
  blockid = 0;
}

void bdd_reorder_done(void) {
  bddtree_del(vartree);
  bdd_operator_reset();
  vartree = NULL;
}

/*=== Reorder using a sliding window of size 2 ===*/
static BddTree *reorder_win2(BddTree *t) {
  BddTree *this=t, *first=t;

  if (t == NULL)
    return t;

  if (verbose > 1)
    printf("Win2 start: %d nodes\n", reorder_nodenum());
  fflush(stdout);

  while (this->next != NULL) {
    int best = reorder_nodenum();
    blockdown(this);
    
    if (best < reorder_nodenum()) {
      blockdown(this->prev);
      this = this->next;
    }
    else
    if (first == this)
      first = this->prev;

    if (verbose > 1) {
      printf(".");
      fflush(stdout);
    }
  }

  if (verbose > 1)
    printf("\nWin2 end: %d nodes\n", reorder_nodenum());
  fflush(stdout);

  return first;
}

static BddTree *reorder_win2ite(BddTree *t) {
  BddTree *this, *first=t;
  int lastsize;
  int c=1;

  if (t == NULL)
    return t;

  if (verbose > 1)
    printf("Win2ite start: %d nodes\n", reorder_nodenum());

  do {
    lastsize = reorder_nodenum();

    this = t;
    while (this->next != NULL) {
      int best = reorder_nodenum();

      blockdown(this);

      if (best < reorder_nodenum()) {
        blockdown(this->prev);
        this = this->next;
      }
      else
      if (first == this)
        first = this->prev;
      if (verbose > 1) {
        printf(".");
        fflush(stdout);
      }
    }

    if (verbose > 1)
      printf(" %d nodes\n", reorder_nodenum());
    c++;
  }
  while (reorder_nodenum() != lastsize);

  return first;
}

/*=== Reorder using a sliding window of size 3 ===*/
#define X(a)

static BddTree *reorder_swapwin3(BddTree *this, BddTree **first) {
  int setfirst = (this->prev == NULL ? 1 : 0);
  BddTree *next = this;
  int best = reorder_nodenum();

  if (this->next->next == NULL) {/* Only two blocks left -> win2 swap */
    blockdown(this);
    
    if (best < reorder_nodenum()) {
      blockdown(this->prev);
      next = this->next;
    }
    else {
      next = this;
      if (setfirst)
        *first = this->prev;
    }
  }
  else /* Real win3 swap */
  {
    int pos = 0;
    X(printf("%d: ", reorder_nodenum()));
    blockdown(this);  /* B A* C (4) */
    X(printf("A"));
    pos++;
    if (best > reorder_nodenum()) {
      X(printf("(%d)", reorder_nodenum()));
      pos = 0;
      best = reorder_nodenum();
    }
    
    blockdown(this);  /* B C A* (3) */
    X(printf("B"));
    pos++;
    if (best > reorder_nodenum()) {
      X(printf("(%d)", reorder_nodenum()));
      pos = 0;
      best = reorder_nodenum();
    }
    
    this = this->prev->prev;
    blockdown(this);  /* C B* A (2) */
    X(printf("C"));
    pos++;
    if (best > reorder_nodenum()) {
      X(printf("(%d)", reorder_nodenum()));
      pos = 0;
      best = reorder_nodenum();
    }
    
    blockdown(this);  /* C A B* (1) */
    X(printf("D"));
    pos++;
    if (best > reorder_nodenum()) {
      X(printf("(%d)", reorder_nodenum()));
      pos = 0;
      best = reorder_nodenum();
    }
    
    this = this->prev->prev;
    blockdown(this);  /* A C* B (0)*/
    X(printf("E"));
    pos++;
    if (best > reorder_nodenum()) {
      X(printf("(%d)", reorder_nodenum()));
      pos = 0;
      best = reorder_nodenum();
    }
    
    X(printf(" -> "));
    
    if (pos >= 1) { /* A C B -> C A* B */    
      this = this->prev;
      blockdown(this);
      next = this;
      if (setfirst)
        *first = this->prev;
      X(printf("a(%d)", reorder_nodenum()));
    }
    
    if (pos >= 2) { /* C A B -> C B A* */
      blockdown(this);
      next = this->prev;
      if (setfirst)
        *first = this->prev->prev;
      X(printf("b(%d)", reorder_nodenum()));
    }
    
    if (pos >= 3) { /* C B A -> B C* A */   
      this = this->prev->prev;
      blockdown(this);
      next = this;
      if (setfirst)
        *first = this->prev;
      X(printf("c(%d)", reorder_nodenum()));
    }
    
    if (pos >= 4) { /* B C A -> B A C* */
      blockdown(this);
      next = this->prev;
      if (setfirst)
        *first = this->prev->prev;
      X(printf("d(%d)", reorder_nodenum()));
    }
    
    if (pos >= 5) { /* B A C -> A B* C */  
      this = this->prev->prev;
      blockdown(this);
      next = this;
      if (setfirst)
        *first = this->prev;
      X(printf("e(%d)", reorder_nodenum()));
    }
    X(printf("\n"));
  }

  return next;
}

static BddTree *reorder_win3(BddTree *t) {
  BddTree *this=t, *first=t;

  if (t == NULL)
    return t;

  if (verbose > 1)
    printf("Win3 start: %d nodes\n", reorder_nodenum());
  fflush(stdout);

  while (this->next != NULL) {
    this = reorder_swapwin3(this, &first);
    
    if (verbose > 1) {
      printf(".");
      fflush(stdout);
    }
  }

  if (verbose > 1)
    printf("\nWin3 end: %d nodes\n", reorder_nodenum());
  fflush(stdout);

  return first;
}

static BddTree *reorder_win3ite(BddTree *t) {
  BddTree *this=t, *first=t;
  int lastsize;

  if (t == NULL)
    return t;

  if (verbose > 1)
    printf("Win3ite start: %d nodes\n", reorder_nodenum());

  do {
    lastsize = reorder_nodenum();
    this = first;
    
    while (this->next != NULL  &&  this->next->next != NULL) {
      this = reorder_swapwin3(this, &first);

      if (verbose > 1) {
        printf(".");
        fflush(stdout);
      }
    }

    if (verbose > 1)
      printf(" %d nodes\n", reorder_nodenum());
  }
  while (reorder_nodenum() != lastsize);
  if (verbose > 1)
    printf("Win3ite end: %d nodes\n", reorder_nodenum());

  return first;
}

/*=== Reorder by sifting ===*/
/* Move a specific block up and down in the order and place at last in
  the best position*/
static void reorder_sift_bestpos(BddTree *blk, int middlePos) {
  int best = reorder_nodenum();
  int maxAllowed;
  int bestpos = 0;
  int dirIsUp = 1;
  int n;

  if (bddmaxnodesize > 0)
    maxAllowed = MIN(best/5+best, bddmaxnodesize-bddmaxnodeincrease-2);
  else
    maxAllowed = best/5+best;

    /* Determine initial direction */
  if (blk->pos > middlePos)
    dirIsUp = 0;

    /* Move block back and forth */
  for (n=0 ; n<2 ; n++) {
    int first = 1;
    
    if (dirIsUp) {
      while (blk->prev != NULL && (reorder_nodenum() <= maxAllowed || first)) {
        first = 0;
        blockdown(blk->prev);
        bestpos--;
        
        if (verbose > 1) {
          printf("-");
          fflush(stdout);
        }
        
        if (reorder_nodenum() < best) {
          best = reorder_nodenum();
          bestpos = 0;

          if (bddmaxnodesize > 0)
            maxAllowed = MIN(best/5+best, bddmaxnodesize-bddmaxnodeincrease-2);
          else
          maxAllowed = best/5+best;
        }
      }
    }
    else {
      while (blk->next != NULL && (reorder_nodenum() <= maxAllowed || first)) {
        first = 0;
        blockdown(blk);
        bestpos++;
        
        if (verbose > 1) {
          printf("+");
          fflush(stdout);
        }
        
        if (reorder_nodenum() < best) {
          best = reorder_nodenum();
          bestpos = 0;

          if (bddmaxnodesize > 0)
          maxAllowed = MIN(best/5+best, bddmaxnodesize-bddmaxnodeincrease-2);
          else
          maxAllowed = best/5+best;
        }
      }
    }

    if (reorder_nodenum() > maxAllowed && verbose > 1) {
      printf("!");
      fflush(stdout);
    }

    dirIsUp = !dirIsUp;
  }

    /* Move to best pos */
  while (bestpos < 0) {
    blockdown(blk);
    bestpos++;
  }
  while (bestpos > 0) {
    blockdown(blk->prev);
    bestpos--;
  }
}

/* Go through all blocks in a specific sequence and find best position for each of them*/
static BddTree *reorder_sift_seq(BddTree *t, BddTree **seq, int num) {
  BddTree *this;
  int n;

  if (t == NULL)
    return t;

  for (n=0 ; n<num ; n++) {
    long c2, c1 = clock();

    if (verbose > 1) {
      printf("Sift ");
      if (reorder_filehandler)
        reorder_filehandler(stdout, seq[n]->id);
      else
        printf("%d", seq[n]->id);
      printf(": ");
    }

    reorder_sift_bestpos(seq[n], num/2);

    if (verbose > 1)
      printf("\n> %d nodes", reorder_nodenum());

    c2 = clock();
    if (verbose > 1)
      printf(" (%.1f sec)\n", (float)(c2-c1)/CLOCKS_PER_SEC);
  }

    /* Find first block */
  for (this=t ; this->prev != NULL ; this=this->prev)
    /* nil */;

  return this;
}

/* Compare function for sorting sifting sequence*/
static int siftTestCmp(const void *aa, const void *bb) {
  const sizePair *a = (sizePair*)aa;
  const sizePair *b = (sizePair*)bb;

  if (a->val < b->val)
    return -1;
  if (a->val > b->val)
    return 1;
  return 0;
}

/* Find sifting sequence based on the number of nodes at each level*/
static BddTree *reorder_sift(BddTree *t) {
  BddTree *this, **seq;
  sizePair *p;
  int n, num;

  for (this=t,num=0 ; this!=NULL ; this=this->next)
    this->pos = num++;

  if ((p=NEW(sizePair,num)) == NULL)
    return t;
  if ((seq=NEW(BddTree*,num)) == NULL) {
    free(p);
    return t;
  }

  for (this=t,n=0 ; this!=NULL ; this=this->next,n++) {
    int v;
    /* Accumulate number of nodes for each block */
    p[n].val = 0;
    for (v=this->first ; v<=this->last ; v++)
      p[n].val -= levels[v].nodenum;
    p[n].block = this;
  }

  /* Sort according to the number of nodes at each level */
  qsort(p, num, sizeof(sizePair), siftTestCmp);

  /* Create sequence */
  for (n=0 ; n<num ; n++)
    seq[n] = p[n].block;

  /* Do the sifting on this sequence */
  t = reorder_sift_seq(t, seq, num);

  free(seq);
  free(p);

  return t;
}

/* Do sifting iteratively until no more improvement can be found */
static BddTree *reorder_siftite(BddTree *t) {
  BddTree *first=t;
  int lastsize;
  int c=1;

  if (t == NULL)
    return t;

  do {
    if (verbose > 1)
      printf("Reorder %d\n", c++);
    
    lastsize = reorder_nodenum();
    first = reorder_sift(first);
  }
  while (reorder_nodenum() != lastsize);

  return first;
}

/*=== Random reordering (mostly for debugging and test ) ===*/
static BddTree *reorder_random(BddTree *t) {
  BddTree *this;
  BddTree **seq;
  int n, num=0;

  if (t == NULL)
    return t;

  for (this=t ; this!=NULL ; this=this->next)
    num++;
  seq = NEW(BddTree*,num);
  for (this=t,num=0 ; this!=NULL ; this=this->next)
    seq[num++] = this;

  for (n=0 ; n<4*num ; n++) {
    int blk = random(num);
    if (seq[blk]->next != NULL)
      blockdown(seq[blk]);
  }

    /* Find first block */
  for (this=t ; this->prev != NULL ; this=this->prev)
    /* nil */;

  free(seq);

  if (verbose)
    printf("Random order: %d nodes\n", reorder_nodenum());
  return this;
}

static void blockdown(BddTree *left) {
  BddTree *right = left->next;
  int n;
  int leftsize = left->last - left->first;
  int rightsize = right->last - right->first;
  int leftstart = bddvar2level[left->seq[0]];
  int *lseq = left->seq;
  int *rseq = right->seq;

  /* Move left past right */
  while (bddvar2level[lseq[0]] < bddvar2level[rseq[rightsize]]) {
    for (n=0 ; n<leftsize ; n++) {
      if (bddvar2level[lseq[n]]+1 != bddvar2level[lseq[n+1]]
         && bddvar2level[lseq[n]]  <  bddvar2level[rseq[rightsize]]) {
        reorder_vardown(lseq[n]);
      }
    }

    if (bddvar2level[lseq[leftsize]] < bddvar2level[rseq[rightsize]])  {
      reorder_vardown(lseq[leftsize]);
    }
  }

  /* Move right to where left started */
  while (bddvar2level[rseq[0]] > leftstart) {
    for (n=rightsize ; n>0 ; n--) {
      if (bddvar2level[rseq[n]]-1 != bddvar2level[rseq[n-1]]
         && bddvar2level[rseq[n]] > leftstart) {
        reorder_varup(rseq[n]);
      }
    }

    if (bddvar2level[rseq[0]] > leftstart)
      reorder_varup(rseq[0]);
  }

    /* Swap left and right data in the order */
  left->next = right->next;
  right->prev = left->prev;
  left->prev = right;
  right->next = left;

  if (right->prev != NULL)
    right->prev->next = right;
  if (left->next != NULL)
    left->next->prev = left;

  n = left->pos;
  left->pos = right->pos;
  right->pos = n;
}

/*=== Garbage collection for reordering ===*/
/* Note: Node may be marked */
static void addref_rec(int r, char *dep) {
  if (r < 2)
    return;

  if (bddnodes[r].refcou == 0) {
    bddfreenum--;

    /* Detect variable dependencies for the interaction matrix */
    dep[VAR(r) & MARKHIDE] = 1;

    /* Make sure the nodenum field is updated. Used in the initial GBC */
    levels[VAR(r) & MARKHIDE].nodenum++;
    
    addref_rec(LOW(r), dep);
    addref_rec(HIGH(r), dep);
  }
  else {
    int n;
    
    /* Update (from previously found) variable dependencies
    * for the interaction matrix */
    for (n=0 ; n<bddvarnum ; n++)
      dep[n] |= imatrixDepends(iactmtx, VAR(r) & MARKHIDE, n);
  }

  INCREF(r);
}

static void addDependencies(char *dep){
  int n,m;

  for (n=0 ; n<bddvarnum ; n++) {
    for (m=n ; m<bddvarnum ; m++) {
      if (dep[n]  &&  dep[m]) {
        imatrixSet(iactmtx, n,m);
        imatrixSet(iactmtx, m,n);
      }
    }
  }
}

/* Make sure all nodes are recursively reference counted and store info about
  nodes that are refcou. externally. This info is used at last to revert
  to the standard GBC mode. */
static int mark_roots(void) {
  char *dep = NEW(char,bddvarnum);
  int n;

  for (n=2,extrootsize=0 ; n<bddnodesize ; n++) {
  /* This is where we go from .level to .var!
  * - Do NOT use the LEVEL macro here. */
    bddnodes[n].level = bddlevel2var[bddnodes[n].level];
    if (bddnodes[n].refcou > 0) {
      SETMARK(n);
      extrootsize++;
    }
  }

  if ((extroots=(int*)(malloc(sizeof(int)*extrootsize))) == NULL)
    return bdd_error(BDD_MEMORY);

  iactmtx = imatrixNew(bddvarnum);

  for (n=2,extrootsize=0 ; n<bddnodesize ; n++) {
    BddNode *node = &bddnodes[n];

    if (MARKEDp(node)) {
      UNMARKp(node);
      extroots[extrootsize++] = n;

      memset(dep,0,bddvarnum);
      dep[VARp(node)] = 1;
      levels[VARp(node)].nodenum++;

      addref_rec(LOWp(node), dep);
      addref_rec(HIGHp(node), dep);

      addDependencies(dep);
    }

    /* Make sure the hash field is empty. This saves a loop in the initial GBC */
    node->hash = 0;
  }

  bddnodes[0].hash = 0;
  bddnodes[1].hash = 0;

  free(dep);
  return 0;
}

/* Now that all nodes are recursively reference counted we must make sure
  that the new hashing scheme is used AND that dead nodes are removed.
  This is also a good time to create the interaction matrix. */
static void reorder_gbc(void) {
  int n;

  bddfreepos = 0;
  bddfreenum = 0;

  /* No need to zero all hash fields - this is done in mark_roots */

  for (n=bddnodesize-1 ; n>=2 ; n--) {
    register BddNode *node = &bddnodes[n];

    if (node->refcou > 0) {
      register unsigned int hash;

      hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = n;
    }
    else {
      LOWp(node) = -1;
      node->next = bddfreepos;
      bddfreepos = n;
      bddfreenum++;
    }
  }
}

static void reorder_setLevellookup(void) {
  int n;
  for (n=0 ; n<bddvarnum ; n++) {
    #ifdef USERESIZE
    levels[n].maxsize = bddnodesize / bddvarnum;
    levels[n].start = n * levels[n].maxsize;
    levels[n].size = MIN(levels[n].maxsize, (levels[n].nodenum*5)/4);
    #else
    levels[n].maxsize = bddnodesize / bddvarnum;
    levels[n].start = n * levels[n].maxsize;
    levels[n].size = levels[n].maxsize;
    #endif
    if (levels[n].size >= 4)
      levels[n].size = bdd_prime_lte(levels[n].size);
  }
}

static void reorder_rehashAll(void) {
  int n;

  reorder_setLevellookup();
  bddfreepos = 0;

  for (n=bddnodesize-1 ; n>=0 ; n--)
    bddnodes[n].hash = 0;

  for (n=bddnodesize-1 ; n>=2 ; n--) {
    register BddNode *node = &bddnodes[n];

    if (node->refcou > 0) {
      register unsigned int hash;

      hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = n;
    }
    else {
      node->next = bddfreepos;
      bddfreepos = n;
    }
  }
}

/*=== Unique table handling for reordering ===*/
/* Note: rehashing must not take place during a makenode call. It is okay
  to resize the table, but *not* to rehash it. 
 */
static int reorder_makenode(int var, int low, int high) {
  register BddNode *node;
  register unsigned int hash;
  register int res;

  #ifdef CACHESTATS
  bddcachestats.uniqueAccess++;
  #endif 
  /* Note: We know that low,high has a refcou greater than zero, so
	there is no need to add reference *recursively* */
  /* check whether childs are equal */
  if (low == high) {
    INCREF(low);
    return low;
  }
  /* Try to find an existing node of this kind */
  hash = NODEHASH(var, low, high);
  res = bddnodes[hash].hash;
      
  while(res != 0) {
    if (LOW(res) == low  &&  HIGH(res) == high) {
      #ifdef CACHESTATS
      bddcachestats.uniqueHit++;
      #endif
      INCREF(res);
      return res;
    }
    res = bddnodes[res].next;
    #ifdef CACHESTATS
    bddcachestats.uniqueChain++;
    #endif
  }
   
  /* No existing node -> build one */
  #ifdef CACHESTATS
  bddcachestats.uniqueMiss++;
  #endif

  /* Any free nodes to use ? */
  if (bddfreepos == 0){
    if (bdderrorcond)
      return 0;
      
  /* Try to allocate more nodes - call noderesize without
  * enabling rehashing.
  * Note: if ever rehashing is allowed here, then remember to
  * update local variable "hash" */
    bdd_noderesize(0);
    resizedInMakenode = 1;

  /* Panic if that is not possible */
    if (bddfreepos == 0) {
      bdd_error(BDD_NODENUM);
      bdderrorcond = abs(BDD_NODENUM);
      return 0;
    }
  }

  /* Build new node */
  res = bddfreepos;
  bddfreepos = bddnodes[bddfreepos].next;
  levels[var].nodenum++;
  bddproduced++;
  bddfreenum--;

  node = &bddnodes[res];
  VARp(node) = var;
  LOWp(node) = low;
  HIGHp(node) = high;

  /* Insert node in hash chain */
  node->next = bddnodes[hash].hash;
  bddnodes[hash].hash = res;

  /* Make sure it is reference counted */
  node->refcou = 1;
  INCREF(LOWp(node));
  INCREF(HIGHp(node));
   
  return res;
}

/*=== Swapping two adjacent variables ===*/
/* Go through var 0 nodes. Move nodes that depends on var 1 to a separate
 * chain (toBeProcessed) and let the rest stay in the table.
 */
static int reorder_downSimple(int var0) {
  int toBeProcessed = 0;
  int var1 = bddlevel2var[bddvar2level[var0]+1];
  int vl0 = levels[var0].start;
  int size0 = levels[var0].size;
  int n;

  levels[var0].nodenum = 0;

  for (n=0 ; n<size0 ; n++) {
    int r;

    r = bddnodes[n + vl0].hash;
    bddnodes[n + vl0].hash = 0;

    while (r != 0) {
      BddNode *node = &bddnodes[r];
      int next = node->next;
      if (VAR(LOWp(node)) != var1  &&  VAR(HIGHp(node)) != var1) {
        /* Node does not depend on next var, let it stay in the chain */
        node->next = bddnodes[n+vl0].hash;
        bddnodes[n+vl0].hash = r;
        levels[var0].nodenum++;
      }
      else {
      	/* Node depends on next var - save it for later procesing */
        node->next = toBeProcessed;
        toBeProcessed = r;
        #ifdef SWAPCOUNT
          bddcachestats.swapCount++;
        #endif
      }
      r = next;
    }
  }

  return toBeProcessed;
}

/* Now process all the var 0 nodes that depends on var 1.
 *
 * It is extremely important that no rehashing is done inside the makenode
 * calls, since this would destroy the toBeProcessed chain.
 */
static void reorder_swap(int toBeProcessed, int var0) {
  int var1 = bddlevel2var[bddvar2level[var0]+1];

  while (toBeProcessed) {
    BddNode *node = &bddnodes[toBeProcessed];
    int next = node->next;
    int f0 = LOWp(node);
    int f1 = HIGHp(node);
    int f00, f01, f10, f11, hash;
    
    /* Find the cofactors for the new nodes */
    if (VAR(f0) == var1) {
      f00 = LOW(f0);
      f01 = HIGH(f0);
    }
    else
      f00 = f01 = f0;
    
    if (VAR(f1) == var1) {
      f10 = LOW(f1);
      f11 = HIGH(f1);
    }
    else
      f10 = f11 = f1;

    /* Note: makenode does refcou. */
    f0 = reorder_makenode(var0, f00, f10);
    f1 = reorder_makenode(var0, f01, f11);
    node = &bddnodes[toBeProcessed];  /* Might change in makenode */

    /* We know that the refcou of the grandchilds of this node
     * is greater than one (these are f00...f11), so there is
     * no need to do a recursive refcou decrease. It is also
     * possible for the LOWp(node)/high nodes to come alive again,
     * so deref. of the childs is delayed until the local GBC. */

    DECREF(LOWp(node));
    DECREF(HIGHp(node));
    
    /* Update in-place */
    VARp(node) = var1;
    LOWp(node) = f0;
    HIGHp(node) = f1;
    
    levels[var1].nodenum++;
    
    /* Rehash the node since it got new childs */
    hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));
    node->next = bddnodes[hash].hash;
    bddnodes[hash].hash = toBeProcessed;

    toBeProcessed = next;
  }
}

/* Now go through the var 1 chains. The nodes live here have survived
 * the call to reorder_swap() and may stay in the chain.
 * The dead nodes are reclaimed.
 */
static void reorder_localGbc(int var0) {
  int var1 = bddlevel2var[bddvar2level[var0]+1];
  int vl1 = levels[var1].start;
  int size1 = levels[var1].size;
  int n;

  for (n=0 ; n<size1 ; n++) {
    int hash = n+vl1;
    int r = bddnodes[hash].hash;
    bddnodes[hash].hash = 0;

    while (r) {
      BddNode *node = &bddnodes[r];
      int next = node->next;

      if (node->refcou > 0) {
        node->next = bddnodes[hash].hash;
        bddnodes[hash].hash = r;
      }
      else {
        DECREF(LOWp(node));
        DECREF(HIGHp(node));
        
        LOWp(node) = -1;
        node->next = bddfreepos; 
        bddfreepos = r;
        levels[var1].nodenum--;
        bddfreenum++;
      }

      r = next;
    }
  }   
}

#ifdef USERESIZE
static void reorder_swapResize(int toBeProcessed, int var0) {
  int var1 = bddlevel2var[bddvar2level[var0]+1];

  while (toBeProcessed) {
    BddNode *node = &bddnodes[toBeProcessed];
    int next = node->next;
    int f0 = LOWp(node);
    int f1 = HIGHp(node);
    int f00, f01, f10, f11;
    
    /* Find the cofactors for the new nodes */
    if (VAR(f0) == var1) {
      f00 = LOW(f0);
      f01 = HIGH(f0);
    }
    else
      f00 = f01 = f0;
    
    if (VAR(f1) == var1) {
      f10 = LOW(f1);
      f11 = HIGH(f1);
    }
    else
      f10 = f11 = f1;

    /* Note: makenode does refcou. */
    f0 = reorder_makenode(var0, f00, f10);
    f1 = reorder_makenode(var0, f01, f11);
    node = &bddnodes[toBeProcessed];  /* Might change in makenode */

    /* We know that the refcou of the grandchilds of this node
     * is greater than one (these are f00...f11), so there is
     * no need to do a recursive refcou decrease. It is also
     * possible for the LOWp(node)/high nodes to come alive again,
     * so deref. of the childs is delayed until the local GBC. */

    DECREF(LOWp(node));
    DECREF(HIGHp(node));
    
    /* Update in-place */
    VARp(node) = var1;
    LOWp(node) = f0;
    HIGHp(node) = f1;
    
    levels[var1].nodenum++;
    
    /* Do not rehash yet since we are going to resize the hash table */
    
    toBeProcessed = next;
  }
}

static void reorder_localGbcResize(int toBeProcessed, int var0) {
  int var1 = bddlevel2var[bddvar2level[var0]+1];
  int vl1 = levels[var1].start;
  int size1 = levels[var1].size;
  int n;

  for (n=0 ; n<size1 ; n++) {
    int hash = n+vl1;
    int r = bddnodes[hash].hash;
    bddnodes[hash].hash = 0;

    while (r) {
      BddNode *node = &bddnodes[r];
      int next = node->next;

      if (node->refcou > 0) {
        node->next = toBeProcessed;
        toBeProcessed = r;
      }
      else {
        DECREF(LOWp(node));
        DECREF(HIGHp(node));
        
        LOWp(node) = -1;
        node->next = bddfreepos; 
        bddfreepos = r;
        levels[var1].nodenum--;
        bddfreenum++;
      }

      r = next;
    }
  }

  /* Resize */
  if (levels[var1].nodenum < levels[var1].size)
    levels[var1].size = MIN(levels[var1].maxsize, levels[var1].size/2);
  else
    levels[var1].size = MIN(levels[var1].maxsize, levels[var1].size*2);

  if (levels[var1].size >= 4)
    levels[var1].size = bdd_prime_lte(levels[var1].size);

  /* Rehash the remaining live nodes */
  while (toBeProcessed) {
    BddNode *node = &bddnodes[toBeProcessed];
    int next = node->next;
    int hash = NODEHASH(VARp(node), LOWp(node), HIGHp(node));

    node->next = bddnodes[hash].hash;
    bddnodes[hash].hash = toBeProcessed;

    toBeProcessed = next;
  }   
}
#endif /* USERESIZE */


static int reorder_varup(int var) {
  if (var < 0  ||  var >= bddvarnum)
    return bdd_error(BDD_VAR);
  if (bddvar2level[var] == 0)
    return 0;
  return reorder_vardown( bddlevel2var[bddvar2level[var]-1]);
}

static int reorder_vardown(int var) {
  int n, level;
  if (var < 0  ||  var >= bddvarnum)
    return bdd_error(BDD_VAR);
  if ((level=bddvar2level[var]) >= bddvarnum-1)
    return 0;
  resizedInMakenode = 0;

  if (imatrixDepends(iactmtx, var, bddlevel2var[level+1])) {
    int toBeProcessed = reorder_downSimple(var);
  #ifdef USERESIZE
    levelData *l = &levels[var];
    
    if (l->nodenum < (l->size)/3  ||
  	  l->nodenum >= (l->size*3)/2  &&  l->size < l->maxsize)
    {
      reorder_swapResize(toBeProcessed, var);
      reorder_localGbcResize(toBeProcessed, var);
    }
    else
  #endif
    {
      reorder_swap(toBeProcessed, var);
      reorder_localGbc(var);
    }
  }
  /* Swap the var<->level tables */
  n = bddlevel2var[level];
  bddlevel2var[level] = bddlevel2var[level+1];
  bddlevel2var[level+1] = n;

  n = bddvar2level[var];
  bddvar2level[var] = bddvar2level[ bddlevel2var[level] ];
  bddvar2level[ bddlevel2var[level] ] = n;
  /* Update all rename pairs */
  bdd_pairs_vardown(level);
  if (resizedInMakenode)
    reorder_rehashAll();

  return 0;
}

int bdd_swapvar(int v1, int v2) {
  int l1, l2;

  /* Do not swap when variable-blocks are used */
  if (vartree != NULL)
    return bdd_error(BDD_VARBLK);

  /* Don't bother swapping x with x */
  if (v1 == v2)
    return 0;

  /* Make sure the variable exists */
  if (v1 < 0  ||  v1 >= bddvarnum  ||  v2 < 0  ||  v2 >= bddvarnum)
    return bdd_error(BDD_VAR);

  l1 = bddvar2level[v1];
  l2 = bddvar2level[v2];

  /* Make sure v1 is before v2 */
  if (l1 > l2) {
    int tmp = v1;
    v1 = v2;
    v2 = tmp;
    l1 = bddvar2level[v1];
    l2 = bddvar2level[v2];
  }

  reorder_init();

  /* Move v1 to v2's position */
  while (bddvar2level[v1] < l2)
    reorder_vardown(v1);

  /* Move v2 to v1's position */
  while (bddvar2level[v2] > l1)
    reorder_varup(v2);

  reorder_done();

  return 0;
}

void bdd_default_reohandler(int prestate) {
  static long c1;

  if (verbose > 0) {
    if (prestate) {
      printf("Start reordering\n");
      c1 = clock();
    }
    else {
      long c2 = clock();
      printf("End reordering. Went from %d to %d nodes (%.1f sec)\n",
      usednum_before, usednum_after, (float)(c2-c1)/CLOCKS_PER_SEC);
    }
  }
}

void bdd_disable_reorder(void) {
   reorderdisabled = 1;
}

void bdd_enable_reorder(void) {
   reorderdisabled = 0;
}

int bdd_reorder_ready(void) {
  if (bddreordermethod == BDD_REORDER_NONE  ||  vartree == NULL  ||
  bddreordertimes == 0  ||  reorderdisabled)
    return 0;
  return 1;
}
   
void bdd_reorder_auto(void) {
  if (!bdd_reorder_ready())
    return;

  if (reorder_handler != NULL)
    reorder_handler(1);

  bdd_reorder(bddreordermethod);
  bddreordertimes--;

  if (reorder_handler != NULL)
    reorder_handler(0);
}

static int reorder_init(void) {
  int n;

  if ((levels=NEW(levelData,bddvarnum)) == NULL)
    return -1;

  for (n=0 ; n<bddvarnum ; n++) {
    levels[n].start = -1;
    levels[n].size = 0;
    levels[n].nodenum = 0;
  }

  /* First mark and recursive refcou. all roots and childs. Also do some
   * setup here for both setLevellookup and reorder_gbc */
  if (mark_roots() < 0)
    return -1;

  /* Initialize the hash tables */
  reorder_setLevellookup();

  /* Garbage collect and rehash to new scheme */
  reorder_gbc();

  return 0;
}

static void reorder_done(void) {
  int n;

  for (n=0 ; n<extrootsize ; n++)
    SETMARK(extroots[n]);
  for (n=2 ; n<bddnodesize ; n++) {
    if (MARKED(n))
    	UNMARK(n);
    else
    	bddnodes[n].refcou = 0;
    bddnodes[n].level = bddvar2level[bddnodes[n].level];
  }
}

static int varseqCmp(const void *aa, const void *bb) {
  int a = bddvar2level[*((const int*)aa)];
  int b = bddvar2level[*((const int*)bb)];

  if (a < b)
    return -1;
  if (a > b)
    return 1;
  return 0;
}

static BddTree *reorder_block(BddTree *t, int method) {
  BddTree *this;

  if (t == NULL)
    return NULL;

  if (t->fixed == BDD_REORDER_FREE  &&  t->nextlevel!=NULL) {
    switch(method) {
      case BDD_REORDER_WIN2:
        t->nextlevel = reorder_win2(t->nextlevel);
        break;
      case BDD_REORDER_WIN2ITE:
        t->nextlevel = reorder_win2ite(t->nextlevel);
        break;
      case BDD_REORDER_SIFT:
        t->nextlevel = reorder_sift(t->nextlevel);
        break;
      case BDD_REORDER_SIFTITE:
        t->nextlevel = reorder_siftite(t->nextlevel);
        break;
      case BDD_REORDER_WIN3:
        t->nextlevel = reorder_win3(t->nextlevel);
        break;
      case BDD_REORDER_WIN3ITE:
        t->nextlevel = reorder_win3ite(t->nextlevel);
        break;
      case BDD_REORDER_RANDOM:
        t->nextlevel = reorder_random(t->nextlevel);
        break;
    }
  }

  for (this=t->nextlevel ; this ; this=this->next)
    reorder_block(this, method);

  if (t->seq != NULL)
    qsort(t->seq, t->last-t->first+1, sizeof(int), varseqCmp);

  return t;
}

void bdd_reorder(int method) {
  BddTree *top;
  int savemethod = bddreordermethod;
  int savetimes = bddreordertimes;

  bddreordermethod = method;
  bddreordertimes = 1;

  if ((top=bddtree_new(-1)) == NULL)
    return;
  if (reorder_init() < 0)
    return;

  usednum_before = bddnodesize - bddfreenum;

  top->first = 0;
  top->last = bdd_varnum()-1;
  top->fixed = 0;
  top->next = NULL;
  top->nextlevel = vartree;

  reorder_block(top, method);
  vartree = top->nextlevel;
  free(top);

  usednum_after = bddnodesize - bddfreenum;

  reorder_done();
  bddreordermethod = savemethod;
  bddreordertimes = savetimes;
}

int bdd_reorder_gain(void) {
  if (usednum_before == 0)
    return 0;

  return (100*(usednum_before - usednum_after)) / usednum_before;
}

bddinthandler bdd_reorder_hook(bddinthandler handler) {
  bddinthandler tmp = reorder_handler;
  reorder_handler = handler;
  return tmp;
}

bddfilehandler bdd_blockfile_hook(bddfilehandler handler) {
  bddfilehandler tmp = reorder_filehandler;
  reorder_filehandler = handler;
  return tmp;
}

int bdd_autoreorder(int method) {
  int tmp = bddreordermethod;
  bddreordermethod = method;
  bddreordertimes = -1;
  return tmp;
}

int bdd_autoreorder_times(int method, int num) {
  int tmp = bddreordermethod;
  bddreordermethod = method;
  bddreordertimes = num;
  return tmp;
}

int bdd_var2level(int var) {
  if (var < 0  ||  var >= bddvarnum)
    return bdd_error(BDD_VAR);

  return bddvar2level[var];
}

int bdd_level2var(int level) {
  if (level < 0  ||  level >= bddvarnum)
    return bdd_error(BDD_VAR);

  return bddlevel2var[level];
}

int bdd_getreorder_times(void) {
  return bddreordertimes;
}

int bdd_getreorder_method(void) {
  return bddreordermethod;
}

int bdd_reorder_verbose(int v) {
  int tmp = verbose;
  verbose = v;
  return tmp;
}

bddsizehandler bdd_reorder_probe(bddsizehandler handler) {
  bddsizehandler old = reorder_nodenum;
  if (handler == NULL)
    return reorder_nodenum;
  reorder_nodenum = handler;
  return old;
}

void bdd_clrvarblocks(void) {
  bddtree_del(vartree);
  vartree = NULL;
  blockid = 0;
}

int bdd_addvarblock(BDD b, int fixed) {
  BddTree *t;
  int n, *v, size;
  int first, last;

  if ((n=bdd_scanset(b, &v, &size)) < 0)
    return n;
  if (size < 1)
    return bdd_error(BDD_VARBLK);

  first = last = v[0];

  for (n=0 ; n<size ; n++) {
    if (v[n] < first)
      first = v[n];
    if (v[n] > last)
      last = v[n];
  }

  if ((t=bddtree_addrange(vartree, first,last, fixed,blockid)) == NULL)
    return bdd_error(BDD_VARBLK);

  vartree = t;
  return blockid++;
}

int bdd_intaddvarblock(int first, int last, int fixed) {
  BddTree *t;

  if (first < 0  ||  first >= bddvarnum  ||  last < 0  ||  last >= bddvarnum)
    return bdd_error(BDD_VAR);

  if ((t=bddtree_addrange(vartree, first,last, fixed,blockid)) == NULL)
    return bdd_error(BDD_VARBLK);

  vartree = t;
  return blockid++;
}

void bdd_varblockall(void) {
  int n;

  for (n=0 ; n<bddvarnum ; n++)
    bdd_intaddvarblock(n,n,1);
}

void bdd_printorder(void) {
  bdd_fprintorder(stdout);
}

void bdd_setvarorder(int *neworder) {
  int level;

  /* Do not set order when variable-blocks are used */
  if (vartree != NULL) {
    bdd_error(BDD_VARBLK);
    return;
  }

  reorder_init();

  for (level=0 ; level<bddvarnum ; level++) {
    int lowvar = neworder[level];

    while (bddvar2level[lowvar] > level)
      reorder_varup(lowvar);
  }

  reorder_done();
}

static void print_order_rec(FILE *o, BddTree *t, int level) {
  if (t == NULL)
    return;

  if (t->nextlevel) {
    fprintf(o, "%*s", level*3, "");
    if (reorder_filehandler)
      reorder_filehandler(o,t->id);
    else
      fprintf(o, "%3d", t->id);
    fprintf(o, "{\n");
    
    print_order_rec(o, t->nextlevel, level+1);
    
    fprintf(o, "%*s", level*3, "");
    if (reorder_filehandler)
      reorder_filehandler(o,t->id);
    else
      fprintf(o, "%3d", t->id);
    fprintf(o, "}\n");
    
    print_order_rec(o, t->next, level);
  }
  else {
    fprintf(o, "%*s", level*3, "");
    if (reorder_filehandler)
      reorder_filehandler(o,t->id);
    else
      fprintf(o, "%3d", t->id);
    fprintf(o, "\n");
    
    print_order_rec(o, t->next, level);
  }
}

void bdd_fprintorder(FILE *ofile) {
   print_order_rec(ofile, vartree, 0);
}

/* In file tree.c */
/*=====================================================================================================*/
BddTree *bddtree_addrange_rec(BddTree *, BddTree *, int, int, int, int);

static void update_seq(BddTree *t) {
  int n;
  int low = t->first;

  for (n=t->first ; n<=t->last ; n++)
    if (bddvar2level[n] < bddvar2level[low])
  low = n;

  for (n=t->first ; n<=t->last ; n++)
    t->seq[bddvar2level[n]-bddvar2level[low]] = n;
}

BddTree *bddtree_new(int id) {
  BddTree *t = NEW(BddTree,1);
  if (t == NULL)
    return NULL;

  t->first = t->last = -1;
  t->fixed = 1;
  t->next = t->prev = t->nextlevel = NULL;
  t->seq = NULL;
  t->id = id;
  return t;
}

void bddtree_del(BddTree *t) {
  if (t == NULL)
    return;

  bddtree_del(t->nextlevel);
  bddtree_del(t->next);
  if (t->seq != NULL)
    free(t->seq);
  free(t);
}

BddTree *bddtree_addrange_rec(BddTree *t, BddTree *prev, int first, int last, int fixed, int id) {
  if (first < 0  ||  last < 0  ||  last < first)
    return NULL;

  /* Empty tree -> build one */
  if (t == NULL) {
    if ((t=bddtree_new(id)) == NULL)
      return NULL;
    t->first = first;
    t->fixed = fixed;
    t->seq = NEW(int,last-first+1);
    t->last = last;
    update_seq(t);
    t->prev = prev;
    return t;
  }

  /* Check for identity */
  if (first == t->first  &&  last == t->last)
    return t;

  /* Before this section -> insert */
  if (last < t->first) {
    BddTree *tnew = bddtree_new(id);
    if (tnew == NULL)
      return NULL;
    tnew->first = first;
    tnew->last = last;
    tnew->fixed = fixed;
    tnew->seq = NEW(int,last-first+1);
    update_seq(tnew);
    tnew->next = t;
    tnew->prev = t->prev;
    t->prev = tnew;
    return tnew;
  }

  /* After this this section -> go to next */
  if (first > t->last) {
    t->next = bddtree_addrange_rec(t->next, t, first, last, fixed, id);
    return t;
  }

  /* Inside this section -> insert in next level */
  if (first >= t->first  &&  last <= t->last) {
    t->nextlevel = bddtree_addrange_rec(t->nextlevel,NULL,first,last,fixed,id);
    return t;
  }

  /* Covering this section -> insert above this level */
  if (first <= t->first) {
    BddTree *tnew;
    BddTree *this = t;

    while (1) {
      /* Partial cover ->error */
      if (last >= this->first  &&  last < this->last)
        return NULL;

      if (this->next == NULL  ||  last < this->next->first) {
        tnew = bddtree_new(id);
        if (tnew == NULL)
           return NULL;
        tnew->first = first;
        tnew->last = last;
        tnew->fixed = fixed;
        tnew->seq = NEW(int,last-first+1);
        update_seq(tnew);
        tnew->nextlevel = t;
        tnew->next = this->next;
        tnew->prev = t->prev;
        if (this->next != NULL)
           this->next->prev = tnew;
        this->next = NULL;
        t->prev = NULL;
        return tnew;
      }

      this = this->next;
    }
    
  }
    
  return NULL;
}

BddTree *bddtree_addrange(BddTree *t, int first, int last, int fixed,int id)
{ return bddtree_addrange_rec(t,NULL,first,last,fixed,id);}

/* In file prime.c */
/*=====================================================================================================*/
#define Random(i) ( (rand() % (i)) + 1 )
#define isEven(src) (!((src) & 0x1))
#define hasFactor(src,n) ( (((src)!=(n)) && ((src)%(n) == 0)) )
#define BitIsSet(src,b) ( ((src) & (1<<(b))) != 0 )

#define CHECKTIMES 20

#if defined(BUDDYUINT64)
 typedef BUDDYUINT64 UINT64;
 #define BUILTIN64
#elif defined(__GNUC__) || defined(__KCC)
 typedef long long UINT64;
 #define BUILTIN64
#elif defined(_MSV_VER)
 typedef unsigned _int64 UINT64;
 #define BUILTIN64
#else
 typedef struct __UINT64
 {
   unsigned int hi;
   unsigned int lo;
 } UINT64;

 #define MAX(a,b) ((a) > (b) ? (a) : (b))
 #define GETCARRY(a,b) ( ((a)+(b)) < MAX((a),(b)) ? 1 : 0 )
#endif

#ifndef BUILTIN64
/*64 bit unsigned int arithmetics*/
static UINT64 u64_mul(unsigned int x, unsigned int y) {
  UINT64 res;
  unsigned int yh = 0;
  unsigned int yl = y;
  int i;

  res.lo = res.hi = 0;

  for (i=0 ; i<32 ; ++i)
  {
    if (x & 0x1)
    {
      unsigned int carry = GETCARRY(res.lo,yl);
      res.lo += yl;
      res.hi += yh + carry;
    }

    yh = (yh << 1) | (yl & 0x80000000 ? 1 : 0);
    yl = (yl << 1);

    x >>= 1;
  }

  return res;
}

static void u64_shl(UINT64* a, unsigned int *carryOut) {
  *carryOut = (*carryOut << 1) | (a->hi & 0x80000000 ? 0x1 : 0x0);
  a->hi     = (a->hi << 1) | (a->lo & 0x80000000 ? 0x1 : 0x0);
  a->lo     = (a->lo << 1);
}

static unsigned int u64_mod(UINT64 dividend, unsigned int divisor) {
  unsigned int remainder = 0;
  int i;

  u64_shl(&dividend, &remainder);
  
  for (i=0 ; i<64 ; ++i)
  {
    if (remainder >= divisor)
      remainder -= divisor;

    u64_shl(&dividend, &remainder);
  }

  return remainder >> 1;
}
#endif /* BUILTIN64 */

#ifdef BUILTIN64
#define u64_mulmod(a,b,c) ((unsigned int)( ((UINT64)a*(UINT64)b)%(UINT64)c ));
#else
#define u64_mulmod(a,b,c) u64_mod( u64_mul((a),(b)), (c) );
#endif

static unsigned int numberOfBits(unsigned int src) {
  unsigned int b;

  if (src == 0)
    return 0;
  
  for (b=(sizeof(unsigned int)*8)-1 ; b>0 ; --b)
    if (BitIsSet(src,b))
      return b+1;

  return 1;
}

static int isWitness(unsigned int witness, unsigned int src) {
  unsigned int bitNum = numberOfBits(src-1)-1;
  unsigned int d = 1;
  int i;

  for (i=bitNum ; i>=0 ; --i)
  {
    unsigned int x = d;

    d = u64_mulmod(d,d,src);
    
    if (d == 1  &&  x != 1  &&  x != src-1)
      return 1;
    
    if (BitIsSet(src-1,i))
      d = u64_mulmod(d,witness,src);
  }

  return d != 1;
}

static int isMillerRabinPrime(unsigned int src) {
  int n;

  for (n=0 ; n<CHECKTIMES ; ++n)
  {
    unsigned int witness = Random(src-1);

    if (isWitness(witness,src))
      return 0;
  }

  return 1;
}

static int hasEasyFactors(unsigned int src) {
  return hasFactor(src, 3)
      || hasFactor(src, 5)
      || hasFactor(src, 7)
      || hasFactor(src, 11)
      || hasFactor(src, 13);
}

static int isPrime(unsigned int src) {
  if (hasEasyFactors(src))
    return 0;

  return isMillerRabinPrime(src);
}

unsigned int bdd_prime_gte(unsigned int src) {
  if (isEven(src))
    ++src;

  while (!isPrime(src))
    src += 2;

  return src;
}

unsigned int bdd_prime_lte(unsigned int src) {
  if (isEven(src))
     --src;

  while (!isPrime(src))
     src -= 2;

  return src;
}

/* In file cache.c */
/*=====================================================================================================*/
int BddCache_init(BddCache *cache, int size) {
  int n;

  size = bdd_prime_gte(size);

  if ((cache->table=NEW(BddCacheData,size)) == NULL)
    return bdd_error(BDD_MEMORY);

  for (n=0 ; n<size ; n++)
    cache->table[n].a = -1;
  cache->tablesize = size;

  return 0;
}

void BddCache_done(BddCache *cache) {
  free(cache->table);
  cache->table = NULL;
  cache->tablesize = 0;
}

int BddCache_resize(BddCache *cache, int newsize) {
  int n;

  free(cache->table);

  newsize = bdd_prime_gte(newsize);

  if ((cache->table=NEW(BddCacheData,newsize)) == NULL)
    return bdd_error(BDD_MEMORY);

  for (n=0 ; n<newsize ; n++)
    cache->table[n].a = -1;
  cache->tablesize = newsize;

  return 0;
}

void BddCache_reset(BddCache *cache) {
  register int n;
  for (n=0 ; n<cache->tablesize ; n++)
    cache->table[n].a = -1;
}

/* In file imatrix.c */
/*=====================================================================================================*/
imatrix* imatrixNew(int size) {
  imatrix *mtx = NEW(imatrix,1);
  int n,m;
  if (!mtx)
    return NULL;
  if ((mtx->rows=NEW(char*,size)) == NULL) {
    free(mtx);
    return NULL;
  }

  for (n=0 ; n<size ; n++) {
    if ((mtx->rows[n]=NEW(char,size/8+1)) == NULL) {
      for (m=0 ; m<n ; m++)
        free(mtx->rows[m]);
      free(mtx);
      return NULL;
    }
    memset(mtx->rows[n], 0, size/8+1);
  }
  mtx->size = size;
  return mtx;
}

void imatrixDelete(imatrix *mtx) {
  int n;

  for (n=0 ; n<mtx->size ; n++)
    free(mtx->rows[n]);
  free(mtx->rows);
  free(mtx);
}

void imatrixFPrint(imatrix *mtx, FILE *ofile) {
  int x,y;

  fprintf(ofile, "    ");
  for (x=0 ; x<mtx->size ; x++)
    fprintf(ofile, "%c", x < 26 ? (x+'a') : (x-26)+'A');
  fprintf(ofile, "\n");

  for (y=0 ; y<mtx->size ; y++) {
    fprintf(ofile, "%2d %c", y, y < 26 ? (y+'a') : (y-26)+'A');
    for (x=0 ; x<mtx->size ; x++)
  fprintf(ofile, "%c", imatrixDepends(mtx,y,x) ? 'x' : ' ');
    fprintf(ofile, "\n");
  }
}

void imatrixPrint(imatrix *mtx)
{	imatrixFPrint(mtx, stdout);}

void imatrixSet(imatrix *mtx, int a, int b)
{	mtx->rows[a][b/8] |= 1<<(b%8);}

void imatrixClr(imatrix *mtx, int a, int b)
{	mtx->rows[a][b/8] &= ~(1<<(b%8));}

int imatrixDepends(imatrix *mtx, int a, int b)
{	return mtx->rows[a][b/8] & (1<<(b%8));}

/* In file kernel.c */
/*=====================================================================================================*/
const BDD bddtrue=1;                     /* The constant true bdd */
const BDD bddfalse=0;                    /* The constant false bdd */

/*=== INTERNAL DEFINITIONS ===*/
/* Min. number of nodes (%) that has to be left after a garbage collect unless a resize should be done. */
static int minfreenodes=20;


/*=== GLOBAL KERNEL VARIABLES ====*/
int          bddrunning;            /* Flag - package initialized */
int          bdderrorcond;          /* Some error condition */
int          bddnodesize;           /* Number of allocated nodes */
int          mtbddvaluesize;         /* Number of allocated values for MTBDD */
int          mtbddvaluevaluecount;

int          bddmaxnodesize;        /* Maximum allowed number of nodes */
int          bddmaxnodeincrease;    /* Max. # of nodes used to inc. table */
BddNode*     bddnodes;          /* All of the bdd nodes */
MTBddValue*  mtbddvalues;

int          bddfreepos;        /* First free node */
int          mtbddfreepos;      /* First free value */
int          bddfreenum;        /* Number of free nodes */
int          mtbddfreenum;
long int     bddproduced;       /* Number of new nodes ever produced */
int          bddvarnum;         /* Number of defined BDD variables */
int*         bddrefstack;       /* Internal node reference stack */
int*         bddrefstacktop;    /* Internal node reference stack top */
int*         bddvar2level;      /* Variable -> level table */
int*         bddlevel2var;      /* Level -> variable table */
jmp_buf      bddexception;      /* Long-jump point for interrupting calc. */
int          bddresized;        /* Flag indicating a resize of the nodetable */

bddCacheStat bddcachestats;





/*=== PRIVATE KERNEL VARIABLES ===*/
static BDD*     bddvarset;             /* Set of defined BDD variables */
static int      gbcollectnum;          /* Number of garbage collections */
static int      cachesize;             /* Size of the operator caches */
static long int gbcclock;              /* Clock ticks used in GBC */
static int      usednodes_nextreorder; /* When to do reorder next time */
static bddinthandler  err_handler;     /* Error handler */
static bddgbchandler  gbc_handler;     /* Garbage collection handler */
static bdd2inthandler resize_handler;  /* Node-table-resize handler */


   /* Strings for all error mesages */
static char *errorstrings[BDD_ERRNUM] =
{ "Out of memory", "Unknown variable", "Value out of range",
  "Unknown BDD root dereferenced", "bdd_init() called twice",
  "File operation failed", "Incorrect file format",
  "Variables not in ascending order", "User called break",
  "Mismatch in size of variable sets",
  "Cannot allocate fewer nodes than already in use",
  "Unknown operator", "Illegal variable set",
  "Bad variable block operation",
  "Trying to decrease the number of variables",
  "Trying to replace with variables already in the bdd",
  "Number of nodes reached user defined maximum",
  "Unknown BDD - was not in node table",
  "Bad size argument",
  "Mismatch in bitvector size",
  "Illegal shift-left/right parameter",
  "Division by zero" };

/*=== OTHER INTERNAL DEFINITIONS ===*/
#define NODEHASH_K(lvl,l,h) (TRIPLE(lvl,l,h) % bddnodesize)
#define VALUEHASH_K(lvl,l,h) (TRIPLE(lvl,l,h) % mtbddvaluesize)

int bdd_init(int initnodesize, int initvaluesize, int cs) {
  // int bdd_init(int initnodesize, int cs)
  int n, err;
  if (bddrunning)
    return bdd_error(BDD_RUNNING);

  bddnodesize = bdd_prime_gte(initnodesize);
  mtbddvaluesize = initvaluesize;


  if ((bddnodes=(BddNode*)malloc(sizeof(BddNode)*bddnodesize)) == NULL)
    return bdd_error(BDD_MEMORY);

  if ((mtbddvalues=(MTBddValue*)malloc(sizeof(MTBddValue)*mtbddvaluesize)) == NULL)
    return bdd_error(BDD_MEMORY);

  mtbddvaluevaluecount = 0;
  bddresized = 0;

  for (n=0 ; n<bddnodesize ; n++) {
    bddnodes[n].refcou = 0;
    LOW(n) = -1;
    bddnodes[n].hash = 0;
    // LEVEL(n) = 0;
    bddnodes[n].level = 0;
    bddnodes[n].next = n+1;
    // bddnodes[n].rule_records = NULL;
  }
  bddnodes[bddnodesize-1].next = 0;

  bddnodes[0].refcou = bddnodes[1].refcou = bddnodes[2].refcou = MAXREF;
  LOW(0) = HIGH(0) = 0;
  LOW(1) = HIGH(1) = 0;
  LOW(2) = HIGH(2) = 0;

  for (n=0 ; n<mtbddvaluesize ; n++) {
    // mtbddvalues[n].refcou = 0;
    mtbddvalues[n].hash = 0;
    mtbddvalues[n].test_count = 0;
    // LEVEL(n) = 0;
    // mtbddvalues[n].level = 0;
    mtbddvalues[n].next = n+1;
    mtbddvalues[n].original_bdd = BDDZERO;
    mtbddvalues[n].self_bdd = BDDZERO;
    mtbddvalues[n].coveredby_bdd = BDDZERO;
    mtbddvalues[n].covering_bdd = BDDZERO;
    mtbddvalues[n].main_rule = NULL;
    mtbddvalues[n].rule_records = NULL;
  }
  mtbddvalues[mtbddvaluesize-1].next = -1;
  // bddnodes[0].refcou = MAXREF;

  if ((err=bdd_operator_init(cs)) < 0) {
    bdd_done();
    return err;
  }

  bddfreepos = 3;
  mtbddfreepos = 0;
  bddfreenum = bddnodesize-3;
  mtbddfreenum = mtbddvaluesize;

  bddrunning = 1;
  bddvarnum = 0;
  gbcollectnum = 0;
  gbcclock = 0;
  cachesize = cs;
  usednodes_nextreorder = bddnodesize;
  bddmaxnodeincrease = DEFAULTMAXNODEINC;

  bdderrorcond = 0;

  bddcachestats.uniqueAccess = 0;
  bddcachestats.uniqueChain = 0;
  bddcachestats.uniqueHit = 0;
  bddcachestats.uniqueMiss = 0;
  bddcachestats.opHit = 0;
  bddcachestats.opMiss = 0;
  bddcachestats.swapCount = 0;

  bdd_gbc_hook(bdd_default_gbchandler);
  bdd_error_hook(bdd_default_errhandler);
  bdd_resize_hook(NULL);
  bdd_pairs_init();
  bdd_reorder_init();
  bdd_fdd_init();

  if (setjmp(bddexception) != 0)
    assert(0);

  return 0;
}

void bdd_done(void) {
  /*sanitycheck(); FIXME */
  bdd_fdd_done();
  bdd_reorder_done();
  bdd_pairs_done();

  for (int i = 0; i < mtbddvaluesize; i++)
    free(mtbddvalues[i].rule_records);
  free(mtbddvalues);
  free(bddnodes);
  free(bddrefstack);
  free(bddvarset);
  free(bddvar2level);
  free(bddlevel2var);

  bddnodes = NULL;
  bddrefstack = NULL;
  bddvarset = NULL;

  bdd_operator_done();

  bddrunning = 0;
  bddnodesize = 0;
  bddmaxnodesize = 0;
  bddvarnum = 0;
  bddproduced = 0;

  err_handler = NULL;
  gbc_handler = NULL;
  resize_handler = NULL;
}

int bdd_setvarnum(int num) {
  int bdv;
  int oldbddvarnum = bddvarnum;

  bdd_disable_reorder();
    
  if (num < 1  ||  num > MAXVAR) {
    bdd_error(BDD_RANGE);
    return bddfalse;
  }
  if (num < bddvarnum)
    return bdd_error(BDD_DECVNUM);
  if (num == bddvarnum)
    return 0;

  if (bddvarset == NULL) {
    if ((bddvarset=(BDD*)malloc(sizeof(BDD)*num*2)) == NULL)
      return bdd_error(BDD_MEMORY);
    if ((bddlevel2var=(int*)malloc(sizeof(int)*(num+1))) == NULL) {
      free(bddvarset);
      return bdd_error(BDD_MEMORY);
    }
    if ((bddvar2level=(int*)malloc(sizeof(int)*(num+1))) == NULL) {
      free(bddvarset);
      free(bddlevel2var);
      return bdd_error(BDD_MEMORY);
    }
  }
  else {
    if ((bddvarset=(BDD*)realloc(bddvarset,sizeof(BDD)*num*2)) == NULL)
      return bdd_error(BDD_MEMORY);
    if ((bddlevel2var=(int*)realloc(bddlevel2var,sizeof(int)*(num+1))) == NULL) {
      free(bddvarset);
      return bdd_error(BDD_MEMORY);
    }
    if ((bddvar2level=(int*)realloc(bddvar2level,sizeof(int)*(num+1))) == NULL) {
      free(bddvarset);
      free(bddlevel2var);
      return bdd_error(BDD_MEMORY);
    }
  }

  if (bddrefstack != NULL)
    free(bddrefstack);
  bddrefstack = bddrefstacktop = (int*)malloc(sizeof(int)*(num*2+4));

  for(bdv=bddvarnum ; bddvarnum < num; bddvarnum++) {
    bddvarset[bddvarnum*2] = PUSHREF( bdd_makenode(bddvarnum, 0, 1) );
    bddvarset[bddvarnum*2+1] = bdd_makenode(bddvarnum, 1, 0);
    POPREF(1);
    
    if (bdderrorcond) {
      bddvarnum = bdv;
      return -bdderrorcond;
    }
    
    bddnodes[bddvarset[bddvarnum*2]].refcou = MAXREF;
    bddnodes[bddvarset[bddvarnum*2+1]].refcou = MAXREF;
    bddlevel2var[bddvarnum] = bddvarnum;
    bddvar2level[bddvarnum] = bddvarnum;
  }
  bddnodes[0].level = num;
  bddnodes[1].level = num;
  // LEVEL(0) = num;
  // LEVEL(1) = num;
  bddvar2level[num] = num;
  bddlevel2var[num] = num;

  bdd_pairs_resize(oldbddvarnum, bddvarnum);
  bdd_operator_varresize();

  bdd_enable_reorder();

  return 0;
}

int bdd_extvarnum(int num) {
  int start = bddvarnum;

  if (num < 0  ||  num > 0x3FFFFFFF)
    return bdd_error(BDD_RANGE);

  bdd_setvarnum(bddvarnum+num);
  return start;
}

bddinthandler bdd_error_hook(bddinthandler handler) {
  bddinthandler tmp = err_handler;
  err_handler = handler;
  return tmp;
}

void bdd_clear_error(void) {
  bdderrorcond = 0;
  bdd_operator_reset();
}

bddgbchandler bdd_gbc_hook(bddgbchandler handler) {
  bddgbchandler tmp = gbc_handler;
  gbc_handler = handler;
  return tmp;
}

bdd2inthandler bdd_resize_hook(bdd2inthandler handler) {
  bdd2inthandler tmp = handler;
  resize_handler = handler;
  return tmp;
}

int bdd_setmaxincrease(int size) {
  int old = bddmaxnodeincrease;

  if (size < 0)
    return bdd_error(BDD_SIZE);

  bddmaxnodeincrease = size;
  return old;
}

int bdd_setmaxnodenum(int size) {
  if (size > bddnodesize  ||  size == 0) {
    int old = bddmaxnodesize;
    bddmaxnodesize = size;
    return old;
  }

  return bdd_error(BDD_NODES);
}

int bdd_setminfreenodes(int mf) {
  int old = minfreenodes;

  if (mf<0 || mf>100)
    return bdd_error(BDD_RANGE);

  minfreenodes = mf;
  return old;
}

int bdd_getnodenum(void) {
  return bddnodesize - bddfreenum;
}

int mtbdd_getvaluenum(void) {
  return mtbddvaluesize - mtbddfreenum;
}


int bdd_getallocnum(void) {
  return bddnodesize;
}

int bdd_isrunning(void) {
  return bddrunning;
}

char *bdd_versionstr(void) {
  static char str[100];
  sprintf(str, "BuDDy -  release %d.%d", VERSION/10, VERSION%10);
  return str;
}

int bdd_versionnum(void) {
  return VERSION;
}

void bdd_stats(bddStat *s) {
  s->produced = bddproduced;
  s->nodenum = bddnodesize;
  s->maxnodenum = bddmaxnodesize;
  s->freenodes = bddfreenum;
  s->minfreenodes = minfreenodes;
  s->varnum = bddvarnum;
  s->cachesize = cachesize;
  s->gbcnum = gbcollectnum;
}

void bdd_cachestats(bddCacheStat *s) {
  *s = bddcachestats;
}

void bdd_fprintstat(FILE *ofile) {
  bddCacheStat s;
  bdd_cachestats(&s);

  fprintf(ofile, "\nCache statistics\n");
  fprintf(ofile, "----------------\n");

  fprintf(ofile, "Unique Access:  %ld\n", s.uniqueAccess);
  fprintf(ofile, "Unique Chain:   %ld\n", s.uniqueChain);
  fprintf(ofile, "Unique Hit:     %ld\n", s.uniqueHit);
  fprintf(ofile, "Unique Miss:    %ld\n", s.uniqueMiss);
  fprintf(ofile, "=> Hit rate =   %.2f\n",
   (s.uniqueHit+s.uniqueMiss > 0) ? 
   ((float)s.uniqueHit)/((float)s.uniqueHit+s.uniqueMiss) : 0);
  fprintf(ofile, "Operator Hits:  %ld\n", s.opHit);
  fprintf(ofile, "Operator Miss:  %ld\n", s.opMiss);
  fprintf(ofile, "=> Hit rate =   %.2f\n",
   (s.opHit+s.opMiss > 0) ? 
   ((float)s.opHit)/((float)s.opHit+s.opMiss) : 0);
  fprintf(ofile, "Swap count =    %ld\n", s.swapCount);
}

void bdd_printstat(void) {
  bdd_fprintstat(stdout);
}

const char *bdd_errstring(int e) {
  e = abs(e);
  if (e<1 || e>BDD_ERRNUM)
    return NULL;
  return errorstrings[e-1];
}

void bdd_default_errhandler(int e) {
  fprintf(stderr, "BDD error: %s\n", bdd_errstring(e));
  exit(1);
}

int bdd_error(int e) {
  if (err_handler != NULL)
  err_handler(e);

  return e;
}

BDD bdd_true(void) {
  return 1;
}

BDD bdd_false(void) {
  return 0;
}

BDD bdd_ithvar(int var) {
  if (var < 0  ||  var >= bddvarnum) {
    bdd_error(BDD_VAR);
    return bddfalse;
  }

  return bddvarset[var*2];
}

BDD bdd_nithvar(int var) {
  if (var < 0  ||  var >= bddvarnum) {
    bdd_error(BDD_VAR);
    return bddfalse;
  }
  return bddvarset[var*2+1];
}

int bdd_varnum(void) {
  return bddvarnum;
}

int bdd_var(BDD root) {
	CHECK(root);
	if (root < 2)
		return bdd_error(BDD_ILLBDD);
	return (bddlevel2var[bddnodes[root].level]);
   // return (bddlevel2var[LEVEL(root)]);
}

BDD bdd_low(BDD root) {
  CHECK(root);
  if (root < 2)
    return bdd_error(BDD_ILLBDD);

  return (LOW(root));
} 

BDD bdd_high(BDD root) {
  CHECK(root);
  if (root < 2)
    return bdd_error(BDD_ILLBDD);

  return (HIGH(root));
}

void bdd_default_gbchandler(int pre, bddGbcStat *s) {
  if (!pre) {
    printf("Garbage collection #%d: %d nodes / %d free",
     s->num, s->nodes, s->freenodes);
    printf(" / %.1fs / %.1fs total\n",
     (float)s->time/(float)(CLOCKS_PER_SEC),
     (float)s->sumtime/(float)CLOCKS_PER_SEC);
  }
}

static void bdd_gbc_rehash(void) {
  int n;

  bddfreepos = 0;
  bddfreenum = 0;

  for (n=bddnodesize-1 ; n>=2 ; n--){
    register BddNode *node = &bddnodes[n];

    if (LOWp(node) != -1) {
      register unsigned int hash;

      hash = NODEHASH_K(((node)->level), LOWp(node), HIGHp(node));
      // hash = NODEHASH_K(LEVELp(node), LOWp(node), HIGHp(node));
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = n;
    }
    else {
      node->next = bddfreepos;
      bddfreepos = n;
      bddfreenum++;
    }
  }
}

void bdd_gbc(void) {
  int *r;
  int n;
  long int c2, c1 = clock();

  // if (gbc_handler != NULL)
  // {
  //    bddGbcStat s;
  //    s.nodes = bddnodesize;
  //    s.freenodes = bddfreenum;
  //    s.time = 0;
  //    s.sumtime = gbcclock;
  //    s.num = gbcollectnum;
  //    gbc_handler(1, &s);
  // }

  for (r=bddrefstack ; r<bddrefstacktop ; r++)
    bdd_mark(*r);

  for (n=0 ; n<bddnodesize ; n++) {
    if (bddnodes[n].refcou > 0)
      bdd_mark(n);
    bddnodes[n].hash = 0;
  }
  bddfreepos = 0;
  bddfreenum = 0;

  for (n=bddnodesize-1 ; n>=3 ; n--) {
    register BddNode *node = &bddnodes[n];

    // if ((LEVELp(node) & MARKON)  &&  LOWp(node) != -1)
    if (((node)->level & MARKON)  &&  LOWp(node) != -1) {
      register unsigned int hash;

      (node)->level &= MARKOFF;
      hash = NODEHASH_K((node)->level, LOWp(node), HIGHp(node));

      // LEVELp(node) &= MARKOFF;
      // hash = NODEHASH_K(LEVELp(node), LOWp(node), HIGHp(node));
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = n;
    }
    else {
      LOWp(node) = -1;
      node->next = bddfreepos;
      bddfreepos = n;
      bddfreenum++;
    }
  }
  bdd_operator_reset();

  c2 = clock();
  gbcclock += c2-c1;
  gbcollectnum++;

  // if (gbc_handler != NULL)
  // {
  //    bddGbcStat s;
  //    s.nodes = bddnodesize;
  //    s.freenodes = bddfreenum;
  //    s.time = c2-c1;
  //    s.sumtime = gbcclock;
  //    s.num = gbcollectnum;
  //    gbc_handler(0, &s);
  // }
}

void bdd_gbc_except_applycache(void) {
  int *r;
  int n;

  for (r=bddrefstack ; r<bddrefstacktop ; r++)
    bdd_mark(*r);

  for (n=0 ; n<bddnodesize ; n++) {
    if (bddnodes[n].refcou > 0)
      bdd_mark(n);
    bddnodes[n].hash = 0;
  }

  bddfreepos = 0;
  bddfreenum = 0;

  for (n=bddnodesize-1 ; n>=2 ; n--) {
    register BddNode *node = &bddnodes[n];

    // if ((LEVELp(node) & MARKON)  &&  LOWp(node) != -1)
    if (((node)->level & MARKON)  &&  LOWp(node) != -1) {
      register unsigned int hash;

      (node)->level &= MARKOFF;
      hash = NODEHASH_K((node)->level, LOWp(node), HIGHp(node));

      // LEVELp(node) &= MARKOFF;
      // hash = NODEHASH_K(LEVELp(node), LOWp(node), HIGHp(node));
      node->next = bddnodes[hash].hash;
      bddnodes[hash].hash = n;
    }
    else {
      LOWp(node) = -1;
      node->next = bddfreepos;
      bddfreepos = n;
      bddfreenum++;
    }
  }
  bdd_operator_reset_except_applycache();
}

void bdd_gbc_fastall(void) {
  int n;
  for (n=0 ; n<bddnodesize ; n++)
    bddnodes[n].hash = 0;  
  bddfreepos = 0;
  bddfreenum = bddnodesize - 2;
  for (n=bddnodesize-1 ; n>=2 ; n--){
    register BddNode *node = &bddnodes[n];
    LOWp(node) = -1;
    node->next = bddfreepos;
    bddfreepos = n;
  }
  bdd_operator_reset();
}

BDD bdd_addref(BDD root) {
  if (root < 2  ||  !bddrunning)
    return root;
  // if (bddnodes[root].low == 1)
  //   return root;

  if (root >= bddnodesize)
    return bdd_error(BDD_ILLBDD);
  if (LOW(root) == -1)
    return bdd_error(BDD_ILLBDD);

  INCREF(root);
  return root;
}

BDD bdd_delref(BDD root) {
  if (root < 2  ||  !bddrunning)
    return root;
  if (root >= bddnodesize)
    return bdd_error(BDD_ILLBDD);
  if (LOW(root) == -1)
    return bdd_error(BDD_ILLBDD);

  /* if the following line is present, fails there much earlier */ 
  if (!HASREF(root)) bdd_error(BDD_BREAK); /* distinctive */

  DECREF(root);
  return root;
}

/*=== RECURSIVE MARK / UNMARK ===*/
void bdd_mark(int i) {
  BddNode *node;

  if (i < 3)
    return;

  node = &bddnodes[i];
  // if (LEVELp(node) & MARKON  ||  LOWp(node) == -1)
  if ((node)->level & MARKON  ||  LOWp(node) == -1)
    return;

  (node)->level |= MARKON;
  // LEVELp(node) |= MARKON;

  if (LOWp(node) == 2)
    return;

  bdd_mark(LOWp(node));
  bdd_mark(HIGHp(node));
}

void bdd_mark_upto(int i, int level) {
  BddNode *node = &bddnodes[i];

  if (i < 2)
    return;

  // if (LEVELp(node) & MARKON  ||  LOWp(node) == -1)
  if ((node)->level & MARKON  ||  LOWp(node) == -1)
    return;

  // if (LEVELp(node) > level)
  if ((node)->level > level)
    return;

  // LEVELp(node) |= MARKON;
  (node)->level |= MARKON;

  bdd_mark_upto(LOWp(node), level);
  bdd_mark_upto(HIGHp(node), level);
}

void bdd_markcount(int i, int *cou) {
  BddNode *node;

  if (i < 2){
    // test_counter ++;
    return;
  }


  node = &bddnodes[i];

  if (MARKEDp(node)  ||  LOWp(node) == -1)
    return;

  if(LOWp(node) == HIGHp(node)){
    // test_counter ++;
    printf("%d;", LOWp(node));
    printf("%d;", LOWp(node));
    printf("\n");
  }

  SETMARKp(node);
  *cou += 1;


  if (LOWp(node) == 2){
    test_counter ++;
    return;
  }

  bdd_markcount(LOWp(node), cou);
  bdd_markcount(HIGHp(node), cou);
}

void bdd_unmark(int i) {
  BddNode *node;

  if (i < 2)
    return;

  node = &bddnodes[i];

  if (!MARKEDp(node)  ||  LOWp(node) == -1)
    return;
  UNMARKp(node);

  if (LOWp(node) == 2)
    return;

  bdd_unmark(LOWp(node));
  bdd_unmark(HIGHp(node));
}

void bdd_unmark_upto(int i, int level) {
  BddNode *node = &bddnodes[i];

  if (i < 2)
    return;

  // if (!(LEVELp(node) & MARKON))
  if (!((node)->level & MARKON))
    return;

  // LEVELp(node) &= MARKOFF;
  (node)->level &= MARKOFF;

  // if (LEVELp(node) > level)
  if ((node)->level > level)
    return;

  bdd_unmark_upto(LOWp(node), level);
  bdd_unmark_upto(HIGHp(node), level);
}

int bdd_makenode(unsigned int level, int low, int high) {
  register BddNode *node;
  register unsigned int hash;
  register int res;

  // #ifdef CACHESTATS
  // bddcachestats.uniqueAccess++;
  // #endif
  /* check whether childs are equal */
  if (low == high)
    if (low != 2)
      return low;

  /* Try to find an existing node of this kind */
  hash = NODEHASH_K(level, low, high);
  res = bddnodes[hash].hash;

  while(res != 0) {
    // if (LEVEL(res) == level  &&  LOW(res) == low  &&  HIGH(res) == high)
    if (bddnodes[res].level == level  &&  LOW(res) == low  &&  HIGH(res) == high) {
      // #ifdef CACHESTATS
      // bddcachestats.uniqueHit++;
      // #endif
      return res;
    }

    res = bddnodes[res].next;
    // #ifdef CACHESTATS
    // bddcachestats.uniqueChain++;
    // #endif
  }

  /* No existing node -> build one */
  // #ifdef CACHESTATS
  // bddcachestats.uniqueMiss++;
  // #endif

  /* Any free nodes to use ? */
  if (bddfreepos == 0) {
    if (bdderrorcond)
      return 0;
  /* Try to allocate more nodes */
    bdd_gbc();
    if ((bddnodesize-bddfreenum) >= usednodes_nextreorder && bdd_reorder_ready()) {
      longjmp(bddexception,1);
    }
    if ((bddfreenum*100) / bddnodesize <= minfreenodes) {
      bdd_noderesize(1);
      hash = NODEHASH_K(level, low, high);
    }

  /* Panic if that is not possible */
    if (bddfreepos == 0) {
      bdd_error(BDD_NODENUM);
      bdderrorcond = abs(BDD_NODENUM);
      return 0;
    }
  }

  /* Build new node */
  res = bddfreepos;
  bddfreepos = bddnodes[bddfreepos].next;
  bddfreenum--;
  bddproduced++;

  node = &bddnodes[res];
  // LEVELp(node) = level;
  (node)->level = level;
  LOWp(node) = low;
  HIGHp(node) = high;

    /* Insert node */
  node->next = bddnodes[hash].hash;
  bddnodes[hash].hash = res;

  return res;
}

int
rule_record_cmp(const void *a, const void *b){
  // struct rule_record *rr1 = *((struct rule_record **)a);
  // struct rule_record *rr2 = *((struct rule_record **)b);
   // if (!a)
   //    printf("there is wrong with a \n");
   // if (!b)
   //    printf("there is wrong with b \n");
   struct rule_record *rr1 = (struct rule_record *)a;
   struct rule_record *rr2 = (struct rule_record *)b;
   // printf("1:%d - %d\n", rr1->sw_idx, rr1->idx);
   // printf("2:%d - %d\n", rr2->sw_idx, rr2->idx);

   if (rr1->sw_idx != rr2->sw_idx)
      return (rr1->sw_idx - rr2->sw_idx);
   return (rr1->idx - rr2->idx);
}

struct rule_records_arr * //rule_records1为基底， rule_records2为添加
gen_rule_records_arr_fr2spec(struct rule_records_arr *rule_records1, struct rule_records_arr *rule_records2, int *sign){ //1:<=, 2:> or 3:others. with orderd rule_records
  // printf("%d - %d\n", rule_records1->rules[0].sw_idx, rule_records1->rules[0].idx);
  // printf("%d - %d\n", rule_records1->rules[rule_records1->nrules-1].sw_idx, rule_records1->rules[rule_records1->nrules-1].idx);
  // printf("%d - %d\n", rule_records2->rules[0].sw_idx, rule_records2->rules[0].idx);
  
  // printf("rule_records1 is fine \n");
  uint32_t *b = (uint32_t *)bsearch(rule_records2->rules, rule_records1->rules, rule_records1->nrules, 2*sizeof(uint32_t), rule_record_cmp);
  if(b){
    *sign = 0;
    return NULL;
  }
  struct rule_records_arr *tmp = malloc(sizeof(uint32_t)+(rule_records1->nrules + 1)*sizeof(struct rule_record));
  tmp->nrules = rule_records1->nrules + 1;
  for (int i = 0; i < tmp->nrules-1; i++)
    tmp->rules[i] = rule_records1->rules[i];
  tmp->rules[tmp->nrules-1] = rule_records2->rules[0];
  qsort(tmp->rules, tmp->nrules,sizeof(struct rule_record), rule_record_cmp);
  *sign = 1;
  return tmp;
}

struct rule_records_arr * //rule_records1为基底， rule_records2为添加
gen_rule_records_arr_fr2spec_simple(struct rule_records_arr *rule_records1, struct rule_records_arr *rule_records2, int *sign){ //1:<=, 2:> or 3:others. with orderd rule_records

  if (rule_records1->rules[0].idx <= rule_records2->rules[0].idx ) {
    *sign = 0;
    return rule_records1;
  }
  *sign = 1;
  return rule_records2;
}

bool
isame_rule_records_arr(struct rule_records_arr *rule_records1, struct rule_records_arr *rule_records2){ 
  if(!rule_records2 || !rule_records1)
    return false;
  if (rule_records1->nrules != rule_records2->nrules)
    return false;
  for (int i = 0; i < rule_records1->nrules; i++)
    if (rule_records1->rules[i].sw_idx != rule_records2->rules[i].sw_idx || rule_records1->rules[i].idx != rule_records2->rules[i].idx )
        return false;
  return true;
}

BDD//from new rule_records, if has same free the new rule_records
mtbdd_maketnode_1r(struct rule_records_arr *rule_records){
  register MTBddValue *node;
  register unsigned int hash;
  register int res;


  /* Try to find an existing node of this kind */
  hash = VALUEHASH_K(rule_records->nrules, rule_records->rules[0].sw_idx, rule_records->rules[0].idx);
  res = mtbddvalues[hash].hash;
  while(res != 0) {
    // if (LEVEL(res) == level  &&  LOW(res) == low  &&  HIGH(res) == high)
    if (isame_rule_records_arr(rule_records, mtbddvalues[res].rule_records)){
      free(rule_records);
      return res;
    }
    res = mtbddvalues[res].next;
  }

  /* No existing node -> build one */
  /* Any free nodes to use ? */
  // if (bddfreepos == 0)

  if (mtbddfreepos == -1) {
    bdd_error(BDD_NODENUM);
    bdderrorcond = abs(BDD_NODENUM);
    return 0;
  }

  /* Build new node */
  res = mtbddfreepos;
  mtbddfreepos = mtbddvalues[mtbddfreepos].next;
  mtbddfreenum--;
  // bddproduced++;
  node = &mtbddvalues[res];
  node->rule_records = rule_records;

  node->next = mtbddvalues[hash].hash;
  mtbddvalues[hash].hash = res;

  res = bdd_makenode(0, 1, res);

  // mtbddvaluevaluecount ++;
  return res;
}

BDD//from new rule's parameter, may have the same, here the same means have same insc_bdd and same 1 ridx. other condition will be have same insc_bdd
mtbdd_maketnode_fr_pofr(BDD bdd_ofr, uint32_t sw_idx, uint32_t idx){
  register MTBddValue *node;
  register unsigned int hash;
  register int res;


  /* Try to find an existing node of this kind */
  hash = VALUEHASH_K(bdd_ofr, sw_idx, idx);
  // hash = VALUEHASH_K(1, sw_idx, idx);
  res = mtbddvalues[hash].hash;
  while(res != 0) {
    if (bdd_ofr ==  mtbddvalues[res].original_bdd && sw_idx == mtbddvalues[res].main_rule->sw_idx  &&  idx == mtbddvalues[res].main_rule->idx)
    // if (bdd_ofr ==  mtbddvalues[res].main_rule->original_bdd && sw_idx == mtbddvalues[res].main_rule->sw_idx  &&  idx == mtbddvalues[res].main_rule->idx)
      return bdd_makenode(0, 2, res);
    res = mtbddvalues[res].next;
  }
  
  /* No existing node -> build one */
  /* Any free nodes to use ? */
  // if (bddfreepos == 0)

  if (mtbddfreepos == -1) {
    bdd_error(BDD_NODENUM);
    bdderrorcond = abs(BDD_NODENUM);
    return 0;
  }

  /* Build new node */
  res = mtbddfreepos;
  mtbddfreepos = mtbddvalues[mtbddfreepos].next;
  mtbddfreenum--;
  // bddproduced++;
  node = &mtbddvalues[res];

  /* Fullfill the new node */
  node->original_bdd = bdd_ofr;
  node->self_bdd = bdd_ofr;
  bdd_addref(node->self_bdd);
  node->coveredby_bdd = BDDZERO;
  node->covering_bdd = BDDZERO;
  node->main_rule = malloc(sizeof(*(node->main_rule)));
  node->main_rule->sw_idx = sw_idx;
  node->main_rule->idx = idx;
  node->rule_records = NULL;
  // node->rule_records = rule_records;

  /* Insert new node to hashtable */
  node->next = mtbddvalues[hash].hash;
  mtbddvalues[hash].hash = res;

  res = bdd_makenode(0, 2, res);

  mtbddvaluevaluecount ++;
  return res;
}

//tnode1为基底， tnode2为添加r的
BDD 
mtbdd_maketnode_fr2spec(BDD tnode1, BDD tnode2){
  int rule_records_sign = 0;
  struct rule_records_arr *tmp = gen_rule_records_arr_fr2spec(mtbddvalues[(bddnodes[tnode1].high)].rule_records, mtbddvalues[(bddnodes[tnode2].high)].rule_records, &rule_records_sign);
  // struct rule_records_arr *tmp = gen_rule_records_arr_fr2spec_simple(mtbddvalues[(bddnodes[tnode1].high)].rule_records, mtbddvalues[(bddnodes[tnode2].high)].rule_records, &rule_records_sign);
  if (!rule_records_sign) 
    return tnode1;
  // return tnode2;
  return mtbdd_maketnode_1r(tmp);
}



BDD
mtbdd_maketnode_fr_2tn_newv(BDD tnode1, BDD tnode2){
  // register MTBddValue *v1 = &mtbddvalues[(bddnodes[tnode1].high)];
  // register MTBddValue *v2 = &mtbddvalues[(bddnodes[tnode2].high)];
  MTBddValue *v1 = &mtbddvalues[(bddnodes[tnode1].high)];
  MTBddValue *v2 = &mtbddvalues[(bddnodes[tnode2].high)];
  /*先留sw_idx不同的问题，在这里先不考虑来实验，只考虑rule 的idx来区分节点
  也就是，在同一个bdd下会发现生成了两个不同的mtbddvalue，那么怎样放到同一个中
  先默认sw_idx相同*/
  
  // if (v1->main_rule->idx <= v2->main_rule->idx) {
  //   return tnode1;
  // }
  // if (v1->main_rule )
  // {
  //   /* code */
  // }
  if (v1->self_bdd == v2->self_bdd)
    return tnode1;
  BDD insc = bdd_apply(v1->self_bdd, v2->self_bdd, bddop_and);
  if (insc){
    if (v1->self_bdd == insc)
      return tnode1;
    if (v2->self_bdd == insc)
      return tnode2;
    bdd_delref(v1->self_bdd);
    v1->self_bdd = bdd_apply(v1->self_bdd, insc, bddop_diff);
    bdd_addref(v1->self_bdd);
    bdd_delref(v2->self_bdd);
    v2->self_bdd = bdd_apply(v2->self_bdd, insc, bddop_diff);
    bdd_addref(v2->self_bdd);
    
    return mtbdd_maketnode_fr_pofr(insc, v1->main_rule->sw_idx, v1->main_rule->idx);
  }

  return tnode1;
}



BDD
mtbdd_maketnode_fr_2tn_add_simple(BDD tnode1, BDD tnode2){
  // register MTBddValue *v1 = &mtbddvalues[(bddnodes[tnode1].high)];
  // register MTBddValue *v2 = &mtbddvalues[(bddnodes[tnode2].high)];
  MTBddValue *v1 = &mtbddvalues[(bddnodes[tnode1].high)];
  MTBddValue *v2 = &mtbddvalues[(bddnodes[tnode2].high)];
  /*先留sw_idx不同的问题，在这里先不考虑来实验，只考虑rule 的idx来区分节点
  也就是，在同一个bdd下会发现生成了两个不同的mtbddvalue，那么怎样放到同一个中
  先默认sw_idx相同*/
  
  if (v1->main_rule->idx <= v2->main_rule->idx) {
    // hit_cache_counter++;
    // if (v1->self_bdd == v2->self_bdd) {
    //   v1->test_count++;
    //   v2->test_count++;
    //   bdd_delref(v2->self_bdd);
    //   v2->self_bdd = BDDZERO;
    //   //v1->self_bdd not change;

    //   bdd_delref(v2->coveredby_bdd);
    //   v2->coveredby_bdd = v1->self_bdd;
    //   bdd_addref(v2->coveredby_bdd);

    //   bdd_delref(v1->covering_bdd);
    //   v1->covering_bdd = v1->self_bdd;
    //   bdd_addref(v1->covering_bdd);
    //   return tnode1;
    // }
    BDD insc = bdd_apply(v1->self_bdd, v2->self_bdd, bddop_and);
    // if(insc){
    //   v1->test_count++;
    //   v2->test_count++;
    // }
    // bdd_delref(v2->self_bdd);
    v2->self_bdd = bdd_apply(v2->self_bdd, v1->self_bdd, bddop_or); 
    // bdd_addref(v2->self_bdd);
    /* v1->self_bdd not change */

    // bdd_delref(v2->coveredby_bdd);
    // v2->coveredby_bdd = bdd_apply(v2->coveredby_bdd, insc, bddop_xor);
    // bdd_addref(v2->coveredby_bdd);

    // bdd_delref(v1->covering_bdd);
    // v1->covering_bdd = bdd_apply(v1->covering_bdd, insc, bddop_xor);;
    // bdd_addref(v1->covering_bdd);
    return tnode1;
  }
  // BDD insc = bdd_apply(v1->self_bdd, v2->self_bdd, bddop_and);
  // if (v1->self_bdd == v2->self_bdd) {
    // v1->test_count++;
    // v2->test_count++;
    // bdd_delref(v1->self_bdd);
    // v1->self_bdd = BDDZERO;
    // v2->self_bdd not change;

  //   bdd_delref(v1->coveredby_bdd);
  //   v1->coveredby_bdd = v2->self_bdd;
  //   bdd_addref(v1->coveredby_bdd);

  //   bdd_delref(v2->covering_bdd);
  //   v2->covering_bdd = v2->self_bdd;
  //   bdd_addref(v2->covering_bdd);
  //   return tnode2;
  // }
  // BDD insc = bdd_apply(v1->self_bdd, v2->self_bdd, bddop_and);
  // if(insc){
  //   v1->test_count++;
  //   v2->test_count++;
  // }
  // bdd_delref(v1->self_bdd);
  // v1->self_bdd = bdd_apply(v1->self_bdd, v2->self_bdd, bddop_diff);
  // bdd_addref(v1->self_bdd);
  /* v2->self_bdd not change */

  // bdd_delref(v1->coveredby_bdd);
  // v1->coveredby_bdd = bdd_apply(v1->coveredby_bdd, insc, bddop_xor);
  // bdd_addref(v1->coveredby_bdd);

  // bdd_delref(v2->covering_bdd);
  // v2->covering_bdd = bdd_apply(v2->covering_bdd, insc, bddop_xor);
  // bdd_addref(v2->covering_bdd);
  // calc_node_counter++;
  return tnode2;
}

int bdd_noderesize(int doRehash) {
  BddNode *newnodes;
  int oldsize = bddnodesize;
  int n;

  if (bddnodesize >= bddmaxnodesize  &&  bddmaxnodesize > 0)
    return -1;

  bddnodesize = bddnodesize << 1;

  if (bddnodesize > oldsize + bddmaxnodeincrease)
    bddnodesize = oldsize + bddmaxnodeincrease;

  if (bddnodesize > bddmaxnodesize  &&  bddmaxnodesize > 0)
    bddnodesize = bddmaxnodesize;

  bddnodesize = bdd_prime_lte(bddnodesize);

  if (resize_handler != NULL)
    resize_handler(oldsize, bddnodesize);

  newnodes = (BddNode*)realloc(bddnodes, sizeof(BddNode)*bddnodesize);
  if (newnodes == NULL)
    return bdd_error(BDD_MEMORY);
  bddnodes = newnodes;

  if (doRehash)
    for (n=0 ; n<oldsize ; n++)
  bddnodes[n].hash = 0;

  for (n=oldsize ; n<bddnodesize ; n++)
  {
    bddnodes[n].refcou = 0;
    bddnodes[n].hash = 0;
    // LEVEL(n) = 0;
    bddnodes[n].level = 0;
    LOW(n) = -1;
    bddnodes[n].next = n+1;
  }
  bddnodes[bddnodesize-1].next = bddfreepos;
  bddfreepos = oldsize;
  bddfreenum += bddnodesize - oldsize;

  if (doRehash)
    bdd_gbc_rehash();

  bddresized = 1;

  return 0;
}

void bdd_checkreorder(void) {
  bdd_reorder_auto();

    /* Do not reorder before twice as many nodes have been used */
  usednodes_nextreorder = 2 * (bddnodesize - bddfreenum);

    /* And if very little was gained this time (< 20%) then wait until
     * even more nodes (upto twice as many again) have been used */
  if (bdd_reorder_gain() < 20)
    usednodes_nextreorder +=
  (usednodes_nextreorder * (20-bdd_reorder_gain())) / 20;
}

int bdd_scanset(BDD r, int **varset, int *varnum) {
  int n, num;

  CHECK(r);
  if (r < 2)
  {
    *varnum = 0;
    *varset = NULL;
    return 0;
  }

  for (n=r, num=0 ; n > 1 ; n=HIGH(n))
    num++;

  if (((*varset) = (int *)malloc(sizeof(int)*num)) == NULL)
    return bdd_error(BDD_MEMORY);

  for (n=r, num=0 ; n > 1 ; n=HIGH(n))
    (*varset)[num++] = bddlevel2var[bddnodes[n].level];

  *varnum = num;

  return 0;
}

BDD bdd_makeset(int *varset, int varnum) {
  int v, res=1;

  for (v=varnum-1 ; v>=0 ; v--) {
    BDD tmp;
    bdd_addref(res);
    tmp = bdd_apply(res, bdd_ithvar(varset[v]), bddop_and);
    bdd_delref(res);
    res = tmp;
  }

  return res;
}

/* In file fdd.c */
/*=====================================================================================================*/
static void fdd_printset_rec(FILE *, int, int *);

typedef struct s_Domain {
  int realsize;   /* The specified domain (0...N-1) */
  int binsize;    /* The number of BDD variables representing the domain */
  int *ivar;      /* Variable indeces for the variable set */
  BDD var;        /* The BDD variable set */
} Domain;

static void Domain_allocate(Domain*, int);
static void Domain_done(Domain*);

static int    firstbddvar;
static int    fdvaralloc;         /* Number of allocated domains */
static int    fdvarnum;           /* Number of defined domains */
static Domain *domain;            /* Table of domain sizes */

static bddfilehandler filehandler;

void bdd_fdd_init(void) {
  domain = NULL;
  fdvarnum = fdvaralloc = 0;
  firstbddvar = 0;
}

void bdd_fdd_done(void) {
  int n;

  if (domain != NULL) {
    for (n=0 ; n<fdvarnum ; n++)
      Domain_done(&domain[n]);
    free(domain);
  }

  domain = NULL;
}

int fdd_extdomain(int *dom, int num) {
  int offset = fdvarnum;
  int binoffset;
  int extravars = 0;
  int n, bn, more;

  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

    /* Build domain table */
  if (domain == NULL) { /* First time */
    fdvaralloc = num;
    if ((domain=(Domain*)malloc(sizeof(Domain)*num)) == NULL)
      return bdd_error(BDD_MEMORY);
  }
  else { /* Allocated before */
    if (fdvarnum + num > fdvaralloc) {
      fdvaralloc += (num > fdvaralloc) ? num : fdvaralloc;

      domain = (Domain*)realloc(domain, sizeof(Domain)*fdvaralloc);
      if (domain == NULL)
      return bdd_error(BDD_MEMORY);
    }
  }

    /* Create bdd variable tables */
  for (n=0 ; n<num ; n++) {
    Domain_allocate(&domain[n+fdvarnum], dom[n]);
    extravars += domain[n+fdvarnum].binsize;
  }

  binoffset = firstbddvar;
  if (firstbddvar + extravars > bddvarnum)
    bdd_setvarnum(firstbddvar + extravars);

    /* Set correct variable sequence (interleaved) */
  for (bn=0,more=1 ; more ; bn++) {
    more = 0;

    for (n=0 ; n<num ; n++)
      if (bn < domain[n+fdvarnum].binsize) {
        more = 1;
        domain[n+fdvarnum].ivar[bn] = binoffset++;
      }
  }

  for (n=0 ; n<num ; n++) {
    domain[n+fdvarnum].var = bdd_makeset(domain[n+fdvarnum].ivar,
  			   domain[n+fdvarnum].binsize);
    bdd_addref(domain[n+fdvarnum].var);
  }

  fdvarnum += num;
  firstbddvar += extravars;

  return offset;
}

int fdd_overlapdomain(int v1, int v2) {
  Domain *d;
  int n;

  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  if (v1 < 0  ||  v1 >= fdvarnum  ||  v2 < 0  ||  v2 >= fdvarnum)
    return bdd_error(BDD_VAR);

  if (fdvarnum + 1 > fdvaralloc) {
    fdvaralloc += fdvaralloc;
    
    domain = (Domain*)realloc(domain, sizeof(Domain)*fdvaralloc);
    if (domain == NULL)
      return bdd_error(BDD_MEMORY);
  }

  d = &domain[fdvarnum];
  d->realsize = domain[v1].realsize * domain[v2].realsize;
  d->binsize = domain[v1].binsize + domain[v2].binsize;
  d->ivar = (int *)malloc(sizeof(int)*d->binsize);

  for (n=0 ; n<domain[v1].binsize ; n++)
    d->ivar[n] = domain[v1].ivar[n];
  for (n=0 ; n<domain[v2].binsize ; n++)
    d->ivar[domain[v1].binsize+n] = domain[v2].ivar[n];

  d->var = bdd_makeset(d->ivar, d->binsize);
  bdd_addref(d->var);

  return fdvarnum++;
}

void fdd_clearall(void) {
  bdd_fdd_done();
  bdd_fdd_init();
}

int fdd_domainnum(void) {
  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  return fdvarnum;
}

int fdd_domainsize(int v) {
  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  if (v < 0  ||  v >= fdvarnum)
    return bdd_error(BDD_VAR);
  return domain[v].realsize;
}

int fdd_varnum(int v) {
  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  if (v >= fdvarnum  ||  v < 0)
    return bdd_error(BDD_VAR);
  return domain[v].binsize;
}

int *fdd_vars(int v) {
  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return NULL;
  }
  if (v >= fdvarnum  ||  v < 0) {
    bdd_error(BDD_VAR);
    return NULL;
  }
  return domain[v].ivar;
}

BDD fdd_ithvar(int var, int val) {
  int n;
  int v=1, tmp;

  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return bddfalse;
  }
  if (var < 0  ||  var >= fdvarnum) {
    bdd_error(BDD_VAR);
    return bddfalse;
  }
  if (val < 0  ||  val >= domain[var].realsize) {
    bdd_error(BDD_RANGE);
    return bddfalse;
  }
  for (n=0 ; n<domain[var].binsize ; n++) {
    bdd_addref(v);
    if (val & 0x1)
      tmp = bdd_apply(bdd_ithvar(domain[var].ivar[n]), v, bddop_and);
    else
      tmp = bdd_apply(bdd_nithvar(domain[var].ivar[n]), v, bddop_and);
    bdd_delref(v);
    v = tmp;
    val >>= 1;
  }

  return v;
}

int fdd_scanvar(BDD r, int var) {
  int *allvar;
  int res;

  CHECK(r);
  if (r == bddfalse)
    return -1;
  if (var < 0  ||  var >= fdvarnum)
    return bdd_error(BDD_VAR);

  allvar = fdd_scanallvar(r);
  res = allvar[var];
  free(allvar);

  return res;
}

int* fdd_scanallvar(BDD r) {
  int n;
  char *store;
  int *res;
  BDD p = r;

  CHECKa(r,NULL);
  if (r == bddfalse)
    return NULL;

  store = NEW(char,bddvarnum);
  for (n=0 ; n<bddvarnum ; n++)
    store[n] = 0;

  while (!ISCONST(p)) {
    if (!ISZERO(LOW(p))) {
      store[bddlevel2var[bddnodes[p].level]] = 0;
      p = LOW(p);
    }
    else {
      store[bddlevel2var[bddnodes[p].level]] = 1;
      p = HIGH(p);
    }
  }

  res = NEW(int, fdvarnum);

  for (n=0 ; n<fdvarnum ; n++) {
    int m;
    int val=0;
    
    for (m=domain[n].binsize-1 ; m>=0 ; m--)
  if ( store[domain[n].ivar[m]] )
    val = val*2 + 1;
       else
    val = val*2;
    
    res[n] = val;
  }

  free(store);

  return res;
}

BDD fdd_ithset(int var) {
  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return bddfalse;
  }
  if (var < 0  ||  var >= fdvarnum) {
    bdd_error(BDD_VAR);
    return bddfalse;
  }
  return domain[var].var;
}

BDD fdd_domain(int var) {
  int n,val;
  Domain *dom;
  BDD d; 
  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return bddfalse;
  }
  if (var < 0  ||  var >= fdvarnum) {
    bdd_error(BDD_VAR);
    return bddfalse;
  }

  /* Encode V<=X-1. V is the variables in 'var' and X is the domain size */

  dom = &domain[var];
  val = dom->realsize-1;
  d = bddtrue;

  for (n=0 ; n<dom->binsize ; n++) {
    BDD tmp;
    if (val & 0x1)
      tmp = bdd_apply( bdd_nithvar(dom->ivar[n]), d, bddop_or );
    else
      tmp = bdd_apply( bdd_nithvar(dom->ivar[n]), d, bddop_and );

    val >>= 1;

    bdd_addref(tmp);
    bdd_delref(d);
    d = tmp;
  }

  return d;
}

BDD fdd_equals(int left, int right) {
  BDD e = bddtrue, tmp1, tmp2;
  int n;
  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return bddfalse;
  }

  if (left < 0  ||  left >= fdvarnum  ||  right < 0  ||  right >= fdvarnum) {
    bdd_error(BDD_VAR);
    return bddfalse;
  }
  if (domain[left].realsize != domain[right].realsize) {
    bdd_error(BDD_RANGE);
    return bddfalse;
  }

  for (n=0 ; n<domain[left].binsize ; n++) {
    tmp1 = bdd_addref( bdd_apply(bdd_ithvar(domain[left].ivar[n]),
  		   bdd_ithvar(domain[right].ivar[n]),
  		   bddop_biimp) );
    
    tmp2 = bdd_addref( bdd_apply(e, tmp1, bddop_and) );
    bdd_delref(tmp1);
    bdd_delref(e);
    e = tmp2;
  }

  bdd_delref(e);
  return e;
}

bddfilehandler fdd_file_hook(bddfilehandler h) {
  bddfilehandler old = filehandler;
  filehandler = h;
  return old;
}

void fdd_printset(BDD r) {
  CHECKn(r);
  fdd_fprintset(stdout, r);
}

void fdd_fprintset(FILE *ofile, BDD r) {
  int *set;

  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return;
  }

  if (r < 2) {
    fprintf(ofile, "%s", r == 0 ? "F" : "T");
    return;
  }

  set = (int *)malloc(sizeof(int)*bddvarnum);
  if (set == NULL) {
    bdd_error(BDD_MEMORY);
    return;
  }

  memset(set, 0, sizeof(int) * bddvarnum);
  fdd_printset_rec(ofile, r, set);
  free(set);
}

static void fdd_printset_rec(FILE *ofile, int r, int *set) {
  int n,m,i;
  int used = 0;
  int *var;
  int *binval;
  int ok, first;

  if (r == 0)
    return;
  else
  if (r == 1) {
    fprintf(ofile, "<");
    first=1;

    for (n=0 ; n<fdvarnum ; n++) {
      int firstval=1;
      used = 0;
      for (m=0 ; m<domain[n].binsize ; m++)
        if (set[domain[n].ivar[m]] != 0)
           used = 1;
      if (used) {
        if (!first)
           fprintf(ofile, ", ");
        first = 0;
        if (filehandler)
           filehandler(ofile, n);
        else
           fprintf(ofile, "%d", n);
        printf(":");
        var = domain[n].ivar;
        for (m=0 ; m<(1<<domain[n].binsize) ; m++) {
          binval = fdddec2bin(n, m);
          ok=1;
          for (i=0 ; i<domain[n].binsize && ok ; i++)
            if (set[var[i]] == 1  &&  binval[i] != 0)
               ok = 0;
            else
            if (set[var[i]] == 2  &&  binval[i] != 1)
               ok = 0;
          if (ok) {
            if (firstval)
              fprintf(ofile, "%d", m);
            else
              fprintf(ofile, "/%d", m);
            firstval = 0;
          }

           free(binval);
        }
      }
    }

    fprintf(ofile, ">");
  }
  else {
    set[bddlevel2var[bddnodes[r].level]] = 1;
    fdd_printset_rec(ofile, LOW(r), set);
    set[bddlevel2var[bddnodes[r].level]] = 2;
    fdd_printset_rec(ofile, HIGH(r), set);
    set[bddlevel2var[bddnodes[r].level]] = 0;
  }
}

int fdd_scanset(BDD r, int **varset, int *varnum) {
  int *fv, fn;
  int num,n,m,i;
    
  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  if ((n=bdd_scanset(r, &fv, &fn)) < 0)
    return n;

  for (n=0,num=0 ; n<fdvarnum ; n++) {
    int found=0;
    
    for (m=0 ; m<domain[n].binsize && !found ; m++) {
      for (i=0 ; i<fn && !found ; i++)
        if (domain[n].ivar[m] == fv[i]) {
           num++;
           found=1;
        }
    }
  }

  if ((*varset=(int*)malloc(sizeof(int)*num)) == NULL)
    return bdd_error(BDD_MEMORY);

  for (n=0,num=0 ; n<fdvarnum ; n++) {
    int found=0;
    
    for (m=0 ; m<domain[n].binsize && !found ; m++) {
      for (i=0 ; i<fn && !found ; i++)
        if (domain[n].ivar[m] == fv[i]) {
          (*varset)[num++] = n;
          found=1;
        }
    }
  }

  *varnum = num;

  return 0;
}

BDD fdd_makeset(int *varset, int varnum) {
  BDD res=bddtrue, tmp;
  int n;

  if (!bddrunning) {
    bdd_error(BDD_RUNNING);
    return bddfalse;
  }

  for (n=0 ; n<varnum ; n++)
    if (varset[n] < 0  ||  varset[n] >= fdvarnum) {
      bdd_error(BDD_VAR);
      return bddfalse;
    }

  for (n=0 ; n<varnum ; n++) {
    bdd_addref(res);
    tmp = bdd_apply(domain[varset[n]].var, res, bddop_and);
    bdd_delref(res);
    res = tmp;
  }

  return res;
}

int fdd_intaddvarblock(int first, int last, int fixed) {
  BDD res = bddtrue, tmp;
  int n, err;

  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  if (first > last ||  first < 0  ||  last >= fdvarnum)
    return bdd_error(BDD_VARBLK);

  for (n=first ; n<=last ; n++) {
    bdd_addref(res);
    tmp = bdd_apply(domain[n].var, res, bddop_and);
    bdd_delref(res);
    res = tmp;
  }

  err = bdd_addvarblock(res, fixed);

  bdd_delref(res);
  return err;
}

int fdd_setpair(bddPair *pair, int p1, int p2) {
  int n,e;

  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  if (p1<0 || p1>=fdvarnum || p2<0 || p2>=fdvarnum)
    return bdd_error(BDD_VAR);

  if (domain[p1].binsize != domain[p2].binsize)
    return bdd_error(BDD_VARNUM);

  for (n=0 ; n<domain[p1].binsize ; n++)
    if ((e=bdd_setpair(pair, domain[p1].ivar[n], domain[p2].ivar[n])) < 0)
  return e;

  return 0;
}

int fdd_setpairs(bddPair *pair, int *p1, int *p2, int size) {
  int n,e;

  if (!bddrunning)
    return bdd_error(BDD_RUNNING);

  for (n=0 ; n<size ; n++)
    if (p1[n]<0 || p1[n]>=fdvarnum || p2[n]<0 || p2[n]>=fdvarnum)
  return bdd_error(BDD_VAR);

  for (n=0 ; n<size ; n++)
    if ((e=fdd_setpair(pair, p1[n], p2[n])) < 0)
  return e;

  return 0;
}

static void Domain_done(Domain* d) {
  free(d->ivar);
  bdd_delref(d->var);
}

static void Domain_allocate(Domain* d, int range) {
  int calcsize = 2;

  if (range <= 0  || range > INT_MAX/2) {
    bdd_error(BDD_RANGE);
    return;
  }

  d->realsize = range;
  d->binsize = 1;

  while (calcsize < range) {
    d->binsize++;
    calcsize <<= 1;
  }

  d->ivar = (int *)malloc(sizeof(int)*d->binsize);
  d->var = bddtrue;
}

int *fdddec2bin(int var, int val) {
  int *res;
  int n = 0;

  res = (int *)malloc(sizeof(int)*domain[var].binsize);
  memset(res, 0, sizeof(int)*domain[var].binsize);

  while (val > 0) {
    if (val & 0x1)
      res[n] = 1;
    val >>= 1;
    n++;
  }

  return res;
}

/* In file bvec.c */
/*=====================================================================================================*/
#define DEFAULT(v) { v.bitnum=0; v.bitvec=NULL; }
typedef BVEC bvec;
typedef BDD bdd;

static bvec bvec_build(int bitnum, int isTrue) {
  bvec vec;
  int n;

  vec.bitvec = NEW(BDD,bitnum);
  vec.bitnum = bitnum;
  if (!vec.bitvec) {
    bdd_error(BDD_MEMORY);
    vec.bitnum = 0;
    return vec;
  }

  for (n=0 ; n<bitnum ; n++)
    if (isTrue)
  vec.bitvec[n] = BDDONE;
    else
  vec.bitvec[n] = BDDZERO;

  return vec;
}

bvec bvec_copy(bvec src) {
  bvec dst;
  int n;

  if (src.bitnum == 0) {
    DEFAULT(dst);
    return dst;
  }

  dst = bvec_build(src.bitnum,0);

  for (n=0 ; n<src.bitnum ; n++)
    dst.bitvec[n] = bdd_addref( src.bitvec[n] );
  dst.bitnum = src.bitnum;

  return dst;
}

bvec bvec_true(int bitnum) {
  return bvec_build(bitnum, 1);
}

bvec bvec_false(int bitnum) {
  return bvec_build(bitnum, 0);
}

bvec bvec_con(int bitnum, int val) {
  bvec v = bvec_build(bitnum,0);
  int n;

  for (n=0 ; n<v.bitnum ; n++) {
    if (val & 0x1)
      v.bitvec[n] = bddtrue;
    else
      v.bitvec[n] = bddfalse;

    val = val >> 1;
  }

  return v;
}

bvec bvec_var(int bitnum, int offset, int step) {
  bvec v;
  int n;

  v = bvec_build(bitnum,0);

  for (n=0 ; n<bitnum ; n++)
    v.bitvec[n] = bdd_ithvar(offset+n*step);

  return v;
}

bvec bvec_varfdd(int var) {
  bvec v;
  int *bddvar = fdd_vars(var);
  int varbitnum = fdd_varnum(var);
  int n;

  if (bddvar == NULL) {
    DEFAULT(v);
    return v;
  }

  v = bvec_build(varbitnum,0);

  for (n=0 ; n<v.bitnum ; n++)
    v.bitvec[n] = bdd_ithvar(bddvar[n]);

  return v;
}

bvec bvec_varvec(int bitnum, int *var) {
  bvec v;
  int n;

  v = bvec_build(bitnum,0);

  for (n=0 ; n<bitnum ; n++)
    v.bitvec[n] = bdd_ithvar(var[n]);

  return v;
}

bvec bvec_coerce(int bitnum, bvec v) {
  bvec res = bvec_build(bitnum,0);
  int minnum = MIN(bitnum, v.bitnum);
  int n;

  for (n=0 ; n<minnum ; n++)
    res.bitvec[n] = bdd_addref( v.bitvec[n] );

  return res;
}

int bvec_isconst(bvec e) {
  int n;

  for (n=0 ; n<e.bitnum ; n++)
    if (!ISCONST(e.bitvec[n]))
  return 0;

  return 1;
}

int bvec_val(bvec e) {
  int n, val=0;

  for (n=e.bitnum-1 ; n>=0 ; n--)
    if (ISONE(e.bitvec[n]))
  val = (val << 1) | 1;
    else if (ISZERO(e.bitvec[n]))
  val = val << 1;
    else
  return 0;

  return val;
}

void bvec_free(bvec v) {
  bvec_delref(v);
  free(v.bitvec);
}

bvec bvec_addref(bvec v) {
  int n;

  for (n=0 ; n<v.bitnum ; n++)
    bdd_addref(v.bitvec[n]);

  return v;
}

bvec bvec_delref(bvec v) {
  int n;

  for (n=0 ; n<v.bitnum ; n++)
    bdd_delref(v.bitvec[n]);

  return v;
}

bvec bvec_map1(bvec a, BDD (*fun)(BDD)) {
  bvec res;
  int n;

  res = bvec_build(a.bitnum,0);
  for (n=0 ; n < a.bitnum ; n++)
    res.bitvec[n] = bdd_addref( fun(a.bitvec[n]) );

  return res;
}

bvec bvec_map2(bvec a, bvec b, BDD (*fun)(BDD,BDD)) {
  bvec res;
  int n;

  DEFAULT(res);
  if (a.bitnum != b.bitnum) {
    bdd_error(BVEC_SIZE);
    return res;
  }

  res = bvec_build(a.bitnum,0);
  for (n=0 ; n < a.bitnum ; n++)
    res.bitvec[n] = bdd_addref( fun(a.bitvec[n], b.bitvec[n]) );

  return res;
}

bvec bvec_map3(bvec a, bvec b, bvec c, BDD (*fun)(BDD,BDD,BDD)) {
  bvec res;
  int n;

  DEFAULT(res);
  if (a.bitnum != b.bitnum  ||  b.bitnum != c.bitnum) {
    bdd_error(BVEC_SIZE);
    return res;
  }

  res = bvec_build(a.bitnum,0);
  for (n=0 ; n < a.bitnum ; n++)
    res.bitvec[n] = bdd_addref( fun(a.bitvec[n], b.bitvec[n], c.bitvec[n]) );

  return res;
}

bvec bvec_add(bvec l, bvec r) {
  bvec res;
  BDD c = bddfalse;
  int n;


  if (l.bitnum == 0  ||  r.bitnum == 0) {
    DEFAULT(res);
    return res;
  }

  if (l.bitnum != r.bitnum) {
    bdd_error(BVEC_SIZE);
    DEFAULT(res);
    return res;
  }

  res = bvec_build(l.bitnum,0);

  for (n=0 ; n<res.bitnum ; n++) {
    BDD tmp1, tmp2, tmp3;

       /* bitvec[n] = l[n] ^ r[n] ^ c; */
    tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_xor) );
    tmp2 = bdd_addref( bdd_apply(tmp1, c, bddop_xor) );
    bdd_delref(tmp1);
    res.bitvec[n] = tmp2;

       /* c = (l[n] & r[n]) | (c & (l[n] | r[n])); */
    tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_or) );
    tmp2 = bdd_addref( bdd_apply(c, tmp1, bddop_and) );
    bdd_delref(tmp1);
    
    tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_and) );
    tmp3 = bdd_addref( bdd_apply(tmp1, tmp2, bddop_or) );
    bdd_delref(tmp1);
    bdd_delref(tmp2);
    
    bdd_delref(c);
    c = tmp3;
  }

  bdd_delref(c);

  return res;
}

bvec bvec_sub(bvec l, bvec r) {
  bvec res;
  BDD c = bddfalse;
  int n;

  if (l.bitnum == 0  ||  r.bitnum == 0) {
    DEFAULT(res);
    return res;
  }

  if (l.bitnum != r.bitnum) {
    bdd_error(BVEC_SIZE);
    DEFAULT(res);
    return res;
  }

  res = bvec_build(l.bitnum,0);

  for (n=0 ; n<res.bitnum ; n++) {
    BDD tmp1, tmp2, tmp3;

       /* bitvec[n] = l[n] ^ r[n] ^ c; */
    tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_xor) );
    tmp2 = bdd_addref( bdd_apply(tmp1, c, bddop_xor) );
    bdd_delref(tmp1);
    res.bitvec[n] = tmp2;

       /* c = (l[n] & r[n] & c) | (!l[n] & (r[n] | c)); */
    tmp1 = bdd_addref( bdd_apply(r.bitvec[n], c, bddop_or) );
    tmp2 = bdd_addref( bdd_apply(l.bitvec[n], tmp1, bddop_less) );
    bdd_delref(tmp1);
    
    tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_and) );
    tmp3 = bdd_addref( bdd_apply(tmp1, c, bddop_and) );
    bdd_delref(tmp1);

    tmp1 = bdd_addref( bdd_apply(tmp3, tmp2, bddop_or) );
    bdd_delref(tmp2);
    bdd_delref(tmp3);
    
    bdd_delref(c);
    c = tmp1;
  }

  bdd_delref(c);

  return res;
}

bvec bvec_mulfixed(bvec e, int c) {
  bvec res, next, rest;
  int n;

  if (e.bitnum == 0) {
    DEFAULT(res);
    return res;
  }

  if (c == 0)
    return bvec_build(e.bitnum,0);  /* return false array (base case) */

  next = bvec_build(e.bitnum,0);
  for (n=1 ; n<e.bitnum ; n++)
    /* e[] is never deleted, so no ref.cou. */
    next.bitvec[n] = e.bitvec[n-1];    
    
  rest = bvec_mulfixed(next, c>>1);

  if (c & 0x1) {
    res = bvec_add(e, rest);
    bvec_free(rest);
  }
  else
    res = rest;

  bvec_free(next);

  return res;
}

bvec bvec_mul(bvec left, bvec right) {
  int n;
  int bitnum = left.bitnum + right.bitnum;
  bvec res;
  bvec leftshifttmp;
  bvec leftshift;

  if (left.bitnum == 0  ||  right.bitnum == 0) {
    DEFAULT(res);
    return res;
  }

  res = bvec_false(bitnum);
  leftshifttmp = bvec_copy(left);
  leftshift = bvec_coerce(bitnum, leftshifttmp);

  /*bvec_delref(leftshifttmp);*/
  bvec_free(leftshifttmp);

  for (n=0 ; n<right.bitnum ; n++) {
    bvec added = bvec_add(res, leftshift);
    int m;

    for (m=0 ; m<bitnum ; m++) {
      bdd tmpres = bdd_addref( bdd_ite(right.bitvec[n],
                   added.bitvec[m], res.bitvec[m]) );
      bdd_delref(res.bitvec[m]);
      res.bitvec[m] = tmpres;
    }

       /* Shift 'leftshift' one bit left */
    bdd_delref(leftshift.bitvec[leftshift.bitnum-1]);
    for (m=bitnum-1 ; m>=1 ; m--)
      leftshift.bitvec[m] = leftshift.bitvec[m-1];
    leftshift.bitvec[0] = bddfalse;
    
    /*bvec_delref(added);*/
    bvec_free(added);
  }

  /*bvec_delref(leftshift);*/
  bvec_free(leftshift);

  return res;
}

static void bvec_div_rec(bvec divisor, bvec *remainder, bvec *result, int step) {
  int n;
  BDD isSmaller = bdd_addref( bvec_lte(divisor, *remainder) );
  bvec newResult = bvec_shlfixed( *result, 1, isSmaller );
  bvec zero = bvec_build(divisor.bitnum, bddfalse);
  bvec newRemainder, tmp, sub = bvec_build(divisor.bitnum, bddfalse);

  for (n=0 ; n<divisor.bitnum ; n++)
    sub.bitvec[n] = bdd_ite(isSmaller, divisor.bitvec[n], zero.bitvec[n]);

  tmp = bvec_sub( *remainder, sub );
  newRemainder = bvec_shlfixed(tmp, 1, result->bitvec[divisor.bitnum-1]);

  if (step > 1)
    bvec_div_rec( divisor, &newRemainder, &newResult, step-1 );

  bvec_free(tmp);
  bvec_free(sub);
  bvec_free(zero);
  bdd_delref(isSmaller);

  bvec_free(*remainder);
  bvec_free(*result);
  *result = newResult;
  *remainder = newRemainder;
}

int bvec_divfixed(bvec e, int c, bvec *res, bvec *rem) {
  if (c > 0) {
    bvec divisor = bvec_con(e.bitnum, c);
    bvec tmp = bvec_build(e.bitnum, 0);
    bvec tmpremainder = bvec_shlfixed(tmp, 1, e.bitvec[e.bitnum-1]);
    bvec result = bvec_shlfixed(e, 1, bddfalse);
    bvec remainder;
    
    bvec_div_rec(divisor, &tmpremainder, &result, divisor.bitnum);
    remainder = bvec_shrfixed(tmpremainder, 1, bddfalse);

    bvec_free(tmp);
    bvec_free(tmpremainder);
    bvec_free(divisor);
    
    *res = result;
    *rem = remainder;
    
    return 0;
  }

  return bdd_error(BVEC_DIVZERO);
}

int bvec_div(bvec left, bvec right, bvec *result, bvec *remainder) {
  int n, m;
  int bitnum = left.bitnum + right.bitnum;
  bvec res;
  bvec rem;
  bvec div, divtmp;

  if (left.bitnum == 0  ||  right.bitnum == 0  ||
     left.bitnum != right.bitnum) {
    return bdd_error(BVEC_SIZE);
  }

  rem = bvec_coerce(bitnum, left);
  divtmp = bvec_coerce(bitnum, right);
  div = bvec_shlfixed(divtmp, left.bitnum, bddfalse);

  /*bvec_delref(divtmp);*/
  bvec_free(divtmp);

  res = bvec_false(right.bitnum);

  for (n=0 ; n<right.bitnum+1 ; n++) {
    bdd divLteRem = bdd_addref( bvec_lte(div, rem) );
    bvec remSubDiv = bvec_sub(rem, div);

    for (m=0 ; m<bitnum ; m++) {
      bdd remtmp = bdd_addref( bdd_ite(divLteRem,
                   remSubDiv.bitvec[m],rem.bitvec[m]) );
      bdd_delref( rem.bitvec[m] );
      rem.bitvec[m] = remtmp;
    }

    if (n > 0)
      res.bitvec[right.bitnum-n] = divLteRem;

       /* Shift 'div' one bit right */
    bdd_delref(div.bitvec[0]);
    for (m=0 ; m<bitnum-1 ; m++)
      div.bitvec[m] = div.bitvec[m+1];
    div.bitvec[bitnum-1] = bddfalse;

    /*bvec_delref(remSubDiv);*/
    bvec_free(remSubDiv);
  }

  /*bvec_delref(*result);*/
  bvec_free(*result);
  /*bvec_delref(*remainder);*/
  bvec_free(*remainder);

  *result = res;
  *remainder = bvec_coerce(right.bitnum, rem);

  /*bvec_delref(rem);*/
  bvec_free(rem);

  return 0;
}

bvec bvec_ite(bdd a, bvec b, bvec c) {
  bvec res;
  int n;

  DEFAULT(res);
  if (b.bitnum != c.bitnum) {
    bdd_error(BVEC_SIZE);
    return res;
  }

  res = bvec_build(b.bitnum, 0);

  for (n=0 ; n<b.bitnum ; ++n) {
    res.bitvec[n] = bdd_addref( bdd_ite(a, b.bitvec[n], c.bitvec[n]) );
  }

  return res;
}

bvec bvec_shlfixed(bvec e, int pos, BDD c) {
  bvec res;
  int n, minnum = MIN(e.bitnum,pos);
  if (pos < 0) {
    bdd_error(BVEC_SHIFT);
    DEFAULT(res);
    return res;
  }
  if (e.bitnum == 0) {
    DEFAULT(res);
    return res;
  }

  res = bvec_build(e.bitnum,0);
  for (n=0 ; n<minnum ; n++)
    res.bitvec[n] = bdd_addref(c);
  for (n=minnum ; n<e.bitnum ; n++)
    res.bitvec[n] = bdd_addref(e.bitvec[n-pos]);

  return res;
}

BVEC bvec_shl(BVEC l, BVEC r, BDD c) {
  BVEC res, val;
  bdd tmp1, tmp2, rEquN;
  int n, m;
  if (l.bitnum == 0  ||  r.bitnum == 0) {
    DEFAULT(res);
    return res;
  }
  res = bvec_build(l.bitnum, 0);

  for (n=0 ; n<=l.bitnum ; n++) {
    val = bvec_con(r.bitnum, n);
    rEquN = bdd_addref( bvec_equ(r, val) );
    
    for (m=0 ; m<l.bitnum ; m++) {
        /* Set the m'th new location to be the (m+n)'th old location */
      if (m-n >= 0)
         tmp1 = bdd_addref( bdd_and(rEquN, l.bitvec[m-n]) );
      else
         tmp1 = bdd_addref( bdd_and(rEquN, c) );
      tmp2 = bdd_addref( bdd_or(res.bitvec[m], tmp1) );
      bdd_delref(tmp1);

      bdd_delref(res.bitvec[m]);
      res.bitvec[m] = tmp2;
    }

    bdd_delref(rEquN);
    /*bvec_delref(val);*/
    bvec_free(val);
  }
    /* At last make sure 'c' is shiftet in for r-values > l-bitnum */
  val = bvec_con(r.bitnum, l.bitnum);
  rEquN = bvec_gth(r, val);
  tmp1 = bdd_addref( bdd_and(rEquN, c) );

  for (m=0 ; m<l.bitnum ; m++) {
    tmp2 = bdd_addref( bdd_or(res.bitvec[m], tmp1) );
    
    bdd_delref(res.bitvec[m]);
    res.bitvec[m] = tmp2;
  }
  bdd_delref(tmp1);
  bdd_delref(rEquN);
  /*bvec_delref(val);*/
  bvec_free(val);
  return res;
}

bvec bvec_shrfixed(bvec e, int pos, BDD c) {
  bvec res;
  int n, maxnum = MAX(0,e.bitnum-pos);
  if (pos < 0) {
    bdd_error(BVEC_SHIFT);
    DEFAULT(res);
    return res;
  }
  if (e.bitnum == 0) {
    DEFAULT(res);
    return res;
  }
  res = bvec_build(e.bitnum,0);
  for (n=maxnum ; n<e.bitnum ; n++)
    res.bitvec[n] = bdd_addref(c);
  for (n=0 ; n<maxnum ; n++)
    res.bitvec[n] = bdd_addref(e.bitvec[n+pos]);
  return res;
}

BVEC bvec_shr(BVEC l, BVEC r, BDD c) {
  BVEC res, val;
  bdd tmp1, tmp2, rEquN;
  int n, m;

  if (l.bitnum == 0  ||  r.bitnum == 0) {
    DEFAULT(res);
    return res;
  }

  res = bvec_build(l.bitnum, 0);

  for (n=0 ; n<=l.bitnum ; n++) {
    val = bvec_con(r.bitnum, n);
    rEquN = bdd_addref( bvec_equ(r, val) );
    
    for (m=0 ; m<l.bitnum ; m++) {
      /* Set the m'th new location to be the (m+n)'th old location */
      if (m+n <= 2)
         tmp1 = bdd_addref( bdd_and(rEquN, l.bitvec[m+n]) );
      else
         tmp1 = bdd_addref( bdd_and(rEquN, c) );
      tmp2 = bdd_addref( bdd_or(res.bitvec[m], tmp1) );
      bdd_delref(tmp1);

      bdd_delref(res.bitvec[m]);
      res.bitvec[m] = tmp2;
    }

    bdd_delref(rEquN);
    /*bvec_delref(val);*/
    bvec_free(val);
  }

  /* At last make sure 'c' is shiftet in for r-values > l-bitnum */
  val = bvec_con(r.bitnum, l.bitnum);
  rEquN = bvec_gth(r, val);
  tmp1 = bdd_addref( bdd_and(rEquN, c) );

  for (m=0 ; m<l.bitnum ; m++) {
    tmp2 = bdd_addref( bdd_or(res.bitvec[m], tmp1) );
    
    bdd_delref(res.bitvec[m]);
    res.bitvec[m] = tmp2;
  }

  bdd_delref(tmp1);
  bdd_delref(rEquN);
  /*bvec_delref(val);*/
  bvec_free(val);

  return res;
}

bdd bvec_lth(bvec l, bvec r) {
  BDD p = bddfalse;
  int n;

  if (l.bitnum == 0  ||  r.bitnum == 0)
    return bddfalse;

  if (l.bitnum != r.bitnum) {
    bdd_error(BVEC_SIZE);
    return p;
  }

  for (n=0 ; n<l.bitnum ; n++) {
    /* p = (!l[n] & r[n]) |
     * bdd_apply(l[n], r[n], bddop_biimp) & p; */
    
    BDD tmp1 = bdd_addref(bdd_apply(l.bitvec[n],r.bitvec[n],bddop_less));
    BDD tmp2 = bdd_addref(bdd_apply(l.bitvec[n],r.bitvec[n],bddop_biimp));
    BDD tmp3 = bdd_addref( bdd_apply(tmp2, p, bddop_and) );
    BDD tmp4 = bdd_addref( bdd_apply(tmp1, tmp3, bddop_or) );
    bdd_delref(tmp1);
    bdd_delref(tmp2);
    bdd_delref(tmp3);
    bdd_delref(p);
    p = tmp4;
  }

  return bdd_delref(p);
}

bdd bvec_lte(bvec l, bvec r) {
  BDD p = bddtrue;
  int n;
  if (l.bitnum == 0  ||  r.bitnum == 0)
    return bddfalse;
  if (l.bitnum != r.bitnum) {
    bdd_error(BVEC_SIZE);
    return p;
  }

  for (n=0 ; n<l.bitnum ; n++) {
    /* p = (!l[n] & r[n]) |
     *     bdd_apply(l[n], r[n], bddop_biimp) & p; */
    BDD tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_less) );
    BDD tmp2 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_biimp) );
    BDD tmp3 = bdd_addref( bdd_apply(tmp2, p, bddop_and) );
    BDD tmp4 = bdd_addref( bdd_apply(tmp1, tmp3, bddop_or) );
    bdd_delref(tmp1);
    bdd_delref(tmp2);
    bdd_delref(tmp3);
    bdd_delref(p);
    p = tmp4;
  }
  return bdd_delref(p);
}

bdd bvec_gth(bvec l, bvec r) {
  BDD tmp = bdd_addref( bvec_lte(l,r) );
  BDD p = bdd_not(tmp);
  bdd_delref(tmp);
  return p;
}

bdd bvec_gte(bvec l, bvec r) {
  BDD tmp = bdd_addref( bvec_lth(l,r) );
  BDD p = bdd_not(tmp);
  bdd_delref(tmp);
  return p;
}

bdd bvec_equ(bvec l, bvec r){
  BDD p = bddtrue;
  int n;
  if (l.bitnum == 0  ||  r.bitnum == 0)
    return bddfalse;
  if (l.bitnum != r.bitnum) {
    bdd_error(BVEC_SIZE);
    return p;
  }
  for (n=0 ; n<l.bitnum ; n++) {
    BDD tmp1, tmp2;
    tmp1 = bdd_addref( bdd_apply(l.bitvec[n], r.bitvec[n], bddop_biimp) );
    tmp2 = bdd_addref( bdd_apply(tmp1, p, bddop_and) );
    bdd_delref(tmp1);
    bdd_delref(p);
    p = tmp2;
  }
  return bdd_delref(p);
}

bdd bvec_neq(bvec l, bvec r) {
  BDD tmp = bdd_addref( bvec_equ(l,r) );
  BDD p = bdd_not(tmp);
  bdd_delref(tmp);
  return p;
}
