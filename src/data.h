#ifndef _DATA_H_
#define _DATA_H_

//.h声明内容
#include "array.h"
#include "parse.h"

//.c中声明内容
#include "tf.h" 
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>

//.h正式内容
struct PACKED file {
  uint32_t arrs_ofs, strs_ofs;
  uint32_t ntfs, stages;
  uint32_t tf_ofs[0];
};

#define VALID_OFS 1

// extern struct file *data_file;
// extern uint8_t     *data_raw;
// extern size_t       data_size;

#define DATA_ARR(X) ( data_arrs + ((X) - VALID_OFS) / sizeof (array_t) )
#define DATA_STR(X) ( data_strs + ((X) - VALID_OFS) )

// extern array_t *data_arrs;
// extern uint32_t data_arrs_len, data_arrs_n;
// extern char    *data_strs;

void data_load   (const char *file);
void data_unload (void);

void data_gen (const char *out, const struct parse_ntf *ntf, const struct parse_tf *ttf);

#endif


//.c正式内容

//全局变量指代文件
struct file *data_file; //指针指向全局文件
uint8_t     *data_raw; //指针指向读取文件的副本映射
size_t       data_size;

array_t *data_arrs;
uint32_t data_arrs_len, data_arrs_n;
char    *data_strs;

struct PACKED arrs {
  uint32_t len, n;
  array_t arrs[0];
};




void
data_load (const char *name)
{
  int fd = open (name, O_RDONLY);
  if (fd < 0) err (1, "open(%s) failed", name);
  data_size = lseek (fd, 0, SEEK_END); //欲将读写位置移到文件尾,返回目前的读写位置, 也就是距离文件开头多少个字节
  // printf ("%d \n",data_size);
  assert (data_size >= 0);

  data_raw = mmap (NULL, data_size, PROT_READ, MAP_PRIVATE, fd, 0); //指针指向映射
  if (data_raw == MAP_FAILED) err (1, "mmap() failed");
  close (fd);
  data_file = (struct file *) data_raw; //指针指向强制化file的映射,提取前8字节

  struct arrs *arrs = (struct arrs *) (data_raw + data_file->arrs_ofs);
  data_arrs_len = arrs->len;
  // printf ("%d \n",data_arrs_len);
  data_arrs_n = arrs->n;
  data_arrs = arrs->arrs;
  data_strs = (char *) (data_raw + data_file->strs_ofs);
}

void
data_unload (void)
{ munmap (data_raw, data_size); }
