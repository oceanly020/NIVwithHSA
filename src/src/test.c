#include <stdio.h>
#include "usedBDD.h"
#include "all.h"

static uint16_t var2sign[16] = {
	0x8000, 0x4000, 0x2000, 0x1000,
	0x0800, 0x0400, 0x0200, 0x0100,
	0x0080, 0x0040, 0x0020, 0x0010, 
	0x0008, 0x0004, 0x0002, 0x0001
};
#define VAR2SIGN(a) (var2sign[(a%16)])
#define FRA2INT(a) ((int) (a))

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
mf2bdd_init(struct mf_uint16_t *mf) {

	BDD root, tmp;
	root = 1;
	tmp = 1;  
	print_mf_uint16_t(mf);
	for (int i = 0; i < MF_LEN; i++){
		int reverse_i = MF_LEN - i - 1;
		uint16_t sign = 0x0001;
		for (int j = 0; j < 16; j++){
			if (!(sign & mf->mf_w[reverse_i])){
				root = bdd_ithvar(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
				// printf("%d\n", root);
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

BDD 
mf2bdd(struct mf_uint16_t *mf) {

	BDD root, tmp;
	root = 1;
	tmp = 1;  
	print_mf_uint16_t(mf);
	for (int i = 0; i < MF_LEN; i++){
		int reverse_i = MF_LEN - i - 1;
		uint16_t sign = 0x0001;
		for (int j = 0; j < 16; j++){
			if (!(sign & mf->mf_w[reverse_i])){
				int level = bdd_var2level(16*MF_LEN - 16*i - j - 1);//生成相应变量的一个节点
				if (sign & mf->mf_v[reverse_i]){
					root = bdd_makenode(level, 0, tmp);
					// LOW(root) = 0;
					// HIGH(root) = tmp;
				}
				else{

					root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点
					// LOW(root) = tmp;
					// HIGH(root) = 0;
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

int
add_node_2_mf(BDD node, BDD prenode,struct mf_uint16_t *mf, uint16_t v) {
	int var_pre, var_prediv16;
	int var = bdd_var(node);
	int vardiv16 = FRA2INT(var/16);
	if (prenode == -1) {
		var_pre = -1;
		var_prediv16 = 0;
	}
	else{
		var_pre = bdd_var(prenode);
		var_prediv16 = FRA2INT(var_pre/16);
	}

	for (int i = var_prediv16; i < vardiv16+1; i++){
		uint16_t w_sign = 0x0000;
		if (i == var_prediv16){
			for (int j = (var_pre%16)+1; j < 16; j++){
				w_sign += var2sign[j];
			}
			if (i == vardiv16){
				uint16_t sign = 0;
				for (int j = 0; j < (var%16); j++){
					sign += var2sign[j];
				}
				w_sign = w_sign & sign;	
			}
			mf->mf_w[i] = w_sign | mf->mf_w[i];
			mf->mf_v[i] = (~w_sign) & mf->mf_v[i];
		}
		else if (i == vardiv16){
			for (int j = 0; j < (var%16); j++){
				w_sign += var2sign[j];
			}
			mf->mf_w[i] = w_sign | mf->mf_w[i];
			mf->mf_v[i] = (~w_sign) & mf->mf_v[i];
		}
		else {
			mf->mf_w[i] = 0xffff;
			mf->mf_v[i] = 0x0000;
		}
	}

	uint16_t sign = VAR2SIGN(var);
	mf->mf_w[vardiv16] = mf->mf_w[vardiv16] & (~sign);

	if (v == 0)
		mf->mf_v[vardiv16] = mf->mf_v[vardiv16] & (~sign);
	else 
		mf->mf_v[vardiv16] = mf->mf_v[vardiv16] | sign;
	return 0;
}

struct mf_uint16_t *
genmf_fr_path(BDD node, struct mf_uint16_t *mf) {
	int var = bdd_var(node);
	// printf("var: %d\n", var);
	int vardiv16 = FRA2INT(var/16);
	struct mf_uint16_t *tmp = copy_mf_uint16_t(mf);
	for (int i = vardiv16; i < MF_LEN; i++){
		if (i == vardiv16){
			uint16_t w_sign = 0x0000;
			for (int j = (var%16)+1; j < 16; j++){
				w_sign += var2sign[j];
			}
			tmp->mf_w[i] = tmp->mf_w[i] | w_sign;
			tmp->mf_v[i] = (~w_sign) & tmp->mf_v[i];
		}
		else {
			tmp->mf_w[i] = 0xffff;
			tmp->mf_v[i] = 0x0000;
		}
	} 
	return tmp;
}

int
back_node_2_mf(BDD node, struct mf_uint16_t *mf) {
	if(node == -1){
		init_mf(mf);
		return 0;
	}
	int var = bdd_var(node);
	int vardiv16 = FRA2INT(var/16);
	// printf("var: %d\n", var);
	for (int i = vardiv16; i < MF_LEN; i++){
		if (i == vardiv16){
			uint16_t w_sign = 0x0000;
			for (int j = (var%16)+1; j < 16; j++){
				w_sign += var2sign[j];
			}
			w_sign = ~w_sign;
			mf->mf_w[i] = mf->mf_w[i] & w_sign;
			mf->mf_v[i] = w_sign & mf->mf_v[i];
		}
		else {
			mf->mf_w[i] = 0x0000;
			mf->mf_v[i] = 0x0000;
		}
	}
	return 0;
}


struct mf_uint16_t_array *
bdd2mf(BDD root, int varnum) {

	if((LOW(root) == 1)&&(HIGH(root) == 1)){
		struct mf_uint16_t_array *mf_arr_tmp = xmalloc(sizeof *mf_arr_tmp);
		mf_arr_tmp->n_mfs = 1;
		mf_arr_tmp->mfs[0] = xcalloc (1,sizeof(struct mf_uint16_t));
		for (int i = 0; i < MF_LEN; i++){
			mf_arr_tmp->mfs[0]->mf_w[i] = 0xffff;
			mf_arr_tmp->mfs[0]->mf_v[i] = 0x0000;
		}
		return mf_arr_tmp;
	}
	BDD record[varnum + 2];
	for (int i = 0; i < varnum; i++)
		record[i] = -1;
	struct mf_uint16_t *arr_tmp[10*varnum];
	int arr_count = 0;
	struct mf_uint16_t *mf_tmp = xcalloc (1, sizeof *mf_tmp);
	init_mf(mf_tmp);
	BDD node_tmp;
	record[0] = -1;
	record[1] = root;
	int curr_pos = 1;
	// printf("this work\n");
	while(curr_pos){
		node_tmp = record[curr_pos];
		if (HIGH(node_tmp) == record[curr_pos+1]){
			// printf("this work1 %d\n", curr_pos );
			if (LOW(node_tmp) == 1){
				// int var = bdd_var(node_tmp);				
				add_node_2_mf(record[curr_pos], record[curr_pos-1], mf_tmp, 0);
				arr_tmp[arr_count] = genmf_fr_path(record[curr_pos], mf_tmp);
				arr_count++;

				record[curr_pos + 1] = 1;
				back_node_2_mf(record[curr_pos], mf_tmp);
			}
			else if ((LOW(node_tmp) == 0)){
				record[curr_pos + 1] = 0;
			}
			else {
				add_node_2_mf(record[curr_pos], record[curr_pos-1], mf_tmp, 0);
				curr_pos++;
				record[curr_pos] = LOW(node_tmp);
			}
		}
		else if (LOW(node_tmp) == record[curr_pos+1]){
			record[curr_pos + 1] = -1;
			curr_pos--;
			// printf("this work3 %d\n", curr_pos);
			if(curr_pos)
				back_node_2_mf(record[curr_pos-1], mf_tmp);
		}
		else{
			// printf("this work2 %d\n", curr_pos);
			if (HIGH(node_tmp) == 1){
				// int var = bdd_var(node_tmp);
				add_node_2_mf(record[curr_pos], record[curr_pos-1], mf_tmp, 1);
				arr_tmp[arr_count] = genmf_fr_path(record[curr_pos], mf_tmp);
				arr_count++;

				record[curr_pos + 1] = 1;
				back_node_2_mf(record[curr_pos], mf_tmp);
			}
			else if ((HIGH(node_tmp) == 0)){
				record[curr_pos + 1] = 0;
			}
			else {
				add_node_2_mf(record[curr_pos], record[curr_pos-1], mf_tmp, 1);
				curr_pos++;
				record[curr_pos] = HIGH(record[curr_pos-1]);
			}
		}
	}

	struct mf_uint16_t_array *tmp = (struct mf_uint16_t_array *)xmalloc(sizeof(uint32_t)+arr_count*sizeof(mf_tmp));	
	tmp->n_mfs = arr_count;
	for (int i = 0; i < arr_count; i++){
		tmp->mfs[i] = arr_tmp[i];
	}
	return tmp;
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
	mf1->mf_w[1] = 0xfaff;
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

    BDD v1, v2, v_op;
    int domain[2] = {16*MF_LEN, 16*MF_LEN};
    bdd_init(2000,100);
    // fdd_extdomain(domain, 2);

    int varnum = 16*MF_LEN;
    bdd_setvarnum(varnum);//变量的数目，也就是二进制位数。
    // v1 = 1;
    v1 = mf2bdd_init(mf1);
    // v1 = mf2bdd(mf1);
    v2 = mf2bdd(mf2);
    // v_op = bdd_and(v1, v2);
	// v_op = bdd_or(v1, v2);
	v_op = bdd_apply(v1, v2, bddop_or);

	struct mf_uint16_t_array *mfarr = bdd2mf(v1, varnum);
	print_mf_uint16_t_array(mfarr);
	struct mf_uint16_t_array *mfarr2 = bdd2mf(v2, varnum);
	print_mf_uint16_t_array(mfarr2);
	struct mf_uint16_t_array *mfarr_op = bdd2mf(v_op, varnum);
	print_mf_uint16_t_array(mfarr_op);

    bdd_printtable(v1);
    bdd_printtable(v2);
    bdd_printtable(v_op);



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