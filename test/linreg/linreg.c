#include <stdio.h>
#include <stdlib.h>
#include <obliv.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../oblivc/common/util.h"
#include "dbg.h"

#include "linreg.h"


void check_mem(float* x, float* y, int party) 
{
    if (party == 1) {
        if (x == NULL) {
            log_err("Memory allocation failed\n");
            clean_errno();
            exit(1);
        }
    }
    if (party == 2) {
        if (y == NULL) {
            log_err("Memory allocation failed\n");
            clean_errno();
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) 
{
    printf("Linear Regression\n");
    printf("=================\n\n");
    // Check args
    if (argc == 4) {
        // Initialize protocols and obtain connection information
        const char *remote_host = strtok(argv[1], ":");
        const char *port = strtok(NULL, ":");
        ProtocolDesc pd;
        protocolIO io;

        // Make connection between two shells
        // Modified ocTestUtilTcpOrDie() function from obliv-c/test/oblivc/common/util.c
        log_info("Connecting to %s on port %s ...\n", remote_host, port);
        if(argv[2][0] == '1') {
            if(protocolAcceptTcp2P(&pd,port)!=0) {
                log_err("TCP accept from %s failed\n", remote_host);
                exit(1);
            }
        } else {
            if(protocolConnectTcp2P(&pd,remote_host,port)!=0) {
                log_err("TCP connect to %s failed\n", remote_host);
                exit(1);
            }
        }

        // Final initializations before entering protocol
        cp = (argv[2][0]=='1'? 1 : 2);
        setCurrentParty(&pd, cp); // only checks for a '1'
        io.src = argv[3]; // filename
        lap = wallClock();

        // Execute Yao's protocol and cleanup
        execYaoProtocol(&pd, linReg, &io);
        cleanupProtocol(&pd);
        double runtime = wallClock() - lap; // stop clock here 

        // Print results and store runtime data
        log_info("Total time: %lf seconds\n", runtime);

        // int gates = yaoGateCount();
        // log_info("Yao Gate Count: %u\n", gates);

        printf("\n");
        log_info("Slope   \tm = %15.6e\n", io.m); // print slope
        log_info("y-intercept\tb = %15.6e\n", io.b); // print y-intercept
        log_info("Correlation\tr = %15.6e\n", sqrt(io.rsqr)); // print correlation
    } else {
        log_info("Usage: %s <hostname:port> <1|2> <filename>\n" 
                 "\tHostname usage:\n" 
                 "\tlocal -> 'localhost' remote -> IP address or DNS name\n", argv[0]);
    }
    return 0;
}

void load_data(protocolIO *io, float** x, float** y, int party) 
{
    FILE *inputFile = fopen(io->src, "r");
    if (inputFile == NULL) {
        log_err("File '%s' not found\n", io->src);
        clean_errno();
        exit(1); // causes TCP error for non-null party
    }
  
    io->n = 0;
    int memsize = ALLOC;
    double a;
    while (!feof(inputFile)) {
        int dataPoints = fscanf(inputFile, "%lf", &a);
        if (dataPoints != 1) {
            if (dataPoints < 0) {
                if (feof(inputFile)) {
	                break;
                } else {
                    log_err("Input from '%s' does not match file format. Check input file.\n\t" 
		                "File format exception found at Line %d or %d in file.\n", 
		                io->src, io->n, io->n + 1); // prints to both parties if filename is same
	                clean_errno();
	                exit(1);
                }
            } else {
	            log_err("Input from '%s' does not match file format. Check input file.\n\t" 
		                "File format exception found at Line %d or %d in file.\n", 
		                io->src, io->n, io->n + 1); // prints to both parties if filename is same
	            clean_errno();
	            exit(1);
            }
        }
        io->n += 1;
        if (io->n > memsize) {
            memsize *= 2;
            *x = realloc(*x, sizeof(float) * memsize);
            *y = realloc(*y, sizeof(float) * memsize);
            check_mem(*x, *y, party);
        }
        float a_float = (float) a;
        if (party == 1) {
            *(*x + io->n - 1) =  a_float; // messy, but needed for dereferencing 
        } else if (party == 2) {
            *(*y + io->n - 1) =  a_float;
        }
    }

    log_info("Loading %d data points ...\n", io->n);
    fclose(inputFile);
}
