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
#include "usedBDD.h"  
#include <sys/time.h>
#include <malloc.h>

//结构体或变量定义
//自定义
#define STANDFORD_W 1
#define MF_LEN 8 //128位bit， 8×16
// #define MF_LEN 2 //32位bit standford
// #define MF_LEN 3 //48位bit i2
// #define SW_NUM 9
#define SW_NUM 16

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

//自己定义
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
#define BDDSIZE     10000000
#define BDDOPCHCHE  400000 

struct bdd_rule {
  uint32_t sw_idx;
  uint32_t idx;
  BDD mf_in;
  BDD mf_out;
  // BDD vtnode_in;
  // BDD mtbdd_in;
  struct mask_uint16_t *mask;
  struct mask_uint16_t *rewrite; 
  struct links_of_rule *lks_in;
  struct links_of_rule *lks_out;
  // struct port_arr *port_in;
  // struct port_arr *port_out;
};

struct switch_bdd_rs {
  uint32_t sw_idx;
  uint32_t nrules;
  struct bdd_rule *rules[0];
};

struct r_to_merge {
  uint32_t nrules;
  uint32_t rules[0];
};

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

// struct PACKED nf_s_pair {
//   struct nf_space nf_s_in;
//   struct nf_space nf_s_out; 
// };

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
  uint32_t idx;
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




//BDD sw 全局数组
struct switch_bdd_rs *bdd_sws_arr[SW_NUM];
struct r_to_merge *r_to_merge_arr[SW_NUM]; //原来的r到新合并的r之间的映射
struct r_to_merge *merged_arr[SW_NUM];
uint32_t r_merge_num_arr[SW_NUM];


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
struct bdd_rule *matrix_idx_to_bddr(const uint32_t *matrix_idx);

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
bool issame_nf_space_pair_action(struct nf_space_pair *ns1, struct nf_space_pair *ns2);
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
struct matrix_element *copy_matrix_element(struct matrix_element *a);

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
BDD bdd_rw_BDD(BDD a, struct mask_uint16_t *mask, struct mask_uint16_t *rw);
BDD bdd_rw_back_BDD(BDD a, BDD a_IN, struct mask_uint16_t *mask);

/*普通矩阵处理及其计算*/
/*========================================================================*/

struct nf_space_pair *nf_space_connect(struct nf_space_pair *a, struct nf_space_pair *b);

/*稀疏矩阵处理及其计算*/
/*========================================================================*/
struct matrix_Tri_express *insc_to_Tri_express_rlimit(struct bdd_rule *r_in, struct bdd_rule *r_out, BDD v_and);
struct matrix_element *matrix_elem_plus(struct matrix_element *a, struct matrix_element *b);
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
    if (rule->idx != 1){
      if (!(rule->in_link))
        return NULL;
    }
    else if (STANDFORD_W){
      if (rule->sw_idx == 1) 
        return NULL;
    }
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
  if(!rs)
    printf("this r_idxs is NULL\n");
  for (int i = 0; i < rs->nrs; i++) {
    printf("%d - %d, ", rs->ridx[i].sw_idx, rs->ridx[i].r_idx);
  }
  printf("\n");
}

