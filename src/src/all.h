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


//结构体或变量定义
//自定义
#define MF_LEN 2
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

struct PACKED nf_space {//匹配域和位置
  struct mf_uint16_t *mf;
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

//函数声明
//array.h
//tf.h
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

void
print_link(const struct link *lk) {
  printf("idx: %d;", lk->idx);
  printf(" ports: %d -- %d\n", lk->port_in, lk->port_out);
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
  printf(",");
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
  for (int i = 0; i < rs->nrs; i++) {
    printf("%d - %d, ", rs->ridx[i].sw_idx, rs->ridx[i].r_idx);
  }
  printf("\n");
}

void
print_nf_space(const struct nf_space *a) {
  printf("matchfield:");
  print_mf_uint16_t(a->mf);
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
  for (int i = 0; i < elem->npairs; i++) {
   print_nf_space_pair(elem->nf_pairs[i]);
  }
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
  for (int i = 0; i < swn; i++) {
    if (*matrix_idx < *(uint32_t *)(sws_r_num+i))
      return rule_get_2idx(i-1, *matrix_idx - *(uint32_t *)(sws_r_num+i-1) +1);
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
  free(a->mf);
  free(a->lks);
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
  if (elem)
  {
    for (uint32_t i = 0; i < elem->npairs; i++){
      if (elem->nf_pairs[i]) {
        free_nf_space_pair(elem->nf_pairs[i]);
      }  
    }
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

void
matrix_add_nf_pair(struct nf_space_pair **pairs, uint32_t *n, uint32_t idx_in, uint32_t idx_out, struct matrix_buf *matrix_buf) {
  // uint64_t idx_in64 = (uint64_t)idx_in;
  // uint64_t idx_out64 = (uint64_t)idx_out;
  uint32_t position_num = idx_in*idx_out + idx_out;
  struct matrix_element **position = (struct matrix_element **)(matrix_buf->elems+position_num);
  // printf("%d\n", *n);
  // printf("%d - %d - %d\n", *(struct matrix_element **)(matrix_buf->elems+position_num), *position, matrix_buf->elems[0]);

  if (!(*position)) {
    // printf("!(*position)\n");

    *position = xmalloc(sizeof(uint32_t)+(*n)*sizeof(struct nf_space_pair *));
    (*position)->npairs = *n;
    for (int i = 0; i < *n; i++) {
      (*position)->nf_pairs[i] = pairs[i]; 
    }
    // print_matrix_element(*position);  
  }
  else {
    uint32_t npairs = (*position)->npairs;
    struct matrix_element *tmp = (struct matrix_element *)xmalloc(sizeof(uint32_t)+((*n)+npairs)*sizeof(struct nf_space_pair *));
    tmp->npairs = (*n)+npairs;
    for (int i = 0; i < npairs; i++) {
      tmp->nf_pairs[i] = (*position)->nf_pairs[i]; 
    }
    for (int i = 0; i < *n; i++) {
      tmp->nf_pairs[i+npairs] = pairs[i];
    }
    free(*position);
    *position = tmp;  
  }
}

void //将计算的连通的两规则添加到矩阵的元素结构中
matrix_add_connect(struct of_rule *r_in, struct of_rule *r_out, struct mf_uint16_t **insc_arr, uint32_t *insc_count, struct matrix_buf *matrix_buf) {
  struct nf_space_pair *pairs[*insc_count];
  // uint32_t count  = 0;
  int add_len = sizeof(uint16_t);
  for (uint32_t i = 0; i < *insc_count; i++) {
    pairs[i] = xcalloc(1, sizeof *(pairs[i]));
    if (r_in->mask) {
      uint16_t mask[MF_LEN];
      uint16_t rewrite[MF_LEN];
      for (uint32_t j = 0; j < MF_LEN; j++) {
        mask[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_in->mask);
        rewrite[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_in->rewrite);
      }
      pairs[i]->in = xcalloc(1, sizeof *(pairs[i]->in));// 1,16(两个指针为16)
      pairs[i]->in->mf = xcalloc(1, sizeof *(pairs[i]->in->mf));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pairs[i]->in->mf->mf_w[j] = ((insc_arr[i]->mf_w[j])&mask[j])+ ((*(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->match.w))&(~mask[j]));
        pairs[i]->in->mf->mf_v[j] = ((insc_arr[i]->mf_v[j])&mask[j])+ ((*(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->match.v))&(~mask[j]));
      }
      if (r_out->mask) {
        uint16_t mask_out[MF_LEN];
        uint16_t rewrite_out[MF_LEN];
        for (uint32_t j = 0; j < MF_LEN; j++) {
          mask_out[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->mask);
          rewrite_out[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->rewrite);
        }
        pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
        for (uint32_t j = 0; j < MF_LEN; j++) {
          pairs[i]->mask->v[j] = mask[j]&mask_out[j];
          pairs[i]->rewrite->v[j] = (rewrite[j]&mask_out[j]) + (rewrite_out[j]&(~mask_out[j]));
        }
      }
      else{
        pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
        for (uint32_t j = 0; j < MF_LEN; j++) {
          pairs[i]->mask->v[j] = mask[j];
          pairs[i]->rewrite->v[j] = rewrite[j];
        }
      }     
    }
    else{
      struct links_of_rule *lks = rule_links_get(r_in, IN_LINK);
      pairs[i]->in = xcalloc(1, sizeof *(pairs[i]->in));// 1,16(两个指针为16)
      // printf("%d\n", sizeof *(pairs[i]->in));
      pairs[i]->in->lks = copy_links_of_rule(lks);
      pairs[i]->in->mf = copy_mf_uint16_t(insc_arr[i]);
      if (!(r_out->mask)) {
        pairs[i]->mask = NULL;
        pairs[i]->rewrite = NULL;
      }
      // free (lks);
    }
    if (r_out->mask) {
      uint16_t mask[MF_LEN];
      uint16_t rewrite[MF_LEN];
      for (uint32_t j = 0; j < MF_LEN; j++) {
        mask[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->mask);
        rewrite[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->rewrite);
      }
      pairs[i]->out = xcalloc(1, sizeof *(pairs[i]->out));// 1,16(两个指针为16)
      pairs[i]->out->mf = xcalloc(1, sizeof *(pairs[i]->out->mf));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pairs[i]->out->mf->mf_w[j] = (insc_arr[i]->mf_w[j])&mask[j];
        pairs[i]->out->mf->mf_v[j] = ((insc_arr[i]->mf_v[j])&mask[j])+ (rewrite[j]&(~mask[j]));
      }
      if (!(r_in->mask)){
        pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
        for (uint32_t j = 0; j < MF_LEN; j++) {
          pairs[i]->mask->v[j] = mask[j];
          pairs[i]->rewrite->v[j] = rewrite[j];
        }
      }
    }
    else{
      struct links_of_rule *lks = rule_links_get(r_out, OUT_LINK);
      // pairs[i]->out = xmalloc((sizeof *(pairs[i]->in)) + (lks->n -1)*sizeof (struct wc_uint16_t));
      pairs[i]->out = xcalloc(1, sizeof *(pairs[i]->out));// 1,16(两个指针为16)
      // printf("%d\n", sizeof *(pairs[i]->in));
      pairs[i]->out->lks = copy_links_of_rule(lks);
      pairs[i]->out->mf = copy_mf_uint16_t(insc_arr[i]);
    }
    pairs[i]->r_arr = xmalloc(sizeof (uint32_t)+2*sizeof (struct r_idx));
    pairs[i]->r_arr->nrs = 2;
    pairs[i]->r_arr->ridx[0].sw_idx = r_in->sw_idx;
    pairs[i]->r_arr->ridx[0].r_idx = r_in->idx;
    pairs[i]->r_arr->ridx[1].sw_idx = r_out->sw_idx;
    pairs[i]->r_arr->ridx[1].r_idx = r_out->idx;
  }

  uint32_t matrix_in = matrix_idx_get_r(r_in);
  uint32_t matrix_out = matrix_idx_get_r(r_out);
  matrix_add_nf_pair(pairs, insc_count, matrix_in, matrix_out, matrix_buf);
}

void //通过对链路文件查找两个同链路的头尾端规则，计算是否连通并添加到矩阵
gen_matrix(struct matrix_buf *matrix_buf) {
  // for (int i = 0; i < link_in_rule_file->swl_num; i++) {
  //   struct link *l = link_get(link_in_rule_file->links[i].link_idx);
  //   print_link(l);
  // }
  printf("%d - %d - %d\n", link_out_rule_file->swl_num, link_in_rule_file->swl_num, link_data_file->swl_num);
  // uint32_t rule_nums = 0;
  // get_link_rules(link_out_rule_file, rule_nums, 0);
  uint32_t rule_nums_in_pre = 0;
  uint32_t rule_nums_out = 0;
  for (uint32_t i = 0; i < link_in_rule_file->swl_num; i++) {
    struct link_to_rule *lout_r = get_link_rules(link_in_rule_file, &rule_nums_out, link_out_rule_file->links[i].link_idx);
    // uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);
    if (lout_r) {
      uint32_t rule_nums_in  = link_in_rule_file->links[i].rule_nums - rule_nums_in_pre;
      uint32_t *lin_arrs = (uint32_t *)(link_in_rule_data_arrs + 2*rule_nums_in_pre);
      uint32_t rule_nums_out_pre = lout_r->rule_nums - rule_nums_out;
      for (uint32_t i_in = 0; i_in < rule_nums_in; i_in++) {  
        struct of_rule *r_in = rule_get_2idx(*(uint32_t *)lin_arrs, *(uint32_t *)(lin_arrs+1));
        struct mf_uint16_t_array *r_in_mf_arr = get_r_in_mf_to_array(r_in);
        uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);
        for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {
          struct of_rule *r_out = rule_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
          struct mf_uint16_t *r_out_mf = get_r_out_mf(r_out); 
          uint32_t insc_count = 0;
          uint32_t mfs_count = 0;
          struct mf_uint16_t *insc_arr[MAX_ARR_SIZE];
          struct mf_uint16_t *mfs_arr[MAX_ARR_SIZE];

          for (uint32_t i_in_mf = 0; i_in_mf < r_in_mf_arr->n_mfs; i_in_mf++) {
            struct mf_uint16_t *insc = calc_insc(r_in_mf_arr->mfs[i_in_mf], r_out_mf);
            if (insc){
              insc_arr[insc_count] = insc;
              insc_count++;
              struct mf_uint16_t_array *arr_tmp = calc_minus_insc(r_in_mf_arr->mfs[i_in_mf], insc);
              if (arr_tmp){
                for (uint32_t j = 0; j < arr_tmp->n_mfs; j++) {
                  mfs_arr[mfs_count] = arr_tmp->mfs[j];
                  mfs_count++;
                }                
              } 
              free(r_in_mf_arr->mfs[i_in_mf]);  
              r_in_mf_arr->mfs[i_in_mf] = NULL;           
            }
            else{
              mfs_arr[mfs_count] = r_in_mf_arr->mfs[i_in_mf];
              mfs_count++;
            }
          }
          if(insc_count){ 
            matrix_add_connect(r_in, r_out, insc_arr, &insc_count, matrix_buf);
            // struct links_of_rule *ls = rule_links_get(r_in, IN_LINK); 
            // print_links_of_rule(ls);
            free_insc_arr(insc_arr, &insc_count);
            if (!mfs_count){
              free(r_out_mf);
              break;
            }
            free(r_in_mf_arr);

            r_in_mf_arr = (struct mf_uint16_t_array *)xmalloc(sizeof(uint32_t)+mfs_count*sizeof(r_out_mf));
            r_in_mf_arr->n_mfs = mfs_count;
            memcpy (r_in_mf_arr->mfs, mfs_arr, mfs_count*sizeof(r_out_mf));    
          }
          lout_arrs += 2;
          free(r_out_mf);
        }
        lin_arrs += 2;
        free_mf_uint16_t_array(r_in_mf_arr);
      }
    }
    rule_nums_in_pre = link_in_rule_file->links[i].rule_nums;
  }
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

struct nf_space_pair *
nf_space_connect(struct nf_space_pair *a, struct nf_space_pair *b) {
  if(!is_insc_links(a->out->lks, b->in->lks))
    return NULL;
  struct mf_uint16_t *insc = calc_insc(a->out->mf, b->in->mf);
  if (!insc) 
    return NULL;
  struct nf_space_pair *pair_tmp = xcalloc(1, sizeof *pair_tmp);
  

  if (a->mask) {
    pair_tmp->in = xcalloc(1, sizeof *(pair_tmp->in));// 1,16(两个指针为16)
    pair_tmp->in->mf = xcalloc(1, sizeof *(pair_tmp->in->mf));
    for (uint32_t j = 0; j < MF_LEN; j++) {
      pair_tmp->in->mf->mf_w[j] = ((insc->mf_w[j])&(a->mask->v[j]))+ ((a->in->mf->mf_w[j])&(a->mask->v[j]));
      pair_tmp->in->mf->mf_v[j] = ((insc->mf_v[j])&(a->mask->v[j]))+ ((a->in->mf->mf_v[j])&(~(a->mask->v[j])));
    }
    if (b->mask) {
      pair_tmp->mask = xcalloc(1, sizeof *(pair_tmp->mask));
      pair_tmp->rewrite = xcalloc(1, sizeof *(pair_tmp->rewrite));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pair_tmp->mask->v[j] = (a->mask->v[j])&(b->mask->v[j]);
        pair_tmp->rewrite->v[j] = ((a->rewrite->v[j])&(b->mask->v[j])) + ((b->rewrite->v[j])&(~(b->mask->v[j])));
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
    pair_tmp->in = xcalloc(1, sizeof *(pair_tmp->in));// 1,16(两个指针为16)
    // printf("%d\n", sizeof *(pair_tmp->in));
    pair_tmp->in->lks = copy_links_of_rule(a->in->lks);
    pair_tmp->in->mf = copy_mf_uint16_t(insc);
    if (!(b->mask)) {
      pair_tmp->mask = NULL;
      pair_tmp->rewrite = NULL;
    }
    // free (lks);
  }
  if (b->mask) {
    pair_tmp->out = xcalloc(1, sizeof *(pair_tmp->out));// 1,16(两个指针为16)
    pair_tmp->out->mf = xcalloc(1, sizeof *(pair_tmp->out->mf));
    for (uint32_t j = 0; j < MF_LEN; j++) {
      pair_tmp->out->mf->mf_w[j] = (insc->mf_w[j])&(b->mask->v[j]);
      pair_tmp->out->mf->mf_v[j] = ((insc->mf_v[j])&(b->mask->v[j])) + ((b->rewrite->v[j])&(~(b->mask->v[j])));
    }
    if (!(a->mask)){
      pair_tmp->mask = xcalloc(1, sizeof *(pair_tmp->mask));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pair_tmp->mask->v[j] = b->mask->v[j];
        pair_tmp->rewrite->v[j] = b->rewrite->v[j];
      }
    }
  }
  else{
    // struct links_of_rule *lks = rule_links_get(r_out, OUT_LINK);
    // pair_tmp->out = xmalloc((sizeof *(pair_tmp->in)) + (lks->n -1)*sizeof (struct wc_uint16_t));
    pair_tmp->out = xcalloc(1, sizeof *(pair_tmp->out));// 1,16(两个指针为16)
    // printf("%d\n", sizeof *(pair_tmp->in));
    pair_tmp->out->lks = copy_links_of_rule(b->out->lks);
    pair_tmp->out->mf = copy_mf_uint16_t(insc);//建立copy
  }

  // pair_tmp->in = xcalloc(1, sizeof *(pair_tmp->in));// 1,16(两个指针为16)
  // pair_tmp->in->lks = copy_links_of_rule(a->in->lks);
  // pair_tmp->in->mf = copy_mf_uint16_t(insc);
  // pair_tmp->out = xcalloc(1, sizeof *(pair_tmp->out));// 1,16(两个指针为16)
  // pair_tmp->out->lks = copy_links_of_rule(b->out->lks);
  // pair_tmp->out->mf = copy_mf_uint16_t(insc);
  // pair_tmp->mask = NULL;
  // pair_tmp->rewrite = NULL;



  pair_tmp->r_arr = xmalloc(sizeof (uint32_t)+(a->r_arr->nrs+b->r_arr->nrs -1)*sizeof (struct r_idx));
  pair_tmp->r_arr->nrs = a->r_arr->nrs+b->r_arr->nrs -1;
  for (uint32_t i = 0; i < a->r_arr->nrs; i++) {
    pair_tmp->r_arr->ridx[i].sw_idx = a->r_arr->ridx[i].sw_idx;
    pair_tmp->r_arr->ridx[i].r_idx = a->r_arr->ridx[i].r_idx;
  }
  for (uint32_t i = 0; i < b->r_arr->nrs -1; i++) {
    pair_tmp->r_arr->ridx[i+a->r_arr->nrs].sw_idx = b->r_arr->ridx[i+1].sw_idx;
    pair_tmp->r_arr->ridx[i+a->r_arr->nrs].r_idx = b->r_arr->ridx[i+1].r_idx;
  }
  free(insc);
  // return pair_tmp;
  return pair_tmp;
}

uint32_t
matrix_elem_connect(struct matrix_element *a, struct matrix_element *b, uint32_t idx_in, uint32_t idx_out, struct matrix_buf *matrix_buf) {
  struct nf_space_pair *tmp[MAX_ARR_SIZE];
  uint32_t count = 0;
  for (uint32_t i = 0; i < a->npairs; i++) {
    for (uint32_t j = 0; j < b->npairs; j++) {
      struct nf_space_pair *result = nf_space_connect(a->nf_pairs[i], a->nf_pairs[j]);
      if (result) {
        tmp[count] = result;
        count++;
      } 
    }
  }
  if (count) {
    matrix_add_nf_pair(tmp, &count, idx_in, idx_out, matrix_buf);
  }
  else
    return 0;
  return 1;
}

struct matrix_buf *
matrix_multiply(struct matrix_buf *a, struct matrix_buf *b) {
  struct matrix_buf *tmp = matrix_init();
  for (uint32_t j = 0; j < tmp->nelems; j++) { //j为列，i为行
    struct matrix_element *row[tmp->nelems];

    for (uint32_t i = 0; i < tmp->nelems; i++) {
      row[j] = *(struct matrix_element **)(b->elems+i*j+j);
    }
    for (uint32_t i = 0; i < tmp->nelems; i++) {
      struct matrix_element **line = (struct matrix_element **)(a->elems+i*j);
      for (uint32_t k = 0; k < 2000; k++)
      {
        if ((line[k])&&(row[k]));
      }
      // for (uint32_t k = 0; k < tmp->nelems; k++) {//  k为行乘以列位数
        // if ((line[k])&&(row[k])) {
          // matrix_elem_connect(line[i], row[i], i, j, tmp);
        // line = (struct matrix_element **)(line+1);
        // }
      // }
    }
  }
  return tmp;
}



//稀疏矩阵处理
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
print_Tri_express(struct matrix_Tri_express *Tri){
  printf("row-col:%d - %d\n", Tri->row_idx, Tri->col_idx);
  print_matrix_element(Tri->elem);
}

struct matrix_Tri_express * 
insc_to_Tri_express_2r(struct of_rule *r_in, struct of_rule *r_out, struct mf_uint16_t **insc_arr, uint32_t *insc_count) {
  struct nf_space_pair *pairs[*insc_count];
  int add_len = sizeof(uint16_t);
  for (uint32_t i = 0; i < *insc_count; i++) {
    pairs[i] = xcalloc(1, sizeof *(pairs[i]));
    if (r_in->mask) {
      uint16_t mask[MF_LEN];
      uint16_t rewrite[MF_LEN];
      for (uint32_t j = 0; j < MF_LEN; j++) {
        mask[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_in->mask);
        rewrite[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_in->rewrite);
      }
      pairs[i]->in = xcalloc(1, sizeof *(pairs[i]->in));// 1,16(两个指针为16)
      pairs[i]->in->mf = xcalloc(1, sizeof *(pairs[i]->in->mf));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pairs[i]->in->mf->mf_w[j] = ((insc_arr[i]->mf_w[j])&mask[j])+ ((*(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->match.w))&(~mask[j]));
        pairs[i]->in->mf->mf_v[j] = ((insc_arr[i]->mf_v[j])&mask[j])+ ((*(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->match.v))&(~mask[j]));
      }
      if (r_out->mask) {
        uint16_t mask_out[MF_LEN];
        uint16_t rewrite_out[MF_LEN];
        for (uint32_t j = 0; j < MF_LEN; j++) {
          mask_out[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->mask);
          rewrite_out[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->rewrite);
        }
        pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
        pairs[i]->rewrite = xcalloc(1, sizeof *(pairs[i]->rewrite));
        for (uint32_t j = 0; j < MF_LEN; j++) {
          pairs[i]->mask->v[j] = mask[j]&mask_out[j];
          pairs[i]->rewrite->v[j] = (rewrite[j]&mask_out[j]) + (rewrite_out[j]&(~mask_out[j]));
        }
      }
      else{
        pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
        pairs[i]->rewrite = xcalloc(1, sizeof *(pairs[i]->rewrite));
        for (uint32_t j = 0; j < MF_LEN; j++) {
          pairs[i]->mask->v[j] = mask[j];
          pairs[i]->rewrite->v[j] = rewrite[j];
        }
      }     
    }
    else{
      struct links_of_rule *lks = rule_links_get(r_in, IN_LINK);
      pairs[i]->in = xcalloc(1, sizeof *(pairs[i]->in));// 1,16(两个指针为16)
      // printf("%d\n", sizeof *(pairs[i]->in));
      pairs[i]->in->lks = copy_links_of_rule(lks);
      pairs[i]->in->mf = copy_mf_uint16_t(insc_arr[i]);
      if (!(r_out->mask)) {
        pairs[i]->mask = NULL;
        pairs[i]->rewrite = NULL;
      }
      // free (lks);
    }
    if (r_out->mask) {
      uint16_t mask[MF_LEN];
      uint16_t rewrite[MF_LEN];
      for (uint32_t j = 0; j < MF_LEN; j++) {
        mask[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->mask);
        rewrite[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_out->rewrite);
      }
      pairs[i]->out = xcalloc(1, sizeof *(pairs[i]->out));// 1,16(两个指针为16)
      pairs[i]->out->mf = xcalloc(1, sizeof *(pairs[i]->out->mf));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pairs[i]->out->mf->mf_w[j] = (insc_arr[i]->mf_w[j])&mask[j];
        pairs[i]->out->mf->mf_v[j] = ((insc_arr[i]->mf_v[j])&mask[j])+ (rewrite[j]&(~mask[j]));
      }
      if (!(r_in->mask)){
        pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
        pairs[i]->rewrite = xcalloc(1, sizeof *(pairs[i]->rewrite));
        for (uint32_t j = 0; j < MF_LEN; j++) {
          pairs[i]->mask->v[j] = mask[j];
          pairs[i]->rewrite->v[j] = rewrite[j];
        }
      }
    }
    else{
      struct links_of_rule *lks = rule_links_get(r_out, OUT_LINK);
      // pairs[i]->out = xmalloc((sizeof *(pairs[i]->in)) + (lks->n -1)*sizeof (struct wc_uint16_t));
      pairs[i]->out = xcalloc(1, sizeof *(pairs[i]->out));// 1,16(两个指针为16)
      // printf("%d\n", sizeof *(pairs[i]->in));
      pairs[i]->out->lks = copy_links_of_rule(lks);
      pairs[i]->out->mf = copy_mf_uint16_t(insc_arr[i]);
    }
    pairs[i]->r_arr = xmalloc(sizeof (uint32_t)+2*sizeof (struct r_idx));
    pairs[i]->r_arr->nrs = 2;
    pairs[i]->r_arr->ridx[0].sw_idx = r_in->sw_idx;
    pairs[i]->r_arr->ridx[0].r_idx = r_in->idx;
    pairs[i]->r_arr->ridx[1].sw_idx = r_out->sw_idx;
    pairs[i]->r_arr->ridx[1].r_idx = r_out->idx;
  }

