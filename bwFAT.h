#ifndef BWFAT_H
#define BWFAT_H

#include <time.h>

#define ROOT_META 1
#define ROOT 2
#define BLOCK_SIZE 512
#define MAX_BLOCKS 3900 // actual max is ~4096 but this attempts to account for the size of the FAT

typedef struct FATentry FATentry;
struct FATentry {
  int index;
  fpos_t block_pos;
  int next;
};

typedef struct DIRentry DIRentry;
struct DIRentry{
  fpos_t loc;
  char *filename;
  int dir;
  int fileNum;
  int empty;
  int last;
  struct tm timestamp;
};

void bw_format(void);
void bw_print_tree(void);
void bw_print_nfree(int);
FILE *bw_open(char*);
int  bw_close(FILE*);
size_t bw_read(void*, size_t, size_t, FILE*);
size_t bw_write(void*, size_t, size_t, FILE*);
int  bw_create_file(char*);
int  bw_create_dir(char*);
int  bw_delete_file(char*);
int  bw_delete_dir(char*);

#endif
