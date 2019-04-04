//集中include
#include <pthread.h>
#include <limits.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>  
#include <setjmp.h>
#include <math.h>
#include <time.h>
// #include "usedMTBDD.h"  
#include <sys/time.h>
#include <malloc.h>

//结构体或变量定义
//自定义


// #define SW_NUM 16

// #define FIELD_LEN 7 //128位bit
// #define MF_LEN 8 //128位bit， 8×16 standford whole

// #define FIELD_LEN 1 
// #define MF_LEN 2 //32位bit standford simple

#define SW_NUM 9
#define FIELD_LEN 2 //48位bit i2
#define MF_LEN 3 //48位bit i2


#define NW_DST_H 0
#define NW_DST_L 1
#define VALID_OFS 1
#define WILDCARD 0
#define VALUE 1
#define RULE_LINK_IN 0
#define RULE_LINK_OUT 1
#define IN_LINK 1
#define OUT_LINK 0


#if __x86_64 || __amd64 || _M_X64 //64位或32位
typedef uint64_t array_t; //8字节一字节8位,16位16进制
#else
typedef uint32_t array_t;
#endif

#define MAX_PREFIX 255
#define MAX_ARR_SIZE 10000
#define MAX_VAL_RATE 0.01

#define PACKED __attribute__ ((__packed__))

//util.h
#define ARR_LEN(A) ( sizeof (A) / sizeof *(A) )
#define DIV_ROUND_UP(X, A) ( ((X) + (A) - 1) / (A) )
#define ROUND_UP(X, A) ( ((X) + (A) - 1) & ~((A) - 1) )
#define PACKED __attribute__ ((__packed__))
#define QUOTE(S) QUOTE_ (S)
#define QUOTE_(S) #S

/* Memory allocation with error checking. */
#define xcalloc(N, SZ) xcalloc_ (N, SZ, __FILE__, __LINE__, __func__)
#define xmalloc(SZ) xmalloc_ (SZ, __FILE__, __LINE__, __func__)
#define xmemalign(A, SZ) xmemalign_ (A, SZ, __FILE__, __LINE__, __func__)
#define xmemdup(P, SZ) xmemdup_ (P, SZ, __FILE__, __LINE__, __func__)
#define xrealloc(P, SZ) xrealloc_ (P, SZ, __FILE__, __LINE__, __func__)
#define xstrdup(S) xstrdup_ (S, __FILE__, __LINE__, __func__)
#define ARR_PTR(T, ID) \
  struct arr_ptr_ ## ID { int n; union { T a[sizeof (T *) / sizeof (T)]; T *p; } e; }
#define ARR(X) ( (X).n > ARR_LEN ((X).e.a) ? (X).e.p : (X).e.a )
#define ARR_ALLOC(X, N) \
  do { (X).n = (N); if ((X).n > ARR_LEN ((X).e.a)) (X).e.p = xmalloc ((N) * sizeof *(X).e.p); } while (0)
#define ARR_FREE(X) \
  do { if ((X).n > ARR_LEN ((X).e.a)) free ((X).e.p); } while (0)

static inline int
int_cmp (const void *a, const void *b)
{ return *(int *)a - *(int *)b; }

static inline bool
int_find (uint32_t x, const uint32_t *a, int n) {
  int l = 0, r = n - 1;
  while (l <= r) {
    int m = (l + r) / 2;
    if (a[m] == x) return true;
    if (a[m] < x) l = m + 1;
    else r = m - 1;
  }
  return false;
}
static inline void *
xcalloc_ (size_t n, size_t size, const char *file, int line, const char *func) {
  void *p = calloc (n, size);
  if (!p) err (1, "%s:%d (%s): calloc() failed", file, line, func);
  return p;
}
static inline void *
xmalloc_ (size_t size, const char *file, int line, const char *func) {
  void *p = malloc (size);
  if (!p) err (1, "%s:%d (%s): malloc() failed", file, line, func);
  return p;
}
static inline void *
xmemalign_ (size_t align, size_t size, const char *file, int line, const char *func) {
  void *p;
  if ((errno = posix_memalign (&p, align, size)))
    err (1, "%s:%d (%s): malloc() failed", file, line, func);
  return p;
}
static inline void *
xmemdup_ (const void *src, size_t size, const char *file, int line, const char *func) {
  void *p = xmalloc_ (size, file, line, func);
  memcpy (p, src, size);
  return p;
}
static inline void *
xrealloc_ (void *p, size_t size, const char *file, int line, const char *func){
  p = realloc (p, size);
  if (!p) err (1, "%s:%d (%s): realloc() failed", file, line, func);
  return p;
}
static inline char *
xstrdup_ (const char *s, const char *file, int line, const char *func) {
  char *p = strdup (s);
  if (!p) err (1, "%s:%d (%s): strdup() failed", file, line, func);
  return p;
}
//array.h
#if __x86_64 || __amd64 || _M_X64 //64位或32位
typedef uint64_t array_t; //8字节一字节8位,16位16进制
#else
typedef uint32_t array_t;
#endif

enum bit_val { BIT_Z = 0, BIT_0, BIT_1, BIT_X, BIT_UNDEF };
#define ARRAY_BYTES(L) ( ROUND_UP (2 * (L), sizeof (array_t)) )

//tf.h
struct PACKED port_map {
  uint32_t n;
  struct PACKED port_map_elem {
    uint32_t port;
    uint32_t start;
  } elems[0];
};

struct PACKED deps {
  uint32_t n;
  struct PACKED dep {
    uint32_t rule;
    uint32_t match;
    int32_t port;
  } deps[0];
};

// struct PACKED ports {
//   uint32_t n;
//   uint32_t arr[0];
// };

struct PACKED rule {
  uint32_t idx;
  int32_t in, out;
  uint32_t match, mask, rewrite;
  uint32_t deps, desc;
};

struct PACKED tf { // 前缀,包含的规则
  uint32_t prefix;
  uint32_t nrules;
  uint32_t map_ofs;
  uint32_t ports_ofs, deps_ofs;
  struct rule rules[0];
};

//data.h
struct PACKED file {
  uint32_t arrs_ofs, strs_ofs;
  uint32_t sws_num, stages;
  uint32_t sw_ofs[0];
};

//data.h中需要,.h中用extern重新定义
struct file *data_file; //指针指向全局文件
uint8_t     *data_raw; //指针指向读取文件的副本映射
size_t       data_size;

struct link_file *link_data_file;
uint8_t *link_data_raw; //指针指向读取文件的副本映射
size_t link_data_size;

struct link_to_rule_file *link_in_rule_file;
uint8_t *link_in_rule_raw; //指针指向读取文件的副本映射
size_t link_in_rule_size;
uint32_t *link_in_rule_data_arrs;

struct link_to_rule_file *link_out_rule_file;
uint8_t *link_out_rule_raw; //指针指向读取文件的副本映射
size_t link_out_rule_size;
uint32_t *link_out_rule_data_arrs;

uint16_t *data_arrs;
uint32_t data_arrs_len, data_arrs_n;
char    *data_strs;

uint32_t *sws_r_num;
uint32_t data_allr_nums;

uint32_t computation_counter;//计数器，记录计算某某的数量
uint32_t compu_true_counter;//计数器，记录计算成功，需要后续常规处理的数量
uint32_t elemconnet_counter;//计数器，记录计算elem连结的数量
uint32_t global_sign; //限定，求得计算打印
uint32_t elem_true_counter;//计数器，记录计算elemtrue的数量
long int time_counter1;//时间计数器
long int time_counter2;
long int time_counter3;
long int time_counter4;
long int time_counter5;
long int time_counter_elemplus;
long int time_counter_elembdd_withpair;
long int time_counter_nf_space_connect;
long int time_counter_eleminsc;

uint16_t uint16_power_sign[16] = {0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};

struct PACKED arrs {
  uint32_t len, n;
  uint16_t arrs[0];
};

#define DATA_ARR(X) ( data_arrs + ((X) - VALID_OFS) / sizeof (array_t) )
#define DATA_STR(X) ( data_strs + ((X) - VALID_OFS) )

//hs.h
struct hs_vec {
  array_t **elems;
  struct hs_vec *diff;
  int used, alloc;
};

struct hs { //头空间,长度和链表
  int len;
  struct hs_vec list;
};

//list.h
#define LIST(T) struct list_ ## T { struct T *head, *tail; int n; }
#define list_append(L, E) \
  do { \
    (E)->next = NULL; \
    if (!(L)->tail) (L)->head = (E); else (L)->tail->next = (E); \
    (L)->tail = (E); \
    (L)->n++; \
  } while (0)
#define list_concat(A, B) \
  do { \
    if (!(B)->head) break; \
    if ((A)->tail) (A)->tail->next = (B)->head; \
    else (A)->head = (B)->head; \
    (A)->tail = (B)->tail; \
    (A)->n += (B)->n; \
  } while (0)
#define list_destroy(L, F) \
  do { \
    while ((L)->head) { \
      (L)->tail = (L)->head->next; \
      (F) ((L)->head); \
      (L)->head = (L)->tail; \
    } \
    (L)->n = 0; \
  } while (0)
#define list_pop(L) \
  do { \
    if ((L)->head->next) (L)->head = (L)->head->next; \
    else { (L)->head = (L)->tail = NULL; } \
    (L)->n--; \
  } while (0)
#define list_remove(L, C, P, F) \
  do { \
    if ((P)) (P)->next = (C)->next; \
    else (L)->head = (C)->next; \
    if ((L)->tail == (C)) (L)->tail = (P); \
    (F) ((C)); \
    (C) = (P) ? (P)->next : (L)->head; \
    (L)->n--; \
  } while (0)

//map.h
struct map_val {
  void *val;
  struct map_val *next;
};

struct map_elem {
  int key;
  LIST (map_val) vals;
};

struct map {
  struct map_elem *elems;
  int alloc, used;
};

#define MAP_START_SIZE 10

static inline void
map_destroy (struct map *m) {
  for (int i = 0; i < m->used; i++)
    list_destroy (&m->elems[i].vals, free);
  free (m->elems);
}

static inline int
map_elem_cmp (const void *a, const void *b)
{ return ((struct map_elem *)a)->key - ((struct map_elem *)b)->key; }

static inline struct map_elem *
map_find (const struct map *m, int key) {
  struct map_elem tmp = {key};
  return bsearch (&tmp, m->elems, m->used, sizeof *m->elems, map_elem_cmp);
}

static inline struct map_elem *
map_find_create (struct map *m, int key) {
  struct map_elem *e = map_find (m, key);
  if (e) return e;

  if (m->used == m->alloc) {
    m->alloc = m->alloc ? 2 * m->alloc : MAP_START_SIZE;
    m->elems = xrealloc (m->elems, m->alloc * sizeof *m->elems);
  }
  e = &m->elems[m->used++];
  memset (e, 0, sizeof *e);
  e->key = key;
  qsort (m->elems, m->used, sizeof *m->elems, map_elem_cmp);
  return map_find (m, key);
}

//parse.h
struct parse_dep {
  struct parse_dep *next;
  int rule;
  array_t *match;
  int nports;
  uint32_t ports[0];
};

struct parse_rule {
  struct parse_rule *next;
  int idx;
  ARR_PTR(uint32_t, uint32_t) in, out, in_link, out_link;
  /**
  #define ARR_PTR(T, ID) \
  struct arr_ptr_ ## ID { int n; union { T a[sizeof (T *) / sizeof (T)]; T *p; } e; }
  ##连接字符串的作用
  结构体就是arr_ptr_uint32_t{int n，}
  union联合体类似结构体struct，struct所有变量是“共存”的，union中是各变量是“互斥”的
  共用一个内存首地址，并且各种变量名都可以同时使用，操作也是共同生效，也就是只体现一个值，可以兼容不同类型
  不过这些“手段”之间却没法互相屏蔽——就好像数组+下标和指针+偏移一样
  在此中放数组或者指针e，n为数量
  **/
  // array_t *match;
  // array_t *mask, *rewrite;
  struct mf_uint16_t *match;
  struct mf_uint16_t *mask;
  struct mf_uint16_t *rewrite;
};

struct parse_sw {
  int len, nrules;
  char *prefix;
  LIST (parse_rule) rules;
  // struct map in_map;
};

struct parse_nsw {
  int sws_num;
  int stages;
  struct parse_sw *sws[0];
};

//res.h
struct tf;

struct res_rule {
  char *tf;
  int rule;
};

struct res {
  struct res *next, *parent;
  int refs;
  pthread_mutex_t lock;

  struct hs hs;
  uint32_t port;
  struct {
    int n, cur;
    struct res_rule arr[0];
  } rules;
};


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





//自己定义
struct PACKED wc_uint8_t {
  uint8_t w;
  uint8_t v;
};

struct PACKED wc_uint16_t {
  uint16_t w;
  uint16_t v;
};

struct PACKED wc_uint32_t {
  uint32_t w;
  uint32_t v;
};

struct PACKED range_uint16_t {
  uint16_t low_v;
  uint16_t high_v;
};

struct PACKED mf_uint16_t {
  uint16_t mf_w[MF_LEN];
  uint16_t mf_v[MF_LEN];
};

struct PACKED mf_uint8_t {
  uint8_t mf_w[MF_LEN*2];
  uint8_t mf_v[MF_LEN*2];
};

struct PACKED mask_uint16_t {
  uint16_t v[MF_LEN];
};

struct mf_uint16_t_array {
  uint32_t n_mfs;
  struct mf_uint16_t *mfs[0];
};

struct PACKED r_idx {
  uint32_t sw_idx;
  uint32_t r_idx;
};

struct PACKED r_idxs {
  uint32_t nrs;
  struct r_idx ridx[0];
};

struct PACKED u32_arrs {
  uint32_t ns;
  uint32_t arrs[0];
};

//with bdd
static uint16_t var2sign[16] = {
  0x8000, 0x4000, 0x2000, 0x1000,
  0x0800, 0x0400, 0x0200, 0x0100,
  0x0080, 0x0040, 0x0020, 0x0010, 
  0x0008, 0x0004, 0x0002, 0x0001
};
#define VAR2SIGN(a) (var2sign[(a%16)])
#define FRA2INT(a) ((int) (a))
#define REF(a)    (bddnodes[a].refcou)
#define BDDSIZE     100000000
#define BDDOPCHCHE  400000 

struct BddNode_saved {
  int var;
  int low;
  int high;
};

struct bdd_saved_arr {//当arr_num == 0,意味着只有0或者1节点。
  int arr_num;
  struct BddNode_saved bdd_s[0];
};

struct PACKED nf_space {//匹配域和位置
  BDD mf;
  // uint32_t nw_src, dl_src, dl_dst, dl_dst, dl_vlan, dl_vlan_pcp, tp_src , dl_type, nw_tos
  struct links_of_rule *lks;
};

struct PACKED nf_space_pair {
  struct nf_space *in;
  struct nf_space *out;
  struct mask_uint16_t *mask;
  struct mask_uint16_t *rewrite;
  struct r_idxs *r_arr;
};

struct PACKED of_rule {
  uint32_t sw_idx;
  uint32_t idx;
  // struct nf_space modify;
  // struct nf_space_pair *nf_ses[0];
  struct wc_uint32_t match;
  uint32_t mask;
  uint32_t rewrite;
  // struct wc_uint32_t rewrite;
  uint32_t in_link;
  uint32_t out_link;
};

struct PACKED arule_uint {
  struct wc_uint32_t match;
  uint32_t mask;
  struct wc_uint32_t rewrite;
  uint32_t in_link;
  uint32_t out_link;
};

struct PACKED link {
  uint16_t idx;
  uint32_t port_in;
  uint32_t port_out;
};

struct PACKED links_idx {
  uint32_t nlinks;
  struct link links[0];
};

struct PACKED rule_links {
  uint32_t nwcs;
  struct wc_uint16_t l_wcs[0];
};
//先位置，保存在文件再记录位置，最后位置保存

struct PACKED link_file {
  uint32_t swl_num, inl_num, outl_num;
  struct link links[0];
};

struct PACKED link_to_rule{
  uint32_t link_idx;
  // uint32_t sw_idx;
  uint32_t rule_nums;
};

struct PACKED link_to_rule_file{
  uint32_t swl_num, outnl_num, idx_num; //属于交换机链路，属于入网或者出网链路，总链路数量
  struct link_to_rule links[0];
};

struct PACKED sw {
  uint32_t sw_idx;
  uint32_t prefix;
  uint32_t nrules;
  // uint32_t map_ofs;
  // uint32_t ports_ofs, deps_ofs;
  uint32_t links_ofs;
  struct of_rule rules[0];
};

struct PACKED links_of_rule{
  uint32_t n;
  struct wc_uint16_t links_wc[0];
};

//parse.h
struct matrix_element {
  uint32_t npairs;
  BDD bdd_in;
  BDD bdd_out;
  struct nf_space_pair *nf_pairs[0];
};

struct matrix_buf {
  uint32_t nelems;
  struct matrix_element *elems[0];
};

//sparse 
struct CS_matrix_idx_v {
  uint32_t idx;
  struct matrix_element *elem;
};

struct CS_matrix_idx_v_arr {
  uint32_t nidx_vs;
  struct CS_matrix_idx_v *idx_vs[0];
};

struct matrix_CSR {
  uint32_t nrows;
  struct CS_matrix_idx_v_arr *rows[0];
};

struct matrix_CSC {
  uint32_t ncols;
  struct CS_matrix_idx_v_arr *cols[0];
};

struct matrix_Tri_express {
  uint32_t row_idx;
  uint32_t col_idx;
  struct matrix_element *elem;
};

struct Tri_arr {
  uint32_t nTris;
  struct matrix_Tri_express *arr[];
};



// new sw and trie
struct port_arr{
  uint32_t nports;
  uint32_t arr[0];
};

struct ex_rule {
  uint32_t sw_idx;
  uint32_t idx;
  struct mf_uint16_t *mf_in;
  struct mf_uint16_t *mf_out;
  struct mask_uint16_t *mask;
  struct mask_uint16_t *rewrite; 
  struct links_of_rule *lks_in;
  struct links_of_rule *lks_out;
  // struct port_arr *port_in;
  // struct port_arr *port_out;
};

struct ex_rules_arr {
  uint32_t nrules;
  struct ex_rule *rules[0];
};

struct switch_rs {
  uint32_t sw_idx;
  uint32_t nrules;
  struct ex_rule *rules[0];
};

struct trie_node {
  uint32_t level;
  uint32_t isleaf;
  struct trie_node *reverse_node;
  struct ex_rules_arr *local_rules;
  struct ex_rules_arr *relevent_rules;
  struct trie_node *branchs[3];
};

// new sw and bdd rules
struct bdd_rule {
  uint32_t sw_idx;
  uint32_t idx;
  BDD mf_in;
  BDD mf_out;
  BDD vtnode_in;
  BDD mtbdd_in;
  struct mask_uint16_t *mask;
  struct mask_uint16_t *rewrite; 
  struct links_of_rule *lks_in;
  struct links_of_rule *lks_out;
  // struct port_arr *port_in;
  // struct port_arr *port_out;
  
};

struct bdd_rules_arr {
  uint32_t nrules;
  struct bdd_rule *rules[0];
};

struct switch_bdd_rs {
  uint32_t sw_idx;
  uint32_t nrules;
  struct bdd_rule *rules[0];
};

struct bdd_arrs{
  uint32_t nbdds;
  uint32_t bdds[0];
};



/*=== SEMI-INTERNAL TYPES ===*/



struct rule_record {
  uint32_t sw_idx;
  uint32_t idx;
};

struct sws_r_record {
  struct bdd_rule *arrs[SW_NUM];
};

struct rule_records_arr {
  // uint32_t main_nrules;
  uint32_t nrules;
  struct rule_record rules[0];
};

struct AP_record {
  BDD ap_bdd;
  struct bdd_rule *r;
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
  // struct rule_record *main_rule;
  // struct sw_r_record *sws_main_rule;
  struct bdd_rule *main_rule;
  struct bdd_rule *main_rule_sws[SW_NUM];
  // struct bdd_rule *main_rule;
  struct rule_records_arr *rule_records;
} MTBddValue;

//veriflow
struct ecs {
  uint32_t necs;
  struct ec *ecs[0];
};



struct trie_node_arr{
  uint32_t nnodes;
  struct trie_node *nodes[0];
};

struct ec_1f {
  uint32_t l_b;
  uint32_t h_b;
};

struct ec {
  struct ec_1f ec_vs[FIELD_LEN];
};

struct ec_1f_node {
  uint32_t idx;
  uint32_t l_b;
  uint32_t h_b;
  struct trie_node *node;
};

struct bound {
  uint32_t bound;
  uint32_t idx;
};


//函数声明
//time
static inline int64_t
diff (struct timeval *a, struct timeval *b)
{
  int64_t x = (int64_t)a->tv_sec * 1000000 + a->tv_usec;
  int64_t y = (int64_t)b->tv_sec * 1000000 + b->tv_usec;
  return x - y;
}
int 
timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y) {
  /** 
    * 计算两个时间的间隔，得到时间差 
    * @param struct timeval* resule 返回计算出来的时间 
    * @param struct timeval* x 需要计算的前一个时间 
    * @param struct timeval* y 需要计算的后一个时间 
    * return -1 failure ,0 success 
  **/ 
  // int nsec;    
  if ( x->tv_sec>y->tv_sec ) 
    return -1;   
  if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) ) 
    return -1;   
  result->tv_sec = ( y->tv_sec-x->tv_sec ); 
  result->tv_usec = ( y->tv_usec-x->tv_usec );     
  if (result->tv_usec<0) { 
    result->tv_sec--; 
    result->tv_usec+=1000000; 
  } 
  return 0; 
}


//array.h

/*得到相应结构数据*/
/*========================================================================*/
struct sw *sw_get(const uint32_t idx);
struct of_rule *rule_get(struct sw *sw, const int idx);
struct ex_rule *ex_rule_get(struct switch_rs *sw, const int idx);
struct of_rule *rule_get_2idx(const uint32_t sw_idx, const uint32_t r_idx);
struct links_of_rule *rule_links_get_2idx(struct sw *sw, const uint32_t idx, const uint32_t sign);
struct links_of_rule *rule_links_get(const struct of_rule *rule, const uint32_t sign);
struct link *link_get(const uint32_t idx);
struct link_to_rule *get_link_rules(struct link_to_rule_file *lr_file, uint32_t *rule_nums, const uint32_t idx);//通过idx查找link
struct u32_arrs *get_link_idx_from_inport (const uint32_t inport);
struct u32_arrs *get_outrules_idx_from_inport (const uint32_t inport);
struct mf_uint16_t *get_r_out_mf(const struct of_rule *rule);
struct mf_uint16_t *get_r_in_mf (const struct of_rule *rule);
struct mf_uint16_t_array *get_r_in_mf_to_array(const struct of_rule *rule);//将r 得到mf_uint16_t *，转换为mf_uint16_t_array *
uint32_t matrix_idx_get_r(struct of_rule *r);
uint32_t matrix_idx_get_2idx(const uint32_t sw_idx, const uint32_t r_idx);
struct of_rule *matrix_idx_to_r(const uint32_t *matrix_idx);

/*打印结构数据*/
/*========================================================================*/
void print_link(const struct link *lk);
void print_link_file(void);
void print_u32_arrs(const struct u32_arrs *u32_arrs);
void print_wc(const uint16_t *w, const uint16_t *v);
void print_mask(const uint16_t *mask);
void print_links_of_rule(const struct links_of_rule *ls);
void print_link_to_rule(const uint32_t link_idx, const struct link_to_rule_file *lr_file, 
                    const uint32_t *lr_data_arrs);
void print_rule(const struct of_rule *rule);
void print_ex_rule(const struct ex_rule *rule);
void print_mf_uint16_t(const struct mf_uint16_t *a);
void print_mf_uint16_t_array(const struct mf_uint16_t_array *arr);
void print_mask_uint16_t(const struct mask_uint16_t *a);
void print_r_idxs(const struct r_idxs *rs);
void print_nf_space(const struct nf_space *a);
void print_nf_space_pair(const struct nf_space_pair *a);
void print_matrix_element(const struct matrix_element *elem);
uint32_t print_CSR_elem_from_idx(const uint32_t row_idx, const uint32_t col_idx, const struct matrix_CSR *matrix);
uint32_t print_CSC_elem_from_idx(const uint32_t row_idx, const uint32_t col_idx, const struct matrix_CSC *matrix);
void print_Tri_express(struct matrix_Tri_express *Tri);

//BDD
void print_node(BDD r);
void print_bdd_saved_arr(struct bdd_saved_arr *bdd_arr);

/*初始化动作*/
/*========================================================================*/
void data_load(const char *name);
void link_data_load(const char *dir);
void init_mf(struct mf_uint16_t *mf);
void init_mf_allx(struct mf_uint16_t *mf);
uint32_t *matrix_idx_init(void);
struct matrix_buf *matrix_init(void);

/*释放结构体*/
/*========================================================================*/
void data_unload(void);
void free_mf_uint16_t_array(struct mf_uint16_t_array *arr);
void free_insc_arr(struct mf_uint16_t **insc_arr, uint32_t *insc_count);
void free_nf_space(struct nf_space *a);
void free_nf_space_pair(struct nf_space_pair *pair);
void free_matrix_element(struct matrix_element *elem);
void free_matrix_buf(struct matrix_buf *matrix_buf);
void free_CS_matrix_idx_v(struct CS_matrix_idx_v *idx_v);
void free_CS_matrix_idx_v_arr(struct CS_matrix_idx_v_arr *idx_v_arr);
void free_matrix_CSC_fr_CSR(struct matrix_CSC *matrix_CSC);

/*矩阵操作*/
/*========================================================================*/


/*结构间计算*/
/*========================================================================*/
struct mf_uint16_t *calc_insc(const struct mf_uint16_t *a, const struct mf_uint16_t *b);//使用这个必须最后free掉 free(*)就可以
struct mf_uint16_t_array *calc_minus_insc(struct mf_uint16_t *a, struct mf_uint16_t *insc);//使用这个必须最后free掉 free(*)就可以
//稀疏矩阵使用
struct mf_uint16_t *merge_mf(struct mf_uint16_t *a, struct mf_uint16_t *b);
struct wc_uint16_t *wc_uint16_t_insc(struct wc_uint16_t *a, struct wc_uint16_t *b);
struct links_of_rule *links_insc(struct links_of_rule *a, struct links_of_rule *b);
// struct matrix_Tri_express *insc_to_Tri_express_rlimit(struct of_rule *r_in, struct of_rule *r_out, BDD v_and);

/*一些需求的比较函数和判断*/
/*========================================================================*/
static int uint32_t_cmp(const void *a, const void *b);
static int link_idx_cmp(const void *a, const void *b); //本头文件限定，bsearch函数要求的比较函数

bool is_mf_allx(struct mf_uint16_t *a);
uint32_t is_insc_wc_uint16_t(struct wc_uint16_t *a, struct wc_uint16_t *b); // 1:yes, 0:no
uint32_t is_insc_links(struct links_of_rule *a, struct links_of_rule *b); // 1:yes, 0:no
bool is_wc_uint16_t_same(struct wc_uint16_t *a, struct wc_uint16_t *b);
bool is_mask_uint16_t_same(struct mask_uint16_t *a, struct mask_uint16_t *b);
bool is_links_of_rule_same(struct links_of_rule *a, struct links_of_rule *b);
bool is_action_same(struct bdd_rule *a, struct bdd_rule *b);
//稀疏矩阵使用
int cmp_matrix_Tri_express(const void *a, const void *b);
int cmp_matrix_Tri_express_CSC(const void *a, const void *b);
bool Tri_is_eq(struct matrix_Tri_express *latter, struct matrix_Tri_express *former);
bool mf_can_merge(struct mf_uint16_t *a, struct mf_uint16_t *b);
//BDD使用
int cmp_bdd_by_var(const void *a, const void *b);

