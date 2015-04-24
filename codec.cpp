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

#include "codec.h"

#include <math.h>
#include <stdlib.h>

enum {
    ACTION_CODER    =   0,
    ACTION_DECODER  =   1,
};

Codec :: Codec( int blockSize, int symSize, uint16_t type, bool verbose ,bool coder) : m_data(0), m_matrix(0)
{
    m_action = ACTION_CODER;
    m_blockSize = blockSize;
    m_symSize = symSize;
    m_type = type;
    m_verbose = verbose;
    m_rand = new Random( m_type );
    m_randDegree = new Random( m_type );
    m_randDegree->setSeed( 0x9D25 );
    m_randDegreeCount = 0;
    m_dist = new Distribution( blockSize );
    m_dist->setUniform();
	 m_coder = coder;
}

Codec :: ~Codec()
{
    free(m_data);
    delete m_rand;
    delete m_randDegree;
    delete m_dist;
    delete m_matrix;
}

void Codec :: setCoder()
{
    m_action = ACTION_CODER;
    free(m_data);
    m_data = (uint8_t *)malloc( sizeof(uint8_t)*m_symSize*m_blockSize );
}

void Codec :: setDecoder( int pckOverhead )
{
    m_action = ACTION_DECODER;
    m_pckOverhead = pckOverhead;
    free(m_data);
    m_data = (uint8_t *)malloc( sizeof(uint8_t)*m_symSize*(m_blockSize+m_pckOverhead) );
    delete m_matrix;
    m_matrix = new BitMatrix( m_blockSize+m_pckOverhead, m_blockSize );
}

void Codec :: setDistribution(char *distFile)
{
    m_dist->setDistribution(distFile);
}

/* Functions implementations */
void Codec :: startNewBlock( int seed )
{
    int s, i;
    // clear data
    for(s=0; s<m_blockSize; s++) {
        for(i=0; i<m_symSize; i++) {
            m_data[ s*m_symSize + i ] = 0;
        }
    }
    m_randDegree->setSeed( seed );
    m_seed = seed;
    m_pckIdx = 0;
    m_randDegreeCount = 0;
}

void Codec :: setToCoder( uint8_t * pck )
{
    int i;
    // copy data
    for(i=0; i<m_symSize; i++) {
        m_data[ m_pckIdx*m_symSize + i ] = pck[i];
//         printf("%X  ", m_data[ m_pckIdx*m_symSize + i ]);
    }
//     printf("\n");
    // update current pck index
    m_pckIdx = (m_pckIdx+1) % m_blockSize;
}

void Codec :: getCodedPck( uint8_t *pck, uint16_t id )
{
    uint16_t i, n, t=1;
    uint16_t symbolsToMix[m_blockSize];

    // 1. get num of symbols to mix
    m_randDegree->setSeed( m_seed );
    m_randDegreeCount = 0;
    while(m_randDegreeCount<id) {
        if(m_verbose) {
            printf("\e[34m.\e[0m");
        }
        t = m_randDegree->getNextStatus();
//		printf("degree:%x\n",t);
        m_randDegreeCount++;
    }
//  t = m_randDegree->getStatus();
    if(m_verbose) {
        printf("\e[36m %4X \e[0m", t);
    }
    n = m_dist->getDegree( t );
    if(m_verbose) {
        printf("deg: %2X\t", n);
    }

// 	2. set seed
//  t = m_randDegree->getStatus();
    if(m_verbose) {
        printf("\e[32m %4X \e[0m", t);
    }
    m_rand->setSeed( t );
	 if (m_coder )printf(" seed:%x", t);
// 	3. get syndols ID
    getSymbolsToMix( symbolsToMix, n );

// 	4. build output pck
    for(i=0; i<m_symSize; i++) {
        pck[i] = 0x00;
    }

    if(m_verbose) {
        printf("mix: \e[34m[ ");
    }
    for(t=0; t<m_blockSize; t++) {
        if(symbolsToMix[t] != 0) {
            if(m_verbose) {
                printf("%2X ", t);
            }
            for(i=0; i<m_symSize; i++) {
                pck[i] ^= m_data[ t*m_symSize + i ];
            }
        }
    }
    if(m_verbose) {
        printf("]\e[0m\t");
    }
}

void Codec :: setToDecoder( uint8_t *pck, uint16_t id )
{
    uint16_t i, n, t=1;
    uint16_t symbolsToMix[m_blockSize];

    // 1. get num of symbols to mix
    m_randDegree->setSeed( m_seed );
    m_randDegreeCount = 0;
    while(m_randDegreeCount<id) {
        t = m_randDegree->getNextStatus();
        if(m_verbose) {
            printf("\e[35m.\e[0m");
        }
        m_randDegreeCount++;
    }
    n = m_dist->getDegree( t/*m_randDegree->getStatus()*/ );

    // 2. set random seed
    m_rand->setSeed( t );

    // 3. get IDs of symbols to combine
    getSymbolsToMix( symbolsToMix, n );

    // 3.5 print symbols to mix
    if(m_verbose) {
        printf("\e[35m[ ");
        for(t=0; t<m_blockSize; t++) {
            if(symbolsToMix[t] != 0) {
                printf("%2X ", t);
            }
        }
        printf("]\e[0m\n");
    }

//   printf("\e[33m%i\e[0m\t", m_pckIdx);
    
	// 4. store encoded packet
    for(i=0; i<m_symSize; i++) {
        m_data[ m_pckIdx*m_symSize + i ] = pck[i];
    }

    // 5. Add entry to encoding matrix G
    for(t=0; t<m_blockSize; t++) {
        if(symbolsToMix[t] != 0) {
            m_matrix->setBitRC( m_pckIdx, t );
//          printf("\e[33m [%i, %i] \e[0m",m_pckIdx, t);
        }
    }

    m_pckIdx++;
//  printf("\e[32m%i %i %i \e[0m\n", m_pckIdx, m_blockSize, m_pckOverhead, m_blockSize+m_pckOverhead);
    if(m_pckIdx>=m_blockSize+m_pckOverhead) {
        m_pckIdx = m_blockSize;
//      printf("\e[31m%i\e[0m\n", m_pckIdx);
    }
}