void
print_nf_space(const struct nf_space *a) {
  printf("matchfield:");
  printf("%d", a->mf);
  // print_mf_uint16_t(a->mf);
  printf("the links:");
  // print_links_of_rule(a->lks); 
  printf("%d\n", a->lks->n);
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

struct bdd_rule *
matrix_idx_to_bddr(const uint32_t *matrix_idx){
  // uint32_t swn = data_file->sws_num;
  uint32_t swn = SW_NUM;
  // if (!(*matrix_idx))
  //   return rule_get_2idx(0, 1);
  for (int i = 0; i < swn; i++) {
    if (*matrix_idx < *(uint32_t *)(sws_r_num+i))
      return bdd_sws_arr[i-1]->rules[*matrix_idx - *(uint32_t *)(sws_r_num+i-1)];
  }
  if (*matrix_idx < data_allr_nums) {
    return bdd_sws_arr[swn-1]->rules[*matrix_idx - *(uint32_t *)(sws_r_num+swn-1)];
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
  if (!mk)
    return NULL;
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

/*生成 BDD 描述的 rules*/
/*------------------------------------------------*/
struct bdd_rule *
gen_bdd_rule_from_of(struct of_rule *of_r) {
  if (!of_r)
    return NULL;
  int add_len = sizeof(uint16_t);
  struct bdd_rule *tmp = xcalloc(1, sizeof *tmp);
  tmp->sw_idx = of_r->sw_idx;
  tmp->idx = of_r->idx;
  struct links_of_rule *lks_out = rule_links_get(of_r, OUT_LINK);
  struct links_of_rule *lks_in = rule_links_get(of_r, IN_LINK);
  tmp->lks_in = copy_links_of_rule(lks_in);
  tmp->lks_out = copy_links_of_rule(lks_out);
  struct mf_uint16_t *mf_in = get_r_out_mf(of_r);
  struct mf_uint16_t *mf_out = get_r_in_mf(of_r);
  tmp->mf_in = mf2bdd(mf_in);
  bdd_addref(tmp->mf_in);
  tmp->mf_out = mf2bdd(mf_out);
  bdd_addref(tmp->mf_out);
  // tmp->mf_in = get_r_out_mf(of_r);
  // tmp->mf_out = get_r_in_mf(of_r);
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
  free(mf_in);
  free(mf_out);
  return tmp;
}

struct switch_bdd_rs *
gen_sw_rules(uint32_t sw_idx) {
  struct sw *sw = sw_get(sw_idx);
  if(!sw)
    return NULL;
  struct switch_bdd_rs *tmp = xmalloc(2*sizeof(uint32_t)+(sw->nrules)*sizeof(struct bdd_rule *));
  tmp->sw_idx = sw->sw_idx;
  tmp->nrules = sw->nrules;
  // struct of_rule *of_r = rule_get(sw, 1);
  // tmp->rules[0] = gen_bdd_rule_from_of(of_r);
  for (int i = 0; i < tmp->nrules; i++) {
    struct of_rule *of_r = rule_get(sw, i+1);
    tmp->rules[i] = gen_bdd_rule_from_of(of_r);
    for (int j = 0; j < tmp->rules[i]->idx - 1; j++){
      if (is_links_of_rule_same(tmp->rules[i]->lks_in, tmp->rules[j]->lks_in)){
        BDD bddtmp = tmp->rules[i]->mf_in;
        tmp->rules[i]->mf_in = bdd_apply(tmp->rules[i]->mf_in, tmp->rules[j]->mf_in, bddop_diff);
        if (tmp->rules[i]->mf_in != bddtmp) {
          bdd_delref(bddtmp);
          bdd_addref(tmp->rules[i]->mf_in);
          if (tmp->rules[i]->mask){
            bdd_delref(tmp->rules[i]->mf_out);
            tmp->rules[i]->mf_out = bdd_rw_BDD(tmp->rules[i]->mf_in, tmp->rules[i]->mask, tmp->rules[i]->rewrite);
            bdd_addref(tmp->rules[i]->mf_out);
          }
          else{
            bdd_delref(tmp->rules[i]->mf_out);
            tmp->rules[i]->mf_out = tmp->rules[i]->mf_in;
            bdd_addref(tmp->rules[i]->mf_out);
          }
        }
      }
    }
  }
  return tmp;
}

void
free_bdd_rule(struct bdd_rule *r) {
  if(r) {
    if(r->mask) {
      free(r->mask);
      free(r->rewrite);
    }
    if(r->lks_in)
      free(r->lks_in);
    if(r->lks_out)
      free(r->lks_out);
    free(r);
  }
}

void
bdd_sw_load(void) {
  for (int i = 0; i < SW_NUM; i++) {
    bdd_sws_arr[i] = gen_sw_rules(i);
  }
}

void
free_switch_bdd_rs(struct switch_bdd_rs *sw) {
  if(sw) {
    for (int i = 0; i < sw->nrules; i++)
      free_bdd_rule(sw->rules[i]);
    free(sw);
  }
}

void
bdd_sw_unload(void) {
  for (int i = 0; i < SW_NUM; i++) {
    free_switch_bdd_rs(bdd_sws_arr[i]);
  }
}

/*生成合并的规则*/
/*------------------------------------------------*/

// struct r_to_merge {
//   uint32_t nrules;
//   uint32_t rules[0];
// };
// struct r_to_merge *r_to_merge_arr[SW_NUM];
// r_merge_num_arr
bool
is_r_action_same(struct bdd_rule *a, struct bdd_rule *b){
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

uint32_t same_num;
void
init_r_to_merge(void) {
  same_num = 0;
  for (int i = 0; i < SW_NUM; i++) {
    r_to_merge_arr[i] = xmalloc((bdd_sws_arr[i]->nrules+1)*sizeof(uint32_t));
    r_to_merge_arr[i]->nrules = bdd_sws_arr[i]->nrules;

    uint32_t arr_tmp[bdd_sws_arr[i]->nrules];
    arr_tmp[0] = 0;
    uint32_t count = 1;

    
    for (int j = 1; j < bdd_sws_arr[i]->nrules; j++) {
      bool issame = false;
      for (int k = 0; k < count; k++) {
        
        if (is_r_action_same(bdd_sws_arr[i]->rules[arr_tmp[k]], bdd_sws_arr[i]->rules[j])) {
          // printf("the same rules %d - %d and %d - %d\n", bdd_sws_arr[i]->rules[arr_tmp[k]]->sw_idx, bdd_sws_arr[i]->rules[arr_tmp[k]]->idx, bdd_sws_arr[i]->rules[j]->sw_idx, bdd_sws_arr[i]->rules[j]->idx);
          r_to_merge_arr[i]->rules[j] = k;
          issame = true;
          same_num ++;
          break;
        }
      }
      if(!issame) {
        arr_tmp[count] = j;
        r_to_merge_arr[i]->rules[j] = count;
        count++;
      }
    }
    merged_arr[i] = xmalloc((count+1)*sizeof(uint32_t));
    merged_arr[i]->nrules = count;
    for (int j = 0; j < count; j++)
      merged_arr[i]->rules[j] = arr_tmp[j];
  }
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
  if((!a ) || (!b))
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
insc_to_Tri_express_rlimit(struct bdd_rule *r_in, struct bdd_rule *r_out, BDD v_and) {

  struct nf_space_pair *pair = xcalloc(1, sizeof *pair);
  // struct links_of_rule *lks = rule_links_get(r_in, IN_LINK);
  // struct links_of_rule *lks_out = rule_links_get(r_in, OUT_LINK);
  // struct links_of_rule *lks_in = rule_links_get(r_out, IN_LINK);
  // lks_out = links_insc(lks_in, lks_out);
  // int add_len = sizeof(uint16_t);
  struct matrix_Tri_express *tmp = xcalloc(1, sizeof *tmp);
  tmp->elem = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+sizeof(struct nf_space_pair *));
  tmp->elem->npairs = 1;
  // tmp->row_idx = matrix_idx_get_r(r_in);
  // tmp->col_idx = matrix_idx_get_r(r_out);
  tmp->row_idx = matrix_idx_get_2idx(r_in->sw_idx, r_in->idx);
  tmp->col_idx = matrix_idx_get_2idx(r_out->sw_idx, r_out->idx);
  // struct bdd_saved_arr *bdd_arr_out = bdd_save_arr(v_and);

  pair->in = xcalloc(1, sizeof *(pair->in));// 1,16(两个指针为16)
  pair->in->lks = copy_links_of_rule(r_in->lks_in);
  pair->out = xcalloc(1, sizeof *(pair->out));
  pair->out->lks = copy_links_of_rule(r_in->lks_out);

  pair->r_arr = NULL;
  // pair->r_arr = xmalloc(sizeof (uint32_t)+2*sizeof (struct r_idx));
  // pair->r_arr->nrs = 2;
  // pair->r_arr->ridx[0].sw_idx = r_in->sw_idx;
  // pair->r_arr->ridx[0].r_idx = r_in->idx;
  // pair->r_arr->ridx[1].sw_idx = r_out->sw_idx;
  // pair->r_arr->ridx[1].r_idx = r_out->idx;

  pair->out->mf = v_and;
  bdd_addref(pair->out->mf);
  tmp->elem->bdd_out = pair->out->mf;
  bdd_addref(pair->out->mf);

  if (r_in->mask) {
    struct mask_uint16_t *mask = xcalloc(1, sizeof *mask);
    struct mask_uint16_t *rewrite = xcalloc(1, sizeof *rewrite);
    for (uint32_t j = 0; j < MF_LEN; j++) {
      mask->v[j] = r_in->mask->v[j];
      rewrite->v[j] = r_in->rewrite->v[j];
    }
    // struct mf_uint16_t *r_in_mf = get_r_out_mf(r_in);
    BDD bdd_in = r_in->mf_in;
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
  struct nf_space_pair *nps[100000];
  uint32_t count = 0;
  for (int i = 0; i < a->npairs; i++) {
    nps[count] = a->nf_pairs[i]; 
    count++;
  }

  for (int i = 0; i < b->npairs; i++) {
    bool issame = false;
    for (int j = 0; j < count; j++) {
      if (issame_nf_space_pair_action(nps[j], b->nf_pairs[i])) {
        bdd_delref(nps[j]->in->mf);
        bdd_delref(nps[j]->out->mf);
        nps[j]->in->mf = bdd_apply(nps[j]->in->mf, b->nf_pairs[i]->in->mf, bddop_or);
        nps[j]->out->mf = bdd_apply(nps[j]->out->mf, b->nf_pairs[i]->out->mf, bddop_or);
        bdd_addref(nps[j]->in->mf);
        bdd_addref(nps[j]->out->mf);
        issame = true;
        free_nf_space_pair(b->nf_pairs[i]);
        break;
      } 
    }
    if (!issame) {
      nps[count] = b->nf_pairs[i];
      count++;
    }
  }

  struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+count*sizeof(struct nf_space_pair *));
  tmp->npairs = count;
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
  for (int i = 0; i < count; i++) 
    tmp->nf_pairs[i] = nps[i]; 

  free(a);
  a = NULL;
  free(b);
  b = NULL;
  return tmp; 
}

struct nf_space *
copy_nf_space(struct nf_space *ns){
  if (!ns)
    return NULL;
  struct nf_space *tmp = xmalloc(sizeof(*tmp));
  tmp->mf = ns->mf;
  bdd_addref(tmp->mf);
  /*if the link has always been changed in compution the copy should be used*/
  tmp->lks = copy_links_of_rule(ns->lks); 
  // tmp->lks = ns->lks;

  return tmp;
}

struct nf_space_pair *
copy_nf_space_pair(struct nf_space_pair *nsp){
  if (!nsp)
    return NULL;
  struct nf_space_pair *tmp = xmalloc(sizeof(*tmp));
  tmp->in = copy_nf_space(nsp->in);
  tmp->out = copy_nf_space(nsp->out);
  /*if the mask and rewrite have always been changed in compution the copy should be used*/
  tmp->mask = copy_mask_uint16_t(nsp->mask);
  tmp->rewrite = copy_mask_uint16_t(nsp->rewrite);
  tmp->mask = nsp->mask;
  tmp->rewrite = nsp->rewrite;
  tmp->r_arr = nsp->r_arr;
  return tmp;
}

struct matrix_element *
copy_matrix_element(struct matrix_element *a) {
  if(!a)
    return NULL;
  struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+(a->npairs)*sizeof(struct nf_space_pair *));
  tmp->npairs = a->npairs;
  tmp->bdd_in = a->bdd_in;
  bdd_addref(tmp->bdd_in);
  tmp->bdd_out = a->bdd_out;
  bdd_addref(tmp->bdd_out);
  for (int i = 0; i < a->npairs; i++)
    tmp->nf_pairs[i] = copy_nf_space_pair(a->nf_pairs[i]);
  return tmp;
}

struct matrix_element *
matrix_elem_plus_keepb(struct matrix_element *a, struct matrix_element *b) {
  if (!b)
    return a;
  if (!a)
    return copy_matrix_element(b);
  struct nf_space_pair *nps[100000];
  uint32_t count = 0;
  for (int i = 0; i < a->npairs; i++) {
    nps[count] = a->nf_pairs[i]; 
    count++;
  }

  for (int i = 0; i < b->npairs; i++) {
    bool issame = false;
    for (int j = 0; j < count; j++) {
      if (issame_nf_space_pair_action(nps[j], b->nf_pairs[i])) {
        bdd_delref(nps[j]->in->mf);
        bdd_delref(nps[j]->out->mf);
        nps[j]->in->mf = bdd_apply(nps[j]->in->mf, b->nf_pairs[i]->in->mf, bddop_or);
        nps[j]->out->mf = bdd_apply(nps[j]->out->mf, b->nf_pairs[i]->out->mf, bddop_or);
        bdd_addref(nps[j]->in->mf);
        bdd_addref(nps[j]->out->mf);
        issame = true;
        // free_nf_space_pair(b->nf_pairs[i]);
        break;
      } 
    }
    if (!issame) {
      nps[count] = copy_nf_space_pair(b->nf_pairs[i]);
      // nps[count] = b->nf_pairs[i];
      count++;
    }
  }

  struct matrix_element *tmp = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+count*sizeof(struct nf_space_pair *));
  tmp->npairs = count;
  if (a->bdd_in == b->bdd_in) {
    tmp->bdd_in = a->bdd_in;
  }
  else {
    tmp->bdd_in = bdd_apply(a->bdd_in, b->bdd_in, bddop_or);
    bdd_delref(a->bdd_in);
    bdd_addref(tmp->bdd_in);
  }
  if (a->bdd_out == b->bdd_out) {
    tmp->bdd_out = a->bdd_out;
  }
  else {
    tmp->bdd_out = bdd_apply(a->bdd_out, b->bdd_out, bddop_or);
    bdd_delref(a->bdd_out);
    bdd_addref(tmp->bdd_out);
  }
  for (int i = 0; i < count; i++) 
    tmp->nf_pairs[i] = nps[i]; 
  free(a);
  a = NULL;
  return tmp; 
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
    tmp->rows[row_idx] = xmalloc(2*sizeof(uint32_t)+sizeof(struct CS_matrix_idx_v *));
    tmp->rows[row_idx]->idx = row_idx;
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
      tmp->rows[row_idx] = xmalloc(2*sizeof(uint32_t)+count_row*sizeof(struct CS_matrix_idx_v *));
      // if(!(tmp->rows[row_idx]))
      //   printf("there is wrong\n");
      tmp->rows[row_idx]->nidx_vs = count_row;
      tmp->rows[row_idx]->idx = row_idx;
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
  tmp->rows[row_idx] = xmalloc(2*sizeof(uint32_t)+count_row*sizeof(struct CS_matrix_idx_v *));
  tmp->rows[row_idx]->idx = row_idx;
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


        struct bdd_rule *r_in = bdd_sws_arr[*(uint32_t *)lin_arrs]->rules[*(uint32_t *)(lin_arrs+1) - 1];

        BDD v_in, v_out;
        v_in = r_in->mf_out;

        uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);
        // struct timeval start,stop; 
        // gettimeofday(&start,NULL);
        // v_in = mf2bdd(r_in_mf);
        // gettimeofday(&stop,NULL);
        // time_counter_elemplus+=diff(&stop, &start);
        
        for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {
          struct bdd_rule *r_out = bdd_sws_arr[*(uint32_t *)lout_arrs]->rules[*(uint32_t *)(lout_arrs+1) - 1];
          // printf("%d-%d,%d-%d;", r_in->sw_idx, r_in->idx, r_out->sw_idx, r_out->idx);
          // struct mf_uint16_t *r_out_mf = get_r_out_mf(r_out); 
          
          
          // gettimeofday(&start,NULL);
          // v_out = mf2bdd(r_out_mf);
          v_out = r_out->mf_in;
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
            // free(r_out_mf);
            break;
          }

          lout_arrs += 2;
          // free(r_out_mf);
          
          
        }
        lin_arrs += 2;
        // free(r_in_mf);
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
      tmp->cols[col_idx] = xmalloc(2*sizeof(uint32_t)+count_col*sizeof(struct CS_matrix_idx_v *));
      tmp->cols[col_idx]->idx = col_idx;
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
  tmp->cols[col_idx] = xmalloc(2*sizeof(uint32_t)+count_col*sizeof(struct CS_matrix_idx_v *));
  tmp->cols[col_idx]->idx = col_idx;
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

/*根据 r_to_merge 来合并规则生成新的矩阵*/
/*------------------------------------------------*/


struct CS_matrix_idx_v_arr *
two_CS_matrix_idx_v_arr_plus(struct CS_matrix_idx_v_arr *row1, struct CS_matrix_idx_v_arr *row2, uint32_t num) {

  if(!row1)
    return row2;
  if(!row2)
    return row1;

  struct matrix_element *elems_arr[num];
  for (int i = 0; i < num; i++)
    elems_arr[i] = NULL;

  for (int i = 0; i < row1->nidx_vs; i++){
    elems_arr[row1->idx_vs[i]->idx] = matrix_elem_plus(elems_arr[row1->idx_vs[i]->idx], row1->idx_vs[i]->elem);
    if(row1->idx_vs[i])
      free(row1->idx_vs[i]);
    row1->idx_vs[i] = NULL;
  }
  for (int i = 0; i < row2->nidx_vs; i++){
    elems_arr[row2->idx_vs[i]->idx] = matrix_elem_plus(elems_arr[row2->idx_vs[i]->idx], row2->idx_vs[i]->elem);
    if(row2->idx_vs[i])
      free(row2->idx_vs[i]);
    row2->idx_vs[i] = NULL;
  }

  struct CS_matrix_idx_v *idx_vs_arr[num];
  uint32_t count = 0;
  for (int i = 0; i < num; i++) {
    if (elems_arr[i]) {
      idx_vs_arr[count] = xmalloc(sizeof(struct CS_matrix_idx_v));
      idx_vs_arr[count]->idx = i;
      idx_vs_arr[count]->elem = elems_arr[i];
      count++;
    }
  }

  struct CS_matrix_idx_v_arr *tmp = xmalloc(2*sizeof(uint32_t) + count*sizeof(struct CS_matrix_idx_v *));
  tmp->idx = row1->idx;
  tmp->nidx_vs = count;
  for (int i = 0; i < count; i++)
    tmp->idx_vs[i] = idx_vs_arr[i];
  if(row1)
    free(row1);
  row1 = NULL;
  if(row2)
    free(row2);
  row2 = NULL;
  return tmp;
}


struct CS_matrix_idx_v *
copy_CS_matrix_idx_v(struct CS_matrix_idx_v *idx_v){
  if (!idx_v)
    return NULL;
  struct CS_matrix_idx_v *tmp = xmalloc(sizeof(*tmp));
  tmp->idx = idx_v->idx;
  tmp->elem = copy_matrix_element(idx_v->elem);
  return tmp;
}

struct CS_matrix_idx_v_arr *
copy_CS_matrix_idx_v_arr(struct CS_matrix_idx_v_arr *idx_v_arr){
  if(!idx_v_arr)
    return NULL;
  struct CS_matrix_idx_v_arr *tmp = xmalloc(2*sizeof(uint32_t) + idx_v_arr->nidx_vs*sizeof(struct CS_matrix_idx_v *));
  tmp->idx = idx_v_arr->idx;
  tmp->nidx_vs = idx_v_arr->nidx_vs;
  for (int i = 0; i < idx_v_arr->nidx_vs; i++) {
    tmp->idx_vs[i] = copy_CS_matrix_idx_v(idx_v_arr->idx_vs[i]);
  }
  return tmp;
}

struct CS_matrix_idx_v_arr *
two_CS_matrix_idx_v_arr_plus_keeprow2(struct CS_matrix_idx_v_arr *row1, struct CS_matrix_idx_v_arr *row2, uint32_t num) {
  if(!row2)
    return row1;
  if(!row1)
    return copy_CS_matrix_idx_v_arr(row2);
  

  struct matrix_element *elems_arr[num];
  for (int i = 0; i < num; i++)
    elems_arr[i] = NULL;

  for (int i = 0; i < row1->nidx_vs; i++){
    elems_arr[row1->idx_vs[i]->idx] = matrix_elem_plus(elems_arr[row1->idx_vs[i]->idx], row1->idx_vs[i]->elem);
    if(row1->idx_vs[i])
      free(row1->idx_vs[i]);
    row1->idx_vs[i] = NULL;
  }
  for (int i = 0; i < row2->nidx_vs; i++){
    elems_arr[row2->idx_vs[i]->idx] = matrix_elem_plus_keepb(elems_arr[row2->idx_vs[i]->idx], row2->idx_vs[i]->elem);
    // if(row2->idx_vs[i])
    //   free(row2->idx_vs[i]);
    // row2->idx_vs[i] = NULL;
  }

  struct CS_matrix_idx_v *idx_vs_arr[num];
  uint32_t count = 0;
  for (int i = 0; i < num; i++) {
    if (elems_arr[i]) {
      idx_vs_arr[count] = xmalloc(sizeof(struct CS_matrix_idx_v));
      idx_vs_arr[count]->idx = i;
      idx_vs_arr[count]->elem = elems_arr[i];
      count++;
    }
  }

  struct CS_matrix_idx_v_arr *tmp = xmalloc(2*sizeof(uint32_t) + count*sizeof(struct CS_matrix_idx_v *));
  tmp->idx = row1->idx;
  tmp->nidx_vs = count;
  for (int i = 0; i < count; i++)
    tmp->idx_vs[i] = idx_vs_arr[i];
  if(row1)
    free(row1);
  row1 = NULL;
  // if(row2)
  //   free(row2);
  // row2 = NULL;
  return tmp;
}

uint32_t
get_merged_matrix_idx_fr_idx(uint32_t idx) {
  struct bdd_rule *r = matrix_idx_to_bddr(&idx);
  uint32_t merged_rown = 0;
  for (int i = 0; i < r->sw_idx; i++)
    merged_rown += merged_arr[i]->nrules;

  merged_rown += r_to_merge_arr[r->sw_idx]->rules[r->idx - 1];

  return merged_rown;
}

uint32_t
get_merged_matrix_idx_fr_2idx(uint32_t sw_idx, uint32_t idx) {
  uint32_t merged_rown = 0;
  for (int i = 0; i < sw_idx; i++)
    merged_rown += merged_arr[i]->nrules;

  merged_rown += r_to_merge_arr[sw_idx]->rules[idx - 1];

  return merged_rown;
}

struct CS_matrix_idx_v_arr *
merge_matrix_idx_v_arr(struct CS_matrix_idx_v_arr *row, uint32_t num) {
  if(!row)
    return NULL;
  struct matrix_element *elems_arr[num];
  for (int i = 0; i < num; i++)
    elems_arr[i] = NULL;

  for (int i = 0; i < row->nidx_vs; i++){
    uint32_t merged_rown = get_merged_matrix_idx_fr_idx(row->idx_vs[i]->idx);
    elems_arr[merged_rown] = matrix_elem_plus(elems_arr[merged_rown], row->idx_vs[i]->elem);
    if(row->idx_vs[i])
      free(row->idx_vs[i]);
    row->idx_vs[i] = NULL;
  }

  struct CS_matrix_idx_v *idx_vs_arr[num];
  uint32_t count = 0;
  for (int i = 0; i < num; i++) {
    if (elems_arr[i]) {
      idx_vs_arr[count] = xmalloc(sizeof(struct CS_matrix_idx_v));
      idx_vs_arr[count]->idx = i;
      idx_vs_arr[count]->elem = elems_arr[i];
      count++;
    }
  }
  struct CS_matrix_idx_v_arr *tmp = xmalloc(2*sizeof(uint32_t) + count*sizeof(struct CS_matrix_idx_v *));
  tmp->idx = row->idx;
  tmp->nidx_vs = count;
  for (int i = 0; i < count; i++)
    tmp->idx_vs[i] = idx_vs_arr[i];
  if(row)
    free(row);
  row = NULL;
  return tmp;
}

struct CS_matrix_idx_v_arr *
merge_matrix_idx_v_arr_newone(struct CS_matrix_idx_v_arr *row, uint32_t num) {
  if(!row)
    return NULL;
  struct matrix_element *elems_arr[num];
  for (int i = 0; i < num; i++)
    elems_arr[i] = NULL;

  for (int i = 0; i < row->nidx_vs; i++){
    uint32_t merged_rown = get_merged_matrix_idx_fr_idx(row->idx_vs[i]->idx);
    elems_arr[merged_rown] = matrix_elem_plus_keepb(elems_arr[merged_rown], row->idx_vs[i]->elem);
    // if(row->idx_vs[i])
    //   free(row->idx_vs[i]);
    // row->idx_vs[i] = NULL;
  }

  struct CS_matrix_idx_v *idx_vs_arr[num];
  uint32_t count = 0;
  for (int i = 0; i < num; i++) {
    if (elems_arr[i]) {
      idx_vs_arr[count] = xmalloc(sizeof(struct CS_matrix_idx_v));
      idx_vs_arr[count]->idx = i;
      idx_vs_arr[count]->elem = elems_arr[i];
      count++;
    }
  }
  struct CS_matrix_idx_v_arr *tmp = xmalloc(2*sizeof(uint32_t) + count*sizeof(struct CS_matrix_idx_v *));
  tmp->idx = row->idx;
  tmp->nidx_vs = count;
  for (int i = 0; i < count; i++)
    tmp->idx_vs[i] = idx_vs_arr[i];
  // if(row)
  //   free(row);
  // row = NULL;
  return tmp;
}

struct matrix_CSR *
gen_merged_CSR(struct matrix_CSR *matrix) {
  uint32_t all_merged_nrs = 0;
  for (int i = 0; i < SW_NUM; i++)
    all_merged_nrs += merged_arr[i]->nrules;
    // all_merged_nrs += bdd_sws_arr[i]->nrules;

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+all_merged_nrs*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = all_merged_nrs;
  for (uint32_t i = 0; i < all_merged_nrs; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix->nrows; i++) {
    // struct bdd_rule *r = matrix_idx_to_bddr(&i);
    uint32_t merged_rown = get_merged_matrix_idx_fr_idx(i);   
    // tmp->rows[merged_rown] = two_CS_matrix_idx_v_arr_plus(tmp->rows[merged_rown], matrix->rows[i], matrix->nrows);
    tmp->rows[merged_rown] = two_CS_matrix_idx_v_arr_plus_keeprow2(tmp->rows[merged_rown], matrix->rows[i], matrix->nrows);
    if (tmp->rows[merged_rown])
      tmp->rows[merged_rown]->idx = merged_rown;
  }
  for (uint32_t i = 0; i < all_merged_nrs; i++)
    tmp->rows[i] = merge_matrix_idx_v_arr(tmp->rows[i], all_merged_nrs);
  return tmp;
}

/*各个元素的连通*/
/*------------------------------------------------*/

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

  // for (int i = 0; i < a->r_arr->nrs - 1; i++) {
  //   for (int j = 0; j < b->r_arr->nrs; j++) {
  //     if ((a->r_arr->ridx[i].sw_idx == b->r_arr->ridx[j].sw_idx)&&(a->r_arr->ridx[i].r_idx == b->r_arr->ridx[j].r_idx))
  //       return NULL;
  //   }
  // }

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

  pair_tmp->r_arr = NULL;
  // pair_tmp->r_arr = xmalloc(sizeof (uint32_t)+(a->r_arr->nrs+b->r_arr->nrs -1)*sizeof (struct r_idx));
  // pair_tmp->r_arr = xcalloc(1,sizeof (uint32_t)+(a->r_arr->nrs+b->r_arr->nrs -1)*sizeof (struct r_idx));

  // pair_tmp->r_arr->nrs = a->r_arr->nrs+b->r_arr->nrs -1;
  // for (uint32_t i = 0; i < a->r_arr->nrs; i++) {
  //   pair_tmp->r_arr->ridx[i].sw_idx = a->r_arr->ridx[i].sw_idx;
  //   pair_tmp->r_arr->ridx[i].r_idx = a->r_arr->ridx[i].r_idx;
  // }
  // for (uint32_t i = 0; i < b->r_arr->nrs -1; i++) {
  //   pair_tmp->r_arr->ridx[i+a->r_arr->nrs].sw_idx = b->r_arr->ridx[i+1].sw_idx;
  //   pair_tmp->r_arr->ridx[i+a->r_arr->nrs].r_idx = b->r_arr->ridx[i+1].r_idx;
  // }

  // gettimeofday(&stop,NULL);
  // time_counter3 += diff(&stop, &start);

  return pair_tmp;
}

