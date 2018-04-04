#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "bwFAT.h"

FATentry get_FATentry(int);
FATentry goto_FATentry(long int);
void print_tree(FILE*, int, int);
void print_frees(int);
int  use_free_block(int, FATentry*);
void update_timestamp(DIRentry);
int  parse_path(char**, int, DIRentry*, DIRentry*);
int  add_entry(char*, int, DIRentry, DIRentry);
int  remove_entry(DIRentry, DIRentry);
void add_free_block(int);

void bw_format(){
  FILE *disk = fopen("Drive2MB","r+"); // maybe "w" is better
  FATentry FATable[MAX_BLOCKS];
  fseek(disk, sizeof(FATentry) * MAX_BLOCKS, SEEK_SET); // goto first block
  int i;
  for(i = 0; i < MAX_BLOCKS; i++){
    if(i > 0)
      FATable[i-1].next = i; // link up the free block entries
    FATable[i].index = i;
    fgetpos(disk, &(FATable[i].block_pos)); // get position of the block (pointer)
    fseek(disk, BLOCK_SIZE, SEEK_CUR); // goto next block
  }
  FATable[i].next = -1; // MAX_BLOCKS - 1
  FATable[0].next = 3;
  FATable[ROOT_META].next = -1;
  FATable[ROOT].next = -1;
  rewind(disk);
  fwrite(FATable, sizeof(FATentry), MAX_BLOCKS, disk); // write the FAT to the disk
  fclose(disk);

  disk = fopen("Drive2MB","r+");
  FATentry rootFAT = get_FATentry(ROOT_META);
  fsetpos(disk, &rootFAT.block_pos);
  DIRentry rootD = {.filename = "root", .fileNum = 2, .dir = 1, .last = 1, .empty = 1};
  fgetpos(disk, &rootD.loc);
  fwrite(&rootD, sizeof(DIRentry), 1, disk);
  fclose(disk);
}

FATentry get_FATentry(int index){
  FILE *disk = fopen("Drive2MB","r");
  FATentry entry;
  fseek(disk, sizeof(FATentry)*index, SEEK_CUR);
  fread(&entry, sizeof(FATentry), 1, disk);
  fclose(disk);
  return entry;
}

FATentry goto_FATentry(long int floc){
  long int FATsize = sizeof(FATentry)*MAX_BLOCKS;
  int index = (int)floor((floc - FATsize)/ BLOCK_SIZE);
  return get_FATentry(index);
}

void bw_print_tree(){
  FILE *disk = fopen("Drive2MB","r"); // maybe "w" is better
  printf("\nPrinting tree:\n");
  print_tree(disk, ROOT_META, 0); // root metadata (DIRentry)
  printf("\n");
  fclose(disk);
}
void print_tree(FILE *disk, int filenum, int level){
  FATentry current = get_FATentry(filenum);
  fsetpos(disk, &current.block_pos);
  DIRentry dire;
  fpos_t resume;
  fgetpos(disk, &resume);
  do{
    fsetpos(disk, &resume);
    bw_read(&dire, sizeof(DIRentry), 1, disk);
    fgetpos(disk, &resume);
    int i;
    for(i = 0; i < level; i++)
      printf("| "); // format stuff
    printf("%s d:%d f:%d", dire.filename, dire.dir, dire.fileNum);
    current = get_FATentry(dire.fileNum);
    while(current.next != -1){
      printf(">%d", current.next);
      current = get_FATentry(current.next);
    }
    if(!dire.dir){
      struct tm stamp = dire.timestamp;
      printf("  %d-%d-%d %d:%d:%d", stamp.tm_year + 1900, stamp.tm_mon + 1, stamp.tm_mday, stamp.tm_hour, stamp.tm_min, stamp.tm_sec);
    }
    printf("\n");
    if(dire.dir && !dire.empty){
      print_tree(disk, dire.fileNum, level+1);
    }
  } while(!dire.last);
}

void bw_print_nfree(int num){
  FILE *disk = fopen("Drive2MB","r"); // maybe "w" is better
  FATentry current = get_FATentry(0);
  int i;
  printf("First %d free blocks: ", num);
  for(i = 0; i < num; i++){
    printf("%d > ", current.next);
    current = get_FATentry(current.next);
  }
  printf("...\n");
  fclose(disk);
}

// this call will remove free blocks! only call if you know what you're doing
int use_free_block(int link, FATentry *current){
  FATentry first, grab;
  first = get_FATentry(0);
  if(!first.next){
    printf("Disk is full!\n");
    return -1;
  }
  grab = get_FATentry(first.next);
  first.next = grab.next;
  grab.next = -1;

  FILE *disk = fopen("Drive2MB","r+");
  rewind(disk); //?
  fwrite(&first, sizeof(FATentry), 1, disk);
  fseek(disk, sizeof(FATentry)*grab.index, SEEK_SET);
  fwrite(&grab, sizeof(FATentry), 1, disk);

  if(link){
    fseek(disk, sizeof(FATentry)*current->index, SEEK_SET);
    current->next = grab.index;
    fwrite(current, sizeof(FATentry), 1, disk);
  }
  fclose(disk);
  return grab.index;
}

