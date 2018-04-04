#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bwFAT.h"

int main(int argc, char **argv){
  bw_format();

  bw_create_file("manual.txt");
  bw_create_file("something.c");
  bw_create_dir("sample");
  bw_create_dir("subdirA");
  bw_create_dir("subdirD");
  bw_create_dir("sample");
  bw_create_dir("subdirF");

  bw_print_tree();

  // test all errors here
  bw_create_dir("subdirD/subdir100");
  bw_create_file("subdirA/example.txt");
  bw_create_file("subdirA/existentialism.txt");
  bw_create_file("subdirA/howmuchtimehaveispentonthis.txt");
  bw_create_file("subdirA/example2.txt");
  bw_create_file("subdirA/creationism.txt");
  bw_create_file("subdirA/creationism2.txt");
  bw_create_file("subdirA/creationism4.txt");
  bw_create_file("subdirA/creationism5.txt");
  bw_create_file("subdirA/creationism6.txt");
  bw_create_file("subdirA/creationism7.txt");
  bw_create_file("subdirA/creationism8.txt");
  bw_create_file("subdirA/creationism9.txt");
  bw_create_file("subdirA/creationism10.txt");
  bw_create_file("subdirA/creationism11.txt");
  bw_create_file("subdirA/creationism12.txt");
  bw_create_dir("subdirD/subdir100/subdirB");
  bw_create_file("idkkkkkkkkkkk.c");

  bw_print_tree();

  bw_delete_file("subdirA/example.txt");
  bw_delete_dir("subdirD/subdir100/subdirB");
  bw_delete_file("something.c");
  bw_delete_file("subdirA/creationism4.txt");
  bw_delete_file("subdirA/creationism5.txt");
  bw_delete_file("subdirA/creationism6.txt");

  bw_print_nfree(10);
  bw_print_tree();

  // test all errors here
  FILE *myFile = bw_open("subdirA/example2.txt");
  char *test = malloc(1052);
  strcpy(test, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas at iaculis est. \
Duis eget est eu justo sollicitudin pulvinar. In hac habitasse platea dictumst. Suspendisse at consectetur \
ante. Interdum et malesuada fames ac ante ipsum primis in faucibus. Ut in ex commodo, dictum nibh et, \
aliquet nisl. Nunc imperdiet odio sed luctus facilisis. Pellentesque rutrum vulputate elit, nec luctus \
ipsum scelerisque vitae. Quisque suscipit accumsan lorem, vel pulvinar augue ultrices in. Aliquam lorem \
massa, gravida nec leo et, dapibus pulvinar orci. Mauris sit amet pulvinar ipsum. Duis enim mi, porta sit \
amet libero id, cursus varius purus. Aenean imperdiet dui mauris, eget convallis mauris pharetra eu. Proin \
tempor risus vitae sodales dignissim. Ut et tellus eros. Nulla varius ante eget pharetra cursus.\
Aenean sed luctus magna, id porta lorem. Cras ante diam, fringilla nec euismod vitae, tempor vel sapien. \
Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nunc viverra \
luctus leo, eu bibendum nullam.");
  bw_write(test, sizeof(char), 1052, myFile);
  bw_close(myFile);

  myFile = bw_open("subdirA/example2.txt");
  //printf("Opened.\n");
  char *pickup = malloc(1053);
  bw_read(pickup, sizeof(char), 1052, myFile);
  bw_close(myFile);
  printf("I read: \"%s\"\n", pickup);

  return 0;
}