  struct matrix_Tri_express *tmp = xcalloc(1, sizeof *tmp);
  tmp->elem = xmalloc(sizeof(uint32_t)+(*insc_count)*sizeof(struct nf_space_pair *));
  tmp->elem->npairs = *insc_count;
  tmp->row_idx = matrix_idx_get_r(r_in);
  tmp->col_idx = matrix_idx_get_r(r_out);
  for (int i = 0; i < *insc_count; i++) {
    tmp->elem->nf_pairs[i] = pairs[i]; 
  }
  return tmp;
}

struct matrix_Tri_express * 
insc_to_Tri_express_rlimit(struct of_rule *r_in, struct of_rule *r_out, struct mf_uint16_t **insc_arr, uint32_t *insc_count) {
  // struct nf_space_pair *pairs[*insc_count];
  struct nf_space_pair **pairs = xmalloc((*insc_count)*sizeof (struct nf_space_pair *));
  struct links_of_rule *lks = rule_links_get(r_in, IN_LINK);
  struct links_of_rule *lks_out = rule_links_get(r_in, OUT_LINK);
  struct links_of_rule *lks_in = rule_links_get(r_out, IN_LINK);
  lks_out = links_insc(lks_in, lks_out);
  // uint32_t count  = 0;
  int add_len = sizeof(uint16_t);
  if (r_in->mask) {
    uint16_t mask[MF_LEN];
    uint16_t rewrite[MF_LEN];
    for (uint32_t j = 0; j < MF_LEN; j++) {
      mask[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_in->mask);
      rewrite[j] = *(uint16_t *)((uint8_t *)data_arrs+j+r_in->rewrite);
    }
    for (uint32_t i = 0; i < *insc_count; i++) {
      pairs[i] = xcalloc(1, sizeof *(pairs[i]));
      pairs[i]->in = xcalloc(1, sizeof *(pairs[i]->in));// 1,16(两个指针为16)
      pairs[i]->in->mf = xcalloc(1, sizeof *(pairs[i]->in->mf));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pairs[i]->in->mf->mf_w[j] = ((insc_arr[i]->mf_w[j])&mask[j])+ ((*(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->match.w))&(~mask[j]));
        pairs[i]->in->mf->mf_v[j] = ((insc_arr[i]->mf_v[j])&mask[j])+ ((*(uint16_t *)((uint8_t *)data_arrs+j*add_len+r_in->match.v))&(~mask[j]));
      }     
      pairs[i]->mask = xcalloc(1, sizeof *(pairs[i]->mask));
      pairs[i]->rewrite = xcalloc(1, sizeof *(pairs[i]->rewrite));
      for (uint32_t j = 0; j < MF_LEN; j++) {
        pairs[i]->mask->v[j] = mask[j];
        pairs[i]->rewrite->v[j] = rewrite[j];
      } 
      pairs[i]->in->lks = copy_links_of_rule(lks);
      pairs[i]->out = xcalloc(1, sizeof *(pairs[i]->out));
      pairs[i]->out->mf = copy_mf_uint16_t(insc_arr[i]);
      pairs[i]->out->lks = copy_links_of_rule(lks_out);
      pairs[i]->r_arr = xmalloc(sizeof (uint32_t)+sizeof (struct r_idx));
      pairs[i]->r_arr->nrs = 2;
      pairs[i]->r_arr->ridx[0].sw_idx = r_in->sw_idx;
      pairs[i]->r_arr->ridx[0].r_idx = r_in->idx;
      pairs[i]->r_arr->ridx[1].sw_idx = r_out->sw_idx;
      pairs[i]->r_arr->ridx[1].r_idx = r_out->idx;
    }
  }
  else{
    for (uint32_t i = 0; i < *insc_count; i++) {
      pairs[i] = xcalloc(1, sizeof *(pairs[i]));
      pairs[i]->in = xcalloc(1, sizeof *(pairs[i]->in));// 1,16(两个指针为16)
      pairs[i]->in->mf = copy_mf_uint16_t(insc_arr[i]);    
      pairs[i]->mask = NULL;
      pairs[i]->rewrite = NULL;
      pairs[i]->in->lks = copy_links_of_rule(lks);
      pairs[i]->out = xcalloc(1, sizeof *(pairs[i]->out));
      pairs[i]->out->mf = copy_mf_uint16_t(insc_arr[i]);
      pairs[i]->out->lks = copy_links_of_rule(lks_out);
      pairs[i]->r_arr = xmalloc(sizeof (uint32_t)+sizeof (struct r_idx));
      pairs[i]->r_arr->nrs = 2;
      pairs[i]->r_arr->ridx[1].sw_idx = r_out->sw_idx;
      pairs[i]->r_arr->ridx[1].r_idx = r_out->idx;
    }
  }

  struct matrix_Tri_express *tmp = xcalloc(1, sizeof *tmp);
  tmp->elem = xmalloc(sizeof(uint32_t)+(*insc_count)*sizeof(struct nf_space_pair *));
  tmp->elem->npairs = *insc_count;
  tmp->row_idx = matrix_idx_get_r(r_in);
  tmp->col_idx = matrix_idx_get_r(r_out);
  for (int i = 0; i < *insc_count; i++) {
    tmp->elem->nf_pairs[i] = pairs[i]; 
  }
  free(pairs);
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
  // printf("there is wrong: %d - %d\n", data_allr_nums, Tri_arr->nTris);
  qsort(Tri_arr->arr, Tri_arr->nTris,sizeof (struct matrix_Tri_express *), cmp_matrix_Tri_express);

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+data_allr_nums*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = data_allr_nums;
  for (int i = 0; i < data_allr_nums; i++){
    tmp->rows[i] = NULL;
  }
  struct matrix_Tri_express *Tri_arr_tmp[Tri_arr->nTris];
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
  printf("%d - %d\n", Tri_arr->arr[21280]->row_idx, Tri_arr->arr[21280]->col_idx);

  for (uint32_t i = 1; i < Tri_arr->nTris; i++) {   
    // for (int i = 0; i < count * MF_LEN; i += MF_LEN) {
    //计数乘以MF_LEN，i+MF_LEN，i，按match所占uint16_t位数循环，在前面的文件中
    if (Tri_is_eq (Tri_arr->arr[i], Tri_arr->arr[last])) {
      Tri_arr_tmp[count-1]->elem = matrix_elem_plus(Tri_arr_tmp[count-1]->elem, Tri_arr->arr[i]->elem);
      free(Tri_arr->arr[i]);
      continue;
    }
    Tri_arr_tmp[count] = Tri_arr->arr[i];
    if (Tri_arr_tmp[count]->row_idx != Tri_arr_tmp[count-1]->row_idx) {
      // if (Tri_arr_tmp[count-1]->row_idx== 2940){
      //   printf("%d-%d\n",Tri_arr_tmp[count-1]->row_idx, Tri_arr_tmp[begin]->row_idx);
      //   printf("%d\n",count_row);
      // }
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
  printf("all num = %d\n", count);
  return tmp;
}

// struct ternarytree_node {
//   struct ternarytree *up_node;
//   struct ternarytree *v_0;
//   struct ternarytree *v_1;
//   struct ternarytree *v_x;
//   struct mf_uint16_t *endnode_v; 
// };

// struct ternarytree_node *
// ternarytree_add_anode(struct mf_uint16_t *endnode, struct ternarytree_node *rootnode) {

// }

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
mf_can_merge(struct mf_uint16_t *a, struct mf_uint16_t *b) {
  bool has_one_sign = false;

  // print_mf_uint16_t(a);
  // print_mf_uint16_t(b);
  // printf("time count1\n");
  for (uint32_t i = 0; i < MF_LEN; i++) {
    if ((a->mf_w[i]) != (b->mf_w[i])){
      return false;
    }
  }
  for (uint32_t i = 0; i < MF_LEN; i++) {
    uint16_t xor_tmp = (a->mf_v[i])^(b->mf_v[i]);
    if (xor_tmp){
      uint16_t field = 0x8000;
      for (uint32_t j = 0; j < 16; j++) {
        if(xor_tmp & field){
          if (has_one_sign){
            return false;
          }
          has_one_sign = true;
        }
        field >>= 1;
      }
    }
  }
  return true;
}

struct mf_uint16_t *
merge_mf(struct mf_uint16_t *a, struct mf_uint16_t *b) {

  // printf("time count merge_mf\n");

  struct mf_uint16_t *tmp = (struct mf_uint16_t *)xcalloc (1, sizeof *tmp);
  for (uint32_t i = 0; i < MF_LEN; i++) {
    
    uint16_t xor_tmp = (a->mf_v[i])^(b->mf_v[i]);
    if (xor_tmp){
      tmp->mf_w[i] = a->mf_w[i] + xor_tmp;
      tmp->mf_v[i] = (a->mf_v[i]) & (~xor_tmp);
    }
    else{
      tmp->mf_w[i] = a->mf_w[i];
      tmp->mf_v[i] = a->mf_v[i];
    }
  }

  free(a);
  a = NULL;
  free(b);
  b = NULL;
  return tmp;
}


struct Tri_arr *
gen_Tri_arr(void) {
  printf("there is wrong\n");
  uint32_t max_CSR = MAX_VAL_RATE*data_allr_nums*data_allr_nums;
  // struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+sizeof(struct CS_matrix_idx_v *))
  struct matrix_Tri_express *Tri_arr[max_CSR];
  uint32_t nTris = 0;
  uint32_t rule_nums_in_pre = 0;
  uint32_t rule_nums_out = 0;
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
        struct mf_uint16_t_array *r_in_mf_arr = get_r_in_mf_to_array(r_in);
        uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);

        
        for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {

          struct of_rule *r_out = rule_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
          // // printf("%d-%d,%d-%d;", r_in->sw_idx, r_in->idx, r_out->sw_idx, r_out->idx);
          struct mf_uint16_t *r_out_mf = get_r_out_mf(r_out); 
          uint32_t insc_count = 0;
          uint32_t mfs_count = 0;
          // struct mf_uint16_t *insc_arr[150000];//这里的大小是有问题的
          // struct mf_uint16_t *mfs_arr1[200000];
          struct mf_uint16_t **insc_arr = xmalloc((r_in_mf_arr->n_mfs)*sizeof(struct mf_uint16_t *));
          // char *buf_mfs;
          // size_t sz_mfs;
          // FILE *mfs_arr = open_memstream (&buf_mfs, &sz_mfs);
          struct mf_uint16_t *mfs_arr[100000];
          if(is_mf_allx(r_out_mf)){
            memcpy(insc_arr, r_in_mf_arr->mfs,(r_in_mf_arr->n_mfs)*sizeof(struct mf_uint16_t *));
            insc_count = r_in_mf_arr->n_mfs;
          }
          else {
            for (uint32_t i_in_mf = 0; i_in_mf < r_in_mf_arr->n_mfs; i_in_mf++) {
              if (mfs_count >10000)
                printf("mfs_count >10000\n");

              struct mf_uint16_t *insc = calc_insc(r_in_mf_arr->mfs[i_in_mf], r_out_mf);
              if (insc){ //如果有交集，且只有一个
                insc_arr[insc_count] = insc;
                insc_count++;
                struct mf_uint16_t_array *arr_tmp = calc_minus_insc(r_in_mf_arr->mfs[i_in_mf], insc); //剩余部分
                if (arr_tmp){ //如果有剩余
                  for (uint32_t j = 0; j < arr_tmp->n_mfs; j++) {                
                    // mfs_arr[mfs_count] = arr_tmp->mfs[j];
                    // fwrite (&(arr_tmp->mfs[j]),sizeof (r_out_mf), 1, mfs_arr);
                    // mfs_count++;
                    //判断与合并部分
                    uint32_t mfs_i = 0;
                    // printf("mfs_count:%d\n", mfs_count);
                    
                    for (mfs_i = 0; mfs_i < mfs_count; mfs_i++) {
                      // printf("mfs_i:%d\n", mfs_i);
                      // printf("mfs_count:%d\n", mfs_count);
                      if (mf_can_merge(mfs_arr[mfs_i], arr_tmp->mfs[j])){
                        mfs_arr[mfs_i] = merge_mf(mfs_arr[mfs_i], arr_tmp->mfs[j]);
                        break;
                      }
                    }
                    if (mfs_i == mfs_count){
                      mfs_arr[mfs_count] = arr_tmp->mfs[j];
                      mfs_count++;
                    }                    
                  }
                  free(arr_tmp);
                } 
                free(r_in_mf_arr->mfs[i_in_mf]);  
                r_in_mf_arr->mfs[i_in_mf] = NULL;           
              }
              else{
                mfs_arr[mfs_count] = r_in_mf_arr->mfs[i_in_mf];

                // fwrite (&(r_in_mf_arr->mfs[i_in_mf]),sizeof (r_out_mf), 1, mfs_arr);
                mfs_count++;
              }
            }
          }

          // fclose(mfs_arr);
          if(insc_count){ 
            // Tri_arr[nTris] = insc_to_Tri_express_rlimit(r_in, r_out, insc_arr, &insc_count);
            // nTris++;
            // struct links_of_rule *ls = rule_links_get(r_in, IN_LINK); 
            // print_links_of_rule(ls);
            free_insc_arr(insc_arr, &insc_count);
            if (!mfs_count){
              free(r_out_mf);
              //free(buf_mfs);
              break;
            }
            free(r_in_mf_arr);
            // if(lout_r->link_idx == 43)
            //   printf("mfs_count%d;", mfs_count);

            r_in_mf_arr = (struct mf_uint16_t_array *)xmalloc(sizeof(uint32_t)+mfs_count*sizeof(r_out_mf));
            uint32_t n_mfs_count = 0;
            for (uint32_t j = 0; j < mfs_count; j++) {                
              uint32_t mfs_i = 0;
              for (mfs_i = 0; mfs_i < n_mfs_count; mfs_i++) {
                if (mf_can_merge(r_in_mf_arr->mfs[mfs_i], mfs_arr[j])){
                  r_in_mf_arr->mfs[mfs_i] = merge_mf(r_in_mf_arr->mfs[mfs_i], mfs_arr[j]);
                  break;
                }
              }
              if (mfs_i == n_mfs_count){
                r_in_mf_arr->mfs[n_mfs_count] = mfs_arr[j];
                n_mfs_count++;
              }                    
            }

            r_in_mf_arr->n_mfs = n_mfs_count;
            // memcpy (r_in_mf_arr->mfs, mfs_arr, mfs_count*sizeof(r_out_mf));
            // memcpy (r_in_mf_arr->mfs, buf_mfs, mfs_count*sizeof(r_out_mf)); 
          }
          lout_arrs += 2;
          free(r_out_mf);
          // free(buf_mfs);
          free(insc_arr);
        }
        lin_arrs += 2;
      }
    }
    rule_nums_in_pre = link_in_rule_file->links[i].rule_nums;
  }

  struct Tri_arr *tmp = xmalloc(sizeof(uint32_t)+nTris*sizeof(struct matrix_Tri_express *));
  tmp->nTris = nTris;
  for (uint32_t i = 0; i < nTris; i++){
    tmp->arr[i] = Tri_arr[i];
  }
  return tmp;
}

