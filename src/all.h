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

#if __x86_64 || __amd64 || _M_X64 //64位或32位
typedef uint64_t array_t; //8字节一字节8位,16位16进制
#else
typedef uint32_t array_t;
#endif

#define MAX_PREFIX 255
#define MAX_ARR_SIZE 255

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
  // #define ARR_PTR(T, ID) \
  // struct arr_ptr_ ## ID { int n; union { T a[sizeof (T *) / sizeof (T)]; T *p; } e; }
  /*##连接字符串的作用
  结构体就是arr_ptr_uint32_t{int n，}
  union联合体类似结构体struct，struct所有变量是“共存”的，union中是各变量是“互斥”的
  共用一个内存首地址，并且各种变量名都可以同时使用，操作也是共同生效，也就是只体现一个值，可以兼容不同类型
  不过这些“手段”之间却没法互相屏蔽——就好像数组+下标和指针+偏移一样
  在此中放数组或者指针e，n为数量*/
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
  uint16_t mf_w[MF_LEN];
};

struct PACKED nf_space {//匹配域和位置
  struct mf_uint16_t mf;
  // uint32_t nw_src, dl_src, dl_dst, dl_dst, dl_vlan, dl_vlan_pcp, tp_src , dl_type, nw_tos
  uint32_t nwcs;
  struct wc_uint16_t link_wcs[0];
};

struct PACKED nf_space_pair {
  struct nf_space *in;
  struct nf_space *out;
  uint16_t mask[MF_LEN];
  uint32_t nrs;
  uint32_t ridx[0];
};

// struct PACKED nf_s_pair {
//   struct nf_space nf_s_in;
//   struct nf_space nf_s_out; 
// };

