#define _GNU_SOURCE 1

// #include "all.h"
#include "all_BDDM.h"
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
}

void
average_v_matrix_all(int x, struct matrix_CSR *matrix_CSR, struct matrix_CSC *matrix_CSC, struct matrix_CSC *matrix_CSC1, struct matrix_CSC *matrix_CSC2) {
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
      if (muti2_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
        gettimeofday(&stop,NULL);
        printf("2-2:%ld us; ", diff(&stop, &start));
        average2 += diff(&stop, &start);
      }
      if (matrix_CSC1) {
        if (muti3_idx_v_arr) {
          gettimeofday(&start,NULL);
          struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(muti3_idx_v_arr, matrix_CSC1);
          gettimeofday(&stop,NULL);
          printf("3-2:%ld us; ", diff(&stop, &start));
          average3 += diff(&stop, &start);
        }
      }

      if (muti2_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
        gettimeofday(&stop,NULL);
        printf("2-3:%ld us; ", diff(&stop, &start));
        average2 += diff(&stop, &start);
      }
      if (matrix_CSC2) {
        if (muti3_idx_v_arr) {
          gettimeofday(&start,NULL);
          struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(muti3_idx_v_arr, matrix_CSC2);
          gettimeofday(&stop,NULL);
          printf("3-3:%ld us; ", diff(&stop, &start));
          average3 += diff(&stop, &start);
        }
      
      }
      
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
}

