/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and   contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2010, SAMSUNG ELECTRONICS CO., LTD. and BRITISH BROADCASTING CORPORATION
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within the Joint Collaborative Team on Video Coding and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of SAMSUNG ELECTRONICS CO., LTD. nor the name of the BRITISH BROADCASTING CORPORATION
      may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * ====================================================================================================================
*/

/** \file     TComTrQuant.cpp
    \brief    transform and quantization class
*/

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "TComTrQuant.h"
#include "TComPic.h"
#include "ContextTables.h"

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define RDOQ_CHROMA                 1           ///< use of RDOQ in chroma

#define DQ_BITS                     6
#define Q_BITS_8                    16
#define SIGN_BITS                   1

// ====================================================================================================================
// Tables
// ====================================================================================================================

// RDOQ parameter
Int entropyBits[128]=
{
  895,    943,    994,   1048,   1105,   1165,   1228,   1294,
  1364,   1439,   1517,   1599,   1686,   1778,   1875,   1978,
  2086,   2200,   2321,   2448,   2583,   2725,   2876,   3034,
  3202,   3380,   3568,   3767,   3977,   4199,   4435,   4684,
  4948,   5228,   5525,   5840,   6173,   6527,   6903,   7303,
  7727,   8178,   8658,   9169,   9714,  10294,  10914,  11575,
  12282,  13038,  13849,  14717,  15650,  16653,  17734,  18899,
  20159,  21523,  23005,  24617,  26378,  28306,  30426,  32768,
  32768,  35232,  37696,  40159,  42623,  45087,  47551,  50015,
  52479,  54942,  57406,  59870,  62334,  64798,  67262,  69725,
  72189,  74653,  77117,  79581,  82044,  84508,  86972,  89436,
  91900,  94363,  96827,  99291, 101755, 104219, 106683, 109146,
  111610, 114074, 116538, 119002, 121465, 123929, 126393, 128857,
  131321, 133785, 136248, 138712, 141176, 143640, 146104, 148568,
  151031, 153495, 155959, 158423, 160887, 163351, 165814, 168278,
  170742, 173207, 175669, 178134, 180598, 183061, 185525, 187989
};

static const Int estErr4x4[6][4][4]=
{
  {
    {25600, 27040, 25600, 27040},
    {27040, 25600, 27040, 25600},
    {25600, 27040, 25600, 27040},
    {27040, 25600, 27040, 25600}
  },
  {
    {30976, 31360, 30976, 31360},
    {31360, 32400, 31360, 32400},
    {30976, 31360, 30976, 31360},
    {31360, 32400, 31360, 32400}
  },
  {
    {43264, 40960, 43264, 40960},
    {40960, 40000, 40960, 40000},
    {43264, 40960, 43264, 40960},
    {40960, 40000, 40960, 40000}
  },
  {
    {50176, 51840, 50176, 51840},
    {51840, 52900, 51840, 52900},
    {50176, 51840, 50176, 51840},
    {51840, 52900, 51840, 52900}
  },
  {
    {65536, 64000, 65536, 64000},
    {64000, 62500, 64000, 62500},
    {65536, 64000, 65536, 64000},
    {64000, 62500, 64000, 62500}
  },
  {
    {82944, 84640, 82944, 84640},
    {84640, 84100, 84640, 84100},
    {82944, 84640, 82944, 84640},
    {84640, 84100, 84640, 84100}
  }
};

static const Int estErr8x8[6][8][8]={
  {
    {6553600, 6677056, 6400000, 6677056, 6553600, 6677056, 6400000, 6677056},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201},
    {6400000, 6658560, 6553600, 6658560, 6400000, 6658560, 6553600, 6658560},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201},
    {6553600, 6677056, 6400000, 6677056, 6553600, 6677056, 6400000, 6677056},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201},
    {6400000, 6658560, 6553600, 6658560, 6400000, 6658560, 6553600, 6658560},
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201}
  },
  {
    {7929856, 8156736, 8028160, 8156736, 7929856, 8156736, 8028160, 8156736},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770},
    {8028160, 7814560, 7840000, 7814560, 8028160, 7814560, 7840000, 7814560},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770},
    {7929856, 8156736, 8028160, 8156736, 7929856, 8156736, 8028160, 8156736},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770},
    {8028160, 7814560, 7840000, 7814560, 8028160, 7814560, 7840000, 7814560},
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770}
  },
  {
    {11075584, 10653696, 11151360, 10653696, 11075584, 10653696, 11151360, 10653696},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652},
    {11151360, 11109160, 11289600, 11109160, 11151360, 11109160, 11289600, 11109160},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652},
    {11075584, 10653696, 11151360, 10653696, 11075584, 10653696, 11151360, 10653696},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652},
    {11151360, 11109160, 11289600, 11109160, 11151360, 11109160, 11289600, 11109160},
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652}
  },
  {
    {12845056, 12503296, 12544000, 12503296, 12845056, 12503296, 12544000, 12503296},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156},
    {12544000, 12588840, 12960000, 12588840, 12544000, 12588840, 12960000, 12588840},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156},
    {12845056, 12503296, 12544000, 12503296, 12845056, 12503296, 12544000, 12503296},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156},
    {12544000, 12588840, 12960000, 12588840, 12544000, 12588840, 12960000, 12588840},
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156}
  },
  {
    {16777216, 16646400, 16384000, 16646400, 16777216, 16646400, 16384000, 16646400},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116},
    {16384000, 16692640, 16646400, 16692640, 16384000, 16692640, 16646400, 16692640},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116},
    {16777216, 16646400, 16384000, 16646400, 16777216, 16646400, 16384000, 16646400},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116},
    {16384000, 16692640, 16646400, 16692640, 16384000, 16692640, 16646400, 16692640},
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116}
  },
  {
    {21233664, 21381376, 21667840, 21381376, 21233664, 21381376, 21667840, 21381376},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376},
    {21667840, 21374440, 21529600, 21374440, 21667840, 21374440, 21529600, 21374440},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376},
    {21233664, 21381376, 21667840, 21381376, 21233664, 21381376, 21667840, 21381376},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376},
    {21667840, 21374440, 21529600, 21374440, 21667840, 21374440, 21529600, 21374440},
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376}
  }
};

static const Int estErr16x16[6] = { 25329, 30580, 42563, 49296, 64244, 82293 };
static const Int estErr32x32[6] = { 25351, 30674, 42843, 49687, 64898, 82136 };

// ====================================================================================================================
// Qp class member functions
// ====================================================================================================================

QpParam::QpParam()
{
}

Void QpParam::initOffsetParam( Int iStartQP, Int iEndQP )
{
  Int iDefaultOffset;
  Int iDefaultOffset_LTR;
  
  Int iPer;
  
  for (Int iQP = iStartQP; iQP <= iEndQP; iQP++)
  {
    for (UInt uiSliceType = 0; uiSliceType < 3; uiSliceType++)
    {
      Int k =  (iQP + 6*g_uiBitIncrement)/6;
      
      Bool bLowPass = (uiSliceType == 0);
      iDefaultOffset = (bLowPass? 10922 : 5462);
      
      bLowPass = (uiSliceType == 0);
      iDefaultOffset_LTR = (bLowPass? 170 : 86);
      
      iPer = QP_BITS + k - QOFFSET_BITS;
      m_aiAdd2x2[iQP][uiSliceType] = iDefaultOffset << iPer;
      m_aiAdd4x4[iQP][uiSliceType] = iDefaultOffset << iPer;
      
      iPer = QP_BITS + 1 + k - QOFFSET_BITS;
      m_aiAdd8x8[iQP][uiSliceType] = iDefaultOffset << iPer;
      
      iPer = ECore16Shift + k - QOFFSET_BITS_LTR;
      m_aiAdd16x16[iQP][uiSliceType] = iDefaultOffset_LTR << iPer;
      
      iPer = ECore32Shift + k - QOFFSET_BITS_LTR;
      m_aiAdd32x32[iQP][uiSliceType] = iDefaultOffset_LTR << iPer;
    }
  }
}

// ====================================================================================================================
// TComTrQuant class member functions
// ====================================================================================================================

TComTrQuant::TComTrQuant()
{
  m_cQP.clear();
  
  // allocate temporary buffers
  m_plTempCoeff  = new Long[ MAX_CU_SIZE*MAX_CU_SIZE ];
  
  // allocate bit estimation class  (for RDOQ)
  m_pcEstBitsSbac = new estBitsSbacStruct;
}

TComTrQuant::~TComTrQuant()
{
  // delete temporary buffers
  if ( m_plTempCoeff )
  {
    delete [] m_plTempCoeff;
    m_plTempCoeff = NULL;
  }
  
  // delete bit estimation class
  if ( m_pcEstBitsSbac ) delete m_pcEstBitsSbac;
}

/// Including Chroma QP Parameter setting
Void TComTrQuant::setQPforQuant( Int iQP, Bool bLowpass, SliceType eSliceType, TextType eTxtType)
{
  iQP = Max( Min( iQP, 51 ), 0 );
  
  if(eTxtType != TEXT_LUMA) //Chroma
  {
    iQP  = g_aucChromaScale[ iQP ];
  }
  
  m_cQP.setQpParam( iQP, bLowpass, eSliceType, m_bEnc );
}


