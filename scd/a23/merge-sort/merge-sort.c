#define G_SIZE 31 // size of Garbler's input array
#define E_SIZE 31 // size of Evaluator's input array
#define O_SIZE 1 // size of output array

int b[G_SIZE]; // temporary array for mergesort
void mergesort(int* array, int l, int r) {
  int i, j, k, m;
  if (r > l) {
    m = (r + l) / 2;
    mergesort(array, l, m);
    mergesort(array, m + 1, r);
    for (i = m + 1; i > l; i--)
      b[i - 1] = array[i - 1];
    for (j = m; j < r; j++)
      b[r + m - j] = array[j + 1];
    for (k = l; k <= r; k++) {
      if (b[i] < b[j]) {
        array[k] = b[i++];
      } else {
        array[k] = b[j--];
      }
    }
  }
}

void gc_main(const int *g,  // Garbler's input array
             const int *e,           // Evaluator's input array
             int *o                  // output array
            ) {

  int array[G_SIZE];

  for (int i = 0; i < G_SIZE; ++i) {
    array[i] = g[i] ^ e[i]; // xor-shared input
  }

  mergesort(array, 0, G_SIZE - 1);

  o[0] = array[G_SIZE / 2];
}