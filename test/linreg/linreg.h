
// initial amount for input data:
#define ALLOC 128 
// doubled when needed in load_data()

double lap;
int cp;

typedef struct {
    char *src; // filename for data read
    int n; // number of data points
    float m; // slope
    float b; // y-intercept
    int rsqr; // correlation numerator, cov^2 / variances
    int r; // correlation
} protocolIO;

void linReg(void* args);
void load_data(protocolIO *io, float** x, float** y, int party);
void check_mem(float* x, float* y, int party);