Void TComTrQuant::xT32( Pel* pSrc, UInt uiStride, Long* pDes )
{
  Int x, y;
  Long aaiTemp[32][32];
  
  Long A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31;
  Long B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B20, B21, B22, B23, B24, B25, B26, B27;
  Long C0, C1, C2, C3, C4, C5, C6, C7, C10, C11, C12, C13, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31;
  Long D0, D1, D2, D3, D5, D6, D8, D9, D10, D11, D12, D13, D14, D15, D18, D19, D20, D21, D26, D27, D28, D29;
  Long E4, E5, E6, E7, E9, E10, E13, E14, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E31;
  Long F8, F9, F10, F11, F12, F13, F14, F15, F17, F18, F21, F22, F25, F26, F29, F30;
  Long G16, G17, G18, G19, G20, G21, G22, G23, G24, G25, G26, G27, G28, G29, G30, G31;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift32x32-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  //--Butterfly
  for( y=0 ; y<32 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    A0  = (pSrc[0] + pSrc[31])<<uiBitDepthIncrease;
    A31 = (pSrc[0] - pSrc[31])<<uiBitDepthIncrease;
    A1  = (pSrc[1] + pSrc[30])<<uiBitDepthIncrease;
    A30 = (pSrc[1] - pSrc[30])<<uiBitDepthIncrease;
    A2  = (pSrc[2] + pSrc[29])<<uiBitDepthIncrease;
    A29 = (pSrc[2] - pSrc[29])<<uiBitDepthIncrease;
    A3  = (pSrc[3] + pSrc[28])<<uiBitDepthIncrease;
    A28 = (pSrc[3] - pSrc[28])<<uiBitDepthIncrease;
    A4  = (pSrc[4] + pSrc[27])<<uiBitDepthIncrease;
    A27 = (pSrc[4] - pSrc[27])<<uiBitDepthIncrease;
    A5  = (pSrc[5] + pSrc[26])<<uiBitDepthIncrease;
    A26 = (pSrc[5] - pSrc[26])<<uiBitDepthIncrease;
    A6  = (pSrc[6] + pSrc[25])<<uiBitDepthIncrease;
    A25 = (pSrc[6] - pSrc[25])<<uiBitDepthIncrease;
    A7  = (pSrc[7] + pSrc[24])<<uiBitDepthIncrease;
    A24 = (pSrc[7] - pSrc[24])<<uiBitDepthIncrease;
    A8  = (pSrc[8] + pSrc[23])<<uiBitDepthIncrease;
    A23 = (pSrc[8] - pSrc[23])<<uiBitDepthIncrease;
    A9  = (pSrc[9] + pSrc[22])<<uiBitDepthIncrease;
    A22 = (pSrc[9] - pSrc[22])<<uiBitDepthIncrease;
    A10 = (pSrc[10] + pSrc[21])<<uiBitDepthIncrease;
    A21 = (pSrc[10] - pSrc[21])<<uiBitDepthIncrease;
    A11 = (pSrc[11] + pSrc[20])<<uiBitDepthIncrease;
    A20 = (pSrc[11] - pSrc[20])<<uiBitDepthIncrease;
    A12 = (pSrc[12] + pSrc[19])<<uiBitDepthIncrease;
    A19 = (pSrc[12] - pSrc[19])<<uiBitDepthIncrease;
    A13 = (pSrc[13] + pSrc[18])<<uiBitDepthIncrease;
    A18 = (pSrc[13] - pSrc[18])<<uiBitDepthIncrease;
    A14 = (pSrc[14] + pSrc[17])<<uiBitDepthIncrease;
    A17 = (pSrc[14] - pSrc[17])<<uiBitDepthIncrease;
    A15 = (pSrc[15] + pSrc[16])<<uiBitDepthIncrease;
    A16 = (pSrc[15] - pSrc[16])<<uiBitDepthIncrease;
#else
    A0 = pSrc[0] + pSrc[31];
    A31 = pSrc[0] - pSrc[31];
    A1 = pSrc[1] + pSrc[30];
    A30 = pSrc[1] - pSrc[30];
    A2 = pSrc[2] + pSrc[29];
    A29 = pSrc[2] - pSrc[29];
    A3 = pSrc[3] + pSrc[28];
    A28 = pSrc[3] - pSrc[28];
    A4 = pSrc[4] + pSrc[27];
    A27 = pSrc[4] - pSrc[27];
    A5 = pSrc[5] + pSrc[26];
    A26 = pSrc[5] - pSrc[26];
    A6 = pSrc[6] + pSrc[25];
    A25 = pSrc[6] - pSrc[25];
    A7 = pSrc[7] + pSrc[24];
    A24 = pSrc[7] - pSrc[24];
    A8 = pSrc[8] + pSrc[23];
    A23 = pSrc[8] - pSrc[23];
    A9 = pSrc[9] + pSrc[22];
    A22 = pSrc[9] - pSrc[22];
    A10 = pSrc[10] + pSrc[21];
    A21 = pSrc[10] - pSrc[21];
    A11 = pSrc[11] + pSrc[20];
    A20 = pSrc[11] - pSrc[20];
    A12 = pSrc[12] + pSrc[19];
    A19 = pSrc[12] - pSrc[19];
    A13 = pSrc[13] + pSrc[18];
    A18 = pSrc[13] - pSrc[18];
    A14 = pSrc[14] + pSrc[17];
    A17 = pSrc[14] - pSrc[17];
    A15 = pSrc[15] + pSrc[16];
    A16 = pSrc[15] - pSrc[16];
#endif
    B0 = A0 + A15;
    B15 = A0 - A15;
    B1 = A1 + A14;
    B14 = A1 - A14;
    B2 = A2 + A13;
    B13 = A2 - A13;
    B3 = A3 + A12;
    B12 = A3 - A12;
    B4 = A4 + A11;
    B11 = A4 - A11;
    B5 = A5 + A10;
    B10 = A5 - A10;
    B6 = A6 + A9;
    B9 = A6 - A9;
    B7 = A7 + A8;
    B8 = A7 - A8;
    B20 = xTrRound( 181 * ( A27 - A20 ) , DenShift32);
    B27 = xTrRound( 181 * ( A27 + A20 ) , DenShift32);
    B21 = xTrRound( 181 * ( A26 - A21 ) , DenShift32);
    B26 = xTrRound( 181 * ( A26 + A21 ) , DenShift32);
    B22 = xTrRound( 181 * ( A25 - A22 ) , DenShift32);
    B25 = xTrRound( 181 * ( A25 + A22 ) , DenShift32);
    B23 = xTrRound( 181 * ( A24 - A23 ) , DenShift32);
    B24 = xTrRound( 181 * ( A24 + A23 ) , DenShift32);;
    
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 181 * ( B13 - B10 ) , DenShift32);
    C13 = xTrRound( 181 * ( B13 + B10 ) , DenShift32);
    C11 = xTrRound( 181 * ( B12 - B11 ) , DenShift32);
    C12 = xTrRound( 181 * ( B12 + B11 ) , DenShift32);
    C16 = A16 + B23;
    C23 = A16 - B23;
    C24 = A31 - B24;
    C31 = A31 + B24;
    C17 = A17 + B22;
    C22 = A17 - B22;
    C25 = A30 - B25;
    C30 = A30 + B25;
    C18 = A18 + B21;
    C21 = A18 - B21;
    C26 = A29 - B26;
    C29 = A29 + B26;
    C19 = A19 + B20;
    C20 = A19 - B20;
    C27 = A28 - B27;
    C28 = A28 + B27;
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 181 * ( C6 - C5 ) , DenShift32);
    D6 = xTrRound( 181 * ( C6 + C5 ) , DenShift32);
    D18 = xTrRound( 97 * C29 - 236 * C18 , DenShift32);
    D20 = xTrRound(  - 236 * C27 - 97 * C20 , DenShift32);
    D26 = xTrRound(  - 236 * C21 + 97 * C26 , DenShift32);
    D28 = xTrRound( 97 * C19 + 236 * C28 , DenShift32);
    D19 = xTrRound( 97 * C28 - 236 * C19 , DenShift32);
    D21 = xTrRound(  - 236 * C26 - 97 * C21 , DenShift32);
    D27 = xTrRound(  - 236 * C20 + 97 * C27 , DenShift32);
    D29 = xTrRound( 97 * C18 + 236 * C29 , DenShift32);;
    
    aaiTemp[0][y] = xTrRound( 181 * ( D0 + D1 ) , DenShift32);
    aaiTemp[16][y] = xTrRound( 181 * ( D0 - D1 ) , DenShift32);
    aaiTemp[8][y] = xTrRound( 236 * D3 + 97 * D2 , DenShift32);
    aaiTemp[24][y] = xTrRound( 97 * D3 - 236 * D2 , DenShift32);
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 97 * D14 - 236 * D9 , DenShift32);
    E10 = xTrRound(  - 236 * D13 - 97 * D10 , DenShift32);
    E13 = xTrRound( 97 * D13 - 236 * D10 , DenShift32);
    E14 = xTrRound( 236 * D14 + 97 * D9 , DenShift32);
    E16 = C16 + D19;
    E19 = C16 - D19;
    E20 = C23 - D20;
    E23 = C23 + D20;
    E24 = C24 + D27;
    E27 = C24 - D27;
    E28 = C31 - D28;
    E31 = C31 + D28;
    E17 = C17 + D18;
    E18 = C17 - D18;
    E21 = C22 - D21;
    E22 = C22 + D21;
    E25 = C25 + D26;
    E26 = C25 - D26;
    E29 = C30 - D29;
    E30 = C30 + D29;
    
    aaiTemp[4][y] = xTrRound( 49 * E4 + 251 * E7 , DenShift32);
    aaiTemp[20][y] = xTrRound( 212 * E5 + 142 * E6 , DenShift32);
    aaiTemp[12][y] = xTrRound( 212 * E6 - 142 * E5 , DenShift32);
    aaiTemp[28][y] = xTrRound( 49 * E7 - 251 * E4 , DenShift32);
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    F17 = xTrRound( 49 * E30 - 251 * E17 , DenShift32);
    F18 = xTrRound(  - 251 * E29 - 49 * E18 , DenShift32);
    F21 = xTrRound( 212 * E26 - 142 * E21 , DenShift32);
    F22 = xTrRound(  - 142 * E25 - 212 * E22 , DenShift32);
    F25 = xTrRound( 212 * E25 - 142 * E22 , DenShift32);
    F26 = xTrRound( 142 * E26 + 212 * E21 , DenShift32);
    F29 = xTrRound( 49 * E29 - 251 * E18 , DenShift32);
    F30 = xTrRound( 251 * E30 + 49 * E17 , DenShift32);;
    
    aaiTemp[2][y] = xTrRound( 25 * F8 + 254 * F15 , DenShift32);
    aaiTemp[18][y] = xTrRound( 197 * F9 + 162 * F14 , DenShift32);
    aaiTemp[10][y] = xTrRound( 120 * F10 + 225 * F13 , DenShift32);
    aaiTemp[26][y] = xTrRound( 244 * F11 + 74 * F12 , DenShift32);
    aaiTemp[6][y] = xTrRound( 244 * F12 - 74 * F11 , DenShift32);
    aaiTemp[22][y] = xTrRound( 120 * F13 - 225 * F10 , DenShift32);
    aaiTemp[14][y] = xTrRound( 197 * F14 - 162 * F9 , DenShift32);
    aaiTemp[30][y] = xTrRound( 25 * F15 - 254 * F8 , DenShift32);
    G16 = E16 + F17;
    G17 = E16 - F17;
    G18 = E19 - F18;
    G19 = E19 + F18;
    G20 = E20 + F21;
    G21 = E20 - F21;
    G22 = E23 - F22;
    G23 = E23 + F22;
    G24 = E24 + F25;
    G25 = E24 - F25;
    G26 = E27 - F26;
    G27 = E27 + F26;
    G28 = E28 + F29;
    G29 = E28 - F29;
    G30 = E31 - F30;
    G31 = E31 + F30;
    
    aaiTemp[1][y] = xTrRound( 12 * G16 + 255 * G31 , DenShift32);
    aaiTemp[17][y] = xTrRound( 189 * G17 + 171 * G30 , DenShift32);
    aaiTemp[9][y] = xTrRound( 109 * G18 + 231 * G29 , DenShift32);
    aaiTemp[25][y] = xTrRound( 241 * G19 + 86 * G28 , DenShift32);
    aaiTemp[5][y] = xTrRound( 62 * G20 + 248 * G27 , DenShift32);
    aaiTemp[21][y] = xTrRound( 219 * G21 + 131 * G26 , DenShift32);
    aaiTemp[13][y] = xTrRound( 152 * G22 + 205 * G25 , DenShift32);
    aaiTemp[29][y] = xTrRound( 253 * G23 + 37 * G24 , DenShift32);
    aaiTemp[3][y] = xTrRound( 253 * G24 - 37 * G23 , DenShift32);
    aaiTemp[19][y] = xTrRound( 152 * G25 - 205 * G22 , DenShift32);
    aaiTemp[11][y] = xTrRound( 219 * G26 - 131 * G21 , DenShift32);
    aaiTemp[27][y] = xTrRound( 62 * G27 - 248 * G20 , DenShift32);
    aaiTemp[7][y] = xTrRound( 241 * G28 - 86 * G19 , DenShift32);
    aaiTemp[23][y] = xTrRound( 109 * G29 - 231 * G18 , DenShift32);
    aaiTemp[15][y] = xTrRound( 189 * G30 - 171 * G17 , DenShift32);
    aaiTemp[31][y] = xTrRound( 12 * G31 - 255 * G16 , DenShift32);
    
    pSrc += uiStride;
  }
  
  for( x=0 ; x<32 ; x++, pDes++ )
  {
    A0 = aaiTemp[x][0] + aaiTemp[x][31];
    A31 = aaiTemp[x][0] - aaiTemp[x][31];
    A1 = aaiTemp[x][1] + aaiTemp[x][30];
    A30 = aaiTemp[x][1] - aaiTemp[x][30];
    A2 = aaiTemp[x][2] + aaiTemp[x][29];
    A29 = aaiTemp[x][2] - aaiTemp[x][29];
    A3 = aaiTemp[x][3] + aaiTemp[x][28];
    A28 = aaiTemp[x][3] - aaiTemp[x][28];
    A4 = aaiTemp[x][4] + aaiTemp[x][27];
    A27 = aaiTemp[x][4] - aaiTemp[x][27];
    A5 = aaiTemp[x][5] + aaiTemp[x][26];
    A26 = aaiTemp[x][5] - aaiTemp[x][26];
    A6 = aaiTemp[x][6] + aaiTemp[x][25];
    A25 = aaiTemp[x][6] - aaiTemp[x][25];
    A7 = aaiTemp[x][7] + aaiTemp[x][24];
    A24 = aaiTemp[x][7] - aaiTemp[x][24];
    A8 = aaiTemp[x][8] + aaiTemp[x][23];
    A23 = aaiTemp[x][8] - aaiTemp[x][23];
    A9 = aaiTemp[x][9] + aaiTemp[x][22];
    A22 = aaiTemp[x][9] - aaiTemp[x][22];
    A10 = aaiTemp[x][10] + aaiTemp[x][21];
    A21 = aaiTemp[x][10] - aaiTemp[x][21];
    A11 = aaiTemp[x][11] + aaiTemp[x][20];
    A20 = aaiTemp[x][11] - aaiTemp[x][20];
    A12 = aaiTemp[x][12] + aaiTemp[x][19];
    A19 = aaiTemp[x][12] - aaiTemp[x][19];
    A13 = aaiTemp[x][13] + aaiTemp[x][18];
    A18 = aaiTemp[x][13] - aaiTemp[x][18];
    A14 = aaiTemp[x][14] + aaiTemp[x][17];
    A17 = aaiTemp[x][14] - aaiTemp[x][17];
    A15 = aaiTemp[x][15] + aaiTemp[x][16];
    A16 = aaiTemp[x][15] - aaiTemp[x][16];
    
    B0 = A0 + A15;
    B15 = A0 - A15;
    B1 = A1 + A14;
    B14 = A1 - A14;
    B2 = A2 + A13;
    B13 = A2 - A13;
    B3 = A3 + A12;
    B12 = A3 - A12;
    B4 = A4 + A11;
    B11 = A4 - A11;
    B5 = A5 + A10;
    B10 = A5 - A10;
    B6 = A6 + A9;
    B9 = A6 - A9;
    B7 = A7 + A8;
    B8 = A7 - A8;
    B20 = xTrRound( 181 * ( A27 - A20 ) , DenShift32);
    B27 = xTrRound( 181 * ( A27 + A20 ) , DenShift32);
    B21 = xTrRound( 181 * ( A26 - A21 ) , DenShift32);
    B26 = xTrRound( 181 * ( A26 + A21 ) , DenShift32);
    B22 = xTrRound( 181 * ( A25 - A22 ) , DenShift32);
    B25 = xTrRound( 181 * ( A25 + A22 ) , DenShift32);
    B23 = xTrRound( 181 * ( A24 - A23 ) , DenShift32);
    B24 = xTrRound( 181 * ( A24 + A23 ) , DenShift32);;
    
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 181 * ( B13 - B10 ) , DenShift32);
    C13 = xTrRound( 181 * ( B13 + B10 ) , DenShift32);
    C11 = xTrRound( 181 * ( B12 - B11 ) , DenShift32);
    C12 = xTrRound( 181 * ( B12 + B11 ) , DenShift32);
    C16 = A16 + B23;
    C23 = A16 - B23;
    C24 = A31 - B24;
    C31 = A31 + B24;
    C17 = A17 + B22;
    C22 = A17 - B22;
    C25 = A30 - B25;
    C30 = A30 + B25;
    C18 = A18 + B21;
    C21 = A18 - B21;
    C26 = A29 - B26;
    C29 = A29 + B26;
    C19 = A19 + B20;
    C20 = A19 - B20;
    C27 = A28 - B27;
    C28 = A28 + B27;
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 181 * ( C6 - C5 ) , DenShift32);
    D6 = xTrRound( 181 * ( C6 + C5 ) , DenShift32);
    D18 = xTrRound( 97 * C29 - 236 * C18 , DenShift32);
    D20 = xTrRound(  - 236 * C27 - 97 * C20 , DenShift32);
    D26 = xTrRound(  - 236 * C21 + 97 * C26 , DenShift32);
    D28 = xTrRound( 97 * C19 + 236 * C28 , DenShift32);
    D19 = xTrRound( 97 * C28 - 236 * C19 , DenShift32);
    D21 = xTrRound(  - 236 * C26 - 97 * C21 , DenShift32);
    D27 = xTrRound(  - 236 * C20 + 97 * C27 , DenShift32);
    D29 = xTrRound( 97 * C18 + 236 * C29 , DenShift32);;
    
    pDes[0] = xTrRound( 181 * ( D0 + D1 ) , DenShift32);
    pDes[512] = xTrRound( 181 * ( D0 - D1 ) , DenShift32);
    pDes[256] = xTrRound( 236 * D3 + 97 * D2 , DenShift32);
    pDes[768] = xTrRound( 97 * D3 - 236 * D2 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[0]   = (pDes[0]  +offset)>>uiBitDepthIncrease;
    pDes[512] = (pDes[512]+offset)>>uiBitDepthIncrease;
    pDes[256] = (pDes[256]+offset)>>uiBitDepthIncrease;
    pDes[768] = (pDes[768]+offset)>>uiBitDepthIncrease;
#endif
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 97 * D14 - 236 * D9 , DenShift32);
    E10 = xTrRound(  - 236 * D13 - 97 * D10 , DenShift32);
    E13 = xTrRound( 97 * D13 - 236 * D10 , DenShift32);
    E14 = xTrRound( 236 * D14 + 97 * D9 , DenShift32);
    E16 = C16 + D19;
    E19 = C16 - D19;
    E20 = C23 - D20;
    E23 = C23 + D20;
    E24 = C24 + D27;
    E27 = C24 - D27;
    E28 = C31 - D28;
    E31 = C31 + D28;
    E17 = C17 + D18;
    E18 = C17 - D18;
    E21 = C22 - D21;
    E22 = C22 + D21;
    E25 = C25 + D26;
    E26 = C25 - D26;
    E29 = C30 - D29;
    E30 = C30 + D29;
    
    pDes[128] = xTrRound( 49 * E4 + 251 * E7 , DenShift32);
    pDes[640] = xTrRound( 212 * E5 + 142 * E6 , DenShift32);
    pDes[384] = xTrRound( 212 * E6 - 142 * E5 , DenShift32);
    pDes[896] = xTrRound( 49 * E7 - 251 * E4 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[128] = (pDes[128]+offset)>>uiBitDepthIncrease;
    pDes[640] = (pDes[640]+offset)>>uiBitDepthIncrease;
    pDes[384] = (pDes[384]+offset)>>uiBitDepthIncrease;
    pDes[896] = (pDes[896]+offset)>>uiBitDepthIncrease;
#endif
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    F17 = xTrRound( 49 * E30 - 251 * E17 , DenShift32);
    F18 = xTrRound(  - 251 * E29 - 49 * E18 , DenShift32);
    F21 = xTrRound( 212 * E26 - 142 * E21 , DenShift32);
    F22 = xTrRound(  - 142 * E25 - 212 * E22 , DenShift32);
    F25 = xTrRound( 212 * E25 - 142 * E22 , DenShift32);
    F26 = xTrRound( 142 * E26 + 212 * E21 , DenShift32);
    F29 = xTrRound( 49 * E29 - 251 * E18 , DenShift32);
    F30 = xTrRound( 251 * E30 + 49 * E17 , DenShift32);;
    
    pDes[64] = xTrRound( 25 * F8 + 254 * F15 , DenShift32);
    pDes[576] = xTrRound( 197 * F9 + 162 * F14 , DenShift32);
    pDes[320] = xTrRound( 120 * F10 + 225 * F13 , DenShift32);
    pDes[832] = xTrRound( 244 * F11 + 74 * F12 , DenShift32);
    pDes[192] = xTrRound( 244 * F12 - 74 * F11 , DenShift32);
    pDes[704] = xTrRound( 120 * F13 - 225 * F10 , DenShift32);
    pDes[448] = xTrRound( 197 * F14 - 162 * F9 , DenShift32);
    pDes[960] = xTrRound( 25 * F15 - 254 * F8 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[64]  = (pDes[64] +offset)>>uiBitDepthIncrease;
    pDes[576] = (pDes[576]+offset)>>uiBitDepthIncrease;
    pDes[320] = (pDes[320]+offset)>>uiBitDepthIncrease;
    pDes[832] = (pDes[832]+offset)>>uiBitDepthIncrease;
    pDes[192] = (pDes[192]+offset)>>uiBitDepthIncrease;
    pDes[704] = (pDes[704]+offset)>>uiBitDepthIncrease;
    pDes[448] = (pDes[448]+offset)>>uiBitDepthIncrease;
    pDes[960] = (pDes[960]+offset)>>uiBitDepthIncrease;
#endif
    G16 = E16 + F17;
    G17 = E16 - F17;
    G18 = E19 - F18;
    G19 = E19 + F18;
    G20 = E20 + F21;
    G21 = E20 - F21;
    G22 = E23 - F22;
    G23 = E23 + F22;
    G24 = E24 + F25;
    G25 = E24 - F25;
    G26 = E27 - F26;
    G27 = E27 + F26;
    G28 = E28 + F29;
    G29 = E28 - F29;
    G30 = E31 - F30;
    G31 = E31 + F30;
    
    pDes[32] = xTrRound( 12 * G16 + 255 * G31 , DenShift32);
    pDes[544] = xTrRound( 189 * G17 + 171 * G30 , DenShift32);
    pDes[288] = xTrRound( 109 * G18 + 231 * G29 , DenShift32);
    pDes[800] = xTrRound( 241 * G19 + 86 * G28 , DenShift32);
    pDes[160] = xTrRound( 62 * G20 + 248 * G27 , DenShift32);
    pDes[672] = xTrRound( 219 * G21 + 131 * G26 , DenShift32);
    pDes[416] = xTrRound( 152 * G22 + 205 * G25 , DenShift32);
    pDes[928] = xTrRound( 253 * G23 + 37 * G24 , DenShift32);
    pDes[96] = xTrRound( 253 * G24 - 37 * G23 , DenShift32);
    pDes[608] = xTrRound( 152 * G25 - 205 * G22 , DenShift32);
    pDes[352] = xTrRound( 219 * G26 - 131 * G21 , DenShift32);
    pDes[864] = xTrRound( 62 * G27 - 248 * G20 , DenShift32);
    pDes[224] = xTrRound( 241 * G28 - 86 * G19 , DenShift32);
    pDes[736] = xTrRound( 109 * G29 - 231 * G18 , DenShift32);
    pDes[480] = xTrRound( 189 * G30 - 171 * G17 , DenShift32);
    pDes[992] = xTrRound( 12 * G31 - 255 * G16 , DenShift32);
#ifdef TRANS_PRECISION_EXT
    pDes[32]  = (pDes[32] +offset)>>uiBitDepthIncrease;
    pDes[544] = (pDes[544]+offset)>>uiBitDepthIncrease;
    pDes[288] = (pDes[288]+offset)>>uiBitDepthIncrease;
    pDes[800] = (pDes[800]+offset)>>uiBitDepthIncrease;
    pDes[160] = (pDes[160]+offset)>>uiBitDepthIncrease;
    pDes[672] = (pDes[672]+offset)>>uiBitDepthIncrease;
    pDes[416] = (pDes[416]+offset)>>uiBitDepthIncrease;
    pDes[928] = (pDes[928]+offset)>>uiBitDepthIncrease;
    pDes[96]  = (pDes[96] +offset)>>uiBitDepthIncrease;
    pDes[608] = (pDes[608]+offset)>>uiBitDepthIncrease;
    pDes[352] = (pDes[352]+offset)>>uiBitDepthIncrease;
    pDes[864] = (pDes[864]+offset)>>uiBitDepthIncrease;
    pDes[224] = (pDes[224]+offset)>>uiBitDepthIncrease;
    pDes[736] = (pDes[736]+offset)>>uiBitDepthIncrease;
    pDes[480] = (pDes[480]+offset)>>uiBitDepthIncrease;
    pDes[992] = (pDes[992]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::xT16( Pel* pSrc, UInt uiStride, Long* pDes )
{
  Int x, y;
  
  Long aaiTemp[16][16];
  Long B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15;
  Long C0, C1, C2, C3, C4, C5, C6, C7, C10, C11, C12, C13;
  Long D0, D1, D2, D3, D5, D6, D8, D9, D10, D11, D12, D13, D14, D15;
  Long E4, E5, E6, E7, E9, E10, E13, E14;
  Long F8, F9, F10, F11, F12, F13, F14, F15;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift16x16-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  
  //--Butterfly
  for( y=0 ; y<16 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    B0  = (pSrc[0] + pSrc[15])<<uiBitDepthIncrease;
    B15 = (pSrc[0] - pSrc[15])<<uiBitDepthIncrease;
    B1  = (pSrc[1] + pSrc[14])<<uiBitDepthIncrease;
    B14 = (pSrc[1] - pSrc[14])<<uiBitDepthIncrease;
    B2  = (pSrc[2] + pSrc[13])<<uiBitDepthIncrease;
    B13 = (pSrc[2] - pSrc[13])<<uiBitDepthIncrease;
    B3  = (pSrc[3] + pSrc[12])<<uiBitDepthIncrease;
    B12 = (pSrc[3] - pSrc[12])<<uiBitDepthIncrease;
    B4  = (pSrc[4] + pSrc[11])<<uiBitDepthIncrease;
    B11 = (pSrc[4] - pSrc[11])<<uiBitDepthIncrease;
    B5  = (pSrc[5] + pSrc[10])<<uiBitDepthIncrease;
    B10 = (pSrc[5] - pSrc[10])<<uiBitDepthIncrease;
    B6  = (pSrc[6] + pSrc[9])<<uiBitDepthIncrease;
    B9  = (pSrc[6] - pSrc[9])<<uiBitDepthIncrease;
    B7  = (pSrc[7] + pSrc[8])<<uiBitDepthIncrease;
    B8  = (pSrc[7] - pSrc[8])<<uiBitDepthIncrease;
#else
    B0 = pSrc[0] + pSrc[15];
    B15 = pSrc[0] - pSrc[15];
    B1 = pSrc[1] + pSrc[14];
    B14 = pSrc[1] - pSrc[14];
    B2 = pSrc[2] + pSrc[13];
    B13 = pSrc[2] - pSrc[13];
    B3 = pSrc[3] + pSrc[12];
    B12 = pSrc[3] - pSrc[12];
    B4 = pSrc[4] + pSrc[11];
    B11 = pSrc[4] - pSrc[11];
    B5 = pSrc[5] + pSrc[10];
    B10 = pSrc[5] - pSrc[10];
    B6 = pSrc[6] + pSrc[9];
    B9 = pSrc[6] - pSrc[9];
    B7 = pSrc[7] + pSrc[8];
    B8 = pSrc[7] - pSrc[8];
#endif
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 45 * ( B13 - B10 ) , DenShift16);
    C13 = xTrRound( 45 * ( B13 + B10 ) , DenShift16);
    C11 = xTrRound( 45 * ( B12 - B11 ) , DenShift16);
    C12 = xTrRound( 45 * ( B12 + B11 ) , DenShift16);
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 45 * ( C6 - C5 ) , DenShift16);
    D6 = xTrRound( 45 * ( C6 + C5 ) , DenShift16);
    
    aaiTemp[0][y] = xTrRound( 45 * ( D0 + D1 ) , DenShift16);
    aaiTemp[8][y] = xTrRound( 45 * ( D0 - D1 ) , DenShift16);
    aaiTemp[4][y] = xTrRound( 59 * D3 + 24 * D2 , DenShift16);
    aaiTemp[12][y] = xTrRound( 24 * D3 - 59 * D2 , DenShift16);
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 24 * D14 - 59 * D9 , DenShift16);
    E10 = xTrRound(  -59 * D13 - 24 * D10 , DenShift16);
    E13 = xTrRound( 24 * D13 - 59 * D10 , DenShift16);
    E14 = xTrRound( 59 * D14 + 24 * D9 , DenShift16);
    
    aaiTemp[2][y] = xTrRound( 12 * E4 + 62 * E7 , DenShift16);
    aaiTemp[10][y] = xTrRound( 53 * E5 + 35 * E6 , DenShift16);
    aaiTemp[6][y] = xTrRound( 53 * E6 - 35 * E5 , DenShift16);
    aaiTemp[14][y] = xTrRound( 12 * E7 - 62 * E4 , DenShift16);
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    
    aaiTemp[1][y] = xTrRound( 6 * F8 + 63 * F15 , DenShift16);
    aaiTemp[9][y] = xTrRound( 49 * F9 + 40 * F14 , DenShift16);
    aaiTemp[5][y] = xTrRound( 30 * F10 + 56 * F13 , DenShift16);
    aaiTemp[13][y] = xTrRound( 61 * F11 + 18 * F12 , DenShift16);
    aaiTemp[3][y] = xTrRound( 61 * F12 - 18 * F11 , DenShift16);
    aaiTemp[11][y] = xTrRound( 30 * F13 - 56 * F10 , DenShift16);
    aaiTemp[7][y] = xTrRound( 49 * F14 - 40 * F9 , DenShift16);
    aaiTemp[15][y] = xTrRound( 6 * F15 - 63 * F8 , DenShift16);
    
    pSrc += uiStride;
  }
  
  for( x=0 ; x<16 ; x++, pDes++ )
  {
    B0 = aaiTemp[x][0] + aaiTemp[x][15];
    B15 = aaiTemp[x][0] - aaiTemp[x][15];
    B1 = aaiTemp[x][1] + aaiTemp[x][14];
    B14 = aaiTemp[x][1] - aaiTemp[x][14];
    B2 = aaiTemp[x][2] + aaiTemp[x][13];
    B13 = aaiTemp[x][2] - aaiTemp[x][13];
    B3 = aaiTemp[x][3] + aaiTemp[x][12];
    B12 = aaiTemp[x][3] - aaiTemp[x][12];
    B4 = aaiTemp[x][4] + aaiTemp[x][11];
    B11 = aaiTemp[x][4] - aaiTemp[x][11];
    B5 = aaiTemp[x][5] + aaiTemp[x][10];
    B10 = aaiTemp[x][5] - aaiTemp[x][10];
    B6 = aaiTemp[x][6] + aaiTemp[x][9];
    B9 = aaiTemp[x][6] - aaiTemp[x][9];
    B7 = aaiTemp[x][7] + aaiTemp[x][8];
    B8 = aaiTemp[x][7] - aaiTemp[x][8];
    
    C0 = B0 + B7;
    C7 = B0 - B7;
    C1 = B1 + B6;
    C6 = B1 - B6;
    C2 = B2 + B5;
    C5 = B2 - B5;
    C3 = B3 + B4;
    C4 = B3 - B4;
    C10 = xTrRound( 45 * ( B13 - B10 ) , DenShift16);
    C13 = xTrRound( 45 * ( B13 + B10 ) , DenShift16);
    C11 = xTrRound( 45 * ( B12 - B11 ) , DenShift16);
    C12 = xTrRound( 45 * ( B12 + B11 ) , DenShift16);
    
    D0 = C0 + C3;
    D3 = C0 - C3;
    D8 = B8 + C11;
    D11 = B8 - C11;
    D12 = B15 - C12;
    D15 = B15 + C12;
    D1 = C1 + C2;
    D2 = C1 - C2;
    D9 = B9 + C10;
    D10 = B9 - C10;
    D13 = B14 - C13;
    D14 = B14 + C13;
    D5 = xTrRound( 45 * ( C6 - C5 ) , DenShift16);
    D6 = xTrRound( 45 * ( C6 + C5 ) , DenShift16);
    
    pDes[0] = xTrRound( 45 * ( D0 + D1 ) , DenShift16);
    pDes[128] = xTrRound( 45 * ( D0 - D1 ) , DenShift16);
    pDes[64] = xTrRound( 59 * D3 + 24 * D2 , DenShift16);
    pDes[192] = xTrRound( 24 * D3 - 59 * D2 , DenShift16);
#ifdef TRANS_PRECISION_EXT
    pDes[0  ] = (pDes[0  ]+offset)>>uiBitDepthIncrease;
    pDes[128] = (pDes[128]+offset)>>uiBitDepthIncrease;
    pDes[64 ] = (pDes[64 ]+offset)>>uiBitDepthIncrease;
    pDes[192] = (pDes[192]+offset)>>uiBitDepthIncrease;
#endif
    E4 = C4 + D5;
    E5 = C4 - D5;
    E6 = C7 - D6;
    E7 = C7 + D6;
    E9 = xTrRound( 24 * D14 - 59 * D9 , DenShift16);
    E10 = xTrRound(  -59 * D13 - 24 * D10 , DenShift16);
    E13 = xTrRound( 24 * D13 - 59 * D10 , DenShift16);
    E14 = xTrRound( 59 * D14 + 24 * D9 , DenShift16);
    
    pDes[32] = xTrRound( 12 * E4 + 62 * E7 , DenShift16);
    pDes[160] = xTrRound( 53 * E5 + 35 * E6 , DenShift16);
    pDes[96] = xTrRound( 53 * E6 - 35 * E5 , DenShift16);
    pDes[224] = xTrRound( 12 * E7 - 62 * E4 , DenShift16);
#ifdef TRANS_PRECISION_EXT
    pDes[32]  = (pDes[32] +offset)>>uiBitDepthIncrease;
    pDes[160] = (pDes[160]+offset)>>uiBitDepthIncrease;
    pDes[96]  = (pDes[96] +offset)>>uiBitDepthIncrease;
    pDes[224] = (pDes[224]+offset)>>uiBitDepthIncrease;
#endif
    F8 = D8 + E9;
    F9 = D8 - E9;
    F10 = D11 - E10;
    F11 = D11 + E10;
    F12 = D12 + E13;
    F13 = D12 - E13;
    F14 = D15 - E14;
    F15 = D15 + E14;
    
    pDes[16] = xTrRound( 6 * F8 + 63 * F15 , DenShift16);
    pDes[144] = xTrRound( 49 * F9 + 40 * F14 , DenShift16);
    pDes[80] = xTrRound( 30 * F10 + 56 * F13 , DenShift16);
    pDes[208] = xTrRound( 61 * F11 + 18 * F12 , DenShift16);
    pDes[48] = xTrRound( 61 * F12 - 18 * F11 , DenShift16);
    pDes[176] = xTrRound( 30 * F13 - 56 * F10 , DenShift16);
    pDes[112] = xTrRound( 49 * F14 - 40 * F9 , DenShift16);
    pDes[240] = xTrRound( 6 * F15 - 63 * F8 , DenShift16);
#ifdef TRANS_PRECISION_EXT
    pDes[16]  = (pDes[16] +offset)>>uiBitDepthIncrease;
    pDes[144] = (pDes[144]+offset)>>uiBitDepthIncrease;
    pDes[80]  = (pDes[80] +offset)>>uiBitDepthIncrease;
    pDes[208] = (pDes[208]+offset)>>uiBitDepthIncrease;
    pDes[48]  = (pDes[48] +offset)>>uiBitDepthIncrease;
    pDes[176] = (pDes[176]+offset)>>uiBitDepthIncrease;
    pDes[112] = (pDes[112]+offset)>>uiBitDepthIncrease;
    pDes[240] = (pDes[240]+offset)>>uiBitDepthIncrease;
#endif
  }
}


