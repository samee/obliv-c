typedef struct {
  float v; // Value
  int party;
  float ores;
} protocolIO;

// Collect runtime information
extern double lap;
double wallClock();

void floatAddi(void* args);
void load_data(protocolIO *io, float *x, float *y, int party);
