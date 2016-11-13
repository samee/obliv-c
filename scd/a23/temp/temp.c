
#define G_SIZE 32 // size of Garbler's input array
#define E_SIZE 32 // size of Evaluator's input array
#define O_SIZE 1 // size of output array

typedef unsigned char uint8_t;
typedef unsigned int  uint32_t;

inline hasCarry(unsigned int a, unsigned int b, unsigned int s){
  return (((a>>31)^(s>>31))&((b>>31)^(s>>31)))^(a>>31)^(b>>31)^(s>>31);
}


void gc_main(const unsigned int *g,  // Garbler's input array
    const unsigned int *e,           // Evaluator's input array
    unsigned int *o                  // output array
    ) {

  int carry = 0;
  for (int i = 0; i < G_SIZE; i++) {
    o[i] = g[i] + e[i] + carry;
    carry = hasCarry(g[i], e[i], o[i]);
  }
}
