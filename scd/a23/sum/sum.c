
#define G_SIZE 2 // size of Garbler's input array
#define E_SIZE 2 // size of Evaluator's input array
#define O_SIZE 2 // size of output array

void gc_main(const int *g,  // Garbler's input array
    const int *e,           // Evaluator's input array
    int *o                  // output array
    ) {
  for (int i = 0; i < G_SIZE; ++i) {
    o[i] = g[i] + e[i];
  }
}