void
average_v_matrix_forall(struct matrix_CSR *matrix_CSR, struct matrix_CSR *matrix_CSR1, struct matrix_CSR *matrix_CSR2) {
  struct timeval start,stop;
  long long int average1 = 0;
  long long int average2 = 0;
  long long int average3 = 0;
  long long int average1_col = 0;
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  struct matrix_CSC *matrix_CSC1 = gen_CSC_from_CSR(matrix_CSR1);
  struct matrix_CSC *matrix_CSC2 = gen_CSC_from_CSR(matrix_CSR2);
  // for (int i = 0; i < matrix_CSR->nrows; i++) {
  for (int i = 0; i < matrix_CSR->nrows; i++) {
    uint32_t rand_num = i;
    // for (rand_num = (rand() % 3000) + 1; rand_num < 3003; rand_num++) {
    //   if (matrix_CSR->rows[rand_num]) {
    //     break;
    //   }
    // }
    // int rand_num = (rand() % 8000) + 1;
    struct of_rule *r = matrix_idx_to_r(&rand_num);

    // printf("the rule: %d - %d;", r->sw_idx, r->idx);

    if (matrix_CSC->cols[rand_num]){
      gettimeofday(&start,NULL); 
      struct CS_matrix_idx_v_arr *muti1_col_idx_v_arr = all_row_col_multiply(matrix_CSR, matrix_CSC->cols[rand_num]);
      gettimeofday(&stop,NULL);
      // printf("1-1-col:%ld us; ", diff(&stop, &start));
      average1_col += diff(&stop, &start);
      // average2_col += diff(&stop, &start);
      // average3_col += diff(&stop, &start);
      if (muti1_col_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti1_col_idx_v_arr = all_row_col_multiply(matrix_CSR1, matrix_CSC->cols[rand_num]);
        gettimeofday(&stop,NULL);
        // printf("1-2-col:%ld us; ", diff(&stop, &start));
        average1_col += diff(&stop, &start);
      }

      if (muti1_col_idx_v_arr){
        gettimeofday(&start,NULL); 
        muti1_col_idx_v_arr = all_row_col_multiply(matrix_CSR2, matrix_CSC->cols[rand_num]);
        gettimeofday(&stop,NULL);
        // printf("1-3-col:%ld us; ", diff(&stop, &start));
        average1_col += diff(&stop, &start);
      }


    }
    // else
    //   printf("1-1-col:0");



    if (matrix_CSR->rows[rand_num]){
      // counter_init();
      if (matrix_CSR->rows[rand_num]->nidx_vs > 30) {
        gettimeofday(&start,NULL); 
        struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
        gettimeofday(&stop,NULL);
        // printf("1-1-row:%ld us; ", diff(&stop, &start));
      }
      else{
        gettimeofday(&start,NULL); 
        struct CS_matrix_idx_v_arr *muti1_idx_v_arr = row_matrix_CSR_multiply(matrix_CSR->rows[rand_num], matrix_CSR);
        gettimeofday(&stop,NULL);
        // printf("1-1-row:%ld us; ", diff(&stop, &start));
      }
      // print_counter();
      average1 += diff(&stop, &start);
      average2 += diff(&stop, &start);
      average3 += diff(&stop, &start);

      struct CS_matrix_idx_v_arr *muti2_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      struct CS_matrix_idx_v_arr *muti3_idx_v_arr = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC);
      if (muti2_idx_v_arr){
        // counter_init();
        if (muti2_idx_v_arr->nidx_vs > 30) {
          gettimeofday(&start,NULL); 
          muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
          gettimeofday(&stop,NULL);
          // printf("2-2-row:%ld us; ", diff(&stop, &start));
          average2 += diff(&stop, &start);
        }
        else{
          gettimeofday(&start,NULL); 
          muti2_idx_v_arr = row_matrix_CSR_multiply(muti2_idx_v_arr, matrix_CSR);
          gettimeofday(&stop,NULL);
          // printf("2-2-row:%ld us; ", diff(&stop, &start));
          average2 += diff(&stop, &start);
        }
        // print_counter();

        
      }
      if (matrix_CSC1) {
        if (muti3_idx_v_arr) {
          // counter_init();
          if (muti3_idx_v_arr->nidx_vs > 30) {
            gettimeofday(&start,NULL);
            struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC1);
            gettimeofday(&stop,NULL);
            // printf("3-2-row:%ld us; ", diff(&stop, &start));
            average3 += diff(&stop, &start);
          }
          else{
            gettimeofday(&start,NULL);
            struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_matrix_CSR_multiply(matrix_CSR->rows[rand_num], matrix_CSR1);
            gettimeofday(&stop,NULL);
            // printf("3-2-row:%ld us; ", diff(&stop, &start));
            average3 += diff(&stop, &start);
          }
          // print_counter();
          
        }
      }

      if (muti2_idx_v_arr){
        if (muti2_idx_v_arr->nidx_vs > 30) {
          gettimeofday(&start,NULL); 
          muti2_idx_v_arr = row_all_col_multiply(muti2_idx_v_arr, matrix_CSC);
          gettimeofday(&stop,NULL);
          // printf("2-3-row:%ld us; ", diff(&stop, &start));
          average2 += diff(&stop, &start);
        }
        else{
          gettimeofday(&start,NULL); 
          muti2_idx_v_arr = row_matrix_CSR_multiply(muti2_idx_v_arr, matrix_CSR);
          gettimeofday(&stop,NULL);
          // printf("2-3-row:%ld us; ", diff(&stop, &start));
          average2 += diff(&stop, &start);
        }
      }
      if (matrix_CSC2) {
        if (muti3_idx_v_arr) {
          if (muti3_idx_v_arr->nidx_vs > 30) {
            gettimeofday(&start,NULL);
            struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_all_col_multiply(matrix_CSR->rows[rand_num], matrix_CSC2);
            gettimeofday(&stop,NULL);
            // printf("3-3-row:%ld us; ", diff(&stop, &start));
            average3 += diff(&stop, &start);
          }
          // average3 += diff(&stop, &start);
          // print_counter();
          else{
            gettimeofday(&start,NULL);
            struct CS_matrix_idx_v_arr *muti3_idx_v_arr_tmp = row_matrix_CSR_multiply(matrix_CSR->rows[rand_num], matrix_CSR2);
            gettimeofday(&stop,NULL);
            // printf("3-3-row:%ld us; ", diff(&stop, &start));
            average3 += diff(&stop, &start);
          }
        }
      
      }
      
    }
    // else
    //   printf("1-1-row:0");
    // printf("\n");

  }
  average1 = average1 / matrix_CSR->nrows;
  average2 = average2 / matrix_CSR->nrows;
  average3 = average3 / matrix_CSR->nrows;
  average1_col = average1_col / matrix_CSR->nrows;
  printf("average vector multiply matrix_1: %lld us\n", average1);
  printf("average vector multiply matrix_2: %lld us\n", average2);
  printf("average vector multiply matrix_3: %lld us\n", average3);
  printf("matrix multiply average vector : %lld us\n", average1_col);
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
test_port(struct matrix_CSR *matrix_CSR){
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);

  uint32_t num = 16;
  struct timeval start,stop;
  uint32_t port[16] = {100021,200010,300003,400002,500003,600002,700003,800002,900003,1000003,1100003,1200002,1300002,1400002,1500004,1600003};

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+num*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = num;
  for (int i = 0; i < num; i++) {
    tmp->rows[i] = gen_sparse_matrix_row_fr_port(port[i]);
  }


  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR1 = sparse_matrix_multiply_CSC(tmp, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR1 = diff(&stop, &start);
  printf("port_CSR multi matrix 1t: %lld us", T_port_CSR1);
  print_vElemsNUM_of_Matrix_CSR(port_CSR1);
  print_npairsNUM_of_Matrix_CSR(port_CSR1);
  print_counter();
  counter_init();
  free_matrix_CSR(tmp);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR2 = sparse_matrix_multiply_CSC(port_CSR1, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR2 = diff(&stop, &start);
  printf("port_CSR multi matrix 2t: %lld us", T_port_CSR2);
  print_vElemsNUM_of_Matrix_CSR(port_CSR2);
  print_npairsNUM_of_Matrix_CSR(port_CSR2);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR1);
  // bdd_gbc();
  printf("/*=====================================================*/\n");

  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR3 = sparse_matrix_multiply_CSC(port_CSR2, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR3 = diff(&stop, &start);
  printf("port_CSR multi matrix 3t: %lld us", T_port_CSR3);
  print_vElemsNUM_of_Matrix_CSR(port_CSR3);
  print_npairsNUM_of_Matrix_CSR(port_CSR3);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR2);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR4 = sparse_matrix_multiply_CSC(port_CSR3, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR4 = diff(&stop, &start);
  printf("port_CSR multi matrix 4t: %lld us", T_port_CSR4);
  print_vElemsNUM_of_Matrix_CSR(port_CSR4);
  print_npairsNUM_of_Matrix_CSR(port_CSR4);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR3);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR5 = sparse_matrix_multiply_CSC(port_CSR4, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR5 = diff(&stop, &start);
  printf("port_CSR multi matrix 5t: %lld us", T_port_CSR5);
  print_vElemsNUM_of_Matrix_CSR(port_CSR5);
  print_npairsNUM_of_Matrix_CSR(port_CSR5);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR4);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR6 = sparse_matrix_multiply_CSC(port_CSR5, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR6 = diff(&stop, &start);
  printf("port_CSR multi matrix 6t: %lld us", T_port_CSR6);
  print_vElemsNUM_of_Matrix_CSR(port_CSR6);
  print_npairsNUM_of_Matrix_CSR(port_CSR6);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR5);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR7 = sparse_matrix_multiply_CSC(port_CSR6, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR7 = diff(&stop, &start);
  printf("port_CSR multi matrix 7t: %lld us", T_port_CSR7);
  print_vElemsNUM_of_Matrix_CSR(port_CSR7);
  print_npairsNUM_of_Matrix_CSR(port_CSR7);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR6);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR8 = sparse_matrix_multiply_CSC(port_CSR7, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR8 = diff(&stop, &start);
  printf("port_CSR multi matrix 8t: %lld us", T_port_CSR8);
  print_vElemsNUM_of_Matrix_CSR(port_CSR8);
  print_npairsNUM_of_Matrix_CSR(port_CSR8);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR7);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR9 = sparse_matrix_multiply_CSC(port_CSR8, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR9 = diff(&stop, &start);
  printf("port_CSR multi matrix 9t: %lld us", T_port_CSR9);
  print_vElemsNUM_of_Matrix_CSR(port_CSR9);
  print_npairsNUM_of_Matrix_CSR(port_CSR9);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR8);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR10 = sparse_matrix_multiply_CSC(port_CSR9, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR10 = diff(&stop, &start);
  printf("port_CSR multi matrix 10t: %lld us", T_port_CSR10);
  print_vElemsNUM_of_Matrix_CSR(port_CSR10);
  print_npairsNUM_of_Matrix_CSR(port_CSR10);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR9);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR11 = sparse_matrix_multiply_CSC(port_CSR10, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR11 = diff(&stop, &start);
  printf("port_CSR multi matrix 11t: %lld us", T_port_CSR11);
  print_vElemsNUM_of_Matrix_CSR(port_CSR11);
  print_npairsNUM_of_Matrix_CSR(port_CSR11);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR10);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR12 = sparse_matrix_multiply_CSC(port_CSR11, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR12 = diff(&stop, &start);
  printf("port_CSR multi matrix 12t: %lld us", T_port_CSR12);
  print_vElemsNUM_of_Matrix_CSR(port_CSR12);
  print_npairsNUM_of_Matrix_CSR(port_CSR12);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR11);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR13 = sparse_matrix_multiply_CSC(port_CSR12, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR13 = diff(&stop, &start);
  printf("port_CSR multi matrix 13t: %lld us", T_port_CSR13);
  print_vElemsNUM_of_Matrix_CSR(port_CSR13);
  print_npairsNUM_of_Matrix_CSR(port_CSR13);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR12);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  free_matrix_CSR(port_CSR13);
  free_matrix_CSC_fr_CSR(matrix_CSC);
}

