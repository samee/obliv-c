typedef struct {
  float v; // Value
  float ores;
} protocolIO;

// Collect runtime information
extern double lap;
double wallClock();
const char* mySide();

void floatAddi(void* args);
void load_data(protocolIO *io, int *x, int *y, int party);
