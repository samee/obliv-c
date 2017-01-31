#define G_SIZE 2 // size of Garbler's input array
#define E_SIZE 2 // size of Evaluator's input array
#define O_SIZE 1 // size of output array

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

/*void gc_main(const int *g,  // Garbler's input array
 const int *e,           // Evaluator's input array
 int *o                  // output array
 ) {

 const uint8_t* g8 = (const uint8_t*) g;
 const uint8_t* e8 = (const uint8_t*) e;

 unsigned int ret = 0;
 for(int j = 0; j < G_SIZE*sizeof(int); j++) {
 unsigned int retj = 0;
 unsigned int t = (unsigned int)(g8[j]^e8[j]);
 for(unsigned int i = 0; i < 8; i++) {
 retj += (t&(1<<i))>>i;
 }
 ret += retj;
 }
 o[0] = ret;
 }*/

inline int hamming_byte(const uint8_t g, const uint8_t e) {
  uint8_t t = g ^ e;
  uint32_t retj = 0;
  for (uint32_t i = 0; i < 8; i++) {
    retj += (t & (1 << i)) >> i;
  }
  return retj;
}

inline int log2(uint32_t a) {
  uint32_t l = 0;
  while (a >>= 1) {
    ++l;
  }
  return l;
}

void gc_main(const int *g,  // Garbler's input array
    const int *e,           // Evaluator's input array
    int *o                  // output array
    ) {

  const uint8_t* g8 = (const uint8_t*) g;
  const uint8_t* e8 = (const uint8_t*) e;

  uint8_t dist[G_SIZE * sizeof(int)];

  for (int j = log2(G_SIZE * sizeof(int)); j >= 0; j--) {
    for (int i = 0; i < (1 << j); i++) {
      if (j == log2(G_SIZE * sizeof(int))) {
        dist[i] = hamming_byte(g8[i], e8[i]);
      } else {
        dist[i] = dist[2 * i] + dist[2 * i + 1];
      }
    }
  }
  o[0] = dist[0];
}