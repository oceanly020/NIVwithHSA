//集中include
#include "all.h"
#include <pthread.h>
#include <limits.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>  

//函数声明
void print_bin(uint16_t mf);
// struct mf_uint16_t *mf_from_str (const char *s);
void array_free (array_t *a);
static void add_rule (struct parse_sw *sw, struct parse_rule *r);
static void free_rule (struct parse_rule *r);
static void free_sw (struct parse_sw *sw);
static void free_nsw (struct parse_nsw *nsw);
static int filter_tfs (const struct dirent *ent);
static struct arr_ptr_uint32_t read_array (char *s, uint32_t *res);
static struct parse_sw * parse_tf (const char *name);
void parse_dir (const char *outdir, const char *tfdir, const char *name);
// void rule_data_gen (const char *name, const struct parse_nsw *nsw)

//apps
void print_bin(uint16_t mf)
{
    int l = sizeof(mf)*8;//总位数。
    uint16_t bool_sign = 0x8000; 
    for(int i = 0 ; i < l; i++, bool_sign >>= 1)
        printf("%d", (mf & bool_sign) != 0);
}


//array.c
struct mf_uint16_t *
mf_from_str (const char *s)//每个数组的数都为一组uint32_t的匹配域中的一个,一一对应
{
	// bool commas = strchr (s, ',');//查找某字符在字符串中首次出现的位置
	// int div = CHAR_BIT * 2; //+ commas;// CHAR_BIT 8位
	// int len = strlen (s); //+ commas;//返回长度
	// assert (len % div == 0);


	
	// len /= div;//字节数
	const char *cur = s;
	// array_t *res = array_create (len, BIT_UNDEF);
	struct mf_uint16_t *mf = xcalloc (1, sizeof *mf);
	
	
	// uint8_t *rcur = (uint8_t *) res;
	for (int i = 0; i < MF_LEN; i++) {
		uint16_t tmp_w = 0;
		uint16_t tmp_v = 0;
		uint16_t bool_sign = 0x8000; 
		// for (int j = 0; j < CHAR_BIT / 2; j++, cur++) {
		for (int j = 0; j < CHAR_BIT * 2 + 2; j++, cur++) {
	// 		enum bit_val val;
			switch (*cur) {	
				case '0': 
					break;
				case '1': 
					tmp_v += bool_sign; break;
				case 'x': case 'X': 
					tmp_w += bool_sign; break;
				case 'z': case 'Z':
					return NULL; break;
				case ',':
					continue;
				default: errx (1, "Invalid character '%c' in \"%s\".", *cur, s);
			}
			bool_sign >>= 1;
			if (!bool_sign){
				cur++;
				break;
			}
	// 		tmp |= val;
		}
		mf->mf_w[i] = tmp_w;
		mf->mf_v[i] = tmp_v;

	// 	*rcur++ = tmp;
	// 	if (commas && (i % 2)) { assert (!*cur || *cur == ','); cur++; }
	}
	// return res;
	return mf;
}

//data.c
static uint32_t mf_len;

static int
rule_cmp (const void *va, const void *vb) {
	const struct rule *a = va, *b = vb;
	// if ((a->in < 0 && b->in < 0) || a->in == b->in) 
	return a->idx - b->idx;
	// return a->in - b->in;
}

static int
arr_cmp (const void *a, const void *b) 
{	return memcmp (a, b, 2*MF_LEN); }

bool
array_is_eq (const uint16_t *a, const uint16_t *b, int len) //uint16_t
{	return !memcmp (a, b, len); }

static uint32_t
arr_find (const struct mf_uint16_t *a, const uint16_t *arrs, int n, int sign) {
	if (!a) return 0;//match值
	array_t *b;
	if (sign)
		b = bsearch (a->mf_v, arrs, n, 2*MF_LEN, arr_cmp);
	else
		b = bsearch (a->mf_w, arrs, n, 2*MF_LEN, arr_cmp);
	//指向要查找元素的指针a，指向进行查找的数组的第一个对象的指针类型转换为 void* arrs
	//arrs元素个数n，每个元素大小len，arr_cmp比较
	//数据必须是经过预先排序的，而排序的规则要和comp所指向比较子函数的规则相同。
	//如果找到元素则返回指向该元素的指针，否则返回NULL
	assert (b);//如果b没有值，出错
	// return VALID_OFS + ((uint8_t *)b - (uint8_t *)arrs);//（指向uint8_t）//uint16_t为一个单位
	return ((uint8_t *)b - (uint8_t *)arrs);
	//返回1 + (指向一字节b - 指向一字节的arrs），b为指针，指向查找，arrs为第一个对象指针，相当于求出偏移量
}

// static uint32_t
// gen_ports (const uint32_t *arr, uint32_t n, FILE *f_ports)
// {
//   if (!n) return 0;
//   if (n == 1) return arr[0];

//   int32_t ret = -(VALID_OFS + ftell (f_ports));
//   fwrite (&n, sizeof n, 1, f_ports);
//   fwrite (arr, sizeof *arr, n, f_ports);
//   return ret;
// } 

static uint16_t *
gen_arrs (const struct parse_nsw *nsw, uint32_t *n) {
	char *buf, *buf2;
	size_t bufsz, buf2sz;
	FILE *f = open_memstream (&buf, &bufsz);//buf和buf大小
	/* uint32_t len = ARRAY_BYTES (arr_len);//数组长度,4字节一位,arr_len = ntf->tfs[0]->len,tfs[0]中多少个
	//arr_len为中间matchfield长度，arr_len = len/4；4bit1位，len 为match的长度
	//在这里就是arr_len×2,即为多少个二进制位，同时每个二进制位是单独的枚举类型保存包含x，z*/

	uint32_t len = 2*MF_LEN;//多少字节
	// uint32_t match_len = 2*mf_len //加上通配符
	int count = 0;//计数match
	for (int i = 0; i < nsw->sws_num; i++) {//0到n，所有tf		
		const struct parse_sw *sw = nsw->sws[i];
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {//一直往下一个走，直到没有下一个
			//head为链表的头指针，可以使用head.next指向链表下一个。
			// if (r->idx == 1) {
			// 	print_mf_uint16_t(r->match);
			// }
			assert (r->match);//uint16_t的数组
			//将所有规则写到f中，长度len为一位
			fwrite (r->match->mf_w, len, 1, f);
			fwrite (r->match->mf_v, len, 1, f);
			count += 2;//位置，每个len，位置+1
			if (r->mask) {
				fwrite (r->mask->mf_v, len, 1, f);
				// fwrite (r->rewrite->mf_w, len, 1, f);//是否有x
				fwrite (r->rewrite->mf_v, len, 1, f);

				count += 2;
			}
			// for (struct parse_dep *dep = r->deps.head; dep; dep = dep->next) {//依赖的
			// 	fwrite (dep->match, len, 1, f);
			// 	count++;
			// }
		}
	}
	fclose (f);//f中保存所有rule的match，mask，rewrite，依赖规则的的match。每位代表着什么并不知道
	printf ("Arrays: %d (%zu)", count, bufsz);//总数量
	fflush (stdout);//清空文件缓冲区，如果文件是以写的方式打开 的，则把缓冲区内容写入文件
	//stdout为文件指针
	//也可用于标准输入（stdin）和标准输出（stdout），用来清空标准输入输出缓冲区。
	assert (count * len == bufsz);

	qsort (buf, count, len, arr_cmp); //快速排序函数，排序数组buf，元素个数count，单个元素大小len，比较函数。
	//以match为单位排序

	uint16_t *arrs = (uint16_t *) buf; //指向buf，文件数组
	int count2 = 0, last = -1;
	f = open_memstream (&buf2, &buf2sz);

	for (int i = 0; i < count * MF_LEN; i += MF_LEN) {
		//计数乘以MF_LEN，i+MF_LEN，i，按match所占uint16_t位数循环，在前面的文件中
		if (last != -1 && array_is_eq (&arrs[i], &arrs[last], len)) continue;
		//如果不是第一次，且，&arrs[i], &arrs[i-1]相等，就跳过
		fwrite (&arrs[i], len, 1, f);
		last = i;
		count2++;
	}

	fclose (f);
	free (buf);//在这里才关闭buf

	printf (" -> %d (%zu)\n", count2, buf2sz);
	assert (count2 * len == buf2sz);

	*n = count2;
	return (uint16_t *) buf2;
}

