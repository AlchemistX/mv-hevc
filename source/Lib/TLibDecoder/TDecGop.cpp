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

/** \file     TDecGop.cpp
    \brief    GOP decoder class
*/

#include "TDecGop.h"
#include "TDecCAVLC.h"
#include "TDecSbac.h"
#include "TDecBinCoder.h"
#include "TDecBinCoderCABAC.h"

#include <time.h>

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TDecGop::TDecGop()
{
  m_iGopSize = 0;
}

TDecGop::~TDecGop()
{
  
}

Void TDecGop::create()
{
  
}

Void TDecGop::destroy()
{
  
}

Void TDecGop::init( TDecEntropy*            pcEntropyDecoder, 
                   TDecSbac*               pcSbacDecoder, 
                   TDecBinCABAC*           pcBinCABAC,
                   TDecCavlc*              pcCavlcDecoder, 
                   TDecSlice*              pcSliceDecoder, 
                   TComLoopFilter*         pcLoopFilter, 
                   TComAdaptiveLoopFilter* pcAdaptiveLoopFilter )
{
  m_pcEntropyDecoder      = pcEntropyDecoder;
  m_pcSbacDecoder         = pcSbacDecoder;
  m_pcBinCABAC            = pcBinCABAC;
  m_pcCavlcDecoder        = pcCavlcDecoder;
  m_pcSliceDecoder        = pcSliceDecoder;
  m_pcLoopFilter          = pcLoopFilter;
  m_pcAdaptiveLoopFilter  = pcAdaptiveLoopFilter;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecGop::decompressGop (Bool bEos, TComBitstream* pcBitstream, TComPic*& rpcPic)
{
  TComSlice*  pcSlice = rpcPic->getSlice();
  
  //-- For time output for each slice
  long iBeforeTime = clock();
  
  UInt iSymbolMode = pcSlice->getSymbolMode();
  if (iSymbolMode)
  {
    m_pcSbacDecoder->init( (TDecBinIf*)m_pcBinCABAC );
    m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
  }
  else
  {
    m_pcEntropyDecoder->setEntropyDecoder (m_pcCavlcDecoder);
  }
  
  m_pcEntropyDecoder->setBitstream      (pcBitstream);
  m_pcEntropyDecoder->resetEntropy      (pcSlice);
  
  ALFParam cAlfParam;
  
  if ( rpcPic->getSlice()->getSPS()->getUseALF() )
  {
#if TSB_ALF_HEADER
    m_pcAdaptiveLoopFilter->setNumCUsInFrame(rpcPic);
#endif
    m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
    m_pcEntropyDecoder->decodeAlfParam( &cAlfParam );
  }
  
  m_pcSliceDecoder->decompressSlice(pcBitstream, rpcPic);
  
  // deblocking filter
  m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), 0, 0);
  m_pcLoopFilter->loopFilterPic( rpcPic );
  
  // adaptive loop filter
  if( rpcPic->getSlice()->getSPS()->getUseALF() )
  {
    m_pcAdaptiveLoopFilter->ALFProcess(rpcPic, &cAlfParam);
    m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);
  }
  
  //-- For time output for each slice
  printf("\nPOC %4d ( %c-SLICE, QP%3d ) ",
         pcSlice->getPOC(),
         pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B',
         pcSlice->getSliceQp() );
  
  Double dDecTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  printf ("[DT %6.3f] ", dDecTime );
  
  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf ("[L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
      printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
    }
    printf ("] ");
  }
  
  rpcPic->setReconMark(true);
}

