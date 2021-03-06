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

/** \file     TDecSbac.cpp
    \brief    Context-adaptive entropy decoder class
*/

#include "TDecSbac.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSbac::TDecSbac() 
// new structure here
: m_pcBitstream               ( 0 )
, m_pcTDecBinIf               ( NULL )
, m_bAlfCtrl                  ( false )
, m_uiMaxAlfCtrlDepth         ( 0 )
, m_cCUSkipFlagSCModel        ( 1,             1,               NUM_SKIP_FLAG_CTX             )
, m_cCUSplitFlagSCModel       ( 1,             1,               NUM_SPLIT_FLAG_CTX            )
#if HHI_MRG
, m_cCUMergeFlagSCModel       ( 1,             1,               NUM_MERGE_FLAG_CTX            )
, m_cCUMergeIndexSCModel      ( 1,             1,               NUM_MERGE_INDEX_CTX           )
#endif
, m_cCUAlfCtrlFlagSCModel     ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         )
, m_cCUPartSizeSCModel        ( 1,             1,               NUM_PART_SIZE_CTX             )
, m_cCUPredModeSCModel        ( 1,             1,               NUM_PRED_MODE_CTX             )
, m_cCUIntraPredSCModel       ( 1,             1,               NUM_ADI_CTX                   )
, m_cCUChromaPredSCModel      ( 1,             1,               NUM_CHROMA_PRED_CTX           )
, m_cCUInterDirSCModel        ( 1,             1,               NUM_INTER_DIR_CTX             )
, m_cCURefPicSCModel          ( 1,             1,               NUM_REF_NO_CTX                )
, m_cCUMvdSCModel             ( 1,             2,               NUM_MV_RES_CTX                )
, m_cCUTransSubdivFlagSCModel ( 1,             1,               NUM_TRANS_SUBDIV_FLAG_CTX     )
, m_cCUQtRootCbfSCModel       ( 1,             1,               NUM_QT_ROOT_CBF_CTX           )
, m_cCUDeltaQpSCModel         ( 1,             1,               NUM_DELTA_QP_CTX              )
, m_cCUQtCbfSCModel           ( 1,             3,               NUM_QT_CBF_CTX                )
, m_cCUSigSCModel             ( MAX_CU_DEPTH,  2,               NUM_SIG_FLAG_CTX              )
, m_cCULastSCModel            ( MAX_CU_DEPTH,  2,               NUM_LAST_FLAG_CTX             )
, m_cCUOneSCModel             ( 1,             2,               NUM_ONE_FLAG_CTX              )
, m_cCUAbsSCModel             ( 1,             2,               NUM_ABS_FLAG_CTX              )
, m_cMVPIdxSCModel            ( 1,             1,               NUM_MVP_IDX_CTX               )
, m_cALFFlagSCModel           ( 1,             1,               NUM_ALF_FLAG_CTX              )
, m_cALFUvlcSCModel           ( 1,             1,               NUM_ALF_UVLC_CTX              )
, m_cALFSvlcSCModel           ( 1,             1,               NUM_ALF_SVLC_CTX              )
{
}

TDecSbac::~TDecSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecSbac::resetEntropy          (TComSlice* pcSlice)
{
  Int  iQp              = pcSlice->getSliceQp();
  SliceType eSliceType  = pcSlice->getSliceType();
  
  m_cCUSplitFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_SPLIT_FLAG );
  m_cCUSkipFlagSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
#if HHI_MRG                              
  m_cCUMergeFlagSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_MERGE_FLAG );
  m_cCUMergeIndexSCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_MERGE_INDEX );
