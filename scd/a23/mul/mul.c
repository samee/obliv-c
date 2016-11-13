#define G_SIZE 1
#define E_SIZE 1
#define O_SIZE 1

void gc_main(const int *g,  // Garbler's input array
             const int *e,           // Evaluator's input array
             int *o                  // output array
            ) {
  o[0] = g[0]*e[0];
}