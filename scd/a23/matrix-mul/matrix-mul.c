
// max 8
#define N 3

#define G_SIZE (N*N) // size of Garbler's input array
#define E_SIZE (N*N) // size of Evaluator's input array
#define O_SIZE (N*N) // size of output array



void gc_main(const int *g,  // Garbler's input array
             const int *e,           // Evaluator's input array
             int *o                  // output array
            ) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int temp = 0;
      for (int k = 0; k < N; ++k) {
        temp += g[i * N + k] * e[k * N + j];
      }
      o[i * N + j] = temp;
    }
  }
}