#include <stdio.h>
#include <stdlib.h>
#include <obliv.oh>


void floatFeedOblivFloat(OblivBit* dest, int party, bool a) {
    int curparty = 1;
    dest->unknown=true;
    if(party==1) { if(curparty==1) dest->knownValue=a; }
    else if(party==2 && curparty == 1)  {
        // orecv(currentProto,2,&dest->knownValue,sizeof(bool));
        __obliv_c__assignBitKnown(dest, a);
    }
    else if(party==2 && curparty == 2) {
        // osend(currentProto,1,&a,sizeof(bool));
    }
    else fprintf(stderr,"Error: This is a 2 party protocol\n");
}

void floatProtoFeedOblivInputs(ProtocolDesc* pd,
    OblivInputs* spec, size_t count, int party)
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    int bit_number;
    int this_bit;
    while(count--){
        unsigned char* float_bytes = (unsigned char*) &(spec->src);
        for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
            bit_number = i % byte_size;
            this_bit = (*float_bytes >> bit_number) & 1;
            floatFeedOblivFloat((spec->dest)+i, party, this_bit);
            if ( bit_number == byte_size - 1 ) {
                float_bytes++;
            }
        }
        spec++;
    }
}

bool floatProtoRevealOblivBits(ProtocolDesc* pd,widest_t* dest,
        const OblivBit* src,size_t size,int party) {
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
        currentByte |= (src[i].knownValue << bit_number);
        if (bit_number == byte_size - 1) {
            floatBytes[j] = currentByte;
            j++;
            currentByte = floatBytes[j];
        }
    }
    float rv = 0;
    memcpy(&rv, floatBytes, float_byte_size);
    if(2==1)
    {
        if(party==0 || party==2) {
            // osend(pd,2,&rv,sizeof(rv));
        }
        if(party==2) return false;
        else { 
            *dest=rv; return true; 
        }
    } else { 
        if(party==0 || party==2) { 
            // orecv(pd,1,dest,sizeof(*dest)); 
            memcpy(dest, &rv, float_byte_size);
            return true; 
        }
        else return false;
    }
}

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
    __obliv_c__float obliv_x;
    __obliv_c__float obliv_y;
    __obliv_c__float obliv_z;
    
    float x = 345.678;
    printf("%f\n", x);
    printAsBinary(x);

    OblivBit obliv_xa[32];
    genOblivFloat(obliv_xa,x,32);
    printOblivBits(obliv_xa);

    
    OblivInputs spec_x;
    setupOblivFloat(&spec_x,&obliv_x,x);
    spec_x.src_f = x;
    printf("%f\n", spec_x.src_f);
    floatProtoFeedOblivInputs(NULL, &spec_x, 1, 2);
    printOblivBits(spec_x.dest);
    // float x2;
    // revOblivFloat(&x2, obliv_x, 32);
    // printf("%f\n", x2);
    
    float y = 256.123;
    printf("%f\n", y);
    printAsBinary(y);

    OblivInputs spec_y;
    setupOblivFloat(&spec_y,&obliv_y,y);
    spec_y.src_f = y;
    floatProtoFeedOblivInputs(NULL, &spec_y, 1, 2);
    // printOblivBits(obliv_y);
    // float y2;
    // revOblivFloat(&y2, obliv_y, 32);
    // printf("%f\n", y2);
    
    float z = 0;

    OblivInputs spec_z;
    setupOblivFloat(&spec_z,&obliv_z,z);
    floatProtoFeedOblivInputs(NULL, &spec_z, 1, 2);

    __obliv_c__setPlainAddF(spec_z.dest, spec_x.dest, spec_y.dest, 32);
    floatProtoRevealOblivBits(NULL, &z, spec_z.dest, 32, 0);
    printf("Res: %f\n", z);
    printAsBinary(z);
    // printOblivBits(obliv_z);

    return 0;
}