Int TComTrQuant::bitCount_LCEC(Int k,Int pos,Int n,Int lpflag,Int levelMode,Int run,Int maxrun,Int vlc_adaptive,Int N)
{
  UInt cn;
  int vlc,x,cx,vlcNum,bits,temp;
  static const int vlctable_8x8[28] = {8,0,0,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
  static const int vlcTable8[8] = {10,10,3,3,3,4,4,4}; // U,V,Y8x8I,Y8x8P,Y8x8B,Y16x16I,Y16x16P,Y16x16B
  static const int vlcTable4[3] = {2,2,2};             // Y4x4I,Y4x4P,Y4x4B,
  static const int VLClength[11][128] =
  {
    { 1, 2, 3, 4, 5, 6, 7, 9, 9,11,11,11,11,13,13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19},
    { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,10,10,10,10,12,12,12,12,12,12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18},
    { 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
    { 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16},
    { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13},
    { 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,32,33,33,34,34,35,35,36,36,37,37,38,38,39,39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,47,47,48,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,63,64,64,65,65},
    { 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,22,22,22,22,23,23,23,23,24,24,24,24,25,25,25,25,26,26,26,26,27,27,27,27,28,28,28,28,29,29,29,29,30,30,30,30,31,31,31,31,32,32,32,32,33,33,33,33,34,34,34,34},
    { 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19},
    { 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 3, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13},
    { 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,15}
  };
  int sign = k<0 ? 1 : 0;
  
  
  
  k = abs(k);
  if (N != 4 && N!= 8)
  {
    FATAL_ERROR_0("unsupported block size in bitCount_LCEC()" , -1 );
  }
  if (k)
  {
    if (lpflag==1)
    {                       
      x = pos + (k==1 ? 0 : N*N);
      if (N==8)
      {
        cx = m_uiLPTableE8[n*128+x];
        vlcNum = vlcTable8[n];
      }
      else // (N==4)
      {
        cx = m_uiLPTableE4[n*32+x];
        vlcNum = vlcTable4[n];
      }
      bits = VLClength[vlcNum][cx];
      if (k>1)
      {
        temp = 2*(k-2)+sign;
        if(temp > 127)
          temp = 127;
        bits += VLClength[0][temp];
      }
      else
        bits += 1;                                     
    }
    else
    {
      if (!levelMode)
      {                                                    
        if (maxrun > 27)
        {
          cn = g_auiLumaRun8x8[28][k>1 ? 1 : 0][run];
        }
        else
        {
          cn = g_auiLumaRun8x8[maxrun][k>1 ? 1 : 0][run];
        }
        vlc = (maxrun>27) ? 3 : vlctable_8x8[maxrun];
        bits = VLClength[vlc][cn];
        if (k>1)
        {
          temp = 2*(k-2)+sign;
          if(temp > 127)
            temp = 127;
          bits += VLClength[0][temp];
        }
        else
          bits += 1;  
        
      }
      else
      {
        if(k > 127)
          k = 127;
        bits = VLClength[vlc_adaptive][k] + 1;
      }
    }
  }
  else
  {
    if (levelMode)
      bits = VLClength[vlc_adaptive][k];
    else
    {                        
      if (pos==0 && lpflag==0)
      {  
        if (maxrun > 27)
        {
          cn = g_auiLumaRun8x8[28][0][run+1];
        }
        else
        {
          cn = g_auiLumaRun8x8[maxrun][0][run+1];
        }
        vlc = (maxrun>27) ? 3 : vlctable_8x8[maxrun];
        bits = VLClength[vlc][cn];
      }
      else
        bits = 0;
    }
  }
  return bits;
}

Void TComTrQuant::xRateDistOptQuant_LCEC             ( TComDataCU*                     pcCU,
                                                      Long*                           plSrcCoeff,
                                                      TCoeff*&                        piDstCoeff,
                                                      UInt                            uiWidth,
                                                      UInt                            uiHeight,
                                                      UInt&                           uiAbsSum,
                                                      TextType                        eTType,
                                                      UInt                            uiAbsPartIdx )
{
  Int iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,iSum_big_coef,iSign;
  Int atable[5] = {4,6,14,28,0xfffffff};
  Int switch_thr[8] = {49,49,0,49,49,0,49,49};
  
  const UInt* pucScan;
  UInt* piQuantCoef = NULL;
  UInt uiBlkPos,uiPosY,uiPosX,uiLog2BlkSize,uiConvBit,uiLevel,uiMaxLevel,uiMinLevel,uiAbsLevel,uiBestAbsLevel,uiBitShift;
  Int iScanning,iQpRem,iBlockType,iRate;
  Int  iQBits      = m_cQP.m_iBits;
  Int64 lLevelDouble;
  Double dErr,dTemp=0,dNormFactor,rd64UncodedCost,rd64CodedCost,dCurrCost;
  
  uiBitShift = 15;
  iQpRem = m_cQP.m_iRem;
  
  Bool bExt8x8Flag = false;
  uiLog2BlkSize = g_aucConvertToBit[ uiWidth ] + 2; 
  uiConvBit = g_aucConvertToBit[ uiWidth ];
  
  if (uiWidth == 4)
  {
    iBlockType = 0 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
    iQBits = m_cQP.m_iBits;                 
    dNormFactor = pow(2., (2*DQ_BITS+19));
    if ( g_uiBitIncrement ) dNormFactor *= 1<<(2*g_uiBitIncrement);
    piQuantCoef = ( g_aiQuantCoef[m_cQP.rem()] );
  }
  else if (uiWidth == 8)
  {
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
    iQBits = m_cQP.m_iBits + 1;                 
    dNormFactor = pow(2., (2*Q_BITS_8+9)); 
    if ( g_uiBitIncrement ) dNormFactor *= 1<<(2*g_uiBitIncrement);
    piQuantCoef = ( g_aiQuantCoef64[m_cQP.rem()] );
  }
  else
  {
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
      iBlockType = eTType-2;
    else
      iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() ); 
    
    if(!pcCU->isIntra(uiAbsPartIdx))
    {
      uiLog2BlkSize = g_aucConvertToBit[ 8 ] + 2; 
      uiConvBit = g_aucConvertToBit[ 8 ];
    }
    dNormFactor = pow(2., 21);
    if ( g_uiBitIncrement ) dNormFactor *= 1<<(2*g_uiBitIncrement);
    
    bExt8x8Flag = true;
    
    if ( uiWidth == 16)
    {
      piQuantCoef = ( g_aiQuantCoef256[m_cQP.rem()] );
      iQBits = ECore16Shift + m_cQP.per();     
      dTemp = estErr16x16[iQpRem]/dNormFactor;
    }
    else if ( uiWidth == 32)
    {
      piQuantCoef = ( g_aiQuantCoef1024[m_cQP.rem()] );
      iQBits = ECore32Shift + m_cQP.per();
      dTemp = estErr32x32[iQpRem]/dNormFactor;
    }
    else
    {
      assert(0);
    }
    memset(&piDstCoeff[0],0,uiWidth*uiHeight*sizeof(TCoeff)); 
  }
  
  pucScan = g_auiFrameScanXY [ uiConvBit + 1 ];
  
  iLpFlag = 1;  // shawn note: last position flag
  iLevelMode = 0;
  iRun = 0;
  iVlc_adaptive = 0;
  iMaxrun = 0;
  iSum_big_coef = 0;
  
  for (iScanning=(uiWidth<8 ? 15 : 63); iScanning>=0; iScanning--) 
  {            
    uiBlkPos = pucScan[iScanning];
    uiPosY   = uiBlkPos >> uiLog2BlkSize;
    uiPosX   = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    
    if (uiWidth==4)
      dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor; 
    else if(uiWidth==8)
      dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if(!pcCU->isIntra(uiAbsPartIdx))
      uiBlkPos = uiWidth*uiPosY+uiPosX;
    
    lLevelDouble = abs(plSrcCoeff[uiBlkPos]);
    
    lLevelDouble = lLevelDouble * (Int64) ( uiWidth == 64? piQuantCoef[m_cQP.rem()]: piQuantCoef[uiBlkPos] );
    
    iSign = plSrcCoeff[uiBlkPos]<0 ? -1 : 1;
    
    
    uiLevel = (UInt)(lLevelDouble  >> iQBits);      
    uiMaxLevel = uiLevel + 1;
    uiMinLevel = Max(1,(Int)uiLevel - 2);
    
    uiBestAbsLevel = 0;
    if (uiWidth==4)
      iRate = bitCount_LCEC(0,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4)<<uiBitShift; 
    else 
      iRate = bitCount_LCEC(0,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8)<<uiBitShift; 
    
    dErr = Double( lLevelDouble );
    rd64UncodedCost = dErr * dErr * dTemp;
    rd64CodedCost   = rd64UncodedCost + xGetICost( iRate ); 
    for(uiAbsLevel = uiMinLevel; uiAbsLevel <= uiMaxLevel ; uiAbsLevel++ ) 
    {
      if (uiWidth==4)
        iRate = bitCount_LCEC(iSign*uiAbsLevel,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,4)<<uiBitShift; 
      else 
        iRate = bitCount_LCEC(iSign*uiAbsLevel,iScanning,iBlockType,iLpFlag,iLevelMode,iRun,iMaxrun,iVlc_adaptive,8)<<uiBitShift; 
      dErr = Double( lLevelDouble  - (((Int64)uiAbsLevel) << iQBits ) );
      rd64UncodedCost = dErr * dErr * dTemp;
      dCurrCost = rd64UncodedCost + xGetICost( iRate ); 
      if( dCurrCost < rd64CodedCost )
      {         
        uiBestAbsLevel  = uiAbsLevel;
        rd64CodedCost   = dCurrCost;
      }
    }
    
    
    if (uiBestAbsLevel)
    {                  
      if (uiWidth>4)
      { 
        if (!iLpFlag && uiBestAbsLevel > 1)
        {
          iSum_big_coef += uiBestAbsLevel;
          if ((63-iScanning) > switch_thr[iBlockType] || iSum_big_coef > 2)
            iLevelMode = 1;
        }
      }
      else
      {
        if (uiBestAbsLevel>1)
          iLevelMode = 1;
      }
      iMaxrun = iScanning-1;
      iLpFlag = 0;
      iRun = 0;
      if (iLevelMode && (uiBestAbsLevel > atable[iVlc_adaptive]))
        iVlc_adaptive++;                    
    }
    else
    {
      iRun += 1;         
    }
    
    uiAbsSum += uiBestAbsLevel;
    piDstCoeff[uiBlkPos] = iSign*uiBestAbsLevel;
  } // for uiScanning
} 


Void TComTrQuant::xQuantLTR  (TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
  Long*   piCoef    = pSrc;
  TCoeff* piQCoef   = pDes;
  UInt* piQuantCoef = NULL;
  Int   iNewBits    = 0;
  Int   iAdd = 0;
  
  switch(iWidth)
  {
    case 2:
    {
      m_puiQuantMtx = &g_aiQuantCoef4[m_cQP.m_iRem];
      xQuant2x2(piCoef, piQCoef, uiAcSum );
      return;
    }
    case 4:
    {
      m_puiQuantMtx = &g_aiQuantCoef[m_cQP.m_iRem][0];
      xQuant4x4(pcCU, piCoef, piQCoef, uiAcSum, eTType, uiAbsPartIdx );
      return;
    }
    case 8:
    {
      m_puiQuantMtx = &g_aiQuantCoef64[m_cQP.m_iRem][0];
      xQuant8x8(pcCU, piCoef, piQCoef, uiAcSum, eTType, uiAbsPartIdx );
      return;
    }
    case 16:
    {
      piQuantCoef = ( g_aiQuantCoef256[m_cQP.rem()] );
      iNewBits = ECore16Shift + m_cQP.per();
      iAdd = m_cQP.m_iAdd16x16;
      break;
    }
    case 32:
    {
      piQuantCoef = ( g_aiQuantCoef1024[m_cQP.rem()] );
      iNewBits = ECore32Shift + m_cQP.per();
      iAdd = m_cQP.m_iAdd32x32;
      break;
    }
    default:
      assert(0);
      break;
  }
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant( pcCU, piCoef, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
  }
  else
  {
    UInt uiAcSum_init = uiAcSum;
    for( Int n = 0; n < iWidth*iHeight; n++ )
    {
      Long iLevel;
      Int  iSign;
      iLevel  = (Long) piCoef[n];
      iSign   = (iLevel < 0 ? -1: 1);
      if ( iWidth == 64 ) iLevel = abs( iLevel ) * piQuantCoef[m_cQP.rem()];
      else                iLevel = abs( iLevel ) * piQuantCoef[n];
      
      if (!pcCU->isIntra( uiAbsPartIdx ) && (m_iSymbolMode == 0) && ((n%iWidth)>=8 || (n/iWidth)>=8))
        iLevel = 0;
      else
        iLevel = ( iLevel + iAdd ) >> iNewBits;
      
      if( 0 != iLevel )
      {
        uiAcSum += iLevel;
        iLevel    *= iSign;
        piQCoef[n] = iLevel;
      }
      else
      {
        piQCoef[n] = 0;
      }
    }
    
    const UInt*  pucScan;
    if(pcCU->isIntra( uiAbsPartIdx ) && m_iSymbolMode == 0 && iWidth >= 16)
    {
      UInt uiConvBit = g_aucConvertToBit[ iWidth ];
      pucScan        = g_auiFrameScanXY [ uiConvBit + 1 ];
      for( Int n = 64; n < iWidth*iHeight; n++ )
      {
        piQCoef[ pucScan[ n ] ] = 0;
      }
      
      uiAcSum = uiAcSum_init;
      
      for( Int n = 0; n < 64; n++ )
      {
        uiAcSum += abs(piQCoef[ pucScan[ n ] ]);
      }
    }
  }
}

Void TComTrQuant::xDeQuantLTR( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight )
{
  UInt* piDeQuantCoef = NULL;
  
  TCoeff* piQCoef   = pSrc;
  Long*   piCoef    = pDes;
  
  if ( iWidth > (Int)m_uiMaxTrSize )
  {
    iWidth  = m_uiMaxTrSize;
    iHeight = m_uiMaxTrSize;
  }
  
  switch(iWidth)
  {
    case 2:
    {
      xDeQuant2x2( piQCoef, piCoef );
      return;
    }
    case 4:
    {
      xDeQuant4x4( piQCoef, piCoef );
      return;
    }
    case 8:
    {
      xDeQuant8x8( piQCoef, piCoef );
      return;
    }
    case 16:
    {
      piDeQuantCoef = ( g_aiDeQuantCoef256[m_cQP.rem()] );
      break;
    }
    case 32:
    {
      piDeQuantCoef = ( g_aiDeQuantCoef1024[m_cQP.rem()] );
      break;
    }
    case 64:
    {
      piDeQuantCoef = ( g_aiDeQuantCoef4096 ); // To save the memory for g_aiDeQuantCoef4096
      break;
    }
  }
  
  Int iLevel;
  Int iDeScale;
  
  for( Int n = 0; n < iWidth*iHeight; n++ )
  {
    iLevel  = piQCoef[n];
    
    if( 0 != iLevel )
    {
      if ( iWidth == 64 ) iDeScale = piDeQuantCoef[m_cQP.rem()];
      else                iDeScale = piDeQuantCoef[n];
      piCoef[n] = (Long) (iLevel*iDeScale) << m_cQP.per();
    }
    else
    {
      piCoef [n] = 0;
    }
  }
}

Void TComTrQuant::xIT16( Long* pSrc, Pel* pDes, UInt uiStride )
{
  Int x, y;
  Long aaiTemp[16][16];
  
  Long B0, B1, B2, B3, B4, B5, B6, B7, B10, B11, B12, B13;
  Long C0, C1, C2, C3, C5, C6, C8, C9, C10, C11, C12, C13, C14, C15;
  Long D0, D1, D2, D3, D4, D5, D6, D7, D9, D10, D13, D14;
  Long E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15;
  Long F8, F9, F10, F11, F12, F13, F14, F15;
  
  UInt uiStride2  = uiStride<<1;
  UInt uiStride3  = uiStride2  + uiStride;
  UInt uiStride4  = uiStride3  + uiStride;
  UInt uiStride5  = uiStride4  + uiStride;
  UInt uiStride6  = uiStride5  + uiStride;
  UInt uiStride7  = uiStride6  + uiStride;
  UInt uiStride8  = uiStride7  + uiStride;
  UInt uiStride9  = uiStride8  + uiStride;
  UInt uiStride10 = uiStride9  + uiStride;
  UInt uiStride11 = uiStride10 + uiStride;
  UInt uiStride12 = uiStride11 + uiStride;
  UInt uiStride13 = uiStride12 + uiStride;
  UInt uiStride14 = uiStride13 + uiStride;
  UInt uiStride15 = uiStride14 + uiStride;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift16x16-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  //--Butterfly
  for( y=0 ; y<16 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    Long     ai0[16];
    ai0[0] =  pSrc[0]<<uiBitDepthIncrease;
    ai0[1] =  pSrc[1]<<uiBitDepthIncrease;
    ai0[2] =  pSrc[2]<<uiBitDepthIncrease;
    ai0[3] =  pSrc[3]<<uiBitDepthIncrease;
    ai0[4] =  pSrc[4]<<uiBitDepthIncrease;
    ai0[5] =  pSrc[5]<<uiBitDepthIncrease;
    ai0[6] =  pSrc[6]<<uiBitDepthIncrease;
    ai0[7] =  pSrc[7]<<uiBitDepthIncrease;
    ai0[8 ] =  pSrc[8 ]<<uiBitDepthIncrease;
    ai0[9 ] =  pSrc[9 ]<<uiBitDepthIncrease;
    ai0[10] =  pSrc[10]<<uiBitDepthIncrease;
    ai0[11] =  pSrc[11]<<uiBitDepthIncrease;
    ai0[12] =  pSrc[12]<<uiBitDepthIncrease;
    ai0[13] =  pSrc[13]<<uiBitDepthIncrease;
    ai0[14] =  pSrc[14]<<uiBitDepthIncrease;
    ai0[15] =  pSrc[15]<<uiBitDepthIncrease;
    F8 = xTrRound( 6 * ai0[1] - 63 * ai0[15] , DenShift16);
    F9 = xTrRound( 49 * ai0[9] - 40 * ai0[7] , DenShift16);
    F10 = xTrRound( 30 * ai0[5] - 56 * ai0[11] , DenShift16);
    F11 = xTrRound( 61 * ai0[13] - 18 * ai0[3] , DenShift16);
    F12 = xTrRound( 61 * ai0[3] + 18 * ai0[13] , DenShift16);
    F13 = xTrRound( 30 * ai0[11] + 56 * ai0[5] , DenShift16);
    F14 = xTrRound( 49 * ai0[7] + 40 * ai0[9] , DenShift16);
    F15 = xTrRound( 6 * ai0[15] + 63 * ai0[1] , DenShift16);
    
    E4 = xTrRound( 12 * ai0[2] - 62 * ai0[14] , DenShift16);
    E5 = xTrRound( 53 * ai0[10] - 35 * ai0[6] , DenShift16);
    E6 = xTrRound( 53 * ai0[6] + 35 * ai0[10] , DenShift16);
    E7 = xTrRound( 12 * ai0[14] + 62 * ai0[2] , DenShift16);
#else
    F8 = xTrRound( 6 * pSrc[1] - 63 * pSrc[15] , DenShift16);
    F9 = xTrRound( 49 * pSrc[9] - 40 * pSrc[7] , DenShift16);
    F10 = xTrRound( 30 * pSrc[5] - 56 * pSrc[11] , DenShift16);
    F11 = xTrRound( 61 * pSrc[13] - 18 * pSrc[3] , DenShift16);
    F12 = xTrRound( 61 * pSrc[3] + 18 * pSrc[13] , DenShift16);
    F13 = xTrRound( 30 * pSrc[11] + 56 * pSrc[5] , DenShift16);
    F14 = xTrRound( 49 * pSrc[7] + 40 * pSrc[9] , DenShift16);
    F15 = xTrRound( 6 * pSrc[15] + 63 * pSrc[1] , DenShift16);
    
    E4 = xTrRound( 12 * pSrc[2] - 62 * pSrc[14] , DenShift16);
    E5 = xTrRound( 53 * pSrc[10] - 35 * pSrc[6] , DenShift16);
    E6 = xTrRound( 53 * pSrc[6] + 35 * pSrc[10] , DenShift16);
    E7 = xTrRound( 12 * pSrc[14] + 62 * pSrc[2] , DenShift16);
#endif
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
#ifdef TRANS_PRECISION_EXT
    D0 = xTrRound( 45 * ( ai0[0] + ai0[8] ) , DenShift16);
    D1 = xTrRound( 45 * ( ai0[0] - ai0[8] ) , DenShift16);
    D2 = xTrRound( 24 * ai0[4] - 59 * ai0[12] , DenShift16);
    D3 = xTrRound( 59 * ai0[4] + 24 * ai0[12] , DenShift16);
#else
    D0 = xTrRound( 45 * ( pSrc[0] + pSrc[8] ) , DenShift16);
    D1 = xTrRound( 45 * ( pSrc[0] - pSrc[8] ) , DenShift16);
    D2 = xTrRound( 24 * pSrc[4] - 59 * pSrc[12] , DenShift16);
    D3 = xTrRound( 59 * pSrc[4] + 24 * pSrc[12] , DenShift16);
#endif
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 24 * E14 - 59 * E9 , DenShift16);
    D10 = xTrRound(  - 59 * E13 - 24 * E10 , DenShift16);
    D13 = xTrRound( 24 * E13 - 59 * E10 , DenShift16);
    D14 = xTrRound( 59 * E14 + 24 * E9 , DenShift16);
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 45 * ( D6 - D5 ) , DenShift16);
    C6 = xTrRound( 45 * ( D6 + D5 ) , DenShift16);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 45 * ( C13 - C10 ) , DenShift16);
    B13 = xTrRound( 45 * ( C13 + C10 ) , DenShift16);
    B11 = xTrRound( 45 * ( C12 - C11 ) , DenShift16);
    B12 = xTrRound( 45 * ( C12 + C11 ) , DenShift16);
    
    aaiTemp[0][y] = B0 + C15;
    aaiTemp[15][y] = B0 - C15;
    aaiTemp[1][y] = B1 + C14;
    aaiTemp[14][y] = B1 - C14;
    aaiTemp[2][y] = B2 + B13;
    aaiTemp[13][y] = B2 - B13;
    aaiTemp[3][y] = B3 + B12;
    aaiTemp[12][y] = B3 - B12;
    aaiTemp[4][y] = B4 + B11;
    aaiTemp[11][y] = B4 - B11;
    aaiTemp[5][y] = B5 + B10;
    aaiTemp[10][y] = B5 - B10;
    aaiTemp[6][y] = B6 + C9;
    aaiTemp[9][y] = B6 - C9;
    aaiTemp[7][y] = B7 + C8;
    aaiTemp[8][y] = B7 - C8;
    
    pSrc += 16;
  }
  
  for( x=0 ; x<16 ; x++, pDes++ )
  {
    F8 = xTrRound( 6 * aaiTemp[x][1] - 63 * aaiTemp[x][15] , DenShift16);
    F9 = xTrRound( 49 * aaiTemp[x][9] - 40 * aaiTemp[x][7] , DenShift16);
    F10 = xTrRound( 30 * aaiTemp[x][5] - 56 * aaiTemp[x][11] , DenShift16);
    F11 = xTrRound( 61 * aaiTemp[x][13] - 18 * aaiTemp[x][3] , DenShift16);
    F12 = xTrRound( 61 * aaiTemp[x][3] + 18 * aaiTemp[x][13] , DenShift16);
    F13 = xTrRound( 30 * aaiTemp[x][11] + 56 * aaiTemp[x][5] , DenShift16);
    F14 = xTrRound( 49 * aaiTemp[x][7] + 40 * aaiTemp[x][9] , DenShift16);
    F15 = xTrRound( 6 * aaiTemp[x][15] + 63 * aaiTemp[x][1] , DenShift16);
    
    E4 = xTrRound( 12 * aaiTemp[x][2] - 62 * aaiTemp[x][14] , DenShift16);
    E5 = xTrRound( 53 * aaiTemp[x][10] - 35 * aaiTemp[x][6] , DenShift16);
    E6 = xTrRound( 53 * aaiTemp[x][6] + 35 * aaiTemp[x][10] , DenShift16);
    E7 = xTrRound( 12 * aaiTemp[x][14] + 62 * aaiTemp[x][2] , DenShift16);
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
    
    D0 = xTrRound( 45 * ( aaiTemp[x][0] + aaiTemp[x][8] ) , DenShift16);
    D1 = xTrRound( 45 * ( aaiTemp[x][0] - aaiTemp[x][8] ) , DenShift16);
    D2 = xTrRound( 24 * aaiTemp[x][4] - 59 * aaiTemp[x][12] , DenShift16);
    D3 = xTrRound( 59 * aaiTemp[x][4] + 24 * aaiTemp[x][12] , DenShift16);
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 24 * E14 - 59 * E9 , DenShift16);
    D10 = xTrRound(  - 59 * E13 - 24 * E10 , DenShift16);
    D13 = xTrRound( 24 * E13 - 59 * E10 , DenShift16);
    D14 = xTrRound( 59 * E14 + 24 * E9 , DenShift16);
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 45 * ( D6 - D5 ) , DenShift16);
    C6 = xTrRound( 45 * ( D6 + D5 ) , DenShift16);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 45 * ( C13 - C10 ) , DenShift16);
    B13 = xTrRound( 45 * ( C13 + C10 ) , DenShift16);
    B11 = xTrRound( 45 * ( C12 - C11 ) , DenShift16);
    B12 = xTrRound( 45 * ( C12 + C11 ) , DenShift16);
    
    pDes[          0] = (Pel)xTrRound(B0 + C15, DCore16Shift);
    pDes[uiStride15] = (Pel)xTrRound(B0 - C15, DCore16Shift);
    pDes[uiStride  ] = (Pel)xTrRound(B1 + C14, DCore16Shift);
    pDes[uiStride14] = (Pel)xTrRound(B1 - C14, DCore16Shift);
    pDes[uiStride2 ] = (Pel)xTrRound(B2 + B13, DCore16Shift);
    pDes[uiStride13] = (Pel)xTrRound(B2 - B13, DCore16Shift);
    pDes[uiStride3 ] = (Pel)xTrRound(B3 + B12, DCore16Shift);
    pDes[uiStride12] = (Pel)xTrRound(B3 - B12, DCore16Shift);
    pDes[uiStride4 ] = (Pel)xTrRound(B4 + B11, DCore16Shift);
    pDes[uiStride11] = (Pel)xTrRound(B4 - B11, DCore16Shift);
    pDes[uiStride5 ] = (Pel)xTrRound(B5 + B10, DCore16Shift);
    pDes[uiStride10] = (Pel)xTrRound(B5 - B10, DCore16Shift);
    pDes[uiStride6 ] = (Pel)xTrRound(B6 + C9, DCore16Shift);
    pDes[uiStride9 ] = (Pel)xTrRound(B6 - C9, DCore16Shift);
    pDes[uiStride7 ] = (Pel)xTrRound(B7 + C8, DCore16Shift);
    pDes[uiStride8 ] = (Pel)xTrRound(B7 - C8, DCore16Shift);
