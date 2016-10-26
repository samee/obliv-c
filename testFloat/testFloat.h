typedef struct {
  float v; // Value
  float ores;
} protocolIO;

// Collect runtime information
extern double lap;
double wallClock();
const char* mySide();

void floatAdd(void* args);
void load_data(protocolIO *io, float *x, float *y, int party);
void check_mem(float *x, float *y, int party);