#endif                                   
  m_cCUAlfCtrlFlagSCModel.initBuffer     ( eSliceType, iQp, (Short*)INIT_SKIP_FLAG );
  m_cCUPartSizeSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PART_SIZE );
  m_cCUPredModeSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer      ( eSliceType, iQp, (Short*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer        ( eSliceType, iQp, (Short*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer          ( eSliceType, iQp, (Short*)INIT_REF_PIC );
  m_cCUDeltaQpSCModel.initBuffer         ( eSliceType, iQp, (Short*)INIT_DQP );
  m_cCUQtCbfSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer       ( eSliceType, iQp, (Short*)INIT_QT_ROOT_CBF );
  m_cCUSigSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_SIG_FLAG );
  m_cCULastSCModel.initBuffer            ( eSliceType, iQp, (Short*)INIT_LAST_FLAG );
  m_cCUOneSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ONE_FLAG );
  m_cCUAbsSCModel.initBuffer             ( eSliceType, iQp, (Short*)INIT_ABS_FLAG );
  m_cMVPIdxSCModel.initBuffer            ( eSliceType, iQp, (Short*)INIT_MVP_IDX );
  m_cALFFlagSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_FLAG );
  m_cALFUvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_UVLC );
  m_cALFSvlcSCModel.initBuffer           ( eSliceType, iQp, (Short*)INIT_ALF_SVLC );
  m_cCUTransSubdivFlagSCModel.initBuffer ( eSliceType, iQp, (Short*)INIT_TRANS_SUBDIV_FLAG );
  
  m_uiLastDQpNonZero  = 0;
  
  // new structure
  m_uiLastQp          = iQp;
  
  m_pcTDecBinIf->start();
}

Void TDecSbac::parseTerminatingBit( UInt& ruiBit )
{
  m_pcTDecBinIf->decodeBinTrm( ruiBit );
}


Void TDecSbac::xReadUnaryMaxSymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
#if FIX108
  if (uiMaxSymbol == 0)
  {
    ruiSymbol = 0;
    return;
  }
#endif
  
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( ruiSymbol == 0 || uiMaxSymbol == 1 )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinIf->decodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont && ( uiSymbol < uiMaxSymbol - 1 ) );
  
  if( uiCont && ( uiSymbol == uiMaxSymbol - 1 ) )
  {
    uiSymbol++;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadMvd( Int& riMvdComp, UInt uiAbsSum, UInt uiCtx )
{
  UInt uiLocalCtx = 0;
  
  if( uiAbsSum >= 3 )
  {
    uiLocalCtx += ( uiAbsSum > 32) ? 2 : 1;
  }
  
  riMvdComp = 0;
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMvdSCModel.get( 0, uiCtx, uiLocalCtx ) );
  
  if (!uiSymbol)
  {
    return;
  }
  
  xReadExGolombMvd( uiSymbol, &m_cCUMvdSCModel.get( 0, uiCtx, 3 ), 3 );
  uiSymbol++;
  
  UInt uiSign;
  m_pcTDecBinIf->decodeBinEP( uiSign );
  
  riMvdComp = ( 0 != uiSign ) ? -(Int)uiSymbol : (Int)uiSymbol;
  
  return;
}

Void TDecSbac::xReadExGolombMvd( UInt& ruiSymbol, ContextModel* pcSCModel, UInt uiMaxBin )
{
  UInt uiSymbol;
  
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  m_pcTDecBinIf->decodeBin( uiSymbol, pcSCModel[1] );
  
  ruiSymbol = 1;
  
  if( !uiSymbol )
  {
    return;
  }
  
  pcSCModel += 2;
  UInt uiCount = 2;
  
  do
  {
    if( uiMaxBin == uiCount )
    {
      pcSCModel++;
    }
    m_pcTDecBinIf->decodeBin( uiSymbol, *pcSCModel );
    uiCount++;
  }
  while( uiSymbol && ( uiCount != 8 ) );
  
  ruiSymbol = uiCount - 1;
  
  if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 3 );
    ruiSymbol += uiSymbol + 1;
  }
  
  return;
}

Void TDecSbac::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  while( uiBit )
  {
    m_pcTDecBinIf->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  uiCount--;
  while( uiCount-- )
  {
    m_pcTDecBinIf->decodeBinEP( uiBit );
    uiSymbol += uiBit << uiCount;
  }
  
  ruiSymbol = uiSymbol;
  
  return;
}

Void TDecSbac::xReadUnarySymbol( UInt& ruiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcTDecBinIf->decodeBin( ruiSymbol, pcSCModel[0] );
  
  if( !ruiSymbol )
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    m_pcTDecBinIf->decodeBin( uiCont, pcSCModel[ iOffset ] );
    uiSymbol++;
  }
  while( uiCont );
  
  ruiSymbol = uiSymbol;
}

Void TDecSbac::xReadExGolombLevel( UInt& ruiSymbol, ContextModel& rcSCModel  )
{
  UInt uiSymbol;
  UInt uiCount = 0;
  do
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, rcSCModel );
    uiCount++;
  }
  while( uiSymbol && ( uiCount != 13 ) );
  
  ruiSymbol = uiCount - 1;
  
  if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 0 );
    ruiSymbol += uiSymbol + 1;
  }
  
  return;
}

