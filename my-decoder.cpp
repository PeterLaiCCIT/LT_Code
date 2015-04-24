/** **************************************************************************
**
** This file is part of the Signet Network Programming System.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation.
**
**
**  @author: Alberto Scarpa <alscarpa@dei.unipd.it>
**
**  @co-author: Michele Rossi <rossi@dei.unipd.it>
**
** Last modified: April 2010
**
****************************************************************************/

#include <getopt.h>
#include <string.h>
#include <time.h>                      		// define time()
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

// Include encoding/decoding libraries
#include "codec.h"

// Default values
#define STRLEN			2000
#define MIN(x,y) 		(x>=y ? y : x)
#define MAX(x,y) 		(x>=y ? x : y)

typedef struct timeval TIME;

void printUsage( char *name ) {
	printf("USAGE\n\n");
	printf("%s K bytes-per-packet distFile simID verbose #seeds\n\n", name);
	printf("K: # of packets in input file\n");
	printf("SymbSize: size in bytes of each packet\n");
	printf("distFile: encoding distribution\n");
	printf("simID: simulation ID\n");
	printf("#seeds: number of random seeds to simulate\n\n");
	
	exit(1);
}

uint8_t intrand(double M) {
	double	r;
	uint8_t res;
	
	r = drand48();		// random in [0,1)
	res = (int)(r*M);	// integer in [0,M)
	
	return (res);
}

int main( int argc, char **argv ) {

// Global variables
	int verbose			= false;
	int verboseEncoder	= false;
	
	long seed, seedStart, seedStop, tottime=0;
	TIME t1, t2;

	int rrank, blockSize, symSize, i, H;

	int RandomGenType;
	
	char distFile[STRLEN];		// file containing the encoding distribution to use

	FILE	*fout;
	char	foutName[STRLEN];

// Types for random number generators defined in "distribution.h"
	enum {
            LINUX_TYPE  =   0,
            AL_TYPE     =   1,
            MSP_TYPE    =   2,
            TINYOS_TYPE =   3,
			MLCG_TYPE	=   4,
        };

// ---------------------------------------------
// Initialize variables
// ---------------------------------------------

	// number of redundancy packets
	H = 0;

// argv[1]=K argv[2]=symb_size[bytes] argv[3]=distFile argv[4]=simID argv[5]=verbose argv[6]=#Seeds

	if (argc<7) printUsage( argv[0] );
	
	blockSize		= atoi(argv[1]); // this is K
	symSize			= atoi(argv[2]);

	// file containing the encoding distribution to use
	strcpy(distFile, argv[3]);

	// output file where the results will be saved
	strcpy(foutName, argv[4]);

	RandomGenType	= MSP_TYPE;		// Set random generator to use 
									// LINUX_TYPE -> drand48()
									// MSP_TYPE -> linear congruential

	verbose		= atoi(argv[5]);
	seedStop	= atoi(argv[6]);		// # of seeds
	
	// Allocate structure for the fountain based encoder
	Codec *coder   = new Codec( blockSize, symSize, RandomGenType, verboseEncoder, false);	
	// Initialize encoder structures for the given distribution file
	coder->setDistribution(distFile);

	// Allocate structures for fountain based decoders at all R receivers
	Codec *decoder;
	decoder = (Codec *)malloc(sizeof(Codec));

	// Allocate structures for fountain based decoder at the receiver
	decoder = new Codec( blockSize, symSize, RandomGenType, verboseEncoder, false);

	// Initialize decoder structures for the given encoding distribution
 	decoder->setDistribution(distFile);
	
	// Define data to Encode -> Transmit and, eventually, -> Decode
	// i.e., K packets of size symSize bytes each
	uint8_t **data;
	data =  (uint8_t **)malloc(sizeof(uint8_t *)*blockSize);
	for (i=0; i<blockSize; i++) data[i] = (uint8_t *)malloc(sizeof(uint8_t)*symSize);
	
	// Initialize structure to store encoded packets
	// i.e., K packets of size symSize bytes each
	uint8_t **encodedPkts;
	encodedPkts = (uint8_t **)malloc(sizeof(uint8_t *)*(blockSize+H));
	for (i=0; i<(blockSize+H); i++) encodedPkts[i] = (uint8_t *)malloc(sizeof(uint8_t)*symSize);

// ---------------------------------------------
// Main simulation cycle
// ---------------------------------------------
	seedStart = 1000;

	// Iterate for different seeds
	for (seed=seedStart; seed<=(seedStart+seedStop); seed++) {

//		if (seed%200==0) { 
//			printf("%2.1f/100", ((double)seed/(double)seedStop)*100.);
//			printf("\n Seed=%ld\n", seed);
//		}

		if (verbose) printf("\n [*]Seed=%ld\n", seed);

		// Initialize data structure, filling packets with random binary numbers
		for(int j=0; j<blockSize; j++) {
			for(i=0; i<symSize; i++) { 
					data[j][i] = intrand(256.0); 
//					printf("%d\t", data[j][i]);
			}
		}

		// Initialize encoder Structures
		coder->setCoder();

		// Initialize encoder structures with current seed
		coder->startNewBlock( seed );
									
		// Send input file to Encoder (blocksize packets)
        for(i=0; i<blockSize; i++) coder->setToCoder( &data[i][0] );

		// Prepare Decoder for RX max. redundancy of H packets (below H=0)
		decoder->setDecoder( H );
				
		// Set decoder @ receiver for the RX of a new block using the specified seed
        decoder->startNewBlock( seed );

		// Actual encoding: based on the blocksize packets in the input file 
		// the encoder generates an encoded pkt (xoring thos in input according to degree distr.)
		for(i=1; i<=(blockSize+H); i++) coder->getCodedPck( encodedPkts[i-1], i );

		// putting gettimeofday here -> allows to capture the time
		// taken to go from seeds -> decoding matrix
		gettimeofday(&t1, NULL);
		
		// receive the packet and pass it to the decoder (error prob. is zero here)
		// and from the seed in the packet -> fill the corresponding row of the decoding matrix
		// i.e., retrieve the corresponding encoding vector
		for(i=1; i<=(blockSize+H); i++) decoder->setToDecoder( encodedPkts[i-1], i );

		// decode and retrieve original packets at the receiver
		// rrank is the residual rank of the RX decoding matrix
		// with "gettimeofday" we measure the decoding time in usec
		rrank = decoder->decode();
		gettimeofday(&t2, NULL); 		

		tottime += (t2.tv_sec - t1.tv_sec)*1e6;
		tottime += t2.tv_usec - t1.tv_usec;

// printf("%d usec\t", t2.tv_usec - t1.tv_usec);

	}

	fout = fopen(foutName, "a");
	fprintf(fout, "%d \t %g\n", blockSize, (double)tottime/(double)(seedStop+1));
	printf("\n Decoding time (K=%d) = %g usec\n", blockSize, (double)tottime/(double)(seedStop+1));
	fclose(fout);

	return 1;		// Program successfully terminated
}
