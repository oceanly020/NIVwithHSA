#define _GNU_SOURCE 1

#include "all.h"
// #include "all_BDD.h"
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

void
average_v_matrix(int x, struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC ) {
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
  struct timeval start,stop;
  long long int average = 0;
  for (int i = 0; i < x; i++) {
    int rand_num = 0;
    for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
      if (matrix_CSR->rows[rand_num]) {
        break;
      }
    }
    

    gettimeofday(&start,NULL); 
    struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);


    if (muti1_idx_v_arr)
      muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);

    if (muti1_idx_v_arr)
      muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
    
    gettimeofday(&stop,NULL);
    printf("%ld us; ", diff(&stop, &start));


    printf("\n");
    average += diff(&stop, &start);
  }
  average = average / x;
  printf("average vector multiply matrix: %lld us\n", average);
  bdd_done();
}

void
average_v_matrix_all(int x, struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC, struct matrix_CSC *matrix_CSC1, struct matrix_CSC *matrix_CSC2) {
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
  struct timeval start,stop;
  long long int average1 = 0;
  long long int average2 = 0;
  long long int average3 = 0;
  for (int i = 0; i < x; i++) {
    int rand_num = 0;
    // for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
    //   if (matrix_CSR->rows[rand_num]) {
    //     break;
    //   }
    // }
    rand_num = (rand() % 3000) + 1;
    if (matrix_CSR->rows[rand_num]){
      gettimeofday(&start,NULL); 
      struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      gettimeofday(&stop,NULL);
      printf("1-1:%ld us; ", diff(&stop, &start));
      average1 += diff(&stop, &start);
      average2 += diff(&stop, &start);
      average3 += diff(&stop, &start);

      struct CS_matrix_idx_v_arr *muti2_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      struct CS_matrix_idx_v_arr *muti3_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      bdd_gbc();
      if (muti2_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
        gettimeofday(&stop,NULL);
        printf("2-2:%ld us; ", diff(&stop, &start));
        average2 += diff(&stop, &start);
      }
      bdd_gbc();
      if (matrix_CSC1) {
        if (muti3_idx_v_arr) {
          gettimeofday(&start,NULL);
          struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(muti3_idx_v_arr, matrix_CSC1);
          gettimeofday(&stop,NULL);
          printf("3-2:%ld us; ", diff(&stop, &start));
          average3 += diff(&stop, &start);
        }
      }
      bdd_gbc();

      if (muti2_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
        gettimeofday(&stop,NULL);
        printf("2-3:%ld us; ", diff(&stop, &start));
        average2 += diff(&stop, &start);
      }
      bdd_gbc();
      if (matrix_CSC2) {
        if (muti3_idx_v_arr) {
          gettimeofday(&start,NULL);
          struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(muti3_idx_v_arr, matrix_CSC2);
          gettimeofday(&stop,NULL);
          printf("3-3:%ld us; ", diff(&stop, &start));
          average3 += diff(&stop, &start);
        }
      
      }
      bdd_gbc();
      
    }
    else
      printf("1-1:0\n");
    printf("\n");

  }
  average1 = average1 / x;
  average2 = average2 / x;
  average3 = average3 / x;
  printf("average vector multiply matrix_1: %lld us\n", average1);
  printf("average vector multiply matrix_2: %lld us\n", average2);
  printf("average vector multiply matrix_3: %lld us\n", average3);
  bdd_done();
}