Void TDecSbac::parseAlfCtrlDepth( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol( uiSymbol, m_cALFUvlcSCModel.get( 0 ), 1, g_uiMaxCUDepth - 1 );
  ruiAlfCtrlDepth = uiSymbol;
}

Void TDecSbac::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( !m_bAlfCtrl )
  {
    return;
  }
  
  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth( uiAbsPartIdx, m_uiMaxAlfCtrlDepth ) )
  {
    return;
  }
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx ) ) );
  
  if( uiDepth > m_uiMaxAlfCtrlDepth )
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth );
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
}

Void TDecSbac::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_MRG && !SAMSUNG_MRG_SKIP_DIRECT
  if( pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    return;
  }
#endif
  
  if( pcCU->getSlice()->isIntra() )
  {
    return;
  }
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, pcCU->getCtxSkipFlag( uiAbsPartIdx ) ) );
  
  if( uiSymbol )
  {
    pcCU->setPredModeSubParts( MODE_SKIP,  uiAbsPartIdx, uiDepth );
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    
    TComMv cZeroMv(0,0);
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    
    pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
    pcCU->setCbfSubParts  ( 0, 0, 0, uiAbsPartIdx, uiDepth );
    
    if ( pcCU->getSlice()->isInterP() )
    {
      pcCU->setInterDirSubParts( 1, uiAbsPartIdx, 0, uiDepth );
      
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( NOT_VALID, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    }
    else
    {
      pcCU->setInterDirSubParts( 3, uiAbsPartIdx, 0, uiDepth );
      
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( 0, SIZE_2Nx2N, uiAbsPartIdx, 0, uiDepth );
    }
  }
}

#if HHI_MRG
Void TDecSbac::parseMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeFlagSCModel.get( 0, 0, pcCU->getCtxMergeFlag( uiAbsPartIdx ) ) );
  pcCU->setMergeFlagSubParts( uiSymbol ? true : false, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseMergeIndex ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUMergeIndexSCModel.get( 0, 0, pcCU->getCtxMergeIndex( uiAbsPartIdx ) ) );
  pcCU->setMergeIndexSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
}
#endif

Void TDecSbac::parseMVPIdx      ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, m_cMVPIdxSCModel.get(0), 1, iMVPNum-1);
  riMVPIdx = uiSymbol;
}

Void TDecSbac::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
  {
    pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
    return;
  }
  
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUSplitFlagSCModel.get( 0, 0, pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth ) ) );
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
  pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );
  
  return;
}

Void TDecSbac::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    return;
  }
#endif
  
  if ( pcCU->isSkip( uiAbsPartIdx ) )
  {
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    return;
  }
  
  UInt uiSymbol, uiMode = 0;
  PartSize eMode;
  
  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
    eMode = uiSymbol ? SIZE_2Nx2N : SIZE_NxN;
  }
  else
  {
#if HHI_RMP_SWITCH
    if ( !pcCU->getSlice()->getSPS()->getUseRMP())
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      if( uiSymbol )
        uiMode = 0;
      else
        uiMode = 3;
    }
    else
#endif
    {
      UInt uiMaxNumBits = 3;
      for ( UInt ui = 0; ui < uiMaxNumBits; ui++ )
      {
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, ui) );
        if ( uiSymbol )
        {
          break;
        }
        uiMode++;
      }
    }
    eMode = (PartSize) uiMode;
    
    if (pcCU->getSlice()->isInterB() && uiMode == 3)
    {
#if HHI_DISABLE_INTER_NxN_SPLIT
      uiSymbol = 0;
      if( g_uiMaxCUWidth>>uiDepth == 8 )
#endif
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 3) );
      if (uiSymbol == 0)
      {
        pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPartSizeSCModel.get( 0, 0, 4) );
        if (uiSymbol == 0)
          eMode = SIZE_2Nx2N;
      }
    }
  }
  
  pcCU->setPartSizeSubParts( eMode, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
  
  UInt uiTrLevel = 0;
  
  UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
  UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
  uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
  
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
  {
    if( pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1+uiTrLevel, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );
    }
  }
}

Void TDecSbac::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if HHI_MRG
  if ( pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    pcCU->setPredModeSubParts( MODE_INTER, uiAbsPartIdx, uiDepth );
    return;
  }
