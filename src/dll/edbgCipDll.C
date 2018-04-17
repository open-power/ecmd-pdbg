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
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// Headers from eCMD
#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBuffer.H>
#include <ecmdSharedUtils.H>

// Extension headers
#include <cipDllCapi.H>
#include <cipStructs.H>

// Headers from ecmd-pdbg
#include <edbgCommon.H>

/* ################################################################# */
/* Base Functions - Base Functions - Base Functions - Base Functions */
/* ################################################################# */
uint32_t dllCipInitExtensionInPlugin() {
  /* Nothing for us to do to init an extention */
  return 0;
}

uint32_t dllCipGetSysInfo(cipSysInfo_t & o_sysInfo) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

/* ######################################################################### */
/* Memory Functions - Memory Functions - Memory Functions - Memory Functions */
/* ######################################################################### */
uint32_t dllCipGetMemProc(ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_memoryData, ecmdDataBuffer & o_memoryTags, ecmdDataBuffer & o_memoryEcc, ecmdDataBuffer & o_memoryEccError) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipPutMemProc(ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & i_memoryData, ecmdDataBuffer & i_memoryTags, ecmdDataBuffer & i_memoryErrorInject) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipGetMemMemCtrl(ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_memoryData, ecmdDataBuffer & o_memoryTags, ecmdDataBuffer & o_memoryEcc, ecmdDataBuffer & o_memoryEccError, ecmdDataBuffer & o_memorySpareBits) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipPutMemMemCtrl(ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & i_memoryData, ecmdDataBuffer & i_memoryTags, ecmdDataBuffer & i_memoryEcc, ecmdDataBuffer & i_memorySpareBits, ecmdDataBuffer & i_memoryErrorInject) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipGetMemPba(ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipPutMemPba(ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & i_data, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipGetMemProcVariousAddrType(ecmdChipTarget & i_target, ecmdDataBuffer i_address, uint32_t i_bytes, cipXlateVariables i_xlateVars, ecmdDataBuffer & o_memoryData, ecmdDataBuffer & o_memoryTags, ecmdDataBuffer & o_memoryEcc, ecmdDataBuffer & o_memoryEccError, ecmdDataBuffer & o_realAddress) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCipPutMemProcVariousAddrType(ecmdChipTarget & i_target, ecmdDataBuffer i_address, uint32_t i_bytes, cipXlateVariables i_xlateVars, ecmdDataBuffer & i_memoryData, ecmdDataBuffer & io_memoryTags, ecmdDataBuffer & o_realAddress) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