// static int
// uint32_t_cmp (const void *a, const void *b)
// { return *(uint32_t *)a - *(uint32_t *)b; }

static int
uint16_t_cmp (const void *a, const void *b)
{ return *(uint16_t *)a - *(uint16_t *)b; }//升序

static uint32_t
linkwc_gen (uint32_t *arr, uint32_t n, FILE *f_links) {//调用parr = ARR (r->in)，n = r->in.n;
	//返回的ret为位置，文件内的位置，需要加上首位置
	if (!n) return 0;
	// int32_t ret = -(VALID_OFS + ftell (f_links));
	int32_t ret =  ftell (f_links);
	uint16_t w_all0 = 0;
	uint32_t *parr = arr;
	uint16_t arr_idx[n];
	for (int j = 0; j < n; j++, parr++) {
		arr_idx[j] = (uint16_t)(*parr);	
	}
	if (n == 1) {
		fwrite (&n, sizeof n, 1, f_links);//第一个
		fwrite (&w_all0, sizeof w_all0, 1, f_links);
		fwrite (arr_idx, sizeof *arr_idx, n, f_links);
		return ret;
	}
	qsort(arr_idx, n, 2, uint16_t_cmp);

	// for (int j = 0; j < n; j++, parr++) {
	// 	printf("%d, ", arr_idx[j]);
	// }
	// printf("\n");
	
	struct wc_uint16_t links_wc[n];
	uint16_t begin = arr_idx[0];
	uint16_t tmp = begin;
	uint32_t wc_nums = 0;

	for (int i = 1; i < n; i++)
	{
		if ((++tmp != arr_idx[i]) || (i == n-1)) {
			if ((tmp - 1 == begin)&&(i != n-1)) {
				links_wc[wc_nums].w = w_all0;
				links_wc[wc_nums].v = begin;
				wc_nums++;
				begin = arr_idx[i];
				tmp = begin;
				continue;
			}
			char v_h, v_l;
			uint16_t vh_arr = 0, vl_arr = 0;
			uint16_t v_sign = 0x8000;
			char first_sign = 1;
			// printf("%d - %d\n", begin,tmp);
			for (int j = 0; j < 15; j++)//sizeof tmp
			{
				v_h = (tmp & v_sign) && 1;
				v_l = (begin & v_sign) && 1;
				if (first_sign) {
					if (v_h == v_l)	{
						if (v_h == 1) {
							vh_arr += v_sign;						
						}						
					}
					else {
						vl_arr = vh_arr;
						first_sign = 0;
						vh_arr += v_sign;
					}
				}
				else {
					if (v_l) {
						vl_arr += v_sign;
					}
					else {
						// uint16_t w = v_sign - 1;//后面补x
						links_wc[wc_nums].w = v_sign - 1;
						links_wc[wc_nums].v = vl_arr + v_sign;
						wc_nums++;
					}
					if (v_h) {
						links_wc[wc_nums].w = v_sign - 1;
						links_wc[wc_nums].v = vh_arr;
						wc_nums++;
						vh_arr += v_sign;
					}
				}
				v_sign >>= 1;
			}
			v_h = (tmp & v_sign) && 1;
			v_l = (begin & v_sign) && 1;
			if (first_sign){
				if (v_h != v_l) {
					links_wc[wc_nums].w = v_sign;
					links_wc[wc_nums].v = vl_arr;
					wc_nums++;						
				}
				else if (v_h == 1) {
					links_wc[wc_nums].w = 0;
					links_wc[wc_nums].v = vl_arr + v_sign;
					wc_nums++;
				}
				else {
					links_wc[wc_nums].w = 0;
					links_wc[wc_nums].v = vh_arr;
					wc_nums++;
				}
			}
				else{
				if (v_l) {
					links_wc[wc_nums].w = 0;
					links_wc[wc_nums].v = vl_arr + v_sign;
					wc_nums++;
				}
				else {
					// uint16_t w = v_sign - 1;//后面补x
					links_wc[wc_nums].w = v_sign;
					links_wc[wc_nums].v = vl_arr;
					wc_nums++;
				}
				if (v_h) {
					links_wc[wc_nums].w = v_sign;
					links_wc[wc_nums].v = vh_arr;
					wc_nums++;
				}
				else {
					links_wc[wc_nums].w = 0;
					links_wc[wc_nums].v = vh_arr;
					wc_nums++;
				}
				// wc_nums++;
				begin = arr_idx[i];
				tmp = begin;
			}


		}
	}
	// for (int i = 0; i < wc_nums; i++)
	// {	
	// 	printf("%d - %d", links_wc[i].w, links_wc[i].v);
	// 	printf(", ");
	// }
	// printf("\n");

	// for (int i = 0; i < wc_nums; i++)
	// {	
	// 	print_wc(&links_wc[i].w, &links_wc[i].v);
	// 	printf(", ");
	// }
	// printf("ret is %d", ret);
	// printf("\n");
	fwrite (&wc_nums, sizeof wc_nums, 1, f_links);
	fwrite (links_wc, sizeof *links_wc, wc_nums, f_links);
	return ret;
}

