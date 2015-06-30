// Fixed point arithmetic
#define SCALE (int) (1 << 16) // 2^16
#define DESCALE(x) (double) x / SCALE
#define MAXN 100

typedef struct {
  char *src; // filename for data read
  int x[MAXN];
  int y[MAXN];
  int n;
  int m; // slope
  int b; // y-intercept
  int r; // correlation
} protocolIO;

void linReg(void* args);
void load_data(protocolIO *io, int x[MAXN], int y[MAXN], int party);
