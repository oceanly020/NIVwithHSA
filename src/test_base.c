
#define _GNU_SOURCE 1


#include "base.h"
#include <libgen.h>
#include <linux/limits.h>
#include <sys/time.h>
#include <unistd.h>


int
main (int argc, char **argv)
{	
	// char *s = "1000000000001100000000000001000x";
	parse_dir ("data", "tfs", "stanford");
	// parse_dir ("data", "tfs", "stanford_whole");
	// parse_dir ("data", "tfs", "i2");

	// generate_connect_link ("data", "tfs", "stanford_whole");

	// struct mf_uint16_t *mf = mf_from_str(s);
	// for (int i = 0; i < MF_LEN; i++) {
	// 	print_bin(mf->mf_w[i]);
	// }
	// printf("\n");
	// for (int i = 0; i < MF_LEN; i++) {
	// 	print_bin(mf->mf_v[i]);	
	// }
	// printf("\n");

 //  return 0;
}