static void
gen_sw (const struct parse_sw *sw, FILE *out, FILE *f_strs, const uint16_t *arrs,
        int narrs, const uint32_t sw_idx) 
{
	// char *buf_deps, *buf_ports;//依赖和端口
	// size_t sz_deps, sz_ports;

	char *buf_links;
	size_t sz_links;
	FILE *f_ports = open_memstream (&buf_links, &sz_links);
	// FILE *f_deps = open_memstream (&buf_deps, &sz_deps);

	int start = ftell (out);//返回当前文件指针位置
	struct sw hdr = {sw_idx, ftell (f_strs) + VALID_OFS, sw->nrules};//创建tf，文件f_strs位置+1
	//就是prefix开始位置，同时填入tf->nrules

	if (sw->prefix) fwrite (sw->prefix, 1, strlen (sw->prefix) + 1, f_strs);//tf名称加在tf实体前面写到f_strs
	else hdr.prefix = 0;
	fwrite (&hdr, sizeof hdr, 1, out); //写uint32_t prefix;uint32_t nrules;插入结构体占位
	/* TODO: Alignment? */

	struct of_rule rules[hdr.nrules];//hdr.nrules个rule空间指向各自rules
	// //构体指针变量一般用”->”，非结构体指针变量，也就是一般结构体变量，一般用”.”。
	memset (rules, 0, sizeof rules);//初始化0
	// uint16_t *mf_c;
	int i = 0;
	for (struct parse_rule *r = sw->rules.head; r; r = r->next, i++) {//选取链表中所有r,从链表转到数组
		struct of_rule *tmp = &rules[i];//指向相应位置，填入到rules中
		tmp->sw_idx = sw_idx;//从0开始，对应str
		tmp->idx = r->idx;
		// tmp->in = gen_ports (ARR (r->in), r->in.n, f_ports, INPORT);//( (X).n > ARR_LEN ((X).e.a) ? (X).e.p : (X).e.a )
		// tmp->out = gen_ports (ARR (r->out), r->out.n, f_ports, OUTPORT);//生成port号
		tmp->match.w = arr_find (r->match, arrs, narrs, WILDCARD);//从arrs中找到match
		// if(r->idx==1){
		// 	uint32_t k_s=tmp->match.w;
		// 	for (int k = 0; k < MF_LEN; k++)
		// 	{
		// 		printf("%x;", *(uint16_t *)((uint8_t *)arrs+k_s));
		// 		k_s+=2;
		// 	}
		// 	printf("\n");
		// }

		tmp->match.v = arr_find (r->match, arrs, narrs, VALUE);//从arrs中找到match
		tmp->mask = arr_find (r->mask, arrs, narrs, VALUE);//预处理，在arrs中查询，应该是生成号，作为hash查询	
		//如果找到元素则返回指向该元素的指针，否则返回NULL，arrs为没有重复的头
		// tmp->rewrite.w = arr_find (r->rewrite, arrs, narrs, WILDCARD);
		// tmp->rewrite.v = arr_find (r->rewrite, arrs, narrs, VALUE);
		tmp->rewrite = arr_find (r->rewrite, arrs, narrs, VALUE);//是否有x
		// // if (r->deps.head) tmp->deps = gen_deps (&r->deps, f_deps, f_ports, arrs, narrs);//如果有依赖，建立
		// //tmp->desc = barfoo;
		// gen_link
		// if (i<3)
		// {
		// 	tmp->in_link = linkwc_gen (ARR (r->in), r->in.n, f_ports);
		// }
		tmp->in_link = linkwc_gen (ARR (r->in_link), r->in_link.n, f_ports);
		if(r->idx==1){
			printf("in_link is %x;\n",tmp->in_link);
			printf("%d\n", ARR (r->in_link)[0]);
		}
		tmp->out_link = linkwc_gen (ARR (r->out_link), r->out_link.n, f_ports);
		if(r->idx==1){
			printf("out_link is %x;\n",tmp->out_link);
			printf("%d\n", ARR (r->out_link)[0]);
		}
		// struct rule_links *rl_out = linkwc_gen (r);
		// tmp->out_link = 0;
		
	}
	fclose (f_ports);
	// fclose (f_deps);

	qsort (rules, hdr.nrules, sizeof *rules, rule_cmp);//排序
	fwrite (rules, hdr.nrules, sizeof *rules, out);//把排序过后的规则填入文件

	// hdr.map_ofs = ftell (out) - start;
	// gen_map (out, &tf->in_map, rules, ARR_LEN (rules));

	hdr.links_ofs = ftell (out) - start;
	printf("links_ofs %d\n", hdr.links_ofs);
	fwrite (buf_links, 1, sz_links, out);
	free (buf_links);

	// hdr.deps_ofs = ftell (out) - start;
	// fwrite (buf_deps, 1, sz_deps, out);
	// free (buf_deps);

	int end = ftell (out);
	fseek (out, start, SEEK_SET);//移动读写位置到开始
	fwrite (&hdr, sizeof hdr, 1, out);//把hdr写入到文件
	fseek (out, end, SEEK_SET);//移动读写位置到最后
}

void
rule_data_gen (const char *name, const struct parse_nsw *nsw) {//name是路径就是.dat
	FILE *out = fopen (name, "w");//建立文件out，并可写
	if (!out) err (1, "Can't open output file %s", name);

	int sws_num = nsw->sws_num; //+ 1;//个数加1
	char *buf_strs;
	size_t sz_strs;
	FILE *f_strs = open_memstream (&buf_strs, &sz_strs);//写文件，建立的为一个文件结构，并不是真的文件，打开一个流

	uint32_t narrs;
	// arr_len = nsw->sws[0]->len;//tfs[0]的长度
	uint16_t *arrs = gen_arrs (nsw, &narrs);//对tfs写，arrs返回首位置，narrs返回计数，有多少个arrs

	int hdr_size = offsetof (struct file, sw_ofs[sws_num]);//一个结构成员的结构从一开始的字节偏移
	//这里就是文件的大小，sw_ofs[nsw->sws_num + 1]的偏移,以字节算，
	//sw_ofs对一开始的偏移uint32_t arrs_ofs, strs_ofs;uint32_t ntfs, stages;为16字节，加上sw_ofs数组

	struct file *hdr = xmalloc (hdr_size);//文件结构的hdr，分配空间为偏移量
	memset (hdr, 0, hdr_size);//将hdr所指向的hdr_size大小的某一块内存中的每个字节初始化为0的ASCII值
	hdr->sws_num = sws_num;
	hdr->stages = nsw->stages;
	fwrite (hdr, hdr_size, 1, out);//像out总写入hdr_size大小的hdr，1为数据个数，将偏移量写入文件
	// //将ntfs和stages写入文件开头，arrs_ofs, strs_ofs值为0，sw_ofs也为0，空一个sw_ofs区域

	for (int i = 0; i < sws_num; i++) {//写tf到文件，0-ntf->ntfs
	  hdr->sw_ofs[i] = ftell (out);//找地址
	  printf ("%" PRIu32 "\n", hdr->sw_ofs[i]); //# define PRIu32 "u"十进制无符号整数
	  // if (!i) gen_sw (ttf, out, f_strs, arrs, narrs);//i=0 topology.tf，link的数据
	  // else gen_sw (ntf->tfs[i - 1], out, f_strs, arrs, narrs);//后面其余
	  // gen_sw (nsw->sws[i - 1], out, f_strs, arrs, narrs);
	  gen_sw (nsw->sws[i], out, f_strs, arrs, narrs, i);
	}
	fclose (f_strs);
	mf_len = 2;//*2字节，2字节为一位uint16_t
	// int len = ARRAY_BYTES (arr_len);
	hdr->arrs_ofs = ftell (out);
	fwrite (&mf_len, sizeof mf_len, 1, out);
	fwrite (&narrs, sizeof narrs, 1, out);
	fwrite (arrs, 2*MF_LEN, narrs, out);
	free (arrs);

	hdr->strs_ofs = ftell (out);
	fwrite (buf_strs, 1, sz_strs, out);
	free (buf_strs);

	int end = ftell (out);
	rewind (out);//文件指针从新到开头，写hdr
	fwrite (hdr, hdr_size, 1, out);
	free (hdr);
	// rewind (end);
	printf ("Total: %d bytes\n", end);
	fclose (out);
}

//parse.c
void
array_free (array_t *a) {
	free (a); 
}

static void
add_rule (struct parse_sw *sw, struct parse_rule *r) {
  r->idx = ++sw->nrules;//sw中规则数+1，然后索引序号这个规则数
  list_append (&sw->rules, r); //链表指向这个规则
 

  // for (int i = 0; i < r->in.n; i++) {//对于r在n中的排序
  //   struct map_elem *e = map_find_create (&sw->in_map, ARR (r->in)[i]);
  //   struct map_val *tmp = xmalloc (sizeof *tmp);
  //   tmp->val = r;
  //   list_append (&e->vals, tmp);
  // }
}

// static void
// free_dep (struct parse_dep *dep) {
// 	array_free (dep->match); free (dep);
// }

static void
free_rule (struct parse_rule *r) {
	ARR_FREE (r->in);
	ARR_FREE (r->out);
	// array_t *arrs[] = {r->match, r->mask, r->rewrite};
	// for (int i = 0; i < ARR_LEN (arrs); i++) array_free (arrs[i]);
	// list_destroy (&r->deps, free_dep);
	free (r);
}

static void
free_sw (struct parse_sw *sw) {
	free (sw->prefix);
	list_destroy (&sw->rules, free_rule);
	// map_destroy (&tf->in_map);
	free (sw);
}

static void
free_nsw (struct parse_nsw *nsw) {
	for (int i = 0; i < nsw->sws_num; i++) free_sw (nsw->sws[i]);
	free (nsw);
}