struct nf_space_pair *
nf_space_connect_backup(struct nf_space_pair *a, struct nf_space_pair *b) {
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

bool
issame_nf_space_pair_action(struct nf_space_pair *ns1, struct nf_space_pair *ns2) {
  if(!ns1 || !ns2)
    return false;
  if (is_mask_uint16_t_same(ns1->mask, ns2->mask) && is_mask_uint16_t_same(ns1->rewrite, ns2->rewrite))
    return true;
  return false;
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
        // bool issame = false;
        // for (int k = 0; k < count; k++) {
        //   if (issame_nf_space_pair_action(result, nps[k])) {
        //     bdd_delref(nps[k]->in->mf);
        //     bdd_delref(nps[k]->out->mf);
        //     nps[k]->in->mf = bdd_apply(nps[k]->in->mf, result->in->mf, bddop_or);
        //     nps[k]->out->mf = bdd_apply(nps[k]->out->mf, result->out->mf, bddop_or);
        //     issame = true;
        //     bdd_addref(nps[k]->in->mf);
        //     bdd_addref(nps[k]->out->mf);
        //     free_nf_space_pair(result);
        //     break;
        //   }
        // }
        // if (!issame) {
          nps[count] = result;
          count++;
        // }
      }
    }
  }

  // gettimeofday(&stop,NULL);
  // time_counter_nf_space_connect += diff(&stop, &start);
  struct matrix_element *tmp = NULL;
  // gettimeofday(&start,NULL);
  if (count) { 
    uint32_t count_ot = 1;
    for (int i = 1; i < count; i++) {
      bool issame = false;
      for (int j = 0; j < count_ot; j++) {
        if (issame_nf_space_pair_action(nps[j], nps[i])) {
          bdd_delref(nps[j]->in->mf);
          bdd_delref(nps[j]->out->mf);
          nps[j]->in->mf = bdd_apply(nps[j]->in->mf, nps[i]->in->mf, bddop_or);
          nps[j]->out->mf = bdd_apply(nps[j]->out->mf, nps[i]->out->mf, bddop_or);
          issame = true;
          bdd_addref(nps[j]->in->mf);
          bdd_addref(nps[j]->out->mf);
          free_nf_space_pair(nps[i]);
          break;
        }
      }
      if (!issame) {
        nps[count_ot] = nps[i];
        count_ot++;
      }
    }


    tmp = xmalloc(sizeof(uint32_t)+2*sizeof(BDD)+count_ot*sizeof(struct nf_space_pair *));
    tmp->bdd_in = 0;
    tmp->bdd_out = 0;
    tmp->npairs = count_ot;
    for (int i = 0; i < count_ot; i++) {
      // tmp->bdd_in = bdd_apply(tmp->bdd_in, nps[i]->in->mf, bddop_or);
      // tmp->bdd_out = bdd_apply(tmp->bdd_out, nps[i]->out->mf,bddop_or);
      tmp->nf_pairs[i] = nps[i];
    }
    // bdd_addref(tmp->bdd_in);
    // bdd_addref(tmp->bdd_out);
  } 
  // gettimeofday(&stop,NULL);
  // time_counter_elembdd_withpair += diff(&stop, &start);
  return tmp;
}