struct PACKED of_rule {
  uint32_t idx;
  // struct nf_space modify;
  // struct nf_space_pair *nf_ses[0];
  struct wc_uint32_t match;
  uint32_t mask;
  struct wc_uint32_t rewrite;
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
  int elem_n;
  struct nf_space_pair *elems[0];
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

struct of_rule *
rule_get(struct sw *sw, const int idx) {
  struct of_rule *rule = &(sw->rules[idx]);
  return (rule);
}

struct of_rule *
rule_get_2idx(const uint32_t sw_idx, const uint32_t r_idx) {
  struct sw *sw = sw_get (sw_idx);
  struct of_rule *rule = &(sw->rules[r_idx]);
  return (rule);
}

struct links_of_rule *
rule_links_get_swidx(struct sw *sw, const uint32_t idx, const uint32_t sign) { 
  struct links_of_rule *ls;
  struct of_rule *rule = &(sw->rules[idx]);
  if (sign) {
    ls = (struct links_of_rule *)(rule->in_link + (char *)sw + sw->links_ofs);
  }
  else {
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
print_rule(const struct of_rule *rule) {
  int add_len = sizeof(uint16_t);
  printf("num:%d; ", rule->idx);

  printf("matchfield:");
  for (int i = 0; i < MF_LEN*add_len; i += add_len)
  {
    print_wc((uint16_t *)((uint8_t *)data_arrs+i+rule->match.w), (uint16_t *)((uint8_t *)data_arrs+i+rule->match.v));
  }
  printf("; ");
  if (rule->mask)
  {
    printf("mask:");
    for (int i = 0; i < MF_LEN*add_len; i += add_len)
    {
      print_mask((uint16_t *)((uint8_t *)(data_arrs+i+rule->mask)));
    }
    printf("; ");
    printf("modify:");
    for (int i = 0; i < MF_LEN*add_len; i += add_len)
    {
      print_wc((uint16_t *)((uint8_t *)data_arrs+i+rule->rewrite.w), (uint16_t *)((uint8_t *)data_arrs+i+rule->rewrite.v));
    }
    printf("; ");

  }
}

void
print_links_wc(const struct links_of_rule *ls) {
  for (int i = 0; i < ls->n; i++)
  { 
    printf("%d - %d", ls->links_wc[i].w, ls->links_wc[i].v);
    printf(", ");
  }
  printf("\n");
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

//data.c
void 
data_load (const char *name) {
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
data_unload (void) { 
  munmap (data_raw, data_size); 
  munmap (link_data_raw, link_data_size);
  munmap (link_in_rule_raw, link_in_rule_size);
  munmap (link_out_rule_raw, link_out_rule_size); 
}

uint32_t *
matrix_idx_init (void) { //返回buf，全为0,长度为规则数
  uint32_t swn = data_file->sws_num;
  uint32_t init_0 = 0;
  char *buf;
  size_t bufsz;
  uint32_t sum_r = 0;
  FILE *f = open_memstream (&buf, &bufsz);
  for (int i = 0; i < swn; i++) {//总的r数量
    struct sw *sw_t = sw_get(i);
    sum_r += sw_t->nrules;
  }
  for (int i = 0; i < sum_r; i++, init_0++) {
    fwrite (&init_0, sizeof init_0, 1, f);//初始化为0
  }
  fclose (f);
  // printf("total rules :%d\n", sum_r);
  // printf("total rules bufsz:%d\n", bufsz/4);

  return buf;
}

uint32_t *
matrix_init (void) { //生成空矩阵
  uint32_t swn = data_file->sws_num;
  uint32_t init_0 = 0;
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
  // int bufsz_check = 0; 
  for (int i = 0; i < sum_r; i++) {
    for (int j = 0; j < sum_r; j++) {
      fwrite (&init_0, sizeof init_0, 1, f);
    }
  }
  fclose (f);
  // printf("total rules :%d\n", sum_r);
  // printf("total rules bufsz:%d\n", bufsz);

  return buf;
}

struct mf_uint16_t_array {
  uint32_t n_mfs;
  struct mf_uint16_t  mfs[0];
};

struct mf_uint16_t *
calc_insc(struct mf_uint16_t *a, struct mf_uint16_t *b) {
  uint32_t noinsc_sign = 0;
  for (int i = 0; i < MF_LEN; i++){
    if ((~(a->mf_w[i] | b->mf_w[i]))&(a->mf_v[i] ^ b->mf_v[i])) {
      noinsc_sign = 1;
    }
  }
  if (noinsc_sign){
    return 0;
  }
  struct mf_uint16_t *mf_insc = xcalloc (1, sizeof *mf_insc);
  for (int i = 0; i < MF_LEN; i++){
    mf_insc->mf_w[i] = a->mf_w[i] & b->mf_w[i];
    mf_insc->mf_v[i] = (~b->mf_w[i])&b->mf_v[i] + (~a->mf_w[i])&b->mf_w[i]&a->mf_v[i];
  }
  return mf_insc;
}

struct mf_uint16_t_array *
calc_minus_insc(struct mf_uint16_t *a, struct mf_uint16_t *insc) {
  struct mf_uint16_t *pre = a;
  for (int i = 0; i < MF_LEN; i++) {
    uint16_t diff_field = a->mf_w[i] ^ insc->mf_w[i];
    uint16_t sign = 0x8000;
    for (int j = 0; j < 16; j++) {
      uint16_t diff_sign = diff_ field & sign;
      if (diff_sign)

      sign >>= 1;
    }

  }

}



static int
link_idx_cmp (const void *a, const void *b) 
{ return *(uint32_t *)a - *(uint32_t *)b;}

struct link_to_rule * //通过idx查找link
get_link_rules(struct link_to_rule_file *lr_file, uint32_t rule_nums, const uint32_t idx) { 

  uint32_t *links = (uint32_t *)lr_file->links;
  uint32_t *b = (uint32_t *)bsearch(&idx, links, lr_file->swl_num, 2*sizeof(uint32_t), link_idx_cmp);
  if (b){
    // printf("%d - %d, ", *b, *(b+1));
    if (*b)
      rule_nums = *(b+1) - *(b-1);
    else
      rule_nums = *(b+1);
    // printf("%d $$$ ", rule_nums);
    return  (struct link_to_rule *)b;
  }
  else 
    return NULL;
}

void //uint32_t *
gen_matrix(uint32_t *matrix_buf) {
  // for (int i = 0; i < link_in_rule_file->swl_num; i++) {
  //   struct link *l = link_get(link_in_rule_file->links[i].link_idx);
  //   print_link(l);
  // }
  printf("%d - %d - %d\n", link_out_rule_file->swl_num, link_in_rule_file->swl_num, link_data_file->swl_num);
  // uint32_t rule_nums = 0;
  // get_link_rules(link_out_rule_file, rule_nums, 0);

  uint32_t rule_nums_in_pre = 0;
  for (int i = 0; i < link_in_rule_file->swl_num; i++) {
    uint32_t rule_nums_out = 0;
    uint32_t rule_nums_in  = link_in_rule_file->links[i].rule_nums - rule_nums_in_pre;
    uint32_t *lin_arrs = (uint32_t *)(link_in_rule_data_arrs + 2*rule_nums_in_pre);
    struct link_to_rule *lout_r = get_link_rules(link_in_rule_file, rule_nums_out, link_out_rule_file->links[i].link_idx);
    uint32_t rule_nums_out_pre = lout_r->rule_nums - rule_nums_out;
    uint32_t *lout_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_out_pre);
    if (lout_r) {
      for (int i_in = 0; i < rule_nums_in; i++) {  
        struct of_rule *r_in = rule_get_2idx(*(uint32_t *)lin_arrs, *(uint32_t *)(lin_arrs+1));
        for (int i_out = 0; i < rule_nums_out; i++) {
          struct of_rule *r_out = rule_get_2idx(*(uint32_t *)lout_arrs, *(uint32_t *)(lout_arrs+1));


        }
      }
    }
    
      
    // arrs += 2;

    rule_nums_in_pre = link_in_rule_file->links[i].rule_nums;
  }
}

//hs.c
static void
vec_destroy (struct hs_vec *v) {
  for (int i = 0; i < v->used; i++) {
    free (v->elems[i]);
    if (v->diff) vec_destroy (&v->diff[i]);
  }
  free (v->elems);
  free (v->diff);
}

void
hs_destroy (struct hs *hs)
{ vec_destroy (&hs->list); }

void
hs_free (struct hs *hs) {
  hs_destroy (hs);
  free (hs);
}

//parse.c
//res.c
struct res *
res_create (int nrules) {
  struct res *res = xcalloc (1, sizeof *res + nrules * sizeof *res->rules.arr);
  res->rules.n = nrules;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
  pthread_mutex_init (&res->lock, &attr);
  return res;
}

void 
res_free (struct res *res) {
  if (res->refs) { res->next = NULL; return; }

  hs_destroy (&res->hs);
  pthread_mutex_destroy (&res->lock);
  struct res *parent = res->parent;
  free (res);
  if (parent) { parent->refs--; res_free (parent); }
}
//ntf.c
//app.c