static int
filter_tfs (const struct dirent *ent) {
	char *ext = strrchr (ent->d_name, '.');//将会找出ent->d_name字符串中最后一次出现的字符'.'的地址，然后将该地址返回。
	if (!ext || strcmp (ext, ".tf")) return false;
	return strcmp (ent->d_name, "topology.tf");
}

static struct arr_ptr_uint32_t
read_array (char *s, uint32_t *res) { //保存成struct arr_ptr_uint32_t
	uint32_t buf[MAX_ARR_SIZE];
	if (!res) res = buf;
	int end, n = 0;
	if (*s == '[') { s++; s[strlen (s) - 1] = 0; }
	while (sscanf (s, " %" SCNu32 "%n", &res[n], &end) == 1) {//" %" SCNu32 为%u 十进制无符号整数,从s中读取
		n++; s += end;
		if (*s == ',') s++;
	}

	struct arr_ptr_uint32_t tmp = {0};
	if (!n) return tmp;//如果没有
	qsort (res, n, sizeof *res, int_cmp);//排序有n个
	tmp.n = n;
	if (res == buf) {
		ARR_ALLOC (tmp, n);
		// printf("%u\n", buf[0]);
		memcpy (ARR (tmp), buf, n * sizeof *buf);
	}
	return tmp;
}

static struct parse_sw *
parse_tf (const char *name)
{
	FILE *in = fopen (name, "r");
	char *line = NULL;
	int len;
	size_t n;

	if (!in || (len = getline (&line, &n, in)) == -1)
	err (1, "Can't read file \"%s\"", name);

	int tflen;
	char prefix[MAX_PREFIX + 1];//前缀数组

	int res = sscanf (line, "%d$%" QUOTE (MAX_PREFIX) "[^$]$", &tflen, prefix);//读取第一行的两个参数到&tflen, prefix
	//tflen为中间matchfield长度，tflen = len/4；4个三元组1位，
	//从一个字符串中读进与指定格式相符的数据，res为读取的参数个数，
	//读取的字符串line，根据参数"%d$%"转换并格式化数据，QUOTE (MAX_PREFIX)为#MAX_PREFIX也就是#255
	//%[^$]匹配非$的任意字符，贪婪直到$
	printf ("%d, %s \n", tflen, prefix);
	if (res < 1) errx (1, "Can't read len from first line \"%s\".", line);
	tflen /= 2; /* Convert to L 8个三元组1位*/

	struct parse_sw *sw = xcalloc (1, sizeof *sw);//分配tf空间
	sw->len = tflen;
	if (res == 2) sw->prefix = xstrdup (prefix);//如果有prefix
	//prefix为字符串，如果字符串为空，报错，要么直接复制给一个指针再赋值到tf->prefix
	// uint32_t sign = 0;
	/* Skip next line 跳过第二行的#*/
	getline (&line, &n, in);
	while ((len = getline (&line, &n, in)) != -1) {//按行读取文件
		char *save; //每次循环定义一次
		char *type, *instr, *match, *mask, *rewrite, *outstr, *affected;
		//char *file, *lines, *id;

		type = strtok_r (line, "$", &save); //type：动作的类型
		/*linux下分割字符串的安全函数，line待分割字符串，
		分割字符"$"，剩余的字符串保存在save变量中，char **saveptr
		发现参数中包涵的分割字符时，则会将该字符改为\0 字符。
		在第一次调用时，必需给予参数s字符串，
		往后的调用则将参数s设置成NULL。每次调用成功则返回指向被分割出片段的指针。*/
		instr = strtok_r (NULL, "$", &save);//输入序列（端口）
		
		match = strtok_r (NULL, "$", &save);//匹配域

		// printf("%s\n", match);

		mask = strtok_r (NULL, "$", &save);//掩码
		rewrite = strtok_r (NULL, "$", &save);//重写的匹配域
		/*inv_match =*/ strtok_r (NULL, "$", &save);//匹配域逆
		/*inv_rewrite =*/ strtok_r (NULL, "$", &save);//重写的匹配域逆
		outstr = strtok_r (NULL, "$", &save);//输出序列（端口）
		affected = strtok_r (NULL, "$", &save);//受影响的规则号用#分割
		/*influence = */strtok_r (NULL, "$", &save);//影响的规则号用#分割
		/* TODO: desc
		file = strtok_r (NULL, "$", &save);
		lines = strtok_r (NULL, "$", &save);
		id = strtok_r (NULL, "$", &save);
		if (!id) { id = file; file = lines = NULL; }*/
		// printf("%s\n", instr);
		struct parse_rule *r = xcalloc (1, sizeof *r);//创建规则分配内存
		r->in = read_array (instr, NULL);//保存成struct arr_ptr_uint32_t

		//ARR (r->in)[0]为一个数组,n为元素个数
		r->out = read_array (outstr, NULL);
		/*if (file) {
		  r->file = xstrdup (file);
		  lines[strlen (lines) - 1] = 0;
		  r->lines = xstrdup (lines);
		}*/

		//link貌似也作为函数，只有出入端口，把端口连起来
		if (strcmp (type, "link")) {//比较字符串，相同返回0，如果不是"link"
			r->match = mf_from_str (match);	
			if (!strcmp (type, "rw")) {//如果是"rw"
				r->mask = mf_from_str (mask);
				r->rewrite = mf_from_str (rewrite);
				for (int i = 0; i < MF_LEN; i++) { //测试是否mask和rewrite没有x
					assert(!(r->mask->mf_w[i] && r->rewrite->mf_w[i]));
				}		
			}
		}

	// 	r->deps = read_deps (affected);
		add_rule (sw, r);//这个是把这个规则保存起来到tf中
	}

	free (line);
	fclose (in);
	return sw; //在一个交换机中保存的规则和关系
}

uint32_t 
link_read_array(char *s)
{
	uint32_t tmp;
	// if (!res) res = buf;
	int end;
	if (*s == '[') { s++; s[strlen (s) - 1] = 0; }
	sscanf (s, " %" SCNu32 "%n", &tmp, &end);
	// {//" %" SCNu32 为%u 十进制无符号整数,从s中读取
	// 	n++; s += end;
	// 	if (*s == ',') s++;
	// }

	return tmp;
}

static int
port_in_cmp (const void *a, const void *b)
{	return *(uint32_t *)a - *(uint32_t *)b;}

static int
port_out_cmp (const void *a, const void *b)
{	return *(uint32_t *)((uint32_t *)a+1) - *(uint32_t *)((uint32_t *)b+1);}

static int
link_cmp (const void *a, const void *b)
{	return *(uint16_t *)a - *(uint16_t *)b;}

static int
link_port_in_cmp (const void *a, const void *b){
	return *(uint32_t *)((uint16_t *)a+1) - *(uint32_t *)((uint16_t *)b+1);}

static int
link_port_out_cmp (const void *a, const void *b){
	return *(uint32_t *)((uint16_t *)a+3) - *(uint32_t *)((uint16_t *)b+3);}

