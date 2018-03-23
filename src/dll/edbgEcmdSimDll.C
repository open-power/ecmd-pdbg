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
#ifndef REMOVE_SIM
#include <string>
#include <unistd.h>

#include "ecmdReturnCodes.H"
#include "ecmdDllCapi.H"
#include <ecmdUtils.H>

uint32_t dllSimaet(const char* i_function) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimcheckpoint(const char* i_checkpoint) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimclock(uint32_t i_cycles) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t simclockWithDftTrace(uint32_t i_cycles, bool i_fixed) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimecho(const char* i_message) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}       

uint32_t dllSimexit(uint32_t i_rc, const char* i_message) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimEXPECTFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_expect, uint64_t i_row, uint32_t i_offset ) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimexpecttcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_expect, uint64_t i_row) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimgetcurrentcycle(uint64_t & o_cyclecount) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGETFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & o_data, uint64_t i_row, uint32_t i_offset) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGETFACX(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & o_data, uint64_t i_row, uint32_t i_offset) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimgettcfac(const char* i_tcfacname, ecmdDataBuffer & o_data, uint64_t i_row, uint32_t i_startbit, uint32_t i_bitLength) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
 
uint32_t dllSiminit(const char* i_checkpoint) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimPUTFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimPUTFACX(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimputtcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_numRows) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
 
uint32_t dllSimrestart(const char* i_checkpoint) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimSTKFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset)
{
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimSTKFACX(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimstktcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_numRows) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimSUBCMD(const char* i_command) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimtckinterval(uint32_t i_cycles) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimUNSTICK(const char* i_facname, uint32_t i_bitLength, uint64_t i_row, uint32_t i_offset) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimunsticktcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_numRows) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGetHierarchy(ecmdChipTarget & i_target, std::string & o_hierarchy) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGetFullFacName(ecmdChipTarget & i_target, std::string i_facName, std::string & o_fullFacName, bool i_hierarchyOnly) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryChipSimModelVersion(ecmdChipTarget & i_target, std::string & o_timestamp) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryChipScandefVersion(ecmdChipTarget & i_target, std::string & o_timestamp) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

std::string dllSimCallFusionCommand(const char* i_fusionObject, const char* i_replicaID, const char* i_command) {
  std::string ret;
  return ret;
}

uint32_t dllSimFusionRand32(uint32_t i_min , uint32_t i_max , const char* i_fusionRandObject ) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint64_t dllSimFusionRand64(uint64_t i_min , uint64_t i_max , const char* i_fusionRandObject ) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimOutputFusionMessage(const char* i_header,  const char * i_message, ecmdFusionSeverity_t i_severity,  ecmdFusionMessageType_t i_type, const char* i_file , uint32_t i_line ) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

void dllSimSetFusionMessageFormat( const char* i_format) {
  return;
}

uint32_t dllSimPutDial(const char* i_dialName, const std::string i_enumValue) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllSimGetDial(const char* i_dialName, std::string & o_enumValue) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGetOutFile(const char* i_filename, std::string & o_absFilename) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGetInFile(const char* i_filename, std::string & o_absFilename) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGetEnvironment(const char* i_envName, std::string & o_envValue) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimWaitUntil(const char* i_facName, uint32_t i_bitNumber, uint32_t i_bitValue, uint64_t i_timeOut , bool i_passive) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimGetModelInfo(ecmdSimModelInfo& o_modelInfo) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSimRunTestcase(const std::vector<std::string>& i_testcaseNames, bool i_continueOnError) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // REMOVE_SIM
