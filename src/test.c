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


#define MAX_REFIX #255

int
main (int argc, char **argv)
{
  uint32_t *matrix_buf, *matrix_idx;
  char *net = "stanford";
  // bool one_step = false;
  load (net);
  // struct sw *sw0 = sw_get(0);
  // struct of_rule *rule1 = rule_get(sw0, 0);
  // print_rule(rule1);

  // struct links_of_rule *ls = rule_links_get_swidx(sw0, 2, RULE_LINK_IN);
  // print_links_wc (ls);

  char *linkdir = "../data/";
  link_data_load (linkdir);
  matrix_buf = matrix_init();
  matrix_idx = matrix_idx_init ();
  gen_matrix(matrix_buf);

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
  // struct timeval start, end; //计时结构
  // gettimeofday (&start, NULL);

  // struct res *in = res_create (data_file->stages + 1);

  // // printf ("%s \n",data_strs);
  // res_free (in);

  // gettimeofday (&end, NULL);

  free(matrix_buf);
  free(matrix_idx);
  data_unload();
  return 0;
}