char *
links_idx_gen (const char *name, const struct parse_nsw *nsw, uint32_t *nlinks, uint32_t *swls)
{	//idx 从0开始，数量为*swls，idx max = *swls - 1
	FILE *in = fopen (name, "r");
	char *line = NULL;
	int len;
	size_t n;
	uint32_t swl_num, inl_num, outl_num;
	uint32_t count = 0;
	uint32_t len_idx = 2;
	uint32_t len_port = 4;
	uint16_t idx = 0;
	FILE *f_tmp;
	char *outdir = "../data/link_idx.dat";

	char *buf, *buf2, *bufout;
	size_t bufsz, buf2sz, bufoutsz;

	if (!in || (len = getline (&line, &n, in)) == -1)
	err (1, "Can't read file \"%s\"", name);

	FILE *f = open_memstream (&buf, &bufsz);
	
	FILE *out = open_memstream (&bufout, &bufoutsz);

	/* Skip next line 跳过一行*/
	//创建规则分配内存
	struct link *l = xcalloc (1, sizeof *l);
	l->idx = 0;
	getline (&line, &n, in);
	while ((len = getline (&line, &n, in)) != -1) {//按行读取文件
		char *save; //每次循环定义一次
		char *instr, *outstr;
		//char *file, *lines, *id;
		/*type = */strtok_r (line, "$", &save); //type：动作的类型
		instr = strtok_r (NULL, "$", &save);//输入序列（端口）
		/*match = */strtok_r (NULL, "$", &save);//匹配域
		/*mask = */strtok_r (NULL, "$", &save);//掩码
		/*rewrite = */strtok_r (NULL, "$", &save);//重写的匹配域
		/*inv_match =*/ strtok_r (NULL, "$", &save);//匹配域逆
		/*inv_rewrite =*/ strtok_r (NULL, "$", &save);//掩码逆
		outstr = strtok_r (NULL, "$", &save);//输出序列（端口）
		/*affected = */strtok_r (NULL, "$", &save);//受影响的规则号用#分割
		/*influence = */strtok_r (NULL, "$", &save);//影响的规则号用#分割

		
		l->port_in = link_read_array (instr);//保存成struct arr_ptr_uint32_t
		fwrite (&(l->port_in), len_port, 1, f);
	
		l->port_out = link_read_array (outstr);
		fwrite (&(l->port_out), len_port, 1, f);
		
		count++;
		// printf("link %d: %d - > %d\n", l->idx, l->port_in, l->port_out);
	}
	fclose (f);
	fflush (f);
	assert (2*count * len_port == bufsz);
	free (line);
	fclose (in);

	qsort (buf, count, 2*len_port, port_in_cmp);
	for (int i = 0; i < 2*count * len_port; i += 2*len_port) {
		fwrite (&idx, len_idx, 1, out);
		fwrite (&buf[i], 2*len_port, 1, out);
		idx++;
	}
	swl_num = count;
	*swls = swl_num;

	uint32_t count2 = 0;
	uint32_t count_tmp = 0;
	f = open_memstream (&buf2, &buf2sz);
	l->port_out = 0;
	for (int i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		char *buf3;
		size_t buf3sz;
		f_tmp = open_memstream (&buf3, &buf3sz);
		uint32_t count3 = 0;
		int r_i = 0;
		for (struct parse_rule *r = sw->rules.head; r; r = r->next, r_i++) {
			uint32_t n = r->out.n;
			uint32_t *parr = ARR (r->out);
			for (int j = 0; j < n; j++, parr++)
			{	
				l->port_in = *parr;
				if (bsearch(&(l->port_in), buf, count, 2*len_port, port_in_cmp)){
					continue;
				}
				// uint32_t *b = bsearch(&(l->port_in), buf3, count3, 2*len_port, port_in_cmp);
				if (bsearch(&(l->port_in), buf3, count3, 2*len_port, port_in_cmp)){
					continue;
				}
				fwrite (&(l->port_in), 2*len_port, 1, f_tmp);
				count3++;
				fflush (f_tmp);
				qsort (buf3, count3, 2*len_port, port_in_cmp);		
			}
		}
		assert (2*count3 * len_port == buf3sz);
		fwrite (buf3, buf3sz, 1, f);
		count_tmp += count3;
		fclose (f_tmp);
		free (buf3);
	}

	fclose (f);
	fflush (f);
	assert (2*count_tmp * len_port == buf2sz);
	qsort (buf2, count_tmp, 2*len_port, port_in_cmp);
	for (int i = 0; i < 2*count_tmp * len_port; i += 2*len_port) {
		fwrite (&idx, len_idx, 1, out);
		fwrite (&buf2[i], 2*len_port, 1, out);
		idx++;
	}
	count2 += count_tmp;
	inl_num = count_tmp;
	free (buf2);

	count_tmp = 0;
	qsort (buf, count, 2*len_port, port_out_cmp);
	f = open_memstream (&buf2, &buf2sz);
	l->port_in = 0;
	for (int i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		char *buf3;
		size_t buf3sz;
		f_tmp = open_memstream (&buf3, &buf3sz);
		uint32_t count3 = 0;
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {
			uint32_t n = r->in.n;
			uint32_t *parr = ARR (r->in);
			for (int j = 0; j < n; j++, parr++)
			{	
				l->port_out = *parr;				
				if (bsearch(&(l->port_in), buf, count, 2*len_port, port_out_cmp)){
					continue;
				}
				if (bsearch(&(l->port_in), buf3, count3, 2*len_port, port_out_cmp)){
					continue;
				}
				fwrite (&(l->port_in), 2*len_port, 1, f_tmp);
				count3++;
				fflush (f_tmp);
				qsort (buf3, count3, 2*len_port, port_out_cmp);		
			}
		}
		assert (2*count3 * len_port == buf3sz);
		fwrite (buf3, buf3sz, 1, f);
		count_tmp += count3;
		fclose (f_tmp);
		free (buf3);
	}
	fclose (f);
	fflush (f);
	assert (2*count_tmp * len_port == buf2sz);
	qsort (buf2, count_tmp, 2*len_port, port_out_cmp);
	for (int i = 0; i < 2*count_tmp * len_port; i += 2*len_port) {
		fwrite (&idx, len_idx, 1, out);
		fwrite (&buf2[i], 2*len_port, 1, out);
		idx++;
	}
	count2 += count_tmp;
	outl_num = count_tmp;
	count += count2;
	free (buf2);
	free (buf);

	fclose (out);
	fflush (out);
	assert (count * (2*len_port + len_idx) == bufoutsz);
	
	
	// printf("idx:%d;", *(uint16_t*)(&buf[0]));
	// printf("%d -- %d\n", *(uint32_t*)(&buf[0]+2), *(uint32_t*)(&buf[0]+6));
	qsort (bufout, idx, len_idx+2*len_port, link_cmp);
	// for (int i = 0; i < 10; i++)
	// {
	// 	printf("%d\n", *(uint16_t *)(bufout+(len_idx+2*len_port)*i));
	// }
	*nlinks = (uint32_t)idx;
	FILE *outfile = fopen (outdir, "w");//建立文件out，并可写
	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&inl_num, 4, 1, outfile);
	fwrite (&outl_num, 4, 1, outfile);
	fwrite (bufout, len_idx+2*len_port, idx, outfile);
	fclose (outfile);
	free (l);


	return bufout; //返回的idx 和port对
}

static int
link_to_rule_cmp (const void *a, const void *b)
{
	uint32_t c = *(uint32_t *)a - *(uint32_t *)b;
	if (c) 
		return c;
	return *((uint32_t *)a+2) - *((uint32_t *)b+2);
	// return memcmp ((void *)((uint16_t *)a+3), (void *)((uint16_t *)b+3), 4);
}