#ifdef TRANS_PRECISION_EXT
    pDes[        0 ] =  (pDes[        0 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride  ] =  (pDes[uiStride  ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride2 ] =  (pDes[uiStride2 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride3 ] =  (pDes[uiStride3 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride4 ] =  (pDes[uiStride4 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride5 ] =  (pDes[uiStride5 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride6 ] =  (pDes[uiStride6 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride7 ] =  (pDes[uiStride7 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride8 ] =  (pDes[uiStride8 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride9 ] =  (pDes[uiStride9 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride10] =  (pDes[uiStride10]+offset)>>uiBitDepthIncrease;
    pDes[uiStride11] =  (pDes[uiStride11]+offset)>>uiBitDepthIncrease;
    pDes[uiStride12] =  (pDes[uiStride12]+offset)>>uiBitDepthIncrease;
    pDes[uiStride13] =  (pDes[uiStride13]+offset)>>uiBitDepthIncrease;
    pDes[uiStride14] =  (pDes[uiStride14]+offset)>>uiBitDepthIncrease;
    pDes[uiStride15] =  (pDes[uiStride15]+offset)>>uiBitDepthIncrease;
#endif
    
  }
}

Void TComTrQuant::xIT32( Long* pSrc, Pel* pDes, UInt uiStride )
{
  Int x, y;
  Long aaiTemp[32][32];
  
  Long A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A20, A21, A22, A23, A24, A25, A26, A27;
  Long B0, B1, B2, B3, B4, B5, B6, B7, B10, B11, B12, B13, B16, B17, B18, B19, B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31;
  Long C0, C1, C2, C3, C5, C6, C8, C9, C10, C11, C12, C13, C14, C15, C18, C19, C20, C21, C26, C27, C28, C29;
  Long D0, D1, D2, D3, D4, D5, D6, D7, D9, D10, D13, D14, D16, D17, D18, D19, D20, D21, D22, D23, D24, D25, D26, D27, D28, D29, D30, D31;
  Long E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E17, E18, E21, E22, E25, E26, E29, E30;
  Long F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30, F31;
  Long G16, G17, G18, G19, G20, G21, G22, G23, G24, G25, G26, G27, G28, G29, G30, G31;
  
  UInt uiStride2  = uiStride<<1;
  UInt uiStride3  = uiStride2  + uiStride;
  UInt uiStride4  = uiStride3  + uiStride;
  UInt uiStride5  = uiStride4  + uiStride;
  UInt uiStride6  = uiStride5  + uiStride;
  UInt uiStride7  = uiStride6  + uiStride;
  UInt uiStride8  = uiStride7  + uiStride;
  UInt uiStride9  = uiStride8  + uiStride;
  UInt uiStride10 = uiStride9  + uiStride;
  UInt uiStride11 = uiStride10 + uiStride;
  UInt uiStride12 = uiStride11 + uiStride;
  UInt uiStride13 = uiStride12 + uiStride;
  UInt uiStride14 = uiStride13 + uiStride;
  UInt uiStride15 = uiStride14 + uiStride;
  UInt uiStride16 = uiStride15 + uiStride;
  UInt uiStride17 = uiStride16 + uiStride;
  UInt uiStride18 = uiStride17 + uiStride;
  UInt uiStride19 = uiStride18 + uiStride;
  UInt uiStride20 = uiStride19 + uiStride;
  UInt uiStride21 = uiStride20 + uiStride;
  UInt uiStride22 = uiStride21 + uiStride;
  UInt uiStride23 = uiStride22 + uiStride;
  UInt uiStride24 = uiStride23 + uiStride;
  UInt uiStride25 = uiStride24 + uiStride;
  UInt uiStride26 = uiStride25 + uiStride;
  UInt uiStride27 = uiStride26 + uiStride;
  UInt uiStride28 = uiStride27 + uiStride;
  UInt uiStride29 = uiStride28 + uiStride;
  UInt uiStride30 = uiStride29 + uiStride;
  UInt uiStride31 = uiStride30 + uiStride;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift32x32-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  //--Butterfly
  for( y=0 ; y<32 ; y++ )
  {
#ifdef TRANS_PRECISION_EXT
    Long     ai0[32];
    ai0[0] =  pSrc[0]<<uiBitDepthIncrease;
    ai0[1] =  pSrc[1]<<uiBitDepthIncrease;
    ai0[2] =  pSrc[2]<<uiBitDepthIncrease;
    ai0[3] =  pSrc[3]<<uiBitDepthIncrease;
    ai0[4] =  pSrc[4]<<uiBitDepthIncrease;
    ai0[5] =  pSrc[5]<<uiBitDepthIncrease;
    ai0[6] =  pSrc[6]<<uiBitDepthIncrease;
    ai0[7] =  pSrc[7]<<uiBitDepthIncrease;
    ai0[8 ] =  pSrc[8 ]<<uiBitDepthIncrease;
    ai0[9 ] =  pSrc[9 ]<<uiBitDepthIncrease;
    ai0[10] =  pSrc[10]<<uiBitDepthIncrease;
    ai0[11] =  pSrc[11]<<uiBitDepthIncrease;
    ai0[12] =  pSrc[12]<<uiBitDepthIncrease;
    ai0[13] =  pSrc[13]<<uiBitDepthIncrease;
    ai0[14] =  pSrc[14]<<uiBitDepthIncrease;
    ai0[15] =  pSrc[15]<<uiBitDepthIncrease;
    ai0[16] =  pSrc[16]<<uiBitDepthIncrease;
    ai0[17] =  pSrc[17]<<uiBitDepthIncrease;
    ai0[18] =  pSrc[18]<<uiBitDepthIncrease;
    ai0[19] =  pSrc[19]<<uiBitDepthIncrease;
    ai0[20] =  pSrc[20]<<uiBitDepthIncrease;
    ai0[21] =  pSrc[21]<<uiBitDepthIncrease;
    ai0[22] =  pSrc[22]<<uiBitDepthIncrease;
    ai0[23] =  pSrc[23]<<uiBitDepthIncrease;
    ai0[24] =  pSrc[24]<<uiBitDepthIncrease;
    ai0[25] =  pSrc[25]<<uiBitDepthIncrease;
    ai0[26] =  pSrc[26]<<uiBitDepthIncrease;
    ai0[27] =  pSrc[27]<<uiBitDepthIncrease;
    ai0[28] =  pSrc[28]<<uiBitDepthIncrease;
    ai0[29] =  pSrc[29]<<uiBitDepthIncrease;
    ai0[30] =  pSrc[30]<<uiBitDepthIncrease;
    ai0[31] =  pSrc[31]<<uiBitDepthIncrease;
    G16 = xTrRound(  12 * ai0[1]  - 255 * ai0[31], DenShift32);
    G17 = xTrRound( 189 * ai0[17] - 171 * ai0[15], DenShift32);
    G18 = xTrRound( 109 * ai0[9]  - 231 * ai0[23], DenShift32);
    G19 = xTrRound( 241 * ai0[25] -  86 * ai0[7], DenShift32);
    G20 = xTrRound(  62 * ai0[5]  - 248 * ai0[27], DenShift32);
    G21 = xTrRound( 219 * ai0[21] - 131 * ai0[11], DenShift32);
    G22 = xTrRound( 152 * ai0[13] - 205 * ai0[19], DenShift32);
    G23 = xTrRound( 253 * ai0[29] -  37 * ai0[3], DenShift32);
    G24 = xTrRound( 253 * ai0[3]  +  37 * ai0[29], DenShift32);
    G25 = xTrRound( 152 * ai0[19] + 205 * ai0[13], DenShift32);
    G26 = xTrRound( 219 * ai0[11] + 131 * ai0[21], DenShift32);
    G27 = xTrRound(  62 * ai0[27] + 248 * ai0[5], DenShift32);
    G28 = xTrRound( 241 * ai0[7]  +  86 * ai0[25], DenShift32);
    G29 = xTrRound( 109 * ai0[23] + 231 * ai0[9], DenShift32);
    G30 = xTrRound( 189 * ai0[15] + 171 * ai0[17], DenShift32);
    G31 = xTrRound(  12 * ai0[31] + 255 * ai0[1], DenShift32);
    
    F8  = xTrRound(  25 * ai0[2]  - 254 * ai0[30], DenShift32);
    F9  = xTrRound( 197 * ai0[18] - 162 * ai0[14], DenShift32);
    F10 = xTrRound( 120 * ai0[10] - 225 * ai0[22], DenShift32);
    F11 = xTrRound( 244 * ai0[26] -  74 * ai0[6], DenShift32);
    F12 = xTrRound( 244 * ai0[6]  +  74 * ai0[26], DenShift32);
    F13 = xTrRound( 120 * ai0[22] + 225 * ai0[10], DenShift32);
    F14 = xTrRound( 197 * ai0[14] + 162 * ai0[18], DenShift32);
    F15 = xTrRound(  25 * ai0[30] + 254 * ai0[2], DenShift32);
#else
    G16 = xTrRound( 12 * pSrc[1] - 255 * pSrc[31], DenShift32);
    G17 = xTrRound( 189 * pSrc[17] - 171 * pSrc[15], DenShift32);
    G18 = xTrRound( 109 * pSrc[9] - 231 * pSrc[23], DenShift32);
    G19 = xTrRound( 241 * pSrc[25] - 86 * pSrc[7], DenShift32);
    G20 = xTrRound( 62 * pSrc[5] - 248 * pSrc[27], DenShift32);
    G21 = xTrRound( 219 * pSrc[21] - 131 * pSrc[11], DenShift32);
    G22 = xTrRound( 152 * pSrc[13] - 205 * pSrc[19], DenShift32);
    G23 = xTrRound( 253 * pSrc[29] - 37 * pSrc[3], DenShift32);
    G24 = xTrRound( 253 * pSrc[3] + 37 * pSrc[29], DenShift32);
    G25 = xTrRound( 152 * pSrc[19] + 205 * pSrc[13], DenShift32);
    G26 = xTrRound( 219 * pSrc[11] + 131 * pSrc[21], DenShift32);
    G27 = xTrRound( 62 * pSrc[27] + 248 * pSrc[5], DenShift32);
    G28 = xTrRound( 241 * pSrc[7] + 86 * pSrc[25], DenShift32);
    G29 = xTrRound( 109 * pSrc[23] + 231 * pSrc[9], DenShift32);
    G30 = xTrRound( 189 * pSrc[15] + 171 * pSrc[17], DenShift32);
    G31 = xTrRound( 12 * pSrc[31] + 255 * pSrc[1], DenShift32);
    
    F8 = xTrRound( 25 * pSrc[2] - 254 * pSrc[30], DenShift32);
    F9 = xTrRound( 197 * pSrc[18] - 162 * pSrc[14], DenShift32);
    F10 = xTrRound( 120 * pSrc[10] - 225 * pSrc[22], DenShift32);
    F11 = xTrRound( 244 * pSrc[26] - 74 * pSrc[6], DenShift32);
    F12 = xTrRound( 244 * pSrc[6] + 74 * pSrc[26], DenShift32);
    F13 = xTrRound( 120 * pSrc[22] + 225 * pSrc[10], DenShift32);
    F14 = xTrRound( 197 * pSrc[14] + 162 * pSrc[18], DenShift32);
    F15 = xTrRound( 25 * pSrc[30] + 254 * pSrc[2], DenShift32);
#endif
    F16 = G16 + G17;
    F17 = G16 - G17;
    F18 = G19 - G18;
    F19 = G19 + G18;
    F20 = G20 + G21;
    F21 = G20 - G21;
    F22 = G23 - G22;
    F23 = G23 + G22;
    F24 = G24 + G25;
    F25 = G24 - G25;
    F26 = G27 - G26;
    F27 = G27 + G26;
    F28 = G28 + G29;
    F29 = G28 - G29;
    F30 = G31 - G30;
    F31 = G31 + G30;
#ifdef TRANS_PRECISION_EXT
    E4 = xTrRound( 49 * ai0[4] - 251 * ai0[28], DenShift32);
    E5 = xTrRound( 212 * ai0[20] - 142 * ai0[12], DenShift32);
    E6 = xTrRound( 212 * ai0[12] + 142 * ai0[20], DenShift32);
    E7 = xTrRound( 49 * ai0[28] + 251 * ai0[4], DenShift32);
#else
    E4 = xTrRound( 49 * pSrc[4] - 251 * pSrc[28], DenShift32);
    E5 = xTrRound( 212 * pSrc[20] - 142 * pSrc[12], DenShift32);
    E6 = xTrRound( 212 * pSrc[12] + 142 * pSrc[20], DenShift32);
    E7 = xTrRound( 49 * pSrc[28] + 251 * pSrc[4], DenShift32);
#endif
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
    E17 = xTrRound( 49 * F30 - 251 * F17, DenShift32);
    E18 = xTrRound(  - 251 * F29 - 49 * F18, DenShift32);
    E21 = xTrRound( 212 * F26 - 142 * F21, DenShift32);
    E22 = xTrRound(  - 142 * F25 - 212 * F22, DenShift32);
    E25 = xTrRound( 212 * F25 - 142 * F22, DenShift32);
    E26 = xTrRound( 142 * F26 + 212 * F21, DenShift32);
    E29 = xTrRound( 49 * F29 - 251 * F18, DenShift32);
    E30 = xTrRound( 251 * F30 + 49 * F17, DenShift32);
#ifdef TRANS_PRECISION_EXT
    D0 = xTrRound( 181 * ( ai0[0] + ai0[16] ), DenShift32);
    D1 = xTrRound( 181 * ( ai0[0] - ai0[16] ), DenShift32);
    D2 = xTrRound( 97 * ai0[8] - 236 * ai0[24], DenShift32);
    D3 = xTrRound( 236 * ai0[8] + 97 * ai0[24], DenShift32);
#else
    D0 = xTrRound( 181 * ( pSrc[0] + pSrc[16] ), DenShift32);
    D1 = xTrRound( 181 * ( pSrc[0] - pSrc[16] ), DenShift32);
    D2 = xTrRound( 97 * pSrc[8] - 236 * pSrc[24], DenShift32);
    D3 = xTrRound( 236 * pSrc[8] + 97 * pSrc[24], DenShift32);
#endif
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 97 * E14 - 236 * E9, DenShift32);
    D10 = xTrRound(  - 236 * E13 - 97 * E10, DenShift32);
    D13 = xTrRound( 97 * E13 - 236 * E10, DenShift32);
    D14 = xTrRound( 236 * E14 + 97 * E9, DenShift32);
    D16 = F16 + F19;
    D19 = F16 - F19;
    D20 = F23 - F20;
    D23 = F23 + F20;
    D24 = F24 + F27;
    D27 = F24 - F27;
    D28 = F31 - F28;
    D31 = F31 + F28;
    D17 = E17 + E18;
    D18 = E17 - E18;
    D21 = E22 - E21;
    D22 = E22 + E21;
    D25 = E25 + E26;
    D26 = E25 - E26;
    D29 = E30 - E29;
    D30 = E30 + E29;
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 181 * ( D6 - D5 ), DenShift32);
    C6 = xTrRound( 181 * ( D6 + D5 ), DenShift32);
    C18 = xTrRound( 97 * D29 - 236 * D18, DenShift32);
    C20 = xTrRound(  - 236 * D27 - 97 * D20, DenShift32);
    C26 = xTrRound(  - 236 * D21 + 97 * D26, DenShift32);
    C28 = xTrRound( 97 * D19 + 236 * D28, DenShift32);
    C19 = xTrRound( 97 * D28 - 236 * D19, DenShift32);
    C21 = xTrRound(  - 236 * D26 - 97 * D21, DenShift32);
    C27 = xTrRound(  - 236 * D20 + 97 * D27, DenShift32);
    C29 = xTrRound( 97 * D18 + 236 * D29, DenShift32);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 181 * ( C13 - C10 ), DenShift32);
    B13 = xTrRound( 181 * ( C13 + C10 ), DenShift32);
    B11 = xTrRound( 181 * ( C12 - C11 ), DenShift32);
    B12 = xTrRound( 181 * ( C12 + C11 ), DenShift32);
    B16 = D16 + D23;
    B23 = D16 - D23;
    B24 = D31 - D24;
    B31 = D31 + D24;
    B17 = D17 + D22;
    B22 = D17 - D22;
    B25 = D30 - D25;
    B30 = D30 + D25;
    B18 = C18 + C21;
    B21 = C18 - C21;
    B26 = C29 - C26;
    B29 = C29 + C26;
    B19 = C19 + C20;
    B20 = C19 - C20;
    B27 = C28 - C27;
    B28 = C28 + C27;
    
    A0 = B0 + C15;
    A15 = B0 - C15;
    A1 = B1 + C14;
    A14 = B1 - C14;
    A2 = B2 + B13;
    A13 = B2 - B13;
    A3 = B3 + B12;
    A12 = B3 - B12;
    A4 = B4 + B11;
    A11 = B4 - B11;
    A5 = B5 + B10;
    A10 = B5 - B10;
    A6 = B6 + C9;
    A9 = B6 - C9;
    A7 = B7 + C8;
    A8 = B7 - C8;
    A20 = xTrRound( 181 * ( B27 - B20 ), DenShift32);
    A27 = xTrRound( 181 * ( B27 + B20 ), DenShift32);
    A21 = xTrRound( 181 * ( B26 - B21 ), DenShift32);
    A26 = xTrRound( 181 * ( B26 + B21 ), DenShift32);
    A22 = xTrRound( 181 * ( B25 - B22 ), DenShift32);
    A25 = xTrRound( 181 * ( B25 + B22 ), DenShift32);
    A23 = xTrRound( 181 * ( B24 - B23 ), DenShift32);
    A24 = xTrRound( 181 * ( B24 + B23 ), DenShift32);
    
    aaiTemp[0][y] = A0 + B31;
    aaiTemp[31][y] = A0 - B31;
    aaiTemp[1][y] = A1 + B30;
    aaiTemp[30][y] = A1 - B30;
    aaiTemp[2][y] = A2 + B29;
    aaiTemp[29][y] = A2 - B29;
    aaiTemp[3][y] = A3 + B28;
    aaiTemp[28][y] = A3 - B28;
    aaiTemp[4][y] = A4 + A27;
    aaiTemp[27][y] = A4 - A27;
    aaiTemp[5][y] = A5 + A26;
    aaiTemp[26][y] = A5 - A26;
    aaiTemp[6][y] = A6 + A25;
    aaiTemp[25][y] = A6 - A25;
    aaiTemp[7][y] = A7 + A24;
    aaiTemp[24][y] = A7 - A24;
    aaiTemp[8][y] = A8 + A23;
    aaiTemp[23][y] = A8 - A23;
    aaiTemp[9][y] = A9 + A22;
    aaiTemp[22][y] = A9 - A22;
    aaiTemp[10][y] = A10 + A21;
    aaiTemp[21][y] = A10 - A21;
    aaiTemp[11][y] = A11 + A20;
    aaiTemp[20][y] = A11 - A20;
    aaiTemp[12][y] = A12 + B19;
    aaiTemp[19][y] = A12 - B19;
    aaiTemp[13][y] = A13 + B18;
    aaiTemp[18][y] = A13 - B18;
    aaiTemp[14][y] = A14 + B17;
    aaiTemp[17][y] = A14 - B17;
    aaiTemp[15][y] = A15 + B16;
    aaiTemp[16][y] = A15 - B16;
    
    pSrc += 32;
  }
  
  for( x=0 ; x<32 ; x++, pDes++ )
  {
    G16 = xTrRound( 12 * aaiTemp[x][1] - 255 * aaiTemp[x][31], DenShift32);
    G17 = xTrRound( 189 * aaiTemp[x][17] - 171 * aaiTemp[x][15], DenShift32);
    G18 = xTrRound( 109 * aaiTemp[x][9] - 231 * aaiTemp[x][23], DenShift32);
    G19 = xTrRound( 241 * aaiTemp[x][25] - 86 * aaiTemp[x][7], DenShift32);
    G20 = xTrRound( 62 * aaiTemp[x][5] - 248 * aaiTemp[x][27], DenShift32);
    G21 = xTrRound( 219 * aaiTemp[x][21] - 131 * aaiTemp[x][11], DenShift32);
    G22 = xTrRound( 152 * aaiTemp[x][13] - 205 * aaiTemp[x][19], DenShift32);
    G23 = xTrRound( 253 * aaiTemp[x][29] - 37 * aaiTemp[x][3], DenShift32);
    G24 = xTrRound( 253 * aaiTemp[x][3] + 37 * aaiTemp[x][29], DenShift32);
    G25 = xTrRound( 152 * aaiTemp[x][19] + 205 * aaiTemp[x][13], DenShift32);
    G26 = xTrRound( 219 * aaiTemp[x][11] + 131 * aaiTemp[x][21], DenShift32);
    G27 = xTrRound( 62 * aaiTemp[x][27] + 248 * aaiTemp[x][5], DenShift32);
    G28 = xTrRound( 241 * aaiTemp[x][7] + 86 * aaiTemp[x][25], DenShift32);
    G29 = xTrRound( 109 * aaiTemp[x][23] + 231 * aaiTemp[x][9], DenShift32);
    G30 = xTrRound( 189 * aaiTemp[x][15] + 171 * aaiTemp[x][17], DenShift32);
    G31 = xTrRound( 12 * aaiTemp[x][31] + 255 * aaiTemp[x][1], DenShift32);
    
    F8 = xTrRound( 25 * aaiTemp[x][2] - 254 * aaiTemp[x][30], DenShift32);
    F9 = xTrRound( 197 * aaiTemp[x][18] - 162 * aaiTemp[x][14], DenShift32);
    F10 = xTrRound( 120 * aaiTemp[x][10] - 225 * aaiTemp[x][22], DenShift32);
    F11 = xTrRound( 244 * aaiTemp[x][26] - 74 * aaiTemp[x][6], DenShift32);
    F12 = xTrRound( 244 * aaiTemp[x][6] + 74 * aaiTemp[x][26], DenShift32);
    F13 = xTrRound( 120 * aaiTemp[x][22] + 225 * aaiTemp[x][10], DenShift32);
    F14 = xTrRound( 197 * aaiTemp[x][14] + 162 * aaiTemp[x][18], DenShift32);
    F15 = xTrRound( 25 * aaiTemp[x][30] + 254 * aaiTemp[x][2], DenShift32);
    F16 = G16 + G17;
    F17 = G16 - G17;
    F18 = G19 - G18;
    F19 = G19 + G18;
    F20 = G20 + G21;
    F21 = G20 - G21;
    F22 = G23 - G22;
    F23 = G23 + G22;
    F24 = G24 + G25;
    F25 = G24 - G25;
    F26 = G27 - G26;
    F27 = G27 + G26;
    F28 = G28 + G29;
    F29 = G28 - G29;
    F30 = G31 - G30;
    F31 = G31 + G30;
    
    E4 = xTrRound( 49 * aaiTemp[x][4] - 251 * aaiTemp[x][28], DenShift32);
    E5 = xTrRound( 212 * aaiTemp[x][20] - 142 * aaiTemp[x][12], DenShift32);
    E6 = xTrRound( 212 * aaiTemp[x][12] + 142 * aaiTemp[x][20], DenShift32);
    E7 = xTrRound( 49 * aaiTemp[x][28] + 251 * aaiTemp[x][4], DenShift32);
    E8 = F8 + F9;
    E9 = F8 - F9;
    E10 = F11 - F10;
    E11 = F11 + F10;
    E12 = F12 + F13;
    E13 = F12 - F13;
    E14 = F15 - F14;
    E15 = F15 + F14;
    E17 = xTrRound( 49 * F30 - 251 * F17, DenShift32);
    E18 = xTrRound(  - 251 * F29 - 49 * F18, DenShift32);
    E21 = xTrRound( 212 * F26 - 142 * F21, DenShift32);
    E22 = xTrRound(  - 142 * F25 - 212 * F22, DenShift32);
    E25 = xTrRound( 212 * F25 - 142 * F22, DenShift32);
    E26 = xTrRound( 142 * F26 + 212 * F21, DenShift32);
    E29 = xTrRound( 49 * F29 - 251 * F18, DenShift32);
    E30 = xTrRound( 251 * F30 + 49 * F17, DenShift32);
    
    D0 = xTrRound( 181 * ( aaiTemp[x][0] + aaiTemp[x][16] ), DenShift32);
    D1 = xTrRound( 181 * ( aaiTemp[x][0] - aaiTemp[x][16] ), DenShift32);
    D2 = xTrRound( 97 * aaiTemp[x][8] - 236 * aaiTemp[x][24], DenShift32);
    D3 = xTrRound( 236 * aaiTemp[x][8] + 97 * aaiTemp[x][24], DenShift32);
    D4 = E4 + E5;
    D5 = E4 - E5;
    D6 = E7 - E6;
    D7 = E7 + E6;
    D9 = xTrRound( 97 * E14 - 236 * E9, DenShift32);
    D10 = xTrRound(  - 236 * E13 - 97 * E10, DenShift32);
    D13 = xTrRound( 97 * E13 - 236 * E10, DenShift32);
    D14 = xTrRound( 236 * E14 + 97 * E9, DenShift32);
    D16 = F16 + F19;
    D19 = F16 - F19;
    D20 = F23 - F20;
    D23 = F23 + F20;
    D24 = F24 + F27;
    D27 = F24 - F27;
    D28 = F31 - F28;
    D31 = F31 + F28;
    D17 = E17 + E18;
    D18 = E17 - E18;
    D21 = E22 - E21;
    D22 = E22 + E21;
    D25 = E25 + E26;
    D26 = E25 - E26;
    D29 = E30 - E29;
    D30 = E30 + E29;
    
    C0 = D0 + D3;
    C3 = D0 - D3;
    C8 = E8 + E11;
    C11 = E8 - E11;
    C12 = E15 - E12;
    C15 = E15 + E12;
    C1 = D1 + D2;
    C2 = D1 - D2;
    C9 = D9 + D10;
    C10 = D9 - D10;
    C13 = D14 - D13;
    C14 = D14 + D13;
    C5 = xTrRound( 181 * ( D6 - D5 ), DenShift32);
    C6 = xTrRound( 181 * ( D6 + D5 ), DenShift32);
    C18 = xTrRound( 97 * D29 - 236 * D18, DenShift32);
    C20 = xTrRound(  - 236 * D27 - 97 * D20, DenShift32);
    C26 = xTrRound(  - 236 * D21 + 97 * D26, DenShift32);
    C28 = xTrRound( 97 * D19 + 236 * D28, DenShift32);
    C19 = xTrRound( 97 * D28 - 236 * D19, DenShift32);
    C21 = xTrRound(  - 236 * D26 - 97 * D21, DenShift32);
    C27 = xTrRound(  - 236 * D20 + 97 * D27, DenShift32);
    C29 = xTrRound( 97 * D18 + 236 * D29, DenShift32);
    
    B0 = C0 + D7;
    B7 = C0 - D7;
    B1 = C1 + C6;
    B6 = C1 - C6;
    B2 = C2 + C5;
    B5 = C2 - C5;
    B3 = C3 + D4;
    B4 = C3 - D4;
    B10 = xTrRound( 181 * ( C13 - C10 ), DenShift32);
    B13 = xTrRound( 181 * ( C13 + C10 ), DenShift32);
    B11 = xTrRound( 181 * ( C12 - C11 ), DenShift32);
    B12 = xTrRound( 181 * ( C12 + C11 ), DenShift32);
    B16 = D16 + D23;
    B23 = D16 - D23;
    B24 = D31 - D24;
    B31 = D31 + D24;
    B17 = D17 + D22;
    B22 = D17 - D22;
    B25 = D30 - D25;
    B30 = D30 + D25;
    B18 = C18 + C21;
    B21 = C18 - C21;
    B26 = C29 - C26;
    B29 = C29 + C26;
    B19 = C19 + C20;
    B20 = C19 - C20;
    B27 = C28 - C27;
    B28 = C28 + C27;
    
    A0 = B0 + C15;
    A15 = B0 - C15;
    A1 = B1 + C14;
    A14 = B1 - C14;
    A2 = B2 + B13;
    A13 = B2 - B13;
    A3 = B3 + B12;
    A12 = B3 - B12;
    A4 = B4 + B11;
    A11 = B4 - B11;
    A5 = B5 + B10;
    A10 = B5 - B10;
    A6 = B6 + C9;
    A9 = B6 - C9;
    A7 = B7 + C8;
    A8 = B7 - C8;
    A20 = xTrRound( 181 * ( B27 - B20 ), DenShift32);
    A27 = xTrRound( 181 * ( B27 + B20 ), DenShift32);
    A21 = xTrRound( 181 * ( B26 - B21 ), DenShift32);
    A26 = xTrRound( 181 * ( B26 + B21 ), DenShift32);
    A22 = xTrRound( 181 * ( B25 - B22 ), DenShift32);
    A25 = xTrRound( 181 * ( B25 + B22 ), DenShift32);
    A23 = xTrRound( 181 * ( B24 - B23 ), DenShift32);
    A24 = xTrRound( 181 * ( B24 + B23 ), DenShift32);
    
    pDes[         0] = (Pel)xTrRound( A0 + B31 , DCore32Shift);
    pDes[uiStride31] = (Pel)xTrRound( A0 - B31 , DCore32Shift);
    pDes[uiStride  ] = (Pel)xTrRound( A1 + B30 , DCore32Shift);
    pDes[uiStride30] = (Pel)xTrRound( A1 - B30 , DCore32Shift);
    pDes[uiStride2 ] = (Pel)xTrRound( A2 + B29 , DCore32Shift);
    pDes[uiStride29] = (Pel)xTrRound( A2 - B29 , DCore32Shift);
    pDes[uiStride3 ] = (Pel)xTrRound( A3 + B28 , DCore32Shift);
    pDes[uiStride28] = (Pel)xTrRound( A3 - B28 , DCore32Shift);
    pDes[uiStride4 ] = (Pel)xTrRound( A4 + A27 , DCore32Shift);
    pDes[uiStride27] = (Pel)xTrRound( A4 - A27 , DCore32Shift);
    pDes[uiStride5 ] = (Pel)xTrRound( A5 + A26 , DCore32Shift);
    pDes[uiStride26] = (Pel)xTrRound( A5 - A26 , DCore32Shift);
    pDes[uiStride6 ] = (Pel)xTrRound( A6 + A25 , DCore32Shift);
    pDes[uiStride25] = (Pel)xTrRound( A6 - A25 , DCore32Shift);
    pDes[uiStride7 ] = (Pel)xTrRound( A7 + A24 , DCore32Shift);
    pDes[uiStride24] = (Pel)xTrRound( A7 - A24 , DCore32Shift);
    pDes[uiStride8 ] = (Pel)xTrRound( A8 + A23 , DCore32Shift);
    pDes[uiStride23] = (Pel)xTrRound( A8 - A23 , DCore32Shift);
    pDes[uiStride9 ] = (Pel)xTrRound( A9 + A22 , DCore32Shift);
    pDes[uiStride22] = (Pel)xTrRound( A9 - A22 , DCore32Shift);
    pDes[uiStride10] = (Pel)xTrRound( A10 + A21 , DCore32Shift);
    pDes[uiStride21] = (Pel)xTrRound( A10 - A21 , DCore32Shift);
    pDes[uiStride11] = (Pel)xTrRound( A11 + A20 , DCore32Shift);
    pDes[uiStride20] = (Pel)xTrRound( A11 - A20 , DCore32Shift);
    pDes[uiStride12] = (Pel)xTrRound( A12 + B19 , DCore32Shift);
    pDes[uiStride19] = (Pel)xTrRound( A12 - B19 , DCore32Shift);
    pDes[uiStride13] = (Pel)xTrRound( A13 + B18 , DCore32Shift);
    pDes[uiStride18] = (Pel)xTrRound( A13 - B18 , DCore32Shift);
    pDes[uiStride14] = (Pel)xTrRound( A14 + B17 , DCore32Shift);
    pDes[uiStride17] = (Pel)xTrRound( A14 - B17 , DCore32Shift);
    pDes[uiStride15] = (Pel)xTrRound( A15 + B16 , DCore32Shift);
    pDes[uiStride16] = (Pel)xTrRound( A15 - B16 , DCore32Shift);
    
#ifdef TRANS_PRECISION_EXT
    pDes[        0 ] =  (pDes[        0 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride  ] =  (pDes[uiStride  ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride2 ] =  (pDes[uiStride2 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride3 ] =  (pDes[uiStride3 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride4 ] =  (pDes[uiStride4 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride5 ] =  (pDes[uiStride5 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride6 ] =  (pDes[uiStride6 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride7 ] =  (pDes[uiStride7 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride8 ] =  (pDes[uiStride8 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride9 ] =  (pDes[uiStride9 ]+offset)>>uiBitDepthIncrease;
    pDes[uiStride10] =  (pDes[uiStride10]+offset)>>uiBitDepthIncrease;
    pDes[uiStride11] =  (pDes[uiStride11]+offset)>>uiBitDepthIncrease;
    pDes[uiStride12] =  (pDes[uiStride12]+offset)>>uiBitDepthIncrease;
    pDes[uiStride13] =  (pDes[uiStride13]+offset)>>uiBitDepthIncrease;
    pDes[uiStride14] =  (pDes[uiStride14]+offset)>>uiBitDepthIncrease;
    pDes[uiStride15] =  (pDes[uiStride15]+offset)>>uiBitDepthIncrease;
    pDes[uiStride16] =  (pDes[uiStride16]+offset)>>uiBitDepthIncrease;
    pDes[uiStride17] =  (pDes[uiStride17]+offset)>>uiBitDepthIncrease;
    pDes[uiStride18] =  (pDes[uiStride18]+offset)>>uiBitDepthIncrease;
    pDes[uiStride19] =  (pDes[uiStride19]+offset)>>uiBitDepthIncrease;
    pDes[uiStride20] =  (pDes[uiStride20]+offset)>>uiBitDepthIncrease;
    pDes[uiStride21] =  (pDes[uiStride21]+offset)>>uiBitDepthIncrease;
    pDes[uiStride22] =  (pDes[uiStride22]+offset)>>uiBitDepthIncrease;
    pDes[uiStride23] =  (pDes[uiStride23]+offset)>>uiBitDepthIncrease;
    pDes[uiStride24] =  (pDes[uiStride24]+offset)>>uiBitDepthIncrease;
    pDes[uiStride25] =  (pDes[uiStride25]+offset)>>uiBitDepthIncrease;
    pDes[uiStride26] =  (pDes[uiStride26]+offset)>>uiBitDepthIncrease;
    pDes[uiStride27] =  (pDes[uiStride27]+offset)>>uiBitDepthIncrease;
    pDes[uiStride28] =  (pDes[uiStride28]+offset)>>uiBitDepthIncrease;
    pDes[uiStride29] =  (pDes[uiStride29]+offset)>>uiBitDepthIncrease;
    pDes[uiStride30] =  (pDes[uiStride30]+offset)>>uiBitDepthIncrease;
    pDes[uiStride31] =  (pDes[uiStride31]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::init( UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxTrSize, Int iSymbolMode, UInt *aTableLP4, UInt *aTableLP8, Bool bUseRDOQ,  Bool bEnc )
{
  m_uiMaxTrSize  = uiMaxTrSize;
  m_bEnc         = bEnc;
  m_bUseRDOQ     = bUseRDOQ;
  m_uiLPTableE8 = aTableLP8;
  m_uiLPTableE4 = aTableLP4;
  m_iSymbolMode = iSymbolMode;
  
  if ( m_bEnc )
  {
    m_cQP.initOffsetParam( MIN_QP, MAX_QP );
  }
}

Void TComTrQuant::xQuant( TComDataCU* pcCU, Long* pSrc, TCoeff*& pDes, Int iWidth, Int iHeight, UInt& uiAcSum, TextType eTType, UInt uiAbsPartIdx )
{
  xQuantLTR(pcCU, pSrc, pDes, iWidth, iHeight, uiAcSum, eTType, uiAbsPartIdx );
}

Void TComTrQuant::xDeQuant( TCoeff* pSrc, Long*& pDes, Int iWidth, Int iHeight )
{
  xDeQuantLTR( pSrc, pDes, iWidth, iHeight );
}

Void TComTrQuant::transformNxN( TComDataCU* pcCU, Pel* pcResidual, UInt uiStride, TCoeff*& rpcCoeff, UInt uiWidth, UInt uiHeight, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  uiAbsSum = 0;
  
  assert( (pcCU->getSlice()->getSPS()->getMaxTrSize() >= uiWidth) );
  
  xT( pcResidual, uiStride, m_plTempCoeff, uiWidth );
  xQuant( pcCU, m_plTempCoeff, rpcCoeff, uiWidth, uiHeight, uiAbsSum, eTType, uiAbsPartIdx );
}

Void TComTrQuant::invtransformNxN( Pel*& rpcResidual, UInt uiStride, TCoeff* pcCoeff, UInt uiWidth, UInt uiHeight )
{
  xDeQuant( pcCoeff, m_plTempCoeff, uiWidth, uiHeight );
  xIT( m_plTempCoeff, rpcResidual, uiStride, uiWidth );
}

Void TComTrQuant::xT2( Pel* piBlkResi, UInt uiStride, Long* psCoeff )
{
  Int itmp1, itmp2;
  
  itmp1 = piBlkResi[0] + piBlkResi[uiStride  ];
  itmp2 = piBlkResi[1] + piBlkResi[uiStride+1];
  
  psCoeff[0] = itmp1 + itmp2;
  psCoeff[1] = itmp1 - itmp2;
  
  itmp1 = piBlkResi[0] - piBlkResi[uiStride  ];
  itmp2 = piBlkResi[1] - piBlkResi[uiStride+1];
  
  psCoeff[2] = itmp1 + itmp2;
  psCoeff[3] = itmp1 - itmp2;
}

Void TComTrQuant::xT4( Pel* piBlkResi, UInt uiStride, Long* psCoeff )
{
  Int aai[4][4];
  Int tmp1, tmp2;
  
  for( Int y = 0; y < 4; y++ )
  {
    tmp1 = piBlkResi[0] + piBlkResi[3];
    tmp2 = piBlkResi[1] + piBlkResi[2];
    
    aai[0][y] = tmp1 + tmp2;
    aai[2][y] = tmp1 - tmp2;
    
    tmp1 = piBlkResi[0] - piBlkResi[3];
    tmp2 = piBlkResi[1] - piBlkResi[2];
    
    aai[1][y] = tmp1 * 2 + tmp2 ;
    aai[3][y] = tmp1  - tmp2 * 2;
    piBlkResi += uiStride;
  }
  
  for( Int x = 0; x < 4; x++, psCoeff++ )
  {
    tmp1 = aai[x][0] + aai[x][3];
    tmp2 = aai[x][1] + aai[x][2];
    
    psCoeff[0] = tmp1 + tmp2;
    psCoeff[8] = tmp1 - tmp2;
    
    tmp1 = aai[x][0] - aai[x][3];
    tmp2 = aai[x][1] - aai[x][2];
    
    psCoeff[4]  = tmp1 * 2 + tmp2;
    psCoeff[12] = tmp1 - tmp2 * 2;
  }
}

Void TComTrQuant::xT8( Pel* piBlkResi, UInt uiStride, Long* psCoeff )
{
  Int aai[8][8];
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift8x8-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  for( Int i = 0; i < 8; i++, piBlkResi += uiStride )
  {
    Int ai1 [8];
    Int ai2 [8];
#ifdef TRANS_PRECISION_EXT
    ai1[0] = (piBlkResi[0] + piBlkResi[7])<<uiBitDepthIncrease;
    ai1[1] = (piBlkResi[1] + piBlkResi[6])<<uiBitDepthIncrease;
    ai1[2] = (piBlkResi[2] + piBlkResi[5])<<uiBitDepthIncrease;
    ai1[3] = (piBlkResi[3] + piBlkResi[4])<<uiBitDepthIncrease;
    
    ai1[4] = (piBlkResi[0] - piBlkResi[7])<<uiBitDepthIncrease;
    ai1[5] = (piBlkResi[1] - piBlkResi[6])<<uiBitDepthIncrease;
    ai1[6] = (piBlkResi[2] - piBlkResi[5])<<uiBitDepthIncrease;
    ai1[7] = (piBlkResi[3] - piBlkResi[4])<<uiBitDepthIncrease;
#else
    ai1[0] = piBlkResi[0] + piBlkResi[7];
    ai1[1] = piBlkResi[1] + piBlkResi[6];
    ai1[2] = piBlkResi[2] + piBlkResi[5];
    ai1[3] = piBlkResi[3] + piBlkResi[4];
    
    ai1[4] = piBlkResi[0] - piBlkResi[7];
    ai1[5] = piBlkResi[1] - piBlkResi[6];
    ai1[6] = piBlkResi[2] - piBlkResi[5];
    ai1[7] = piBlkResi[3] - piBlkResi[4];
#endif
    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);
    
    aai[0][i] =  ai2[0]     +  ai2[1];
    aai[2][i] =  ai2[2]     + (ai2[3]>>1);
    aai[4][i] =  ai2[0]     -  ai2[1];
    aai[6][i] = (ai2[2]>>1) -  ai2[3];
    
    aai[1][i] =  ai2[4]     + (ai2[7]>>2);
    aai[3][i] =  ai2[5]     + (ai2[6]>>2);
    aai[5][i] =  ai2[6]     - (ai2[5]>>2);
    aai[7][i] = (ai2[4]>>2) -  ai2[7];
  }
  
  // vertical transform
  for( Int n = 0; n < 8; n++, psCoeff++)
  {
    Int ai1[8];
    Int ai2[8];
    
    ai1[0] = aai[n][0] + aai[n][7];
    ai1[1] = aai[n][1] + aai[n][6];
    ai1[2] = aai[n][2] + aai[n][5];
    ai1[3] = aai[n][3] + aai[n][4];
    ai1[4] = aai[n][0] - aai[n][7];
    ai1[5] = aai[n][1] - aai[n][6];
    ai1[6] = aai[n][2] - aai[n][5];
    ai1[7] = aai[n][3] - aai[n][4];
    
    ai2[0] = ai1[0] + ai1[3];
    ai2[1] = ai1[1] + ai1[2];
    ai2[2] = ai1[0] - ai1[3];
    ai2[3] = ai1[1] - ai1[2];
    ai2[4] = ai1[5] + ai1[6] + ((ai1[4]>>1) + ai1[4]);
    ai2[5] = ai1[4] - ai1[7] - ((ai1[6]>>1) + ai1[6]);
    ai2[6] = ai1[4] + ai1[7] - ((ai1[5]>>1) + ai1[5]);
    ai2[7] = ai1[5] - ai1[6] + ((ai1[7]>>1) + ai1[7]);
    
    psCoeff[ 0] =  ai2[0]     +  ai2[1];
    psCoeff[16] =  ai2[2]     + (ai2[3]>>1);
    psCoeff[32] =  ai2[0]     -  ai2[1];
    psCoeff[48] = (ai2[2]>>1) -  ai2[3];
    
    psCoeff[ 8] =  ai2[4]     + (ai2[7]>>2);
    psCoeff[24] =  ai2[5]     + (ai2[6]>>2);
    psCoeff[40] =  ai2[6]     - (ai2[5]>>2);
    psCoeff[56] = (ai2[4]>>2) -  ai2[7];
#ifdef TRANS_PRECISION_EXT
    psCoeff[ 0] =  (psCoeff[ 0]+offset)>>uiBitDepthIncrease;
    psCoeff[16] =  (psCoeff[16]+offset)>>uiBitDepthIncrease;
    psCoeff[32] =  (psCoeff[32]+offset)>>uiBitDepthIncrease;
    psCoeff[48] =  (psCoeff[48]+offset)>>uiBitDepthIncrease;
    
    psCoeff[ 8] =  (psCoeff[ 8]+offset)>>uiBitDepthIncrease;
    psCoeff[24] =  (psCoeff[24]+offset)>>uiBitDepthIncrease;
    psCoeff[40] =  (psCoeff[40]+offset)>>uiBitDepthIncrease;
    psCoeff[56] =  (psCoeff[56]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::xQuant2x2( Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum )
{
  Int iLevel;
  Int iSign;
  Int iOffset = 1<<(6+m_cQP.per());
  Int iBits   = m_cQP.per()+7;
  
  iSign       = plSrcCoef[0]>>31;
  iLevel      = abs(plSrcCoef[0]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[0] = iLevel;
  }
  else
  {
    pDstCoef[0] = 0;
  }
  
  iSign       = plSrcCoef[1]>>31;
  iLevel      = abs(plSrcCoef[1]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[1] = iLevel;
  }
  else
  {
    pDstCoef[1] = 0;
  }
  
  iSign       = plSrcCoef[2]>>31;
  iLevel      = abs(plSrcCoef[2]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[2] = iLevel;
  }
  else
  {
    pDstCoef[2] = 0;
  }
  
  iSign       = plSrcCoef[3]>>31;
  iLevel      = abs(plSrcCoef[3]);
  iLevel     *= m_puiQuantMtx[0];
  iLevel      = (iLevel + iOffset)>>iBits;
  
  if( iLevel != 0)
  {
    uiAbsSum   += iLevel;
    iLevel     ^= iSign;
    iLevel     -= iSign;
    pDstCoef[3] = iLevel;
  }
  else
  {
    pDstCoef[3] = 0;
  }
}

Void TComTrQuant::xQuant4x4( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, plSrcCoef, pDstCoef, 4, 4, uiAbsSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant(pcCU, plSrcCoef, pDstCoef, 4, 4, uiAbsSum, eTType, uiAbsPartIdx );
  }
  else
  {
    for( Int n = 0; n < 16; n++ )
    {
      Int iLevel, iSign;
      iLevel  = plSrcCoef[n];
      iSign   = iLevel;
      iLevel  = abs( iLevel ) * m_puiQuantMtx[n];
      
      iLevel      = ( iLevel + m_cQP.m_iAdd4x4 ) >> m_cQP.m_iBits;
      
      if( 0 != iLevel )
      {
        iSign     >>= 31;
        uiAbsSum   += iLevel;
        iLevel     ^= iSign;
        iLevel     -= iSign;
        pDstCoef[n] = iLevel;
      }
      else
      {
        pDstCoef [n] = 0;
      }
    }
  }
}

Void TComTrQuant::xQuant8x8( TComDataCU* pcCU, Long* plSrcCoef, TCoeff*& pDstCoef, UInt& uiAbsSum, TextType eTType, UInt uiAbsPartIdx )
{
  Int iBit = m_cQP.m_iBits + 1;
  
  if ( m_bUseRDOQ && (eTType == TEXT_LUMA || RDOQ_CHROMA) )
  {
    if ( m_iSymbolMode == 0)
      xRateDistOptQuant_LCEC(pcCU, plSrcCoef, pDstCoef, 8, 8, uiAbsSum, eTType, uiAbsPartIdx );
    else
      xRateDistOptQuant(pcCU, plSrcCoef, pDstCoef, 8, 8, uiAbsSum, eTType, uiAbsPartIdx );
  }
  else
  {
    for( Int n = 0; n < 64; n++ )
    {
      Int iLevel, iSign;
      
      iLevel  = plSrcCoef[n];
      iSign   = iLevel;
      iLevel  = abs( iLevel ) * m_puiQuantMtx[n];
      
      iLevel      = ( iLevel + m_cQP.m_iAdd8x8 ) >> iBit;
      
      if( 0 != iLevel )
      {
        iSign     >>= 31;
        uiAbsSum   += iLevel;
        iLevel     ^= iSign;
        iLevel     -= iSign;
        pDstCoef[n] = iLevel;
      }
      else
      {
        pDstCoef [n] = 0;
      }
    }
  }
}

Void TComTrQuant::xIT2( Long* plCoef, Pel* pResidual, UInt uiStride )
{
  Int itemp, itmp1, itmp2;
  Int iSign;
  UInt uiBits = 5;
  UInt uiOffset = 1<<(uiBits-1);
  
  itmp1 = plCoef[0] + plCoef[2];
  itmp2 = plCoef[1] + plCoef[3];
  
  itemp = itmp1 + itmp2;
  iSign = itemp>>31;
  pResidual[0] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[0] ^= iSign;
  pResidual[0] -= iSign;
  
  itemp = itmp1 - itmp2;
  iSign = itemp>>31;
  pResidual[1] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[1] ^= iSign;
  pResidual[1] -= iSign;
  
  itmp1 = plCoef[0] - plCoef[2];
  itmp2 = plCoef[1] - plCoef[3];
  
  itemp = itmp1 + itmp2;
  iSign = itemp>>31;
  pResidual[uiStride] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[uiStride] ^= iSign;
  pResidual[uiStride] -= iSign;
  
  itemp = itmp1 - itmp2;
  iSign = itemp>>31;
  pResidual[uiStride+1] = (abs(itemp) + uiOffset)>>uiBits;
  pResidual[uiStride+1] ^= iSign;
  pResidual[uiStride+1] -= iSign;
}

Void TComTrQuant::xIT4( Long* plCoef, Pel* pResidual, UInt uiStride )
{
  Int aai[4][4];
  Int tmp1, tmp2;
  Int x, y;
  Int uiStride2=(uiStride<<1);
  Int uiStride3=uiStride2 + uiStride;
  
  for( x = 0; x < 4; x++, plCoef+=4 )
  {
    tmp1 = plCoef[0] + plCoef[2];
    tmp2 = (plCoef[3]>>1) + plCoef[1];
    
    aai[0][x] = tmp1 + tmp2;
    aai[3][x] = tmp1 - tmp2;
    
    tmp1 = plCoef[0] - plCoef[2];
    tmp2 = (plCoef[1]>>1) - plCoef[3];
    
    aai[1][x] = tmp1 + tmp2;
    aai[2][x] = tmp1 - tmp2;
  }
  
  for( y = 0; y < 4; y++, pResidual ++ )
  {
    tmp1 =  aai[y][0] + aai[y][2];
    tmp2 = (aai[y][3]>>1) + aai[y][1];
    
    pResidual[0]         =  xRound( tmp1 + tmp2);
    pResidual[uiStride3] =  xRound( tmp1 - tmp2);
    
    tmp1 =  aai[y][0] - aai[y][2];
    tmp2 = (aai[y][1]>>1) - aai[y][3];
    
    pResidual[uiStride]  =  xRound( tmp1 + tmp2);
    pResidual[uiStride2] =  xRound( tmp1 - tmp2);
  }
}

Void TComTrQuant::xIT8( Long* plCoef, Pel* pResidual, UInt uiStride )
{
  Long aai[8][8];
  Int n;
#ifdef TRANS_PRECISION_EXT
  Int uiBitDepthIncrease=g_iShift8x8-g_uiBitIncrement;
  Int offset = (uiBitDepthIncrease==0)? 0:(1<<(uiBitDepthIncrease-1));
#endif
  UInt uiStride2 = uiStride<<1;
  UInt uiStride3 = uiStride2 + uiStride;
  UInt uiStride4 = uiStride3 + uiStride;
  UInt uiStride5 = uiStride4 + uiStride;
  UInt uiStride6 = uiStride5 + uiStride;
  UInt uiStride7 = uiStride6 + uiStride;
  
  for( n = 0; n < 8; n++ )
  {
    Long* pi = plCoef + (n<<3);
    Long     ai1[8];
    Long     ai2[8];
#ifdef TRANS_PRECISION_EXT
    Long     ai0[8];
    ai0[0] =  pi[0]<<uiBitDepthIncrease;
    ai0[1] =  pi[1]<<uiBitDepthIncrease;
    ai0[2] =  pi[2]<<uiBitDepthIncrease;
    ai0[3] =  pi[3]<<uiBitDepthIncrease;
    ai0[4] =  pi[4]<<uiBitDepthIncrease;
    ai0[5] =  pi[5]<<uiBitDepthIncrease;
    ai0[6] =  pi[6]<<uiBitDepthIncrease;
    ai0[7] =  pi[7]<<uiBitDepthIncrease;
    ai1[0] = ai0[0] + ai0[4];
    ai1[2] = ai0[0] - ai0[4];
    
    ai1[4] = (ai0[2]>>1) -  ai0[6];
    ai1[6] =  ai0[2]     + (ai0[6]>>1);
    
    ai1[1] = ai0[5] - ai0[3] - ai0[7] - (ai0[7]>>1);
    ai1[3] = ai0[1] + ai0[7] - ai0[3] - (ai0[3]>>1);;
    ai1[5] = ai0[7] - ai0[1] + ai0[5] + (ai0[5]>>1);
    ai1[7] = ai0[3] + ai0[5] + ai0[1] + (ai0[1]>>1);
#else
    ai1[0] = pi[0] + pi[4];
    ai1[2] = pi[0] - pi[4];
    
    ai1[4] = (pi[2]>>1) -  pi[6];
    ai1[6] =  pi[2]     + (pi[6]>>1);
    
    ai1[1] = pi[5] - pi[3] - pi[7] - (pi[7]>>1);
    ai1[3] = pi[1] + pi[7] - pi[3] - (pi[3]>>1);;
    ai1[5] = pi[7] - pi[1] + pi[5] + (pi[5]>>1);
    ai1[7] = pi[3] + pi[5] + pi[1] + (pi[1]>>1);
#endif
    ai2[0] = ai1[0] + ai1[6];
    ai2[6] = ai1[0] - ai1[6];
    
    ai2[2] = ai1[2] + ai1[4];
    ai2[4] = ai1[2] - ai1[4];
    
    ai2[1] = ai1[1] + (ai1[7]>>2);
    ai2[7] = ai1[7] - (ai1[1]>>2);
    
    ai2[3] =  ai1[3]     + (ai1[5]>>2);
    ai2[5] = (ai1[3]>>2) -  ai1[5];
    
    aai[n][0] = ai2[0] + ai2[7];
    aai[n][1] = ai2[2] + ai2[5];
    aai[n][2] = ai2[4] + ai2[3];
    aai[n][3] = ai2[6] + ai2[1];
    aai[n][4] = ai2[6] - ai2[1];
    aai[n][5] = ai2[4] - ai2[3];
    aai[n][6] = ai2[2] - ai2[5];
    aai[n][7] = ai2[0] - ai2[7];
  }
  
  for( n = 0; n < 8; n++, pResidual++ )
  {
    Int ai1[8];
    Int ai2[8];
    
    ai1[0] =  aai[0][n]     +  aai[4][n];
    ai1[1] =  aai[5][n]     -  aai[3][n]     - aai[7][n] - (aai[7][n]>>1);
    ai1[2] =  aai[0][n]     -  aai[4][n];
    ai1[3] =  aai[1][n]     +  aai[7][n]     - aai[3][n] - (aai[3][n]>>1);
    ai1[4] = (aai[2][n]>>1) -  aai[6][n];
    ai1[5] =  aai[7][n]     -  aai[1][n]     + aai[5][n] + (aai[5][n]>>1);
    ai1[6] =  aai[2][n]     + (aai[6][n]>>1);
    ai1[7] =  aai[3][n]     +  aai[5][n]     + aai[1][n] + (aai[1][n]>>1);
    
    ai2[2] = ai1[2] + ai1[4];
    ai2[4] = ai1[2] - ai1[4];
    
    ai2[0] = ai1[0] + ai1[6];
    ai2[6] = ai1[0] - ai1[6];
    
    ai2[1] = ai1[1] + (ai1[7]>>2);
    ai2[7] = ai1[7] - (ai1[1]>>2);
    
    ai2[3] =  ai1[3]     + (ai1[5]>>2);
    ai2[5] = (ai1[3]>>2) -  ai1[5];
    
    pResidual[        0] = xRound( ai2[0] + ai2[7] );
    pResidual[uiStride ] = xRound( ai2[2] + ai2[5] );
    pResidual[uiStride2] = xRound( ai2[4] + ai2[3] );
    pResidual[uiStride3] = xRound( ai2[6] + ai2[1] );
    pResidual[uiStride4] = xRound( ai2[6] - ai2[1] );
    pResidual[uiStride5] = xRound( ai2[4] - ai2[3] );
    pResidual[uiStride6] = xRound( ai2[2] - ai2[5] );
    pResidual[uiStride7] = xRound( ai2[0] - ai2[7] );
    
#ifdef TRANS_PRECISION_EXT
    pResidual[        0] =  (pResidual[        0]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride ] =  (pResidual[uiStride ]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride2] =  (pResidual[uiStride2]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride3] =  (pResidual[uiStride3]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride4] =  (pResidual[uiStride4]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride5] =  (pResidual[uiStride5]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride6] =  (pResidual[uiStride6]+offset)>>uiBitDepthIncrease;
    pResidual[uiStride7] =  (pResidual[uiStride7]+offset)>>uiBitDepthIncrease;
#endif
  }
}

Void TComTrQuant::xDeQuant2x2( TCoeff* pSrcCoef, Long*& rplDstCoef )
{
  Int iDeScale = g_aiDequantCoef4[m_cQP.m_iRem];
  
  if( pSrcCoef[0] != 0 )
  {
    rplDstCoef[0] = pSrcCoef[0]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[0] = 0;
  }
  
  if( pSrcCoef[1] != 0 )
  {
    rplDstCoef[1] = pSrcCoef[1]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[1] = 0;
  }
  
  if( pSrcCoef[2] != 0 )
  {
    rplDstCoef[2] = pSrcCoef[2]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[2] = 0;
  }
  
  if( pSrcCoef[3] != 0 )
  {
    rplDstCoef[3] = pSrcCoef[3]*iDeScale<<m_cQP.per();
  }
  else
  {
    rplDstCoef[3] = 0;
  }
}

Void TComTrQuant::xDeQuant4x4( TCoeff* pSrcCoef, Long*& rplDstCoef )
{
  Int iLevel;
  Int iDeScale;
  
  for( Int n = 0; n < 16; n++ )
  {
    iLevel  = pSrcCoef[n];
    
    if( 0 != iLevel )
    {
      iDeScale = g_aiDequantCoef[m_cQP.m_iRem][n];
      
      rplDstCoef[n] = iLevel*iDeScale << m_cQP.m_iPer;
    }
    else
    {
      rplDstCoef[n] = 0;
    }
  }
}

Void TComTrQuant::xDeQuant8x8( TCoeff* pSrcCoef, Long*& rplDstCoef )
{
  Int iLevel;
  Int iDeScale;
  
  Int iAdd = ( 1 << 5 ) >> m_cQP.m_iPer;
  
  for( Int n = 0; n < 64; n++ )
  {
    iLevel  = pSrcCoef[n];
    
    if( 0 != iLevel )
    {
      iDeScale = g_aiDequantCoef64[m_cQP.m_iRem][n];
      rplDstCoef[n]   = ( (iLevel*iDeScale*16 + iAdd) << m_cQP.m_iPer ) >> 6;
    }
    else
    {
      rplDstCoef[n] = 0;
    }
  }
}

Void TComTrQuant::invRecurTransformNxN( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eTxt, Pel*& rpcResidual, UInt uiAddr, UInt uiStride, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TCoeff* rpcCoeff )
{
  if( !pcCU->getCbf(uiAbsPartIdx, eTxt, uiTrMode) )
    return;
  
  UInt uiLumaTrMode, uiChromaTrMode;
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
  const UInt uiStopTrMode = eTxt == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;
  
  assert(1); // as long as quadtrees are not used for residual transform
  
  if( uiTrMode == uiStopTrMode )
  {
    UInt uiDepth      = pcCU->getDepth( uiAbsPartIdx ) + uiTrMode;
    UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
    if( eTxt != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
      {
        return;
      }
      uiWidth  <<= 1;
      uiHeight <<= 1;
    }
    Pel* pResi = rpcResidual + uiAddr;
    invtransformNxN( pResi, uiStride, rpcCoeff, uiWidth, uiHeight );
  }
  else
  {
    uiTrMode++;
    uiWidth  >>= 1;
    uiHeight >>= 1;
    UInt uiAddrOffset = uiHeight * uiStride;
    UInt uiCoefOffset = uiWidth * uiHeight;
    UInt uiPartOffset = pcCU->getTotalNumPart() >> (uiTrMode<<1);
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr                         , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiWidth               , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset          , uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff ); rpcCoeff += uiCoefOffset; uiAbsPartIdx += uiPartOffset;
    invRecurTransformNxN( pcCU, uiAbsPartIdx, eTxt, rpcResidual, uiAddr + uiAddrOffset + uiWidth, uiStride, uiWidth, uiHeight, uiMaxTrMode, uiTrMode, rpcCoeff );
  }
}

// ------------------------------------------------------------------------------------------------
// Logical transform
// ------------------------------------------------------------------------------------------------

Void TComTrQuant::xT( Pel* piBlkResi, UInt uiStride, Long* psCoeff, Int iSize )
{
  switch( iSize )
  {
    case  2: xT2 ( piBlkResi, uiStride, psCoeff ); break;
    case  4: xT4 ( piBlkResi, uiStride, psCoeff ); break;
    case  8: xT8 ( piBlkResi, uiStride, psCoeff ); break;
    case 16: xT16( piBlkResi, uiStride, psCoeff ); break;
    case 32: xT32( piBlkResi, uiStride, psCoeff ); break;
    default: assert(0); break;
  }
}

Void TComTrQuant::xIT( Long* plCoef, Pel* pResidual, UInt uiStride, Int iSize )
{
  switch( iSize )
  {
    case  2: xIT2 ( plCoef, pResidual, uiStride ); break;
    case  4: xIT4 ( plCoef, pResidual, uiStride ); break;
    case  8: xIT8 ( plCoef, pResidual, uiStride ); break;
    case 16: xIT16( plCoef, pResidual, uiStride ); break;
    case 32: xIT32( plCoef, pResidual, uiStride ); break;
    default: assert(0); break;
  }
}

// RDOQ
Void TComTrQuant::xRateDistOptQuant                 ( TComDataCU*                     pcCU,
                                                     Long*                           plSrcCoeff,
                                                     TCoeff*&                        piDstCoeff,
                                                     UInt                            uiWidth,
                                                     UInt                            uiHeight,
                                                     UInt&                           uiAbsSum,
                                                     TextType                        eTType,
                                                     UInt                            uiAbsPartIdx )
{
  Bool   bExt8x8Flag = false;
  Bool   b64Flag     = false;
  Int    iQuantCoeff = 0;
  Int    iQpRem      = m_cQP.m_iRem;
  Int    iQBits      = m_cQP.m_iBits;
  Double dNormFactor = 0;
  Double dTemp       = 0;
  
  if( uiWidth == 4 && uiHeight == 4 )
  {
    dNormFactor = pow( 2., ( 2 * DQ_BITS + 19 ) );
    if( g_uiBitIncrement )
    {
      dNormFactor *=  1 << ( 2 * g_uiBitIncrement );
    }
    m_puiQuantMtx = &g_aiQuantCoef[ m_cQP.m_iRem ][ 0 ];
  }
  else if( uiWidth == 8 && uiHeight == 8 )
  {
    iQBits++;
    dNormFactor = pow( 2., ( 2 * Q_BITS_8 + 9 ) );
    if( g_uiBitIncrement )
    {
      dNormFactor *= 1 << ( 2 * g_uiBitIncrement );
    }
    m_puiQuantMtx = &g_aiQuantCoef64[ m_cQP.m_iRem ][ 0 ];
  }
  else if( uiWidth == 16 && uiHeight == 16 )
  {
    iQBits = ECore16Shift + m_cQP.per();
    dNormFactor = pow( 2., 21 );
    if( g_uiBitIncrement )
    {
      dNormFactor *=  1 << ( 2 * g_uiBitIncrement );
    }
    dTemp = estErr16x16[ iQpRem ] / dNormFactor;
    m_puiQuantMtx = ( &g_aiQuantCoef256[ m_cQP.m_iRem ][ 0 ] );
    bExt8x8Flag = true;
  }
  else if ( uiWidth == 32 && uiHeight == 32 )
  {
    iQBits = ECore32Shift + m_cQP.per();
    dNormFactor = pow( 2., 21 );
    if( g_uiBitIncrement )
    {
      dNormFactor *= 1 << ( 2 * g_uiBitIncrement );
    }
    dTemp = estErr32x32[ iQpRem ] / dNormFactor;
    m_puiQuantMtx = ( &g_aiQuantCoef1024[ m_cQP.m_iRem ][ 0 ] );
    bExt8x8Flag = true;
  }
  else
  {
    assert( 0 );
  }
  
  UInt       uiDownLeft          = 1;
  UInt       uiNumSigTopRight    = 0;
  UInt       uiNumSigBotLeft     = 0;
  UInt       uiMaxLineNum        = 0;
  Double     d64BlockUncodedCost = 0;
  const UInt uiLog2BlkSize       = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt uiBlkSizeM1         = ( 1 << uiLog2BlkSize ) - 1;
  const UInt uiMaxNumCoeff       = 1 << ( uiLog2BlkSize << 1 );
  const UInt uiNum4x4Blk         = max<UInt>( 1, uiMaxNumCoeff >> 4 );
  
  Int  piCoeff      [ MAX_CU_SIZE * MAX_CU_SIZE ];
  Long plLevelDouble[ MAX_CU_SIZE * MAX_CU_SIZE ];
  UInt puiOneCtx    [ MAX_CU_SIZE * MAX_CU_SIZE ];
  UInt puiAbsCtx    [ MAX_CU_SIZE * MAX_CU_SIZE ];
  UInt puiBaseCtx   [ ( MAX_CU_SIZE * MAX_CU_SIZE ) >> 4 ];
  
  
  ::memset( piDstCoeff,    0, sizeof(TCoeff) *   uiMaxNumCoeff        );
  ::memset( piCoeff,       0, sizeof(Int)    *   uiMaxNumCoeff        );
  ::memset( plLevelDouble, 0, sizeof(Long)   *   uiMaxNumCoeff        );
  ::memset( puiOneCtx,     0, sizeof(UInt)   *   uiMaxNumCoeff        );
  ::memset( puiAbsCtx,     0, sizeof(UInt)   *   uiMaxNumCoeff        );
  ::memset( puiBaseCtx,    0, sizeof(UInt)   * ( uiMaxNumCoeff >> 4 ) );
  
  //===== quantization =====
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
    UInt    uiBlkPos = g_auiSigLastScan[ uiLog2BlkSize ][ uiDownLeft ][ uiScanPos ];
    UInt    uiPosY   = uiBlkPos >> uiLog2BlkSize;
    UInt    uiPosX   = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    Long lLevelDouble = plSrcCoeff[ uiBlkPos ];
    
    if      ( uiWidth == 4 ) dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if ( uiWidth == 8 ) dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    
    lLevelDouble = abs( lLevelDouble * ( Long )( b64Flag ? iQuantCoeff : m_puiQuantMtx[ uiBlkPos ] ) );
    
    plLevelDouble[ uiBlkPos ] = lLevelDouble;
    UInt uiMaxAbsLevel = lLevelDouble >> iQBits;
    Bool bLowerInt     = ( ( lLevelDouble - Long( uiMaxAbsLevel << iQBits ) ) < Long( 1 << ( iQBits - 1 ) ) ) ? true : false;
    
    if( !bLowerInt )
    {
      uiMaxAbsLevel++;
    }
    
    Double dErr          = Double( lLevelDouble );
    d64BlockUncodedCost += dErr * dErr * dTemp;
    
    piCoeff[ uiBlkPos ] = plSrcCoeff[ uiBlkPos ] > 0 ? uiMaxAbsLevel : -Int( uiMaxAbsLevel );
    
    if ( uiMaxAbsLevel > 0 )
    {
      UInt uiLineNum = uiPosY + uiPosX;
      
      if( uiLineNum > uiMaxLineNum )
      {
        uiMaxLineNum = uiLineNum;
      }
      
      //----- update coeff counts -----
      if( uiPosX > uiPosY )
      {
        uiNumSigTopRight++;
      }
      else if( uiPosY > uiPosX )
      {
        uiNumSigBotLeft ++;
      }
    }
    
    //===== update scan direction =====
#if !HHI_DISABLE_SCAN
    if( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiBlkSizeM1 ) ) ||
       ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiBlkSizeM1 ) )   )
    {
      uiDownLeft = ( uiNumSigTopRight >= uiNumSigBotLeft ? 1 : 0 );
    }
#else
    if( uiScanPos && 
       ( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiBlkSizeM1 ) ) ||
        ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiBlkSizeM1 ) )   ) )
    {
      uiDownLeft = 1 - uiDownLeft;
    }
#endif
  }
  
  //===== estimate context models =====
  if ( uiNum4x4Blk > 1 )
  {
    Bool bFirstBlock  = true;
    UInt uiNumOne = 0;
    
    for( UInt uiSubBlk = 0; uiSubBlk < uiNum4x4Blk; uiSubBlk++ )
    {
      UInt uiCtxSet    = 0;
      UInt uiSubNumSig = 0;
      UInt uiSubPosX   = 0;
      UInt uiSubPosY   = 0;
      
      if( g_aucConvertToBit[ uiWidth ] > 1 )
      {
        uiSubPosX = g_auiFrameScanX[ g_aucConvertToBit[ uiWidth ] - 1 ][ uiSubBlk ] << 2;
        uiSubPosY = g_auiFrameScanY[ g_aucConvertToBit[ uiWidth ] - 1 ][ uiSubBlk ] << 2;
      }
      else
      {
        uiSubPosX = ( uiSubBlk < 2      ) ? 0 : 1;
        uiSubPosY = ( uiSubBlk % 2 == 0 ) ? 0 : 1;
        uiSubPosX = uiSubPosX << 2;
        uiSubPosY = uiSubPosY << 2;
      }
      
      Int* piCurr = &piCoeff[ uiSubPosX + uiSubPosY * uiWidth ];
      
      for( UInt uiY = 0; uiY < 4; uiY++ )
      {
        for( UInt uiX = 0; uiX < 4; uiX++ )
        {
          if( piCurr[ uiX ] )
          {
            uiSubNumSig++;
          }
        }
        piCurr += uiWidth;
      }
      
      if( uiSubNumSig > 0 )
      {
        Int c1 = 1;
        Int c2 = 0;
        UInt uiAbs  = 0;
        UInt uiSign = 0;
        
        if( bFirstBlock )
        {
          bFirstBlock = false;
          uiCtxSet = 5;
        }
        else
        {
          uiCtxSet = ( uiNumOne >> 2 ) + 1;
          uiNumOne = 0;
        }
        
        puiBaseCtx[ ( uiSubPosX >> 2 ) + ( uiSubPosY >> 2 ) * ( uiWidth >> 2 ) ] = uiCtxSet;
        
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos - ( uiPosY << 2 );
          UInt  uiIndex   = (uiSubPosY + uiPosY) * uiWidth + uiSubPosX + uiPosX;
          
          puiOneCtx[ uiIndex ] = min<UInt>( c1, 4 );
          puiAbsCtx[ uiIndex ] = min<UInt>( c2, 4 );
          
          if( piCoeff[ uiIndex ]  )
          {
            if( piCoeff[ uiIndex ] > 0) { uiAbs = static_cast<UInt>(  piCoeff[ uiIndex ] );  uiSign = 0; }
            else                        { uiAbs = static_cast<UInt>( -piCoeff[ uiIndex ] );  uiSign = 1; }
            
            UInt uiSymbol = uiAbs > 1 ? 1 : 0;
            
            if( uiSymbol )
            {
              c1 = 0; c2++;
              uiNumOne++;
            }
            else if( c1 )
            {
              c1++;
            }
          }
        }
      }
    }
  }
  else
  {
    Int c1 = 1;
    Int c2 = 0;
    UInt uiAbs  = 0;
    UInt uiSign = 0;
    
    for ( UInt uiScanPos = 0; uiScanPos < uiWidth*uiHeight; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ (int)g_aucConvertToBit[ uiWidth ] + 1 ][ uiWidth * uiHeight - uiScanPos - 1 ];
      
      puiOneCtx[ uiIndex ] = min<UInt>( c1, 4 );
      puiAbsCtx[ uiIndex ] = min<UInt>( c2, 4 );
      
      if( piCoeff[ uiIndex ]  )
      {
        if( piCoeff[ uiIndex ] > 0) { uiAbs = static_cast<UInt>(  piCoeff[ uiIndex ] );  uiSign = 0; }
        else                        { uiAbs = static_cast<UInt>( -piCoeff[ uiIndex ] );  uiSign = 1; }
        
        UInt uiSymbol = uiAbs > 1 ? 1 : 0;
        
        if( uiSymbol )
        {
          c1 = 0; c2++;
        }
        else if( c1 )
        {
          c1++;
        }
      }
    }
  }
  
  uiDownLeft        = 1;
  uiNumSigTopRight  = 0;
  uiNumSigBotLeft   = 0;
  
  Int     ui16CtxCbf        = pcCU->getCtxCbf( uiAbsPartIdx, eTType, pcCU->getTransformIdx( 0 ) );
  Double  dCBPCost          = xGetICost( m_pcEstBitsSbac->blockCbpBits[ 3 - ui16CtxCbf ][ 0 ] ) + xGetICost( m_pcEstBitsSbac->blockCbpBits[ 3 - ui16CtxCbf ][ 1 ] );
  UInt    uiBestLastIdxP1   = 0;
  Double  d64BestCost       = d64BlockUncodedCost + xGetICost( dCBPCost );
  Double  d64BaseCost       = d64BestCost - xGetICost( m_pcEstBitsSbac->blockCbpBits[ 3 - ui16CtxCbf ][ 0 ] ) + xGetICost( m_pcEstBitsSbac->blockCbpBits[ 3 - ui16CtxCbf ][ 1 ] );
  Double  d64CodedCost      = 0;
  Double  d64UncodedCost    = 0;
  
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
  {
    UInt   uiBlkPos     = g_auiSigLastScan[ uiLog2BlkSize ][ uiDownLeft ][ uiScanPos ];
    UInt   uiPosY       = uiBlkPos >> uiLog2BlkSize;
    UInt   uiPosX       = uiBlkPos - ( uiPosY << uiLog2BlkSize );
    UInt   uiCtxBase    = uiNum4x4Blk > 0 ? puiBaseCtx[ ( uiPosX >> 2 ) + ( uiPosY >> 2 ) * ( uiWidth >> 2 ) ] : 0;
    
    if( uiPosY + uiPosX > uiMaxLineNum )
    {
      break;
    }
    
    if      ( uiWidth == 4 ) dTemp = estErr4x4[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    else if ( uiWidth == 8 ) dTemp = estErr8x8[ iQpRem ][ uiPosX ][ uiPosY ] / dNormFactor;
    
    UShort  uiCtxSig                = getSigCtxInc( piDstCoeff, uiPosX, uiPosY, uiLog2BlkSize, uiWidth, ( uiDownLeft > 0 ) );
    Bool    bLastScanPos            = ( uiScanPos == uiMaxNumCoeff - 1 );
    UInt    uiLevel                 = xGetCodedLevel( d64UncodedCost, d64CodedCost, plLevelDouble[ uiBlkPos ], abs( piCoeff[ uiBlkPos ] ), bLastScanPos, uiCtxSig, puiOneCtx[ uiBlkPos ], puiAbsCtx[ uiBlkPos ], iQBits, dTemp, uiCtxBase );
    piDstCoeff[ uiBlkPos ]          = plSrcCoeff[ uiBlkPos ] < 0 ? -Int( uiLevel ) : uiLevel;
    d64BaseCost                    -= d64UncodedCost;
    d64BaseCost                    += d64CodedCost;
    
    if( uiLevel )
    {
      //----- check for last flag -----
      UShort  uiCtxLast             = getLastCtxInc( uiPosX, uiPosY, uiLog2BlkSize );
      Double  d64CostLastZero       = xGetICost( m_pcEstBitsSbac->lastBits[ uiCtxLast ][ 0 ] );
      Double  d64CostLastOne        = xGetICost( m_pcEstBitsSbac->lastBits[ uiCtxLast ][ 1 ] );
      Double  d64CurrIsLastCost     = d64BaseCost + d64CostLastOne;
      d64BaseCost                  += d64CostLastZero;
      
      if( d64CurrIsLastCost < d64BestCost )
      {
        d64BestCost       = d64CurrIsLastCost;
        uiBestLastIdxP1   = uiScanPos + 1;
      }
      
      //----- update coeff counts -----
      if( uiPosX > uiPosY )
      {
        uiNumSigTopRight++;
      }
      else if( uiPosY > uiPosX )
      {
        uiNumSigBotLeft ++;
      }
    }
    //===== update scan direction =====
#if !HHI_DISABLE_SCAN
    if( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiBlkSizeM1 ) ) ||
       ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiBlkSizeM1 ) )   )
    {
      uiDownLeft = ( uiNumSigTopRight >= uiNumSigBotLeft ? 1 : 0 );
    }
#else
    if( uiScanPos && 
       ( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiBlkSizeM1 ) ) ||
        ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiBlkSizeM1 ) )   ) )
    {
      uiDownLeft = 1 - uiDownLeft;
    }
#endif
  }
  
  //===== clean uncoded coefficients =====
  {
    uiDownLeft          = 1;
    uiNumSigTopRight    = 0;
    uiNumSigBotLeft     = 0;
    for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeff; uiScanPos++ )
    {
      UInt    uiBlkPos  = g_auiSigLastScan[ uiLog2BlkSize ][ uiDownLeft ][ uiScanPos ];
      UInt    uiPosY    = uiBlkPos >> uiLog2BlkSize;
      UInt    uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlkSize );
      
      if( uiScanPos < uiBestLastIdxP1 )
      {
        if( piDstCoeff[ uiBlkPos ] )
        {
          if( uiPosX > uiPosY )
          {
            uiNumSigTopRight++;
          }
          else if( uiPosY > uiPosX )
          {
            uiNumSigBotLeft ++;
          }
        }
      }
      else
      {
        piDstCoeff[ uiBlkPos ] = 0;
      }
      
      uiAbsSum += abs( piDstCoeff[ uiBlkPos ] );
      