/*复制各个结构*/
/*========================================================================*/
struct links_of_rule *copy_links_of_rule(struct links_of_rule *lks);
struct mf_uint16_t *copy_mf_uint16_t(struct mf_uint16_t *mf);
struct mask_uint16_t *copy_mask_uint16_t(struct mask_uint16_t *mk);
struct bdd_saved_arr *copy_bdd_saved_arr(struct bdd_saved_arr *bdd_arr);

/*BDD相关各项处理*/
/*========================================================================*/
BDD mf2bdd_init(struct mf_uint16_t *mf);
BDD mf2bdd(struct mf_uint16_t *mf);
int add_node_2_mf(BDD node, BDD prenode,struct mf_uint16_t *mf, uint16_t v);
struct mf_uint16_t *genmf_fr_path(BDD node, struct mf_uint16_t *mf);
int back_node_2_mf(BDD node, struct mf_uint16_t *mf);
BDD bdd_ref(BDD root);
void bdd_save2stru(BDD root, BDD *r, int *count);
struct bdd_saved_arr *bdd_save_arr(BDD root);
BDD load_saved_bddarr(struct bdd_saved_arr *bdd_arr);
BDD bdd_mask2x(struct bdd_saved_arr *bdd_arr, struct mask_uint16_t *mask);
BDD rw2bdd(struct mask_uint16_t *mask, struct mask_uint16_t *rw);
struct bdd_saved_arr *bdd_rw(struct bdd_saved_arr *bdd_arr, struct mask_uint16_t *mask, struct mask_uint16_t *rw);
struct bdd_saved_arr *bdd_rw_back(struct bdd_saved_arr *bdd_arr, struct bdd_saved_arr *bdd_arr_IN, struct mask_uint16_t *mask);

/*普通矩阵处理及其计算*/
/*========================================================================*/

struct nf_space_pair *nf_space_connect(struct nf_space_pair *a, struct nf_space_pair *b);

/*稀疏矩阵处理及其计算*/
/*========================================================================*/
struct matrix_Tri_express *insc_to_Tri_express_rlimit(struct of_rule *r_in, struct of_rule *r_out, BDD v_and);
struct matrix_element *matrix_elem_plus(struct matrix_element *a, struct matrix_element *b);
void matrix_elem_plus_void(struct matrix_element *a, struct matrix_element *b);
void matrix_elem_plus_test(struct matrix_element *a);
struct matrix_CSR *gen_matrix_CSR_from_Tris(struct Tri_arr *Tri_arr);
struct Tri_arr *gen_Tri_arr_bdd(void);
struct matrix_CSR *gen_sparse_matrix(void);  //通过对链路文件查找两个同链路的头尾端规则，计算是否连通并添加到矩阵 gen_sparse_matrix(void);
struct matrix_CSC *gen_CSC_from_CSR(struct matrix_CSR *matrix);
struct matrix_element *elem_connect(struct matrix_element *a, struct matrix_element *b); //a*b,a作用b，不可交换 elem_connect(struct matrix_element *a, struct matrix_element *b, uint32_t check);
struct matrix_element *row_col_multiply(struct CS_matrix_idx_v_arr *row, struct CS_matrix_idx_v_arr *col);
struct CS_matrix_idx_v_arr *row_multi_col_multiply(struct CS_matrix_idx_v_arr *row, uint32_t *arr, uint32_t count, struct matrix_CSC *matrix_CSC);
struct CS_matrix_idx_v_arr *row_all_col_multiply(struct CS_matrix_idx_v_arr *row, struct matrix_CSC *matrix_CSC);
struct matrix_CSR *sparse_matrix_multiply(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1);
int get_value_num_matrix_CSR(struct matrix_CSR *matrix_CSR);
struct matrix_CSR *selected_rs_matrix_multiply(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC, struct u32_arrs *rs);
struct matrix_CSR *sparse_matrix_multiply_nsqure(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC);



//data.h
void data_load   (const char *file);
void data_unload (void);

//hs.h
struct hs *hs_create  (int len);
void       hs_destroy (struct hs *hs);
void       hs_free    (struct hs *hs);
//parse.h
//res.h
struct res *res_create   (int nrules);
void        res_free     (struct res *res);
//ntf.h
//app.h