/*
void
rule_ports_to_links_backup(uint16_t *links, const uint32_t *nlinks, const uint32_t *swls, 
					const struct parse_nsw *nsw)
{
	char *outdirout = "../data/link_out_rule.dat";
	char *outdirin = "../data/link_in_rule.dat";
	// int link_len = 5;
	struct link *lk = xcalloc (1, sizeof *lk);
	// struct link_to_rule *lr = xcalloc (1, sizeof *lr);
	uint32_t len_idx = 2;
	uint32_t len_port = 4;
	uint16_t *b;
	char *buf1, *buf2;
	size_t buf1sz, buf2sz;
	FILE *f = open_memstream (&buf1, &buf1sz);

	uint32_t count = 0;
	uint32_t r_idx = 0;
	lk->idx = 0;	
	qsort (links, *nlinks, len_idx+2*len_port, link_port_out_cmp);
	lk->port_in = 0;
	for (uint32_t i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {
			r_idx = (uint32_t)r->idx;
			uint32_t n = r->in.n;
			uint32_t *parr = ARR (r->in);
			for (uint32_t j = 0; j < n; j++, parr++) {	
				lk->port_out = *parr;
				b = bsearch (lk, links, *nlinks, len_idx+2*len_port, link_port_out_cmp);			
				assert (b);	
				*parr = (uint32_t)*b;
				fwrite (parr, 4, 1, f);
				fwrite (&i, 4, 1, f);
				fwrite (&(r_idx), 4, 1, f);
				count++;
			}
		}
	}
	fclose (f);
	fflush (f);
	assert (3*count*sizeof(uint32_t) == buf1sz);
	qsort (buf1, count, 3*sizeof(uint32_t), link_to_rule_cmp);

	FILE *outfile = fopen (outdirout, "w");//建立文件out，并可写

	uint32_t swl_num = 0;
	uint32_t outnl_num = 0;
	uint32_t idx_num = 0;
	uint32_t idx_tmp = 0;
	uint32_t total_num = 0;

	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);

	// printf("buf1%d\n", *(uint32_t *)buf1);
	for (uint32_t i = 0; i < 12*count; i+=12) {	
		if (*(uint32_t *)(buf1+i) != idx_tmp) {
			
			if (idx_tmp <= *swls) {
				swl_num++;
			}
			// printf("% d;", idx_tmp);
			fwrite (&idx_tmp, 4, 1, outfile);
			fwrite (&total_num, 4, 1, outfile);
			idx_tmp = *(uint32_t *)(buf1+i);
			idx_num++;
			
		}
		total_num++;
	}
	printf("outdirout%d\n", swl_num);
	// printf("swl_num%d\n", swl_num);
	fwrite (&idx_tmp, 4, 1, outfile);
	// printf("idx_tmp%d\n", idx_tmp);
	fwrite (&total_num, 4, 1, outfile);
	idx_num++;
	outnl_num = idx_num - swl_num;

	uint32_t *buf = (uint32_t *)buf1;
	for (uint32_t i = 0; i < 3*count; i+=3) {
			fwrite ((uint32_t *)(buf+i+1), 8, 1, outfile);
			// printf("%d - ", *(uint32_t *)(buf+i));
			// printf("%d - ", *(uint32_t *)(buf+i+1));
			// printf("%d;", *(uint32_t *)(buf+i+2));

	}

	rewind (outfile);//文件指针从新到开头，写hdr
	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);

	fclose (outfile);
	free (buf1);

	f = open_memstream (&buf2, &buf2sz);
	count = 0;
	r_idx = 0;
	lk->idx = 0;	
	qsort (links, *nlinks, len_idx+2*len_port, link_port_in_cmp);
	lk->port_out = 0;
	for (uint32_t i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {
			r_idx = (uint32_t)r->idx;
			uint32_t n = r->out.n;
			uint32_t *parr = ARR (r->out);
			for (uint32_t j = 0; j < n; j++, parr++) {	
				lk->port_in = *parr;
				b = bsearch (lk, links, *nlinks, len_idx+2*len_port, link_port_in_cmp);			
				assert (b);	
				*parr = (uint32_t)*b;
				fwrite (parr, 4, 1, f);
				fwrite (&i, 4, 1, f);
				fwrite (&(r_idx), 4, 1, f);
				count++;
			}
		}
	}
	fclose (f);
	fflush (f);
	assert (3*count*sizeof(uint32_t) == buf2sz);
	qsort (buf2, count, 3*sizeof(uint32_t), link_to_rule_cmp);

	outfile = fopen (outdirin, "w");//建立文件out，并可写

	swl_num = 0;
	outnl_num = 0;
	idx_num = 0;
	idx_tmp = 0;
	total_num = 0;

	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);
	
	for (uint32_t i = 0; i < 12*count; i+=12) {	
		if (*(uint32_t *)(buf2+i) != idx_tmp) {
			
			if (idx_tmp <= *swls) {
				swl_num++;
			}
			// printf("% d;", idx_tmp);
			fwrite (&idx_tmp, 4, 1, outfile);
			fwrite (&total_num, 4, 1, outfile);
			idx_tmp = *(uint32_t *)(buf2+i);
			// printf("%d", idx_tmp);
			idx_num++;
		}
		total_num++;		
	}
	printf("indirout%d\n", swl_num);
	// printf("%d\n", idx_num);
	// printf("swl_num%d\n", swl_num);
	fwrite (&idx_tmp, 4, 1, outfile);
	fwrite (&total_num, 4, 1, outfile);
	idx_num++;
	outnl_num = idx_num - idx_tmp;

	buf = (uint32_t *)buf2;
	for (uint32_t i = 0; i < 3*count; i+=3) {
			fwrite ((uint32_t *)(buf+i+1), 8, 1, outfile);
			// printf("%d - ", *(uint32_t *)(buf+i));
			// printf("%d - ", *(uint32_t *)(buf+i+1));
			// printf("%d;", *(uint32_t *)(buf+i+2));
	}

	rewind (outfile);//文件指针从新到开头，写hdr
	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);

	fclose (outfile);
	free (buf2);
	free (lk);
	// free (lr);
}
*/

void
arule_PtoL(struct parse_rule *r, uint16_t *links, const uint32_t *nlinks, const uint32_t io_sign) {
	/*	io_sign: RULE_LINK_IN 0, RULE_LINK_OUT 1
		RULE_LINK_IN: r->in 中的port转换到r->in_link中的link idx
		links: 保存了 link_idx, link_inport, link_outport, 所有link编号的查询
	*/
	struct link *lk = xcalloc (1, sizeof *lk);
	lk->idx = 0;
	uint32_t len_lk_u8 = 2*sizeof(uint32_t) +sizeof(uint16_t);
	uint32_t len_lk_u16 = len_lk_u8/2;
	uint32_t buf[MAX_ARR_SIZE];
	// uint32_t n_l = 0;
	uint32_t count = 0;
	uint16_t *begin;
	uint16_t *b;

	if (io_sign) {//RULE_LINK_IN 0, RULE_LINK_OUT 1,
		lk->port_out = 0;
		uint32_t n = r->out.n;
		uint32_t *parr = ARR (r->out);
		for (uint32_t i = 0; i < n; i++, parr++) {
			lk->port_in = *parr;
			b = (uint16_t *)bsearch (lk, links, *nlinks, len_lk_u8, link_port_in_cmp);	
			//void指针赋值给其他类型的指针时都要进行转换,bsearch返回的是空指针void指针，那么后面就都是uint16_t		
			assert (b);	
			if (*b == *links) { 
				begin = (uint16_t *)links;
			}
			else {
				for (uint32_t j = 1; j < *nlinks; j++) {
					if (link_port_in_cmp(b, (uint16_t *)(b - j*len_lk_u16))) {
						begin = (uint16_t *)(b - (j-1)*len_lk_u16);
						break;
					}
				}
			}
			buf[count] = (uint32_t)*begin;
			count++;
			if (*b != *(uint16_t *)(links+ ((*nlinks)-1)*len_lk_u16)) {
				for (uint32_t j = 1; j < *nlinks; j++) {
					if (!link_port_in_cmp(begin, (uint16_t *)(begin + j*len_lk_u16))) {
						buf[count] = (uint32_t)*(uint16_t *)(begin + j*len_lk_u16);
						count++;
					}
					else
						break;
				}
			}
		}
	}
	else{
		lk->port_in = 0;
		uint32_t n = r->in.n;
		uint32_t *parr = ARR (r->in);
		for (uint32_t i = 0; i < n; i++, parr++) {
			lk->port_out = *parr;
			b = (uint16_t *)bsearch (lk, links, *nlinks, len_lk_u8, link_port_out_cmp);	
			assert (b);	
			if (*b == *links) {
				begin = (uint16_t *)links;
			}
			else {
				for (uint32_t j = 1; j < *nlinks; j++) {
					if (link_port_out_cmp(b, (uint16_t *)(b - j*len_lk_u16))) {
						begin = (uint16_t *)(b - (j-1)*len_lk_u16);
						break;
					}
				}
			}
			buf[count] = (uint32_t)*begin;
			count++;
			if (*b != *(uint16_t *)(links+ ((*nlinks)-1)*len_lk_u16)) {
				for (uint32_t j = 1; j < *nlinks; j++) {
					if (!link_port_out_cmp(begin, (uint16_t *)(begin + j*len_lk_u16))) {
						buf[count] = (uint32_t)*(uint16_t *)(begin + j*len_lk_u16);
						count++;
					}
					else
						break;				
				}
			}
		}
	}
	struct arr_ptr_uint32_t tmp = {0};
	tmp.n = count;
	//这里port不相同link一定不相同，那么就先不用检测重复的问题
	if (count){
		ARR_ALLOC (tmp, count);
		memcpy (ARR (tmp), buf, count * sizeof *buf);
		if (io_sign) 
			r->out_link = tmp;
		else
			r->in_link = tmp;
	}
	else {
		if (io_sign) 
			r->out_link = tmp;
		else
			r->in_link = tmp;
	}
	
	// if (r->idx == 1)
	// {
	// 	uint32_t n = r->in_link.n;
	// 	uint32_t *parr = ARR (r->in_link);
	// 	for (uint32_t j = 0; j < n; j++, parr++)
	// 		printf("%d - %d; ", buf[j], *parr);
	// 	printf("%d", len_lk_u8);
	// 	printf("\n");
	// }
	// uint32_t *parr = ARR (tmp);
	// if ((r->idx == 201) && io_sign){
	// 	printf("%d:", count);
		// for (int i = 0; i < count; i++)
		// {
		// 	printf("%d;", *parr);
		// 	parr++;
		// }
	// }
	free (lk);
}

