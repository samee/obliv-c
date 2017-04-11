double lap;
int cp;

typedef struct {
  float v; // Value
  float ores;
} protocolIO;

void floatOps(void* args);
void load_data(protocolIO *io, float *x, float *y, int party);

void printAsBinary(float x);
void printOblivBits(OblivBit* n);
