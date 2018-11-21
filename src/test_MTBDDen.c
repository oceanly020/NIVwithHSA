#define _GNU_SOURCE 1

// #include "all.h"
#include "all_MTBDD.h"
// #include "app.h"
// #include "data.h"
#include <libgen.h>
#include <linux/limits.h>

#include <unistd.h>
// #include "ntf.h"



#ifndef NTF_STAGES
#define NTF_STAGES 1
#endif
#define MAX_REFIX #255




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
counter_init(void) {
  computation_counter = 0;
  elemconnet_counter = 0;
  compu_true_counter = 0;
  elem_true_counter = 0;
  time_counter_elembdd_withpair = 0;
  time_counter_elemplus = 0;
  time_counter_nf_space_connect = 0;
  time_counter_eleminsc = 0;
  global_sign = 0;
  time_counter1 = 0;
  time_counter2 = 0;
  time_counter3 = 0;
  time_counter4 = 0;
  time_counter5 = 0;
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
  // char *net = "stanford";
  // char *net = "stanford_32";
  char *net = "stanford_whole";
  // char *net = "i2";
  // bool one_step = false;
  load (net);
  // struct sw *sw0 = sw_get(0);
  // printf("sw%d\n", sw0->sw_idx);
  // struct of_rule *rule1 = rule_get(sw0, 22);
  // print_rule(rule1);
  // counter_init();

  // struct links_of_rule *ls = rule_links_get_swidx(sw0, 2, RULE_LINK_IN);
  // print_links_wc (ls);

  // 加载link数据
  char *linkdir = "../data/";
  link_data_load (linkdir);
  matrix_idx = matrix_idx_init();
  // gettimeofday(&start,NULL);
  // struct matrix_buf *matrix_buf = matrix_init();
  // struct sw *sw0 = sw_get(0);
  // struct of_rule *rule1 = rule_get(sw0, 22);
  // print_rule(rule1);
  struct switch_rs *swr0 = gen_sw_rules(14);
  struct ex_rule *ex_rule1 = ex_rule_get(swr0, 22);
  // print_ex_rule(ex_rule1);

  //made trie and updating rules
  /*==============================================================================*/
  struct trie_node *trie_root = crate_trie_node_init();
  gettimeofday(&start,NULL);
  trie_add_rules_for_sw(trie_root, swr0);
  gettimeofday(&stop,NULL);
  long long int build_trie = diff(&stop, &start);
  printf("build trie from sw0: %lld us\n", build_trie);

  // struct trie_node *tn = get_terminal_by_r(root, ex_rule1);
  // print_trie_node(tn);

  //made MTBDD(muli-terminal) and updating rules
  /*==============================================================================*/
  bdd_init(100000000,10000000,100000);

  bdd_setvarnum(16*MF_LEN);



  // printf("level of 0 is %d\n", bddnodes[0].level);
  gettimeofday(&start,NULL);
  BDD ex_rule_bdd = ex_rule_to_BDD(ex_rule1);
  gettimeofday(&stop,NULL);
  long long int T_rule_to_BDD = diff(&stop, &start);
  printf("get BDD from a rule: %lld us\n", T_rule_to_BDD);

  gettimeofday(&start,NULL);
  struct bdd_arrs *arr = get_bdds_by_sw(swr0);
  gettimeofday(&stop,NULL);
  long long int sw0_to_BDDs = diff(&stop, &start);
  printf("get BDD arrs from sw: %lld us\n", sw0_to_BDDs);

  // rule_record_cmp()
  // bdd_printtable(arr->bdds[10]);

  gettimeofday(&start,NULL);
  BDD mtbdd_root = mtbdd_add_rules_fr_arrs(arr);
  gettimeofday(&stop,NULL);
  long long int sw0_to_mtbdd = diff(&stop, &start);
  printf("get MTBDD from sw: %lld us\n", sw0_to_mtbdd);

  bdd_addref(mtbdd_root);
  bdd_gbc();
  printf("bdd_getnodenum :%d\n", bdd_getnodenum()); 

  // printf("bdd_getnodenum :%d\n", bdd_getnodenum()); 

  bdd_done();





  data_unload();
  free(matrix_idx);  
  return 0;
}