struct Tri_arr *
gen_Tri_arr_backup(void) {
  uint32_t max_CSR = MAX_VAL_RATE*data_allr_nums*data_allr_nums;
  // struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+sizeof(struct CS_matrix_idx_v *))
  struct matrix_Tri_express *Tri_arr[max_CSR];
  uint32_t nTris = 0;
  uint32_t rule_nums_in_pre = 0;
  uint32_t rule_nums_out = 0;
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
        struct mf_uint16_t_array *r_in_mf_arr = get_r_in_mf_to_array(r_in);
        uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);
        for (uint32_t i_out = 0; i_out < rule_nums_out; i_out++) {

          struct of_rule *r_out = rule_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));
          // // printf("%d-%d,%d-%d;", r_in->sw_idx, r_in->idx, r_out->sw_idx, r_out->idx);
          struct mf_uint16_t *r_out_mf = get_r_out_mf(r_out); 
          uint32_t insc_count = 0;
          uint32_t mfs_count = 0;
          // struct mf_uint16_t *insc_arr[150000];//这里的大小是有问题的
          // struct mf_uint16_t *mfs_arr1[200000];
          struct mf_uint16_t **insc_arr = xmalloc((r_in_mf_arr->n_mfs)*sizeof(struct mf_uint16_t *));
          char *buf_mfs;
          size_t sz_mfs;
          FILE *mfs_arr = open_memstream (&buf_mfs, &sz_mfs);
          if(is_mf_allx(r_out_mf)){
            memcpy(insc_arr, r_in_mf_arr->mfs,(r_in_mf_arr->n_mfs)*sizeof (struct mf_uint16_t *));
            insc_count = r_in_mf_arr->n_mfs;
          }
          else {
            for (uint32_t i_in_mf = 0; i_in_mf < r_in_mf_arr->n_mfs; i_in_mf++) {
              struct mf_uint16_t *insc = calc_insc(r_in_mf_arr->mfs[i_in_mf], r_out_mf);
              if (insc){
                insc_arr[insc_count] = insc;
                insc_count++;
                struct mf_uint16_t_array *arr_tmp = calc_minus_insc(r_in_mf_arr->mfs[i_in_mf], insc);
                if (arr_tmp){
                  for (uint32_t j = 0; j < arr_tmp->n_mfs; j++) {                
                    // mfs_arr[mfs_count] = arr_tmp->mfs[j];
                    fwrite (&(arr_tmp->mfs[j]),sizeof (r_out_mf), 1, mfs_arr);
                    mfs_count++;
                  }
                } 
                free(r_in_mf_arr->mfs[i_in_mf]);  
                r_in_mf_arr->mfs[i_in_mf] = NULL;           
              }
              else{
                // mfs_arr[mfs_count] = r_in_mf_arr->mfs[i_in_mf];

                fwrite (&(r_in_mf_arr->mfs[i_in_mf]),sizeof (r_out_mf), 1, mfs_arr);
                mfs_count++;
              }
            }
          }

          fclose(mfs_arr);
          if(insc_count){ 
            Tri_arr[nTris] = insc_to_Tri_express_rlimit(r_in, r_out, insc_arr, &insc_count);
            nTris++;
            // struct links_of_rule *ls = rule_links_get(r_in, IN_LINK); 
            // print_links_of_rule(ls);
            free_insc_arr(insc_arr, &insc_count);
            if (!mfs_count){
              free(r_out_mf);
              free(buf_mfs);
              break;
            }
            free(r_in_mf_arr);
            // if(lout_r->link_idx == 43)
            //   printf("mfs_count%d;", mfs_count);

            r_in_mf_arr = (struct mf_uint16_t_array *)xmalloc(sizeof(uint32_t)+mfs_count*sizeof(r_out_mf));
            r_in_mf_arr->n_mfs = mfs_count;
            // memcpy (r_in_mf_arr->mfs, mfs_arr, mfs_count*sizeof(r_out_mf));
            memcpy (r_in_mf_arr->mfs, buf_mfs, mfs_count*sizeof(r_out_mf)); 
          }
          lout_arrs += 2;
          free(r_out_mf);
          free(buf_mfs);
          free(insc_arr);
        }
        lin_arrs += 2;
      }
    }
    rule_nums_in_pre = link_in_rule_file->links[i].rule_nums;
  }

  struct Tri_arr *tmp = xmalloc(sizeof(uint32_t)+nTris*sizeof(struct matrix_Tri_express *));
  tmp->nTris = nTris;
  for (uint32_t i = 0; i < nTris; i++){
    tmp->arr[i] = Tri_arr[i];
  }
  return tmp;
}