struct matrix_element * //removed the elem->in/out
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
      // if(bdd_apply(row->idx_vs[count_row]->elem->bdd_out, col->idx_vs[count_col]->elem->bdd_in, bddop_and)) {
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
      // }
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
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->idx = row->idx;
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
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->idx = row->idx;
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  // bdd_gbc();
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
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    tmp->idx = col->idx;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}

struct CS_matrix_idx_v_arr *
all_row_col_multiply_noloop(struct matrix_CSR *matrix_CSR, struct CS_matrix_idx_v_arr *col) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;

  for (uint32_t i = 0; i < matrix_CSR->nrows; i++){
    if (matrix_CSR->rows[i]){
      if (col->idx != matrix_CSR->rows[i]->idx){
        struct matrix_element *elem_tmp = row_col_multiply(matrix_CSR->rows[i], col);
        if (elem_tmp) {
          vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
          vs[vs_count]->idx = i;
          vs[vs_count]->elem = elem_tmp;
          vs_count++;
        }
      }
    }   
  }

  if(vs_count){
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    tmp->idx = col->idx;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}
struct CS_matrix_idx_v_arr *
row_all_col_multiply_noloop(struct CS_matrix_idx_v_arr *row, struct matrix_CSC *matrix_CSC) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;

  for (uint32_t i = 0; i < matrix_CSC->ncols; i++){
    if ( matrix_CSC->cols[i]){
      if (row->idx != matrix_CSC->cols[i]->idx){
        struct matrix_element *elem_tmp = row_col_multiply(row, matrix_CSC->cols[i]);
        if (elem_tmp) {
          vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
          vs[vs_count]->idx = i;
          vs[vs_count]->elem = elem_tmp;
          vs_count++;
        }
      }
    }   
  }

  if(vs_count){
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->idx = row->idx;
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

struct CS_matrix_idx_v_arr * //最后需要排序，row找对每个csr 的 row 每个乘 保存到数组，然后合并
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
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->idx = row->idx;
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}

