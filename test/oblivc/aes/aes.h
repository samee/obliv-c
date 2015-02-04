#pragma once
void goaes(void*arg);
void aestest(void*arg);

typedef struct {
  const char *testkey, *testplain;
  char testcipher[17];
} protocolIO;
