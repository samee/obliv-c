#include <stdio.h>
#include <stdlib.h>
#include <obliv.oh>


void genOblivFloat(OblivBit* dest, float x, int size) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    unsigned char* float_bytes = (unsigned char*) &x;
    int bit_number;
    int this_bit;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        bit_number = i % byte_size;
        this_bit = (*float_bytes >> bit_number) & 1;
        __obliv_c__assignBitKnown(dest+(i), this_bit);
        if ( bit_number == byte_size - 1 ) {
            float_bytes++;
        }
    }
}

void revOblivFloat(float* dest, OblivBit* bits, int size) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    int tmp = 0;
    unsigned char floatBytes[float_byte_size];
    memcpy(floatBytes, (unsigned char*) &tmp, float_byte_size);
    int j = 0;
    unsigned char currentByte = floatBytes[j];
    int bit_number;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        bit_number = i % byte_size;
        currentByte |= (bits[i].knownValue << bit_number);
        if (bit_number == byte_size - 1) {
            floatBytes[j] = currentByte;
            j++;
            currentByte = floatBytes[j];
        }
    }
    memcpy(dest, floatBytes, float_byte_size);
}

void printAsBinary(float x) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    unsigned char* float_bytes = (unsigned char*) &x;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        int bit_number = i % byte_size;
        int this_bit = (*float_bytes >> bit_number) & 1;
        printf("%i", this_bit); 
        if ( bit_number == byte_size - 1 ) {
            float_bytes++;
        }
    }
    printf("\n");
}

void printOblivBits(OblivBit* bits) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        printf("%i", bits[i].knownValue);
    }
    printf("\n");
}

int main() {
    OblivBit obliv_x[32];
    OblivBit obliv_y[32];
    OblivBit obliv_z[32];
    
    /*float x = 3.1415;
    float y = 1.1312;
    float a = 0;
    float b = 0;
    float c = 0;
    float z = 0;
    genOblivFloat(obliv_x, x, 32);
    genOblivFloat(obliv_y, y, 32);
    revOblivFloat(&a, obliv_x, 32);
    revOblivFloat(&b, obliv_y, 32);
    c = a + b;
    genOblivFloat(obliv_z, c, 32);
    revOblivFloat(&z, obliv_z, 32);
    printf("%f\n", z);*/
    
    float x = 345.678;
    printAsBinary(x);
    genOblivFloat(obliv_x, x, 32);
    printOblivBits(obliv_x);
    float x2;
    revOblivFloat(&x2, obliv_x, 32);
    printf("%f\n", x2);
    
    float y = 256.123;
    printAsBinary(y);
    genOblivFloat(obliv_y, y, 32);
    printOblivBits(obliv_y);
    float y2;
    revOblivFloat(&y2, obliv_y, 32);
    printf("%f\n", y2);
    
    float z = 0;

    __obliv_c__setPlainAddF(obliv_z, obliv_x, obliv_y, 32);
    revOblivFloat(&z, obliv_z, 32);
    printf("Res: %f\n", z);
    printAsBinary(z);
    printOblivBits(obliv_z);

    return 0;
}