//.c中函数具体定义
//array.c
//tf.c




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
  if (r == 1 || l == 1){
    printf("the wrong %d - %d\n", l, r);
    bdd_error(BDD_ILLBDD);
  }

  entry = BddCache_lookup(&applycache, APPLYHASH(l,r,14));
  if (entry->a == l  &&  entry->b == r  &&  entry->c == 14) {
    // hit_cache_counter++;
    return entry->r.res;
  }
  
  // if ((LOW(l) == 2) && (LOW(r) == 2))
  //   return mtbdd_maketnode_fr_2tn_add(l, r);

  if (LOW(l) == 2) {
    if ((LOW(r) == 2)) {
  // if ((LOW(l) == 2) && (LOW(r) == 2)) {
      res = mtbdd_maketnode_fr_2tn_add_simple(l, r);
    }
    else{
      res = bdd_makenode(LEVEL(r), mtbdd_apply_addr_rec(l, LOW(r)), mtbdd_apply_addr_rec(l, HIGH(r)));
    }
    
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
      PUSHREF( simplify_rec(LOW(f), LOW(d)) );
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
{ imatrixFPrint(mtx, stdout);}

void imatrixSet(imatrix *mtx, int a, int b)
{ mtx->rows[a][b/8] |= 1<<(b%8);}

void imatrixClr(imatrix *mtx, int a, int b)
{ mtx->rows[a][b/8] &= ~(1<<(b%8));}

int imatrixDepends(imatrix *mtx, int a, int b)
{ return mtx->rows[a][b/8] & (1<<(b%8));}

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

  // mtbddvaluevaluecount = 0;
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
    for (int i = 0; i < SW_NUM; i++)
      mtbddvalues[n].main_rule_sws[i] = NULL;
    // mtbddvalues[n].main_rule = NULL;
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

  for (int i = 0; i < mtbddvaluesize; i++){
    free(mtbddvalues[i].rule_records);
  }
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
  hash = VALUEHASH_K(1, sw_idx, idx);
  res = mtbddvalues[hash].hash;
  while(res != 0) {
    if (sw_idx == mtbddvalues[res].main_rule->sw_idx  &&  idx == mtbddvalues[res].main_rule->idx)
      return res;
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

  // mtbddvaluevaluecount ++;
  return res;
}

BDD//from new rule's parameter, may have the same, here the same means have same insc_bdd and same 1 ridx. other condition will be have same insc_bdd
mtbdd_maketnode_from_r(struct bdd_rule *r){
  register MTBddValue *node;
  register unsigned int hash;
  register int res;


  /* Try to find an existing node of this kind */
  hash = VALUEHASH_K(1, r->sw_idx, r->idx);
  res = mtbddvalues[hash].hash;
  while(res != 0) {
    bool issame = true;
    for (int i = 0; i < SW_NUM; i++){
      if (i == r->sw_idx){
        if (mtbddvalues[res].main_rule_sws[i]!=r){
          issame = false;
          break;
        }
      }
      else{
        if (mtbddvalues[res].main_rule_sws[i]!=NULL){
          issame = false;
          break;
        }
      }
    }
    if (issame)
      return bdd_makenode(0, 2, res);;
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
  // node->self_bdd = bdd_ofr;
  node->self_bdd = r->mf_in;
  bdd_addref(node->self_bdd);
  node->coveredby_bdd = BDDZERO;
  node->covering_bdd = BDDZERO;
  node->main_rule = r;
  // node->main_rule = malloc(sizeof(*(node->main_rule)));
  // node->main_rule->sw_idx = sw_idx;
  // node->main_rule->idx = idx;
  for (int i = 0; i < SW_NUM; i++)
    node->main_rule_sws[i] = NULL;
  node->main_rule_sws[r->sw_idx] = r;
  node->rule_records = NULL;
  // node->rule_records = rule_records;

  /* Insert new node to hashtable */
  node->next = mtbddvalues[hash].hash;
  mtbddvalues[hash].hash = res;

  res = bdd_makenode(0, 2, res);

  // mtbddvaluevaluecount ++;
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
mtbdd_maketnode_from_v(MTBddValue *v1, struct bdd_rule *r){
  register MTBddValue *node;
  register unsigned int hash;
  register int res;

  uint32_t sum_sw_idx = 0;
  uint32_t sum_idx = 0;
  for (int i = 0; i < SW_NUM; i++){
    if ((v1->main_rule_sws[i]) && (i != r->sw_idx)){
      sum_sw_idx += i;
      sum_idx += v1->main_rule_sws[i]->idx;
    }
  }
  sum_sw_idx += r->sw_idx;
  sum_idx += r->idx;



  /* Try to find an existing node of this kind */
  hash = VALUEHASH_K(1, sum_sw_idx, sum_idx);
  res = mtbddvalues[hash].hash;
  while(res != 0) {
    bool issame = true;
    for (int i = 0; i < SW_NUM; i++){
      if (i != r->sw_idx){
        if (v1->main_rule_sws[i] != mtbddvalues[res].main_rule_sws[i]){
          issame = false;
          break;
        }
      }
      else{
        if (r != mtbddvalues[res].main_rule_sws[i]){
          issame = false;
          break;
        }
      }
    }
    if (issame){
      // if (LOW(res) != 2){
      //   for (int i = 0; i < SW_NUM; i++) {
      //     printf("%d;", mtbddvalues[res].main_rule_sws[i]);

      //   }
      //   printf("mtbdd_maketnodehash %d\n", res);
      // }
      return bdd_makenode(0, 2, res);
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

  /* Fullfill the new node */
  node->self_bdd = BDDZERO;
  // bdd_addref(node->self_bdd);
  node->coveredby_bdd = BDDZERO;
  node->covering_bdd = BDDZERO;
  node->main_rule = NULL;

  for (int i = 0; i < SW_NUM; i++)
    node->main_rule_sws[i] = v1->main_rule_sws[i];
  node->main_rule_sws[r->sw_idx] = r;

  // node->main_rule = malloc(sizeof(*(node->main_rule)));
  // node->main_rule->sw_idx = sw_idx;
  // node->main_rule->idx = idx;
  node->rule_records = NULL;
  // node->rule_records = rule_records;

  /* Insert new node to hashtable */
  node->next = mtbddvalues[hash].hash;
  mtbddvalues[hash].hash = res;

  res = bdd_makenode(0, 2, res);

  if (LOW(res) != 2)
  {
    printf("mtbdd_maketnodev %d\n", res);
  }

  // mtbddvaluevaluecount ++;
  return res;
}

BDD
mtbdd_maketnode_fr_2tn_add_simple(BDD tnode1, BDD tnode2){
  // register MTBddValue *v1 = &mtbddvalues[(bddnodes[tnode1].high)];
  // register MTBddValue *v2 = &mtbddvalues[(bddnodes[tnode2].high)];
  if (tnode1 == tnode2) 
    return tnode1;

  MTBddValue *v1 = &mtbddvalues[(bddnodes[tnode1].high)];
  MTBddValue *v2 = &mtbddvalues[(bddnodes[tnode2].high)];
  /*先留sw_idx不同的问题，在这里先不考虑来实验，只考虑rule 的idx来区分节点
  也就是，在同一个bdd下会发现生成了两个不同的mtbddvalue，那么怎样放到同一个中
  先默认sw_idx相同*/
  if (v1->main_rule_sws[v2->main_rule->sw_idx]) {
    // if (v1->main_rule_sws[v2->main_rule->sw_idx]->idx <= v2->main_rule->idx)
      return tnode1;
  }
  BDD tmp = mtbdd_maketnode_from_v(v1, v2->main_rule);

  // if (tmp == 1 || LOW(tmp) == 1 || HIGH(tmp)  == 1 || LOW(tmp) != 2)
  // {
  //   printf("mtbdd_maketnode %d\n", tmp);
  // }
  
  // return mtbdd_maketnode_from_v(v1, v2->main_rule);
  return tmp;

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



struct sw *
sw_get (const uint32_t idx) {//得到tf,从data_file第idx个得到data_raw后面的位数
  assert (idx >= 0 && idx < data_file->sws_num);
  uint32_t ofs = data_file->sw_ofs[idx];
  return (struct sw *) (data_raw + ofs);
}

struct of_rule * //sw从0开始，r_idx为of_rule->idx从1开始
rule_get(struct sw *sw, const int idx) {
  struct of_rule *rule = &(sw->rules[idx-1]);
  return (rule);
}

struct ex_rule * //sw从0开始，r_idx为of_rule->idx从1开始
ex_rule_get(struct switch_rs *sw, const int idx) {
  struct ex_rule *rule = sw->rules[idx-1];
  return (rule);
}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
struct of_rule *
rule_get_2idx(const uint32_t sw_idx, const uint32_t r_idx) {//sw从0开始，r_idx为of_rule->idx从1开始
  struct sw *sw = sw_get (sw_idx);
  struct of_rule *rule = &(sw->rules[r_idx-1]);
  return (rule);
}

struct links_of_rule * //sign: IN_LINK 1, OUT_LINK 0
rule_links_get_2idx(struct sw *sw, const uint32_t idx, const uint32_t sign) { 
  struct links_of_rule *ls;
  struct of_rule *rule = &(sw->rules[idx-1]);//idx从1开始
  if (sign) {
    ls = (struct links_of_rule *)(rule->in_link + (char *)sw + sw->links_ofs);
  }
  else {
    ls = (struct links_of_rule *)(rule->out_link + (char *)sw + sw->links_ofs);
  }
  return (ls);
}

struct links_of_rule * //sign: IN_LINK 1, OUT_LINK 0
rule_links_get(const struct of_rule *rule, const uint32_t sign) { 
  struct links_of_rule *ls;
  struct sw *sw = sw_get(rule->sw_idx);
  if (sign) {
    if (!(rule->in_link))
      return NULL;
    ls = (struct links_of_rule *)(rule->in_link + (char *)sw + sw->links_ofs);
  }
  else {
    if (!(rule->out_link))
      return NULL;
    ls = (struct links_of_rule *)(rule->out_link + (char *)sw + sw->links_ofs);
  }
  return (ls);
}

struct link *
link_get(const uint32_t idx) { 
  struct link *lk = &(link_data_file->links[idx]);
  return (lk);
}

struct link_to_rule * //通过idx查找link
get_link_rules (struct link_to_rule_file *lr_file, uint32_t *rule_nums, const uint32_t idx) { 

  uint32_t *links = (uint32_t *)lr_file->links;
  uint32_t *b = (uint32_t *)bsearch(&idx, links, lr_file->swl_num, 2*sizeof(uint32_t), link_idx_cmp);

  if (b){

    // printf("%d - %d, ", *b, *(b+1));
    if (*b)
      *rule_nums = *(b+1) - *(b-1);
    else
      *rule_nums = *(b+1);
    return  (struct link_to_rule *)b;
  }
  else 
    return NULL;
}

struct link_to_rule * //通过idx查找link
get_inoutlink_rules (struct link_to_rule_file *lr_file, uint32_t *rule_nums, const uint32_t idx) { 

  uint32_t *links = (uint32_t *)lr_file->links;
  uint32_t *b = (uint32_t *)bsearch(&idx, links, lr_file->idx_num, 2*sizeof(uint32_t), link_idx_cmp);

  if (b){

    // printf("%d - %d, ", *b, *(b+1));
    if (*b)
      *rule_nums = *(b+1) - *(b-1);
    else
      *rule_nums = *(b+1);
    return  (struct link_to_rule *)b;
  }
  else 
    return NULL;
}

struct u32_arrs *
get_link_idx_from_inport (const uint32_t inport) {
  uint32_t links_num = link_data_file->swl_num + link_data_file->inl_num + link_data_file->outl_num;
  uint32_t count = 0;
  uint32_t arr_tmp[500];
  for (uint32_t i = 0; i < links_num; i++) {
    if (inport == link_data_file->links[i].port_out){
      arr_tmp[count] = link_data_file->links[i].idx;
      count ++;
    }
  }

  struct u32_arrs *tmp = xmalloc((count+1)*sizeof(uint32_t));
  tmp->ns = count;
  for (uint32_t i = 0; i < count; i++) {
    tmp->arrs[i] = arr_tmp[i];
  }
  return tmp;
}

struct u32_arrs *
get_outrules_idx_from_inport (const uint32_t inport) {
  uint32_t arrs_tmp[5000];
  uint32_t count = 0;
  struct u32_arrs *link_idx = get_link_idx_from_inport(inport);
  printf("link_idx = %d\n", link_idx->ns);
  uint32_t rule_nums_out = 0;
  printf("%d\n", link_idx->arrs[0]);
  for (uint32_t i = 0; i < link_idx->ns; i++) {
    struct link_to_rule *lout_r = get_inoutlink_rules(link_out_rule_file, &rule_nums_out, link_idx->arrs[i]);
    printf("idx: %d; \n", lout_r->link_idx);
    if (lout_r){
      printf("idx: %d; \n", lout_r->link_idx);
      uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*(lout_r->rule_nums - rule_nums_out));
      for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {
        arrs_tmp[count] = matrix_idx_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
        count ++;
        lout_arrs += 2;

      }  
    }
  }
  free(link_idx);
  
  struct u32_arrs *tmp = NULL;
  if(count) {
    qsort(arrs_tmp, count,sizeof (uint32_t), uint32_t_cmp);
    uint32_t arrs_unrep_tmp[5000];
    uint32_t count_unrep = 1;
    arrs_unrep_tmp[0] = arrs_tmp[0];
    uint32_t pre_one = arrs_tmp[0];
    for (int i = 1; i < count; i++) {
      if (arrs_tmp[i] != pre_one) {
        arrs_unrep_tmp[count_unrep] = arrs_tmp[i];
        count_unrep++;
        pre_one = arrs_tmp[i];
      }
    }

    tmp = xmalloc((count+1)*sizeof(uint32_t));
    tmp->ns = count_unrep;
    for (int i = 0; i < count_unrep; i++)
      tmp->arrs[i] = arrs_unrep_tmp[i];
  }
  return tmp;
}

void
print_link(const struct link *lk) {
  if(!lk){
    printf("The link is NULL\n");
    return;
  }
  printf("idx: %d;", lk->idx);
  printf(" ports: %d -- %d\n", lk->port_in, lk->port_out);
}

void
print_link_file(void) {
  uint32_t links_num = link_data_file->swl_num + link_data_file->inl_num + link_data_file->outl_num;
  for (int i = 0; i < links_num; i++) {
    printf("idx: %d;", link_data_file->links[i].idx);
    printf(" ports: %d -- %d\n", link_data_file->links[i].port_in, link_data_file->links[i].port_out);
  }
}


void
print_u32_arrs(const struct u32_arrs *u32_arrs) {
  for (int i = 0; i < u32_arrs->ns; i++) {
    printf("idx: %d;", u32_arrs->arrs[i]);
  }
  printf("\n");
}

void
print_wc(const uint16_t *w, const uint16_t *v) { 
  // printf("%d", w);
  // printf("%d\n", v);
  uint16_t flag_bit = 0x8000;
  
  for (int j = 0; j<8*sizeof(uint16_t)-1; j++)
  { 
    if (flag_bit & *w)
      printf("x");
    else
      printf("%d", (bool)(flag_bit & *v));
    flag_bit >>= 1;
  }
  if (flag_bit & *w)
      printf("x");
  else
    printf("%d", (bool)(flag_bit & *v));
  printf(",");
}

void
print_mask(const uint16_t *mask) {
  uint16_t flag_bit = 0x8000;
  for (int j = 0; j<sizeof(uint16_t)*8-1; j++)
  { 
    printf("%d", (bool)(flag_bit & *mask));
    flag_bit >>= 1;
  }
  printf("%d", (bool)(flag_bit & *mask));
}

void
print_links_of_rule(const struct links_of_rule *ls) {

  if(ls){
    for (int i = 0; i < ls->n; i++)
    { 
      // printf("%d - %d", ls->links_wc[i].w, ls->links_wc[i].v);
      print_wc(&(ls->links_wc[i].w), &(ls->links_wc[i].v));
      printf(", ");
    }
    printf("\n");
  }
  else
    printf("the links_of_rule is NULL\n");
}

void
print_link_to_rule (const uint32_t link_idx, const struct link_to_rule_file *lr_file, 
                    const uint32_t *lr_data_arrs)
{
  uint32_t idx_num = lr_file->idx_num;
  uint32_t rule_nums_pre = 0;
  uint32_t rule_nums = 0;
  // uint32_t *links = &(link_in_rule_file->links[0]);

  for (int i = 0; i < idx_num; i++)
  {
    // printf("%d;", lr_file->links[i].link_idx);
    if (link_idx == lr_file->links[i].link_idx)
    {
      rule_nums = lr_file->links[i].rule_nums;

      if (i != 0) 
        rule_nums_pre = lr_file->links[i - 1].rule_nums;
      break;
    }
  }
  
  rule_nums -= rule_nums_pre;
  printf("rule_nums:%d\n", rule_nums);
  uint32_t *arrs = (uint32_t *)(lr_data_arrs + 2*rule_nums_pre);
  printf("%d - %d\n", *lr_data_arrs, *(uint32_t *)(lr_data_arrs + 1));

  for (int i = 0; i < rule_nums; i++)
  {     
    printf("%d - ", *(uint32_t *)arrs);
    printf("%d;", *(uint32_t *)(arrs+1));
    arrs += 2;
  }
  printf("\n");
}

void
print_rule(const struct of_rule *rule) {
  int add_len = sizeof(uint16_t);
  printf("sw_idx:%d; ", rule->sw_idx);
  printf("r_idx:%d; ", rule->idx);

  printf("matchfield:");
  for (int i = 0; i < MF_LEN*add_len; i += add_len) {
    print_wc((uint16_t *)((uint8_t *)data_arrs+i+rule->match.w), (uint16_t *)((uint8_t *)data_arrs+i+rule->match.v));
  }
  printf("; ");
  if (rule->mask) {
    printf("mask:");
    for (int i = 0; i < MF_LEN*add_len; i += add_len)
      print_mask((uint16_t *)((uint8_t *)data_arrs+i+rule->mask));
    printf("; ");
    printf("modify:");
    for (int i = 0; i < MF_LEN*add_len; i += add_len)
      // print_wc((uint16_t *)((uint8_t *)data_arrs+i+rule->rewrite.w), (uint16_t *)((uint8_t *)data_arrs+i+rule->rewrite.v));
      print_mask((uint16_t *)((uint8_t *)data_arrs+i+rule->rewrite));
    printf("; ");
  }
  struct links_of_rule *lks_in = rule_links_get(rule, IN_LINK);
  struct links_of_rule *lks_out = rule_links_get(rule, OUT_LINK);
  printf("IN_LINK:");
  if (lks_in)
    print_links_of_rule(lks_in);
  else
    printf("NULL");
  printf("; ");
  printf("OUT_LINK:");
  if (lks_out)
    print_links_of_rule(lks_out);
  else
    printf("NULL");
  printf("\n");
}

void
print_ex_rule(const struct ex_rule *rule) {
  // int add_len = sizeof(uint16_t);
  printf("sw_idx:%d; ", rule->sw_idx);
  printf("r_idx:%d; ", rule->idx);
  printf("mf_in:");
  print_mf_uint16_t(rule->mf_in);
  printf("; ");
  printf("mf_out:");
  print_mf_uint16_t(rule->mf_out);
  printf("; ");
  if (rule->mask) {
    printf("mask:");
    print_mask_uint16_t(rule->mask);
    printf("; ");
    printf("rewrite:");
    print_mask_uint16_t(rule->rewrite);
    printf("; ");
  }
  printf("IN_LINK:");
  if (rule->lks_in)
    print_links_of_rule(rule->lks_in);
  else
    printf("NULL");
  printf("; ");
  printf("OUT_LINK:");
  if (rule->lks_out)
    print_links_of_rule(rule->lks_out);
  else
    printf("NULL");
  printf("\n");
}

void
print_mf_uint16_t (const struct mf_uint16_t *a) {
  for (int i = 0; i < MF_LEN; i++) 
    print_wc(&(a->mf_w[i]), &(a->mf_v[i]));
  printf("\n");
}

void
print_mf_uint16_t_array (const struct mf_uint16_t_array *arr) {
  for (int i = 0; i < arr->n_mfs; i++) 
    print_mf_uint16_t(arr->mfs[i]); 
  printf("\n");
}

void
print_mask_uint16_t (const struct mask_uint16_t *a){
  for (int i = 0; i < MF_LEN; i++) {
    print_mask(&(a->v[i]));
  }
  printf("\n");
}

void
print_r_idxs (const struct r_idxs *rs) {
  for (int i = 0; i < rs->nrs; i++) {
    printf("%d - %d, ", rs->ridx[i].sw_idx, rs->ridx[i].r_idx);
  }
  printf("\n");
}

void
print_nf_space(const struct nf_space *a) {
  printf("matchfield:");
  // print_mf_uint16_t(a->mf);
  printf("the links:");
  print_links_of_rule(a->lks); 
}

void
print_nf_space_pair(const struct nf_space_pair *a) {
  if (a->in) {
    printf("IN:");
    print_nf_space(a->in);
  }
  if (a->out) {
    printf("OUT:");
    print_nf_space(a->out);
  }
  if (a->mask) {
    printf("MASK:");
    print_mask_uint16_t(a->mask);
  }
  if (a->rewrite) {
    printf("REWRITE:");
    print_mask_uint16_t(a->rewrite);
  }
  if (a->r_arr) {
    printf("rule idxs:");
    print_r_idxs(a->r_arr);
  }
}

void
print_matrix_element(const struct matrix_element *elem) {
  if (elem){
    for (int i = 0; i < elem->npairs; i++) {
     print_nf_space_pair(elem->nf_pairs[i]);
    }
  }
  else
    printf("This element is empty!\n");
}

uint32_t
print_CSR_elem_from_idx(const uint32_t row_idx, const uint32_t col_idx, const struct matrix_CSR *matrix) {
  // printf("%d\n", matrix->nrows);
  if(matrix->rows[row_idx]){
    struct CS_matrix_idx_v_arr *idx_v = matrix->rows[row_idx];
    printf("row-col: %d - %d;", row_idx, col_idx);
    for (int i = 0; i < idx_v->nidx_vs; i++){
      if (col_idx == idx_v->idx_vs[i]->idx) {
        print_matrix_element(idx_v->idx_vs[i]->elem);
        return 1;
      }
    }
    printf("There is empty!\n");
    return 0;
  }
  printf("This row is empty!\n");
  return 0;
}

uint32_t
print_CSC_elem_from_idx(const uint32_t row_idx, const uint32_t col_idx, const struct matrix_CSC *matrix) {
  // printf("%d\n", matrix->nrows);
  if(matrix->cols[col_idx]){
    struct CS_matrix_idx_v_arr *idx_v = matrix->cols[col_idx];
    printf("row-col: %d - %d;", row_idx, col_idx);
    for (int i = 0; i < idx_v->nidx_vs; i++){
      if (row_idx == idx_v->idx_vs[i]->idx) {
        print_matrix_element(idx_v->idx_vs[i]->elem);
        return 1;
      }
    }
    printf("There is empty!\n");
    return 0;
  }
  printf("This row is empty!\n");
  return 0;
}


//data.c
void 
data_load(const char *name) {
  // int i;	
  int fd = open (name, O_RDONLY);
  if (fd < 0) err (1, "open(%s) failed", name);
  data_size = lseek (fd, 0, SEEK_END); //欲将读写位置移到文件尾,返回目前的读写位置, 也就是距离文件开头多少个字节
  assert (data_size >= 0);
  printf ("%d \n", (int)data_size);
  data_raw = mmap (NULL, data_size, PROT_READ, MAP_PRIVATE, fd, 0); //指针指向映射,按文件一一对应,每个指针对应一字节,未加工数据
  if (data_raw == MAP_FAILED) err (1, "mmap() failed");
  close (fd);
  data_file = (struct file *) data_raw; //指针指向强制化file的映射,提取data_raw每行前4字节重排
  struct arrs *arrs = (struct arrs *) (data_raw + data_file->arrs_ofs);//data_raw起始位置+arrs_ofs
  data_arrs_len = arrs->len;
  data_arrs_n = arrs->n;
  data_arrs = arrs->arrs;

  // printf("%x\n", data_arrs[4]);
  data_strs = (char *) (data_raw + data_file->strs_ofs);//data_raw起始位置+strs_ofs指向的是文件名位置，数组
  // printf("%s\n", data_strs);
}

void
link_data_load(const char *dir) {
  char buf[255];
  snprintf (buf, sizeof buf, "%s", dir);
  char *base = buf + strlen (buf); //base指向buf后面的部分
  strcpy (base, "link_idx.dat");//buf后面接上"/links_idx.dat"

  int fd = open (buf, O_RDONLY);
  if (fd < 0) err (1, "open(%s) failed", buf);  
  link_data_size = lseek (fd, 0, SEEK_END);
  assert (link_data_size >= 0);
  printf ("link_data_size:%d \n", (int)link_data_size);
  link_data_raw = mmap (NULL, link_data_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (link_data_raw == MAP_FAILED) err (1, "mmap() failed");
  close (fd);

  link_data_file = (struct link_file *)link_data_raw;

  *base = 0;
  strcpy (base , "link_out_rule.dat");
  fd = open (buf, O_RDONLY);
  if (fd < 0) err (1, "open(%s) failed", buf);
  link_out_rule_size = lseek (fd, 0, SEEK_END);
  assert (link_out_rule_size >= 0);
  printf ("link_out_rule_size:%d \n", (int)link_out_rule_size);
  link_out_rule_raw = mmap (NULL, link_out_rule_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (link_out_rule_raw == MAP_FAILED) err (1, "mmap() failed");
  close (fd);
  link_out_rule_file = (struct link_to_rule_file *)link_out_rule_raw;
  link_out_rule_data_arrs = (uint32_t *)(&(link_out_rule_file->links[0].link_idx)+2*link_out_rule_file->idx_num);

  // print_link_to_rule (1, link_out_rule_file, link_out_rule_data_arrs);
  *base = 0;
  strcpy (base , "link_in_rule.dat");
  fd = open (buf, O_RDONLY);
  if (fd < 0) err (1, "open(%s) failed", buf);
  link_in_rule_size = lseek (fd, 0, SEEK_END);
  assert (link_in_rule_size >= 0);
  printf ("link_in_rule_size:%d \n", (int)link_in_rule_size);
  link_in_rule_raw = mmap (NULL, link_in_rule_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (link_in_rule_raw == MAP_FAILED) err (1, "mmap() failed");
  close (fd);
  link_in_rule_file = (struct link_to_rule_file *)link_in_rule_raw;
  link_in_rule_data_arrs = (uint32_t *)(&(link_in_rule_file->links[0].link_idx)+2*link_in_rule_file->idx_num);
  // print_link_to_rule (5, link_in_rule_file, link_in_rule_data_arrs);
}

void
data_unload(void) { 
  munmap (data_raw, data_size); 
  munmap (link_data_raw, link_data_size);
  munmap (link_in_rule_raw, link_in_rule_size);
  munmap (link_out_rule_raw, link_out_rule_size); 
}

void
init_mf(struct mf_uint16_t *mf) {
  for (int i = 0; i < MF_LEN; i++){
    mf->mf_w[i] = 0x0000;
    mf->mf_v[i] = 0x0000;
  }
}

void
init_mf_allx(struct mf_uint16_t *mf) {

  for (int i = 0; i < MF_LEN; i++){
    mf->mf_w[i] = 0xffff;
    mf->mf_v[i] = 0x0000;
  }
}

bool
is_wc_uint16_t_same(struct wc_uint16_t *a, struct wc_uint16_t *b) {
  if (a == b) 
    return true;
  if (!a || !b)
    return false;
  if (a->v != b->v )
    return false;
  return true;
}

bool
is_mask_uint16_t_same(struct mask_uint16_t *a, struct mask_uint16_t *b) {
  if (a == b) 
    return true;
  if (!a || !b)
    return false;
  for (int i = 0; i < MF_LEN; i++)
    if (a->v[i] != b->v[i] )
      return false;
  return true;
}

bool
is_links_of_rule_same(struct links_of_rule *a, struct links_of_rule *b) {
  if (a == b) 
    return true;
  if (!a || !b)
    return false;
  if (a->n != b->n )
    return false;

  for (int i = 0; i < a->n; i++)
    if (!is_wc_uint16_t_same(&(a->links_wc[i]), &(b->links_wc[i])))
      return false;
  return true;
}



uint32_t *
matrix_idx_init(void) { //返回buf，全为0,长度为规则数
  uint32_t swn = data_file->sws_num;
  uint32_t init_0 = 0;
  char *buf;
  size_t bufsz;
  uint32_t sum_r = 0;
  FILE *f = open_memstream (&buf, &bufsz);
  sws_r_num = (uint32_t *)xcalloc(swn,sizeof(uint32_t));
  for (int i = 0; i < swn; i++) {//总的r数量
    struct sw *sw_t = sw_get(i);
    *(uint32_t *)(sws_r_num+i) = sum_r;
    sum_r += sw_t->nrules;
    
  }
  data_allr_nums = sum_r;
  for (int i = 0; i < sum_r; i++, init_0++) {
    fwrite (&init_0, sizeof init_0, 1, f);//初始化为0
  }
  fclose (f);
  printf("The all num of rs is %d\n", data_allr_nums);
  for  (int i = 0; i < swn; i++) {//总的r数量
    printf("%d-",*(uint32_t *)(sws_r_num+i));   
  }
  printf("\n");
  // printf("total rules :%d\n", sum_r);
  // printf("total rules bufsz:%d\n", bufsz/4);
  return (uint32_t *)buf;
}

struct of_rule * //出错返回-1
matrix_idx_to_r(const uint32_t *matrix_idx) {//从0开始计算
  uint32_t swn = data_file->sws_num;
  // if (!(*matrix_idx))
  //   return rule_get_2idx(0, 1);
  for (int i = 0; i < swn; i++) {
    if (*matrix_idx < *(uint32_t *)(sws_r_num+i))
      return rule_get_2idx(i-1, *matrix_idx - *(uint32_t *)(sws_r_num+i-1) +1);
  }
  if (*matrix_idx < data_allr_nums) {
    return rule_get_2idx(swn-1, *matrix_idx - *(uint32_t *)(sws_r_num+swn-1) +1);
  }
  return NULL;
}

uint32_t 
matrix_idx_get_r(struct of_rule *r)//得到的从0开始
{ return *(uint32_t *)(sws_r_num+r->sw_idx) + r->idx - 1;}

uint32_t //sw从0开始，r_idx为of_rule->idx从1开始
matrix_idx_get_2idx(const uint32_t sw_idx, const uint32_t r_idx)
{ return *(uint32_t *)(sws_r_num+sw_idx) + r_idx - 1;}

struct matrix_buf *
matrix_init (void) { //生成空矩阵
  uint32_t swn = data_file->sws_num;
  char init_0[sizeof (struct matrix_element *)];
  for (int size = 0; size<sizeof (struct matrix_element *); size++) {
    init_0[size] = 0;
  }
  char *buf;
  size_t bufsz;
  FILE *f = open_memstream (&buf, &bufsz);
  // struct of_rule rules[hdr.nrules];//hdr.nrules个rule空间指向各自rules
  // // //构体指针变量一般用”->”，非结构体指针变量，也就是一般结构体变量，一般用”.”。
  // memset (rules, 0, sizeof rules);//初始化0
  uint32_t sum_r = 0;
  for (int i = 0; i < swn; i++) {
    struct sw *sw_t = sw_get(i);
    sum_r += sw_t->nrules;
  }
  printf("sum_r%d\n", sum_r);
  fwrite (&sum_r, sizeof sum_r, 1, f);
  // int bufsz_check = 0; 
  for (int i = 0; i < sum_r; i++) {
    for (int j = 0; j < sum_r; j++) {
      fwrite (&init_0, 1, sizeof (struct matrix_element *), f);   
    }
  }
  fclose (f);
  // printf("total rules bufsz:%d\n", bufsz);
  return (struct matrix_buf *)buf;
}

void
free_mf_uint16_t_array (struct mf_uint16_t_array *arr) {
  for (int i = 0; i < arr->n_mfs; i++){
    if (arr->mfs[i]) 
      free(arr->mfs[i]);
  }
  free(arr);
}

void
free_insc_arr(struct mf_uint16_t **insc_arr, uint32_t *insc_count) {
  for (int i = 0; i < *insc_count; i++){
    free(insc_arr[i]);
  }
}

void
free_nf_space(struct nf_space *a) {
  if(a){
      // free(a->mf);
      bdd_delref(a->mf);
      free(a->lks);
      free(a);
  }
}

void
free_nf_space_pair(struct nf_space_pair *pair) {
  if (pair){
    if (pair->in)
      free_nf_space(pair->in);
    if (pair->out)
      free_nf_space(pair->out);
    if (pair->mask)
      free(pair->mask);
    if (pair->rewrite)
      free(pair->rewrite);
    if (pair->r_arr)
      free(pair->r_arr);
    free(pair);
  } 
}

void
free_matrix_element(struct matrix_element *elem) {
  if (elem){
    for (uint32_t i = 0; i < elem->npairs; i++){
      if (elem->nf_pairs[i]) 
        free_nf_space_pair(elem->nf_pairs[i]); 
    }
    bdd_delref(elem->bdd_in);
    bdd_delref(elem->bdd_out);
    free(elem);
  } 
}

void
free_matrix_buf(struct matrix_buf *matrix_buf) {
  for (uint32_t i = 0; i < matrix_buf->nelems; i++){
    if (matrix_buf->elems[i]){
      free_matrix_element(matrix_buf->elems[i]);
    }
  }
  free(matrix_buf);
}

void
free_CS_matrix_idx_v(struct CS_matrix_idx_v *idx_v) {
  if(idx_v){
    free_matrix_element(idx_v->elem);
    free(idx_v);
  }
}

void
free_CS_matrix_idx_v_arr(struct CS_matrix_idx_v_arr *idx_v_arr) {
  if(idx_v_arr){
    for (int i = 0; i < idx_v_arr->nidx_vs; i++)
      free_CS_matrix_idx_v(idx_v_arr->idx_vs[i]);
    free(idx_v_arr);
  }
}

void
free_matrix_CSR(struct matrix_CSR *matrix_CSR) {
  if(matrix_CSR){
    for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
      if (matrix_CSR->rows[i])
        free_CS_matrix_idx_v_arr(matrix_CSR->rows[i]);
    }
    free(matrix_CSR);
  }
}

void
free_matrix_CSC(struct matrix_CSC *matrix_CSC) {
  if(matrix_CSC){
    for (uint32_t i = 0; i < matrix_CSC->ncols; i++) {
      if (matrix_CSC->cols[i]){
        free_CS_matrix_idx_v_arr(matrix_CSC->cols[i]);
      }
    }
    free(matrix_CSC);
  }
}

void
free_matrix_CSC_fr_CSR(struct matrix_CSC *matrix_CSC) {
  if(matrix_CSC){
    for (uint32_t i = 0; i < matrix_CSC->ncols; i++) {
      if (matrix_CSC->cols[i]){
        for (int j = 0; j < matrix_CSC->cols[i]->nidx_vs; j++)
          free(matrix_CSC->cols[i]->idx_vs[j]);
        free(matrix_CSC->cols[i]);
      }
    }
    free(matrix_CSC);
  }
}



struct mf_uint16_t * //使用这个必须最后free掉 free(*)就可以
calc_insc (const struct mf_uint16_t *a, const struct mf_uint16_t *b) {
  assert(a);
  // uint32_t noinsc_sign = 0;
  for (int i = 0; i < MF_LEN; i++){
    if ((~(a->mf_w[i] | b->mf_w[i]))&(a->mf_v[i] ^ b->mf_v[i])) {
      // noinsc_sign = 1;
      // break;
      return NULL;
    }
  }
  // if (noinsc_sign){
  //   return NULL;
  // }
  struct mf_uint16_t *mf_insc = xcalloc (1, sizeof *mf_insc);
  for (int i = 0; i < MF_LEN; i++){
    mf_insc->mf_w[i] = a->mf_w[i] & b->mf_w[i];
    mf_insc->mf_v[i] = ((~(b->mf_w[i]))&b->mf_v[i]) + ((~(a->mf_w[i]))&b->mf_w[i]&a->mf_v[i]);
  }
  return mf_insc;
}

struct mf_uint16_t_array * //使用这个必须最后free掉 free(*)就可以
calc_minus_insc (struct mf_uint16_t *a, struct mf_uint16_t *insc) {
  uint32_t count = 0;
  struct mf_uint16_t *arr[16*MF_LEN+5];
  struct mf_uint16_t *pre = xcalloc (1, sizeof *pre);
  for (int i = 0; i < MF_LEN; i++) {
    pre->mf_w[i] = a->mf_w[i];
    pre->mf_v[i] = a->mf_v[i];
  }
  for (int i = 0; i < MF_LEN; i++) {
    uint16_t diff_field = a->mf_w[i] ^ insc->mf_w[i];
    if(diff_field) {
      uint16_t sign = 0x8000;
      for (int j = 0; j < 16; j++) {
        uint16_t diff_sign = diff_field & sign;
        if (diff_sign) {
          arr[count] = (struct mf_uint16_t *)xcalloc (1, sizeof *pre);
          for (int i_copy = 0; i_copy < MF_LEN; i_copy++) {
            if (i_copy == i) {
              arr[count]->mf_w[i_copy] = pre->mf_w[i_copy] - diff_sign;
              pre->mf_w[i_copy] = arr[count]->mf_w[i_copy];
              if (diff_sign & insc->mf_v[i]){
                arr[count]->mf_v[i_copy] = pre->mf_v[i_copy];
                pre->mf_v[i_copy] = pre->mf_v[i_copy] + diff_sign;
              }
              else{
                arr[count]->mf_v[i_copy] = pre->mf_v[i_copy] + diff_sign;
              }
            }
            else {
              arr[count]->mf_w[i_copy] = pre->mf_w[i_copy];
              arr[count]->mf_v[i_copy] = pre->mf_v[i_copy];
            }
          }
          count++;
        }
        sign >>= 1;
      }
    }
  }
  if(!count) 
    return NULL;
  struct mf_uint16_t_array *buf = xmalloc(sizeof(uint32_t)+count*sizeof(pre));
  buf->n_mfs = count;
  memcpy (buf->mfs, arr, count*sizeof(pre));
  // for (uint32_t i = 0; i < count; i++) {
  //   buf->mfs[i] = (struct mf_uint16_t *)xcalloc (1, sizeof *pre);
  //   // struct mf_uint16_t *mf =  xcalloc (1, sizeof *mf);
  //   memcpy (buf->mfs[i], &arr[i], sizeof *pre);
  //   // buf->mfs[i] = mf;
  // }
  free(pre);
  return buf;
}

static int //本头文件限定，bsearch函数要求的比较函数
link_idx_cmp (const void *a, const void *b) 
{ return *(uint32_t *)a - *(uint32_t *)b;}

struct mf_uint16_t *
get_r_out_mf (const struct of_rule *rule) {
  int add_len = sizeof(uint16_t);
  struct mf_uint16_t *tmp = xcalloc(1, sizeof *tmp);
  for (int i = 0; i < MF_LEN; i++) {
    tmp->mf_w[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->match.w);
    tmp->mf_v[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->match.v);
  }
  return tmp;
}

struct mf_uint16_t * //将r 得到mf_uint16_t *，转换为mf_uint16_t_array *
get_r_in_mf (const struct of_rule *rule) {
  int add_len = sizeof(uint16_t);
  struct mf_uint16_t *tmp = xcalloc(1, sizeof *tmp);
  for (uint32_t i = 0; i < MF_LEN; i++) {
    tmp->mf_w[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->match.w);
    tmp->mf_v[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->match.v);
  }

  if (rule->mask) {
    uint16_t mask[MF_LEN];
    uint16_t rewrite[MF_LEN];
    for (uint32_t i = 0; i < MF_LEN; i++) {
      mask[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->mask);
      rewrite[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->rewrite);
    }

    for (uint32_t i = 0; i < MF_LEN; i++) {
      tmp->mf_w[i] &= mask[i];
      tmp->mf_v[i] = (tmp->mf_v[i] & mask[i]) + ((~mask[i]) & rewrite[i]);
    }
  }
  return tmp;
}

struct mf_uint16_t_array * //将r 得到mf_uint16_t *，转换为mf_uint16_t_array *
get_r_in_mf_to_array (const struct of_rule *rule) {
  int add_len = sizeof(uint16_t);
  struct mf_uint16_t_array *mf_arr = xmalloc(sizeof *mf_arr);// 8
  // printf("%d\n", sizeof *mf_arr);
  mf_arr->n_mfs = 1;
  mf_arr->mfs[0] = xcalloc(1, sizeof *(mf_arr->mfs[0]));
  for (uint32_t i = 0; i < MF_LEN; i++) {
    mf_arr->mfs[0]->mf_w[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->match.w);
    mf_arr->mfs[0]->mf_v[i] = *(uint16_t *)((uint8_t *)data_arrs+i*add_len+rule->match.v);
  }
  if (rule->mask) {
    uint16_t mask[MF_LEN];
    uint16_t rewrite[MF_LEN];
    for (uint32_t i = 0; i < MF_LEN; i++) {
      mask[i] = *(uint16_t *)((uint8_t *)data_arrs+i+rule->mask);
      rewrite[i] = *(uint16_t *)((uint8_t *)data_arrs+i+rule->rewrite);
    }

    for (uint32_t i = 0; i < MF_LEN; i++) {
      mf_arr->mfs[0]->mf_w[i] &= mask[i];
      mf_arr->mfs[0]->mf_v[i] = (mf_arr->mfs[0]->mf_v[i] & mask[i]) + ((~mask[i]) & rewrite[i]);
    }
  }
  return mf_arr;
}

struct links_of_rule *
copy_links_of_rule(struct links_of_rule *lks) {
  if(!lks)
    return NULL;
  struct links_of_rule *tmp = xmalloc(sizeof(uint32_t) + (lks->n)*sizeof (lks->links_wc[0]));
  // uint32_t size = sizeof *tmp;
  // printf("copy_links_of_rule:%d-%d-%d; ", lks->n, sizeof *tmp, sizeof(uint32_t) + (lks->n)*sizeof (lks->links_wc[0]));
  tmp->n = lks->n;
  for (uint32_t i = 0; i < lks->n; i++) {
    tmp->links_wc[i].w = lks->links_wc[i].w;
    tmp->links_wc[i].v = lks->links_wc[i].v;
  }
  return tmp;
}

struct mf_uint16_t *
copy_mf_uint16_t(struct mf_uint16_t *mf) {
  struct mf_uint16_t *tmp = xcalloc(1, sizeof *tmp);
  for (uint32_t i = 0; i < MF_LEN; i++) {
    tmp->mf_w[i] = mf->mf_w[i];
    tmp->mf_v[i] = mf->mf_v[i];
  }
  return tmp;
}

struct mask_uint16_t *
copy_mask_uint16_t(struct mask_uint16_t *mk) {
  struct mask_uint16_t *tmp = xcalloc(1, sizeof *tmp);
  for (uint32_t i = 0; i < MF_LEN; i++) 
    tmp->v[i] = mk->v[i];
  return tmp;
}

uint32_t // 1:yes, 0:no
is_insc_wc_uint16_t(struct wc_uint16_t *a, struct wc_uint16_t *b) {
  if ((~(a->w | b->w))&(a->v ^ b->v))
    return 0;
  else
    return 1;
}

uint32_t // 1:yes, 0:no
is_insc_links(struct links_of_rule *a, struct links_of_rule *b) {
  if((!a)||(!b))
    return 0;
  for (uint32_t i = 0; i < a->n; i++) {
    for (uint32_t j = 0; j < b->n; j++) {
      if (!((~(a->links_wc[i].w | b->links_wc[i].w))&(a->links_wc[i].v ^ b->links_wc[i].v)))
        return 1;
    }
  }
  return 0;
}

/*BDD相关各项处理*/
/*========================================================================*/
BDD 
mf2bdd_init(struct mf_uint16_t *mf) {

  BDD root, tmp;
  root = 1;
  tmp = 1;  
  print_mf_uint16_t(mf);
  for (int i = 0; i < MF_LEN; i++){
    int reverse_i = MF_LEN - i - 1;
    uint16_t sign = 0x0001;
    for (int j = 0; j < 16; j++){
      if (!(sign & mf->mf_w[reverse_i])){
        root = bdd_ithvar(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
        // printf("%d\n", root);
        // printf("bdd_var %d\n", bdd_var(root));
        // root = bdd_ithvar(16*MF_LEN - i*j - 1);
        if (sign & mf->mf_v[reverse_i]){
          // root = bdd_ithvar(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
          LOW(root) = 0;
          HIGH(root) = tmp;
        }
        else{
          // root = bdd_nithvar(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
          LOW(root) = tmp;
          HIGH(root) = 0;
        }
        tmp = root;
      }
      sign <<= 1;
    }
  }
  // if (root == 1){
  //   root = bdd_ithvar(0);
  //   LOW(root) = 1;
  //   HIGH(root) = 1;
  // }
  return root;
}

BDD 
mf2bdd(struct mf_uint16_t *mf) {
  BDD root, tmp;
  root = 1;
  tmp = 1;  
  // print_mf_uint16_t(mf);
  for (int i = 0; i < MF_LEN; i++){
    int reverse_i = MF_LEN - i - 1;
    uint16_t sign = 0x0001;
    for (int j = 0; j < 16; j++){
      if (!(sign & mf->mf_w[reverse_i])){
        int level = bdd_var2level(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
        if (sign & mf->mf_v[reverse_i]){
          root = bdd_makenode(level, 0, tmp);
        }
        else{
          root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点
        }
        tmp = root;
      }
      sign <<= 1;
    }
  }
  return root;
}

int
add_node_2_mf(BDD node, BDD prenode,struct mf_uint16_t *mf, uint16_t v) {
  int var_pre, var_prediv16;
  int var = bdd_var(node);
  int vardiv16 = FRA2INT(var/16);
  if (prenode == -1) {
    var_pre = -1;
    var_prediv16 = 0;
  }
  else{
    var_pre = bdd_var(prenode);
    var_prediv16 = FRA2INT(var_pre/16);
  }

  for (int i = var_prediv16; i < vardiv16+1; i++){
    uint16_t w_sign = 0x0000;
    if (i == var_prediv16){
      for (int j = (var_pre%16)+1; j < 16; j++){
        w_sign += var2sign[j];
      }
      if (i == vardiv16){
        uint16_t sign = 0;
        for (int j = 0; j < (var%16); j++){
          sign += var2sign[j];
        }
        w_sign = w_sign & sign; 
      }
      mf->mf_w[i] = w_sign | mf->mf_w[i];
      mf->mf_v[i] = (~w_sign) & mf->mf_v[i];
    }
    else if (i == vardiv16){
      for (int j = 0; j < (var%16); j++){
        w_sign += var2sign[j];
      }
      mf->mf_w[i] = w_sign | mf->mf_w[i];
      mf->mf_v[i] = (~w_sign) & mf->mf_v[i];
    }
    else {
      mf->mf_w[i] = 0xffff;
      mf->mf_v[i] = 0x0000;
    }
  }

  uint16_t sign = VAR2SIGN(var);
  mf->mf_w[vardiv16] = mf->mf_w[vardiv16] & (~sign);

  if (v == 0)
    mf->mf_v[vardiv16] = mf->mf_v[vardiv16] & (~sign);
  else 
    mf->mf_v[vardiv16] = mf->mf_v[vardiv16] | sign;
  return 0;
}

struct mf_uint16_t *
genmf_fr_path(BDD node, struct mf_uint16_t *mf) {
  int var = bdd_var(node);
  // printf("var: %d\n", var);
  int vardiv16 = FRA2INT(var/16);
  struct mf_uint16_t *tmp = copy_mf_uint16_t(mf);
  for (int i = vardiv16; i < MF_LEN; i++){
    if (i == vardiv16){
      uint16_t w_sign = 0x0000;
      for (int j = (var%16)+1; j < 16; j++){
        w_sign += var2sign[j];
      }
      tmp->mf_w[i] = tmp->mf_w[i] | w_sign;
      tmp->mf_v[i] = (~w_sign) & tmp->mf_v[i];
    }
    else {
      tmp->mf_w[i] = 0xffff;
      tmp->mf_v[i] = 0x0000;
    }
  } 
  return tmp;
}

int
back_node_2_mf(BDD node, struct mf_uint16_t *mf) {
  if(node == -1){
    init_mf(mf);
    return 0;
  }
  int var = bdd_var(node);
  int vardiv16 = FRA2INT(var/16);
  // printf("var: %d\n", var);
  for (int i = vardiv16; i < MF_LEN; i++){
    if (i == vardiv16){
      uint16_t w_sign = 0x0000;
      for (int j = (var%16)+1; j < 16; j++){
        w_sign += var2sign[j];
      }
      w_sign = ~w_sign;
      mf->mf_w[i] = mf->mf_w[i] & w_sign;
      mf->mf_v[i] = w_sign & mf->mf_v[i];
    }
    else {
      mf->mf_w[i] = 0x0000;
      mf->mf_v[i] = 0x0000;
    }
  }
  return 0;
}

struct mf_uint16_t_array *
bdd2mf(BDD root, int varnum) {
  // CHECK(root);
  if(root == 1){
    struct mf_uint16_t_array *mf_arr_tmp = xmalloc(sizeof *mf_arr_tmp);
    mf_arr_tmp->n_mfs = 1;
    mf_arr_tmp->mfs[0] = xcalloc (1,sizeof(struct mf_uint16_t));
    for (int i = 0; i < MF_LEN; i++){
      mf_arr_tmp->mfs[0]->mf_w[i] = 0xffff;
      mf_arr_tmp->mfs[0]->mf_v[i] = 0x0000;
    }
    return mf_arr_tmp;
  }
  if(root == 0)
    return NULL;

  BDD record[varnum + 2];
  for (int i = 0; i < varnum; i++)
    record[i] = -1;
  struct mf_uint16_t *arr_tmp[20*varnum];
  int arr_count = 0;
  struct mf_uint16_t *mf_tmp = xcalloc (1, sizeof *mf_tmp);
  init_mf(mf_tmp);
  BDD node_tmp;
  record[0] = -1;
  record[1] = root;
  int curr_pos = 1;

  while(curr_pos){
    node_tmp = record[curr_pos];
    if (HIGH(node_tmp) == record[curr_pos+1]){
      if (LOW(node_tmp) == 1){      
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 0);
        arr_tmp[arr_count] = genmf_fr_path(node_tmp, mf_tmp);
        arr_count++;

        record[curr_pos + 1] = 1;
        back_node_2_mf(record[curr_pos-1], mf_tmp);
      }
      else if ((LOW(node_tmp) == 0)){
        record[curr_pos + 1] = 0;
      }
      else {
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 0);
        curr_pos++;
        record[curr_pos] = LOW(node_tmp);
      }
    }
    else if (LOW(node_tmp) == record[curr_pos+1]){
      record[curr_pos + 1] = -1;
      curr_pos--;
      if(curr_pos)
        back_node_2_mf(record[curr_pos-1], mf_tmp);
    }
    else{
      if (HIGH(node_tmp) == 1){
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 1);
        arr_tmp[arr_count] = genmf_fr_path(node_tmp, mf_tmp);
        arr_count++;

        record[curr_pos + 1] = 1;
        back_node_2_mf(record[curr_pos-1], mf_tmp);
      }
      else if ((HIGH(node_tmp) == 0)){
        record[curr_pos + 1] = 0;
      }
      else {
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 1);
        curr_pos++;
        record[curr_pos] = HIGH(node_tmp);
      }
    }
  }

  struct mf_uint16_t_array *tmp = (struct mf_uint16_t_array *)xmalloc(sizeof(uint32_t)+arr_count*sizeof(mf_tmp)); 
  tmp->n_mfs = arr_count;
  for (int i = 0; i < arr_count; i++){
    tmp->mfs[i] = arr_tmp[i];
  }
  return tmp;
}

BDD 
bdd_ref(BDD root) {
   CHECK(root);
   if (root < 2)
      return bdd_error(BDD_ILLBDD);
   return (REF(root));
}

void
print_node(BDD r) {
  if (r < 2)
    printf("base node: %d\n", r);
  else{
    printf("num: %d; ", r);
    printf("low: %d; ", bdd_low(r));
    printf("high: %d; ", bdd_high(r));
    printf("var: %d; ", bdd_var(r));
    printf("refcou: %d\n", bdd_ref(r));
  }
}

int
cmp_bdd_by_var(const void *a, const void *b)
{ return bdd_var(*(int*)b) - bdd_var(*(int*)a);}

void
bdd_save2stru(BDD root, BDD *r, int *count) {
  BddNode *node;

  if (root < 2)
    return;

  node = &bddnodes[root];
  if ((node)->level & MARKON  ||  LOWp(node) == -1)
    return;

  (node)->level |= MARKON;
  r[*count] = root;
  (*count)++;
  bdd_save2stru(LOWp(node), r, count);
  bdd_save2stru(HIGHp(node), r, count); 
}

struct bdd_saved_arr *
bdd_save_arr(BDD root){
  if (root < 2){
    if (root == 1){
      struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+sizeof(struct BddNode_saved));
      tmp->arr_num = 1;
      tmp->bdd_s[0].var = 1;
      tmp->bdd_s[0].low = -1;
      tmp->bdd_s[0].high = -1;
      return tmp;
    }
    if (root == 0){
      struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+sizeof(struct BddNode_saved));
      tmp->arr_num = 1;
      tmp->bdd_s[0].var = 0;
      tmp->bdd_s[0].low = -1;
      tmp->bdd_s[0].high = -1;
      return tmp;
    }
    else
      return NULL;
  }
  int count = 0;
  // BDD arr_tmp[16*MF_LEN*10];
  BDD arr_tmp[5000];
  bdd_save2stru(root, arr_tmp, &count);

  bdd_unmark(root);
  
  qsort(arr_tmp, count,sizeof (BDD), cmp_bdd_by_var);
  int var_arr[16*MF_LEN];
  int var_tmp = 16*MF_LEN - 1;
  var_arr[var_tmp] = 0;
  for (int i = 0; i < count; i++){
    int var = bdd_var(arr_tmp[i]);
    if (var != var_tmp){
      for (int j = var; j < var_tmp; j++)
        var_arr[j] = i;
      var_tmp = var;
    }
  }
  if (var_tmp != 0){
    for (int i = 0; i < var_tmp; i++)
      var_arr[i] = count;
  }

  struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+(count+2)*sizeof(struct BddNode_saved));
  tmp->arr_num = count+2;
  tmp->bdd_s[0].var = -1;
  tmp->bdd_s[0].low = -1;
  tmp->bdd_s[0].high = -1;
  tmp->bdd_s[1].var = -1;
  tmp->bdd_s[1].low = -1;
  tmp->bdd_s[1].high = -1;
  for (int i = 0; i < count; i++){
    int var = bdd_var(arr_tmp[i]);
    int low = LOW(arr_tmp[i]);
    int high = HIGH(arr_tmp[i]);
    tmp->bdd_s[i+2].var = var;
    if (low < 2)
      tmp->bdd_s[i+2].low = low;
    else {
      int low_var = bdd_var(low);
      if (low_var){
        for (int j = var_arr[low_var]; j < var_arr[low_var - 1]; j++){
          if (low == arr_tmp[j]){
            tmp->bdd_s[i+2].low = j+2;
            break;
          }
        }
      }
      else {
        for (int j = var_arr[low_var]; j < count; j++){
          if (low == arr_tmp[j]){
            tmp->bdd_s[i+2].low = j+2;
            break;
          }
        }
      }
    }
    if (high < 2)
      tmp->bdd_s[i+2].high = high;
    else {
      int high_var = bdd_var(high);
      if (high_var != 0){
        for (int j = var_arr[high_var]; j < var_arr[high_var - 1]; j++){
          if (high == arr_tmp[j]){
            tmp->bdd_s[i+2].high = j+2;
            break;
          }
        }
      }
      else {
        for (int j = var_arr[high_var]; j < count; j++){
          if (high == arr_tmp[j]){
            tmp->bdd_s[i+2].high = j+2;
            break;
          }
        }
      }
    }
  }
  return tmp;
}

BDD
load_saved_bddarr(struct bdd_saved_arr *bdd_arr) {
  if (bdd_arr->arr_num == 1)
    return bdd_arr->bdd_s[0].var;

  BDD arr_tmp[bdd_arr->arr_num];
  arr_tmp[0] = 0;
  arr_tmp[1] = 1;
  for (int i = 2; i < bdd_arr->arr_num; i++){

    int level = bdd_var2level(bdd_arr->bdd_s[i].var);
    arr_tmp[i] = bdd_makenode(level, arr_tmp[bdd_arr->bdd_s[i].low], arr_tmp[bdd_arr->bdd_s[i].high]);
  }
  return arr_tmp[bdd_arr->arr_num-1];
}

/*稀疏矩阵处理with BDD*/
/*========================================================================*/
static int
uint32_t_cmp (const void *a, const void *b)
{ return *(uint32_t *)a - *(uint32_t *)b; }

struct wc_uint16_t *
wc_uint16_t_insc(struct wc_uint16_t *a, struct wc_uint16_t *b){
  if ((~(a->w | b->w))&(a->v ^ b->v)) 
    return NULL;
  struct wc_uint16_t *insc = xcalloc (1, sizeof *insc);
  insc->w = a->w & b->w;
  insc->v = ((~(b->w))&b->v) + ((~(a->w))&(b->w)&(a->v));
  return insc;
}

struct links_of_rule *
links_insc(struct links_of_rule *a, struct links_of_rule *b) {
  if((!a )|| (!b))
    return NULL;
  struct wc_uint16_t arr[MAX_ARR_SIZE];
  uint32_t count = 0;
  for (uint32_t i = 0; i < a->n; i++) {
    for (uint32_t j = 0; j < b->n; j++) {
      struct wc_uint16_t *insc =  wc_uint16_t_insc(&(a->links_wc[i]), &(b->links_wc[i]));
      if (insc){
        arr[count].w = insc->w;
        arr[count].v = insc->v;
        count++;
        free(insc); 
      }
    } 
  }
  if(!count)
    return NULL;
  struct links_of_rule *lks = xmalloc(sizeof (uint32_t) + count*sizeof(struct wc_uint16_t));
  lks->n = count;
  memcpy (lks->links_wc, arr, count*sizeof(struct wc_uint16_t)); 
  return lks;
}

void
print_Tri_express(struct matrix_Tri_express *Tri) {
  printf("Tri row-col:%d - %d\n", Tri->row_idx, Tri->col_idx);
  print_matrix_element(Tri->elem);
}

BDD
bdd_mask2x(struct bdd_saved_arr *bdd_arr, struct mask_uint16_t *mask) {
  BDD arr_tmp[bdd_arr->arr_num];
  arr_tmp[0] = 0;
  arr_tmp[1] = 1;

  for (int i = 2; i < bdd_arr->arr_num; i++){
    int var = bdd_arr->bdd_s[i].var;
    int level = bdd_var2level(var);
    BDD low = arr_tmp[bdd_arr->bdd_s[i].low];
    BDD high = arr_tmp[bdd_arr->bdd_s[i].high];

    if (VAR2SIGN(var)&(mask->v[FRA2INT(var)])){
      if (low == high)
        arr_tmp[i] = low;
      else
        arr_tmp[i] = bdd_makenode(level, low, high);
    }
    else 
      arr_tmp[i] = bdd_apply(low, high, bddop_or);
  }

  return arr_tmp[bdd_arr->arr_num-1];
}


BDD
bdd_v2x_bymask_old(BDD root, struct mask_uint16_t *mask) {
  
  if (root < 2)
    return root;
  int var = bdd_var(root);
  int lvl = (int)(var/(16));
  if ((uint16_power_sign[15 - var%16]) & (uint16_t)(~(mask->v[lvl]))){
  
    return bdd_apply(bdd_v2x_bymask_old(LOW(root), mask), bdd_v2x_bymask_old(HIGH(root), mask), bddop_or);
  }
  else{
    int level = bdd_var2level(var);
    BDD a = bdd_v2x_bymask_old(LOW(root), mask);
    BDD b = bdd_v2x_bymask_old(HIGH(root), mask);
    if (a == b)
      return a;
    if (a == LOW(root)){
      if (HIGH(root) == b)
        return root;
    }
      return bdd_makenode(level, a, b);
  }
}


BDD
rw2bdd(struct mask_uint16_t *mask, struct mask_uint16_t *rw){
  BDD root, tmp;
  root = 1;
  tmp = 1;  
  // print_mf_uint16_t(mf);
  for (int i = 0; i < MF_LEN; i++){
    int reverse_i = MF_LEN - i - 1;
    uint16_t sign = 0x0001;
    for (int j = 0; j < 16; j++){
      if (!(sign & mask->v[reverse_i])){
        int level = bdd_var2level(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
        if (sign & rw->v[reverse_i]){
          root = bdd_makenode(level, 0, tmp);
        }
        else{
          root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点
        }
        tmp = root;
      }
      sign <<= 1;
    }
  }
  return root;
}

BDD
mask2bdd(struct mask_uint16_t *mask){
  BDD root, tmp;
  root = 1;
  tmp = 1;  
  // print_mf_uint16_t(mf);
  for (int i = 0; i < MF_LEN; i++){
    int reverse_i = MF_LEN - i - 1;
    uint16_t sign = 0x0001;
    for (int j = 0; j < 16; j++){
      if (!(sign & mask->v[reverse_i])){
        int level = bdd_var2level(16*MF_LEN - 16*i - j - 1);//
        root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点,为0
        tmp = root;
      }
      sign <<= 1;
    }
  }
  return root;
}

BDD
bdd_v2x_bymask(BDD root, struct mask_uint16_t *mask) {
  BDD mask_bdd = mask2bdd(mask); 
  applyop = 2;
  return bdd_v2x_rec(root, mask_bdd);
} 


struct bdd_saved_arr *
bdd_rw(struct bdd_saved_arr *bdd_arr, struct mask_uint16_t *mask, struct mask_uint16_t *rw) {
  BDD root_maskx = bdd_mask2x(bdd_arr, mask);
  BDD root_rw = rw2bdd(mask, rw);
  root_rw = bdd_apply(root_maskx, root_rw, bddop_and);
  return bdd_save_arr(root_rw);
}


struct bdd_saved_arr *
bdd_rw_back(struct bdd_saved_arr *bdd_arr, struct bdd_saved_arr *bdd_arr_IN, struct mask_uint16_t *mask) {
  BDD root_maskx = bdd_mask2x(bdd_arr, mask);
  BDD root_IN = load_saved_bddarr(bdd_arr_IN);
  root_IN = bdd_apply(root_maskx, root_IN, bddop_and);
  return bdd_save_arr(root_IN);
}


BDD
bdd_rw_BDD(BDD a, struct mask_uint16_t *mask, struct mask_uint16_t *rw) {
  // struct timeval start,stop; 
  // gettimeofday(&start,NULL);
  BDD root_maskx = bdd_v2x_bymask(a, mask);
  // gettimeofday(&stop,NULL);
  // time_counter4 += diff(&stop, &start);
  // gettimeofday(&start,NULL);
  BDD root_rw = rw2bdd(mask, rw);
  root_rw = bdd_apply(root_maskx, root_rw, bddop_and);
  // gettimeofday(&stop,NULL);
  // time_counter5 += diff(&stop, &start);
  return root_rw;
}

BDD
bdd_rw_back_BDD(BDD a, BDD a_IN, struct mask_uint16_t *mask) {
  // struct timeval start,stop; 
  // gettimeofday(&start,NULL);
  BDD root_maskx = bdd_v2x_bymask(a, mask);
  // gettimeofday(&stop,NULL);
  // time_counter4 += diff(&stop, &start);
  // gettimeofday(&start,NULL);
  BDD root_IN = bdd_apply(root_maskx, a_IN, bddop_and);
  // gettimeofday(&stop,NULL);
  // time_counter5 += diff(&stop, &start);
  return root_IN;
}

struct bdd_saved_arr *
copy_bdd_saved_arr(struct bdd_saved_arr *bdd_arr) {
  struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+(bdd_arr->arr_num)*sizeof(struct BddNode_saved));
  tmp->arr_num = bdd_arr->arr_num;
  memcpy (tmp->bdd_s, bdd_arr->bdd_s, (bdd_arr->arr_num)*sizeof(struct BddNode_saved)); 
  return tmp;
}

void
print_bdd_saved_arr(struct bdd_saved_arr *bdd_arr){
  printf("There are %d nodes\n", bdd_arr->arr_num);
  for (int i = 0; i < bdd_arr->arr_num; i++){
    printf("node:%d, var:%d, low:%d, high:%d\n", i, bdd_arr->bdd_s[i].var, bdd_arr->bdd_s[i].low, bdd_arr->bdd_s[i].high);
  }
}

struct matrix_Tri_express * 
insc_to_Tri_express_rlimit(struct of_rule *r_in, struct of_rule *r_out, BDD v_and) {

  struct nf_space_pair *pair = xcalloc(1, sizeof *pair);
  struct links_of_rule *lks = rule_links_get(r_in, IN_LINK);
  struct links_of_rule *lks_out = rule_links_get(r_in, OUT_LINK);
  struct links_of_rule *lks_in = rule_links_get(r_out, IN_LINK);
  lks_out = links_insc(lks_in, lks_out);
  int add_len = sizeof(uint16_t);
  struct matrix_Tri_express *tmp = xcalloc(1, sizeof *tmp);
  tmp->elem = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+sizeof(struct nf_space_pair *));
  tmp->elem->npairs = 1;
  tmp->row_idx = matrix_idx_get_r(r_in);
  tmp->col_idx = matrix_idx_get_r(r_out);

  // struct bdd_saved_arr *bdd_arr_out = bdd_save_arr(v_and);

  pair->in = xcalloc(1, sizeof *(pair->in));// 1,16(两个指针为16)
  pair->in->lks = copy_links_of_rule(lks);
  pair->out = xcalloc(1, sizeof *(pair->out));
  pair->out->lks = copy_links_of_rule(lks_out);

  pair->r_arr = xmalloc(sizeof (uint32_t)+2*sizeof (struct r_idx));

  pair->r_arr->nrs = 2;
  pair->r_arr->ridx[0].sw_idx = r_in->sw_idx;
  pair->r_arr->ridx[0].r_idx = r_in->idx;
  pair->r_arr->ridx[1].sw_idx = r_out->sw_idx;
  pair->r_arr->ridx[1].r_idx = r_out->idx;
  pair->out->mf = v_and;
  bdd_addref(pair->out->mf);
  tmp->elem->bdd_out = pair->out->mf;
  bdd_addref(pair->out->mf);

  if (r_in->mask) {
    struct mask_uint16_t *mask = xcalloc(1, sizeof *mask);
    struct mask_uint16_t *rewrite = xcalloc(1, sizeof *rewrite);
    for (uint32_t j = 0; j < MF_LEN; j++) {
      mask->v[j] = *(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->mask);
      rewrite->v[j] = *(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->rewrite);
    }
    struct mf_uint16_t *r_in_mf = get_r_out_mf(r_in);
    BDD bdd_in = mf2bdd(r_in_mf);
    BDD bdd_arr_tmp = bdd_rw_back_BDD(v_and, bdd_in, mask);
    // BDD bdd_arr_tmp = bdd_rw_back(v_and, bdd_in, mask);

    pair->in->mf = bdd_arr_tmp;
    bdd_addref(pair->in->mf);
    tmp->elem->bdd_in = pair->in->mf;
    bdd_addref(pair->in->mf);
    pair->mask = mask;
    pair->rewrite = rewrite;
  }
  else {
    pair->in->mf = v_and;
    bdd_addref(pair->in->mf);
    tmp->elem->bdd_in = pair->in->mf;
    bdd_addref(pair->in->mf);
    pair->mask = NULL;
    pair->rewrite = NULL;
  }
  tmp->elem->nf_pairs[0] = pair;
  return tmp;
}

int
cmp_matrix_Tri_express(const void *a, const void *b){
  struct matrix_Tri_express *a_tmp = *((struct matrix_Tri_express **)a);
  struct matrix_Tri_express *b_tmp = *((struct matrix_Tri_express **)b);
  uint32_t cmp = a_tmp->row_idx - b_tmp->row_idx;
  if (cmp)
    return cmp;
  return a_tmp->col_idx - b_tmp->col_idx;
}

int
cmp_matrix_Tri_express_CSC(const void *a, const void *b){
  struct matrix_Tri_express *a_tmp = *((struct matrix_Tri_express **)a);
  struct matrix_Tri_express *b_tmp = *((struct matrix_Tri_express **)b);
  uint32_t cmp = a_tmp->col_idx - b_tmp->col_idx;
  if (cmp)
    return cmp;
  return a_tmp->row_idx - b_tmp->row_idx;
}

bool
Tri_is_eq (struct matrix_Tri_express *latter, struct matrix_Tri_express *former){
  if ((latter->row_idx == former->row_idx)&&(latter->col_idx == former->col_idx))
    return 1;
  return 0;
}

struct matrix_element *
matrix_elem_plus(struct matrix_element *a, struct matrix_element *b) {
  if (!b)
    return a;
  if (!a)
    return b;
  struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+((a->npairs)+(b->npairs))*sizeof(struct nf_space_pair *));
  tmp->npairs = a->npairs + b->npairs;
  if (a->bdd_in == b->bdd_in) {
    tmp->bdd_in = a->bdd_in;
    bdd_delref(a->bdd_in);
  }
  else {
    tmp->bdd_in = bdd_apply(a->bdd_in, b->bdd_in, bddop_or);
    bdd_delref(a->bdd_in);
    bdd_delref(b->bdd_in);
    bdd_addref(tmp->bdd_in);
  }
  if (a->bdd_out == b->bdd_out) {
    tmp->bdd_out = a->bdd_out;
    bdd_delref(a->bdd_out);
  }
  else {
    tmp->bdd_out = bdd_apply(a->bdd_out, b->bdd_out, bddop_or);
    bdd_delref(a->bdd_out);
    bdd_delref(b->bdd_out);
    bdd_addref(tmp->bdd_out);
  }
  for (int i = 0; i < a->npairs; i++) {
    tmp->nf_pairs[i] = a->nf_pairs[i]; 
  }
  for (int i = 0; i < b->npairs; i++) {
    tmp->nf_pairs[i+a->npairs] = b->nf_pairs[i];
  }
  free(a);
  a = NULL;
  free(b);
  b = NULL;
  return tmp; 
}

