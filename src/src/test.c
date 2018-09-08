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
#define REF(a)    (bddnodes[a].refcou)

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
				}
				else{
					root = bdd_makenode(level, tmp, 0);//生成相应变量的一个节点
				}
				tmp = root;
			}
			sign <<= 1;
		}
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
  CHECK(root);
  if(root == 1){
    struct mf_uint16_t_array *mf_arr_tmp = xmalloc(sizeof *mf_arr_tmp);
    mf_arr_tmp->n_mfs = 1;
    mf_arr_tmp->mfs[0] = xcalloc (1,sizeof(struct mf_uint16_t));
    for (int i = 0; i < MF_LEN; i++){
      mf_arr_tmp->mfs[0]->mf_w[i] = 0xffff;
      mf_arr_tmp->mfs[0]->mf_v[i] = 0x0000;
    }
    return mf_arr_tmp;
  }
  if(root == 0)
    return NULL;

  BDD record[varnum + 2];
  for (int i = 0; i < varnum; i++)
    record[i] = -1;
  struct mf_uint16_t *arr_tmp[20*varnum];
  int arr_count = 0;
  struct mf_uint16_t *mf_tmp = xcalloc (1, sizeof *mf_tmp);
  init_mf(mf_tmp);
  BDD node_tmp;
  record[0] = -1;
  record[1] = root;
  int curr_pos = 1;

  while(curr_pos){
    node_tmp = record[curr_pos];
    if (HIGH(node_tmp) == record[curr_pos+1]){
      if (LOW(node_tmp) == 1){      
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 0);
        arr_tmp[arr_count] = genmf_fr_path(node_tmp, mf_tmp);
        arr_count++;

        record[curr_pos + 1] = 1;
        back_node_2_mf(record[curr_pos-1], mf_tmp);
      }
      else if ((LOW(node_tmp) == 0)){
        record[curr_pos + 1] = 0;
      }
      else {
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 0);
        curr_pos++;
        record[curr_pos] = LOW(node_tmp);
      }
    }
    else if (LOW(node_tmp) == record[curr_pos+1]){
      record[curr_pos + 1] = -1;
      curr_pos--;
      if(curr_pos)
        back_node_2_mf(record[curr_pos-1], mf_tmp);
    }
    else{
      if (HIGH(node_tmp) == 1){
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 1);
        arr_tmp[arr_count] = genmf_fr_path(node_tmp, mf_tmp);
        arr_count++;

        record[curr_pos + 1] = 1;
        back_node_2_mf(record[curr_pos-1], mf_tmp);
      }
      else if ((HIGH(node_tmp) == 0)){
        record[curr_pos + 1] = 0;
      }
      else {
        add_node_2_mf(node_tmp, record[curr_pos-1], mf_tmp, 1);
        curr_pos++;
        record[curr_pos] = HIGH(node_tmp);
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

BDD 
bdd_ref(BDD root)
{
   CHECK(root);
   if (root < 2)
      return bdd_error(BDD_ILLBDD);
   return (REF(root));
}

void
print_node(BDD r) {
  if (r < 2)
    printf("base node: %d\n", r);
  else{
    printf("num: %d; ", r);
    printf("low: %d; ", bdd_low(r));
    printf("high: %d; ", bdd_high(r));
    printf("var: %d; ", bdd_var(r));
    printf("refcou: %d\n", bdd_ref(r));
  }
}




struct BddNode_saved {
	int var;
	int low;
	int high;
};

struct bdd_saved_arr {//当arr_num == 0,意味着只有0或者1节点。
	int arr_num;
	struct BddNode_saved bdd_s[0];
};

int
cmp_bdd_by_var(const void *a, const void *b)
{	return bdd_var(*(int*)b) - bdd_var(*(int*)a);}

void
bdd_save2stru(BDD root, BDD *r, int *count) {
	BddNode *node;

	if (root < 2)
		return;

	node = &bddnodes[root];
	if ((node)->level & MARKON  ||  LOWp(node) == -1)
		return;

	(node)->level |= MARKON;
	r[*count] = root;
	(*count)++;
	bdd_save2stru(LOWp(node), r, count);
	bdd_save2stru(HIGHp(node), r, count);	
}

struct bdd_saved_arr *
bdd_save_arr(BDD root){
	if (root < 2){
		if (root == 1){
			struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+sizeof(struct BddNode_saved));
			tmp->arr_num = 1;
			tmp->bdd_s[0].var = 1;
			tmp->bdd_s[0].low = -1;
			tmp->bdd_s[0].high = -1;
			return tmp;
		}
		if (root == 0){
			struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+sizeof(struct BddNode_saved));
			tmp->arr_num = 1;
			tmp->bdd_s[0].var = 0;
			tmp->bdd_s[0].low = -1;
			tmp->bdd_s[0].high = -1;
			return tmp;
		}
		else
			return NULL;
	}
	int count = 0;

	BDD arr_tmp[16*MF_LEN*10];

	bdd_save2stru(root, arr_tmp, &count);
	bdd_unmark(root);
	qsort(arr_tmp, count,sizeof (BDD), cmp_bdd_by_var);
	int var_arr[16*MF_LEN];
	int var_tmp = 16*MF_LEN - 1;
	var_arr[var_tmp] = 0;
	
	for (int i = 0; i < count; i++){
		int var = bdd_var(arr_tmp[i]);
		if (var != var_tmp){
			for (int j = var; j < var_tmp; j++)
				var_arr[j] = i;
			var_tmp = var;
		}
	}
	if (var_tmp != 0){
		for (int i = 0; i < var_tmp; i++)
			var_arr[i] = count;
	}

	struct bdd_saved_arr *tmp = xmalloc(sizeof(int)+(count+2)*sizeof(struct BddNode_saved));
	tmp->arr_num = count+2;
	tmp->bdd_s[0].var = -1;
	tmp->bdd_s[0].low = -1;
	tmp->bdd_s[0].high = -1;
	tmp->bdd_s[1].var = -1;
	tmp->bdd_s[1].low = -1;
	tmp->bdd_s[1].high = -1;
	for (int i = 0; i < count; i++){
		int var = bdd_var(arr_tmp[i]);
		int low = LOW(arr_tmp[i]);
		int high = HIGH(arr_tmp[i]);
		tmp->bdd_s[i+2].var = var;
		if (low < 2)
			tmp->bdd_s[i+2].low = low;
		else {
			int low_var = bdd_var(low);
			if (low_var){
				for (int j = var_arr[low_var]; j < var_arr[low_var - 1]; j++){
					if (low == arr_tmp[j]){
						tmp->bdd_s[i+2].low = j+2;
						break;
					}
				}
			}
			else {
				for (int j = var_arr[low_var]; j < count; j++){
					if (low == arr_tmp[j]){
						tmp->bdd_s[i+2].low = j+2;
						break;
					}
				}
			}
		}
		if (high < 2)
			tmp->bdd_s[i+2].high = high;
		else {
			int high_var = bdd_var(high);
			if (high_var != 0){
				for (int j = var_arr[high_var]; j < var_arr[high_var - 1]; j++){
					if (high == arr_tmp[j]){
						tmp->bdd_s[i+2].high = j+2;
						break;
					}
				}
			}
			else {
				for (int j = var_arr[high_var]; j < count; j++){
					if (high == arr_tmp[j]){
						tmp->bdd_s[i+2].high = j+2;
						break;
					}
				}
			}
		}
	}
	return tmp;
}