#endif
  
  if( pcCU->getSlice()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return;
  }
  
  UInt uiSymbol;
  Int  iPredMode = MODE_INTER;
  
#if HHI_MRG && !SAMSUNG_MRG_SKIP_DIRECT
  if ( !pcCU->getSlice()->getSPS()->getUseMRG() )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
    if ( uiSymbol == 0 )
    {
      iPredMode = MODE_SKIP;
    }
  }
#else
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
  if ( uiSymbol == 0 )
  {
    iPredMode = MODE_SKIP;
  }
#endif
  
  if ( pcCU->getSlice()->isInterB() )
  {
    pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
    return;
  }
  
  if ( iPredMode != MODE_SKIP )
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUPredModeSCModel.get( 0, 0, 1 ) );
    iPredMode += uiSymbol;
  }
  
  pcCU->setPredModeSubParts( (PredMode)iPredMode, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );
  
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 0) );
  
  if ( uiSymbol )
    uiIPredMode = iMostProbable;
  else
  {
    Int iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
    if ( g_aucIntraModeBitsAng[iIntraIdx] < 6 )
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode  = uiSymbol;
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 1; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 2; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 4 ) { m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 3; }
    }
    else
    {
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode  = uiSymbol;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 1;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 2;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 3;
      m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) ); uiIPredMode |= uiSymbol << 4;
      
      if (uiIPredMode == 31)
      { // Escape coding for the last two modes
        m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUIntraPredSCModel.get( 0, 0, 1 ) );
        uiIPredMode = uiSymbol ? 32 : 31;
      }
    }
    
    if (uiIPredMode >= iMostProbable)
      uiIPredMode++;
  }
  
  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0, pcCU->getCtxIntraDirChroma( uiAbsPartIdx ) ) );
  
  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, m_cCUChromaPredSCModel.get( 0, 0 ) + 3, 0, 3 );
    uiSymbol++;
  }
  
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  
  return;
}

Void TDecSbac::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  UInt uiCtx = pcCU->getCtxInterDir( uiAbsPartIdx );
  
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, uiCtx ) );
  
  if ( uiSymbol )
  {
    uiSymbol = 2;
  }
#if MS_NO_BACK_PRED_IN_B0
  else if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    uiSymbol = 0;
  }
#endif
  else
  {
    m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUInterDirSCModel.get( 0, 0, 3 ) );
  }
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecSbac::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  UInt uiCtx = pcCU->getCtxRefIdx( uiAbsPartIdx, eRefList );
  
  m_pcTDecBinIf->decodeBin ( uiSymbol, m_cCURefPicSCModel.get( 0, 0, uiCtx ) );
  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, &m_cCURefPicSCModel.get( 0, 0, 4 ), 1, pcCU->getSlice()->getNumRefIdx( eRefList )-2 );
    
    uiSymbol++;
  }
  riRefFrmIdx = uiSymbol;
  return;
}

Void TDecSbac::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  Int iHor, iVer;
  UInt uiAbsPartIdxL, uiAbsPartIdxA;
  Int iHorPred, iVerPred;
  
  TComDataCU* pcCUL   = pcCU->getPULeft ( uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
  TComDataCU* pcCUA   = pcCU->getPUAbove( uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx );
  
  TComCUMvField* pcCUMvFieldL = ( pcCUL == NULL || pcCUL->isIntra( uiAbsPartIdxL ) ) ? NULL : pcCUL->getCUMvField( eRefList );
  TComCUMvField* pcCUMvFieldA = ( pcCUA == NULL || pcCUA->isIntra( uiAbsPartIdxA ) ) ? NULL : pcCUA->getCUMvField( eRefList );
  
  iHorPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsHor() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsHor() );
  iVerPred = ( (pcCUMvFieldL == NULL) ? 0 : pcCUMvFieldL->getMvd( uiAbsPartIdxL ).getAbsVer() ) +
  ( (pcCUMvFieldA == NULL) ? 0 : pcCUMvFieldA->getMvd( uiAbsPartIdxA ).getAbsVer() );
  
  TComMv cTmpMv( 0, 0 );
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );
  
  xReadMvd( iHor, iHorPred, 0 );
  xReadMvd( iVer, iVerPred, 1 );
  
  // set mvd
  TComMv cMv( iHor, iVer );
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiPartIdx, uiDepth );
  
  return;
}


