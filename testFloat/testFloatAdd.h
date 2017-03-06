double lap;
int cp;

typedef struct {
  float v; // Value
  int party;
  float ores;
} protocolIO;

void floatAddi(void* args);
void load_data(protocolIO *io, float *x, float *y, int party);


void printAsBinary(float x);
// int ocCurrentParty2();