BDD
load_saved_bddarr(struct bdd_saved_arr *bdd_arr) {
	if (bdd_arr->arr_num == 1)
		return bdd_arr->bdd_s[0].var;

	BDD arr_tmp[bdd_arr->arr_num];
	arr_tmp[0] = 0;
	arr_tmp[1] = 1;
	for (int i = 2; i < bdd_arr->arr_num; i++){

		int level = bdd_var2level(bdd_arr->bdd_s[i].var);
		arr_tmp[i] = bdd_makenode(level, arr_tmp[bdd_arr->bdd_s[i].low], arr_tmp[bdd_arr->bdd_s[i].high]);
	}
	return arr_tmp[bdd_arr->arr_num-1];

}

bool
bddarr_is_insc (struct bdd_saved_arr *a, struct bdd_saved_arr *b) {
	if (a->arr_num == 1) {
		/* code */
	}
	if (b->arr_num == 1)
	{
		/* code */
	}
	if (l == r)
	  return l;
       if (ISZERO(l)  ||  ISZERO(r))
	  return 0;
       if (ISONE(l))
	  return r;
       if (ISONE(r))
	  return l;
       break;

	BddCacheData *entry;
   BDD res;
   
   switch (applyop)
   {
    case bddop_and:
       if (l == r)
	  return l;
       if (ISZERO(l)  ||  ISZERO(r))
	  return 0;
       if (ISONE(l))
	  return r;
       if (ISONE(r))
	  return l;
       break;
    case bddop_or:
       if (l == r)
	  return l;
       if (ISONE(l)  ||  ISONE(r))
	  return 1;
       if (ISZERO(l))
	  return r;
       if (ISZERO(r))
	  return l;
       break;
    case bddop_xor:
       if (l == r)
	  return 0;
       if (ISZERO(l))
	  return r;
       if (ISZERO(r))
	  return l;
       break;
    case bddop_nand:
       if (ISZERO(l) || ISZERO(r))
	  return 1;
       break;
    case bddop_nor:
       if (ISONE(l)  ||  ISONE(r))
	  return 0;
       break;
   case bddop_imp:
      if (ISZERO(l))
	 return 1;
      if (ISONE(l))
	 return r;
      if (ISONE(r))
	 return 1;
      break;
   }

   if (ISCONST(l)  &&  ISCONST(r))
      res = oprres[applyop][l<<1 | r];
   else
   {
      entry = BddCache_lookup(&applycache, APPLYHASH(l,r,applyop));
      
      if (entry->a == l  &&  entry->b == r  &&  entry->c == applyop)
      {
#ifdef CACHESTATS
	 bddcachestats.opHit++;
#endif
	 return entry->r.res;
      }
#ifdef CACHESTATS
      bddcachestats.opMiss++;
#endif
      
      if (LEVEL(l) == LEVEL(r))
      {
	 PUSHREF( apply_rec(LOW(l), LOW(r)) );
	 PUSHREF( apply_rec(HIGH(l), HIGH(r)) );
	 res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
      }
      else
      if (LEVEL(l) < LEVEL(r))
      {
	 PUSHREF( apply_rec(LOW(l), r) );
	 PUSHREF( apply_rec(HIGH(l), r) );
	 res = bdd_makenode(LEVEL(l), READREF(2), READREF(1));
      }
      else
      {
	 PUSHREF( apply_rec(l, LOW(r)) );
	 PUSHREF( apply_rec(l, HIGH(r)) );
	 res = bdd_makenode(LEVEL(r), READREF(2), READREF(1));
      }

      POPREF(2);

      entry->a = l;
      entry->b = r;
      entry->c = applyop;
      entry->r.res = res;
   }

   return res;
}