Void TDecSbac::parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize )
{
  m_pcTDecBinIf->decodeBin( ruiSubdivFlag, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiLog2TransformBlockSize ) );
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( ruiSubdivFlag )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiLog2TransformBlockSize )
  DTRACE_CABAC_T( "\n" )
}

Void TDecSbac::parseQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtRootCbf( uiAbsPartIdx );
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
  
  uiQtRootCbf = uiSymbol;
}

Void TDecSbac::parseDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiDQp;
  Int  iDQp;
  
  m_pcTDecBinIf->decodeBin( uiDQp, m_cCUDeltaQpSCModel.get( 0, 0, 0 ) );
  
  if ( uiDQp == 0 )
  {
    uiDQp = pcCU->getSlice()->getSliceQp();
  }
  else
  {
    xReadUnarySymbol( uiDQp, &m_cCUDeltaQpSCModel.get( 0, 0, 2 ), 1 );
    iDQp = ( uiDQp + 2 ) / 2;
    
    if ( uiDQp & 1 )
    {
      iDQp = -iDQp;
    }
    uiDQp = pcCU->getSlice()->getSliceQp() + iDQp;
  }
  
  pcCU->setQPSubParts( uiDQp, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  const UInt uiCtx = pcCU->getCtxQtCbf( uiAbsPartIdx, eType, uiTrDepth );
  m_pcTDecBinIf->decodeBin( uiSymbol , m_cCUQtCbfSCModel.get( 0, eType ? eType - 1: eType, uiCtx ) );
  
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tetype=" )
  DTRACE_CABAC_V( eType )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
  
  pcCU->setCbfSubParts( uiSymbol << uiTrDepth, eType, uiAbsPartIdx, uiDepth );
}

Void TDecSbac::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  DTRACE_CABAC_V( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseCoeffNxN()\teType=" )
  DTRACE_CABAC_V( eTType )
  DTRACE_CABAC_T( "\twidth=" )
  DTRACE_CABAC_V( uiWidth )
  DTRACE_CABAC_T( "\theight=" )
  DTRACE_CABAC_V( uiHeight )
  DTRACE_CABAC_T( "\tdepth=" )
  DTRACE_CABAC_V( uiDepth )
  DTRACE_CABAC_T( "\tabspartidx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\ttoCU-X=" )
  DTRACE_CABAC_V( pcCU->getCUPelX() )
  DTRACE_CABAC_T( "\ttoCU-Y=" )
  DTRACE_CABAC_V( pcCU->getCUPelY() )
  DTRACE_CABAC_T( "\tCU-addr=" )
  DTRACE_CABAC_V(  pcCU->getAddr() )
  DTRACE_CABAC_T( "\tinCU-X=" )
  DTRACE_CABAC_V( g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_T( "\tinCU-Y=" )
  DTRACE_CABAC_V( g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_T( "\tpredmode=" )
  DTRACE_CABAC_V(  pcCU->getPredictionMode( uiAbsPartIdx ) )
  DTRACE_CABAC_T( "\n" )
  
  if( uiWidth > pcCU->getSlice()->getSPS()->getMaxTrSize() )
  {
    uiWidth  = pcCU->getSlice()->getSPS()->getMaxTrSize();
    uiHeight = pcCU->getSlice()->getSPS()->getMaxTrSize();
  }
  
  UInt uiCTXIdx;
  
  switch(uiWidth)
  {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
  }
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : ( eTType == TEXT_NONE ? TEXT_NONE : TEXT_CHROMA );
  
  //----- parse significance map -----
  const UInt  uiLog2BlockSize   = g_aucConvertToBit[ uiWidth ] + 2;
  const UInt  uiMaxNumCoeff     = 1 << ( uiLog2BlockSize << 1 );
  const UInt  uiMaxNumCoeffM1   = uiMaxNumCoeff - 1;
  const UInt  uiNum4x4Blk       = max<UInt>( 1, uiMaxNumCoeff >> 4 );
  UInt        uiDownLeft        = 1;
  UInt        uiNumSigTopRight  = 0;
  UInt        uiNumSigBotLeft   = 0;
  bool        bLastReceived     = false;
  
  for( UInt uiScanPos = 0; uiScanPos < uiMaxNumCoeffM1; uiScanPos++ )
  {
    UInt  uiBlkPos  = g_auiSigLastScan[ uiLog2BlockSize ][ uiDownLeft ][ uiScanPos ];
    UInt  uiPosY    = uiBlkPos >> uiLog2BlockSize;
    UInt  uiPosX    = uiBlkPos - ( uiPosY << uiLog2BlockSize );
    
    //===== code significance flag =====
    UInt  uiSig     = 0;
    UInt  uiCtxSig  = TComTrQuant::getSigCtxInc( pcCoef, uiPosX, uiPosY, uiLog2BlockSize, uiWidth, ( uiDownLeft > 0 ) );
    m_pcTDecBinIf->decodeBin( uiSig, m_cCUSigSCModel.get( uiCTXIdx, eTType, uiCtxSig ) );
    
    if( uiSig )
    {
      pcCoef[ uiBlkPos ] = 1;
      
      if( uiPosX > uiPosY )
      {
        uiNumSigTopRight++;
      }
      else if( uiPosY > uiPosX )
      {
        uiNumSigBotLeft ++;
      }
      
      //===== code last flag =====
      UInt  uiLast     = 0;
      UInt  uiCtxLast  = TComTrQuant::getLastCtxInc( uiPosX, uiPosY, uiLog2BlockSize );
      m_pcTDecBinIf->decodeBin( uiLast, m_cCULastSCModel.get( uiCTXIdx, eTType, uiCtxLast ) );
      
      if( uiLast )
      {
        bLastReceived = true;
        break;
      }
    }
    
    //===== update scan direction =====
#if !HHI_DISABLE_SCAN
    if( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiHeight - 1 ) ) ||
       ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiWidth  - 1 ) )   )
    {
      uiDownLeft = ( uiNumSigTopRight >= uiNumSigBotLeft ? 1 : 0 );
    }
