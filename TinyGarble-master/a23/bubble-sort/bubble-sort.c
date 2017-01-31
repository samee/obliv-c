#define G_SIZE 31 // size of Garbler's input array
#define E_SIZE 31 // size of Evaluator's input array
#define O_SIZE 1 // size of output array

void bubble_sort(int* array) {
  for (int j = 0; j < G_SIZE - 1; ++j) {
    for (int i = 0; i < G_SIZE - j - 1; ++i) {
      if (array[i] > array[i + 1]) {
        int temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
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


  bubble_sort(array);

  o[0] = array[G_SIZE/2];
}