struct CS_matrix_idx_v_arr * //最后需要排序，row找对每个csr 的 row 每个乘 保存到数组，然后合并
row_matrix_CSR_multiply_noloop(struct CS_matrix_idx_v_arr *row, struct matrix_CSR *matrix_CSR) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;

  for (int i = 0; i < row->nidx_vs; i++) {
    struct CS_matrix_idx_v *row_idxv = row->idx_vs[i];
    if (matrix_CSR->rows[row_idxv->idx]) {
      struct CS_matrix_idx_v_arr *row_matrix = matrix_CSR->rows[row_idxv->idx];
      for (uint32_t j = 0; j < row_matrix->nidx_vs; j++) {
        elemconnet_counter ++;
        if (row->idx == row_matrix->idx_vs[j]->idx)
          continue;
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
    tmp = xmalloc(2*sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->idx = row->idx;
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

    tmp = xmalloc(2*sizeof(uint32_t) + count*sizeof(struct CS_matrix_idx_v *));
    tmp->idx = row->idx;
    tmp->nidx_vs = count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  uint32_t threshold = 0;
  if((!matrix_CSR)||(!matrix_CSR1))
    return NULL;

  struct timeval start,stop; 
  gettimeofday(&start,NULL);
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR1);
  gettimeofday(&stop,NULL);
  printf("gen CSC: %ld ms\n", diff(&stop, &start)/1000);
  
  bool hasvalue = false;
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;

  for (uint32_t i = 0; i < matrix_CSR->nrows; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {

      //   gettimeofday(&start,NULL);

      //   // tmp->rows[i] = row_matrix_CSR_multiply_bysort(matrix_CSR->rows[i], matrix_CSR1);
      //   // tmp->rows[i] = row_matrix_CSR_multiply(matrix_CSR->rows[i], matrix_CSR1);
        tmp->rows[i] = row_matrix_CSR_multiply_noloop(matrix_CSR->rows[i], matrix_CSR1);
      //   gettimeofday(&stop,NULL);
      //   time_counter4+= diff(&stop, &start);
      // //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      }
      else{
      //   // printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      //   gettimeofday(&start,NULL);
        tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
      //   // tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      //   gettimeofday(&stop,NULL);
      //   time_counter5+= diff(&stop, &start);
      }
      if(tmp->rows[i])
        hasvalue = true;
    }
  }


  free_matrix_CSC_fr_CSR(matrix_CSC);
  // printf("row number:%d\n", matrix_CSR->nrows);
  // printf("tmp:%d\n", tmp->nrows);
  if (!hasvalue){
    free(tmp);
    return NULL;
  }
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply_2diff(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  uint32_t threshold = 0;
  if((!matrix_CSR)||(!matrix_CSR1))
    return NULL;

  struct timeval start,stop; 
  gettimeofday(&start,NULL);
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR1);
  gettimeofday(&stop,NULL);
  printf("gen CSC: %ld ms\n", diff(&stop, &start)/1000);
  
  bool hasvalue = false;
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;

  for (uint32_t i = 0; i < matrix_CSR->nrows; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {

      //   gettimeofday(&start,NULL);

      //   // tmp->rows[i] = row_matrix_CSR_multiply_bysort(matrix_CSR->rows[i], matrix_CSR1);
        tmp->rows[i] = sparse_matrix_multiply(matrix_CSR->rows[i], matrix_CSR1);
        // tmp->rows[i] = row_matrix_CSR_multiply_noloop(matrix_CSR->rows[i], matrix_CSR1);
      //   gettimeofday(&stop,NULL);
      //   time_counter4+= diff(&stop, &start);
      // //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      }
      else{
      //   // printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      //   gettimeofday(&start,NULL);
        // tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
        tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      //   gettimeofday(&stop,NULL);
      //   time_counter5+= diff(&stop, &start);
      }
      if(tmp->rows[i])
        hasvalue = true;
    }
  }


  free_matrix_CSC_fr_CSR(matrix_CSC);
  // printf("row number:%d\n", matrix_CSR->nrows);
  // printf("tmp:%d\n", tmp->nrows);
  if (!hasvalue){
    free(tmp);
    return NULL;
  }
  return tmp;
}

struct matrix_CSC *
sparse_matrix_multiply_CSC_allrowcol(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC) {
  if((!matrix_CSR)||(!matrix_CSC))
    return NULL;
  bool hasvalue = false;
  struct matrix_CSC *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSC->ncols)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->ncols = matrix_CSC->ncols;
  for (uint32_t i = 0; i < matrix_CSC->ncols; i++)
    tmp->cols[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSC->ncols; i++) {
    if (matrix_CSC->cols[i]){
      tmp->cols[i] = all_row_col_multiply_noloop(matrix_CSR, matrix_CSC->cols[i]);
      if(tmp->cols[i])
        hasvalue = true;
    }
  }

  if (!hasvalue){
    free(tmp);
    return NULL;
  }
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply_CSC(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1, struct matrix_CSC *matrix_CSC) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  if((!matrix_CSR)||(!matrix_CSR1))
    return NULL;
  bool hasvalue = false;
  uint32_t threshold = 0;
  // struct timeval start,stop; 
  // gettimeofday(&start,NULL);
  // gettimeofday(&stop,NULL);
  // printf("gen CSC: %ld ms\n", diff(&stop, &start)/1000);


  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      // tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {

      //   // gettimeofday(&start,NULL);


        tmp->rows[i] = row_matrix_CSR_multiply_noloop(matrix_CSR->rows[i], matrix_CSR1);
      //   // gettimeofday(&stop,NULL);
      //   // time_counter4+= diff(&stop, &start);
      // //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      }
      else{
      //   // printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      //   // gettimeofday(&start,NULL);
        tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
      //   // gettimeofday(&stop,NULL);
      //   // time_counter5+= diff(&stop, &start);
      }
      if(tmp->rows[i])
        hasvalue = true;
    }
  }
  if (!hasvalue){
    free(tmp);
    return NULL;
  }
  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply_CSC_2diff(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1, struct matrix_CSC *matrix_CSC) {
  // uint32_t threshold = matrix_CSR->nrows/600;
  if((!matrix_CSR)||(!matrix_CSR1))
    return NULL;
  bool hasvalue = false;
  uint32_t threshold = 0;
  // struct timeval start,stop; 
  // gettimeofday(&start,NULL);
  // gettimeofday(&stop,NULL);
  // printf("gen CSC: %ld ms\n", diff(&stop, &start)/1000);


  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = matrix_CSR->nrows;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++)
    tmp->rows[i] = NULL;
  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      // tmp->rows[i] = row_all_col_multiply_noloop(matrix_CSR->rows[i], matrix_CSC);
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {

      //   // gettimeofday(&start,NULL);


        tmp->rows[i] = row_matrix_CSR_multiply(matrix_CSR->rows[i], matrix_CSR1);
      //   // gettimeofday(&stop,NULL);
      //   // time_counter4+= diff(&stop, &start);
      // //   printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      }
      else{
      //   // printf("rows %d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      //   // gettimeofday(&start,NULL);
        tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      //   // gettimeofday(&stop,NULL);
      //   // time_counter5+= diff(&stop, &start);
      }
      if(tmp->rows[i])
        hasvalue = true;
    }
  }
  if (!hasvalue){
    free(tmp);
    return NULL;
  }
  return tmp;
}