void
matrix_elem_plus_void(struct matrix_element *a, struct matrix_element *b) {
  if (a&&b){
    printf("there is begin\n");
    printf("%d\n", (a->npairs)+(b->npairs));
    struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+((a->npairs)+(b->npairs))*sizeof(struct nf_space_pair *));
    tmp->npairs = a->npairs + b->npairs;
    for (int i = 0; i < a->npairs; i++) {
      tmp->nf_pairs[i] = a->nf_pairs[i]; 
    }
    for (int i = 0; i < b->npairs; i++) {
      tmp->nf_pairs[i+a->npairs] = b->nf_pairs[i];
    }
    free(a);
    a = NULL;
    free(b);
    b = NULL;
    a = tmp;
    free(tmp);
  } 
  else if ((!a)&&(b)){
    struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+(b->npairs)*sizeof(struct nf_space_pair *));
    tmp->npairs = b->npairs;
    for (int i = 0; i < b->npairs; i++) {
      tmp->nf_pairs[i] = b->nf_pairs[i]; 
    }
    free(b);
    b = NULL;
    a = tmp;
    
  }
  free(a);
  a = NULL;
  free(b);
  b = NULL;
}

void
matrix_elem_plus_test(struct matrix_element *a){
  if(a){
    struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+(a->npairs)*sizeof(struct nf_space_pair *));
    tmp->npairs = a->npairs;
    for (int i = 0; i < a->npairs; i++) {
      tmp->nf_pairs[i] = a->nf_pairs[i]; 
    }
    // free(tmp);
    free(a);
    a = NULL;
  }
}

struct matrix_CSR *
gen_matrix_CSR_from_Tris(struct Tri_arr *Tri_arr) {

  if (Tri_arr->nTris==0){
    return NULL;
  }
  printf("matrix_CSR\n");
  // printf("there is wrong: %d - %d\n", data_allr_nums, Tri_arr->nTris);
  qsort(Tri_arr->arr, Tri_arr->nTris,sizeof (struct matrix_Tri_express *), cmp_matrix_Tri_express);
  
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+data_allr_nums*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = data_allr_nums;
  for (int i = 0; i < data_allr_nums; i++){
    tmp->rows[i] = NULL;
  }
  printf("matrix_CSR\n");
  // printf("matrix_CSR\n");
  // struct matrix_Tri_express *Tri_arr_tmp[Tri_arr->nTris];
  struct matrix_Tri_express **Tri_arr_tmp = xcalloc(Tri_arr->nTris,sizeof(struct matrix_Tri_express *));
  Tri_arr_tmp[0] = Tri_arr->arr[0];
  if (Tri_arr->nTris==1){   
    uint32_t row_idx = Tri_arr_tmp[0]->row_idx;
    tmp->rows[row_idx] = xmalloc(sizeof(uint32_t)+sizeof(struct CS_matrix_idx_v *));
    tmp->rows[row_idx]->nidx_vs = 1;
    tmp->rows[row_idx]->idx_vs[0] = xmalloc(sizeof(struct CS_matrix_idx_v));
    tmp->rows[row_idx]->idx_vs[0]->idx = Tri_arr_tmp[0]->col_idx;
    tmp->rows[row_idx]->idx_vs[0]->elem = Tri_arr_tmp[0]->elem;
    return tmp;
  }
  uint32_t count = 1, last = 0;
  uint32_t begin = 0, count_row = 1;
  // uint32_t check = 0;
  // printf("matrix_CSR %d - %d\n", Tri_arr->arr[21280]->row_idx, Tri_arr->arr[21280]->col_idx);

  for (uint32_t i = 1; i < Tri_arr->nTris; i++) {   
    // for (int i = 0; i < count * MF_LEN; i += MF_LEN) {
    //计数乘以MF_LEN，i+MF_LEN，i，按match所占uint16_t位数循环，在前面的文件中
    if (Tri_is_eq (Tri_arr->arr[i], Tri_arr->arr[last])) {


      // Tri_arr_tmp[count-1]->elem = matrix_elem_plus(Tri_arr_tmp[count-1]->elem, Tri_arr->arr[i]->elem);//保留相同部分，r-r通过两条链路
      // free(Tri_arr->arr[i]);
      free_matrix_element(Tri_arr->arr[i]->elem);
      free(Tri_arr->arr[i]);
      continue;
    }
    Tri_arr_tmp[count] = Tri_arr->arr[i];
    if (Tri_arr_tmp[count]->row_idx != Tri_arr_tmp[count-1]->row_idx) {
      uint32_t row_idx = Tri_arr_tmp[begin]->row_idx;     
      tmp->rows[row_idx] = xmalloc(sizeof(uint32_t)+count_row*sizeof(struct CS_matrix_idx_v *));
      // if(!(tmp->rows[row_idx]))
      //   printf("there is wrong\n");
      tmp->rows[row_idx]->nidx_vs = count_row;
      // check+=tmp->rows[row_idx]->nidx_vs;
      for (uint32_t j = 0; j < count_row; j++){
        tmp->rows[row_idx]->idx_vs[j] = xmalloc(sizeof(struct CS_matrix_idx_v));
        tmp->rows[row_idx]->idx_vs[j]->idx = Tri_arr_tmp[begin+j]->col_idx;
        tmp->rows[row_idx]->idx_vs[j]->elem = Tri_arr_tmp[begin+j]->elem;
      }
      count_row = 0;
      begin = count;
    }
    last = i;
    count++; 
    count_row++;  
  }
  // assert(!(check-count));
  uint32_t row_idx = Tri_arr_tmp[begin]->row_idx;
  // printf("%d\n", row_idx);
  tmp->rows[row_idx] = xmalloc(sizeof(uint32_t)+count_row*sizeof(struct CS_matrix_idx_v *));
  // if(!(tmp->rows[row_idx]))
  //   printf("there is wrong\n");
  tmp->rows[row_idx]->nidx_vs = count_row;
  // check+=tmp->rows[row_idx]->nidx_vs;
  for (uint32_t j = 0; j < count_row; j++){
    tmp->rows[row_idx]->idx_vs[j] = xmalloc(sizeof(struct CS_matrix_idx_v));
    tmp->rows[row_idx]->idx_vs[j]->idx = Tri_arr_tmp[begin+j]->col_idx;
    tmp->rows[row_idx]->idx_vs[j]->elem = Tri_arr_tmp[begin+j]->elem;
  }
  for (uint32_t i = 0; i < count; i++) {
    free(Tri_arr_tmp[i]);
  }
  free(Tri_arr_tmp);
  printf("all num = %d\n", count);
  return tmp;
}

bool
is_mf_allx(struct mf_uint16_t *a) {
  bool sign = true;
  for (uint32_t i = 0; i < MF_LEN; i++) {
    if ((a->mf_w[i]) != 0xffff){
      sign = false;
      break;
    }
  }
  return sign;
}