#if !HHI_DISABLE_SCAN
      if( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiBlkSizeM1 ) ) ||
         ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiBlkSizeM1 ) )   )
      {
        uiDownLeft = ( uiNumSigTopRight >= uiNumSigBotLeft ? 1 : 0 );
      }
#else
      if( uiScanPos && 
         ( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiBlkSizeM1 ) ) ||
          ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiBlkSizeM1 )   ) ) )
      {
        uiDownLeft = 1 - uiDownLeft;
      }
#endif
    }
  }
}

UInt TComTrQuant::getSigCtxInc    ( TCoeff*                         pcCoeff,
                                   const UInt                      uiPosX,
                                   const UInt                      uiPosY,
                                   const UInt                      uiLog2BlkSize,
                                   const UInt                      uiStride,
                                   const bool                      bDownLeft )
{
  UInt  uiCtxInc  = 0;
  UInt  uiSizeM1  = ( 1 << uiLog2BlkSize ) - 1;
  if( uiLog2BlkSize <= 3 )
  {
    UInt  uiShift = uiLog2BlkSize > 2 ? uiLog2BlkSize - 2 : 0;
    uiCtxInc      = ( ( uiPosY >> uiShift ) << 2 ) + ( uiPosX >> uiShift );
  }
  else if( uiPosX <= 1 && uiPosY <= 1 )
  {
    uiCtxInc            = ( uiPosY << 1 ) + uiPosX;
  }
  else if( uiPosY == 0 )
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    UInt        uiCnt   = ( pData[         -1 ] ? 1 : 0 );
    uiCnt              += ( pData[         -2 ] ? 1 : 0 );
    uiCnt              += ( pData[ iStride -2 ] ? 1 : 0 );
    if( ! bDownLeft )
    {
      uiCnt            += ( pData[ iStride -1 ] ? 1 : 0 );
    }
    uiCtxInc            = 4 + ( ( uiCnt + 1 ) >> 1 );
  }
  else if( uiPosX == 0 )
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    int         iStride2=  iStride << 1;
    UInt        uiCnt   = ( pData[  -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[  -iStride2 ] ? 1 : 0 );
    uiCnt              += ( pData[ 1-iStride2 ] ? 1 : 0 );
    if( bDownLeft )
    {
      uiCnt            += ( pData[ 1-iStride  ] ? 1 : 0 );
    }
    uiCtxInc            = 7 + ( ( uiCnt + 1 ) >> 1 );
  }
  else
  {
    const int*  pData   = &pcCoeff[ uiPosX + uiPosY * uiStride ];
    int         iStride =  uiStride;
    int         iStride2=  iStride << 1;
    UInt        uiCnt   = ( pData[    -iStride  ] ? 1 : 0 );
    uiCnt              += ( pData[ -1           ] ? 1 : 0 );
    uiCnt              += ( pData[ -1 -iStride  ] ? 1 : 0 );
    if( uiPosX > 1 )
    {
      uiCnt            += ( pData[ -2           ] ? 1 : 0 );
      uiCnt            += ( pData[ -2 -iStride  ] ? 1 : 0 );
      if( uiPosY < uiSizeM1 )
      {
        uiCnt          += ( pData[ -2 +iStride  ] ? 1 : 0 );
      }
    }
    if( uiPosY > 1 )
    {
      uiCnt            += ( pData[    -iStride2 ] ? 1 : 0 );
      uiCnt            += ( pData[ -1 -iStride2 ] ? 1 : 0 );
      if( uiPosX < uiSizeM1 )
      {
        uiCnt          += ( pData[  1 -iStride2 ] ? 1 : 0 );
      }
    }
    if( bDownLeft )
    {
      if( uiPosX < uiSizeM1 )
      {
        uiCnt          += ( pData[  1 -iStride  ] ? 1 : 0 );
      }
    }
    else
    {
      if( uiPosY < uiSizeM1 )
      {
        uiCnt          += ( pData[ -1 +iStride  ] ? 1 : 0 );
      }
    }
    uiCtxInc      = 10 + min<UInt>( 4, ( uiCnt + 1 ) >> 1 );
  }
  return uiCtxInc;
}