void
rules_ports_to_links(uint16_t *links, const uint32_t *nlinks, const uint32_t *swls, 
					const struct parse_nsw *nsw)
{
	char *outdirout = "../data/link_out_rule.dat";
	char *outdirin = "../data/link_in_rule.dat";
	// int link_len = 5;
	struct link *lk = xcalloc (1, sizeof *lk);
	// struct link_to_rule *lr = xcalloc (1, sizeof *lr);
	uint32_t len_idx = 2;
	uint32_t len_port = 4;
	// uint16_t *b;
	char *buf1, *buf2;
	size_t buf1sz, buf2sz;
	FILE *f = open_memstream (&buf1, &buf1sz);

	uint32_t count = 0;
	uint32_t r_idx = 0;
	lk->idx = 0;	
	qsort (links, *nlinks, len_idx+2*len_port, link_port_out_cmp);
	lk->port_in = 0;
	for (uint32_t i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {
			arule_PtoL(r, links, nlinks, RULE_LINK_IN);
			// if (r->idx == 1)
			// {
			// 	uint32_t n = r->in_link.n;
			// 	uint32_t *parr = ARR (r->in_link);
			// 	for (uint32_t j = 0; j < n; j++, parr++)
			// 		printf("%d #", *parr);
			// 	printf("\n");
			// }
			r_idx = (uint32_t)r->idx;
			uint32_t n = r->in_link.n;
			uint32_t *parr = ARR (r->in_link);
			for (uint32_t j = 0; j < n; j++, parr++){				
				fwrite (parr, 4, 1, f);
				fwrite (&i, 4, 1, f);
				fwrite (&(r_idx), 4, 1, f);
				count++;
			}
		} 
	}
	fclose (f);
	fflush (f);
	assert (3*count*sizeof(uint32_t) == buf1sz);
	qsort (buf1, count, 3*sizeof(uint32_t), link_to_rule_cmp);
	FILE *outfile = fopen (outdirout, "w");//建立文件out，并可写

	uint32_t swl_num = 0;
	uint32_t outnl_num = 0;
	uint32_t idx_num = 0;
	uint32_t idx_tmp = 0;
	uint32_t total_num = 0;
	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);

	// printf("buf1%d\n", *(uint32_t *)buf1);
	for (uint32_t i = 0; i < 12*count; i+=12) {	
		if (*(uint32_t *)(buf1+i) != idx_tmp) {	
			if (idx_tmp < *swls) {
				swl_num++;
			}
			// printf("% d;", idx_tmp);
			fwrite (&idx_tmp, 4, 1, outfile);
			fwrite (&total_num, 4, 1, outfile);
			idx_tmp = *(uint32_t *)(buf1+i);
			idx_num++;		
		}
		total_num++;
	}
	printf("outdirout%d  ; %d\n", swl_num, *swls);

	// printf("swl_num%d\n", swl_num);
	fwrite (&idx_tmp, 4, 1, outfile);
	// printf("idx_tmp%d\n", idx_tmp);
	fwrite (&total_num, 4, 1, outfile);
	idx_num++;
	outnl_num = idx_num - swl_num;

	uint32_t *buf = (uint32_t *)buf1;
	for (uint32_t i = 0; i < 3*count; i+=3) {
			fwrite ((uint32_t *)(buf+i+1), 8, 1, outfile);
			// printf("%d - ", *(uint32_t *)(buf+i));
			// printf("%d - ", *(uint32_t *)(buf+i+1));
			// printf("%d;", *(uint32_t *)(buf+i+2));
	}

	rewind (outfile);//文件指针从新到开头，写hdr
	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);
	fclose (outfile);
	free (buf1);

	f = open_memstream (&buf2, &buf2sz);
	count = 0;
	r_idx = 0;
	lk->idx = 0;	
	qsort (links, *nlinks, len_idx+2*len_port, link_port_in_cmp);
	lk->port_out = 0;
	for (uint32_t i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {
			arule_PtoL(r, links, nlinks, RULE_LINK_OUT);
			r_idx = (uint32_t)r->idx;
			uint32_t n = r->out_link.n;
			uint32_t *parr = ARR (r->out_link);
			for (uint32_t j = 0; j < n; j++, parr++){				
				fwrite (parr, 4, 1, f);
				fwrite (&i, 4, 1, f);
				fwrite (&(r_idx), 4, 1, f);
				count++;
			}
		} 
	}
	fclose (f);
	fflush (f);
	assert (3*count*sizeof(uint32_t) == buf2sz);
	qsort (buf2, count, 3*sizeof(uint32_t), link_to_rule_cmp);

	outfile = fopen (outdirin, "w");//建立文件out，并可写

	swl_num = 0;
	outnl_num = 0;
	idx_num = 0;
	idx_tmp = 0;
	total_num = 0;

	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);
	
	for (uint32_t i = 0; i < 12*count; i+=12) {	
		if (*(uint32_t *)(buf2+i) != idx_tmp) {		
			if (idx_tmp < *swls) {
				swl_num++;
			}
			// printf("% d;", idx_tmp);
			fwrite (&idx_tmp, 4, 1, outfile);
			fwrite (&total_num, 4, 1, outfile);
			idx_tmp = *(uint32_t *)(buf2+i);
			// printf("%d", idx_tmp);
			idx_num++;		
		}
		total_num++;	
	}
	printf("indirout%d  ; %d\n", swl_num, *swls);
	// printf("%d\n", idx_num);
	// printf("swl_num%d\n", swl_num);
	fwrite (&idx_tmp, 4, 1, outfile);
	fwrite (&total_num, 4, 1, outfile);
	idx_num++;
	outnl_num = idx_num - idx_tmp;

	buf = (uint32_t *)buf2;
	for (uint32_t i = 0; i < 3*count; i+=3) {
			fwrite ((uint32_t *)(buf+i+1), 8, 1, outfile);
			// printf("%d - ", *(uint32_t *)(buf+i));
			// printf("%d - ", *(uint32_t *)(buf+i+1));
			// printf("%d;", *(uint32_t *)(buf+i+2));
	}

	rewind (outfile);//文件指针从新到开头，写hdr
	fwrite (&swl_num, 4, 1, outfile);
	fwrite (&outnl_num, 4, 1, outfile);
	fwrite (&idx_num, 4, 1, outfile);
	fclose (outfile);
	free (buf2);
	free (lk);
	// free (lr);
}



