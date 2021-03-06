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

/** \file     TDecTop.cpp
    \brief    decoder class
*/

#include "TDecTop.h"

TDecTop::TDecTop()
{
  m_iGopSize      = 0;
  m_bGopSizeSet   = false;
  m_iMaxRefPicNum = 0;
  m_uiValidPS = 0;
#if ENC_DEC_TRACE
  g_hTrace = fopen( "TraceDec.txt", "wb" );
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif
}

TDecTop::~TDecTop()
{
#if ENC_DEC_TRACE
  fclose( g_hTrace );
#endif
}

Void TDecTop::create()
{
  m_cGopDecoder.create();
  m_apcSlicePilot = new TComSlice;
}

Void TDecTop::destroy()
{
  m_cGopDecoder.destroy();
  
  delete m_apcSlicePilot;
  m_apcSlicePilot = NULL;
  
  m_cSliceDecoder.destroy();
}

Void TDecTop::init()
{
  // initialize ROM
  initROM();
  
  m_cGopDecoder.  init( &m_cEntropyDecoder, &m_cSbacDecoder, &m_cBinCABAC, &m_cCavlcDecoder, &m_cSliceDecoder, &m_cLoopFilter, &m_cAdaptiveLoopFilter );
  m_cSliceDecoder.init( &m_cEntropyDecoder, &m_cCuDecoder );
  m_cEntropyDecoder.init(&m_cPrediction);
}

Void TDecTop::deletePicBuffer ( )
{
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );
  
  for (Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
    pcPic->destroy();
    
    delete pcPic;
    pcPic = NULL;
  }
  
  // destroy ALF temporary buffers
  m_cAdaptiveLoopFilter.destroy();
  
  m_cLoopFilter.        destroy();
  
  // destroy ROM
  destroyROM();
}

Void TDecTop::xUpdateGopSize (TComSlice* pcSlice)
{
  if ( !pcSlice->isIntra() && !m_bGopSizeSet)
  {
    m_iGopSize    = pcSlice->getPOC();
    m_bGopSizeSet = true;
    
    m_cGopDecoder.setGopSize(m_iGopSize);
  }
}

Void TDecTop::xGetNewPicBuffer ( TComSlice* pcSlice, TComPic*& rpcPic )
{
  xUpdateGopSize(pcSlice);
  
  m_iMaxRefPicNum = Max(m_iMaxRefPicNum, Max(Max(2, pcSlice->getNumRefIdx(REF_PIC_LIST_0)+1), m_iGopSize/2 + 2 + pcSlice->getNumRefIdx(REF_PIC_LIST_0)));
  
  if (m_cListPic.size() < (UInt)m_iMaxRefPicNum)
  {
    rpcPic = new TComPic;
    
    rpcPic->create ( pcSlice->getSPS()->getWidth(), pcSlice->getSPS()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, true);
    m_cListPic.pushBack( rpcPic );
    
    return;
  }
  
  Bool bBufferIsAvailable = false;
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  while (iterPic != m_cListPic.end())
  {
    rpcPic = *(iterPic++);
    if ( rpcPic->getReconMark() == false )
    {
      bBufferIsAvailable = true;
      break;
    }
  }
  
  if ( !bBufferIsAvailable )
  {
    pcSlice->sortPicList(m_cListPic);
    iterPic = m_cListPic.begin();
    rpcPic = *(iterPic);
    rpcPic->setReconMark(false);
    
    // mark it should be extended
    rpcPic->getPicYuvRec()->setBorderExtension(false);
  }
}

