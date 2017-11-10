/* File:     Blocked matrix multiplication
 * From: http://www.cs.usfca.edu/~peter/math202/blocked.c
 *
 * Purpose:  Compare the run time of the standard matrix multiplication
 *           algorithm with blocked matrix multiplication.
 *
 * Compile:  gcc -g -Wall -I. [-DDEBUG] -o blocked blocked.c
 * Run:      ./blocked <order of matrices> <order of blocks> [i]
 *              <-> required argument, [-] optional argument
 *
 * Input:    If the "i" flag is given on the command line,
 *           the two matrices must be input.
 * Output:   Elapsed time for the two multiplication methods.
 *           If the DEBUG flag is set, the product matrix as
 *           computed by each method is also output.
 *
 * Notes:
 * 1.  The file timer.h should be in the directory containing
 *     the source file.
 * 2.  The order of the blocks (b) must evenly divide the order 
 *     of the matrices (n)
 * 3.  Set the DEBUG flag to see the product matrices
 * 4.  If the command line flag "i" isn't present, matrices are 
 *     generated using a random number generator.
 * 5.  There are a number of optimizations that can be made to 
 *     the source code that will improve the performance of both
 *     algorithms.
 * 6.  Note that unless the DEBUG flag is set the product matrices will, 
 *     in general, be different using the two algorithms, since the two 
 *     algorithms use identical storage for A and B, but they assume 
 *     the storage has different structures.
 * 7.  If the matrix order is n and the block size is b, define
 *     
 *        n_bar = n/b = number of block rows = number of block cols
 *        b_sqr = b*b = size of the blocks
 *
 *     If we're in block row i_bar, block column j_bar, then the 
 *     address of the first entry in matrix X will be 
 *
 *        X + (i_bar*n_bar + j_bar)*b_sqr
 *
 *     The remainder of the b x b block is stored in the next b^2
 *     memory locations.
 * 8.  This has received *very* little testing.  Students who find
 *     and correct bugs will receive many gold stars.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memset
#include "timer.h"

#define PAD 120

struct record {
    double value;
    char padding[PAD];
};

// Global Variables 
const double DRAND_MAX = RAND_MAX;
struct record *A, *B, *C;
struct record *C_p;
int n, b;
int n_bar, b_sqr;

void Usage(char prog_name[]);
void Get_matrices(struct record A[], struct record B[], int n, int argc);
void Blocked_mat_mult(void);
void Zero_C(int i_bar, int j_bar);
void Mult_add(int i_bar, int j_bar, int k_bar);
void Print_matrix(struct record C[], int n);
void To_blocked(struct record A[], int n, int b);
void From_blocked(struct record C[], int n, int b);

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   double start2, finish2;
	volatile char MARKER_START, MARKER_END;
	/* Record marker addresses */
	FILE* marker_fp = fopen("blocked.marker","w");
	if(marker_fp == NULL ) {
		perror("Couldn't open marker file:");
		exit(1);
	}
	fprintf(marker_fp, "%p %p", &MARKER_START, &MARKER_END );
	fclose(marker_fp);

	MARKER_START = 33;

   if (argc < 3 || argc > 4) Usage(argv[0]);
   n = strtol(argv[1], NULL, 10);
   b = strtol(argv[2], NULL, 10);
   if (n % b != 0) Usage(argv[0]);
   A = malloc(n*n*sizeof(struct record));
   B = malloc(n*n*sizeof(struct record));
   C = malloc(n*n*sizeof(struct record));
   if (A == NULL || B == NULL || C == NULL) {
      fprintf(stderr, "Can't allocate storage!\n");
      exit(-1);
   }

   n_bar = n/b;
   b_sqr = b*b;
   Get_matrices(A, B, n, argc);

#  ifdef DEBUG
   To_blocked(A, n, b);
   To_blocked(B, n, b);
#  endif
   GET_TIME(start2);
   Blocked_mat_mult();
   GET_TIME(finish2);
#  ifdef DEBUG 
   printf("Blocked algorithm\n");
   From_blocked(C, n, b);
   Print_matrix(C, n);
#  endif

   printf("Elapsed time for blocked algorithm = %e seconds\n",
         finish2-start2);

   free(A);
   free(B);
   free(C);
	MARKER_END = 34;
   return 0;
}  /* main */

/*-------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message showing how the program is used and quit
 * In arg:    prog_name:  the program name
 */
void Usage(char prog_name[]) {
   fprintf(stderr, "usage:  %s <order of matrices> <order of blocks>k[i]\n",
         prog_name);
   fprintf(stderr, "   order of blocks must evenly divide order of matrices\n");
   fprintf(stderr, "   i indicates that the user will input A and B\n");
   exit(0);
}  /* Usage */


/*-------------------------------------------------------------------
 * Function:  Get_matrices
 * Purpose:   Read in the factor matrices from stdin or generate
 *            them if argc == 3
 * In args:   n:  order of the matrices
 *            argc:  3, generate matrices, 4 read in the matrices
 * Out args:  A, B: the matrices
 */
void Get_matrices(struct record A[], struct record B[], int n, int argc) {
   int i;

   for (i = 0; i < n*n; i++) {
       A[i].value = random()/DRAND_MAX;
       B[i].value = random()/DRAND_MAX;
   }
}  /* Get_matrices */


