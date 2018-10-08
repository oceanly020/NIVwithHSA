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
    int rand_num = i;
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

void
print_counter(void){
  printf("computation_counter = %d\n", computation_counter);
  printf("compu_true_counter = %d\n", compu_true_counter);
  printf("elemconnet_counter = %d\n", elemconnet_counter);
  printf("elem_true_counter = %d\n", elem_true_counter);
  printf("time_counter1 = %ld us\n", time_counter1);
  printf("time_counter2 = %ld us\n", time_counter2);
  printf("time_counter3 = %ld us\n", time_counter3);
  printf("time_counter4 = %ld us\n", time_counter4);
  printf("time_counter5 = %ld us\n", time_counter5);
  printf("time_counter_elembdd_withpair = %ld us\n", time_counter_elembdd_withpair);
  printf("time_counter_elemplus = %ld us\n", time_counter_elemplus);
  printf("time_counter_nf_space_connect = %ld us\n", time_counter_nf_space_connect);
  printf("time_counter_eleminsc = %ld us\n", time_counter_eleminsc);
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


void
test_port_forallsquare(struct matrix_CSR *matrix_CSR){
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
  struct matrix_CSR *muti10_CSR = sparse_matrix_multiply(muti9_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti10_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR11 = sparse_matrix_multiply_CSC(tmp, muti10_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR11 = diff(&stop, &start);
  printf("port_CSR multi matrix 11t: %lld us", T_port_CSR11);
  print_vElemsNUM_of_Matrix_CSR(port_CSR11);
  print_npairsNUM_of_Matrix_CSR(port_CSR11);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti9_CSR);
  free_matrix_CSR(port_CSR11);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti11_CSR = sparse_matrix_multiply(muti10_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti11_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR12 = sparse_matrix_multiply_CSC(tmp, muti11_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR12 = diff(&stop, &start);
  printf("port_CSR multi matrix 12t: %lld us", T_port_CSR12);
  print_vElemsNUM_of_Matrix_CSR(port_CSR12);
  print_npairsNUM_of_Matrix_CSR(port_CSR12);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti10_CSR);
  free_matrix_CSR(port_CSR12);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  struct matrix_CSR *muti12_CSR = sparse_matrix_multiply(muti11_CSR, matrix_CSR);
  matrix_CSC = gen_CSC_from_CSR(muti12_CSR);
  counter_init();
  gettimeofday(&start,NULL); 
  struct matrix_CSR *port_CSR13 = sparse_matrix_multiply_CSC(tmp, muti12_CSR, matrix_CSC);
  gettimeofday(&stop,NULL);
  long long int T_port_CSR13 = diff(&stop, &start);
  printf("port_CSR multi matrix 13t: %lld us", T_port_CSR13);
  print_vElemsNUM_of_Matrix_CSR(port_CSR13);
  print_npairsNUM_of_Matrix_CSR(port_CSR13);
  print_counter();
  counter_init();
  free_matrix_CSC_fr_CSR(matrix_CSC);
  free_matrix_CSR(muti11_CSR);
  free_matrix_CSR(port_CSR13);
  bdd_gbc();
  printf("/*=====================================================*/\n");
  free_matrix_CSR(tmp);
  free_matrix_CSR(muti12_CSR);
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
  // gettimeofday(&start,NULL);
  // struct matrix_buf *matrix_buf = matrix_init();
  struct sw *sw0 = sw_get(0);
  struct of_rule *rule1 = rule_get(sw0, 22);
  print_rule(rule1);
  struct switch_rs *swr0 = gen_sw_rules(0);
  struct ex_rule *ex_rule1 = ex_rule_get(swr0, 22);
  print_ex_rule(ex_rule1);

  struct trie_node *root = crate_trie_node_init();

  gettimeofday(&start,NULL);
  trie_add_rules_for_sw(root, swr0);
  gettimeofday(&stop,NULL);
  long long int build_trie = diff(&stop, &start);
  printf("build trie from sw0: %lld us\n", build_trie);
  data_unload();

  
  
  
  return 0;
}