Void TDecTop::decode (Bool bEos, TComBitstream* pcBitstream, UInt& ruiPOC, TComList<TComPic*>*& rpcListPic)
{
  rpcListPic = NULL;
  TComPic*    pcPic = NULL;
  
  // Initialize entropy decoder
  m_cEntropyDecoder.setEntropyDecoder (&m_cCavlcDecoder);
  m_cEntropyDecoder.setBitstream      (pcBitstream);
  
#if HHI_NAL_UNIT_SYNTAX
  // don't feel like adding the whole chain of interface crap just to access the first byte in the buffer
  const UChar* pucBuffer = reinterpret_cast<const UChar*>(pcBitstream->getStartStream());
  const NalUnitType eNalUnitType = NalUnitType(pucBuffer[0]&31); 
  const bool bDecodeSPS   = ( NAL_UNIT_SPS == eNalUnitType );
  const bool bDecodePPS   = ( NAL_UNIT_PPS == eNalUnitType );
  const bool bDecodeSlice = ( NAL_UNIT_CODED_SLICE == eNalUnitType );
#else
  const bool bDecodeSlice = true;
  bool bDecodeSPS   = false;
  bool bDecodePPS   = false;
  if( 0 == m_uiValidPS )
  {
    bDecodeSPS = bDecodePPS = true;
  }
#endif
  
  if( bDecodeSPS )
  {
    m_cEntropyDecoder.decodeSPS( &m_cSPS );
    
    // initialize DIF
    m_cPrediction.setDIFTap ( m_cSPS.getDIFTap () );
    // create ALF temporary buffer
    m_cAdaptiveLoopFilter.create( m_cSPS.getWidth(), m_cSPS.getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
    
    m_cLoopFilter.        create( g_uiMaxCUDepth );
    m_uiValidPS |= 1;
  }
  
  if( bDecodePPS )
  {
    m_cEntropyDecoder.decodePPS( &m_cPPS );
    m_uiValidPS |= 2;
  }
  
  if( false == bDecodeSlice )
  {
    return;
  }
  
  // make sure we already received both parameter sets
  assert( 3 == m_uiValidPS );
  
  m_apcSlicePilot->initSlice();
  
  //  Read slice header
  m_apcSlicePilot->setSPS( &m_cSPS );
  m_apcSlicePilot->setPPS( &m_cPPS );
  m_cEntropyDecoder.decodeSliceHeader (m_apcSlicePilot);
  
  // Buffer initialize for prediction.
  m_cPrediction.initTempBuff();
  //  Get a new picture buffer
  xGetNewPicBuffer (m_apcSlicePilot, pcPic);
  
  // Recursive structure
  m_cCuDecoder.create ( g_uiMaxCUDepth, g_uiMaxCUWidth, g_uiMaxCUHeight );
  m_cCuDecoder.init   ( &m_cEntropyDecoder, &m_cTrQuant, &m_cPrediction );
  m_cTrQuant.init     ( g_uiMaxCUWidth, g_uiMaxCUHeight, m_apcSlicePilot->getSPS()->getMaxTrSize());
  
  m_cSliceDecoder.create( m_apcSlicePilot, m_apcSlicePilot->getSPS()->getWidth(), m_apcSlicePilot->getSPS()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  
  //  Set picture slice pointer
  TComSlice*  pcSlice = m_apcSlicePilot;
  m_apcSlicePilot = pcPic->getPicSym()->getSlice();
  pcPic->getPicSym()->setSlice(pcSlice);
  
  // Set reference list
  pcSlice->setRefPicList( m_cListPic );
  
  // HierP + GPB case
  if ( m_cSPS.getUseLDC() && pcSlice->isInterB() )
  {
    Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
    pcSlice->setNumRefIdx( REF_PIC_LIST_1, iNumRefIdx );
    
    for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
    {
      pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
    }
  }
  
  // For generalized B
  // note: maybe not existed case (always L0 is copied to L1 if L1 is empty)
  if (pcSlice->isInterB() && pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0)
  {
    Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
    pcSlice->setNumRefIdx        ( REF_PIC_LIST_1, iNumRefIdx );
    
    for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
    {
      pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
    }
  }
  
  //---------------
  pcSlice->setRefPOCList();
  
#if MS_NO_BACK_PRED_IN_B0
  pcSlice->setNoBackPredFlag( false );
  if ( pcSlice->getSliceType() == B_SLICE )
  {
    if ( pcSlice->getNumRefIdx(RefPicList( 0 ) ) == pcSlice->getNumRefIdx(RefPicList( 1 ) ) )
    {
      pcSlice->setNoBackPredFlag( true );
      int i;
      for ( i=0; i < pcSlice->getNumRefIdx(RefPicList( 1 ) ); i++ )
      {
        if ( pcSlice->getRefPOC(RefPicList(1), i) != pcSlice->getRefPOC(RefPicList(0), i) ) 
        {
          pcSlice->setNoBackPredFlag( false );
          break;
        }
      }
    }
  }
#endif
  
  //  Decode a picture
  m_cGopDecoder.decompressGop ( bEos, pcBitstream, pcPic );
  
  pcSlice->sortPicList(m_cListPic);       //  sorting for application output
  
  ruiPOC = pcPic->getSlice()->getPOC();
  
  rpcListPic = &m_cListPic;
  
  m_cCuDecoder.destroy();
  
  return;
}

