
#define G_SIZE 1 // size of Garbler's input array
#define E_SIZE 1 // size of Evaluator's input array
#define O_SIZE 64 // size of output array

void gc_main(const int *g,  // Garbler's input array
    const int *e,           // Evaluator's input array
    int *o                  // output array
    ) {
  int j;
  for(j=0;j<O_SIZE;j++) {
    o[j] = j;
  }
}