struct Tri_arr *
gen_Tri_arr_bdd(void) {
  
  uint32_t max_CSR = MAX_VAL_RATE*data_allr_nums*data_allr_nums;
  // struct matrix_Tri_express *Tri_arr[max_CSR];
  struct matrix_Tri_express **Tri_arr = xmalloc(max_CSR*sizeof(struct matrix_Tri_express *));
  // struct matrix_Tri_express *Tri_arr[200000];
  uint32_t nTris = 0;
  uint32_t rule_nums_in_pre = 0;
  uint32_t rule_nums_out = 0;

  printf("gen_Tri_arr_bdd\n" );
  for (uint32_t i = 0; i < link_in_rule_file->swl_num; i++) {
    struct link_to_rule *lout_r = get_link_rules(link_out_rule_file, &rule_nums_out, link_in_rule_file->links[i].link_idx);
    if (lout_r) {
      struct link *lk = link_get(lout_r->link_idx);
      print_link(lk);
      uint32_t rule_nums_in  = link_in_rule_file->links[i].rule_nums - rule_nums_in_pre;
      printf("%d: %d-%d\n", lout_r->link_idx, rule_nums_in, rule_nums_out);
      uint32_t *lin_arrs = (uint32_t *)(link_in_rule_data_arrs + 2*rule_nums_in_pre);
      uint32_t rule_nums_out_pre = lout_r->rule_nums - rule_nums_out;
      for (uint32_t i_in = 0; i_in < rule_nums_in; i_in++) {  
        struct of_rule *r_in = rule_get_2idx(*(uint32_t *)lin_arrs, *(uint32_t *)(lin_arrs+1));

        BDD v_in, v_out;        
        struct mf_uint16_t *r_in_mf = get_r_in_mf(r_in);

        uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);
        struct timeval start,stop; 
        gettimeofday(&start,NULL);
        v_in = mf2bdd(r_in_mf);
        gettimeofday(&stop,NULL);
        time_counter_elemplus+=diff(&stop, &start);
        // if ((r_in->sw_idx == 0) && (r_in->idx == 1))
        // {
        //   print_rule(r_in);
        //   print_mf_uint16_t(r_in_mf);
        // }
        // if ((r_in->sw_idx == 0) && (r_in->idx == 2))
        // {
        //   print_rule(r_in);
        //   print_mf_uint16_t(r_in_mf);
        // }
        
  
        // bdd_addref(v_in);
        for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {
          struct of_rule *r_out = rule_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
          // printf("%d-%d,%d-%d;", r_in->sw_idx, r_in->idx, r_out->sw_idx, r_out->idx);
          struct mf_uint16_t *r_out_mf = get_r_out_mf(r_out); 
          // if ((r_out->sw_idx == 14) && (r_out->idx == 1885))
          // {
          //   print_rule(r_out);
          //   print_mf_uint16_t(r_out_mf);
          // }
          
          // gettimeofday(&start,NULL);
          v_out = mf2bdd(r_out_mf);
          // gettimeofday(&stop,NULL);
          // time_counter_elemplus+=diff(&stop, &start);
          BDD v_and, v_diff;
          v_and = bdd_apply(v_in, v_out, bddop_and);
                  
          if (v_and){
            Tri_arr[nTris] = insc_to_Tri_express_rlimit(r_in, r_out, v_and);
            nTris++;
            v_diff = bdd_apply(v_in, v_and, bddop_diff);
          }
          else {
            v_diff = v_in;
          } 
          v_in = v_diff;
          if (!v_in){
            free(r_out_mf);
            break;
          }

          lout_arrs += 2;
          free(r_out_mf);
          
          
        }
        lin_arrs += 2;
        free(r_in_mf);
        // bdd_delref(v_in); 
      }  
    }
    rule_nums_in_pre = link_in_rule_file->links[i].rule_nums;
  }

  struct Tri_arr *tmp = xmalloc(sizeof(uint32_t)+nTris*sizeof(struct matrix_Tri_express *));
  tmp->nTris = nTris;
  printf("nTris %d\n", nTris);
  for (uint32_t i = 0; i < nTris; i++){
    tmp->arr[i] = Tri_arr[i];
  }
  free(Tri_arr);
  return tmp;
}

struct matrix_CSR *  //通过对链路文件查找两个同链路的头尾端规则，计算是否连通并添加到矩阵
gen_sparse_matrix(void) {
  struct timeval start,stop; 
  gettimeofday(&start,NULL);
  struct Tri_arr *Tri_arr = gen_Tri_arr_bdd();
  gettimeofday(&stop,NULL);
  printf("gen_Tri_arr_bdd: %ld ms\n", diff(&stop, &start)/1000);
  printf("mf2bdd: %ld ms\n", time_counter_elemplus/1000);
  // printf("all num = %d\n", nTris);
  struct matrix_CSR *tmp = gen_matrix_CSR_from_Tris(Tri_arr);
  free(Tri_arr);
  return tmp;
}

struct matrix_CSC *
gen_CSC_from_CSR(struct matrix_CSR *matrix) {
  if(!matrix)
    return NULL;
  uint32_t valid_n = 0;
  for (uint32_t i = 0; i < matrix->nrows; i++) {
    if(matrix->rows[i])
      valid_n += matrix->rows[i]->nidx_vs;
  }
  // uint32_t max_CSR = MAX_VAL_RATE*data_allr_nums*data_allr_nums;
  // struct matrix_Tri_express *Tri_arr[valid_n];
  // printf("valid_n%d\n", valid_n);
  struct matrix_Tri_express **Tri_arr = xmalloc(valid_n*sizeof(struct matrix_Tri_express *));
  for (int i = 0; i < valid_n; i++)
    Tri_arr[i] = xcalloc(1,sizeof *(Tri_arr[i]));
  uint32_t count = 0;
  for (uint32_t i = 0; i < matrix->nrows; i++) {
    if(matrix->rows[i]){
      for (uint32_t j = 0; j < matrix->rows[i]->nidx_vs; j++){
        Tri_arr[count]->row_idx = i;
        Tri_arr[count]->col_idx = matrix->rows[i]->idx_vs[j]->idx;
        Tri_arr[count]->elem = matrix->rows[i]->idx_vs[j]->elem;
        count++;   
      }
    }
  }
  assert(!(valid_n - count));
  qsort(Tri_arr, valid_n,sizeof (struct matrix_Tri_express *), cmp_matrix_Tri_express_CSC);
  struct matrix_CSC *tmp = xmalloc(sizeof(uint32_t)+data_allr_nums*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->ncols = data_allr_nums;
  for (int i = 0; i < data_allr_nums; i++)
    tmp->cols[i] = NULL;

  uint32_t begin = 0, count_col = 1;
  for (uint32_t i = 1; i < count; i++) {
    if (Tri_arr[i]->col_idx != Tri_arr[i-1]->col_idx) {
      uint32_t col_idx = Tri_arr[begin]->col_idx;
      tmp->cols[col_idx] = xmalloc(sizeof(uint32_t)+count_col*sizeof(struct CS_matrix_idx_v *));
      tmp->cols[col_idx]->nidx_vs = count_col;
      for (uint32_t j = 0; j < count_col; j++){
        tmp->cols[col_idx]->idx_vs[j] = xmalloc(sizeof(struct CS_matrix_idx_v));
        tmp->cols[col_idx]->idx_vs[j]->idx = Tri_arr[begin+j]->row_idx;
        tmp->cols[col_idx]->idx_vs[j]->elem = Tri_arr[begin+j]->elem;
      }
      count_col = 0;
      begin = i;
    }
    count_col++; 
  }
  uint32_t col_idx = Tri_arr[begin]->col_idx;
  tmp->cols[col_idx] = xmalloc(sizeof(uint32_t)+count_col*sizeof(struct CS_matrix_idx_v *));
  tmp->cols[col_idx]->nidx_vs = count_col;
  for (uint32_t j = 0; j < count_col; j++){
    tmp->cols[col_idx]->idx_vs[j] = xmalloc(sizeof(struct CS_matrix_idx_v));
    tmp->cols[col_idx]->idx_vs[j]->idx = Tri_arr[begin+j]->row_idx;
    tmp->cols[col_idx]->idx_vs[j]->elem = Tri_arr[begin+j]->elem;
  }
  for (uint32_t i = 0; i < valid_n; i++) 
    free(Tri_arr[i]);
  free(Tri_arr);
  return tmp;
}


struct nf_space_pair *
nf_space_connect(struct nf_space_pair *a, struct nf_space_pair *b) {
  // printf("starting nf_space_connect\n");
  // if(!is_insc_links(a->out->lks, b->in->lks))
  //   return NULL;
  // struct timeval start,stop;  //计算时间差 usec
  // BDD root_a = load_saved_bddarr(a->out->mf);
  // BDD root_b = load_saved_bddarr(b->in->mf);
  // gettimeofday(&start,NULL);
  BDD insc = bdd_apply(a->out->mf, b->in->mf, bddop_and);
  // gettimeofday(&stop,NULL);
  // time_counter1 += diff(&stop, &start);

  computation_counter ++;


  if (!insc) 
    return NULL;
  // gettimeofday(&start,NULL);

  for (int i = 0; i < a->r_arr->nrs - 1; i++) {
    for (int j = 0; j < b->r_arr->nrs; j++) {
      if ((a->r_arr->ridx[i].sw_idx == b->r_arr->ridx[j].sw_idx)&&(a->r_arr->ridx[i].r_idx == b->r_arr->ridx[j].r_idx))
        return NULL;
    }
  }

  compu_true_counter ++;
  // struct bdd_saved_arr *bdd_arr_insc = bdd_save_arr(insc);
  struct nf_space_pair *pair_tmp = xcalloc(1, sizeof *pair_tmp);
  pair_tmp->in = xcalloc(1, sizeof *(pair_tmp->in));// 1,16(两个指针为16)
  pair_tmp->in->lks = copy_links_of_rule(a->in->lks);
  pair_tmp->out = xcalloc(1, sizeof *(pair_tmp->out));// 1,16(两个指针为16)
  pair_tmp->out->lks = copy_links_of_rule(b->out->lks);

  if (a->mask) {
    if(a->out->mf== insc){
      pair_tmp->in->mf = a->in->mf;
      bdd_addref(pair_tmp->in->mf);
    }
    else{
      pair_tmp->in->mf = bdd_rw_back_BDD(insc, a->in->mf, a->mask);
      bdd_addref(pair_tmp->in->mf);
    }

    if (b->mask) {
      pair_tmp->mask = xcalloc(1, sizeof *(pair_tmp->mask));
      pair_tmp->rewrite = xcalloc(1, sizeof *(pair_tmp->rewrite));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pair_tmp->mask->v[j] = (a->mask->v[j])&(b->mask->v[j]);
        pair_tmp->rewrite->v[j] = ((a->rewrite->v[j])&(b->mask->v[j])) | ((b->rewrite->v[j])&(~(b->mask->v[j])));
      }
    }
    else{
      pair_tmp->mask = xcalloc(1, sizeof *(pair_tmp->mask));
      pair_tmp->rewrite = xcalloc(1, sizeof *(pair_tmp->rewrite));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pair_tmp->mask->v[j] = a->mask->v[j];
        pair_tmp->rewrite->v[j] = a->rewrite->v[j];
      }
    }     
  }
  else{    
    pair_tmp->in->mf = insc;
    bdd_addref(pair_tmp->in->mf);
    if (!(b->mask)) {
      pair_tmp->mask = NULL;
      pair_tmp->rewrite = NULL;
    }
  }
  if (b->mask) {
    if(b->in->mf == insc){
      pair_tmp->out->mf = b->out->mf;
      bdd_addref(pair_tmp->out->mf);
    }
    else{
      pair_tmp->out->mf = bdd_rw_BDD(insc, b->mask, b->rewrite);
      bdd_addref(pair_tmp->out->mf);
    }
    if (!(a->mask)){
      // gettimeofday(&startin,NULL);
      pair_tmp->mask = xcalloc(1, sizeof *(pair_tmp->mask));
      pair_tmp->rewrite = xcalloc(1, sizeof *(pair_tmp->rewrite));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pair_tmp->mask->v[j] = b->mask->v[j];
        pair_tmp->rewrite->v[j] = b->rewrite->v[j];
      }
    }
  }
  else{
    pair_tmp->out->mf = insc;//建立copy
    bdd_addref(pair_tmp->out->mf);
  }

  // gettimeofday(&stop,NULL);
  // time_counter2 += diff(&stop, &start);
  // gettimeofday(&start,NULL);

  pair_tmp->r_arr = xmalloc(sizeof (uint32_t)+(a->r_arr->nrs+b->r_arr->nrs -1)*sizeof (struct r_idx));
  // pair_tmp->r_arr = xcalloc(1,sizeof (uint32_t)+(a->r_arr->nrs+b->r_arr->nrs -1)*sizeof (struct r_idx));

  pair_tmp->r_arr->nrs = a->r_arr->nrs+b->r_arr->nrs -1;
  for (uint32_t i = 0; i < a->r_arr->nrs; i++) {
    pair_tmp->r_arr->ridx[i].sw_idx = a->r_arr->ridx[i].sw_idx;
    pair_tmp->r_arr->ridx[i].r_idx = a->r_arr->ridx[i].r_idx;
  }
  for (uint32_t i = 0; i < b->r_arr->nrs -1; i++) {
    pair_tmp->r_arr->ridx[i+a->r_arr->nrs].sw_idx = b->r_arr->ridx[i+1].sw_idx;
    pair_tmp->r_arr->ridx[i+a->r_arr->nrs].r_idx = b->r_arr->ridx[i+1].r_idx;
  }

  // gettimeofday(&stop,NULL);
  // time_counter3 += diff(&stop, &start);

  return pair_tmp;
}

struct matrix_element * //a*b,a作用b，不可交换
elem_connect(struct matrix_element *a, struct matrix_element *b) { 
  
  // struct timeval start,stop; 
  struct nf_space_pair *nps[100000];
  // printf("%d\n", a->npairs*b->npairs);
  uint32_t count = 0;

  // gettimeofday(&start,NULL);
  for (uint32_t i = 0; i < a->npairs; i++) {
    struct nf_space_pair *np_a = a->nf_pairs[i];
    for (uint32_t j = 0; j < b->npairs; j++) {

      struct nf_space_pair *result = nf_space_connect(np_a, b->nf_pairs[j]);
      if (result) {
        nps[count] = result;
        count++;
      }
    }
  }
  // gettimeofday(&stop,NULL);
  // time_counter_nf_space_connect += diff(&stop, &start);
  struct matrix_element *tmp = NULL;
  // gettimeofday(&start,NULL);
  if (count) {
    tmp = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+count*sizeof(struct nf_space_pair *));
    tmp->bdd_in = 0;
    tmp->bdd_out = 0;
    tmp->npairs = count;
    for (int i = 0; i < count; i++) {
      tmp->bdd_in = bdd_apply(tmp->bdd_in, nps[i]->in->mf, bddop_or);
      tmp->bdd_out = bdd_apply(tmp->bdd_out, nps[i]->out->mf,bddop_or);
      tmp->nf_pairs[i] = nps[i];
    }
    bdd_addref(tmp->bdd_in);
    bdd_addref(tmp->bdd_out);
  } 
  // gettimeofday(&stop,NULL);
  // time_counter_elembdd_withpair += diff(&stop, &start);
  return tmp;
}

struct matrix_element *
row_col_multiply(struct CS_matrix_idx_v_arr *row, struct CS_matrix_idx_v_arr *col) {
  uint32_t num_row = row->nidx_vs;
  uint32_t num_col = col->nidx_vs;
  if ((row->idx_vs[0]->idx > col->idx_vs[num_col-1]->idx)||(row->idx_vs[num_row-1]->idx < col->idx_vs[0]->idx)) 
    return NULL;
  uint32_t count_row = 0, count_col = 0;
  struct matrix_element *tmp = NULL;
  struct matrix_element *elem_tmp = NULL;
  for (uint32_t i = 0; i < num_row + num_col; i++) {
    if ((row->idx_vs[count_row]->idx) == (col->idx_vs[count_col]->idx)){
      // struct timeval start,stop; 
      // gettimeofday(&start,NULL);
      // BDD insc = bdd_apply(row->idx_vs[count_row]->elem->bdd_out, col->idx_vs[count_col]->elem->bdd_in, bddop_and);
      // gettimeofday(&stop,NULL);
      // time_counter_eleminsc += diff(&stop, &start);
      elemconnet_counter ++;
      if(bdd_apply(row->idx_vs[count_row]->elem->bdd_out, col->idx_vs[count_col]->elem->bdd_in, bddop_and)) {
        // gettimeofday(&start,NULL);
        elem_tmp = elem_connect(row->idx_vs[count_row]->elem, col->idx_vs[count_col]->elem);
        
        // gettimeofday(&stop,NULL);
        // time_counter1 += diff(&stop, &start);
        if (elem_tmp)
          elem_true_counter++;
        
        // gettimeofday(&start,NULL);
        tmp = matrix_elem_plus(tmp, elem_tmp);
        // gettimeofday(&stop,NULL);
        // time_counter_elemplus += diff(&stop, &start);
        elem_tmp = NULL;
      }
      count_col++;
      count_row++;
    }
    else if ((row->idx_vs[count_row]->idx) > (col->idx_vs[count_col]->idx))
      count_col++;
    else
      count_row++;
    if ((count_row >= num_row)||(count_col>=num_col))
      break;
  }
  // if(tmp)
  //   printf("there has a computing\n");
  return tmp;
}

struct CS_matrix_idx_v_arr *
row_multi_col_multiply(struct CS_matrix_idx_v_arr *row, uint32_t *arr, uint32_t count, struct matrix_CSC *matrix_CSC) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;
  
  for (uint32_t i = 0; i < count; i++){
    struct matrix_element *elem_tmp = row_col_multiply(row, matrix_CSC->cols[arr[i]]);
    if (elem_tmp) {
      vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
      vs[vs_count]->idx = arr[i];
      vs[vs_count]->elem = elem_tmp;
      vs_count++;
    }
  }
  if(vs_count){
    tmp = xmalloc(sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  // qsort
  return tmp;
}

struct CS_matrix_idx_v_arr *
row_all_col_multiply(struct CS_matrix_idx_v_arr *row, struct matrix_CSC *matrix_CSC) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;

  for (uint32_t i = 0; i < matrix_CSC->ncols; i++){
    if ( matrix_CSC->cols[i]){
      struct matrix_element *elem_tmp = row_col_multiply(row, matrix_CSC->cols[i]);
      if (elem_tmp) {
        vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
        vs[vs_count]->idx = i;
        vs[vs_count]->elem = elem_tmp;
        vs_count++;
      }
    }   
  }

  if(vs_count){
    tmp = xmalloc(sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  // bdd_gbc();
  return tmp;
}

static int
CS_matrix_idx_v_cmp (const void *a, const void *b){
  struct CS_matrix_idx_v *va = *(struct CS_matrix_idx_v **)a;
  struct CS_matrix_idx_v *vb = *(struct CS_matrix_idx_v **)b;
  uint32_t c = (va->idx) - (vb->idx);
  return c;
}

struct CS_matrix_idx_v_arr *
row_matrix_CSR_multiply(struct CS_matrix_idx_v_arr *row, struct matrix_CSR *matrix_CSR) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;

  for (int i = 0; i < row->nidx_vs; i++) {
    struct CS_matrix_idx_v *row_idxv = row->idx_vs[i];
    if (matrix_CSR->rows[row_idxv->idx]) {
      struct CS_matrix_idx_v_arr *row_matrix = matrix_CSR->rows[row_idxv->idx];
      for (uint32_t j = 0; j < row_matrix->nidx_vs; j++) {
        elemconnet_counter ++;
        if(bdd_apply(row_idxv->elem->bdd_out, row_matrix->idx_vs[j]->elem->bdd_in, bddop_and)) {
          struct matrix_element *elem_tmp = elem_connect(row_idxv->elem, row_matrix->idx_vs[j]->elem);

          if (elem_tmp) {
            elem_true_counter++;
            if (vs_count){
              uint32_t sign = 1;
              for (int k = 0; k < vs_count; k++) {
                if(row_matrix->idx_vs[j]->idx == vs[k]->idx){
                  vs[k]->elem = matrix_elem_plus(vs[k]->elem, elem_tmp);
                  sign = 0;
                  break;
                }
              }
              if (sign) {
                vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
                vs[vs_count]->idx = row_matrix->idx_vs[j]->idx;
                vs[vs_count]->elem = elem_tmp;
                vs_count ++;
              }      
            }
            else {
              vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
              vs[vs_count]->idx = row_matrix->idx_vs[j]->idx;
              vs[vs_count]->elem = elem_tmp;
              vs_count ++;
            }     
          }
          elem_tmp = NULL;
        }
      }
    }
  }


  if(vs_count){
    qsort (vs, vs_count,sizeof(struct CS_matrix_idx_v *), CS_matrix_idx_v_cmp); 
    tmp = xmalloc(sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}

struct CS_matrix_idx_v_arr *
row_matrix_CSR_multiply_bysort(struct CS_matrix_idx_v_arr *row, struct matrix_CSR *matrix_CSR) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[100000];
  // printf("there not wrong\n");
  // uint32_t max_CSR = MAX_VAL_RATE*data_allr_nums*data_allr_nums;
  // struct matrix_Tri_express **Tri_arr = xmalloc(max_CSR*sizeof(struct matrix_Tri_express *));
  uint32_t vs_count = 0;

  for (int i = 0; i < row->nidx_vs; i++) {
    struct CS_matrix_idx_v *row_idxv = row->idx_vs[i];
    if (matrix_CSR->rows[row_idxv->idx]) {
      struct CS_matrix_idx_v_arr *row_matrix = matrix_CSR->rows[row_idxv->idx];
      for (uint32_t j = 0; j < row_matrix->nidx_vs; j++) {
        elemconnet_counter ++;
        if(bdd_apply(row_idxv->elem->bdd_out, row_matrix->idx_vs[j]->elem->bdd_in, bddop_and)) {
          struct matrix_element *elem_tmp = elem_connect(row_idxv->elem, row_matrix->idx_vs[j]->elem);

          if (elem_tmp) {
            elem_true_counter++;
            vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
            vs[vs_count]->idx = row_matrix->idx_vs[j]->idx;
            vs[vs_count]->elem = elem_tmp;
            vs_count ++;
            // if (vs_count>100000 - 1)
            //   printf("there is wrong\n");
          }     
          elem_tmp = NULL;
        }
      }
    }
  }


  if(vs_count){
    qsort (vs, vs_count,sizeof(struct CS_matrix_idx_v *), CS_matrix_idx_v_cmp); 
    uint32_t count = 1;
    for (uint32_t i = 1; i < vs_count; i++) {   
      if (vs[i]->idx == vs[count-1]->idx) {
        vs[count-1]->elem = matrix_elem_plus(vs[count-1]->elem, vs[i]->elem);//保留相同部分，r-r通过两条链路
        free(vs[i]);
        continue;
      }
      vs[count] = vs[i];
      count++; 
    }

    tmp = xmalloc(sizeof(uint32_t) + count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}

struct CS_matrix_idx_v_arr *
all_row_col_multiply(struct matrix_CSR *matrix_CSR, struct CS_matrix_idx_v_arr *col) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;

  for (uint32_t i = 0; i < matrix_CSR->nrows; i++){
    if (matrix_CSR->rows[i]){
      struct matrix_element *elem_tmp = row_col_multiply(matrix_CSR->rows[i], col);
      if (elem_tmp) {
        vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
        vs[vs_count]->idx = i;
        vs[vs_count]->elem = elem_tmp;
        vs_count++;
      }
    }   
  }

  if(vs_count){
    tmp = xmalloc(sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  uint32_t threshold = 60;
  struct timeval start,stop; 
  gettimeofday(&start,NULL);
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR1);
  gettimeofday(&stop,NULL);
  printf("gen CSC: %ld ms\n", diff(&stop, &start)/1000);
  
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {

        gettimeofday(&start,NULL);

        // tmp->rows[i] = row_matrix_CSR_multiply_bysort(matrix_CSR->rows[i], matrix_CSR1);
        tmp->rows[i] = row_matrix_CSR_multiply(matrix_CSR->rows[i], matrix_CSR1);
        gettimeofday(&stop,NULL);
        time_counter4+= diff(&stop, &start);
      //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      }
      else{
        // printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
        gettimeofday(&start,NULL);
        tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
        gettimeofday(&stop,NULL);
        time_counter5+= diff(&stop, &start);
      }
    }
  }

  free_matrix_CSC_fr_CSR(matrix_CSC);
  // printf("row number:%d\n", matrix_CSR->nrows);
  // printf("tmp:%d\n", tmp->nrows);
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply_CSC(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1, struct matrix_CSC *matrix_CSC) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  uint32_t threshold = 9000;
  struct timeval start,stop; 
  gettimeofday(&start,NULL);
  gettimeofday(&stop,NULL);
  printf("gen CSC: %ld ms\n", diff(&stop, &start)/1000);


  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {

        gettimeofday(&start,NULL);


        tmp->rows[i] = row_matrix_CSR_multiply_bysort(matrix_CSR->rows[i], matrix_CSR1);
        gettimeofday(&stop,NULL);
        time_counter4+= diff(&stop, &start);
      //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      }
      else{
        // printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
        gettimeofday(&start,NULL);
        tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
        gettimeofday(&stop,NULL);
        time_counter5+= diff(&stop, &start);
      }
    }
  }
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply_otway(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+data_allr_nums*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = data_allr_nums;
  for (uint32_t i = 0; i < data_allr_nums; i++)
    tmp->rows[i] = NULL;

  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      tmp->rows[i] = row_matrix_CSR_multiply(matrix_CSR->rows[i], matrix_CSR1);
    }
  }

  // printf("row number:%d\n", matrix_CSR->nrows);
  // printf("tmp:%d\n", tmp->nrows);
  return tmp;
}


int
get_value_num_matrix_CSR(struct matrix_CSR *matrix_CSR) {
  int count = 0;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      count += matrix_CSR->rows[i]->nidx_vs;
    }
  }
  return count;
}

struct matrix_CSR *
selected_rs_matrix_multiply(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC, struct u32_arrs *rs) {
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+matrix_CSR->nrows*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = rs->ns;
  for (uint32_t i = 0; i < tmp->nrows; i++)
    tmp->rows[i] = NULL;

  for (uint32_t i = 0; i < rs->ns; i++) {
    uint32_t idx = rs->arrs[i];
    if (matrix_CSR->rows[idx]){
      tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[idx], matrix_CSC);
    }
  }

  return tmp;
}


struct matrix_CSR *
sparse_matrix_multiply_nsqure(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+matrix_CSR->nrows*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;
  for (uint32_t i = 0; i < tmp->nrows; i++)
    tmp->rows[i] = NULL;

  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      // if (matrix_CSR->rows[i]->nidx_vs < threshold) {
      //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      //   // uint32_t arr[threshold*matrix_CSR->nrows];
      //   uint32_t *arr = xmalloc((threshold*matrix_CSR->nrows)*sizeof(uint32_t));
      //   uint32_t count = 0;
      //   struct CS_matrix_idx_v_arr *row = matrix_CSR->rows[i];
      //   for (uint32_t j = 0; j < row->nidx_vs; j++) {
      //     uint32_t col_row = row->idx_vs[j]->idx;
      //     struct CS_matrix_idx_v_arr *row_tmp = matrix_CSR->rows[col_row];
      //     if (row_tmp){
      //       for (uint32_t k = 0; k < row_tmp->nidx_vs; k++){
      //         arr[count] = row_tmp->idx_vs[k]->idx;
      //         count++;
      //       }
      //     }
      //   }
      //   if (count){
      //     uint32_t count1 = 1;
      //     qsort(arr, count,sizeof (uint32_t), uint32_t_cmp);
      //     uint32_t arr1[matrix_CSR->nrows];
      //     arr1[0] = arr[0];
      //     for (uint32_t j = 1; j < count; j++){
      //       if (arr[j] != arr[j-1]) {
      //         arr1[count1] = arr[j];
      //         count1++;
      //       }
      //     }
      //     tmp->rows[i] = row_multi_col_multiply(row, arr1, count1, matrix_CSC);
      //   }
      //   free(arr);
      // }
      // else{
      //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      //   tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      // }
    }
  }

  // printf("row number:%d\n", matrix_CSR->nrows);
  // printf("tmp:%d\n", tmp->nrows);
  return tmp;
}