void
test_1port(struct matrix_CSR *matrix_CSR, uint32_t port){
  struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);

  uint32_t num = 1;
  struct timeval start,stop;
  // uint32_t port[16] = {100021,200010,300003,400002,500003,600002,700003,800002,900003,1000003,1100003,1200002,1300002,1400002,1500004,1600003};

  struct matrix_CSR *tmp = xmalloc(sizeof(uint32_t)+num*sizeof(struct CS_matrix_idx_v_arr *));
  tmp->nrows = num;
  tmp->rows[0] = gen_sparse_matrix_row_fr_port(port);



  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR1 = sparse_matrix_multiply_CSC(tmp, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR1 = diff(&stop, &start);
  printf("port_CSR multi matrix 1t: %lld us", T_port_CSR1);
  print_vElemsNUM_of_Matrix_CSR(port_CSR1);
  print_npairsNUM_of_Matrix_CSR(port_CSR1);
  print_counter();
  counter_init();
  free_matrix_CSR(tmp);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR2 = sparse_matrix_multiply_CSC(port_CSR1, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR2 = diff(&stop, &start);
  printf("port_CSR multi matrix 2t: %lld us", T_port_CSR2);
  print_vElemsNUM_of_Matrix_CSR(port_CSR2);
  print_npairsNUM_of_Matrix_CSR(port_CSR2);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR1);
  // bdd_gbc();
  printf("/*=====================================================*/\n");

  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR3 = sparse_matrix_multiply_CSC(port_CSR2, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR3 = diff(&stop, &start);
  printf("port_CSR multi matrix 3t: %lld us", T_port_CSR3);
  print_vElemsNUM_of_Matrix_CSR(port_CSR3);
  print_npairsNUM_of_Matrix_CSR(port_CSR3);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR2);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR4 = sparse_matrix_multiply_CSC(port_CSR3, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR4 = diff(&stop, &start);
  printf("port_CSR multi matrix 4t: %lld us", T_port_CSR4);
  print_vElemsNUM_of_Matrix_CSR(port_CSR4);
  print_npairsNUM_of_Matrix_CSR(port_CSR4);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR3);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR5 = sparse_matrix_multiply_CSC(port_CSR4, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR5 = diff(&stop, &start);
  printf("port_CSR multi matrix 5t: %lld us", T_port_CSR5);
  print_vElemsNUM_of_Matrix_CSR(port_CSR5);
  print_npairsNUM_of_Matrix_CSR(port_CSR5);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR4);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR6 = sparse_matrix_multiply_CSC(port_CSR5, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR6 = diff(&stop, &start);
  printf("port_CSR multi matrix 6t: %lld us", T_port_CSR6);
  print_vElemsNUM_of_Matrix_CSR(port_CSR6);
  print_npairsNUM_of_Matrix_CSR(port_CSR6);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR5);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR7 = sparse_matrix_multiply_CSC(port_CSR6, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR7 = diff(&stop, &start);
  printf("port_CSR multi matrix 7t: %lld us", T_port_CSR7);
  print_vElemsNUM_of_Matrix_CSR(port_CSR7);
  print_npairsNUM_of_Matrix_CSR(port_CSR7);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR6);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR8 = sparse_matrix_multiply_CSC(port_CSR7, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR8 = diff(&stop, &start);
  printf("port_CSR multi matrix 8t: %lld us", T_port_CSR8);
  print_vElemsNUM_of_Matrix_CSR(port_CSR8);
  print_npairsNUM_of_Matrix_CSR(port_CSR8);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR7);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR9 = sparse_matrix_multiply_CSC(port_CSR8, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR9 = diff(&stop, &start);
  printf("port_CSR multi matrix 9t: %lld us", T_port_CSR9);
  print_vElemsNUM_of_Matrix_CSR(port_CSR9);
  print_npairsNUM_of_Matrix_CSR(port_CSR9);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR8);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR10 = sparse_matrix_multiply_CSC(port_CSR9, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR10 = diff(&stop, &start);
  printf("port_CSR multi matrix 10t: %lld us", T_port_CSR10);
  print_vElemsNUM_of_Matrix_CSR(port_CSR10);
  print_npairsNUM_of_Matrix_CSR(port_CSR10);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR9);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR11 = sparse_matrix_multiply_CSC(port_CSR10, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR11 = diff(&stop, &start);
  printf("port_CSR multi matrix 11t: %lld us", T_port_CSR11);
  print_vElemsNUM_of_Matrix_CSR(port_CSR11);
  print_npairsNUM_of_Matrix_CSR(port_CSR11);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR10);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR12 = sparse_matrix_multiply_CSC(port_CSR11, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR12 = diff(&stop, &start);
  printf("port_CSR multi matrix 12t: %lld us", T_port_CSR12);
  print_vElemsNUM_of_Matrix_CSR(port_CSR12);
  print_npairsNUM_of_Matrix_CSR(port_CSR12);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR11);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR13 = sparse_matrix_multiply_CSC(port_CSR12, matrix_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR13 = diff(&stop, &start);
  printf("port_CSR multi matrix 13t: %lld us", T_port_CSR13);
  print_vElemsNUM_of_Matrix_CSR(port_CSR13);
  print_npairsNUM_of_Matrix_CSR(port_CSR13);
  print_counter();
  counter_init();
  free_matrix_CSR(port_CSR12);
  // bdd_gbc();
  printf("/*=====================================================*/\n");
  free_matrix_CSR(port_CSR13);
  free_matrix_CSC_fr_CSR(matrix_CSC);
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
  // char *net = "stanford_whole";
  char *net = "i2";
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
  matrix_idx = matrix_idx_init();
  BDD_init_multiply();

  gettimeofday(&start,NULL);
  bdd_sw_load();
  gettimeofday(&stop,NULL);
  long long int gen_bdd_sws = diff(&stop, &start)/1000;
  printf("gen bdd_sws_arr: %lld ms\n", gen_bdd_sws);

  if (is_r_action_same(bdd_sws_arr[0]->rules[0], bdd_sws_arr[0]->rules[1])) {
    printf("there is same\n");
  }
  else
    printf("there is not the same\n");
  init_r_to_merge();

  printf("same number = %d\n", same_num);



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


  
  gettimeofday(&start,NULL);
  struct matrix_CSR *matrix_CSR_old = gen_sparse_matrix(); 
  gettimeofday(&stop,NULL);
  long long int gen_CSR = diff(&stop, &start)/1000;
  printf("gen CSR: %lld ms\n", gen_CSR);
  printf("this matrix_CSR has %d rules\n", matrix_CSR_old->nrows);
  print_vElemsNUM_of_Matrix_CSR(matrix_CSR_old);
  print_npairsNUM_of_Matrix_CSR(matrix_CSR_old);
  // bdd_gbc();
  // data_unload();

  printf("--------------------------------------\n");
  printf("same number = %d\n", same_num);
  struct matrix_CSR *matrix_CSR = gen_merged_CSR(matrix_CSR_old);
  printf("this matrix_CSR has %d rules\n", matrix_CSR->nrows);
  print_vElemsNUM_of_Matrix_CSR(matrix_CSR);
  print_npairsNUM_of_Matrix_CSR(matrix_CSR);
  bdd_gbc();
  printf("--------------------------------------\n");

  // average_updating_link_merged(matrix_CSR, matrix_CSR_old);
  // test_someport_forall_merged(matrix_CSR, matrix_CSR_old);
  data_unload();


  /* test updating rules */
  average_updating_r_merged(matrix_CSR, matrix_CSR_old);
  // average_updating_r_ord(matrix_CSR_old);


  // struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  // int count = 0;
  // for (int i = 0; i < matrix_CSR->nrows; i++)
  //   if(matrix_CSR->rows[i])
  //     count++;
  
  // printf("matrix_CSR has %d rows\n", count);
  // count = 0;
  // for (int i = 0; i < matrix_CSC->ncols; i++)
  //   if(matrix_CSC->cols[i])
  //     count++;
  // printf("matrix_CSC has %d cols\n", count);
  // gettimeofday(&start,NULL);
  // struct CS_matrix_idx_v_arr *port_CSR_row = gen_sparse_matrix_row_fr_port(300004); 
  // gettimeofday(&stop,NULL);
  // long long int gen_port_CSR_row = diff(&stop, &start)/1000;

  // gettimeofday(&start,NULL);
  // struct matrix_CSC *matrix_CSC = gen_CSC_from_CSR(matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int gen_CSC = diff(&stop, &start)/1000;
  // printf("gen CSC: %lld ms\n", gen_CSC);
  // print_npairsNUM_of_Matrix_CSC(matrix_CSC);
  // printf("/*=====================================================*/\n");

/*================================port生成的向量 对矩阵的计算======================================*/
  // test_port(matrix_CSR);
  // test_port(matrix_CSR);
  // test_1port(matrix_CSR, 100027);


  
  
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  
  
  // // print_CS_matrix_idx_v_arr(port_CSR_row4);
  // bdd_done();


/*================================矩阵 对矩阵的计算======================================*/


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti1_CSR = sparse_matrix_multiply(matrix_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre = diff(&stop, &start)/1000;
  // printf("matrix squre: %lld ms\n", squre);
  // print_vElemsNUM_of_Matrix_CSR(muti1_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti1_CSR);
  // print_counter();
  // counter_init();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti1_CSR_old = sparse_matrix_multiply(matrix_CSR_old, matrix_CSR_old);
  // gettimeofday(&stop,NULL);
  // long long int squre_old = diff(&stop, &start)/1000;
  // printf("matrix squre_old: %lld ms\n", squre_old);
  // print_vElemsNUM_of_Matrix_CSR(muti1_CSR_old);
  // print_npairsNUM_of_Matrix_CSR(muti1_CSR_old);
  // print_counter();
  // counter_init();
  // printf("--------------------------------------\n");


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti3_twice_CSR = sparse_matrix_multiply(muti1_CSR, muti1_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre3_twice = diff(&stop, &start)/1000;
  // printf("matrix squre3_twice: %lld ms\n", squre3_twice);
  // print_vElemsNUM_of_Matrix_CSR(muti3_twice_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti3_twice_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti3_twice_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");
  
  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti1_CSC = gen_CSC_from_CSR(muti1_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre_CSC = diff(&stop, &start)/1000;
  // printf("matrix squre->CSC: %lld ms\n", squre_CSC);
  // print_vElemsNUM_of_Matrix_CSC(muti1_CSC);
  // print_npairsNUM_of_Matrix_CSC(muti1_CSC);
  // printf("--------------------------------------\n");
  // print_counter();
  // counter_init();


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *other_CSR = sparse_matrix_multiply_otway(matrix_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int other_squre = diff(&stop, &start)/1000;
  // printf("other_CSRmatrix squre: %lld ms\n", other_squre);
  // print_vElemsNUM_of_Matrix_CSR(other_CSR);
  // print_npairsNUM_of_Matrix_CSR(other_CSR);
  // print_counter();
  // counter_init();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSC *muti1_CSC = gen_CSC_from_CSR(other_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre_CSC = diff(&stop, &start)/1000;
  // printf("matrix squre->CSC: %lld ms\n", squre_CSC);
  // print_vElemsNUM_of_Matrix_CSC(muti1_CSC);
  // print_npairsNUM_of_Matrix_CSC(muti1_CSC);
    // printf("/*=====================================================*/\n");


  // struct matrix_CSC *muti1_CSC = gen_CSC_from_CSR(muti1_CSR);
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti1 = vec_matrix_multiply(port_CSR_row, muti1_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti1 = diff(&stop, &start);
  // printf("port->vs multi matrix muti1: %lld us; the len = %d\n", T_port_CSR_row_muti1, port_CSR_row_muti1->nidx_vs);
  // print_counter();
  // counter_init();
  // printf("/*=====================================================*/\n");

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

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti2_CSR = sparse_matrix_multiply(muti1_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre2 = diff(&stop, &start)/1000;
  // printf("matrix squre2: %lld ms\n", squre2);
  // print_vElemsNUM_of_Matrix_CSR(muti2_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti2_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti1_CSR);
  // bdd_gbc();
  // // correct_verifination(muti2_CSR);
  // // printf("--------------------------------------\n");
  // printf("/*=====================================================*/\n");
  
  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti2_CSR_old = sparse_matrix_multiply(muti1_CSR_old, matrix_CSR_old);
  // gettimeofday(&stop,NULL);
  // long long int squre2_old = diff(&stop, &start)/1000;
  // printf("matrix squre2: %lld ms\n", squre2_old);
  // print_vElemsNUM_of_Matrix_CSR(muti2_CSR_old);
  // print_npairsNUM_of_Matrix_CSR(muti2_CSR_old);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti1_CSR_old);
  // bdd_gbc();
  // // correct_verifination(muti2_CSR);
  // // printf("--------------------------------------\n");
  // printf("/*=====================================================*/\n");
  

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti2_r_CSR = sparse_matrix_multiply(matrix_CSR, muti1_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre2_r = diff(&stop, &start)/1000;
  // printf("matrix squre2_r: %lld ms\n", squre2_r);
  // print_vElemsNUM_of_Matrix_CSR(muti2_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti2_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti2_r_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");
  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti5_twice_CSR = sparse_matrix_multiply(muti2_CSR, muti2_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre5_twice = diff(&stop, &start)/1000;
  // printf("matrix squre5_twice: %lld ms\n", squre5_twice);
  // print_vElemsNUM_of_Matrix_CSR(muti5_twice_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti5_twice_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti5_twice_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *other_muti2_CSR = sparse_matrix_multiply_otway(other_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int other_squre2 = diff(&stop, &start)/1000;
  // printf("other_CSRmatrix squre2: %lld ms\n", other_squre2);
  // print_vElemsNUM_of_Matrix_CSR(other_muti2_CSR);
  // print_npairsNUM_of_Matrix_CSR(other_muti2_CSR);
  // print_counter();
  // counter_init();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *other_muti2_r_CSR = sparse_matrix_multiply_otway(matrix_CSR, other_CSR);
  // gettimeofday(&stop,NULL);
  // long long int other_squre2_r = diff(&stop, &start)/1000;
  // printf("other_CSRmatrix squre2_r: %lld ms\n", other_squre2_r);
  // print_vElemsNUM_of_Matrix_CSR(other_muti2_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(other_muti2_r_CSR);
  // print_counter();
  // counter_init();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti2_CSC = gen_CSC_from_CSR(muti2_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre2_CSC = diff(&stop, &start)/1000;
  // printf("matrix squre2->CSC: %lld ms\n", squre_CSC);
  // print_vElemsNUM_of_Matrix_CSC(muti2_CSC);
  // print_npairsNUM_of_Matrix_CSC(muti2_CSC);
  // printf("/*=====================================================*/\n");


  // struct matrix_CSC *muti2_CSC = gen_CSC_from_CSR(muti2_CSR);
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti2 = vec_matrix_multiply(port_CSR_row, muti2_CSC);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti2 = diff(&stop, &start);
  // printf("port->vs multi matrix muti2: %lld us; the len = %d\n", T_port_CSR_row_muti2, port_CSR_row_muti2->nidx_vs);
  // printf("/*=====================================================*/\n");

  

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti3_CSR = sparse_matrix_multiply(muti2_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre3 = diff(&stop, &start)/1000;
  // printf("matrix squre3: %lld ms\n", squre3);
  // print_vElemsNUM_of_Matrix_CSR(muti3_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti3_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti2_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti3_r_CSR = sparse_matrix_multiply(matrix_CSR, muti2_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre3_r = diff(&stop, &start)/1000;
  // printf("matrix squre3_r: %lld ms\n", squre3_r);
  // print_vElemsNUM_of_Matrix_CSR(muti3_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti3_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti2_CSR);
  // free_matrix_CSR(muti3_r_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti7_twice_CSR = sparse_matrix_multiply(muti3_CSR, muti3_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre7_twice = diff(&stop, &start)/1000;
  // printf("matrix squre7_twice: %lld ms\n", squre7_twice);
  // print_vElemsNUM_of_Matrix_CSR(muti7_twice_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti7_twice_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti7_twice_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");
  // struct matrix_CSR *muti3_CSC = gen_CSC_from_CSR(muti3_CSR);
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti3 = row_matrix_CSR_multiply(port_CSR_row, muti3_CSR);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti3 = diff(&stop, &start);
  // printf("port->vs multi matrix muti3: %lld us; the len = %d\n", T_port_CSR_row_muti3, port_CSR_row_muti3->nidx_vs);
  // printf("/*=====================================================*/\n");



  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti4_CSR = sparse_matrix_multiply(muti3_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre4 = diff(&stop, &start)/1000;
  // printf("matrix squre4: %lld ms\n", squre4);
  // print_vElemsNUM_of_Matrix_CSR(muti4_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti4_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti3_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti4_r_CSR = sparse_matrix_multiply(matrix_CSR, muti3_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre4_r = diff(&stop, &start)/1000;
  // printf("matrix squre4_r: %lld ms\n", squre4_r);
  // print_vElemsNUM_of_Matrix_CSR(muti4_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti4_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti3_CSR);
  // free_matrix_CSR(muti4_r_CSR);
  // bdd_gbc();

  // printf("/*=====================================================*/\n");

  // struct matrix_CSR *muti4_CSC = gen_CSC_from_CSR(muti4_CSR);
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti4 = row_matrix_CSR_multiply(port_CSR_row, muti4_CSR);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti4 = diff(&stop, &start);
  // printf("port->vs multi matrix muti4: %lld us; the len = %d\n", T_port_CSR_row_muti4, port_CSR_row_muti4->nidx_vs);
  // printf("/*=====================================================*/\n");


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti9_twice_CSR = sparse_matrix_multiply(muti4_CSR, muti4_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre9_twice = diff(&stop, &start)/1000;
  // printf("matrix squre9_twice: %lld ms\n", squre9_twice);
  // print_vElemsNUM_of_Matrix_CSR(muti9_twice_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti9_twice_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti9_twice_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti5_CSR = sparse_matrix_multiply(muti4_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre5 = diff(&stop, &start)/1000;
  // printf("matrix squre5: %lld ms\n", squre5);
  // print_vElemsNUM_of_Matrix_CSR(muti5_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti5_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti4_CSR);
  // bdd_gbc();
  // correct_verifination(muti5_CSR);
  // printf("--------------------------------------\n");

  
  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti5_r_CSR = sparse_matrix_multiply(matrix_CSR, muti4_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre5_r = diff(&stop, &start)/1000;
  // printf("matrix squre5_r: %lld ms\n", squre5_r);
  // print_vElemsNUM_of_Matrix_CSR(muti5_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti5_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti4_CSR);
  // free_matrix_CSR(muti5_r_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // struct matrix_CSR *muti5_CSC = gen_CSC_from_CSR(muti5_CSR);
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti5 = row_matrix_CSR_multiply(port_CSR_row, muti5_CSR);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti5 = diff(&stop, &start);
  // printf("port->vs multi matrix muti5: %lld us; the len = %d\n", T_port_CSR_row_muti5, port_CSR_row_muti5->nidx_vs);
  // printf("/*=====================================================*/\n");


  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti6_CSR = sparse_matrix_multiply(muti5_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre6 = diff(&stop, &start)/1000;
  // printf("matrix squre6: %lld ms\n", squre6);
  // print_vElemsNUM_of_Matrix_CSR(muti6_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti6_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti5_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti6_r_CSR = sparse_matrix_multiply(matrix_CSR, muti5_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre6_r = diff(&stop, &start)/1000;
  // printf("matrix squre6_r: %lld ms\n", squre5_r);
  // print_vElemsNUM_of_Matrix_CSR(muti6_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti6_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti5_CSR);
  // free_matrix_CSR(muti6_r_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // struct matrix_CSR *muti6_CSC = gen_CSC_from_CSR(muti6_CSR);
  // gettimeofday(&start,NULL); 
  // struct CS_matrix_idx_v_arr *port_CSR_row_muti6 = row_matrix_CSR_multiply(port_CSR_row, muti6_CSR);
  // // muti1_idx_v_arr = row_all_col_multiply(muti1_idx_v_arr, matrix_CSC);
  // gettimeofday(&stop,NULL);
  // long long int T_port_CSR_row_muti6 = diff(&stop, &start);
  // printf("port->vs multi matrix muti6: %lld us; the len = %d\n", T_port_CSR_row_muti6, port_CSR_row_muti6->nidx_vs);
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti7_CSR = sparse_matrix_multiply(muti6_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre7 = diff(&stop, &start)/1000;
  // printf("matrix squre7: %lld ms\n", squre7);
  // print_vElemsNUM_of_Matrix_CSR(muti7_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti7_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti6_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti7_r_CSR = sparse_matrix_multiply(matrix_CSR, muti6_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre7_r = diff(&stop, &start)/1000;
  // printf("matrix squre7_r: %lld ms\n", squre7_r);
  // print_vElemsNUM_of_Matrix_CSR(muti7_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti7_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti6_CSR);
  // free_matrix_CSR(muti7_r_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti8_CSR = sparse_matrix_multiply(muti7_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre8 = diff(&stop, &start)/1000;
  // printf("matrix squre8: %lld ms\n", squre8);
  // print_vElemsNUM_of_Matrix_CSR(muti8_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti8_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti7_CSR);
  // bdd_gbc();
  // printf("--------------------------------------\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti8_r_CSR = sparse_matrix_multiply(matrix_CSR, muti7_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre8_r = diff(&stop, &start)/1000;
  // printf("matrix squre8_r: %lld ms\n", squre8_r);
  // print_vElemsNUM_of_Matrix_CSR(muti8_r_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti8_r_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti7_CSR);
  // free_matrix_CSR(muti8_r_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti9_CSR = sparse_matrix_multiply(muti8_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre9 = diff(&stop, &start)/1000;
  // printf("matrix squre9: %lld ms\n", squre9);
  // print_vElemsNUM_of_Matrix_CSR(muti9_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti9_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti8_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti10_CSR = sparse_matrix_multiply(muti9_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre10 = diff(&stop, &start)/1000;
  // printf("matrix squre10: %lld ms\n", squre10);
  // print_vElemsNUM_of_Matrix_CSR(muti10_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti10_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti9_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti11_CSR = sparse_matrix_multiply(muti10_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre11 = diff(&stop, &start)/1000;
  // printf("matrix squre11: %lld ms\n", squre11);
  // print_vElemsNUM_of_Matrix_CSR(muti11_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti11_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti10_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti12_CSR = sparse_matrix_multiply(muti11_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre12 = diff(&stop, &start)/1000;
  // printf("matrix squre12: %lld ms\n", squre12);
  // print_vElemsNUM_of_Matrix_CSR(muti12_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti12_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti11_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");

  // gettimeofday(&start,NULL);
  // struct matrix_CSR *muti13_CSR = sparse_matrix_multiply(muti12_CSR, matrix_CSR);
  // gettimeofday(&stop,NULL);
  // long long int squre13 = diff(&stop, &start)/1000;
  // printf("matrix squre13: %lld ms\n", squre13);
  // print_vElemsNUM_of_Matrix_CSR(muti13_CSR);
  // print_npairsNUM_of_Matrix_CSR(muti13_CSR);
  // print_counter();
  // counter_init();
  // free_matrix_CSR(muti12_CSR);
  // bdd_gbc();
  // printf("/*=====================================================*/\n");
  // free_matrix_CSR(muti13_CSR);

  // average_v_matrix_forall(matrix_CSR, muti1_CSR, muti2_CSR);
  // free_matrix_CSR(muti1_CSR);
  // free_matrix_CSR(muti2_CSR);
  // average_v_matrix_forall(matrix_CSR, matrix_CSC, muti1_CSC, NULL);
  
  // free_matrix_CSR(matrix_CSR);
  // free_matrix_CSC_fr_CSR(matrix_CSC);
  // free_matrix_CSR(muti1_CSR);
  // data_unload();
  bdd_done(); 
  free(matrix_idx);
  bdd_sw_unload();
  
  return 0;
}