struct matrix_CSR * // n row * CSR
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
  bdd_init(BDDSIZE, BDDOPCHCHE);
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
  
  struct CS_matrix_idx_v_arr *tmp = xmalloc(2*sizeof(uint32_t)+Tri_arr->nTris*sizeof(struct CS_matrix_idx_v *));
  tmp->idx = 0;
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
    // printf("gen_Tri_arr_bdd_fr_port\n");
    struct link_to_rule *lout_r = get_inoutlink_rules(link_out_rule_file, &rule_nums_out, links->arrs[0]);
     if (lout_r){
      uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*(lout_r->rule_nums - rule_nums_out));

      struct mf_uint16_t *r_in_mf = get_allx_mf_uint16_t(); 
      BDD v_in, v_out; 
      v_in = mf2bdd(r_in_mf);

      for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {
        struct bdd_rule *r_out = bdd_sws_arr[*(uint32_t *)lout_arrs]->rules[*(uint32_t *)(lout_arrs+1) - 1];
        v_out = r_out->mf_in;
        BDD v_and, v_diff;
        v_and = bdd_apply(v_in, v_out, bddop_and);
                  
        if (v_and){
          Tri_arr[nTris] = insc_to_Tri_express_rlimit_simple(links->arrs[0], r_out, v_and);
          nTris++;
          v_diff = bdd_apply(v_in, v_and, bddop_diff);
        }
        else {
          v_diff = v_in;
        } 
        v_in = v_diff;
        if (!v_in){
          break;
        }
        lout_arrs += 2;
      } 
      free(r_in_mf);
    }
  }

  struct Tri_arr *tmp = xmalloc(sizeof(uint32_t)+nTris*sizeof(struct matrix_Tri_express *));
  tmp->nTris = nTris;
  // printf("nTris %d\n", nTris);
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
  printf("--------------------------------------\n");
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
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
}


/*for test*/
/*========================================================================*/



// struct CS_matrix_idx_v {
//   uint32_t idx;
//   struct matrix_element *elem;
// };

// struct CS_matrix_idx_v_arr {
//   uint32_t nidx_vs;
//   uint32_t idx;
//   struct CS_matrix_idx_v *idx_vs[0];
// };

struct matrix_CSR *
get_delta_merged_from_a_rule(struct matrix_CSR *matrix_CSR, struct bdd_rule *r, uint32_t num) {
  uint32_t row_idx = matrix_idx_get_2idx(r->sw_idx, r->idx);

  if(!matrix_CSR->rows[row_idx])
    return NULL;
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(num)*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = num;
  for (int i = 0; i < num; i++)
    tmp->rows[i] = NULL;
  uint32_t merged_idx = get_merged_matrix_idx_fr_2idx(r->sw_idx, r->idx);
  // printf("merged_idx %d\n", merged_idx);
  // printf("row_idx %d-%d\n", row_idx, matrix_CSR->rows[row_idx]->idx);
  tmp->rows[merged_idx] = merge_matrix_idx_v_arr_newone(matrix_CSR->rows[row_idx], num);
  tmp->rows[merged_idx]->idx = merged_idx;

  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  struct CS_matrix_idx_v_arr *arr = merge_matrix_idx_v_arr_newone(matrix_CSC->cols[row_idx], num);
  if (arr) {
    arr->idx = merged_idx;
    for (int i = 0; i < arr->nidx_vs; i++) {   
      tmp->rows[arr->idx_vs[i]->idx] = xmalloc(2*sizeof(uint32_t)+sizeof(struct CS_matrix_idx_v *));
      tmp->rows[arr->idx_vs[i]->idx]->nidx_vs = 1;
      tmp->rows[arr->idx_vs[i]->idx]->idx = arr->idx;
      // tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0] = xmalloc(sizeof(struct CS_matrix_idx_v));
      // tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0]->idx = merged_idx;
      tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0] = arr->idx_vs[i];
      tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0]->idx = merged_idx;
    }
  }
  free_matrix_CSC_fr_CSR(matrix_CSC);
  return tmp;
}

struct matrix_CSR *
get_delta_from_a_rule_old(struct matrix_CSR *matrix_CSR, struct bdd_rule *r){
  uint32_t row_idx = matrix_idx_get_2idx(r->sw_idx, r->idx);
  if(!matrix_CSR->rows[row_idx])
    return NULL;

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+(matrix_CSR->nrows)*sizeof(struct CS_matrix_idx_v_arr *));
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  tmp->nrows = matrix_CSR->nrows;
  for (int i = 0; i < tmp->nrows; i++)
    tmp->rows[i] = NULL;
  tmp->rows[row_idx] = copy_CS_matrix_idx_v_arr(matrix_CSR->rows[row_idx]);
  struct CS_matrix_idx_v_arr *arr = copy_CS_matrix_idx_v_arr(matrix_CSC->cols[row_idx]);
  if (arr) {
    for (int i = 0; i < arr->nidx_vs; i++) {   
      tmp->rows[arr->idx_vs[i]->idx] = xmalloc(2*sizeof(uint32_t)+sizeof(struct CS_matrix_idx_v *));
      tmp->rows[arr->idx_vs[i]->idx]->nidx_vs = 1;
      tmp->rows[arr->idx_vs[i]->idx]->idx = arr->idx;
      // tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0] = xmalloc(sizeof(struct CS_matrix_idx_v));
      // tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0]->idx = merged_idx;
      tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0] = arr->idx_vs[i];
      tmp->rows[arr->idx_vs[i]->idx]->idx_vs[0]->idx = row_idx;
    }
  }


  free_matrix_CSC_fr_CSR(matrix_CSC);
  return tmp;
}

