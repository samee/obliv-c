typedef struct {
  int *x; // independent points
  int *y; // dependent points
  int n; // number of data points
} DataSet;

typedef struct {
  int m; // slope
  int b; // y-intercept
  int r; // correlation constant
} LinRegResult;

// forward declarations
DataSet load_data(FILE *inputFile);
LinRegResult linear_regression(DataSet theData);
int sqr(int x);
int dotProd(int *a, int *b, int n);
int sum(int *a, int n);
void clean(DataSet data);