struct matrix_CSR *  //通过对链路文件查找两个同链路的头尾端规则，计算是否连通并添加到矩阵
gen_sparse_matrix(void) {
  struct Tri_arr *Tri_arr = gen_Tri_arr();
  // print_Tri_express(Tri_arr[2000]);
  // printf("all num = %d\n", nTris);
  // struct matrix_CSR *tmp = gen_matrix_CSR_from_Tris(Tri_arr);
  free(Tri_arr);
  // return tmp;
  return NULL;
}

struct matrix_CSC *
gen_CSC_from_CSR(struct matrix_CSR *matrix) {
  uint32_t valid_n = 0;
  for (uint32_t i = 0; i < matrix->nrows; i++) {
    if(matrix->rows[i])
      valid_n += matrix->rows[i]->nidx_vs;
  }
  // uint32_t max_CSR = MAX_VAL_RATE*data_allr_nums*data_allr_nums;
  struct matrix_Tri_express *Tri_arr[valid_n];
  // printf("valid_n%d\n", valid_n);
  // struct matrix_Tri_express *Tri_arr[valid_n];
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
  return tmp;
}


struct matrix_element * //a*b,a作用b，不可交换
elem_connect(struct matrix_element *a, struct matrix_element *b, uint32_t check) { 
  if (check){
    printf("there is elem_connect begin\n");
  }
  struct nf_space_pair *nps[30000];
  // printf("%d\n", a->npairs*b->npairs);
  uint32_t count = 0;
  for (uint32_t i = 0; i < a->npairs; i++) {
    // if (check){
    //   if (i > 4000){
    //     printf("%d begin\n", i);
    //     // printf("a->npairs %d\n", a->npairs);
    //   }
    // }
    struct nf_space_pair *np_a = a->nf_pairs[i];
    // if (check){
    //   if (i > 1476)
    //     printf("%d np_a\n", i);
    // }

    for (uint32_t j = 0; j < b->npairs; j++) {
      // if (check){
      //   if (i == 1476){
      //     printf("j = %d test begin\n", j);
      //     // if(j == 27){
      //     //   printf("j = 27 test begin\n");
      //     //   printf("b->npairs %d\n", b->npairs);
      //     // }
      //   }
      // }
      struct nf_space_pair *result = nf_space_connect(np_a, b->nf_pairs[j]);
      // if (check){
      //   if (i == 1476){
      //     if(j == 1){
      //       printf("end test\n");
      //       printf("count %d,\n", count);
      //     }

      //   }
      // }
      if (result) {
        nps[count] = result;
        count++;
      }
      
    }
    // if (check){
    //   if (i > 1476){
    //     printf("%d end\n", i);
    //   }
    // }
  }
  if (check){
    printf("elem_connect end for\n");
  }
  // printf("count%d\n", count);
  struct matrix_element *tmp = NULL;
  if (count) {
    tmp = xmalloc(sizeof(struct matrix_element) + count*sizeof(struct nf_space_pair *));
    tmp->npairs = count;
    for (int i = 0; i < count; i++) {
      tmp->nf_pairs[i] = nps[i]; 
    }
  }
  return tmp;
}

