#define _GNU_SOURCE 1

#include "all.h"
// #include "app.h"
// #include "data.h"
#include <libgen.h>
#include <linux/limits.h>
#include <sys/time.h>
#include <unistd.h>
// #include "ntf.h"



#ifndef NTF_STAGES
#define NTF_STAGES 1
#endif
#define MAX_REFIX #255


static inline int64_t
diff (struct timeval *a, struct timeval *b)
{
  int64_t x = (int64_t)a->tv_sec * 1000000 + a->tv_usec;
  int64_t y = (int64_t)b->tv_sec * 1000000 + b->tv_usec;
  return x - y;
}

static void
unload (void)
{ data_unload (); }


static void
load (char *net)
{
  char name[PATH_MAX + 1];
  snprintf (name, sizeof name, "../data/%s.dat", net);            
  data_load (name);
  if (atexit (unload)) errx (1, "Failed to set exit handler.");
}

int
calc_set_test (void) {
  struct mf_uint16_t *a = xcalloc (1, sizeof *a);
  struct mf_uint16_t *b = xcalloc (1, sizeof *b);
  for (int i = 0; i < MF_LEN; i++) {
    a->mf_w[i] = 0x58d0;
    a->mf_v[i] = 0x8623;
    b->mf_w[i] = 0x6949;
    b->mf_v[i] = 0x8622;
  }

  printf("the oringinal:\n");
  print_mf_uint16_t(a);
  // print_mf_uint16_t(b);
  struct mf_uint16_t *insc = calc_insc(a, b);
  if (!insc){
      printf("not success\n");
      return -1;
    }
  printf("the insection:\n");
  print_mf_uint16_t(insc);
  struct mf_uint16_t_array *minus = calc_minus_insc(a, insc);
  printf("the minus set:\n");
  print_mf_uint16_t_array(minus);
  free(insc);
  free(minus);
  return 0;
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

void
test(void) {
  char *buf;
  size_t bufsz;
  FILE *f = open_memstream (&buf, &bufsz);
  char *str_arr = "this is the test";
  uint32_t str_len = strlen(str_arr);
  
  fwrite (str_arr, str_len, 1, f);
  fflush(f);
  // fclose(f);
  printf("%s\n", buf);
  buf[0] = 'o';
  buf[1] = 'o';
  printf("%s\n", buf);
  fwrite (str_arr, str_len, 1, f);
  fflush(f);
  printf("%s\n", buf);
  fclose(f);
  
  // fclose(f);
}

void
print_linktorule_test(void) {

  struct link *lk = link_get(link_out_rule_file->links[1].link_idx);
  print_link(lk);
  uint32_t rule_nums_in_pre = link_out_rule_file->links[0].rule_nums;
  uint32_t rule_nums_in  = link_out_rule_file->links[1].rule_nums - rule_nums_in_pre;
  uint32_t *lin_arrs = (uint32_t *)(link_out_rule_data_arrs + 2*rule_nums_in_pre);
  printf("%d", rule_nums_in);
  for (uint32_t i_in = 0; i_in < rule_nums_in; i_in++) {
    struct of_rule *r_in = rule_get_2idx(*(uint32_t *)lin_arrs, *(uint32_t *)(lin_arrs+1));
    print_rule(r_in);
    lin_arrs += 2;
  }
}

int
main (int argc, char **argv)
{
  struct timeval start,stop;  //计算时间差 usec
  // gettimeofday(&start,NULL);
  // gettimeofday(&stop,NULL);
  uint32_t *matrix_idx;
  // test();

  // 加载net数据
  char *net = "stanford";
  // char *net = "stanford_whole";
  // bool one_step = false;
  load (net);
  // struct sw *sw0 = sw_get(0);
  // printf("sw%d\n", sw0->sw_idx);
  // struct of_rule *rule1 = rule_get(sw0, 22);
  // print_rule(rule1);


  // struct links_of_rule *ls = rule_links_get_swidx(sw0, 2, RULE_LINK_IN);
  // print_links_wc (ls);

  // 加载link数据
  char *linkdir = "../data/";
  link_data_load (linkdir);
  // gettimeofday(&start,NULL);
  // struct matrix_buf *matrix_buf = matrix_init();
  matrix_idx = matrix_idx_init ();
 
  // calc_set_test();
  
  // 生成普通矩阵
  // gen_matrix(matrix_buf);
  // struct link *lk = link_get(00);
  // print_link(lk);
  // print_linktorule_test();
  // struct of_rule *r =  rule_get_2idx(0, 201);
  // print_rule(r);
  // r = rule_get_2idx(1, 201);
  // print_rule(r);

  // 生成稀疏矩阵
  gettimeofday(&start,NULL);
  struct matrix_CSR *matrix_CSR = gen_sparse_matrix(); 
  gettimeofday(&stop,NULL);
  long long int gen_CSR = diff(&stop, &start)/1000;
  data_unload();

  // printf("Staring matrix_CSC gen\n");
  gettimeofday(&start,NULL);
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  gettimeofday(&stop,NULL);
  long long int gen_CSC = diff(&stop, &start)/1000;
  

  // printf("matrix_CSC gen:%lld us", diff(&stop, &start));
  
  // print_CSR_elem_from_idx(2940,200,matrix_CSR);
  // print_CSC_elem_from_idx(2940,200,matrix_CSC);
  gettimeofday(&start,NULL); 
  struct matrix_CSR *muti1_CSR = sparse_matrix_multiply(matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre = diff(&stop, &start)/1000;
  // printf("matrix_buf squre:%lld us", diff(&stop, &start));
  
  
  // uint32_t row_idx = matrix_idx_get_2idx(9,108);
  // uint32_t col_idx = matrix_idx_get_2idx(0,1);
  // printf("row-col: %d - %d;", row_idx, col_idx);
  // struct of_rule *r1 = rule_get_2idx(9,108);
  // struct of_rule *r2 = rule_get_2idx(0,1);  
  // print_rule(r1);
  // print_rule(r2);
  // uint32_t sign = print_CSR_elem_from_idx(row_idx, col_idx, matrix_CSR);

  // gettimeofday(&start,NULL); 
  // struct matrix_buf *squre = matrix_multiply(matrix_buf,matrix_buf);
  // time_for_test();
  // gettimeofday(&stop,NULL);
  // printf("matrix_buf squre:%lld us", diff(&stop, &start));

  // struct link *lk = link_get(3);
  // print_link (lk);
  // lk = link_get(0);
  // print_link (lk);
  
  // struct hs hs;
  // hs.len = data_arrs_len;
  // // int hop_count = 0;
  // // int offset = 1;
  // // bool find_loop = false;

  // // printf ("%d \n %d \n", data_file->arrs_ofs, data_file->strs_ofs);

  // struct res *in = res_create (data_file->stages + 1);

  // // printf ("%s \n",data_strs);
  // res_free (in);
  printf("gen CSR: %lld ms\n", gen_CSR);
  printf("gen CSC: %lld ms\n", gen_CSC);
  printf("matrix squre: %lld ms\n", squre);
  free_matrix_CSR(matrix_CSR);
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti1_CSR);
  free(matrix_idx);
  
  return 0;
}