FILE *bw_open(char *path){ // permissions?
  FILE *disk = fopen("Drive2MB","r+"); // maybe "w" is better
  char *name = malloc(128);
  strcpy(name, path);
  DIRentry Cdir, Pdir;
  if(parse_path(&name, 1, &Pdir, &Cdir)){
    if(!Cdir.dir){
      FATentry current = get_FATentry(Cdir.fileNum);
      fsetpos(disk, &current.block_pos);
      update_timestamp(Cdir);
      return disk;
    }else{
      printf("Path points to a directory, not a file.\n");
      return NULL;
    }
  }
  printf("%s is not a valid path. Please try again.", path);
  return NULL;
}

int bw_close(FILE *bwfile){
  return fclose(bwfile);
}

size_t bw_read(void *ptr, size_t size, size_t nmemb, FILE *file){
  FATentry current = goto_FATentry(ftell(file));
  size_t total = size * nmemb;
  size_t FATsize = sizeof(FATentry)*MAX_BLOCKS;

  FILE *temp = tmpfile();
  if(temp == NULL){
    printf("Couldn't create temp file.\n");
    return (size_t)-1;
  }
  char r;
  int i;
  int loc;
  for(i = 0; i < total && r != EOF; i++){
    r = fgetc(file);
    fputc(r, temp);
    loc = (int)ftell(file) - FATsize;
    if(loc % BLOCK_SIZE == 0){
      if(current.next == -1){ // necessary?
        printf("File isn't big enough to read all of that.\n");
          return (size_t)-1;
      }
      current = get_FATentry(current.next);
      fsetpos(file, &current.block_pos);
    }
  }

  rewind(temp);
  fread(ptr, size, nmemb, temp);
  fclose(temp);
  return total;
}

size_t bw_write(void *ptr, size_t size, size_t nmemb, FILE *file){
  FATentry current = goto_FATentry(ftell(file));
  size_t total = size * nmemb;
  size_t FATsize = sizeof(FATentry)*MAX_BLOCKS;

  char *r = (char*)ptr;
  int i;
  int loc;
  for(i = 0; i < total && *r != EOF; i++, r++){
    fputc(*r, file);
    loc = (int)ftell(file) - FATsize;
    if(loc % BLOCK_SIZE == 0){
      if(current.next == -1){
        if(!use_free_block(1, &current))
          return (size_t)-1;
      }
      current = get_FATentry(current.next);
      fsetpos(file, &current.block_pos);
    }
  }
  return total;
}

void update_timestamp(DIRentry dire){
  FILE *disk = fopen("Drive2MB","r+");
  fsetpos(disk, &dire.loc);
  time_t thistime = time(NULL);
  struct tm stamp = *localtime(&thistime);
  dire.timestamp = stamp;
  bw_write(&dire, sizeof(DIRentry), 1, disk);
  fclose(disk);
}

int bw_create_file(char *path){
  char *name = malloc(128);
  strcpy(name, path);
  DIRentry lastdire, parentdire;
  if(parse_path(&name, 0, &parentdire, &lastdire) > 0){
    return add_entry(name, 0, parentdire, lastdire);
  }else
    return -1;
}

int bw_create_dir(char *path){
  char *name = malloc(128);
  strcpy(name, path);
  DIRentry lastdire, parentdire;
  if(parse_path(&name, 0, &parentdire, &lastdire) > 0){
    return add_entry(name, 1, parentdire, lastdire);
  }else
    return -1;
}

int parse_path(char **path, int hitMode, DIRentry *Pdir, DIRentry *Cdir){
  FILE *disk = fopen("Drive2MB", "r");
  char *curpath = malloc(128);
  FATentry current;

  current = get_FATentry(ROOT_META); // get root metadata entry
  fsetpos(disk, &current.block_pos); // goto root metadata (DIRentry)
  bw_read(Pdir, sizeof(DIRentry), 1, disk);

  current = get_FATentry(ROOT); // get root entry - same as parentdire.fileNum
  fsetpos(disk, &current.block_pos); // goto root block

  int cont;
  curpath = strtok(*path, "/");
  do{ // what goes in? ---> parent DIRentry
    cont = 0;
    if(Pdir->empty)
      break;
    bw_read(Cdir, sizeof(DIRentry), 1, disk); // read next entry
    if(strcmp(Cdir->filename, curpath) == 0){ // if the path matches a file or directory
      current = get_FATentry(Cdir->fileNum); // goto matching dir
      fsetpos(disk, &current.block_pos);    // goto matching dir
      strcpy(*path, curpath); //?
      curpath = strtok(NULL, "/");
      if(curpath == NULL){
        if(hitMode){
          fclose(disk);
          return 1;
        } else {
          printf("There's already a file with name %s. Try again.\n", *path);
          fclose(disk);
          return -1;
        }
      } else if(Cdir->dir){ // directory match - switch to search for filename
        memcpy(Pdir, Cdir, sizeof(DIRentry));
        cont = 1;
      } else {
        printf("A member of the path is a file (not a directory). Try again.\n"); // heh
        fclose(disk);
        return -1;
      }
    }
  } while(!Cdir->last || cont); // once i'm out of entries to check

  strcpy(*path, curpath);
  curpath = strtok(NULL, "/");
  fclose(disk);
  if(curpath == NULL && !hitMode){
    return 1;
  } else {
    printf("No matches found in path. Try again.\n");
    return -1;
  }
}