#else
    if( uiScanPos && 
       ( ( uiDownLeft == 1 && ( uiPosX == 0 || uiPosY == uiHeight - 1 ) ) ||
        ( uiDownLeft == 0 && ( uiPosY == 0 || uiPosX == uiWidth  - 1 ) )   ) )
    {
      uiDownLeft = 1 - uiDownLeft;
    }
#endif
  }
  if( !bLastReceived )
  {
    pcCoef[ uiMaxNumCoeffM1 ] = 1;
  }
  
  /*
   * Sign and bin0 PCP (Section 3.2 and 3.3 of JCTVC-B088)
   */
  Int  c1, c2;
  UInt uiSign;
  UInt uiLevel;
  
  if( uiNum4x4Blk > 1 )
  {
    Bool b1stBlk  = true;
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
      
      TCoeff* piCurr = &pcCoef[ uiSubPosX + uiSubPosY * uiWidth ];
      
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
        c1 = 1;
        c2 = 0;
        
        if( b1stBlk )
        {
          b1stBlk  = false;
          uiCtxSet = 5;
        }
        else
        {
          uiCtxSet = ( uiNumOne >> 2 ) + 1;
          uiNumOne = 0;
        }
        
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos - ( uiPosY << 2 );
          UInt  uiIndex   = ( ( uiSubPosY + uiPosY ) << uiLog2BlockSize ) + uiSubPosX + uiPosX;
          
          uiLevel = pcCoef[ uiIndex ];
          
          if( uiLevel )
          {
            UInt uiCtx = min<UInt>(c1, 4);
            m_pcTDecBinIf->decodeBin( uiLevel, m_cCUOneSCModel.get( 0, eTType, ( uiCtxSet << 2 ) + uiCtxSet + uiCtx ) );
            if( uiLevel == 1 )
            {
              c1      = 0;
              uiLevel = 2;
            }
            else if( c1 )
            {
              c1++;
              uiLevel++;
            }
            else
            {
              uiLevel++;
            }
            pcCoef[ uiIndex ] = uiLevel;
          }
        }
        
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos - ( uiPosY << 2 );
          UInt  uiIndex   = ( ( uiSubPosY + uiPosY ) << uiLog2BlockSize ) + uiSubPosX + uiPosX;
          
          uiLevel = pcCoef[ uiIndex ];
          
          if( uiLevel )
          {
            if( uiLevel == 2 )
            {
              UInt uiCtx = min<UInt>(c2, 4);
              c2++;
              uiNumOne++;
              xReadExGolombLevel( uiLevel, m_cCUAbsSCModel.get( 0, eTType, ( uiCtxSet << 2 ) + uiCtxSet + uiCtx ) );
              uiLevel += 2;
            }
            pcCoef[ uiIndex ] = uiLevel;
          }
        }
        
        for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
        {
          UInt  uiBlkPos  = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
          UInt  uiPosY    = uiBlkPos >> 2;
          UInt  uiPosX    = uiBlkPos - ( uiPosY << 2 );
          UInt  uiIndex   = (uiSubPosY + uiPosY) * uiWidth + uiSubPosX + uiPosX;
          
          uiLevel = pcCoef[ uiIndex ];
          
          if( uiLevel )
          {
            m_pcTDecBinIf->decodeBinEP( uiSign );
            pcCoef[ uiIndex ] = ( uiSign ? -(Int)uiLevel : (Int)uiLevel );
          }
        }
      }
    }
  }
  else
  {
    c1 = 1;
    c2 = 0;
    
    for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
      uiLevel = pcCoef[ uiIndex ];
      
      if( uiLevel )
      {
        UInt uiCtx = min<UInt>(c1, 4);
        m_pcTDecBinIf->decodeBin( uiLevel, m_cCUOneSCModel.get( 0, eTType, uiCtx ) );
        if( uiLevel == 1 )
        {
          c1      = 0;
          uiLevel = 2;
        }
        else if( c1 )
        {
          c1++;
          uiLevel++;
        }
        else
        {
          uiLevel++;
        }
        pcCoef[ uiIndex ] = uiLevel;
      }
    }
    
    for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
      uiLevel = pcCoef[ uiIndex ];
      
      if( uiLevel )
      {
        if( uiLevel == 2 )
        {
          UInt uiCtx = min<UInt>(c2, 4);
          c2++;
          xReadExGolombLevel( uiLevel, m_cCUAbsSCModel.get( 0, eTType, uiCtx ) );
          uiLevel += 2;
        }
        pcCoef[ uiIndex ] = uiLevel;
      }
    }
    
    for( UInt uiScanPos = 0; uiScanPos < 16; uiScanPos++ )
    {
      UInt uiIndex = g_auiFrameScanXY[ 1 ][ 15 - uiScanPos ];
      uiLevel = pcCoef[ uiIndex ];
      
      if( uiLevel )
      {
        m_pcTDecBinIf->decodeBinEP( uiSign );
        pcCoef[ uiIndex ] = ( uiSign ? -(Int)uiLevel : (Int)uiLevel );
      }
    }
  }
  
  return;
}