UInt TComTrQuant::getLastCtxInc   ( const UInt                      uiPosX,
                                   const UInt                      uiPosY,
                                   const UInt                      uiLog2BlkSize )
{
  if( uiLog2BlkSize <= 2 )
  {
    return ( uiPosY << 2 ) + uiPosX;
  }
  else
  {
    return ( uiPosX + uiPosY ) >> ( uiLog2BlkSize - 3 );
  }
}

__inline UInt TComTrQuant::xGetCodedLevel  ( Double&                         rd64UncodedCost,
                                            Double&                         rd64CodedCost,
                                            Long                            lLevelDouble,
                                            UInt                            uiMaxAbsLevel,
                                            bool                            bLastScanPos,
                                            UShort                          ui16CtxNumSig,
                                            UShort                          ui16CtxNumOne,
                                            UShort                          ui16CtxNumAbs,
                                            Int                             iQBits,
                                            Double                          dTemp,
                                            UShort                          ui16CtxBase   ) const
{
  UInt   uiBestAbsLevel = 0;
  Double dErr1          = Double( lLevelDouble );
  
  rd64UncodedCost = dErr1 * dErr1 * dTemp;
  rd64CodedCost   = rd64UncodedCost + xGetICRateCost( 0, bLastScanPos, ui16CtxNumSig, ui16CtxNumOne, ui16CtxNumAbs, ui16CtxBase );
  
  UInt uiMinAbsLevel = ( uiMaxAbsLevel > 1 ? uiMaxAbsLevel - 1 : 1 );
  for( UInt uiAbsLevel = uiMaxAbsLevel; uiAbsLevel >= uiMinAbsLevel ; uiAbsLevel-- )
  {
    Double i64Delta  = Double( lLevelDouble  - Long( uiAbsLevel << iQBits ) );
    Double dErr      = Double( i64Delta );
    Double dCurrCost = dErr * dErr * dTemp + xGetICRateCost( uiAbsLevel, bLastScanPos, ui16CtxNumSig, ui16CtxNumOne, ui16CtxNumAbs, ui16CtxBase );
    
    if( dCurrCost < rd64CodedCost )
    {
      uiBestAbsLevel  = uiAbsLevel;
      rd64CodedCost   = dCurrCost;
    }
  }
  return uiBestAbsLevel;
}