void
average_v_matrix_forall(struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC, struct matrix_CSC *matrix_CSC1, struct matrix_CSC *matrix_CSC2) {
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
  struct timeval start,stop;
  long long int average1 = 0;
  long long int average2 = 0;
  long long int average3 = 0;
  for (int i = 0; i < matrix_CSR->nrows; i++) {
    int rand_num = 0;
    // for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
    //   if (matrix_CSR->rows[rand_num]) {
    //     break;
    //   }
    // }
    rand_num = (rand() % 3000) + 1;
    if (matrix_CSR->rows[rand_num]){
      gettimeofday(&start,NULL); 
      struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      gettimeofday(&stop,NULL);
      printf("1-1:%ld us; ", diff(&stop, &start));
      average1 += diff(&stop, &start);
      average2 += diff(&stop, &start);
      average3 += diff(&stop, &start);

      struct CS_matrix_idx_v_arr *muti2_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      struct CS_matrix_idx_v_arr *muti3_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      bdd_gbc();
      if (muti2_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
        gettimeofday(&stop,NULL);
        printf("2-2:%ld us; ", diff(&stop, &start));
        average2 += diff(&stop, &start);
      }
      bdd_gbc();
      if (matrix_CSC1) {
        if (muti3_idx_v_arr) {
          gettimeofday(&start,NULL);
          struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(muti3_idx_v_arr, matrix_CSC1);
          gettimeofday(&stop,NULL);
          printf("3-2:%ld us; ", diff(&stop, &start));
          average3 += diff(&stop, &start);
        }
      }
      bdd_gbc();

      if (muti2_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
        gettimeofday(&stop,NULL);
        printf("2-3:%ld us; ", diff(&stop, &start));
        average2 += diff(&stop, &start);
      }
      bdd_gbc();
      if (matrix_CSC2) {
        if (muti3_idx_v_arr) {
          gettimeofday(&start,NULL);
          struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(muti3_idx_v_arr, matrix_CSC2);
          gettimeofday(&stop,NULL);
          printf("3-3:%ld us; ", diff(&stop, &start));
          average3 += diff(&stop, &start);
        }
      
      }
      bdd_gbc();
      
    }
    else
      printf("1-1:0\n");
    printf("\n");

  }
  average1 = average1 / matrix_CSR->nrows;
  average2 = average2 / matrix_CSR->nrows;
  average3 = average3 / matrix_CSR->nrows;
  printf("average vector multiply matrix_1: %lld us\n", average1);
  printf("average vector multiply matrix_2: %lld us\n", average2);
  printf("average vector multiply matrix_3: %lld us\n", average3);
  bdd_done();
}
 
void
average_v_matrix_1t(int x, struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC ) {
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
  struct timeval start,stop;
  long long int average = 0;
  for (int i = 0; i < x; i++) {
    int rand_num = 0;
    for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
      if (matrix_CSR->rows[rand_num]) {
        break;
      }
    }
    

    gettimeofday(&start,NULL); 
    struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
    // if (muti1_idx_v_arr)
    //   muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
    // if (muti1_idx_v_arr)
    //   muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
    
    gettimeofday(&stop,NULL);
    printf("%ld us; ", diff(&stop, &start));
    printf("\n");
    average += diff(&stop, &start);
  }
  average = average / x;
  printf("average vector multiply matrix 1t: %lld us\n", average);
  bdd_done();
}
void
average_v_matrix_Rc(int x, struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC, struct matrix_CSC *matrix_CSC1, struct matrix_CSC *matrix_CSC2) {
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
  struct timeval start,stop;
  long long int average = 0;
  for (int i = 0; i < x; i++) {
    int rand_num = 0;
    for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
      if (matrix_CSR->rows[rand_num]) {
        break;
      }
    }
    

    gettimeofday(&start,NULL); 
    struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
    if (matrix_CSC1) {
      if (muti1_idx_v_arr)
        muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC1);
    }
    if (matrix_CSC2) {
      if (muti1_idx_v_arr)
        muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC2);
    }
    gettimeofday(&stop,NULL);
    printf("%ld us; ", diff(&stop, &start));
    printf("\n");
    average += diff(&stop, &start);
  }
  average = average / x;
  printf("average vector multiply matrix: %lld us\n", average);
  bdd_done();
}

void
link_matrix(int x, struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC) {
  bdd_init(BDDSIZE, BDDOPCHCHE);
  bdd_setvarnum(16*MF_LEN);
  struct timeval start,stop;
  long long int average = 0;
  for (int i = 0; i < x; i++) {
    int rand_num = 0;
    for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
      if (matrix_CSR->rows[rand_num]) {
        break;
      }
    }
    

    gettimeofday(&start,NULL); 
    struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
    gettimeofday(&stop,NULL);
    printf("%ld us; ", diff(&stop, &start));
    printf("\n");
    average += diff(&stop, &start);
  }
  average = average / x;
  printf("average vector multiply matrix: %lld us\n", average);
  bdd_done();
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
  computation_firstcounter = 0;
  global_sign = 0;
  time_counter1 = 0;
  time_counter2 = 0;
  time_counter3 = 0;
}