BDD
bdd_v2x_byvar(BDD root, struct mask_uint16_t *mask) {
	BddNode *node;

	if (root < 2)
		return;
	if
	node = &bddnodes[root];
	if ((node)->level & MARKON  ||  LOWp(node) == -1)
		return;

	(node)->level |= MARKON;
	r[*count] = root;
	(*count)++;
	bdd_save2stru(LOWp(node), r, count);
	bdd_save2stru(HIGHp(node), r, count);
}

BDD
bdd_v2x_byvar(BDD root, struct mask_uint16_t *mask) {

	int var = bdd_var(root)
	if (root < 2)
		return root;


	if (root < mask)
		return bdd_apply(bdd_v2x_byvar(LOW(root), mask), bdd_v2x_byvar(HIGH(root), mask), bddop_or);
	else{
		int level = bdd_var2level(var);
		return bdd_makenode(level, bdd_v2x_byvar(LOW(root), mask), bdd_v2x_byvar(HIGH(root), mask));
	}
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

	// mf2->mf_w[0] = 0x0fff;
	// mf2->mf_w[1] = 0xffff;
	// mf2->mf_v[0] = 0x3000;
	// mf2->mf_v[1] = 0x0000;

    BDD v1, v2;
    bdd_init(2000,100);
    // fdd_extdomain(domain, 2);

    int varnum = 16*MF_LEN;
    bdd_setvarnum(varnum);//变量的数目，也就是二进制位数。

    // v1 = bdd_ithvar(0);
    // v1 = mf2bdd_init(mf1);
    v1 = mf2bdd(mf1);
    // bdd_addref(v1);

    v2 = mf2bdd(mf2);
    
	BDD v_or = bdd_or(v1, v2);
	// BDD v_diff = bdd_apply(v1, v2, bddop_diff);

	// BDD v_and = bdd_apply(v1, v2, bddop_and);
	// BDD v_op = bdd_apply(v1, v_and, bddop_diff);
	// bdd_addref(v_and);
	// struct mf_uint16_t_array *mfarr = bdd2mf(v1, varnum);
	// print_mf_uint16_t_array(mfarr);
	// free_mf_uint16_t_array(mfarr);
	// mfarr = bdd2mf(v2, varnum);
	// print_mf_uint16_t_array(mfarr);
	// free_mf_uint16_t_array(mfarr);
	// mfarr = bdd2mf(v_diff, varnum);
	// print_mf_uint16_t_array(mfarr);
	// free_mf_uint16_t_array(mfarr);
	// mfarr = bdd2mf(v_op, varnum);
	// print_mf_uint16_t_array(mfarr);
	// free_mf_uint16_t_array(mfarr);
	struct mf_uint16_t_array *mfarr = bdd2mf(v_or, varnum);
	print_mf_uint16_t_array(mfarr);

	// bdd_printtable(v_or);
	struct bdd_saved_arr *tmp = bdd_save_arr(v_or);
	bdd_gbc();
	v_or = load_saved_bddarr(tmp);
	// bdd_printtable(v_or);
	mfarr = bdd2mf(v_or, varnum);
	print_mf_uint16_t_array(mfarr);



    // print_node(v1);
    // bdd_delref(v_and);
    // bdd_gbc();
    // bdd_printtable(v1);
    // bdd_printtable(v2);
    // bdd_printtable(v_and);

    // print_node(v1);
    // bdd_printtable(v2);
    // print_node(v_op);
    // print_node(v2);



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
    bdd_done();
}