void
BDD_init_matrix_multiply(void) {
  bdd_init(BDDSIZE, BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);

  bdd_done();
}

struct CS_matrix_idx_v_arr *
gen_matrix_row_CSR_fr_Tris(struct Tri_arr *Tri_arr) {
  if (Tri_arr->nTris==0){
    return NULL;
  }
  // printf("there is wrong: %d - %d\n", data_allr_nums, Tri_arr->nTris);
  qsort(Tri_arr->arr, Tri_arr->nTris,sizeof (struct matrix_Tri_express *), cmp_matrix_Tri_express);
  
  struct CS_matrix_idx_v_arr *tmp = xmalloc(sizeof(uint32_t)+Tri_arr->nTris*sizeof(struct CS_matrix_idx_v *));
  tmp->nidx_vs = Tri_arr->nTris;
  for (int i = 0; i < Tri_arr->nTris; i++) {
    tmp->idx_vs[i] = xmalloc(sizeof(struct CS_matrix_idx_v));
    tmp->idx_vs[i]->idx = Tri_arr->arr[i]->col_idx;
    tmp->idx_vs[i]->elem = Tri_arr->arr[i]->elem;
  }
  return tmp;
  // return NULL;
}


struct matrix_Tri_express * 
insc_to_Tri_express_rlimit_simple(uint32_t lk, struct of_rule *r_out, BDD v_and) {
  struct nf_space_pair *pair = xcalloc(1, sizeof *pair);

  struct links_of_rule *lks_out_tmp = xmalloc(sizeof(uint32_t)+sizeof (lks_out_tmp->links_wc[0]));
  lks_out_tmp->n = 1;
  lks_out_tmp->links_wc[0].w = 0x00;
  lks_out_tmp->links_wc[0].v = (uint16_t)lk;


  // struct links_of_rule *lks_in = rule_links_get(r_out, IN_LINK);
  // struct links_of_rule *lks_out = links_insc(lks_in, lks_out_tmp);
  // print_links_of_rule(lks_out_tmp);
  // print_links_of_rule(lks_out);
  // print_links_of_rule(lks_out);

  struct matrix_Tri_express *tmp = xcalloc(1, sizeof *tmp);
  tmp->elem = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+sizeof(struct nf_space_pair *));
  tmp->elem->npairs = 1;
  tmp->row_idx = 0;
  tmp->col_idx = matrix_idx_get_r(r_out);

  // struct bdd_saved_arr *bdd_arr_out = bdd_save_arr(v_and);
  // bdd_printtable(v_and);

  pair->in = xcalloc(1, sizeof *(pair->in));// 1,16(两个指针为16)
  pair->in->lks = NULL;
  pair->out = xcalloc(1, sizeof *(pair->out));
  pair->out->lks = lks_out_tmp;
  pair->r_arr = xmalloc(sizeof (uint32_t)+2*sizeof (struct r_idx));


  pair->r_arr->nrs = 2;
  pair->r_arr->ridx[0].sw_idx = -1;
  pair->r_arr->ridx[0].r_idx = -1;
  pair->r_arr->ridx[1].sw_idx = r_out->sw_idx;
  pair->r_arr->ridx[1].r_idx = r_out->idx;
  pair->out->mf = v_and;
  bdd_addref(pair->out->mf);
  tmp->elem->bdd_out = pair->out->mf;
  bdd_addref(pair->out->mf);


  pair->in->mf = v_and;
  bdd_addref(pair->in->mf);
  tmp->elem->bdd_in = pair->out->mf;
  bdd_addref(pair->in->mf);

  pair->mask = NULL;
  pair->rewrite = NULL;

  tmp->elem->nf_pairs[0] = pair;
  return tmp;
}

struct mf_uint16_t *
get_allx_mf_uint16_t(void) {
  struct mf_uint16_t *tmp = xcalloc(1, sizeof *tmp);
  init_mf_allx(tmp);
  return tmp;
}


struct Tri_arr *
gen_Tri_arr_bdd_fr_port(uint32_t inport) {

  uint32_t max_CSR = data_allr_nums;
  // printf("max_CSR%d\n", max_CSR);
  struct matrix_Tri_express *Tri_arr[max_CSR];
  uint32_t nTris = 0;
  uint32_t rule_nums_out = 0;
  struct u32_arrs *links = get_link_idx_from_inport(inport);
  print_u32_arrs(links);

  if (links) {
    printf("gen_Tri_arr_bdd_fr_port\n");
    struct link_to_rule *lout_r = get_inoutlink_rules(link_out_rule_file, &rule_nums_out, links->arrs[0]);
     if (lout_r){
      uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*(lout_r->rule_nums - rule_nums_out));

      struct mf_uint16_t *r_in_mf = get_allx_mf_uint16_t(); 
      BDD v_in, v_out; 
      v_in = mf2bdd(r_in_mf);
      // bdd_printtable(v_in);

      for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {
        // printf("%d-%d;\n", *(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
        struct of_rule *r_out = rule_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
        struct mf_uint16_t *r_out_mf = get_r_out_mf(r_out); 
        v_out = mf2bdd(r_out_mf);
        BDD v_and, v_diff;
        v_and = bdd_apply(v_in, v_out, bddop_and);
                  
        if (v_and){
          struct timeval start,stop;
          gettimeofday(&start,NULL);
          Tri_arr[nTris] = insc_to_Tri_express_rlimit_simple(links->arrs[0], r_out, v_and);
          nTris++;

          v_diff = bdd_apply(v_in, v_and, bddop_diff);
          gettimeofday(&stop,NULL);
          // printf("the diff =: %lld us\n", diff(&stop, &start));
          // free_mf_uint16_t_array(mfarr);
          // v_diff = bdd_apply(v_in, v_and, bddop_diff);
        }
        else {
          v_diff = v_in;
        } 
        v_in = v_diff;
        lout_arrs += 2;
        free(r_out_mf);

      } 
      free(r_in_mf);

    }
  }

  struct Tri_arr *tmp = xmalloc(sizeof(uint32_t)+nTris*sizeof(struct matrix_Tri_express *));
  tmp->nTris = nTris;
  printf("nTris %d\n", nTris);
  for (uint32_t i = 0; i < nTris; i++){
    tmp->arr[i] = Tri_arr[i];
  }
  return tmp;
}

struct CS_matrix_idx_v_arr * //通过对链路文件查找两个同链路的头尾端规则，计算是否连通并添加到矩阵
gen_sparse_matrix_row_fr_port(uint32_t inport) {
  struct Tri_arr *Tri_arr = gen_Tri_arr_bdd_fr_port(inport);
  printf("gen_sparse_matrix_row_fr_port\n");
  printf("all num = %d\n", Tri_arr->nTris);
  struct CS_matrix_idx_v_arr *tmp = gen_matrix_row_CSR_fr_Tris(Tri_arr);
  free(Tri_arr);
  return tmp;
}

struct CS_matrix_idx_v_arr * 
vec_matrix_multiply (struct CS_matrix_idx_v_arr *vec, struct matrix_CSC *matrix_CSC) {
  if(!vec)
    return NULL;
  // bdd_init(BDDSIZE, BDDOPCHCHE);
  // bdd_setvarnum(16*MF_LEN);
  struct CS_matrix_idx_v_arr *tmp = row_all_col_multiply(vec, matrix_CSC);
  // bdd_done();
  return tmp;
}

void
print_CS_matrix_idx_v_arr(struct CS_matrix_idx_v_arr *v_arr) {
  if (v_arr) {
    printf("All num of the arr is: %d\n", v_arr->nidx_vs);
    for (int i = 0; i < v_arr->nidx_vs; i++)
      printf("%d;", v_arr->idx_vs[i]->idx);
    printf("\n");
    
  }
  else
    printf("This vector is NULL\n");
}

void
print_CS_matrix_v_arr(struct CS_matrix_idx_v_arr *v_arr) {
  if (v_arr) {
    printf("All num of the arr is: %d\n", v_arr->nidx_vs);
    for (int i = 0; i < v_arr->nidx_vs; i++){
      printf("%d:", v_arr->idx_vs[i]->idx);
      print_matrix_element(v_arr->idx_vs[i]->elem);
    }
    printf("\n");
    
  }
  else
    printf("This vector is NULL\n");
}

