gcc -c test.c
gcc -c bwFAT.c
gcc -o bwFAT_test test.o bwFAT.o -lm
rm test.o
rm bwFAT.o
bwFAT_test