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

#ifndef __SIGNET_BIT_MATRIX_H__
#define __SIGNET_BIT_MATRIX_H__

#include <math.h>

#include "global.h"

class BitMatrix
{
    public:
        BitMatrix(int h, int w);
        ~BitMatrix();

        /** Initialize the encoding matrix with all zero elements */
        inline void clear();

        /** Inizialize the encoding matrix with all 1s */
//         inline void SET_TABLE(uint8_t tab[MATRIX_NUM_OF_ROW][MATRIX_NUM_OF_COL]);

        /** Inizialize the encoding matrix with all elements equal to val */
//         inline void INITIALIZE_TABLE(uint8_t tab[MATRIX_NUM_OF_ROW][MATRIX_NUM_OF_COL], uint8_t val);

        /** Manipulate single bits of the encoding matrix */
        uint8_t testBitRC( uint16_t r, uint16_t c );
        void setBitRC( uint16_t r, uint16_t c );
        void clearBitRC( uint16_t r, uint16_t c );
        void setBitRC_V( uint16_t r, uint16_t c, uint8_t val );

        /** ROW SWAP */
        void swapRows( uint16_t r1, uint16_t r2 );
        void xorRows( uint16_t r1, uint16_t r2 );

        /** COL SWAP */
        void swapCols( uint16_t c1, uint16_t c2 );

        /** Debug functions */
        void printByte( uint8_t mem, uint8_t nl );
        void printMatrix();

    private:
        uint8_t *m_matrix;
        int m_h, m_w, m_wBytes;
};

#endif //__SIGNET_BIT_MATRIX_H__
