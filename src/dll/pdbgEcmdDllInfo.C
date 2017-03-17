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

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#define pdbgEcmdDllInfo_C
#include <stdio.h>

#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#undef pdbgEcmdDllInfo_C

//---------------------------------------------------------------------
// Function Definitions
//---------------------------------------------------------------------
uint32_t dllQueryDllInfo(ecmdDllInfo & o_dllInfo) {
  o_dllInfo.dllBuildInfo = "pdbg eCMD Plugin";

  // dllType and dllProduct are enums in eCMD < 15.0, strings >= 15
  o_dllInfo.dllType = ECMD_DLL_PDBG;
  o_dllInfo.dllProduct = ECMD_DLL_PRODUCT_OPENPOWER;
  //o_dllInfo.dllType = "pdbg";
  //o_dllInfo.dllProduct = "OpenPOWER";
  o_dllInfo.dllProductType = GIT_COMMIT_REV;
  o_dllInfo.dllEnv = ECMD_DLL_ENV_HW;  

  o_dllInfo.dllBuildDate = BUILD_DATE;
  o_dllInfo.dllCapiVersion = ECMD_CAPI_VERSION;

  return 0;
}