__inline Double TComTrQuant::xGetICRateCost  ( UInt                            uiAbsLevel,
                                              bool                            bLastScanPos,
                                              UShort                          ui16CtxNumSig,
                                              UShort                          ui16CtxNumOne,
                                              UShort                          ui16CtxNumAbs,
                                              UShort                          ui16CtxBase   ) const
{
  if( uiAbsLevel == 0 )
  {
    Double iRate = 0;
    if( !bLastScanPos )
    {
      iRate += m_pcEstBitsSbac->significantBits[ ui16CtxNumSig ][ 0 ];
    }
    return xGetICost( iRate );
  }
  Double iRate = xGetIEPRate();
  if( !bLastScanPos )
  {
    iRate += m_pcEstBitsSbac->significantBits[ ui16CtxNumSig ][ 1 ];
  }
  if( uiAbsLevel == 1 )
  {
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 0 ];
  }
  else if( uiAbsLevel < 15 )
  {
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 0 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 1 ] * (int)( uiAbsLevel - 2 );
  }
  else
  {
    uiAbsLevel -= 14;
    int iEGS    = 1;  for( UInt uiMax = 2; uiAbsLevel >= uiMax; uiMax <<= 1, iEGS += 2 );
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 0 ][ ui16CtxNumOne ][ 1 ];
    iRate += m_pcEstBitsSbac->greaterOneBits[ ui16CtxBase ][ 1 ][ ui16CtxNumAbs ][ 1 ] * 13;
    iRate += xGetIEPRate() * iEGS;
  }
  return xGetICost( iRate );
}

__inline Double TComTrQuant::xGetICost        ( Double                          dRate         ) const
{
  return m_dLambda * dRate;
}

__inline Double TComTrQuant::xGetIEPRate      (                                               ) const
{
  return 32768;
}