bool
average_updating_r_ord(struct matrix_CSR *matrix_CSR){
  struct timeval start,stop;
  // long long int average = 0;
  for (int r_i = 0; r_i < 2; r_i++) {
      /* code */
    
    struct bdd_rule *r = bdd_sws_arr[2]->rules[r_i];
    
    struct matrix_CSR *delta_CSR = get_delta_from_a_rule_old(matrix_CSR, r);

    if (!delta_CSR){
      printf("the %d - %d rule is NULL in orin_matrix_CSR!!\n", r->sw_idx, r->idx);
      continue;
    }
    else
      printf("the updating of rule %d - %d start!!\n", r->sw_idx, r->idx);

    struct matrix_CSR *delta_CSR_fw = delta_CSR;
    struct matrix_CSC *delta_CSC_bk = gen_CSC_from_CSR(delta_CSR);
    struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);

    for (int i = 0; i < 3; i++) {
      gettimeofday(&start,NULL);
      delta_CSR_fw = sparse_matrix_multiply_CSC(delta_CSR_fw, matrix_CSR, matrix_CSC);
      gettimeofday(&stop,NULL);
      if(!delta_CSR_fw)
        printf("NULL ");
      printf("fw: %ld us; ", diff(&stop, &start));
      gettimeofday(&start,NULL);
      delta_CSC_bk = sparse_matrix_multiply_CSC_allrowcol(matrix_CSR, delta_CSC_bk);
      gettimeofday(&stop,NULL);
      if(!delta_CSC_bk)
        printf("NULL ");
      printf("bk: %ld us; ", diff(&stop, &start));    
    }
    printf("\n");
  }
  printf("--------------------------------------\n");
  return 1;
}

bool
average_updating_r_merged(struct matrix_CSR *matrix_CSR, struct matrix_CSR *orin_matrix_CSR) {
  struct timeval start,stop;
  // long long int average = 0;
  for (int r_i = 0; r_i < 300; r_i++) {
    struct bdd_rule *r = bdd_sws_arr[3]->rules[r_i];
    
    // counter_init();
    struct matrix_CSR *delta_CSR = get_delta_merged_from_a_rule(orin_matrix_CSR, r, matrix_CSR->nrows);

    if (!delta_CSR){
      printf("the %d - %d rule is NULL in orin_matrix_CSR!!\n", r->sw_idx, r->idx);
      continue;
    }
    else
      printf("the updating merging of rule %d - %d start!!\n", r->sw_idx, r->idx);

    struct matrix_CSR *delta_CSR_fw = delta_CSR;
    struct matrix_CSC *delta_CSC_bk = gen_CSC_from_CSR(delta_CSR);
    struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);

    for (int i = 0; i < 3; i++) {
      gettimeofday(&start,NULL);
      delta_CSR_fw = sparse_matrix_multiply_CSC(delta_CSR_fw, matrix_CSR, matrix_CSC);
      gettimeofday(&stop,NULL);
      if(!delta_CSR_fw)
        printf("NULL ");
      printf("fw: %ld us; ", diff(&stop, &start));
      gettimeofday(&start,NULL);
      delta_CSC_bk = sparse_matrix_multiply_CSC_allrowcol(matrix_CSR, delta_CSC_bk);
      gettimeofday(&stop,NULL);
      if(!delta_CSC_bk)
        printf("NULL ");
      printf("bk: %ld us; ", diff(&stop, &start));    
    }
    printf("\n");
    // print_counter();
    // printf("--------------------------------------\n");
  }
  printf("--------------------------------------\n");
  return 1;
}

struct r_idxs *
get_r_idxs_fr_merged_matrix_idx(uint32_t merged_idx){
  uint32_t idx = merged_idx;
  struct r_idx arr[5000];
  uint32_t count = 0;
  for (int i = 0; i < SW_NUM; i++){
    if (idx < merged_arr[i]->nrules){
      for (int j = 0; j < r_to_merge_arr[i]->nrules; j++){
        if (r_to_merge_arr[i]->rules[j] == idx) {
          arr[count].sw_idx = i;
          arr[count].r_idx = j+1;
          count++;
        }
      }
      break;
    }
    else
      idx = idx - merged_arr[i]->nrules;
  }
  struct r_idxs *tmp = xmalloc(sizeof(uint32_t) + count*sizeof(struct r_idx));
  tmp->nrs = count;
  for (int i = 0; i < count; i++)
    tmp->ridx[i] = arr[i];
  return tmp;
}

void
correct_verifination(struct matrix_CSR *matrix_CSR) {
  uint32_t row_idx = 0;
  for (int i = 128; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]) {
      row_idx = i;
      break;
    }
  }
  printf("row-col: %d - ", row_idx);
  struct matrix_element *elm = matrix_CSR->rows[row_idx]->idx_vs[0]->elem;
  uint32_t col_idx = matrix_CSR->rows[row_idx]->idx_vs[0]->idx;
  printf("%d:\n", col_idx);
  struct r_idxs *row_idxs = get_r_idxs_fr_merged_matrix_idx(row_idx);
  print_r_idxs(row_idxs);
  // for (int i = 0; i < row_idxs->nrs; i++) {
  //   uint32_t merge = get_merged_matrix_idx_fr_2idx(row_idxs->ridx[i].sw_idx, row_idxs->ridx[i].r_idx);
  //   printf("%d ; ", merge);
  // }
  // printf("\n");

  printf("the 1 - 833 :%d - %d\n", bdd_sws_arr[1]->rules[832]->mf_in, bdd_sws_arr[1]->rules[832]->mf_out);
  for (int i = 0; i < row_idxs->nrs; i++){
    struct bdd_rule *tmp_r = bdd_sws_arr[row_idxs->ridx[i].sw_idx]->rules[row_idxs->ridx[i].r_idx-1];
    printf("%d - %d \n", tmp_r->mf_in,elm->bdd_in);
    if(bdd_apply(elm->bdd_in, tmp_r->mf_in, bddop_and)){
      printf("rowr: %d - %d\n", tmp_r->sw_idx, tmp_r->idx);
      break;
    }
    else
      printf("there is not a intersec\n");
    
  }
  struct r_idxs *col_idxs = get_r_idxs_fr_merged_matrix_idx(col_idx);
  print_r_idxs(col_idxs);
  // col_idxs = get_r_idxs_fr_merged_matrix_idx(0);
  // print_r_idxs(col_idxs);
}


void
print_vElemsNUM_of_Matrix_CSR(struct matrix_CSR *matrix_CSR) {
  uint32_t sum = 0;
  for (int i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]) {
      sum += matrix_CSR->rows[i]->nidx_vs;
    }
  }
  printf("This Matrix_CSR has %d valid elements\n", sum);
}

void
print_npairsNUM_of_Matrix_CSR(struct matrix_CSR *matrix_CSR) {
  uint32_t sum = 0;
  for (int i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]) {
      for (int j = 0; j < matrix_CSR->rows[i]->nidx_vs; j++) {
        sum += matrix_CSR->rows[i]->idx_vs[j]->elem->npairs;
      }
    }
  }
  printf("This Matrix_CSR has %d valid npairs\n", sum);
}

void
print_vElemsNUM_of_Matrix_CSC(struct matrix_CSC *matrix_CSC) {
  uint32_t sum = 0;
  for (int i = 0; i < matrix_CSC->ncols; i++) {
    if (matrix_CSC->cols[i]) {
      sum += matrix_CSC->cols[i]->nidx_vs;
    }
  }
  printf("This Matrix_CSC has %d valid elements\n", sum);
}

