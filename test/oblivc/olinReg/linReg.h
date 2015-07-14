// Fixed point arithmetic
#define SCALE (1 << 16) // 2^16
#define DESCALE(x) x / SCALE // for use in multiplication
//#define MAXN 5001
//#define SEED 5 // used for osqrt(), adjust for average values of given dataset

typedef struct {
  char *src; // filename for data read
  int n; // number of data points
  int m; // slope
  int b; // y-intercept

  int rsqr; // correlation numerator
  int r; // correlation
} protocolIO;

// Collect runtime information
extern double lap;
double wallClock();
const char* mySide();

void linReg(void* args);
void load_data(protocolIO *io, int *x, int *y, int party);
void write_runtime(int n, double time, int party, const char* dest);
void check_mem(int *x, int *y, int party);