/*-------------------------------------------------------------------
 * Function:     Zero_C
 * Purpose:      Assign 0 to the current block of C and set the global
 *               pointer C_p to refer to this block
 * In args:      i_bar:  current block row
 *               j_bar:  current block col
 * Globals in:   n_bar:  the number of blocks
 *               b_sqr:  square of the blocksize
 * Globals out:  C_p:  pointer to the start of the current block of C
 */
void Zero_C(int i_bar, int j_bar) {
   C_p = C + (i_bar*n_bar + j_bar)*b_sqr;

   memset(C_p, 0, b_sqr*sizeof(struct record));
}  /* Zero_C */

/*-------------------------------------------------------------------
 * Function:      Mult_add
 * Purpose:       Add the product of the current blocks of A and B
 *                into C
 * In args:       i_bar:  current block row in C and A
 *                j_bar:  current block col in C and B
 *                k_bar:  current block col in A, block row in B
 * Globals in:    C_p:  pointer to start of current block of C
 *                A:  the factor matrix A
 *                B:  the factor matrix B
 *                n_bar:  the number of blocks = n/b
 *                b:  the blocksize
 *                b_sqr:  b*b
 * Global in/out: C:  the product matrix
 */
void Mult_add(int i_bar, int j_bar, int k_bar) {
   struct record *c_p = C_p;
   struct record *a_p = A + (i_bar*n_bar + k_bar)*b_sqr;
   struct record *b_p = B + (k_bar*n_bar + j_bar)*b_sqr;
   int i, j, k;

   for (i = 0; i < b; i++)
      for (j = 0; j < b; j++) 
         for (k = 0; k < b; k++)
            (*(c_p + i*b + j)).value += 
               (*(a_p + i*b+k)).value*(*(b_p + k*b + j)).value;
}  /* Mult_add */

/*-------------------------------------------------------------------
 * Function:      Blocked_mat_mult
 * Purpose:       Implement blocked matrix-matrix multiplication
 * Globals in:    n_bar:  the number blocks = n/b
 *                A, B: the factor matrices (used in Mat_mult)
 * Globals out:   C, C_p:  the product matrix, and pointer to current
 *                   block in product matrix
 */
void Blocked_mat_mult(void){
   int i_bar, j_bar, k_bar;  // index block rows and columns

   for (i_bar = 0; i_bar < n_bar; i_bar++)
      for (j_bar = 0; j_bar < n_bar; j_bar++) {
         Zero_C(i_bar, j_bar);
         for (k_bar = 0; k_bar < n_bar; k_bar++) 
            Mult_add(i_bar, j_bar, k_bar);
      }
}  /* Blocked_mat_mult */

/*-------------------------------------------------------------------
 * Function:  Print_matrix
 * Purpose:   Print a matrix on stdout
 * In args:   n:  order of matrix
 *            A:  the matrix
 */
void Print_matrix(struct record C[], int n) {
   int i, j;

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++)
         printf("%.2e ", C[i*n+j].value);
      printf("\n");
   }
}  /* Print_matrix */

/*-------------------------------------------------------------------
 * Function:    To_blocked
 * Purpose:     Convert the matrix A from row-major to blocked storage
 * In args:     n:  order of matrix
 *              b:  blocksize
 * In/out arg:  A:  on input matrix stored in row-major format, on
 *                  output matrix stored in blocked format
 */
void To_blocked(struct record A[], int n, int b) {
   int i, j;
   int i_bar, j_bar;  // index block rows and block cols
   int n_bar = n/b;
   struct record *T, *a_p, *t_p;

   T = malloc(n*n*sizeof(struct record));
   if (T == NULL) {
      fprintf(stderr, "Can't allocate temporary in To_blocked\n");
      exit(-1);
   }

   // for each block in A
   t_p = T;
   for (i_bar = 0; i_bar < n_bar; i_bar++)
      for (j_bar = 0; j_bar < n_bar; j_bar++) {

         // Copy block into contiguous locations in T
         a_p = A + (i_bar*b*n + j_bar*b);
         for (i = 0; i < b; i++, a_p += (n-b)) 
            for (j = 0; j < b; j++) {
				(*t_p).value = (*a_p).value;
               	t_p++;
				a_p++;
            }
      }   

   memcpy(A, T, n*n*sizeof(struct record));

   free(T);
}  /* To_blocked */

/*-------------------------------------------------------------------
 * Function:    From_blocked
 * Purpose:     Convert the matrix C from blocked storage to row-major 
 *              storage
 * In args:     n:  order of matrix
 *              b:  blocksize
 * In/out arg:  C:  on input matrix stored in blocked format, on
 *                  output matrix stored in row-major format
 */
void From_blocked(struct record C[], int n, int b) {
   int i, j;
   int i_bar, j_bar;  // index blocks of C
   int n_bar = n/b;
   struct record *T, *c_p, *t_p;

   T = malloc(n*n*sizeof(struct record));
   if (T == NULL) {
      fprintf(stderr, "Can't allocate temporary in To_blocked\n");
      exit(-1);
   }

   // for each block of C
   c_p = C;
   for (i_bar = 0; i_bar < n_bar; i_bar++)
      for (j_bar = 0; j_bar < n_bar; j_bar++) {

         // Copy block into correct locations in T
         t_p = T + (i_bar*b*n + j_bar*b);
         for (i = 0; i < b; i++, t_p += (n-b))
            for (j = 0; j < b; j++) {
               (*t_p).value = (*c_p).value;
			   t_p++;
			   c_p++;
            }
      }

   memcpy(C, T, n*n*sizeof(struct record));
   free(T);
}  /* From_blocked */
