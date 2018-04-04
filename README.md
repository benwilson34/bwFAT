# bwFAT
a FAT file system for linux

## bwFAT API:
NOTE: Remember to use `#include “bwFAT.h”` in source files, and compile files with bwFAT.o (using `gcc -c bwFAT.c`). Compile an object file from test.c with `gcc -c test.c` then compile with `gcc bwFAT.o test.o -lm`. Remember the -lm , you need the math library!!!!

Function | Description
--- | ---
void bw_format(void); | Formats the file “Drive2MB” for use with the bwFAT API. The bwFAT.c source must be reworked if you wish to use a different file.
void bw_print_tree(void); | Recursively prints the entire directory tree, directory status, file numbers, and timestamp if applicable.
void bw_print_nfree(int num); | Prints the first num free blocks.
FILE \*bw_open(char \*path); | Opens the file at path for reading or writing. 
int  bw_close(FILE \*bwfile); | Closes the file bwfile.
size_t bw_read(void \*ptr, size_t size, size_t nmemb, FILE \*bwfile); | Reads (size * nmemb) bytes from the file bwfile into the buffer ptr. | size_t bw_write(void \*ptr, size_t size, size_t nmemb, FILE \*bwfile); | Reads (size * nmemb) bytes from the buffer ptr into the file bwfile.
int  bw_create_file(char \*path); | Creates a file at path if it does not already exist.
int  bw_create_dir(char \*path); | Creates a directory at path if it does not already exist.
int  bw_delete_file(char \*path); | Deletes the file at path if it exists.
int  bw_delete_dir(char \*path); | Deletes the directory at path if it exists.

Enjoy!

Last update: April 27, 2016 - 11:20 pm EST