int add_entry(char *name, int directory, DIRentry Pdir, DIRentry Cdir){
  FILE *disk = fopen("Drive2MB","r+");
  DIRentry writedire;

  if(Pdir.empty){
    writedire = Pdir;
    writedire.empty = 0;
    fsetpos(disk, &Pdir.loc);
    bw_write(&writedire, sizeof(DIRentry), 1, disk);
    FATentry current = get_FATentry(writedire.fileNum);
    fsetpos(disk, &current.block_pos);
  } else {
    writedire = Cdir;
    writedire.last = 0;
    fsetpos(disk, &Cdir.loc);
    bw_write(&writedire, sizeof(DIRentry), 1, disk); // it's not last anymore!
  }

  int filenum = use_free_block(0, NULL);
  if(!filenum)
    return -1;
  char *copyname = malloc(64); //?
  strcpy(copyname, name); //?
  DIRentry add = {.filename = copyname, .dir = directory, .fileNum = filenum, .last = 1, .empty = directory};
  fgetpos(disk, &add.loc);
  bw_write(&add, sizeof(DIRentry), 1, disk);
  fclose(disk);

  if(directory){
    printf("Done making directory %s.\n", add.filename);
  }else{
    update_timestamp(add);
    printf("Done making file %s.\n", add.filename);
  }
  return 1;
}

int bw_delete_file(char *path){
  char *name = malloc(64);
  strcpy(name, path);
  DIRentry parentdire, lastdire;
  if(parse_path(&name, 1, &parentdire, &lastdire) > 0){ // hit mode
    if(!lastdire.dir){
      remove_entry(parentdire, lastdire);
      return 1;
    }else{
      printf("The path points to a directory, not a file.\n");
    }
  }else{
    printf("Path is not valid.\n");
    return -1;
  }
}

int bw_delete_dir(char *path){
  char *name = malloc(64);
  strcpy(name, path);
  DIRentry parentdire, lastdire;
  if(parse_path(&name, 1, &parentdire, &lastdire) > 0){ // hit mode
    if(lastdire.dir){
      remove_entry(parentdire, lastdire);
      return 1;
    }else{
      printf("The path points to a file, not a directory.\n");
    }
  }else{
    printf("Path is not valid.\n");
    return -1;
  }
}

int remove_entry(DIRentry Pdir, DIRentry rem){
  FILE *disk = fopen("Drive2MB","r+");
  FATentry current = get_FATentry(Pdir.fileNum);
  fsetpos(disk, &current.block_pos); // goto directory

  DIRentry Cdir;
  bw_read(&Cdir, sizeof(DIRentry), 1, disk);
  if(Cdir.last && strcmp(Cdir.filename, rem.filename) == 0){ // name check not necessary
    fsetpos(disk, &Pdir.loc);
    DIRentry writedire = Pdir;
    writedire.empty = 1;
    bw_write(&writedire, sizeof(DIRentry), 1, disk);
  } else {
    DIRentry secondtolast;
    while(!Cdir.last){ // get last entry
      memcpy(&secondtolast, &Cdir, sizeof(DIRentry));
      bw_read(&Cdir, sizeof(DIRentry), 1, disk);
    }
    // swap into rem and update entries
    fsetpos(disk, &rem.loc);
    fgetpos(disk, &Cdir.loc);
    Cdir.last = 0;
    bw_write(&Cdir, sizeof(DIRentry), 1, disk);

    fsetpos(disk, &secondtolast.loc);
    bw_read(&secondtolast, sizeof(DIRentry), 1, disk);
    fsetpos(disk, &secondtolast.loc);
    secondtolast.last = 1;
    bw_write(&secondtolast, sizeof(DIRentry), 1, disk);
  }
  fclose(disk);

  int next = rem.fileNum;
  while(next != -1){ // add blocks to free list
    current = get_FATentry(next);
    next = current.next;
    add_free_block(current.index);
  }
  printf("%s was removed.\n", rem.filename);
  return 1;
}

void add_free_block(int index){
  FATentry first, put;
  first = get_FATentry(0);
  put = get_FATentry(index);
  put.next = first.next;
  first.next = put.index; // same as param index but yeah

  printf("block %d has been freed.\n", put.index);
  FILE *disk = fopen("Drive2MB","r+");
  fwrite(&first, sizeof(FATentry), 1, disk);
  fseek(disk, sizeof(FATentry)*put.index, SEEK_SET); // same as param index
  fwrite(&put, sizeof(FATentry), 1, disk);
  fclose(disk);
}