// Given a linear system y = G x, where G is the encoded binary matrix
// at the receiver and y is the vector containing the corresponding 
// encoded packets, this function inverts the linear system using
// Gaussian Elimination
int Codec :: decode()
{
    int cIdx, rIdx, r;
    int n_blackCol = 0;
    bool found = false;

    if(m_verbose) {
        printf("Encoded matrix at receiver:\n");
        m_matrix->printMatrix();
    }

    // rIdx, cIdx, row and column indices
	
    rIdx = 0;
    
	// spans over all columns of the encoding matrix G
	for(cIdx=0; cIdx<m_blockSize; cIdx++) {
//         printf("\ncIdx: %i\t", cIdx);
        
		// look for the first row of G containing a 1 in the current column cIdx
        r		= rIdx;
        found	= false;
        while(!found && r<m_blockSize+m_pckOverhead) {
            if(m_matrix->testBitRC(r, cIdx)) {
                found = true;
            } else {
                r++;
            }
        }
        if(!found) {
            n_blackCol++;
			// printf("Pivot element not found for column %d\n", cIdx);
            continue;
        }
		
		// printf("Pivot element found for column %d in row %d\n", cIdx, r);

        // Swap row r and row rIdx
        if(rIdx != r) {
            swapRow(rIdx, r);
        }
		
        // Clean current column cIdx, i.e., remove all ones in rows different from r
        for(r=0; r<m_blockSize+m_pckOverhead; r++) {
            if(r==rIdx) continue;
            if(m_matrix->testBitRC(r, cIdx)) xorRow(rIdx, r);
        }
        rIdx++;
    }

    if(m_verbose) {
        printf("\nDecoded matrix at receiver:\n");
        m_matrix->printMatrix();
    }
	
    return n_blackCol;
}

void Codec :: printMatrix()
{
    m_matrix->printMatrix();
    printf("\n");
}

uint8_t * Codec :: getData()
{
    return m_data;
}

// n: degree of the encoded packet
// p: encoding vector (EV)
// getSymbolsToMix: picks n packets among the K in input 
// uniformly at random and obtains the corresponding EV
void Codec :: getSymbolsToMix( uint16_t *p, uint16_t n )
{
    int i, t;
    int	*temp;
    
	// m_blocksize is K, i.e., the number of input symbols
	temp = (int *)malloc(sizeof(int)*m_blockSize);
    
    for(i=0; i<m_blockSize; i++) {
		// p is a vector of size K containing the encoding vector 
		// (whose elements are either 1 or 0)
        p[i] = 0;
		
		// local vector used to speed up the random selection of pkts
		temp[i] = i;
    }
	
    i = n;

    // optimized random permutation
	// executes in exactly n cycles
    while(i != 0) {
        t = m_rand->getNextStatus() % (m_blockSize-n+i);
		p[temp[t]] = 1;
		i--;
		temp[t] = temp[m_blockSize-n+i];
    }

	// release memory
	free(temp);
}

// swapRow: considering the linear system y = G x, where 
// y: column vector of received (encoded) packets 
// G: encoding binary matrix
// swapRow, swaps the two rows r1 and r2 of G and the 
// corresponding elements in y
void Codec :: swapRow( int r1, int r2 )
{
    int c;
    // swap matrix
    m_matrix->swapRows( r1, r2 );
    // swap data
    for(c=0; c<m_symSize; c++) { 
        //r1->t   r2->r1    t->r2
        #if 0
        t = m_data[r1*m_symSize + c];
        m_data[r1*m_symSize + c] = m_data[r2*m_symSize + c];
        m_data[r2*m_symSize + c] = t;
        #endif
        m_data[r1*m_symSize + c] ^= m_data[r2*m_symSize + c];
        m_data[r2*m_symSize + c] ^= m_data[r1*m_symSize + c];
        m_data[r1*m_symSize + c] ^= m_data[r2*m_symSize + c];
    }
}

// Given y = G x, xorRow, XORs two rows of a binary 
// matrix G and the corresponding elements of y
void Codec :: xorRow( int r1, int r2 )
{
    int c;
    // xor matrix
    m_matrix->xorRows( r1, r2 );
    // xor data
    for(c=0; c<m_symSize; c++) {
        m_data[r2*m_symSize + c] ^= m_data[r1*m_symSize + c];
    }
}

// For debug purposes
void Codec :: printData( )
{
    int r, c;
    for(r=0; r<m_blockSize+m_pckOverhead; r++) {
        printf("%2i [ ", r);
        for(c=0; c<m_symSize; c++) {
            printf("%2X ", m_data[r*m_symSize+c]);
        }
        printf("]\n");
    }
}