void
check_mf(const struct parse_nsw *nsw){
	int count = 0;//计数match
	for (int i = 0; i < nsw->sws_num; i++) {//0到n，所有tf		
		const struct parse_sw *sw = nsw->sws[i];
		for (struct parse_rule *r = sw->rules.head; r; r = r->next){
			assert (r->match);
		}
	}
}

void
parse_dir (const char *outdir, const char *tfdir, const char *name)
{
	printf ("Parsing: \n");
	fflush (stdout);
	struct parse_nsw *nsw;
	// struct parse_tf *ttf;
	int stages;

	char buf[PATH_MAX + 1];
	snprintf (buf, sizeof buf, "../%s/%s", tfdir, name);
	char *base = buf + strlen (buf); //base指向buf后面的部分
	strcpy (base, "/stages");//buf后面接上"/stages"
	// printf("%s\n", buf);

	FILE *f = fopen (buf, "r");//打开"/stages"
	if (!f) err (1, "Can't open %s", buf);//stanford为3
	if (!fscanf (f, "%d", &stages)) errx (1, "Can't read NTF stages from %s", buf);
	fclose (f);

	*base = 0;
	strcpy (base , "/");
	// printf("base%s\n", base);
	// printf("buf%s\n", buf);
	struct dirent **tfs;//#include<dirent.h>，为了获取某文件夹目录内容
	//成功则返回复制到tfs数组中的数据结构数目，每读取一个传给filter_tfs，过滤掉不想要的，这里要.tf
	int n = scandir (buf, &tfs, filter_tfs, alphasort);//为了获取某文件夹目录内容，按字母排序
	if (n <= 0) err (1, "Couldn't find .tf files in %s", buf);
	// n = 1;//控制只取一个来实验

	nsw = xmalloc (sizeof *nsw + n * sizeof *nsw->sws);
	nsw->sws_num = n;
	nsw->stages = stages;
	for (int i = 0; i < n; i++) {//对找到的 .tf 文件处理 0到n-1
	    strcpy (base + 1, tfs[i]->d_name); //文件名，base+1写文件名，记录文件名,也就是要读取的名字
	    free (tfs[i]);
	    struct parse_sw *sw = parse_tf (buf);//解析 .tf
	    assert (sw);
	    nsw->sws[i] = sw;//保存在nsw中 
	}
	check_mf(nsw);
	free (tfs);
	printf("%d\n", nsw->sws[0]->nrules);

	uint32_t nlinks = 0;
	uint32_t swl_num = 0;
	// uint32_t nstrus = 0;

	strcpy (base, "/topology.tf");//base为/topology.tf
	char *link_arr = links_idx_gen (buf, nsw, &nlinks, &swl_num);
	rules_ports_to_links ((uint16_t *)link_arr, &nlinks, &swl_num, nsw);

	snprintf (buf, sizeof buf, "../%s/%s.dat", outdir, name);//输出文件路径名称在buf中
	rule_data_gen (buf, nsw);//将ntf, ttf合并生成数据
	// rule_link_gen (nsw, nstrus);

	free_nsw (nsw);
	// free_tf (ttf);
}

void
connect_link_data_gen (const struct parse_nsw *nsw)
{
	uint32_t count = 0;
	uint32_t len_port = 4;
	char *outdir = "../data/connect_link.txt";
	char *buf;
	size_t bufsz;

	FILE *f = open_memstream (&buf, &bufsz);

	for (int i = 0; i < nsw->sws_num; i++) {
		struct parse_sw *sw = nsw->sws[i];
		assert (sw);
		for (struct parse_rule *r = sw->rules.head; r; r = r->next) {
			uint32_t n = r->in.n;
			uint32_t *parr = ARR (r->in);
			for (int j = 0; j < n; j++, parr++) {	
				uint32_t port_tmp = *parr;
				if(i==5)
					printf("%d;", port_tmp/10000);
				if ((port_tmp/10000 - 10*(port_tmp/100000)) == 1){

					if (count){
						if (bsearch(&port_tmp, buf, count, len_port, uint32_t_cmp))
							continue;
					}
					fwrite (&port_tmp, len_port, 1, f);
					count++;
				}
				fflush (f);
				qsort (buf, count, len_port, uint32_t_cmp);		
			}
			n = r->out.n;
			parr = ARR (r->out);
			for (int j = 0; j < n; j++, parr++) {
				uint32_t port_tmp = *parr;
				if ((port_tmp/10000 - 10*(port_tmp/100000)) == 1){
					if (count){
						if (bsearch(&port_tmp, buf, count, len_port, uint32_t_cmp))
							continue;
					}
					fwrite (&port_tmp, len_port, 1, f);
					count++;
				}
				fflush (f);
				qsort (buf, count, len_port, uint32_t_cmp);		
			}
		}
	}
	assert (count * len_port == bufsz);
	fclose (f);
	

	FILE *outfile = fopen (outdir, "a");//建立文件out，并可写
	uint32_t *buf_32 = (uint32_t *)buf;
	printf("count%d\n", count);

	for (uint32_t i = 0; i < count; i++) {
		fprintf (outfile, "link$[%d]$None$None$None$None$None$[%d]$#$#$$$_%d$\n", *(buf_32 + i), *(buf_32 + i), 91+i);
	}


	fclose (outfile);
	free(buf);
}

void
generate_connect_link (const char *outdir, const char *tfdir, const char *name)
{
	printf ("Parsing: \n");
	fflush (stdout);
	struct parse_nsw *nsw;
	// struct parse_tf *ttf;
	int stages;

	char buf[PATH_MAX + 1];
	snprintf (buf, sizeof buf, "../%s/%s", tfdir, name);
	char *base = buf + strlen (buf); //base指向buf后面的部分
	strcpy (base, "/stages");//buf后面接上"/stages"
	// printf("%s\n", buf);

	FILE *f = fopen (buf, "r");//打开"/stages"
	if (!f) err (1, "Can't open %s", buf);//stanford为3
	if (!fscanf (f, "%d", &stages)) errx (1, "Can't read NTF stages from %s", buf);
	fclose (f);

	*base = 0;
	strcpy (base , "/");
	// printf("base%s\n", base);
	// printf("buf%s\n", buf);
	struct dirent **tfs;//#include<dirent.h>，为了获取某文件夹目录内容
	//成功则返回复制到tfs数组中的数据结构数目，每读取一个传给filter_tfs，过滤掉不想要的，这里要.tf
	int n = scandir (buf, &tfs, filter_tfs, alphasort);//为了获取某文件夹目录内容，按字母排序
	if (n <= 0) err (1, "Couldn't find .tf files in %s", buf);
	// n = 1;//控制只取一个来实验

	nsw = xmalloc (sizeof *nsw + n * sizeof *nsw->sws);
	nsw->sws_num = n;
	nsw->stages = stages;
	for (int i = 0; i < n; i++) {//对找到的 .tf 文件处理 0到n-1
	    strcpy (base + 1, tfs[i]->d_name); //文件名，base+1写文件名，记录文件名,也就是要读取的名字
	    free (tfs[i]);
	    struct parse_sw *sw = parse_tf (buf);//解析 .tf
	    assert (sw);
	    nsw->sws[i] = sw;//保存在nsw中 
	}
	free (tfs);
	printf("%d\n", nsw->sws[0]->nrules);


	connect_link_data_gen(nsw);

	free_nsw (nsw);
	// free_tf (ttf);
}