void
BDD_init_multiply(void) {

  bdd_init(BDDSIZE, BDDSIZE,BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
}


/*提取rules 到用于冲突处理的结构体*/
/*========================================================================*/
struct network_wc {
  struct switch_rs *sws[SW_NUM];
};

struct ex_rule *
gen_ex_rule_from_of(struct of_rule *of_r) {
  if (!of_r)
    return NULL;
  int add_len = sizeof(uint16_t);
  struct ex_rule *tmp = xcalloc(1, sizeof *tmp);
  tmp->sw_idx = of_r->sw_idx;
  tmp->idx = of_r->idx;
  struct links_of_rule *lks_out = rule_links_get(of_r, OUT_LINK);
  struct links_of_rule *lks_in = rule_links_get(of_r, IN_LINK);
  tmp->lks_in = copy_links_of_rule(lks_in);
  tmp->lks_out = copy_links_of_rule(lks_out);
  tmp->mf_in = get_r_out_mf(of_r);
  tmp->mf_out = get_r_in_mf(of_r);
  if (of_r->mask) {
    tmp->mask = xcalloc(1,sizeof(struct mask_uint16_t));
    tmp->rewrite = xcalloc(1,sizeof(struct mask_uint16_t));
    for (uint32_t j = 0; j < MF_LEN; j++) {
      tmp->mask->v[j] = *(uint16_t *)((uint8_t *)data_arrs+j*add_len+of_r->mask);
      tmp->rewrite->v[j] = *(uint16_t *)((uint8_t *)data_arrs+j*add_len+of_r->rewrite);
    }
  }
  else {
    tmp->mask = NULL;
    tmp->rewrite = NULL;
  }
  return tmp;
}

struct switch_rs *
gen_sw_rules(uint32_t sw_idx) {
  struct sw *sw = sw_get(sw_idx);

  if(!sw)
    return NULL;
  struct switch_rs *tmp = xmalloc(2*sizeof(uint32_t)+(sw->nrules)*sizeof(struct ex_rule *));
  tmp->sw_idx = sw->sw_idx;
  tmp->nrules = sw->nrules;
  for (int i = 0; i < tmp->nrules; i++) {
    struct of_rule *of_r = rule_get(sw, i+1);
    tmp->rules[i] = gen_ex_rule_from_of(of_r);
  }

  return tmp;
}

struct network_wc *
gen_network_wc(void){
  struct network_wc *tmp = xmalloc(sizeof(*tmp));
  printf("Switch - nrules :");
  for (int i = 0; i < SW_NUM; i++) {
    tmp->sws[i] = gen_sw_rules(i);
    printf(" %d - %d;", tmp->sws[i]->sw_idx, tmp->sws[i]->nrules);
  }
  printf("\n");
  return tmp;
}



/*trie结构*/
/*========================================================================*/

void
print_ex_rules_arr(struct ex_rules_arr *rules_arr) {
  if(!rules_arr)
    printf("NULL;");
  for (int i = 0; i < rules_arr->nrules; i++)
    printf("%d - %d, ", rules_arr->rules[i]->sw_idx, rules_arr->rules[i]->idx);
}

void
print_trie_node(struct trie_node *tn) {
  if (!tn)
    printf("This node isn't existed!\n");
  else{
    if (tn->isleaf){
      printf("The terminal trie node: ");
      printf("level: %d;", tn->level);
      printf("local_rules:");
      print_ex_rules_arr(tn->local_rules);
      printf("relevent_rules:");
      print_ex_rules_arr(tn->relevent_rules);

    }
    else {
      printf("The normal trie node: ");
      printf("level: %d;", tn->level);
    }
    printf("\n");
  }
}

struct trie_node *
crate_trie_node_init(void) {
  struct trie_node *tmp = xcalloc(1, sizeof *tmp);
  tmp->level = 0;
  tmp->isleaf = 0;
  tmp->reverse_node = NULL;
  tmp->local_rules = NULL;
  tmp->relevent_rules = NULL;
  for (int i = 0; i < 3; i++)
    tmp->branchs[i] = NULL;
  return tmp;
}

uint32_t
local_rules_has_r(struct ex_rules_arr *local_rs, struct ex_rule *r){//1->has same r
  if (!local_rs) {
    return 0;
  }
  for (int i = 0; i <local_rs->nrules; i++) {
    if ((r->sw_idx == local_rs->rules[i]->sw_idx)&& (r->idx == local_rs->rules[i]->idx))
      return 1;
  }
  return 0;
}

int
ex_rule_cmp(const void *a, const void *b){
  struct ex_rule *r1 = *(struct ex_rule **)a;
  struct ex_rule *r2 = *(struct ex_rule **)b;

  if (r1->sw_idx != r2->sw_idx)
    return (r1->sw_idx - r2->sw_idx);
  return (r1->idx - r2->idx);
}

struct ex_rules_arr *
add_rule_to_ex_rules_arr(struct ex_rules_arr *local_rs, struct ex_rule *r){
  if (!local_rs) {
    struct ex_rules_arr *tmp = xcalloc(1, sizeof *tmp);
    tmp->nrules = 1;
    tmp->rules[0] = r;
    return tmp;
  }

  uint32_t *b = (uint32_t *)bsearch(&r, local_rs->rules, local_rs->nrules,sizeof(struct ex_rule *), ex_rule_cmp);
  if(b)
    return local_rs;
  // for (int i = 0; i <local_rs->nrules; i++) {
  //   if ((r->sw_idx == local_rs->rules[i]->sw_idx)&& (r->idx == local_rs->rules[i]->idx))
  //     return local_rs;
  // }
  struct ex_rules_arr *tmp = xmalloc(sizeof(uint32_t)+(local_rs->nrules+1)*sizeof(struct ex_rule *));
  tmp->nrules = local_rs->nrules + 1;
  for (int i = 0; i < local_rs->nrules; i++) 
    tmp->rules[i] = local_rs->rules[i];
  tmp->rules[local_rs->nrules] = r;
  qsort(tmp->rules, tmp->nrules,sizeof(struct ex_rule *), ex_rule_cmp);
  free(local_rs);
  return tmp;
}

uint32_t
trie_insert_rule(struct trie_node *root, struct ex_rule *r) {
  struct trie_node *tmp = root;
  struct mf_uint16_t *mf = r->mf_in;
  for (int i = 0; i < MF_LEN; i++){
    uint16_t sign = 0x8000;
    for (int j = 0; j < 16; j++){
      if (sign & mf->mf_w[i]) {
        if (tmp->branchs[2]) {//*
          tmp = tmp->branchs[2];
        }
        else {
          tmp->branchs[2] = crate_trie_node_init();
          tmp->branchs[2]->level = tmp->level+1;
          tmp->branchs[2]->reverse_node = tmp;
          tmp = tmp->branchs[2];
        }
      }
      else {
        if (sign & mf->mf_v[i]) {
          if (tmp->branchs[1]) {//1
            tmp = tmp->branchs[1];
          }
          else {
            tmp->branchs[1] = crate_trie_node_init();
            tmp->branchs[1]->level = tmp->level+1;
            tmp->branchs[1]->reverse_node = tmp;
            tmp = tmp->branchs[1];
          }
        }
        else {
          if (tmp->branchs[0]) {//0
            tmp = tmp->branchs[0];
          }
          else {
            tmp->branchs[0] = crate_trie_node_init();
            tmp->branchs[0]->level = tmp->level+1;
            tmp->branchs[0]->reverse_node = tmp;
            tmp = tmp->branchs[0];
          }
        }
      }
      sign >>= 1;
    }  
  }
  tmp->isleaf = 1;
  if (local_rules_has_r(tmp->local_rules, r))
    return 0;
  tmp->local_rules = add_rule_to_ex_rules_arr(tmp->local_rules, r);
  return 1;
}

void
trie_find_insect_rules(struct trie_node *root, struct ex_rule *r) { //breadth first search
  // struct trie_node *tmp = root;
  struct mf_uint16_t *mf = r->mf_in;
  uint32_t count = 1;
  struct trie_node *arr[data_allr_nums];
  arr[0] = root;
  // printf("this is right\n");
  for (int i = 0; i < MF_LEN; i++) {
    uint16_t sign = 0x8000;
    for (int j = 0; j < 16; j++) {
      uint32_t count_tmp = 0;
      struct trie_node *tmp[data_allr_nums];
      if (sign & mf->mf_w[i]) {
        for (int k = 0; k < count; k++) {
          if (arr[k]->branchs[0]){
            tmp[count_tmp] = arr[k]->branchs[0];
            count_tmp++;
          }
          if (arr[k]->branchs[1]){
            tmp[count_tmp] = arr[k]->branchs[1];
            count_tmp++;
          }
          if (arr[k]->branchs[2]){
            tmp[count_tmp] = arr[k]->branchs[2];
            count_tmp++;
          }   
        }
      }
      else {
        if (sign & mf->mf_v[i]) {
          for (int k = 0; k < count; k++) {
            if (arr[k]->branchs[1]){
              tmp[count_tmp] = arr[k]->branchs[1];
              count_tmp++;
            }
            if (arr[k]->branchs[2]){
              tmp[count_tmp] = arr[k]->branchs[2];
              count_tmp++;
            }   
          }
        }
        else {
          for (int k = 0; k < count; k++) {
            if (arr[k]->branchs[0]){
              tmp[count_tmp] = arr[k]->branchs[0];
              count_tmp++;
            }
            if (arr[k]->branchs[2]){
              tmp[count_tmp] = arr[k]->branchs[2];
              count_tmp++;
            }   
          }
        }
      }
      sign >>= 1;
      for (int k = 0; k < count_tmp; k++) {
        arr[k] = tmp[k];
      }
      count = count_tmp;
    }  
  }
  for (int i = 0; i < count; i++)
    arr[i]->relevent_rules = add_rule_to_ex_rules_arr(arr[i]->relevent_rules, r);
}



uint32_t
trie_add_rule(struct trie_node *root, struct ex_rule *r) {
  uint32_t haschange = trie_insert_rule (root, r);
  if (!haschange)
    return 0;
  trie_find_insect_rules(root, r);
  return 1;
}


void
trie_add_rules_for_sw(struct trie_node *root, struct switch_rs *sw) {
  for (int i = 0; i < sw->nrules; i++) {
    trie_add_rule(root, sw->rules[i]);
  }
}

void
greed_calc_arule_insc_sw(struct switch_rs *sw, uint32_t idx) {
  uint32_t insc_counter = 0;
  for (int i = 0; i < sw->nrules; i++) {
    struct mf_uint16_t *a = calc_insc(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in);
    if(a){
      insc_counter++;
      free(a);
    }
  }
  printf("This rule insect %d rules\n", insc_counter);
}



// uint32_t field_sign[8] = {0, 2, 6, 10, 11, 13, 15, 16};
// uint32_t field_bit[7] = {16, 32, 32, 8, 16, 16, 8};
// uint32_t field_sign[2] = {0, 4};
// uint32_t field_bit[1] = {32};
uint32_t field_sign[3] = {0, 2, 6};
uint32_t field_bit[2] = {16, 32};

// struct ecs {
//   uint32_t necs;
//   struct ec *ecs[0];
// };



// struct trie_node_arr{
//   uint32_t nnodes;
//   struct trie_node *nodes[0];
// };

// struct ec_1f {
//   uint32_t l_b;
//   uint32_t h_b;
// };

// struct ec {
//   struct ec_1f ec_vs[FIELD_LEN];
// };

// struct ec_1f_node {
//   uint32_t idx;
//   uint32_t l_b;
//   uint32_t h_b;
//   struct trie_node *node;
// };

// struct bound {
//   uint32_t bound;
//   uint32_t idx;
// };



uint32_t
trie_process_ec_arule(struct trie_node *root, struct ex_rule *r) {
  struct mf_uint16_t *mf = r->mf_in;
  uint32_t count = 1;
  struct trie_node *arr[data_allr_nums];
  arr[0] = root;

  struct timeval start,stop;
 
  uint32_t field_sign[1] = {0};
  uint32_t field_len[1] = {32};


  // gettimeofday(&start,NULL);
  // uint32_t node_tr_counter = 0;
  // printf("this is right\n");

  //find all the inscrules
  for (int i = 0; i < MF_LEN; i++) {
    uint16_t sign = 0x8000;
    for (int j = 0; j < 16; j++) {
      uint32_t count_tmp = 0;
      struct trie_node *tmp[data_allr_nums];
      if (sign & mf->mf_w[i]) {
        for (int k = 0; k < count; k++) {
          if (arr[k]->branchs[0]){
            tmp[count_tmp] = arr[k]->branchs[0];
            count_tmp++;
            // node_tr_counter++;

          }
          if (arr[k]->branchs[1]){
            tmp[count_tmp] = arr[k]->branchs[1];
            count_tmp++;
            // node_tr_counter++;
          }
          if (arr[k]->branchs[2]){
            tmp[count_tmp] = arr[k]->branchs[2];
            count_tmp++;
            // node_tr_counter++;
          }   
        }
      }
      else {
        if (sign & mf->mf_v[i]) {
          for (int k = 0; k < count; k++) {
            if (arr[k]->branchs[1]){
              tmp[count_tmp] = arr[k]->branchs[1];
              count_tmp++;
              // node_tr_counter++;
            }
            if (arr[k]->branchs[2]){
              tmp[count_tmp] = arr[k]->branchs[2];
              count_tmp++;
              // node_tr_counter++;
            }   
          }
        }
        else {
          for (int k = 0; k < count; k++) {
            if (arr[k]->branchs[0]){
              tmp[count_tmp] = arr[k]->branchs[0];
              count_tmp++;
              // node_tr_counter++;
            }
            if (arr[k]->branchs[2]){
              tmp[count_tmp] = arr[k]->branchs[2];
              count_tmp++;
              // node_tr_counter++;
            }   
          }
        }
      }
      sign >>= 1;
      for (int k = 0; k < count_tmp; k++) {
        arr[k] = tmp[k];
      }
      count = count_tmp;
    }  
  }

  // gettimeofday(&stop,NULL);
  // long long int T_for_add = diff(&stop, &start);
  // printf("Trie test rule with %d - %d: %lld us with searching\n", r->sw_idx, r->idx, T_for_add);
  // printf("node_tr_counter is %d\n", node_tr_counter);
  // gettimeofday(&start,NULL);

  struct ex_rule *r_arr[data_allr_nums];
  uint32_t insc_r_count = 0;
  for (int i = 0; i < count; i++){
    for (int j = 0; j < arr[i]->local_rules->nrules; j++) {
      r_arr[insc_r_count] = arr[i]->local_rules->rules[j];
      insc_r_count++;
    }
    r_arr[insc_r_count] = arr[i]->local_rules->rules[0];
    insc_r_count++;
  }

  // for (int field_i = 0; field_i < FIELD_LEN; field_i++) {//standford data has 7fields
  //   uint32_t sign = 0;
  //   // uint16_t upmask = 0x8000;
  //   // uint16_t downmask = 0x0080;
  //   uint32_t len = field_sign[field_i+1] - field_sign[field_i];
  //   // sign = field_sign[field_i]/2;
  //   uint32_t iscross = 0;
  //   // if (field_sign[field_i]%2 != 0) {
  //   //   // sign = field_sign[field_i]/2;
  //   //   iscross = 1;
  //   // }
  //   // else{
  //   //   // sign = field_sign[field_i]/2;
  //   //   iscross = 0;
  //   // }
  //   // uint32_t l_v_arr[insc_r_count];
  //   // uint32_t h_v_arr[insc_r_count];
  //   uint32_t v_arr[2*insc_r_count];

  //   for (int r_i = 0; r_i < insc_r_count; r_i++) {//for every r in the insect set
  //     uint32_t h_v = 0;
  //     uint32_t l_v = 0;
  //     for (int len_i = 0; len_i < len; len_i++) {
  //       uint16_t masksign = 0;
  //       uint32_t mf_sign = (field_sign[field_i] + len_i)/2;
  //       if ((field_sign[field_i] + len_i)%2)
  //         masksign = 0x0080;
  //       else
  //         masksign = 0x8000;
  //       for (int c_i = 0; c_i < 8; c_i++) {
  //         if (masksign & r_arr[r_i]->mf_in->mf_w[mf_sign]) {//wildcard 
  //           h_v |= 1;
  //         }
  //         else if (masksign & r_arr[r_i]->mf_in->mf_v[mf_sign]) {//1
  //           l_v |= 1;
  //           h_v |= 1;
  //         }
  //         // else {
  //         //   l_v <<= 1;
  //         //   h_v <<= 1;
  //         // }
  //         if ((len_i == (len-1))&&(c_i == 7))
  //           break;
  //         l_v <<= 1;
  //         h_v <<= 1;
  //         masksign >>=1;
  //       }
  //     }
  //     v_arr[r_i*2] = l_v;
  //     v_arr[r_i*2+1] = h_v;
  //   }

  //   uint32_t h_end = 0;
  //   uint32_t l_last = 0;
  //   for (int len_i = 0; len_i < len; len_i++) {
  //     uint16_t masksign = 0;
  //     uint32_t mf_sign = (field_sign[field_i] + len_i)/2;
  //     if ((field_sign[field_i] + len_i)%2)
  //       masksign = 0x0080;
  //     else
  //       masksign = 0x8000;
  //     for (int c_i = 0; c_i < 8; c_i++) {
  //       if (masksign & r->mf_in->mf_w[mf_sign]) {//wildcard 
  //         h_end |= 1;
  //       }
  //       else if (masksign & r->mf_in->mf_v[mf_sign]) {//1
  //         l_last |= 1;
  //         h_end |= 1;
  //       }
  //       // else {
  //       //   l_last <<= 1;
  //       //   h_v <<= 1;
  //       // }
  //       if ((len_i == (len-1))&&(c_i == 7))
  //         break;
  //       l_last <<= 1;
  //       h_end <<= 1;
  //       masksign >>=1;
  //     }
  //   }


  //   qsort(v_arr, 2*insc_r_count,sizeof(uint32_t), uint32_t_cmp);

  //   uint32_t last = v_arr[0];

  //   for (int i = 0; i < 2*insc_r_count; i++) {

  //     if(v_arr[i] > last){
  //       struct ec_1f *tmp = xmalloc(sizeof *tmp);
  //       tmp->l_b = last;
  //       tmp->h_b = v_arr[i];
  //       free(tmp);
  //       last = v_arr[i];
  //     }
  //   }
  // }
  // gettimeofday(&stop,NULL);
  // T_for_add = diff(&stop, &start);
  // printf("Trie test rule with %d - %d: %lld us with computing EC\n", r->sw_idx, r->idx, T_for_add);
  // printf("This rule insect %d rules\n", insc_r_count);
    // arr[i]->relevent_rules = add_rule_to_ex_rules_arr(arr[i]->relevent_rules, r);

  return insc_r_count;
}

static uint32_t
bound_cmp (const void *a, const void *b){
  uint32_t a32 = *(uint32_t *)a;
  uint32_t b32 = *(uint32_t *)b;
  if (a32 > b32)
    return 5;
  else if (a32 == b32)
    return 0;
  return -5;
  // return *(uint32_t *)a - *(uint32_t *)b; 
}

struct ecs *
get_ecs_rec(struct trie_node_arr *tn_arr, struct ec *r_ec, uint32_t field_i){
  // printf("%d\n", tn_arr->nnodes);
  // uint32_t sign 
  
  struct ec *ecs_arr[5000];
  uint32_t count = 0;
  struct ec_1f_node e1ns[5000];
  uint32_t count_e1ns = 0;
  // printf("there is true\n");
  for (int i = 0; i < tn_arr->nnodes; i++) {
    // printf("tn_arr->nnodes %d,%d\n", tn_arr->nnodes, tn_arr->nodes[i]);
    struct trie_node *ctn = tn_arr->nodes[i];
    bool matchFound = true;
    uint32_t rlb = 0;
    uint32_t rhb = 0;
    uint32_t last = 0;

    // printf("%x - %x\n", r_ec->ec_vs[field_i].l_b, r_ec->ec_vs[field_i].h_b);
    // printf("%d-%d-%d\n", ctn->branchs[0], ctn->branchs[1], ctn->branchs[2]);
    for (int j = 0; j < field_bit[field_i]; j++) {
      // printf("this is true\n");
      last = j+1;
      uint32_t maskBit = (uint32_t)1 << ((field_bit[field_i] - 1) - j);
      
      // printf("%x\n", maskBit);
      

      if ((r_ec->ec_vs[field_i].l_b & maskBit) != (r_ec->ec_vs[field_i].h_b & maskBit)){
        // if(j == 0)
        last = j;
        // last = j;
        matchFound = true;
        break;
      }
      else if((r_ec->ec_vs[field_i].l_b & maskBit) == 0){
        if (ctn->branchs[0]){
          rlb <<= 1;
          rhb <<= 1; 
          ctn = ctn->branchs[0];
        }
        else{
          matchFound = false;
          break;
        }
      }
      else{
        if (ctn->branchs[1]){
          rlb <<= 1;
          rhb <<= 1; 
          rlb |= 1;
          rhb |= 1;
          ctn = ctn->branchs[1];
        }
        else{
          matchFound = false;
          break;
        }
      }
      // if(last!=field_bit[field_i]){
      //   rlb <<= 1;
      //   rhb <<= 1; 
      // }  
    }
    if(matchFound == false)
      continue;
    if (last == field_bit[field_i]) {
      e1ns[count_e1ns].idx = count_e1ns;
      e1ns[count_e1ns].l_b = rlb;
      e1ns[count_e1ns].h_b = rhb;
      e1ns[count_e1ns].node = ctn;
      count_e1ns++;
      continue;
    }
    // printf("there is true\n");
    struct ec_1f_node tmp1[5000];
    uint32_t counttmp1=0;
    uint32_t counttmp2=0;
    struct ec_1f_node tmp2[5000];
    if (last%2) {
      tmp1[0].node = ctn;
      tmp1[0].l_b = rlb;
      tmp1[0].h_b = rhb;
      counttmp1 = 1;
      
      // printf("%d-%d-%d\n", ctn->branchs[0], ctn->branchs[1], ctn->branchs[2]);
    }
    else{
      tmp2[0].node = ctn;
      tmp2[0].l_b = rlb;
      tmp2[0].h_b = rhb;
      counttmp2 = 1;
      // printf("%x - %x\n", r_ec->ec_vs[field_i].l_b, r_ec->ec_vs[field_i].h_b);
      // printf("%d-%d-%d\n", ctn->branchs[0], ctn->branchs[1], ctn->branchs[2]);
    }
    // printf("this is true %d run\n", i);
    for (int j = last; j < field_bit[field_i]; j++) {
      // printf("j %d\n", j);
      // bool arrsign = false;
      if (j%2) {
        counttmp2 = 0; 
        for (int node_i = 0; node_i < counttmp1; node_i++) {
          // printf("this is true %d\n", tmp1[node_i].node);
          // printf("this is true %d - %d - %d\n", tmp1[node_i].node->branchs[0], tmp1[node_i].node->branchs[1], tmp1[node_i].node->branchs[2]);
          if (tmp1[node_i].node->branchs[0]){

            tmp2[counttmp2].node = tmp1[node_i].node->branchs[0];
            tmp2[counttmp2].l_b = tmp1[node_i].l_b;
            tmp2[counttmp2].h_b = tmp1[node_i].h_b;
            tmp2[counttmp2].l_b <<= 1;
            tmp2[counttmp2].h_b <<= 1;
            counttmp2++;
          }
          if (tmp1[node_i].node->branchs[1]){
            tmp2[counttmp2].node = tmp1[node_i].node->branchs[1];
            tmp2[counttmp2].l_b = tmp1[node_i].l_b;
            tmp2[counttmp2].h_b = tmp1[node_i].h_b;
            tmp2[counttmp2].l_b <<= 1;
            tmp2[counttmp2].h_b <<= 1;
            tmp2[counttmp2].l_b |= 1;
            tmp2[counttmp2].h_b |= 1;
            counttmp2++;
          }
          if (tmp1[node_i].node->branchs[2]){
            tmp2[counttmp2].node = tmp1[node_i].node->branchs[2];
            tmp2[counttmp2].l_b = tmp1[node_i].l_b;
            tmp2[counttmp2].h_b = tmp1[node_i].h_b;
            tmp2[counttmp2].l_b <<= 1;
            tmp2[counttmp2].h_b <<= 1;
            tmp2[counttmp2].h_b |= 1;
            counttmp2++;
          }
        }
      }
      else{
        counttmp1 = 0; 
        for (int node_i = 0; node_i < counttmp2; node_i++) {
          // printf("this is true %d\n", tmp2[node_i].node);
          // printf("this is true %d - %d - %d\n", tmp2[node_i].node->branchs[0], tmp2[node_i].node->branchs[1], tmp2[node_i].node->branchs[2]);
          if (tmp2[node_i].node->branchs[0]){
            tmp1[counttmp1].node = tmp2[node_i].node->branchs[0];
            tmp1[counttmp1].l_b = tmp2[node_i].l_b;
            tmp1[counttmp1].h_b = tmp2[node_i].h_b;
            tmp1[counttmp1].l_b <<= 1;
            tmp1[counttmp1].h_b <<= 1;
            counttmp1++;
          }
          if (tmp2[node_i].node->branchs[1]){
            tmp1[counttmp1].node = tmp2[node_i].node->branchs[1];
            tmp1[counttmp1].l_b = tmp2[node_i].l_b;
            tmp1[counttmp1].h_b = tmp2[node_i].h_b;
            tmp1[counttmp1].l_b <<= 1;
            tmp1[counttmp1].h_b <<= 1;
            tmp1[counttmp1].l_b |= 1;
            tmp1[counttmp1].h_b |= 1;
            counttmp1++;
          }
          if (tmp2[node_i].node->branchs[2]){
            tmp1[counttmp1].node = tmp2[node_i].node->branchs[2];
            tmp1[counttmp1].l_b = tmp2[node_i].l_b;
            tmp1[counttmp1].h_b = tmp2[node_i].h_b;
            tmp1[counttmp1].l_b <<= 1;
            tmp1[counttmp1].h_b <<= 1;
            tmp1[counttmp1].h_b |= 1;
            counttmp1++;
          }
        }
      }     
    }
    // printf("this is true %d\n", count_e1ns);
    if (counttmp2) {
      for (int tmp2_i = 0; tmp2_i < counttmp2; tmp2_i++) {
        e1ns[count_e1ns].idx = count_e1ns;
        e1ns[count_e1ns].l_b = tmp2[tmp2_i].l_b;
        e1ns[count_e1ns].h_b = tmp2[tmp2_i].h_b;
        e1ns[count_e1ns].node = tmp2[tmp2_i].node;
        // printf("%x-%x\n", e1ns[count_e1ns].l_b, e1ns[count_e1ns].h_b);
        count_e1ns++;
        // printf("%x-%x\n", tmp2[tmp2_i].l_b, tmp2[tmp2_i].h_b);
        // printf("%d-%d-%d\n", tmp2[tmp2_i].node->branchs[0], tmp2[tmp2_i].node->branchs[1], tmp2[tmp2_i].node->branchs[2]);
        // printf("%d-%d-%d\n", e1ns[count_e1ns-1].node->branchs[0], e1ns[count_e1ns-1].node->branchs[1], e1ns[count_e1ns-1].node->branchs[2]);

      }
    }
  }
  // printf("this point\n");
  // printf("%d - %d\n", field_i, FIELD_LEN);
  // printf("this is true\n");
  // bool eqsign = false;

  // printf("this is true %d\n", count_e1ns);


  if (count_e1ns) {
    struct bound bounds[2*count_e1ns];
    for (int i = 0; i < count_e1ns; i++){
      bounds[2*i].bound =  e1ns[i].l_b;
      bounds[2*i].idx = e1ns[i].idx;
      bounds[2*i+1].bound =  e1ns[i].h_b;
      bounds[2*i+1].idx = e1ns[i].idx;
      // printf("-%x-%x\n",e1ns[i].l_b,e1ns[i].h_b);
      // printf("-%x-%x\n",bounds[2*i].bound,bounds[2*i+1].bound);
    }
    // printf("-%x-%x\n",bounds[0].bound,bounds[1].bound);
    qsort(bounds, 2*count_e1ns,2*sizeof(uint32_t), bound_cmp);
    uint32_t last = r_ec->ec_vs[field_i].l_b;

    uint32_t arr[50000];
    arr[0] = bounds[0].idx;
    uint32_t count_tn_arr = 1;
    uint32_t eq_arr[50000];
    uint32_t count_eq_arr = 0;
    for (int i = 1; i < 2*count_e1ns; i++) {
      bool in = false;
      // bool delect_sign = false;
      uint32_t delect = 0;
      for (int j = 0; j < count_tn_arr; j++) {
        if(bounds[i].idx == arr[j]){
          in = true;
          delect = j;
          // arr[j] = arr[count_tn_arr-1];
          // count_tn_arr--;
          break;
        }
      }
      // printf("%d ---- count_tn_arr %d -------%d-%d-----last %x-%x-%x\n",in,  count_tn_arr, bounds[0].idx, bounds[1].idx, last,bounds[0].bound,bounds[1].bound);

      if (last < bounds[i].bound) {    
        if (count_eq_arr)  {
          if (field_i == FIELD_LEN-1) {
            ecs_arr[count] = xmalloc(sizeof(struct ec));
            ecs_arr[count]->ec_vs[field_i].l_b = last;
            ecs_arr[count]->ec_vs[field_i].h_b = last; 
            count++;
          }
          else{
            if(count_tn_arr+count_eq_arr){
              struct trie_node_arr *tmp = xmalloc(sizeof(uint32_t)+(count_tn_arr+count_eq_arr)*sizeof(struct trie_node *));
              tmp->nnodes = count_tn_arr+count_eq_arr;
              for (int tn_i = 0; tn_i < count_tn_arr; tn_i++) {
                tmp->nodes[tn_i] = e1ns[arr[tn_i]].node;
                // printf("%d-%d-%d-%d\n", tmp->nodes[i], tmp->nodes[i]->branchs[0], tmp->nodes[i]->branchs[1], tmp->nodes[i]->branchs[2]);
              }
              for (int tn_i = 0; tn_i < count_eq_arr; tn_i++) {
                tmp->nodes[tn_i+count_tn_arr] = e1ns[eq_arr[tn_i]].node;
                // printf("%d-%d-%d-%d\n", tmp->nodes[i+count_tn_arr], tmp->nodes[i+count_tn_arr]->branchs[0], tmp->nodes[i+count_tn_arr]->branchs[1], tmp->nodes[i+count_tn_arr]->branchs[2]);
              }
              struct ecs *ecsget = get_ecs_rec(tmp, r_ec, field_i+1);
              if(ecsget){
                // printf("get_ecs_rec\n");
                for (int j = 0; j < ecsget->necs; j++) {
                  // for (int field = field_i+1; field < FIELD_LEN; field++){
                  //   // ecs[count]->ec_vs[field].h_b = ecsget->ecs[j]->ec_vs[field].h_b;
                  //   // ecs[count]->ec_vs[field].h_b = ecsget->ecs[j]->ec_vs[field].h_b;
                    
                  // }
                  ecs_arr[count] = ecsget->ecs[j];
                  ecs_arr[count]->ec_vs[field_i].l_b = last;
                  ecs_arr[count]->ec_vs[field_i].h_b = last;
                  count++;
                }
                // printf("%d\n", ecsget->necs);
                free(ecsget);
                // printf("free this\n");
                ecsget = NULL;
              }
              // printf("%d\n", tmp);
              free(tmp);
              tmp = NULL;
            }
          }
          // eqsign = false;
          last = last+1;
          count_eq_arr = 0;
        }

        if (in) {
          // printf("%x----%x\n", last , bounds[i].bound);
          // printf(" in there\n");
          if (last < bounds[i].bound){
            if (field_i == FIELD_LEN-1) {
              // printf(" in there\n");
              ecs_arr[count] = xmalloc(sizeof(struct ec));
              ecs_arr[count]->ec_vs[field_i].l_b = last;
              ecs_arr[count]->ec_vs[field_i].h_b = bounds[i].bound; 
              count++;
            }
            else{
              if (count_tn_arr){
                struct trie_node_arr *tmp = xmalloc(sizeof(uint32_t)+count_tn_arr*sizeof(struct trie_node *));
                tmp->nnodes = count_tn_arr;
                for (int tn_i = 0; tn_i < count_tn_arr; tn_i++) {
                  tmp->nodes[tn_i] = e1ns[arr[tn_i]].node;
                  // printf("%d-%d-%d-%d\n", tmp->nodes[i], tmp->nodes[i]->branchs[0], tmp->nodes[i]->branchs[1], tmp->nodes[i]->branchs[2]);
                }

                struct ecs *ecsget = get_ecs_rec(tmp, r_ec, field_i+1);
                if(ecsget){
                  // printf("get_ecs_rec\n");
                  // printf("1\n");
                  for (int j = 0; j < ecsget->necs; j++) {
                    // for (int field = field_i+1; field < FIELD_LEN; field++){
                    //   // ecs[count]->ec_vs[field].h_b = ecsget->ecs[j]->ec_vs[field].h_b;
                    //   // ecs[count]->ec_vs[field].h_b = ecsget->ecs[j]->ec_vs[field].h_b;
                      
                    // }
                    ecs_arr[count] = ecsget->ecs[j];
                    ecs_arr[count]->ec_vs[field_i].l_b = last;
                    ecs_arr[count]->ec_vs[field_i].h_b = bounds[i].bound;
                    count++;
                  }
                  // printf("ecsget->necs%d\n", ecsget->necs);
                  free(ecsget);
                  // printf("free this\n");
                  ecsget = NULL;
                }
                // eqsign = false;
                // printf("%d\n", tmp);
                free(tmp);
                tmp = NULL;
              }
            }
            last = bounds[i].bound+1;
          }
          else if (last == bounds[i].bound) {
            eq_arr[count_eq_arr] = bounds[i].idx;
            // eqsign = true;
            count_eq_arr ++;
          }
        }
        else{
          if (last < bounds[i].bound){
            if (field_i == FIELD_LEN-1) {
              ecs_arr[count] = xmalloc(sizeof(struct ec));
              ecs_arr[count]->ec_vs[field_i].l_b = last;
              ecs_arr[count]->ec_vs[field_i].h_b = bounds[i].bound-1; 
              count++;
            }
            else{
              if(count_tn_arr){
                struct trie_node_arr *tmp = xmalloc(sizeof(uint32_t)+count_tn_arr*sizeof(struct trie_node *));
                tmp->nnodes = count_tn_arr;
                for (int tn_i = 0; tn_i < count_tn_arr; tn_i++) {
                  tmp->nodes[tn_i] = e1ns[arr[tn_i]].node;
                  // printf("%d-%d-%d-%d\n", tmp->nodes[i], tmp->nodes[i]->branchs[0], tmp->nodes[i]->branchs[1], tmp->nodes[i]->branchs[2]);
                }
                // printf("%d-%d-%d-%d\n", tmp->nodes[0], tmp->nodes[0]->branchs[0], tmp->nodes[0]->branchs[1], tmp->nodes[0]->branchs[2]);
                struct ecs *ecsget = get_ecs_rec(tmp, r_ec, field_i+1);
                if(ecsget){
                  // printf("get_ecs_rec\n");
                  // printf("1\n");
                  for (int j = 0; j < ecsget->necs; j++) {
                    // for (int field = field_i+1; field < FIELD_LEN; field++){
                    //   // ecs[count]->ec_vs[field].hb = ecsget->ecs[j]->ec_vs[field].hb;
                    //   // ecs[count]->ec_vs[field].hb = ecsget->ecs[j]->ec_vs[field].hb;
                    // }
                    ecs_arr[count] = ecsget->ecs[j];
                    ecs_arr[count]->ec_vs[field_i].l_b = last;
                    ecs_arr[count]->ec_vs[field_i].h_b = bounds[i].bound-1;
                    // printf("%x-%x\n", ecs_arr[count]->ec_vs[field_i].l_b, ecs_arr[count]->ec_vs[field_i].h_b);
                    count++;
                  }
                  // printf("%d\n", ecsget->necs);
                  free(ecsget);
                  // printf("free this\n");
                  ecsget = NULL;
                }
                // eqsign = false;
                // printf("%d\n", tmp);
                free(tmp);
                tmp = NULL;
              }
            }
            last = bounds[i].bound;
          }
        }
      }
      else if (last == bounds[i].bound) {
        if (in){
          eq_arr[count_eq_arr] = bounds[i].idx;
          count_eq_arr ++;
        }
      }

      if (!in) {
        arr[count_tn_arr] = bounds[i].idx;
        count_tn_arr++;
      }
      else{
        arr[delect] = arr[count_tn_arr-1];
        count_tn_arr--;
      }

      if(last >r_ec->ec_vs[field_i].h_b)
        break;
      else if((last == r_ec->ec_vs[field_i].h_b) && (i==2*count_e1ns-1)){
        // printf("there is in\n");
        if (field_i == FIELD_LEN-1) {
          ecs_arr[count] = xmalloc(sizeof(struct ec));
          ecs_arr[count]->ec_vs[field_i].l_b = last;
          ecs_arr[count]->ec_vs[field_i].h_b = last; 
          count++;
        }
        else{
          if(count_tn_arr+count_eq_arr){
            struct trie_node_arr *tmp = xmalloc(sizeof(uint32_t)+(count_tn_arr+count_eq_arr)*sizeof(struct trie_node *));
            tmp->nnodes = count_tn_arr+count_eq_arr;
            // printf("there is in %d\n", count_tn_arr+count_eq_arr);
            for (int tn_i = 0; tn_i < count_tn_arr; tn_i++) {
              tmp->nodes[tn_i] = e1ns[arr[tn_i]].node;
              // printf("%d-%d-%d-%d\n", tmp->nodes[tn_i], tmp->nodes[tn_i]->branchs[0], tmp->nodes[tn_i]->branchs[1], tmp->nodes[tn_i]->branchs[2]);
            }
            for (int tn_i = 0; tn_i < count_eq_arr; tn_i++) {
              tmp->nodes[tn_i+count_tn_arr] = e1ns[eq_arr[tn_i]].node;
              // printf("%d-%d-%d-%d\n", tmp->nodes[tn_i+count_tn_arr], tmp->nodes[tn_i+count_tn_arr]->branchs[0], tmp->nodes[tn_i+count_tn_arr]->branchs[1], tmp->nodes[tn_i+count_tn_arr]->branchs[2]);
            }

            struct ecs *ecsget = get_ecs_rec(tmp, r_ec, field_i+1);
            if(ecsget){
              // printf("get_ecs_rec\n");
              for (int j = 0; j < ecsget->necs; j++) {
                // for (int field = field_i+1; field < FIELD_LEN; field++){
                //   // ecs[count]->ec_vs[field].hb = ecsget->ecs[j]->ec_vs[field].hb;
                //   // ecs[count]->ec_vs[field].hb = ecsget->ecs[j]->ec_vs[field].hb;
                  
                // }
                ecs_arr[count] = ecsget->ecs[j];
                ecs_arr[count]->ec_vs[field_i].l_b = last;
                ecs_arr[count]->ec_vs[field_i].h_b = last;
                count++;
              }
              // printf("%d\n", ecsget->necs);
              free(ecsget);
              // printf("free this\n");
              ecsget = NULL;
            }
            free(tmp);
            // printf("%d\n", tmp);
            tmp = NULL;
          }
        }
        break;
      }
    }
  }

  struct ecs *ecs_tmp = NULL;
  // printf("has ecs %d\n", count);
  if (count) {
    ecs_tmp = xmalloc(sizeof(uint32_t)+count*sizeof(struct ec *));
    // printf("%d-%d\n", sizeof(struct ec *), count);
    // uint32_t *a = xmalloc(25*sizeof(uint32_t));
    // struct ec_1f *a = xmalloc(2*sizeof(uint32_t));
    ecs_tmp->necs = count;
    // if (a!=NULL)
    // {
    //   free(a);
    // }
    // a = NULL;
    for (int i = 0; i < count; i++) {
      // printf("%d_%d-%d\n",sizeof(ecs_tmp->ecs[i]),sizeof(ecs[i]));
      ecs_tmp->ecs[i] = ecs_arr[i];
      // printf("%x-%x\n", ecs_arr[i]->ec_vs[0].l_b, ecs_arr[i]->ec_vs[0].h_b);
    }
  }
  // printf("this is true1 %d\n", ecs_tmp);
  
  // printf("this is true2 \n");
  // ecs_tmp = NULL;
  return ecs_tmp;
}

void
free_ecs(struct ecs *a){
  if(a){
    for (int i = 0; i < a->necs; i++){
      if (a->ecs[i])
        free(a->ecs[i]);
    }
    free(a);
  }
}

uint32_t
trie_process_ec_arule_veriflow(struct trie_node *root, struct ex_rule *r){

  struct trie_node_arr *tn_arr = xmalloc(sizeof(uint32_t)+sizeof(struct trie_node *));
  tn_arr->nnodes = 1;
  tn_arr->nodes[0] = root;
  struct ec *ec_r = xmalloc(sizeof(*ec_r));


  for (int field_i = 0; field_i < FIELD_LEN; field_i++) {//standford data has 7fields
    uint32_t sign = 0;
    // uint16_t upmask = 0x8000;
    // uint16_t downmask = 0x0080;
    uint32_t len = field_sign[field_i+1] - field_sign[field_i];
    uint32_t h_v = 0;
    uint32_t l_v = 0;
    for (int len_i = 0; len_i < len; len_i++) {
      uint16_t masksign = 0;
      uint32_t mf_sign = (field_sign[field_i] + len_i)/2;
      if ((field_sign[field_i] + len_i)%2){
        masksign = 0x0080;
      }
      else{
        masksign = 0x8000;
      }
      for (int c_i = 0; c_i < 8; c_i++) {
        if (masksign & r->mf_in->mf_w[mf_sign]) {//wildcard 
          h_v |= 1;
        }
        else if (masksign & r->mf_in->mf_v[mf_sign]) {//1
          l_v |= 1;
          h_v |= 1;
        }
        // else {
        //   l_v <<= 1;
        //   h_v <<= 1;
        // }
        if ((len_i == (len-1))&&(c_i == 7))
          break;
        l_v <<= 1;
        h_v <<= 1;
        masksign >>=1;
      }
    }
    ec_r->ec_vs[field_i].l_b = l_v;
    // printf("%x\n", ec_r->ec_vs[field_i].l_b);
    ec_r->ec_vs[field_i].h_b = h_v;
    // printf(" rule %x-%x\n", ec_r->ec_vs[field_i].l_b, ec_r->ec_vs[field_i].h_b);

  }

  // printf("rule %d - %d\n", r);
  struct ecs *ecsget = get_ecs_rec(tn_arr, ec_r, 0);

  trie_process_ec_arule(root,r);
  uint32_t necs = 0;
  if (ecsget){
    necs =  ecsget->necs;
    // printf("the affected ecs num is %d\n", ecsget->necs);
  }
  // printf("ecsget\n" );
  free_ecs(ecsget);
  // printf("ecsget\n" );
  return necs;

}


uint32_t
trie_process_ec_arule_intrie(struct trie_node *root, struct ex_rule *r) {
  struct mf_uint16_t *mf = r->mf_in;
  uint32_t count = 1;
  struct trie_node *arr[data_allr_nums];
  arr[0] = root;

  struct timeval start,stop;
 

  // gettimeofday(&start,NULL);
  uint32_t node_tr_counter = 0;
  // printf("this is right\n");

  //find all the inscrules
  for (int i = 0; i < MF_LEN; i++) {
    uint16_t sign = 0x8000;
    for (int j = 0; j < 16; j++) {
      uint32_t count_tmp = 0;
      struct trie_node *tmp[data_allr_nums];
      if (sign & mf->mf_w[i]) {
        for (int k = 0; k < count; k++) {
          if (arr[k]->branchs[0]){
            tmp[count_tmp] = arr[k]->branchs[0];
            count_tmp++;
            // node_tr_counter++;

          }
          if (arr[k]->branchs[1]){
            tmp[count_tmp] = arr[k]->branchs[1];
            count_tmp++;
            // node_tr_counter++;
          }
          if (arr[k]->branchs[2]){
            tmp[count_tmp] = arr[k]->branchs[2];
            count_tmp++;
            // node_tr_counter++;
          }   
        }
      }
      else {
        if (sign & mf->mf_v[i]) {
          for (int k = 0; k < count; k++) {
            if (arr[k]->branchs[1]){
              tmp[count_tmp] = arr[k]->branchs[1];
              count_tmp++;
              // node_tr_counter++;
            }
            // if (arr[k]->branchs[2]){
            //   tmp[count_tmp] = arr[k]->branchs[2];
            //   count_tmp++;
            //   // node_tr_counter++;
            // }   
          }
        }
        else {
          for (int k = 0; k < count; k++) {
            if (arr[k]->branchs[0]){
              tmp[count_tmp] = arr[k]->branchs[0];
              count_tmp++;
              // node_tr_counter++;
            }
            // if (arr[k]->branchs[2]){
            //   tmp[count_tmp] = arr[k]->branchs[2];
            //   count_tmp++;
            //   // node_tr_counter++;
            // }   
          }
        }
      }
      sign >>= 1;
      for (int k = 0; k < count_tmp; k++) {
        arr[k] = tmp[k];
      }
      count = count_tmp;
    }  
  }

  // gettimeofday(&stop,NULL);
  // long long int T_for_add = diff(&stop, &start);
  // printf("Trie test rule with %d - %d: %lld us with searching\n", r->sw_idx, r->idx, T_for_add);
  // printf("node_tr_counter is %d\n", node_tr_counter);
  // gettimeofday(&start,NULL);

  struct ex_rule *r_arr[data_allr_nums];
  uint32_t insc_r_count = 0;
  for (int i = 0; i < count; i++){
    // for (int j = 0; j < arr[i]->local_rules->nrules; j++) {
    //   r_arr[insc_r_count] = arr[i]->local_rules->rules[j];
    //   insc_r_count++;
    // }
    r_arr[insc_r_count] = arr[i]->local_rules->rules[0];
    insc_r_count++;
  }

  for (int field_i = 0; field_i < FIELD_LEN; field_i++) {//standford data has 7fields
    uint32_t sign = 0;
    // uint16_t upmask = 0x8000;
    // uint16_t downmask = 0x0080;
    uint32_t len = field_sign[field_i+1] - field_sign[field_i+1];
    // sign = field_sign[field_i]/2;
    uint32_t iscross = 0;
    // if (field_sign[field_i]%2 != 0) {
    //   // sign = field_sign[field_i]/2;
    //   iscross = 1;
    // }
    // else{
    //   // sign = field_sign[field_i]/2;
    //   iscross = 0;
    // }
    // uint32_t l_v_arr[insc_r_count];
    // uint32_t h_v_arr[insc_r_count];
    uint32_t v_arr[2*insc_r_count];

    for (int r_i = 0; r_i < insc_r_count; r_i++) {//for every r in the insect set
      uint32_t h_v = 0;
      uint32_t l_v = 0;
      for (int len_i = 0; len_i < len; len_i++) {
        uint16_t masksign = 0;
        uint32_t mf_sign = (field_sign[field_i] + len_i)/2;
        if ((field_sign[field_i] + len_i)%2)
          masksign = 0x0080;
        else
          masksign = 0x8000;
        for (int c_i = 0; c_i < 8; c_i++) {
          if (masksign & r_arr[r_i]->mf_in->mf_w[mf_sign]) {//wildcard 
            h_v |= 1;
          }
          else if (masksign & r_arr[r_i]->mf_in->mf_v[mf_sign]) {//1
            l_v |= 1;
            h_v |= 1;
          }
          // else {
          //   l_v <<= 1;
          //   h_v <<= 1;
          // }
          if ((len_i == (len-1))&&(c_i == 7))
            break;
          l_v <<= 1;
          h_v <<= 1;
          masksign >>=1;
        }
      }
      v_arr[r_i*2] = l_v;
      v_arr[r_i*2+1] = h_v;
    }

    uint32_t h_end = 0;
    uint32_t l_last = 0;
    for (int len_i = 0; len_i < len; len_i++) {
      uint16_t masksign = 0;
      uint32_t mf_sign = (field_sign[field_i] + len_i)/2;
      if ((field_sign[field_i] + len_i)%2)
        masksign = 0x0080;
      else
        masksign = 0x8000;
      for (int c_i = 0; c_i < 8; c_i++) {
        if (masksign & r->mf_in->mf_w[mf_sign]) {//wildcard 
          h_end |= 1;
        }
        else if (masksign & r->mf_in->mf_v[mf_sign]) {//1
          l_last |= 1;
          h_end |= 1;
        }
        // else {
        //   l_last <<= 1;
        //   h_v <<= 1;
        // }
        if ((len_i == (len-1))&&(c_i == 7))
          break;
        l_last <<= 1;
        h_end <<= 1;
        masksign >>=1;
      }
    }


    qsort(v_arr, 2*insc_r_count,sizeof(uint32_t), uint32_t_cmp);

    uint32_t last = v_arr[0];

    for (int i = 0; i < 2*insc_r_count; i++) {

      if(v_arr[i] > last){
        struct ec_1f *tmp = xmalloc(sizeof *tmp);
        tmp->l_b = last;
        tmp->h_b = v_arr[i];
        free(tmp);
        last = v_arr[i];
      }
    }
  }
  // gettimeofday(&stop,NULL);
  // T_for_add = diff(&stop, &start);
  // printf("Trie test rule with %d - %d: %lld us with computing EC\n", r->sw_idx, r->idx, T_for_add);
  printf("This rule insect %d rules\n", insc_r_count);
    // arr[i]->relevent_rules = add_rule_to_ex_rules_arr(arr[i]->relevent_rules, r);

  return insc_r_count;
}

void
trie_add_rules_for_sw_test_difflast1(struct trie_node *root, struct switch_rs *sw, uint32_t idx) {
  struct timeval start,stop;
  uint32_t order[sw->nrules];
  for (int i = 0; i < sw->nrules; i++)
    order[i] = i;
  order[idx-1] = sw->nrules - 1;
  order[sw->nrules - 1] = idx - 1;

  for (int i = 0; i < sw->nrules-1; i++) {
    trie_insert_rule (root, sw->rules[order[i]]);
  }


  gettimeofday(&start,NULL);
  trie_insert_rule (root, sw->rules[order[sw->nrules-1]]);
  uint32_t insc_r_count = trie_process_ec_arule(root, sw->rules[order[sw->nrules-1]]);
  gettimeofday(&stop,NULL);
  long long int T_up_last1 = diff(&stop, &start);
  printf("Build trie test\n");
  printf("Test the last one with idx %d: %lld us\n", sw->rules[order[sw->nrules - 1]]->idx, T_up_last1);
  printf("This rule insect %d rules\n", insc_r_count);
}

void
trie_add_rules_for_sw_test_all_difflast1(struct trie_node *root, struct switch_rs *sw, uint32_t idx) {
  struct timeval start,stop;
  uint32_t order[sw->nrules];
  for (int i = 0; i < sw->nrules; i++)
    order[i] = i;
  order[idx-1] = sw->nrules - 1;
  order[sw->nrules - 1] = idx - 1;

  for (int i = 0; i < sw->nrules-1; i++) {
    gettimeofday(&start,NULL);
    trie_insert_rule (root, sw->rules[order[i]]);
    uint32_t insc_r_count = trie_process_ec_arule(root, sw->rules[order[i]]);
    gettimeofday(&stop,NULL);
    long long int T_for_add = diff(&stop, &start);
    printf("Test the rule idx %d: %lld us\n", i+1, T_for_add);
  }



  gettimeofday(&start,NULL);
  trie_insert_rule (root, sw->rules[order[sw->nrules-1]]);
  uint32_t insc_r_count = trie_process_ec_arule(root, sw->rules[order[sw->nrules-1]]);
  gettimeofday(&stop,NULL);
  long long int T_up_last1 = diff(&stop, &start);
  printf("Build trie test\n");
  printf("Test the last one with idx %d: %lld us\n", sw->rules[order[sw->nrules - 1]]->idx, T_up_last1);
  printf("This rule insect %d rules\n", insc_r_count);
}

void
trie_add_rules_for_sw_test_all(struct trie_node *root, struct switch_rs *sw) {
  struct timeval start,stop;
  for (int i = 0; i < sw->nrules; i++) {
    gettimeofday(&start,NULL);
    trie_insert_rule (root, sw->rules[i]);
    uint32_t insc_r_count = trie_process_ec_arule(root, sw->rules[i]);
    gettimeofday(&stop,NULL);
    long long int T_for_add = diff(&stop, &start);
    printf("Trie test rule with %d - %d: %lld us\n", sw->sw_idx, sw->rules[i]->idx, T_for_add);
    // printf("This rule insect %d rules\n", insc_r_count);
  }
  // printf("This rule insect %d rules\n", insc_r_count);
}

void
trie_add_rules_for_nt_test_all(struct trie_node *root, struct network_wc *nt) {
  printf("Build trie test!!!\n");
  struct timeval start,stop;
  uint32_t arr[SW_NUM] = {29, 94, 23, 40, 34, 49, 26, 28, 68};

  for (int i = 0; i < SW_NUM; i++) {
  // for (int i = 0; i < 1; i++) {
    // trie_add_rules_for_sw_test_all(root, nt->sws[i]);
    struct switch_rs *sw = nt->sws[i];
    // for (int j = 0; j < sw->nrules; j++) {
      // if((j>600))
      //   break;
    // for (int j = 0; j < 5; j++) {
    for (int j = arr[i]; j < sw->nrules; j++) {
    // for (int k = arr[i]; k < sw->nrules; k++) {
    //   uint32_t j = sw->nrules - k + arr[i]-1;
      gettimeofday(&start,NULL);
      trie_insert_rule (root, sw->rules[j]);
      // gettimeofday(&stop,NULL);
      // uint32_t insc_r_count = trie_process_ec_arule(root, sw->rules[j]);
      // uint32_t insc_r_count = trie_process_ec_arule_intrie(root, sw->rules[j]);
      uint32_t insc_r_count = trie_process_ec_arule_veriflow(root, sw->rules[j]);
      gettimeofday(&stop,NULL);
      long long int T_for_add = diff(&stop, &start);
      printf("Trie test rule with %d - %d: %lld us with inserting\n", sw->sw_idx, sw->rules[j]->idx, T_for_add);
      printf("This rule insect %d rules\n", insc_r_count);
    }
  }
    
    // trie_add_rules_for_sw_test_all_difflast1(root, nt->sws[i],9);
}



struct trie_node *
get_terminal_by_r(struct trie_node *root, struct ex_rule *r) {
  struct trie_node *tmp = root;
  struct mf_uint16_t *mf = r->mf_in;
  for (int i = 0; i < MF_LEN; i++){
    uint16_t sign = 0x8000;
    for (int j = 0; j < 16; j++){
      if (sign & mf->mf_w[i]) {
        if (!(tmp->branchs[2])) //*
          return NULL;
        tmp = tmp->branchs[2];        
      }
      else {
        if (sign & mf->mf_v[i]) {
          if (!(tmp->branchs[1])) //1
            return NULL;
          tmp = tmp->branchs[1];        
        }
        else {
          if (!(tmp->branchs[0])) //0
            return NULL;
          tmp = tmp->branchs[0];
        }
      }
      sign >>= 1;
    }  
  }
  if (!(tmp->isleaf))
    return NULL;
  return tmp;
}

/*MTBDD结构*/
/*========================================================================*/

struct network_bdd {
  struct switch_bdd_rs *sws[SW_NUM];
};

BDD
ex_rule_to_MTBDD(struct ex_rule *r) {
  BDD root, tmp;
  struct rule_records_arr *rr = xmalloc(sizeof *rr);
  rr->nrules = 1;
  rr->rules[0].sw_idx = r->sw_idx;
  rr->rules[0].idx = r->idx;
  tmp = mtbdd_maketnode_1r(rr);  
  root = tmp;
  for (int i = 0; i < MF_LEN; i++){
    int reverse_i = MF_LEN - i - 1;
    uint16_t sign = 0x0001;
    for (int j = 0; j < 16; j++){
      if (!(sign & r->mf_in->mf_w[reverse_i])){
        int level = bdd_var2level(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
        if (sign & r->mf_in->mf_v[reverse_i]){
          root = bdd_makenode(level, 0, tmp);
        }
        else{
          root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点
        }
        tmp = root;
      }
      sign <<= 1;
    }
  }
  return root;
}

BDD
bdd_rule_get_mtbdd_in(struct ex_rule *r, BDD tnode) {
  BDD root, tmp;
  tmp = tnode;
  root = tmp;
  for (int i = 0; i < MF_LEN; i++){
    int reverse_i = MF_LEN - i - 1;
    uint16_t sign = 0x0001;
    for (int j = 0; j < 16; j++){
      if (!(sign & r->mf_in->mf_w[reverse_i])){
        int level = bdd_var2level(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
        if (sign & r->mf_in->mf_v[reverse_i]){
          root = bdd_makenode(level, 0, tmp);
        }
        else{
          root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点
        }
        tmp = root;
      }
      sign <<= 1;
    }
  }
  return root;
}

bool
is_action_same(struct bdd_rule *a, struct bdd_rule *b){
  if (!is_mask_uint16_t_same(a->mask, b->mask))
    return false;
  if (!is_mask_uint16_t_same(a->rewrite, b->rewrite))
    return false;
  if (!is_links_of_rule_same(a->lks_out, b->lks_out))
    return false;
  if (!is_links_of_rule_same(a->lks_in, b->lks_in))
    return false;
  return true;
}

bool
is_inport_same_rwc(struct bdd_rule *a, struct bdd_rule *b){
  if (!is_links_of_rule_same(a->lks_in, b->lks_in))
    return false;
  return true;
}

bool
is_action_same_rwc(struct bdd_rule *a, struct bdd_rule *b){
  if (!is_mask_uint16_t_same(a->mask, b->mask))
    return false;
  if (!is_mask_uint16_t_same(a->rewrite, b->rewrite))
    return false;
  if (!is_links_of_rule_same(a->lks_out, b->lks_out))
    return false;
  if (!is_links_of_rule_same(a->lks_in, b->lks_in))
    return false;
  return true;
}

struct bdd_rule *
ex_rule_to_bdd_rule(struct ex_rule *r) {
  struct bdd_rule *bdd_r = xmalloc(sizeof *bdd_r);
  bdd_r->sw_idx = r->sw_idx;  
  bdd_r->idx = r->idx;
  bdd_r->mask = r->mask;
  bdd_r->rewrite = r->rewrite;
  bdd_r->lks_in = r->lks_in;
  bdd_r->lks_out = r->lks_out;
  bdd_r->mf_in = mf2bdd(r->mf_in);
  bdd_addref(bdd_r->mf_in);
  bdd_r->mf_out = mf2bdd(r->mf_out);
  bdd_addref(bdd_r->mf_out);
  bdd_r->vtnode_in = mtbdd_maketnode_from_r(bdd_r);
  bdd_addref(bdd_r->vtnode_in);
  bdd_r->mtbdd_in = bdd_rule_get_mtbdd_in(r, bdd_r->vtnode_in); 
  bdd_addref(bdd_r->mtbdd_in);
  return bdd_r;
}

struct bdd_rule *
ex_rule_to_bdd_rule_noredun(struct ex_rule *r, struct switch_bdd_rs *sw, uint32_t count) {
  bool isame = false;
  uint32_t rsame = 0;
  
  struct bdd_rule *bdd_r = xmalloc(sizeof *bdd_r);
  bdd_r->sw_idx = r->sw_idx;  
  bdd_r->idx = r->idx;
  bdd_r->mask = r->mask;
  bdd_r->rewrite = r->rewrite;
  bdd_r->lks_in = r->lks_in;
  bdd_r->lks_out = r->lks_out;
  bdd_r->mf_in = mf2bdd(r->mf_in);
  bdd_addref(bdd_r->mf_in);
  bdd_r->mf_out = mf2bdd(r->mf_out);
  bdd_addref(bdd_r->mf_out);

  for (int i = 0; i < count; i++) {
    if (is_action_same_rwc(bdd_r, sw->rules[i])) {
      isame = true;
      rsame = i;
      break;
    }
  }
  if (isame){
    bdd_r->vtnode_in = sw->rules[rsame]->vtnode_in;
  }
  else
    bdd_r->vtnode_in = mtbdd_maketnode_from_r(bdd_r);

  for (int i = 0; i < count; i++){
    if (is_inport_same_rwc(bdd_r, sw->rules[i]))
      bdd_r->mf_in = bdd_apply(bdd_r->mf_in, sw->rules[i]->mf_in, bddop_diff);
  }

  bdd_addref(bdd_r->vtnode_in);
  bdd_r->mtbdd_in = bdd_rule_get_mtbdd_in(r, bdd_r->vtnode_in); 
  bdd_addref(bdd_r->mtbdd_in);
  // printf("the root has %d nodes,idx: %d - %d - %d -%d\n", bdd_nodecount(bdd_r->mtbdd_in), bdd_r->idx, bdd_r->vtnode_in, LOW(bdd_r->vtnode_in), HIGH(bdd_r->vtnode_in));
  return bdd_r;
}

struct switch_bdd_rs *
switch_rs_to_bdd_rs(struct switch_rs *sw){
  struct switch_bdd_rs *tmp = xmalloc(2*sizeof(uint32_t)+(sw->nrules)*sizeof(struct bdd_rule *));
  tmp->sw_idx = sw->sw_idx;
  tmp->nrules = sw->nrules;
  for (int i = 0; i < sw->nrules; i++) {
  // for (int i = 0; i < 3; i++) {
    tmp->rules[i] = ex_rule_to_bdd_rule(sw->rules[i]);
    // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  }
  printf("switch_bdd_rs %d has %d rules.\n", sw->sw_idx, sw->nrules); 
  printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  return tmp;
}

struct switch_bdd_rs *
switch_rs_to_bdd_rs_noredun(struct switch_rs *sw){
  struct switch_bdd_rs *tmp = xmalloc(2*sizeof(uint32_t)+(sw->nrules)*sizeof(struct bdd_rule *));
  tmp->sw_idx = sw->sw_idx;
  tmp->nrules = sw->nrules;
  for (int i = 0; i < sw->nrules; i++) {
  // for (int i = 0; i < 3; i++) {
    tmp->rules[i] = ex_rule_to_bdd_rule_noredun(sw->rules[i], tmp, i);
    // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  }
  printf("switch_bdd_rs %d has %d rules.\n", sw->sw_idx, sw->nrules); 
  printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  return tmp;
}

struct switch_bdd_rs *
switch_rs_to_bdd_rs_merged(struct switch_rs *sw){
  struct bdd_rule *arrtmp[20000];
  uint32_t count = 0;

  // struct switch_bdd_rs *tmp = xmalloc(2*sizeof(uint32_t)+(sw->nrules)*sizeof(struct bdd_rule *));
  // tmp->sw_idx = sw->sw_idx;
  // tmp->nrules = sw->nrules;
  for (int i = 0; i < sw->nrules; i++) {
  // for (int i = 0; i < 3; i++) {
    bool isame = false;
    struct bdd_rule *rtmp = ex_rule_to_bdd_rule(sw->rules[i]);


    for (int j = 0; j < count; j++){
      if (is_inport_same_rwc(rtmp, arrtmp[j]))
        rtmp->mf_in = bdd_apply(rtmp->mf_in, arrtmp[j]->mf_in, bddop_diff);    
    }
    for (int j = 0; j < count; j++) {
      if (is_action_same_rwc(rtmp, arrtmp[j])){
        arrtmp[j]->mf_in = bdd_apply(arrtmp[j]->mf_in, rtmp->mf_in, bddop_or);
        isame = true;
        break;
      }   
    }
    if (!isame) {
      arrtmp[count] = rtmp;
      count ++;
    }

    // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  }
  struct switch_bdd_rs *tmp = xmalloc(2*sizeof(uint32_t)+(count)*sizeof(struct bdd_rule *));
  tmp->sw_idx = sw->sw_idx;
  tmp->nrules = count;
  for (int i = 0; i < count; i++)
    tmp->rules[i] = arrtmp[i];
  printf("switch_bdd_rs %d has %d rules.\n", tmp->sw_idx, tmp->nrules); 
  printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  return tmp;
}

struct network_bdd *
network_wc_to_bdd(struct network_wc *nt){
  struct network_bdd *tmp = xmalloc(sizeof(*tmp));
  for (int i = 0; i < SW_NUM; i++) {
    tmp->sws[i] = switch_rs_to_bdd_rs(nt->sws[i]);
  }
  return tmp;
}

struct network_bdd *
network_wc_to_bdd_noredun(struct network_wc *nt){
  struct network_bdd *tmp = xmalloc(sizeof(*tmp));
  for (int i = 0; i < SW_NUM; i++) {
    tmp->sws[i] = switch_rs_to_bdd_rs_noredun(nt->sws[i]);
  }
  return tmp;
}

struct network_bdd *
network_wc_to_bdd_merged(struct network_wc *nt){
  struct network_bdd *tmp = xmalloc(sizeof(*tmp));
  for (int i = 0; i < SW_NUM; i++) {
    tmp->sws[i] = switch_rs_to_bdd_rs_merged(nt->sws[i]);
  }
  return tmp;
}





void
switch_bddrs_getinscbdd_test_all(struct network_bdd *nt) {
  struct timeval start,stop;
  uint32_t arr[SW_NUM] = {29, 94, 23, 40, 34, 49, 26, 28, 68};

  BDD tmp = 0;
  for (int k = 0; k < SW_NUM; k++) {
    struct switch_bdd_rs *sw = nt->sws[k];
  
    for (int i = arr[k]; i < sw->nrules; i++) {
    // for (int i = 0; i < sw->nrules; i++) {
      gettimeofday(&start,NULL);
      BDD self_final = sw->rules[i]->mf_in;


      for (int j = arr[k]; j < i; j++) {
      // for (int j = 0; j < i; j++) {
        self_final = bdd_apply(self_final, sw->rules[j]->mf_in, bddop_diff);
        if(!self_final)
          break;
      }
      gettimeofday(&stop,NULL);
      long long int T_for_add = diff(&stop, &start);   
      printf("get diff the rule idx %d: %lld us\n", i+1, T_for_add);
    }
  }
}

void
get_APs(struct network_bdd *nt){
  BDD arr1[500000];
  arr1[0] = 1;
  uint32_t count1 = 1;
  BDD arr2[500000];
  uint32_t count2 = 0;


  for (int k = 0; k < SW_NUM; k++) {
    printf("the sw %d has been completed\n", k);
    struct switch_bdd_rs *sw = nt->sws[k];
    for (int i = 0; i < sw->nrules; i++) {
      BDD in = sw->rules[i]->mf_in;
      BDD notin = bdd_not(in);
      for (int arr1_i = 0; arr1_i < count1; arr1_i++) {
        BDD insc = bdd_apply (arr1[arr1_i], in, bddop_and);
        if (insc)
        {
          arr2[count2] = insc;
          count2 ++;
        }
        
        insc= bdd_apply (arr1[arr1_i], notin, bddop_and);
        if (insc)
        {
          arr2[count2] = insc;
          count2 ++;
        }
      }
      count1 = count2;
      for (int arr2_i = 0; arr2_i < count2; arr2_i++) {
        arr1[arr2_i] = arr2[arr2_i];
      }
      count2 = 0;
    }
  }
  printf("there has been the %d aps\n", count1);
}

void
switch_bddrs_getinscbdd_test_diff1(struct switch_bdd_rs *sw, uint32_t idx) {
  BDD self_final = sw->rules[idx-1]->mf_in;
  BDD tmp = 0;


  uint32_t insc_counter = 0;
  for (int i = 0; i < idx-1; i++) {
    // BDD insc = bdd_apply(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in, bddop_and);
    // if (bdd_apply(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in, bddop_and)){
    //   // insc_counter++;
    // }
    // bdd_delref(self_final);
    self_final = bdd_apply(self_final, sw->rules[i]->mf_in, bddop_diff);
    // bdd_addref(self_final);
    // // check_bddfalse(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in);
    // if (sw->rules[idx-1]->mf_in == 0){
    //   printf("this rule is been coverd\n");
    //   break;
    // }
    // bdd_gbc();
    // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  }
  for (int i = idx; i < sw->nrules; i++){
    tmp = bdd_apply(self_final, sw->rules[i]->mf_in, bddop_diff);
    if (tmp) {
      tmp = 0;
    }
    tmp = 0;

    // BDD insc = bdd_apply(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in, bddop_and);
    // if (bdd_apply(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in, bddop_and)){
      // insc_counter++;
      // printf("the insec idx %d\n", i);
      // printf("the last rule has %d nodes\n", bdd_nodecount(insc));
    // }
  }
  // printf("This rule insect %d rules\n", insc_counter);
  // if (sw->rules[idx-1]->mf_in != 0){
  //   for (int i = idx; i < sw->nrules; i++) {
  //     // check_bddfalse(sw->rules[idx-1]->mf_in, sw->rules[i]->mf_in);
  //     BDD insc = bdd_apply(self_final, sw->rules[i]->mf_in, bddop_and);
  //     BDD diff = bdd_apply(sw->rules[i]->mf_in, insc, bddop_diff);
  //   }
  // }
}

BDD
switch_bddrs_to_mtbdd_test_difflast1(struct switch_bdd_rs *sw, uint32_t idx) {
  struct timeval start,stop;
  BDD root = 0;

  uint32_t order[sw->nrules];
  // root = mtbdd_add_r(root, sw->rules[sw->nrules - 1]->mtbdd_in, 0);
  for (int i = 0; i < sw->nrules; i++)
    order[i] = i;
  order[idx-1] = sw->nrules - 1;
  order[sw->nrules - 1] = idx-1;

  for (int i = 0; i < sw->nrules-1; i++) {

    gettimeofday(&start,NULL);
    bdd_delref(root);
    root = mtbdd_add_r(root, sw->rules[order[i]]->mtbdd_in, 0);
    bdd_addref(root);
    gettimeofday(&stop,NULL);
    long long int T_for_add = diff(&stop, &start);
    // if (i > 0 && i < 50) {
    printf("Test the rule idx %d: %lld us\n", i+1, T_for_add);
    // }
    // printf("the root has %d nodes\n", bdd_nodecount(root));
    // bdd_gbc();
    // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  }
  // printf("the root has %d nodes\n", bdd_nodecount(root));
  gettimeofday(&start,NULL);
  bdd_delref(root);
  root = mtbdd_add_r(root, sw->rules[order[sw->nrules - 1]]->mtbdd_in, 0);
  bdd_addref(root);
  gettimeofday(&stop,NULL);
  long long int T_up_last1 = diff(&stop, &start);
  // printf("the last rule has %d nodes\n", bdd_nodecount(sw->rules[order[sw->nrules - 1]]->mtbdd_in));
  // printf("the root has %d nodes\n", bdd_nodecount(root));
  printf("Test the rule idx %d: %lld us\n", sw->rules[order[sw->nrules - 1]]->idx, T_up_last1);
  // printf("This rule insect %d rules\n", mtbddvalues[(bddnodes[sw->rules[order[sw->nrules - 1]]->vtnode_in].high)].test_count);
  // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  return root;
}

BDD
nt_bdd_to_mtbdd_test_all(struct network_bdd *nt) {
  struct timeval start,stop;
  BDD root = 0;
  uint32_t arr[SW_NUM] = {29, 94, 23, 40, 34, 49, 26, 28, 68};

  // for (int i = 0; i < SW_NUM; i++) {
  for (int i = 0; i < SW_NUM; i++) {
    struct switch_bdd_rs *sw = nt->sws[i];
    for (int j = 0; j < sw->nrules; j++) {
    // for (int j = 0; j < 9; j++) {
    // for (int j = 0; j < arr[i]+1; j++) {
    // for (int j = arr[i]; j < sw->nrules; j++) {
    // for (int k = arr[i]; k < sw->nrules; k++) {
    //   uint32_t j = sw->nrules - k + arr[i]-1;
      // if (j == 89)
      //   continue;
      gettimeofday(&start,NULL);
      bdd_delref(root);
      root = mtbdd_add_r(root, sw->rules[j]->mtbdd_in, 0);
      bdd_addref(root);
      gettimeofday(&stop,NULL);
      long long int T_for_add = diff(&stop, &start);
      printf("MTBDD test rule with %d - %d: %lld us\n", sw->rules[j]->sw_idx, sw->rules[j]->idx,T_for_add);

      // printf("the root has %d nodes\n", bdd_nodecount(root));
      // bdd_gbc();
      // printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
    }

    // for (int j = arr[i]; j < 100; j++) {

    //   gettimeofday(&start,NULL);
    //   bdd_delref(root);
    //   root = mtbdd_add_r(root, sw->rules[j]->mtbdd_in, 0);
    //   bdd_addref(root);
    //   gettimeofday(&stop,NULL);
    //   long long int T_for_add = diff(&stop, &start);
    //   printf("MTBDD test rule with %d - %d: %lld us\n", sw->rules[j]->sw_idx, sw->rules[j]->idx,T_for_add);
    // }
    // gettimeofday(&start,NULL);
    // bdd_delref(root);
    // root = mtbdd_add_r(root, sw->rules[89]->mtbdd_in, 0);
    // bdd_addref(root);
    // gettimeofday(&stop,NULL);
    // long long int T_for_add = diff(&stop, &start);
    // printf("MTBDD test rule with %d - %d: %lld us\n", sw->rules[89]->sw_idx, sw->rules[89]->idx,T_for_add);
  }
  printf("the root has %d nodes\n", bdd_nodecount(root));

  printf("bdd_getnodenum :%d - %d\n", bdd_getnodenum(),mtbdd_getvaluenum()); 
  return root;
}
//end