void
print_counter(void){
  printf("computation_firstcounter = %d\n", elemconnet_counter);
  printf("computation_counter = %d\n", computation_counter);
  printf("compu_true_counter = %d\n", compu_true_counter);
  printf("elemconnet_counter = %d\n", elemconnet_counter);
  printf("time_counter1 = %ld us\n", time_counter1);
  printf("time_counter2 = %ld us\n", time_counter2);
  printf("time_counter3 = %ld us\n", time_counter3);
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
  counter_init();

  // struct links_of_rule *ls = rule_links_get_swidx(sw0, 2, RULE_LINK_IN);
  // print_links_wc (ls);

  // 加载link数据
  char *linkdir = "../data/";
  link_data_load (linkdir);
  // gettimeofday(&start,NULL);
  // struct matrix_buf *matrix_buf = matrix_init();
  matrix_idx = matrix_idx_init ();


  // struct u32_arrs *rs_idx_frport = get_outrules_idx_from_inport(100002);
  // print_u32_arrs(rs_idx_frport);
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

/*================================生成稀疏矩阵======================================*/
  // gettimeofday(&start,NULL);
  // struct Tri_arr *Tri_arr = gen_Tri_arr_bdd();
  // gettimeofday(&stop,NULL);
  // long long int gen_Tri_arr = diff(&stop, &start)/1000;
  // printf("gen Tri_arr: %lld ms\n", gen_Tri_arr);

  // BDD_init_multiply();

  gettimeofday(&start,NULL);
  struct matrix_CSR *matrix_CSR = gen_sparse_matrix(); 
  gettimeofday(&stop,NULL);
  long long int gen_CSR = diff(&stop, &start)/1000;
  printf("gen CSR: %lld ms\n", gen_CSR);
  print_vElemsNUM_of_Matrix_CSR(matrix_CSR);
  print_npairsNUM_of_Matrix_CSR(matrix_CSR);
  printf("================================================================\n");


  // gettimeofday(&start,NULL);
  // struct CS_matrix_idx_v_arr *port_CSR_row = gen_sparse_matrix_row_fr_port(300003); 
  // // for (int i = 0; i < port_CSR_row->nidx_vs; i++)
  // //   printf("%d;", port_CSR_row->idx_vs[i]->idx);
  // // printf("\n");
  // gettimeofday(&stop,NULL);
  // long long int gen_port_CSR_row = diff(&stop, &start)/1000;


  // printf("gen CSR_row from inport: %lld ms\n", gen_port_CSR_row);
  // print_CS_matrix_idx_v_arr(port_CSR_row);
  // printf("matrix idx of %d-%d: %d\n", 1, 0, matrix_idx_get_2idx(1, 0));

  // data_unload();

  gettimeofday(&start,NULL);
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  gettimeofday(&stop,NULL);
  long long int gen_CSC = diff(&stop, &start)/1000;
  printf("gen CSC: %lld ms\n", gen_CSC);
  printf("================================================================\n");
  // print_CSR_elem_from_idx(2940,200,matrix_CSR);
  // print_CSC_elem_from_idx(2940,200,matrix_CSC);



  

/*================================Selected_rs 对矩阵的计算======================================*/
  // BDD_init_matrix_multiply();
  // gettimeofday(&start,NULL); 
  // struct matrix_CSR *muti_rs_CSR = selected_rs_matrix_multiply(matrix_CSR, matrix_CSC, rs_idx_frport);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_muti_rs = diff(&stop, &start);
  // printf("rs multi matrix: %lld us\n", T_muti_rs);

  // gettimeofday(&start,NULL); 
  // struct matrix_CSR *muti_rs_m_CSR = sparse_matrix_multiply_nsqure(muti_rs_CSR, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_muti_rs_m = diff(&stop, &start);
  // printf("rs_matrix multi matrix 1: %lld us\n", T_muti_rs_m);

  // gettimeofday(&start,NULL); 
  // struct matrix_CSR *muti_rs_m1_CSR = sparse_matrix_multiply_nsqure(muti_rs_m_CSR, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_muti_rs_m1 = diff(&stop, &start);
  // printf("rs_matrix multi matrix 2: %lld us\n", T_muti_rs_m1);

  // print_CS_matrix_v_arr(port_CSR_row);
/*================================port生成的向量 对矩阵的计算======================================*/
  // print_CS_matrix_idx_v_arr(port_CSR_row);
  // print_bdd_saved_arr(port_CSR_row->idx_vs[0]->elem->nf_pairs[0]->in->mf);


  // BDD_init_multiply();
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row1 = vec_matrix_multiply(port_CSR_row, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row1 = diff(&stop, &start);
  // // print_matrix_element(port_CSR_row1->idx_vs[0]->elem);
  // printf("port->vs multi matrix 1t: %lld us; the len = %d\n", T_port_CSR_row1, port_CSR_row1->nidx_vs);
  // // for (int i = 0; i < port_CSR_row1->nidx_vs; i++)
  // //   printf("%d;", port_CSR_row1->idx_vs[i]->idx);
  // // printf("\n");
  // print_counter();
  // counter_init();
  // printf("================================================================\n");


  // // print_CS_matrix_idx_v_arr(port_CSR_row1);
  // // print_bdd_saved_arr(port_CSR_row->idx_vs[0]->elem->nf_pairs[0]->in->mf);
  // print_matrix_element(port_CSR_row1->idx_vs[0]->elem);
  // uint32_t matrixidx = 881;
  // struct of_rule *r = matrix_idx_to_r(&matrixidx);
  // print_rule(r);


  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row2 = vec_matrix_multiply(port_CSR_row1, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row2 = diff(&stop, &start);
  // printf("port->vs multi matrix 2t: %lld us; the len = %d\n", T_port_CSR_row2, port_CSR_row2->nidx_vs);
  // for (int i = 0; i < port_CSR_row2->nidx_vs; i++)
  // {
  //   printf("%d;", port_CSR_row2->idx_vs[i]->idx);
  // }
  // printf("\n");
  // print_counter();
  // counter_init();
  // printf("================================================================\n");

  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_ot_row2 = row_matrix_CSR_multiply(port_CSR_row1, matrix_CSR);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_ot_row2 = diff(&stop, &start);
  // printf("port->vs multi matrix 2t_ot: %lld us; the len = %d\n", T_port_CSR_ot_row2, port_CSR_ot_row2->nidx_vs);
  // for (int i = 0; i < port_CSR_ot_row2->nidx_vs; i++)
  // {
  //   printf("%d;", port_CSR_ot_row2->idx_vs[i]->idx);
  // }
  // printf("\n");
  // print_counter();
  // counter_init();
  // printf("================================================================\n");
  // // print_matrix_element(port_CSR_row2->idx_vs[0]->elem);

  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row3 = vec_matrix_multiply(port_CSR_row2, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row3 = diff(&stop, &start);
  // print_counter();
  // counter_init();
  // printf("port->vs multi matrix 3t: %lld us; the len = %d\n", T_port_CSR_row3, port_CSR_row3->nidx_vs);
  // print_matrix_element(port_CSR_row3->idx_vs[0]->elem);

  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_ot_row3 = row_matrix_CSR_multiply(port_CSR_ot_row2, matrix_CSR);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_ot_row3 = diff(&stop, &start);
  // if (port_CSR_ot_row3)
  //   printf("port->vs multi matrix 3t_ot: %lld us; the len = %d\n", T_port_CSR_ot_row3, port_CSR_ot_row3->nidx_vs);
  // else
  //   printf("port->vs multi matrix 3t_ot: %lld us; the len = 0\n", T_port_CSR_ot_row3);
  
  // print_counter();
  // counter_init();
  // printf("================================================================\n");

  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row4 = vec_matrix_multiply(port_CSR_row3, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row4 = diff(&stop, &start);
  // // print_matrix_element(port_CSR_row4->idx_vs[0]->elem);
  // printf("port->vs multi matrix 4t: %lld us; the len = %d\n", T_port_CSR_row4, port_CSR_row4->nidx_vs);
  // // print_CS_matrix_idx_v_arr(port_CSR_row4);\

  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row5 = vec_matrix_multiply(port_CSR_row4, matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row5 = diff(&stop, &start);
  // // print_matrix_element(port_CSR_row4->idx_vs[0]->elem);
  // printf("port->vs multi matrix 5t: %lld us; the len = %d\n", T_port_CSR_row5, port_CSR_row5->nidx_vs);

  // bdd_done();


/*================================矩阵 对矩阵的计算======================================*/

  gettimeofday(&start,NULL);
  struct matrix_CSR *muti1_CSR = sparse_matrix_multiply(matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre = diff(&stop, &start)/1000;
  printf("matrix squre: %lld ms\n", squre);
  print_vElemsNUM_of_Matrix_CSR(muti1_CSR);
  print_npairsNUM_of_Matrix_CSR(muti1_CSR);
  print_counter();
  counter_init();
  // struct matrix_CSR *muti1_CSR_compare = sparse_matrix_multiply(matrix_CSR, matrix_CSC);
  // print_vElemsNUM_of_Matrix_CSR(muti1_CSR_compare);
  // print_npairsNUM_of_Matrix_CSR(muti1_CSR_compare);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(matrix_CSR);
  // uint32_t r_idx = matrix_idx_get_2idx(14, 1885);
  // print_matrix_element(matrix_CSR->rows[r_idx]->idx_vs[0]->elem);
  // print_matrix_element(muti1_CSR->rows[r_idx]->idx_vs[0]->elem);

  
  // gettimeofday(&start,NULL);
  // struct matrix_CSC *muti1_CSC = gen_CSC_from_CSR(muti1_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre_CSC = diff(&stop, &start)/1000;
  // printf("matrix squre->CSC: %lld ms\n", squre_CSC);
  // print_vElemsNUM_of_Matrix_CSC(muti1_CSC);
  printf("================================================================\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *other_CSR = sparse_matrix_multiply_otway(matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int other_squre = diff(&stop, &start)/1000;
  // printf("other_CSRmatrix squre: %lld ms\n", squre);
  // print_counter();
  // counter_init();
  // printf("================================================================\n");


  
  

  // BDD_init_multiply();
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti1 = vec_matrix_multiply(port_CSR_row, muti1_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti1 = diff(&stop, &start);
  // printf("port->vs multi matrix muti1: %lld us; the len = %d\n", T_port_CSR_row_muti1, port_CSR_row_muti1->nidx_vs);
  // // print_matrix_element(port_CSR_row_muti1->idx_vs[0]->elem);
  // print_counter();
  // counter_init();
  // bdd_done();

  // print_CS_matrix_v_arr(matrix_CSR->rows[0]);
  // uint32_t matrixidx = 2265;
  // print_CS_matrix_v_arr(matrix_CSR->rows[matrixidx]);
  // struct of_rule *r = matrix_idx_to_r(&matrixidx);
  // print_rule(r);
  // matrixidx = 3344;
  // r = matrix_idx_to_r(&matrixidx);
  // print_rule(r);
  // print_CS_matrix_v_arr(muti1_CSR->rows[0]);

  

  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[3724], matrix_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int v_m_multiply1 = diff(&stop, &start);


  // gettimeofday(&start,NULL); 
  // muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[3736], matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int v_m_multiply2 = diff(&stop, &start);

  gettimeofday(&start,NULL);
  struct matrix_CSR *muti2_CSR = sparse_matrix_multiply(muti1_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre2 = diff(&stop, &start)/1000;
  printf("matrix squre2: %lld ms\n", squre2);
  print_vElemsNUM_of_Matrix_CSR(muti2_CSR);
  print_npairsNUM_of_Matrix_CSR(muti2_CSR);
  print_counter();
  counter_init(); 
  // struct matrix_CSR *muti2_CSR_compare = sparse_matrix_multiply(muti1_CSR_compare, matrix_CSC);
  // print_vElemsNUM_of_Matrix_CSR(muti2_CSR_compare);
  // print_npairsNUM_of_Matrix_CSR(muti2_CSR_compare);
  // print_counter();
  // counter_init();


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti2_r_CSR = sparse_matrix_multiply(matrix_CSR, muti1_CSC);
  // gettimeofday(&stop,NULL);
  // long long int squre2_r = diff(&stop, &start)/1000;
  // printf("matrix squre2_r: %lld ms\n", squre2_r);
  // print_vElemsNUM_of_Matrix_CSR(muti2_r_CSR);
  // print_counter();
  // counter_init();
  free_matrix_CSR(muti1_CSR);
  printf("================================================================\n");

  

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti2_CSC = gen_CSC_from_CSR(muti2_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre2_CSC = diff(&stop, &start)/1000;
  // printf("matrix squre2->CSC: %lld ms\n", squre2_CSC);
  // print_vElemsNUM_of_Matrix_CSC(muti2_CSC);



  // BDD_init_multiply();
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti2 = vec_matrix_multiply(port_CSR_row, muti2_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti2 = diff(&stop, &start);
  // printf("port->vs multi matrix muti2: %lld us; the len = %d\n", T_port_CSR_row_muti2, port_CSR_row_muti2->nidx_vs);
  // bdd_done();

  

  gettimeofday(&start,NULL);
  struct matrix_CSR *muti3_CSR = sparse_matrix_multiply(muti2_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre3 = diff(&stop, &start)/1000;
  printf("matrix squre3: %lld ms\n", squre3);
  print_vElemsNUM_of_Matrix_CSR(muti3_CSR);
  print_counter();
  counter_init();
  // free_matrix_CSR(muti3_CSR);
  

  
  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti3_r_CSR = sparse_matrix_multiply(matrix_CSR, muti2_CSC);
  // gettimeofday(&stop,NULL);
  // long long int squre3_r = diff(&stop, &start)/1000;
  // printf("matrix squre3_r: %lld ms\n", squre3_r);
  // print_vElemsNUM_of_Matrix_CSR(muti3_r_CSR);
  // print_counter();
  // counter_init();
  free_matrix_CSR(muti2_CSR);
  printf("================================================================\n");





  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti3_CSC = gen_CSC_from_CSR(muti3_r_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre3_CSC = diff(&stop, &start)/1000;
  // printf("matrix squre3->CSC: %lld ms\n", squre3_CSC);
  // print_vElemsNUM_of_Matrix_CSC(muti3_CSC);





  gettimeofday(&start,NULL);
  struct matrix_CSR *muti4_CSR = sparse_matrix_multiply(muti3_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre4 = diff(&stop, &start)/1000;
  printf("matrix squre4: %lld ms\n", squre4);
  print_vElemsNUM_of_Matrix_CSR(muti4_CSR);
  print_counter();
  counter_init();
  free_matrix_CSR(muti3_CSR);
  printf("================================================================\n");

  data_unload();

  gettimeofday(&start,NULL);
  struct matrix_CSR *muti5_CSR = sparse_matrix_multiply(muti4_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre5 = diff(&stop, &start)/1000;
  printf("matrix squre4: %lld ms\n", squre5);
  print_vElemsNUM_of_Matrix_CSR(muti5_CSR);
  print_counter();
  counter_init();
  free_matrix_CSR(muti4_CSR);
  printf("================================================================\n");
  gettimeofday(&start,NULL);
  struct matrix_CSR *muti6_CSR = sparse_matrix_multiply(muti5_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int squre6 = diff(&stop, &start)/1000;
  printf("matrix squre4: %lld ms\n", squre6);
  print_vElemsNUM_of_Matrix_CSR(muti6_CSR);
  print_counter();
  counter_init();
  free_matrix_CSR(muti5_CSR);
  printf("================================================================\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti5_CSR = sparse_matrix_multiply(muti1_CSR, muti1_CSC);
  // gettimeofday(&stop,NULL);
  // long long int squre5 = diff(&stop, &start)/1000;
  // printf("matrix squre3: %lld ms\n", squre5);



  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti4_r_CSR = sparse_matrix_multiply(matrix_CSR, muti3_CSC);
  // gettimeofday(&stop,NULL);
  // long long int squre4_r = diff(&stop, &start)/1000;
  // printf("matrix squre4_r: %lld ms\n", squre4_r);
  // free_matrix_CSR(muti3_r_CSR);
  // print_vElemsNUM_of_Matrix_CSR(muti4_r_CSR);
  // print_counter();
  // counter_init();
  // struct matrix_CSC *muti4_CSC = gen_CSC_from_CSR(muti4_r_CSR);



  // BDD_init_multiply();
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti4 = vec_matrix_multiply(port_CSR_row, muti4_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti4 = diff(&stop, &start);
  // printf("port->vs multi matrix muti4: %lld us; the len = %d\n", T_port_CSR_row_muti4, port_CSR_row_muti4->nidx_vs);
  // // print_matrix_element(port_CSR_row_muti1->idx_vs[0]->elem);
  // print_counter();
  // counter_init();
  // bdd_done();


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti5_r_CSR = sparse_matrix_multiply(matrix_CSR, muti4_CSC);
  // gettimeofday(&stop,NULL);
  // long long int squre5_r = diff(&stop, &start)/1000;
  // printf("matrix squre5_r: %lld ms\n", squre5_r);
  // free_matrix_CSR(muti4_r_CSR);
  // print_vElemsNUM_of_Matrix_CSR(muti5_r_CSR);
  // print_counter();
  // counter_init();
  
  // struct matrix_CSC *muti5_CSC = gen_CSC_from_CSR(muti5_r_CSR);


  // gettimeofday(&start,NULL);
  // muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[3736], muti2_CSC);
  // gettimeofday(&stop,NULL);
  // long long int v_m_multiply2_2 = diff(&stop, &start)/1000;

  // gettimeofday(&start,NULL);
  // muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[3724], muti2_CSC);
  // gettimeofday(&stop,NULL);
  // long long int v_m_multiply1_2 = diff(&stop, &start)/1000;

  
  


  // printf("matrix squre2: %lld ms\n", squre2); 
  // printf("matrix squre3: %lld ms\n", squre3);  
  // printf("matrix squre4: %lld ms\n", squre4);  
  // printf("vector multiply matrix: %lld us\n", v_m_multiply1);
  // printf("vector multiply matrix(little elem): %lld us\n", v_m_multiply2);
  // printf("vector multiply squre2matrix: %lld us\n", v_m_multiply1_2);
  // printf("vector multiply squre2matrix(little elem): %lld us\n", v_m_multiply2_2);
  // printf("matrix has value: %d\n", get_value_num_matrix_CSR(matrix_CSR)); 
  // printf("matrix squre has value: %d\n", get_value_num_matrix_CSR(muti1_CSR));  
  // printf("matrix squre2 has value: %d\n", get_value_num_matrix_CSR(muti2_CSR));  
  // printf("matrix squre3 has value: %d\n", get_value_num_matrix_CSR(muti3_CSR)); 
  // printf("matrix squre4 has value: %d\n", get_value_num_matrix_CSR(muti4_CSR));  
  // average_v_matrix_1t(500, matrix_CSR, matrix_CSC);
  // average_v_matrix(500, matrix_CSR, matrix_CSC);

  // average_v_matrix_forall(matrix_CSR, matrix_CSC, muti1_CSC, muti2_CSC);
  // average_v_matrix_forall(matrix_CSR, matrix_CSC, muti1_CSC, NULL);

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
  
  // free_matrix_CSR(matrix_CSR);
  // free_matrix_CSC_fr_CSR(matrix_CSC);
  // free_matrix_CSR(muti1_CSR);
  // bdd_done();
  free(matrix_idx);
  
  return 0;
}