void
print_npairsNUM_of_Matrix_CSC(struct matrix_CSC *matrix_CSC) {
  uint32_t sum = 0;
  for (int i = 0; i < matrix_CSC->ncols; i++) {
    if (matrix_CSC->cols[i]) {
      for (int j = 0; j < matrix_CSC->cols[i]->nidx_vs; j++) {
        sum += matrix_CSC->cols[i]->idx_vs[j]->elem->npairs;
      }
    }
  }
  printf("This Matrix_CSC has %d valid npairs\n", sum);
}

void
test_someport_forallsquare(struct matrix_CSR *matrix_CSR){
  // struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);

  uint32_t num = 16;
  struct timeval start,stop;
  uint32_t port[16] = {100021,200010,300003,400002,500003,600002,700003,800002,900003,1000003,1100003,1200002,1300002,1400002,1500004,1600003};

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+num*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = num;
  for (int i = 0; i < num; i++) {

    tmp->rows[i] = gen_sparse_matrix_row_fr_port(port[i]);
  }


  struct matrix_CSR *muti1_CSR = sparse_matrix_multiply(matrix_CSR, matrix_CSR);
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(muti1_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR2 = sparse_matrix_multiply_CSC(tmp, muti1_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR2 = diff(&stop, &start);
  printf("port_CSR multi matrix 2t: %lld us", T_port_CSR2);
  print_vElemsNUM_of_Matrix_CSR(port_CSR2);
  print_npairsNUM_of_Matrix_CSR(port_CSR2);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(port_CSR2);
  bdd_gbc();
  printf("/*=====================================================*/\n");

  struct matrix_CSR *muti2_CSR = sparse_matrix_multiply(muti1_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti2_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR3 = sparse_matrix_multiply_CSC(tmp, muti2_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR3 = diff(&stop, &start);
  printf("port_CSR multi matrix 3t: %lld us", T_port_CSR3);
  print_vElemsNUM_of_Matrix_CSR(port_CSR3);
  print_npairsNUM_of_Matrix_CSR(port_CSR3);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti1_CSR);
  free_matrix_CSR(port_CSR3);
  bdd_gbc();
  printf("/*=====================================================*/\n");

  struct matrix_CSR *muti3_CSR = sparse_matrix_multiply(muti2_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti3_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR4 = sparse_matrix_multiply_CSC(tmp, muti3_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR4 = diff(&stop, &start);
  printf("port_CSR multi matrix 4t: %lld us", T_port_CSR4);
  print_vElemsNUM_of_Matrix_CSR(port_CSR4);
  print_npairsNUM_of_Matrix_CSR(port_CSR4);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti2_CSR);
  free_matrix_CSR(port_CSR4);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti4_CSR = sparse_matrix_multiply(muti3_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti4_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR5 = sparse_matrix_multiply_CSC(tmp, muti4_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR5 = diff(&stop, &start);
  printf("port_CSR multi matrix 5t: %lld us", T_port_CSR5);
  print_vElemsNUM_of_Matrix_CSR(port_CSR5);
  print_npairsNUM_of_Matrix_CSR(port_CSR5);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti3_CSR);
  free_matrix_CSR(port_CSR5);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti5_CSR = sparse_matrix_multiply(muti4_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti5_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR6 = sparse_matrix_multiply_CSC(tmp, muti5_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR6 = diff(&stop, &start);
  printf("port_CSR multi matrix 6t: %lld us", T_port_CSR6);
  print_vElemsNUM_of_Matrix_CSR(port_CSR6);
  print_npairsNUM_of_Matrix_CSR(port_CSR6);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti4_CSR);
  free_matrix_CSR(port_CSR6);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti6_CSR = sparse_matrix_multiply(muti5_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti6_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR7 = sparse_matrix_multiply_CSC(tmp, muti6_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR7 = diff(&stop, &start);
  printf("port_CSR multi matrix 7t: %lld us", T_port_CSR7);
  print_vElemsNUM_of_Matrix_CSR(port_CSR7);
  print_npairsNUM_of_Matrix_CSR(port_CSR7);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti5_CSR);
  free_matrix_CSR(port_CSR7);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti7_CSR = sparse_matrix_multiply(muti6_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti7_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR8 = sparse_matrix_multiply_CSC(tmp, muti7_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR8 = diff(&stop, &start);
  printf("port_CSR multi matrix 8t: %lld us", T_port_CSR8);
  print_vElemsNUM_of_Matrix_CSR(port_CSR8);
  print_npairsNUM_of_Matrix_CSR(port_CSR8);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti6_CSR);
  free_matrix_CSR(port_CSR8);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti8_CSR = sparse_matrix_multiply(muti7_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti8_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR9 = sparse_matrix_multiply_CSC(tmp, muti8_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR9 = diff(&stop, &start);
  printf("port_CSR multi matrix 9t: %lld us", T_port_CSR9);
  print_vElemsNUM_of_Matrix_CSR(port_CSR9);
  print_npairsNUM_of_Matrix_CSR(port_CSR9);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti7_CSR);
  free_matrix_CSR(port_CSR9);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti9_CSR = sparse_matrix_multiply(muti8_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti9_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR10 = sparse_matrix_multiply_CSC(tmp, muti9_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR10 = diff(&stop, &start);
  printf("port_CSR multi matrix 10t: %lld us", T_port_CSR10);
  print_vElemsNUM_of_Matrix_CSR(port_CSR10);
  print_npairsNUM_of_Matrix_CSR(port_CSR10);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti8_CSR);
  free_matrix_CSR(port_CSR10);
  bdd_gbc();
  printf("/*=====================================================*/\n");
}

void
test_someport_forallsquare_merged(struct matrix_CSR *matrix_CSR, struct matrix_CSR *orin_matrix_CSR){
  // struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);

  uint32_t num = 1;
  struct timeval start,stop;
  uint32_t port[16] = {100021,200010,300003,400002,500003,600002,700003,800002,900003,1000003,1100003,1200002,1300002,1400002,1500004,1600003};

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+num*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = num;
  for (int i = 0; i < num; i++) {
    struct CS_matrix_idx_v_arr *row_tmp = gen_sparse_matrix_row_fr_port(port[i]);
    tmp->rows[i] = merge_matrix_idx_v_arr_newone(row_tmp, matrix_CSR->nrows);
  }


  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR1 = sparse_matrix_multiply_CSC_2diff(tmp, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR1 = diff(&stop, &start);
  printf("port_CSR multi matrix 1t: %lld us", T_port_CSR1);
  print_vElemsNUM_of_Matrix_CSR(port_CSR1);
  print_npairsNUM_of_Matrix_CSR(port_CSR1);
  print_counter();
  counter_init();
  bdd_gbc();
  printf("/*=====================================================*/\n");
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR2 = sparse_matrix_multiply_CSC_2diff(port_CSR1, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR2 = diff(&stop, &start);
  printf("port_CSR multi matrix 2t: %lld us", T_port_CSR2);
  print_vElemsNUM_of_Matrix_CSR(port_CSR2);
  print_npairsNUM_of_Matrix_CSR(port_CSR2);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR1);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR3 = sparse_matrix_multiply_CSC_2diff(port_CSR2, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR3 = diff(&stop, &start);
  printf("port_CSR multi matrix 3t: %lld us", T_port_CSR3);
  print_vElemsNUM_of_Matrix_CSR(port_CSR3);
  print_npairsNUM_of_Matrix_CSR(port_CSR3);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR2);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR4 = sparse_matrix_multiply_CSC_2diff(port_CSR3, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR4 = diff(&stop, &start);
  printf("port_CSR multi matrix 4t: %lld us", T_port_CSR4);
  print_vElemsNUM_of_Matrix_CSR(port_CSR4);
  print_npairsNUM_of_Matrix_CSR(port_CSR4);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR3);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR5 = sparse_matrix_multiply_CSC_2diff(port_CSR4, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR5 = diff(&stop, &start);
  printf("port_CSR multi matrix 5t: %lld us", T_port_CSR5);
  print_vElemsNUM_of_Matrix_CSR(port_CSR5);
  print_npairsNUM_of_Matrix_CSR(port_CSR5);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR4);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR6 = sparse_matrix_multiply_CSC_2diff(port_CSR5, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR6 = diff(&stop, &start);
  printf("port_CSR multi matrix 6t: %lld us", T_port_CSR6);
  print_vElemsNUM_of_Matrix_CSR(port_CSR6);
  print_npairsNUM_of_Matrix_CSR(port_CSR6);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR5);
  bdd_gbc();
  printf("/*=====================================================*/\n");
}

