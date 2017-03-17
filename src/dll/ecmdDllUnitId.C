//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2017,2017 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//IBM_PROLOG_END_TAG

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <list>
#include <ecmdDllCapi.H>
#include <ecmdStructs.H>

//--------------------------------------------------------------------
//  Function Definitions                                               
//--------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t dllFuncNotYetSupported(const char* message) // added from ecmdDllError.C
{
  std::string l_mesg = message;
  l_mesg += "() Not Yet Supported\n";
  dllOutputError(l_mesg.c_str());

  return ECMD_FUNCTION_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void  ecmdRegisterErrorMsgHelper( uint32_t i_errorCode , // added from ecmdDllError.C
                                  const char * i_whom,
                                  const char * i_message,
                                  ecmdChipTarget * i_targetptr )
{
#ifdef FIPSODE
  TRACFCOMP(G_ecmdTracDesc, ERR_MRK" %s: %s ,Error mapped to rc = 0x%x", i_whom,
            i_message, i_errorCode);
#endif
  uint32_t registerMsg = ECMD_SUCCESS;
  uint32_t registerErrTgt = ECMD_SUCCESS;

  registerMsg = dllRegisterErrorMsg(i_errorCode, i_whom,i_message);
  if(registerMsg!=ECMD_SUCCESS){
#ifdef FIPSODE
    TRACFCOMP(G_ecmdTracDesc, ERR_MRK "%s(): Got Error Log Back from "
              "ecmdRegisterErrorMsgError Mapped to this rc: 0x%08x", __FUNCTION__, registerMsg);
#endif
  }

  if (i_targetptr){
    registerErrTgt = dllRegisterErrorTarget(i_errorCode, *i_targetptr);
    if(registerErrTgt!=ECMD_SUCCESS){
#ifdef FIPSODE
      TRACFCOMP(G_ecmdTracDesc, ERR_MRK "%s(): Got Error Log Back from "
              "ecmdRegisterErrorTarget Error Mapped to this rc: 0x%08x",  __FUNCTION__, registerErrTgt);
#endif
    }
  }

  //clear the array     
  //for(int i = 0;i<150;i++)      {
  //  g_errMsg[i] = '\0';
  //}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t dllTargetToUnitId(ecmdChipTarget & io_target)
{
  return dllFuncNotYetSupported("dllTargetToUnitId");  
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t dllUnitIdStringToTarget(std::string i_unitId, std::list<ecmdChipTarget> & o_target)
{
  return dllFuncNotYetSupported("dllUnitIdStringToTarget");
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t dllUnitIdToTarget(uint32_t i_unitId, std::list<ecmdChipTarget> & o_target)
{
  return dllFuncNotYetSupported("dllUnitIdToTarget");
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t dllUnitIdToString(uint32_t i_unitId, std::string & o_unitIdStr)
{
  return dllFuncNotYetSupported("dllUnitIdToString");
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//For a given Sequence Id and valid ecmdChipTarget state this function converts from SequenceId to target
uint32_t dllSequenceIdToTarget(uint32_t i_coreSeqNum, ecmdChipTarget & io_chipletEXTarget, uint32_t i_threadSeqNum)
{
  return dllFuncNotYetSupported("dllSequencIdToTarget");  
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//Converts Target to sequence ID
uint32_t dllTargetToSequenceId(ecmdChipTarget i_chipletEXTarget, uint32_t & o_coreSeqNum, uint32_t & o_threadSeqNum)
{
  return dllFuncNotYetSupported("dllTargetToSequenceId");  
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint32_t dllGetUnitIdVersion(uint32_t & o_unitIdVersion)
{
  return dllFuncNotYetSupported("dllGetUnitIdVersion");
}


