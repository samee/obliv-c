typedef struct {
  float v; // Value
  float ores;
} protocolIO;

// Collect runtime information
extern double lap;
double wallClock();
const char* mySide();

void floatAddi(void* args);
void load_data(protocolIO *io, float *x, float *y, int party);
