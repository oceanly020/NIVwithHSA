#include <stdio.h>

#include "usedBDD.h"
#include "all.h"
// #define MF_LEN 2
// struct mf_uint16_t {
//   uint16_t mf_w[MF_LEN];
//   uint16_t mf_v[MF_LEN];
// };

void
init_mf(struct mf_uint16_t *mf) {
	for (int i = 0; i < MF_LEN; i++){
		mf->mf_w[i] = 0x0000;
		mf->mf_v[i] = 0x0000;
	}
}

BDD 
mf2bdd(struct mf_uint16_t *mf) {
	BDD root, tmp;
	root = 1;
	tmp = 1; 

	for (int i = 0; i < MF_LEN; i++){
		int reverse_i = MF_LEN - i - 1;
		uint16_t sign = 0x0001;
		for (int j = 0; j < 16; j++){
			if (!(sign & mf->mf_w[reverse_i])){
				root = bdd_ithvar(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
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
	if (root == 1){
		root = bdd_ithvar(0);
		LOW(root) = 1;
		HIGH(root) = 1;
	}
	return root;
}

struct mf_uint16_t_array *
bdd2mf(BDD root, int *varnum) {

	

	if(LOW(root) == HIGH(root) == 1){
		struct mf_uint16_t_array *mf_arr_tmp = 
		for (int i = 0; i < MF_LEN; i++){
			mf->mf_w[i] = 0xffff;
			mf->mf_v[i] = 0x0000;
		}
		return mf_arr_tmp;
	}

	BDD record[*varnum + 2];
	for (int i = 0; i < *varnum; i++)
		record[i] = -1;
	struct mf_uint16_t *mf_tmp = xcalloc (1, sizeof *mf_tmp);
	struct mf_uint16_t *arr_tmp[10*(*varnum)]
	int arr_count = 0;
	struct mf_uint16_t *mf_tmp = xcalloc (1, sizeof *mf_tmp);
	init_mf(mf_tmp);
	BDD node_tmp;
	record[0] = -1;
	record[1] = root;
	int sign = 1;
	while(sign){
		node_tmp = record[sign];

		if (HIGH(node_tmp) == record[sign+1]){
			if (LOW(node_tmp) == 1){
				int var = bdd_var(node_tmp);


				record[sign + 1] = -1;
				sign--;
			}
			else if ((LOW(node_tmp) == 0)){
				record[sign + 1] = -1;
				sign--;
			}
			else {

				sign++;
				record[sign] = LOW(node_tmp);
			}
		}
		else if (LOW(node_tmp) == record[sign+1]){
			record[sign + 1] = -1
			sign--;
		}
		else{
			if (HIGH(node_tmp) == 1){
				int var = bdd_var(node_tmp);


				record[sign + 1] = 1;
			}
			else if ((HIGH(node_tmp) == 0)){
				record[sign + 1] = 0;
			}
			else {

				sign++;
				record[sign] = HIGH(node_tmp);
			}
		}



	}

	




	mf_uint16_t_array tmp
	return mf;
}

void
print_node(BDD r) {
	printf("num: %d; ", r);
    printf("low: %d; ", bdd_low(r));
    printf("high: %d; ", bdd_high(r));
    printf("var: %d\n", bdd_var(r));
}

int 
main() {
	struct mf_uint16_t *mf1 = xcalloc (1, sizeof *mf1);
	struct mf_uint16_t *mf2 = xcalloc (1, sizeof *mf2);
	mf1->mf_w[0] = 0x3ca0;
	mf1->mf_w[1] = 0xeaff;
	mf1->mf_v[0] = 0x8019;
	mf1->mf_v[1] = 0x0400;
	mf2->mf_w[0] = 0xff40;
	mf2->mf_w[1] = 0x8fff;
	mf2->mf_v[0] = 0x0019;
	mf2->mf_v[1] = 0x1000;

	// mf1->mf_w[0] = 0xffff;
	// mf1->mf_w[1] = 0xffff;
	// mf1->mf_v[0] = 0xffff;
	// mf1->mf_v[1] = 0xffff;


    BDD v1, v2;
    BDD seta, setb;
    static int v[2] = {1,3};
    
    bdd_init(1000,100);
    bdd_setvarnum(16*MF_LEN);//变量的数目，也就是二进制位数。
    // v1 = 1;
    v1 = mf2bdd(mf1);
    bdd_printtable(v1);


    // v2 = mf2bdd(mf2);
    // v1 = bdd_ithvar(0);
    // v1 = bdd_ithvar(1);
    // v2 = bdd_nithvar(1);
    // print_node(v1);
    // print_node(v2);
    // bdd_printdot(v2);
       // One way 
    // seta = bdd_addref( bdd_apply(v1,v2,bddop_and) );
    // bdd_printdot(seta);
    // bdd_printset(seta);
    // bdd_printtable(6);

       // Another way 
    // setb = bdd_addref( bdd_makeset(v,2) );
    // bdd_printset(setb);
}