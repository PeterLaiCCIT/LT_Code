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

#include <stdlib.h>

#include "bit_matrix.h"

/**
 *  The following macros have as a first parameter the byte (called "mem", 8bits)
 *  that has to be modified. The second parameter is the bit index (idx)
 *  of the position (in 0,1,...,7) that we want to modify in this byte
 *
 *  NOTE:
 *  For the sake of improved performance we do not control the correctness of 
 *  the range for input parameters. In what follows we detail the correct range
 *  for both variables:
 *
 *  idx MUST be in {0,1,...,7}
 *  val MUST be in {0, 255}
 */

// #define SET_BYTE(mem)           ( mem = 0xFF )
// #define CLEAR_BYTE(mem)         ( mem = 0x00 )
// 
#define SET_BIT(mem, idx)           ( (mem) |= 1<<(idx) )
#define CLEAR_BIT(mem, idx)         ( (mem) &= ~(1<<(idx)) )
// #define SET_BIT_VAL(mem, idx, val)  ( (val) ? SET_BIT((mem), (idx)) : CLEAR_BIT((mem), (idx)) )
// 
#define TEST_BIT(mem, idx)      ( (mem) & 1<<(idx) )
// 
// #define TOGGLE_BIT(mem, idx)    ( TEST_BIT((mem), (idx)) ? CLEAR_BIT((mem), (idx)) : SET_BIT((mem), (idx)) )


BitMatrix :: BitMatrix( int h, int w )
{
    m_h = h;
    m_w = w;
    m_wBytes = (int)floor( w/8 );
    if(w%8!=0) m_wBytes++;
    m_matrix = (uint8_t *)malloc( m_h*m_wBytes );
    clear();
}

BitMatrix :: ~BitMatrix()
{
    free( m_matrix );
    m_matrix = 0;
}

/* Initialize the encoding matrix G with all zeros */
void BitMatrix :: clear()
{
    int r, c;
    for(r=0; r<m_h; r++) {
        for(c=0; c<m_wBytes; c++) {
            m_matrix[ r*m_wBytes + c ] = 0x00;
        }
    }
};

/* Initialize encoding matrix with all ones */
// inline void SET_TABLE(uint8_t tab[MATRIX_NUM_OF_ROW][MATRIX_NUM_OF_COL])
// {
//     int _r, _c;
//     for(_r=0; _r<MATRIX_NUM_OF_ROW; _r++) {
//         for(_c=0; _c<MATRIX_NUM_OF_COL; _c++) {
//             tab[_r][_c] = 0xFF;
//         }
//     }
// };

/* Inizialize encoding matrix with all elements equal to val */
// inline void INITIALIZE_TABLE(uint8_t tab[MATRIX_NUM_OF_ROW][MATRIX_NUM_OF_COL], uint8_t val)
// {
//     if(val) {
//         SET_TABLE( tab );
//     } else {
//         CLEAR_TABLE( tab );
//     }
// };
// 

uint8_t BitMatrix :: testBitRC( uint16_t r, uint16_t c )
{
    return TEST_BIT( m_matrix[ r*m_wBytes + (c>>3) ], c&0x7 );
}

void BitMatrix :: setBitRC( uint16_t r, uint16_t c )
{
    SET_BIT( m_matrix[r*m_wBytes + (c>>3)], c&0x7 );
}

void BitMatrix :: clearBitRC( uint16_t r, uint16_t c )
{
    CLEAR_BIT(m_matrix[r*m_wBytes + (c>>3)], c&0x7);
}

void BitMatrix :: setBitRC_V( uint16_t r, uint16_t c, uint8_t val )
{
    if(val) {
        setBitRC(r, c);
    } else {
        clearBitRC(r, c);
    }
}

/** ROW SWAP */
void BitMatrix :: swapRows( uint16_t r1, uint16_t r2 )
{
//     uint8_t t;
    int c;
    for(c=0; c<m_wBytes; c++) { 
        //r1->t   r2->r1    t->r2
        #if 0
        t = m_matrix[r1*m_wBytes + c];
        m_matrix[r1*m_wBytes + c] = m_matrix[r2*m_wBytes + c];
        m_matrix[r2*m_wBytes + c] = t;
        #endif
        m_matrix[r1*m_wBytes + c] ^= m_matrix[r2*m_wBytes + c];
        m_matrix[r2*m_wBytes + c] ^= m_matrix[r1*m_wBytes + c];
        m_matrix[r1*m_wBytes + c] ^= m_matrix[r2*m_wBytes + c];
    }
}

void BitMatrix :: xorRows( uint16_t r1, uint16_t r2 )
{
    int c;
    for(c=0; c<m_wBytes; c++) {
        m_matrix[r2*m_wBytes + c] ^= m_matrix[r1*m_wBytes + c];
    }
}

/** COL SWAP */
void BitMatrix :: swapCols( uint16_t c1, uint16_t c2 )
{
    uint8_t t;
    int r;
    for(r=0; r<m_h; r++) {
        //c1->t   c2->c1    t->c2
        t = testBitRC(r, c1);
        setBitRC_V(r, c1, testBitRC(r, c2));
        setBitRC_V(r, c2, t);
    }
}

/** Debug functions */
void BitMatrix :: printByte( uint8_t mem, uint8_t nl )
{
    int b;
	
//     printf("[ ");
    for(b=0; b<8; b++) {
        if(TEST_BIT(mem, b)) {
            printf("1 ");
        } else {
            printf("0 ");
        }
    }
//     printf("]");
    if(nl)
        printf("\n");
}

void BitMatrix :: printMatrix()
{
    int r, c;
    for(r=0; r<m_h; r++) {
        printf("%3i ",r);
        for(c=0; c<m_wBytes; c++) {
            printByte( m_matrix[r*m_wBytes + c], 0 );
        }
        printf("\n");
    }
}
