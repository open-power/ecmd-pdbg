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
#include <string>
#include <unistd.h>

#include "ecmdReturnCodes.H"
#include "ecmdDllCapi.H"
#include <ecmdUtils.H>

uint32_t dllSimaet(const char* i_function) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllSimcheckpoint(const char* i_checkpoint) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}


uint32_t dllSimclock(uint32_t i_cycles) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}


uint32_t simclockWithDftTrace(uint32_t i_cycles, bool i_fixed) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}


uint32_t dllSimecho(const char* i_message) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}       
 

uint32_t dllSimexit(uint32_t i_rc, const char* i_message) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 


uint32_t dllSimEXPECTFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_expect, uint32_t i_row, uint32_t i_offset )
{
    return dllSimEXPECTFACHidden(i_facname, i_bitLength, i_expect, i_row, i_offset);
}

uint32_t dllSimEXPECTFACHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_expect, uint64_t i_row, uint32_t i_offset ) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimexpecttcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_expect, uint32_t i_row)
{
    return dllSimexpecttcfacHidden(i_tcfacname, i_bitLength, i_expect, i_row);
}

uint32_t dllSimexpecttcfacHidden(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_expect, uint64_t i_row) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimgetcurrentcycle(uint64_t & o_cyclecount) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

 

uint32_t dllSimGETFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & o_data, uint32_t i_row, uint32_t i_offset)
{
    return dllSimGETFACHidden(i_facname, i_bitLength, o_data, i_row, i_offset);
}

uint32_t dllSimGETFACHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & o_data, uint64_t i_row, uint32_t i_offset) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimGETFACX(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & o_data, uint32_t i_row, uint32_t i_offset)
{
    return dllSimGETFACXHidden(i_facname, i_bitLength, o_data, i_row, i_offset);
}

uint32_t dllSimGETFACXHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & o_data, uint64_t i_row, uint32_t i_offset) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}


uint32_t dllSimgettcfac(const char* i_tcfacname, ecmdDataBuffer & o_data, uint32_t i_row, uint32_t i_startbit, uint32_t i_bitLength)
{
    return dllSimgettcfacHidden(i_tcfacname, o_data, i_row, i_startbit, i_bitLength);
}

uint32_t dllSimgettcfacHidden(const char* i_tcfacname, ecmdDataBuffer & o_data, uint64_t i_row, uint32_t i_startbit, uint32_t i_bitLength) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 
uint32_t dllSiminit(const char* i_checkpoint) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 
 

uint32_t dllSimPUTFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_offset)
{
    return dllSimPUTFACHidden(i_facname, i_bitLength, i_data, i_row, i_offset);
}

uint32_t dllSimPUTFACHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimPUTFACX(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_offset)
{
    return dllSimPUTFACXHidden( i_facname, i_bitLength, i_data, i_row, i_offset);
}

uint32_t dllSimPUTFACXHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimputtcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_numRows)
{
    return dllSimputtcfacHidden(i_tcfacname, i_bitLength, i_data, i_row, i_numRows);
}

uint32_t dllSimputtcfacHidden(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_numRows) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

 
uint32_t dllSimrestart(const char* i_checkpoint) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 


uint32_t dllSimSTKFAC(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_offset)
{
    return dllSimSTKFACHidden(i_facname, i_bitLength, i_data, i_row, i_offset);
}

uint32_t dllSimSTKFACHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset)
{
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllSimSTKFACX(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_offset)
{
    return dllSimSTKFACXHidden(i_facname, i_bitLength, i_data, i_row, i_offset);
}

uint32_t dllSimSTKFACXHidden(const char* i_facname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_offset) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}


uint32_t dllSimstktcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_numRows)
{
    return dllSimstktcfacHidden(i_tcfacname, i_bitLength, i_data, i_row, i_numRows);
}

uint32_t dllSimstktcfacHidden(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_numRows) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimSUBCMD(const char* i_command) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimtckinterval(uint32_t i_cycles) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}



uint32_t dllSimUNSTICK(const char* i_facname, uint32_t i_bitLength, uint32_t i_row, uint32_t i_offset)
{
    return dllSimUNSTICKHidden(i_facname, i_bitLength, i_row, i_offset);
}

uint32_t dllSimUNSTICKHidden(const char* i_facname, uint32_t i_bitLength, uint64_t i_row, uint32_t i_offset) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}
 

uint32_t dllSimunsticktcfac(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint32_t i_row, uint32_t i_numRows)
{
    return dllSimunsticktcfacHidden(i_tcfacname, i_bitLength, i_data, i_row, i_numRows);
}

uint32_t dllSimunsticktcfacHidden(const char* i_tcfacname, uint32_t i_bitLength, ecmdDataBuffer & i_data, uint64_t i_row, uint32_t i_numRows) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

uint32_t dllSimGetHierarchy(ecmdChipTarget & i_target, std::string & o_hierarchy) {

  uint32_t rc = ECMD_SUCCESS;
  return rc;

}

uint32_t dllSimGetFullFacName(ecmdChipTarget & i_target, std::string i_facName, std::string & o_fullFacName, bool i_hierarchyOnly) {

  uint32_t rc = ECMD_SUCCESS;
  return rc;

}


uint32_t dllQueryChipSimModelVersion(ecmdChipTarget & i_target, std::string & o_timestamp) {
  uint32_t rc = ECMD_SUCCESS;
  o_timestamp = "NA";
  return rc;
}

uint32_t dllQueryChipScandefVersion(ecmdChipTarget & i_target, std::string & o_timestamp) {
  uint32_t rc = ECMD_SUCCESS;
  return rc;
}

std::string dllSimCallFusionCommand(const char* i_fusionObject, const char* i_replicaID, const char* i_command) {
  std::string ret;
  return ret;
}

uint32_t dllSimFusionRand32(uint32_t i_min , uint32_t i_max , const char* i_fusionRandObject ) {
  return 0;
}

uint64_t dllSimFusionRand64(uint64_t i_min , uint64_t i_max , const char* i_fusionRandObject ) {
  return 0;
}

uint32_t dllSimOutputFusionMessage(const char* i_header,  const char * i_message, ecmdFusionSeverity_t i_severity,  ecmdFusionMessageType_t i_type, const char* i_file , uint32_t i_line ) {
  uint32_t rc = 0;
  return rc;
}

void dllSimSetFusionMessageFormat( const char* i_format) {
  return;
}

uint32_t dllSimPutDial(const char* i_dialName, const std::string i_enumValue) {
  uint32_t rc = 0;
  return rc;
} 


uint32_t dllSimGetDial(const char* i_dialName, std::string & o_enumValue) {
  uint32_t rc = 0;
  return rc;
}

uint32_t dllSimGetOutFile(const char* i_filename, std::string & o_absFilename) {
  uint32_t rc = 0;
  return rc;
}

uint32_t dllSimGetInFile(const char* i_filename, std::string & o_absFilename) {
  uint32_t rc = 0;
  return rc;
}

uint32_t dllSimGetEnvironment(const char* i_envName, std::string & o_envValue) {
  uint32_t rc = 0;
  return rc;
}

uint32_t dllSimWaitUntil(const char* i_facName, uint32_t i_bitNumber, uint32_t i_bitValue, uint64_t i_timeOut , bool i_passive) {
  uint32_t rc = 0;
  return rc;
}

uint32_t dllSimGetModelInfo(ecmdSimModelInfo& o_modelInfo) {
  uint32_t rc = 0;
  return rc;
}

uint32_t dllSimRunTestcase(const std::vector<std::string>& i_testcaseNames, bool i_continueOnError) {
  uint32_t rc = 0;
  return rc;
}
