/* genrmf maxflow input generator */

#include <stdio.h>
#include "gen_maxflow_typedef.h"
#include "genio.h"


void print_usage(void);

void main(int argc, char * argv[])
{
  network * n;
  int i, feas, quiet;
  FILE * output;
  int a, b, c1, c2;
  char comm[10][80];
  char * com1[10];
  int seed;

  output = stdout;
  a = -1;
  b = -1;
  c1 = -1;
  c2 = -1;
  seed = -1;
  
  for (i = 1; i < argc; i++){
    switch (look_up(argv[i]))
    {
      case 0: 
      output = fopen(argv[++i],"w");
      if (output == NULL) {
        fprintf(stderr
                    ,"genrmf: Output file %s can't be opened\n",argv[i]);
        exit(-1);
      }  
      break;
    
      case 1: 
      a = atoi(argv[++i]);
    
      break;
    
      case 2: 
      b = atoi(argv[++i]);
    
      break;
      case 3: 
      c1 = atoi(argv[++i]);
      break;
      case 4: 
      c2 = atoi(argv[++i]);
      break;
      case 5: 
      seed = atoi(argv[++i]);
      break;
      default:
      break;
    } 
  }

  if (a == -1 || b == -1 || c1 == -1 || c2 == -1)
    print_usage();
  
  if (seed == -1)
    seed = (int) time(0);
  
  srand48(seed);
  
  n = gen_rmf(a, b, c1, c2);

  sprintf(comm[0], "This file was generated by genrmf.");
  sprintf(comm[1], "The parameters are: a: %d b: %d c1: %d c2: %d seed: %d"
      , a, b, c1, c2,seed);

  com1[0] = comm[0];
  com1[1] = comm[1];
  
  print_max_format(output, n, com1, 2);

  gen_free_net(n);

  if (output != stdout)
    fclose(output);
}
/*=================================================================*/
#define OPS_NUM 6

int look_up(char * s)
{
  char * ops[OPS_NUM] 
    = { "-out", "-a", "-b", "-c1", "-c2", "-seed"};
  int i;
  
  for (i = 0; i < OPS_NUM; i++){
    if (strcmp(ops[i], s) == 0)
      return i;
  } 
  return -1;
} 

void print_usage(void)
{
  printf("Usage: genrmf [-out out_file]\n");
  printf("              -a frame_size -b depth\n");
  printf("              -c1 cap_range1 -c2 cap_range2 -seed SEED\n");
  printf("\n\
  Here without the -out option the generator will\n\
  write to stdout.\n\
\n\
  The generated network is as follows:\n\
  It has b pieces of frames of size (a x a).\n\
  (So alltogether the number of vertices is a*a*b)\n\
\n\
  In each frame all the vertices are connected with \n\
  their neighbours. (forth and back)\n\
  In addition the vertices of a frame are connected\n\
  one to one with the vertices of next frame using \n\
  a random permutation of those vertices.\n\
\n\
  The source is the lower left vertex of the first frame,\n\
  the sink is the upper right vertex of the b'th frame. \n\
\n\
  The capacities are randomly chosen integers\n\
  from the range of (c1, c2) in the case of interconnecting\n\
  edges, and c2 * a * a or the in-frame edges.\n\
\n\
  This generator was used by U. Derigs & W. Meier (1989)\n\
  in the article 'Implementing Goldberg's Max-Flow-Algorithm\n\
  A Computational Investigation'\n\
  ZOR - Methods & Models of OR (1989) 33:383-403\n\
\n");

  exit(0);
}

