// Fixed point arithmetic
#define SCALE (1 << 16) // 2^16
#define DESCALE(x) x / SCALE // for use in multiplication
#define MAXN 100

typedef struct {
  char *src; // filename for data read
  int n; // number of data points
  int m; // slope
  int b; // y-intercept
  int r; // correlation
} protocolIO;

void linReg(void* args);
void load_data(protocolIO *io, int x[MAXN], int y[MAXN], int party);