struct matrix_element *
row_col_multiply(struct CS_matrix_idx_v_arr *row, struct CS_matrix_idx_v_arr *col, uint32_t check) {
  if (check){
    printf("there is row_col begin\n");
  }
  uint32_t num_row = row->nidx_vs;
  uint32_t num_col = col->nidx_vs;
  uint32_t check_1 = 0;
  if (check){
    printf("num_row %d, num_col %d\n", num_row, num_col);
  }
  if ((row->idx_vs[0]->idx > col->idx_vs[num_col-1]->idx)||(row->idx_vs[num_row-1]->idx < col->idx_vs[0]->idx)) 
    return NULL;
  if (check){
    printf("there is row_col check\n");
  }
  uint32_t count_row = 0, count_col = 0;
  struct matrix_element *tmp = NULL;
  struct matrix_element *elem_tmp = NULL;
  for (uint32_t i = 0; i < num_row + num_col; i++) {
    if ((row->idx_vs[count_row]->idx) == (col->idx_vs[count_col]->idx)){
      if (check){
        if (i == 7)
          check_1 = 1;
      }
      elem_tmp = elem_connect(row->idx_vs[count_row]->elem, col->idx_vs[count_col]->elem, check_1);
      tmp = matrix_elem_plus(tmp, elem_tmp);
      elem_tmp = NULL;
      // tmp = elem_tmp;
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
  if (check){
    printf("after for\n");
  }

  if (check){
    printf("after free\n");
  }
  return tmp;
}

struct CS_matrix_idx_v_arr *
row_multi_col_multiply(struct CS_matrix_idx_v_arr *row, uint32_t *arr, uint32_t count, struct matrix_CSC *matrix_CSC, uint32_t check) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;
  
  for (uint32_t i = 0; i < count; i++){
    struct matrix_element *elem_tmp = row_col_multiply(row, matrix_CSC->cols[arr[i]], check);
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
row_all_col_multiply(struct CS_matrix_idx_v_arr *row, struct matrix_CSC *matrix_CSC, uint32_t check) {
  struct CS_matrix_idx_v_arr *tmp = NULL;
  struct CS_matrix_idx_v *vs[data_allr_nums];
  uint32_t vs_count = 0;
  uint32_t check_1 = 0;
  if (check){
    printf("row_all_col begin\n");
  }
  for (uint32_t i = 0; i < matrix_CSC->ncols; i++){
    if ( matrix_CSC->cols[i]){
      if (check){
        // printf("i = %d\n", i);
        if (i == 3438){
          check_1 = 1;
        }
      }
      struct matrix_element *elem_tmp = row_col_multiply(row, matrix_CSC->cols[i], check_1);
      if (elem_tmp) {
        vs[vs_count] = xmalloc(sizeof (struct CS_matrix_idx_v *));
        vs[vs_count]->idx = i;
        vs[vs_count]->elem = elem_tmp;
        vs_count++;
      }
    }
  }
  if (check){
    printf("row_all_col end for\n");
  }
  if(vs_count){
    printf("%d\n", vs_count);
    tmp = xmalloc(sizeof(uint32_t) + vs_count*sizeof(struct CS_matrix_idx_v *));
    tmp->nidx_vs = vs_count;
    // memcpy (tmp->idx_vs, vs, vs_count*sizeof(struct CS_matrix_idx_v *)); 
    for (uint32_t i = 0; i < vs_count; i++)
      tmp->idx_vs[i] = vs[i];
  }
  //qsort

  return tmp;
}

struct matrix_CSR *
sparse_matrix_multiply(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC) {
  // uint32_t valid_n = 0;
  // for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
  //   if(matrix_CSR->rows[i])
  //     valid_n += matrix_CSR->rows[i]->nidx_vs;
  // }

  uint32_t threshold = matrix_CSR->nrows/600;
  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+data_allr_nums*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = data_allr_nums;
  for (uint32_t i = 0; i < data_allr_nums; i++){
    tmp->rows[i] = NULL;
  }


  for (uint32_t i = 0; i < matrix_CSR->nrows; i++) {
    if (matrix_CSR->rows[i]){
      // printf("%d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
      // tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC);
      
      if (matrix_CSR->rows[i]->nidx_vs < threshold) {
        printf("%d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);
        if (matrix_CSR->rows[i]->nidx_vs == 1){
          uint32_t idx = matrix_CSR->rows[i]->idx_vs[0]->idx;
          if (matrix_CSR->rows[idx]){
            printf("%d\n",matrix_CSR->rows[idx]->nidx_vs);
          } 
        }
        // uint32_t arr[threshold*matrix_CSR->nrows];
        uint32_t *arr = xmalloc((threshold*matrix_CSR->nrows)*sizeof(uint32_t));
        uint32_t count = 0;
        struct CS_matrix_idx_v_arr *row = matrix_CSR->rows[i];
        for (uint32_t j = 0; j < row->nidx_vs; j++) {
          uint32_t col_row = row->idx_vs[j]->idx;
          struct CS_matrix_idx_v_arr *row_tmp = matrix_CSR->rows[col_row];
          if (row_tmp){
            for (uint32_t k = 0; k < row_tmp->nidx_vs; k++){ 
              arr[count] = row_tmp->idx_vs[k]->idx;
              count++;
            }
          }
        }
        if (count){
          uint32_t count1 = 1;
          qsort(arr, count,sizeof (uint32_t), uint32_t_cmp);
          uint32_t arr1[matrix_CSR->nrows];
          arr1[0] = arr[0];
          for (uint32_t j = 1; j < count; j++){
            if (arr[j] != arr[j-1]) {
              arr1[count1] = arr[j];
              count1++;
            }
          }
          printf("%d\n", count1);
          
          tmp->rows[i] = row_multi_col_multiply(row, arr1, count1, matrix_CSC, 0);
        }
        free(arr);
      }
      else{
        printf("%d - %d \n", i, matrix_CSR->rows[i]->nidx_vs);

        uint32_t check = 0;
        if (i == 2875){
            check = 1;
          }   
        tmp->rows[i] = row_all_col_multiply(matrix_CSR->rows[i], matrix_CSC, check);
      }
    }
  }
  return tmp;
}

