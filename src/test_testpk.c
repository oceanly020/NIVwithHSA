#define _GNU_SOURCE 1

#include "all_testpk.h"
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
main (int argc, char **argv) {
  struct timeval start,stop;  //计算时间差 usec
  // gettimeofday(&start,NULL);
  // gettimeofday(&stop,NULL);
  BDD_init_multiply();

/*================================初始化加载数据使用json数据======================================*/ 
  // char *jsonet = "json_stanford_fwd";
  char *jsonet = "json_stanford";
  // char *jsonet = "json_i2";
  // init_sw_port_relations(jsonet);
  // init_bdd_merged_sws();
  struct network_bdd *sws_json = get_network_bdd_jsondata("tfs", jsonet);
  // struct network_bdd *sws_json_noconf = get_network_bdd_jsondata_noconf("tfs", jsonet);
  // struct APs *APs = get_network_bdd_jsondata_inc_APs("tfs", jsonet);
  // for (int i = 0; i < sws_json->sws[0]->nrules; i++)
  //   print_links_of_rule(sws_json->sws[0]->rules[i]->lks_out);

/*================================测试MTBDD维持对级联流表======================================*/
  init_mtbdd_sws();
  generate_mtbddrules(sws_json);
  build_network_by_update_rules(sws_json);
  get_mtbdd_probes_num();
  test_remove_then_readd_rules(sws_json);
  

/*================================测试MTBDD生成测试数据包头对单一流表======================================*/
  // generate_mtbddrules(sws_json);
  // build_network_by_update_rules(sws_json);
  // test_remove_then_readd_rules(sws_json);
  // test_mtbdd_get_right_dominant(sws_json_noconf, sws_json);
 
/*================================测试RuleChecker======================================*/
// struct network_bdd *sws_rc = generate_empty_net(sws_json);
// sws_rc = UpdateProbesInsert_forall(sws_rc, sws_json);
// count_RuleChecker_result(sws_rc);

/*================================测试RuleChecker改======================================*/
// generate_mtbddrules(sws_json);
// struct network_mtbdd *net_mt = generate_mtbdd_foreach_table(sws_json);
// uint32_t probes_num = get_mtbdd_probes_num(net_mt);

/*================================测试APV======================================*/
  // struct network_bdd *sws_uncover =  get_bdd_sws_uncover();
  // struct network_bdd *sws_merge = get_bdd_sws_merge(sws_json);
  // uint32_t num = get_num_of_rules_innet(sws_merge);

  // uint32_t num = get_num_of_rules_innet(sws_json);
  // printf("the rules\' num of sws_merge is: %d\n", num); 

  // gettimeofday(&start,NULL);
  // struct APs *APs_merge = get_APs_simple(sws_merg e);
  // struct APs *APs_merge_simple = get_APs_simple(sws_json);
  // test_APs_simple(APs_merge_simple, sws_json_noconf);

  // struct APs *APs_merge_simple = get_APs_simple_withdiff(sws_json);
  // struct APs *APs_merge = get_APs(sws_json);
  // struct APs *APs_merge = get_APs_bounding(sws_json);

  // struct APs *APs = get_network_bdd_jsondata_inc_APs("tfs", jsonet);  

  // gettimeofday(&stop,NULL);
  // long long int gen_APs_merge = diff(&stop, &start)/1000;
  // printf("gen APs_merge: %lld ms\n", gen_APs_merge);
  // test_AP_generation(APs_merge, sws_merge);
  // test_AP_generation(APs_merge, sws_json);

/*================================free part======================================*/
  // free_matrix_CSR(matrix_CSR);
  // free_matrix_CSC_fr_CSR(matrix_CSC);
  // free_matrix_CSR(muti1_CSR);
  // data_unload();
  bdd_done(); 
  // free(matrix_idx);
  // bdd_sw_unload();
  
  return 0;
}