Void TDecSbac::parseAlfFlag (UInt& ruiVal)
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
  
  ruiVal = uiSymbol;
}

#if TSB_ALF_HEADER
Void TDecSbac::parseAlfFlagNum( UInt& ruiVal, UInt minValue, UInt depth )
{
  UInt uiLength = 0;
  UInt maxValue = (minValue << (depth*2));
  UInt temp = maxValue - minValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      uiLength = i+1;
    }
    temp = (temp >> 1);
  }
  ruiVal = 0;
  UInt uiBit;
  if(uiLength)
  {
    while( uiLength-- )
    {
      m_pcTDecBinIf->decodeBinEP( uiBit );
      ruiVal += uiBit << uiLength;
    }
  }
  else
  {
    ruiVal = 0;
  }
  ruiVal += minValue;
}

Void TDecSbac::parseAlfCtrlFlag( UInt &ruiAlfCtrlFlag )
{
  UInt uiSymbol;
  m_pcTDecBinIf->decodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, 0 ) );
  ruiAlfCtrlFlag = uiSymbol;
}
#endif

Void TDecSbac::parseAlfUvlc (UInt& ruiVal)
{
  UInt uiCode;
  Int  i;
  
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
  if ( uiCode == 0 )
  {
    ruiVal = 0;
    return;
  }
  
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
    if ( uiCode == 0 ) break;
    i++;
  }
  
  ruiVal = i;
}

Void TDecSbac::parseAlfSvlc (Int&  riVal)
{
  UInt uiCode;
  Int  iSign;
  Int  i;
  
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  
  if ( uiCode == 0 )
  {
    riVal = 0;
    return;
  }
  
  // read sign
  m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
  
  if ( uiCode == 0 ) iSign =  1;
  else               iSign = -1;
  
  // read magnitude
  i=1;
  while (1)
  {
    m_pcTDecBinIf->decodeBin( uiCode, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
    if ( uiCode == 0 ) break;
    i++;
  }
  
  riVal = i*iSign;
}
