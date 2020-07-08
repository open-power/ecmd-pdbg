//IBM_PROLOG_BEGIN_TAG
/*
 * eCMD for pdbg Project
 *
 * Copyright 2017,2018 IBM International Business Machines Corp.
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

// Headers from pdbg
extern "C" {
#include <libpdbg.h>
}

// Headers from ecmd-pdbg
#include <edbgCommon.H>
#include <edbgOutput.H>
#include <p10_edbgEcmdDllScom.H>
#include <p10_scominfo.H>

#ifndef ECMD_REMOVE_SCOM_FUNCTIONS
//convert the enum to string for use in code
uint32_t p10x_convertCUEnum_to_String(p10ChipUnits_t i_P10CU, std::string &o_chipUnitType)
{
  uint32_t rc = ECMD_SUCCESS;
  uint32_t l_index;

  for (l_index = 0;
       l_index < (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t));
       l_index++)
  {
      //Looking for input ekb chip unit in table
      if (i_P10CU == ChipUnitTable[l_index].ekbChipUnit)
          break;

  }
  // Can't find i_P10CU in table
  if (l_index >= (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t))){
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown chip unit enum:%d\n", i_P10CU);
  }
      
  o_chipUnitType = ChipUnitTable[l_index].chipUnitType;
  return rc;
}

//convert chipunit string to pdbg class type, as pdbg does not accept ecmd strings
uint32_t p10x_convertCUString_to_pdbgClassString(std::string cuString, std::string &o_pdbgClassType)
{
  uint32_t rc = ECMD_SUCCESS;
  uint32_t l_index;
  
  for (l_index = 0;
       l_index < (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t));
       l_index++)
  {
      //Looking for input chip unit type in table
      if (cuString == ChipUnitTable[l_index].chipUnitType)
          break;

  }
  // Can't find cuString in table
  if (l_index >= (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t))){
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown chip unit:%S\n", cuString.c_str());
  }
      
  o_pdbgClassType = ChipUnitTable[l_index].pdbgClassType;
  return rc;
}

//convert chipunit string to pdbg class type, as pdbg does not accept ecmd strings
uint32_t p10x_convertPDBGClassString_to_CUString(std::string pdbgClassType, std::string &o_chipUnitType) {

  uint32_t rc = ECMD_SUCCESS;
  uint32_t l_index;

  for (l_index = 0;
       l_index < (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t));
       l_index++)
  {
      //Looking for input chip unit type in table
      if (pdbgClassType == ChipUnitTable[l_index].pdbgClassType)
          break;

  }
  // Can't find pdbgClassType in table
  if (l_index >= (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t))){
      return  out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown pdbg class unit:%s\n", pdbgClassType.c_str());
  }
      
  o_chipUnitType = ChipUnitTable[l_index].chipUnitType;
  return rc;
}


uint32_t p10_dllQueryScom(const ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdScomData sdReturn;

  // Wipe out the data structure provided by the user
  o_queryData.clear();

  sdReturn.address = i_address;
  sdReturn.length = 64;
  sdReturn.isChipUnitRelated = false;
  sdReturn.endianMode = ECMD_BIG_ENDIAN;
  std::vector<p10_chipUnitPairing_t> l_chipUnitPairing;
  p10ChipUnits_t l_P10CU = P10_NO_CU;
  sdReturn.relatedChipUnit.clear();
  sdReturn.relatedChipUnitShort.clear();

  rc = p10_scominfo_isChipUnitScom(l_P10CU, 0x10, i_address, sdReturn.isChipUnitRelated, l_chipUnitPairing, P10_DEFAULT_MODE);
  if (rc) {
    return out.error(rc, FUNCNAME,"Invalid scom addr via scom address lookup via p10_scominfo_isChipUnitScom failed\n");
  }
  if (sdReturn.isChipUnitRelated) {
    std::vector<p10_chipUnitPairing_t>::const_iterator cuPairingIter = l_chipUnitPairing.begin();

    while(cuPairingIter != l_chipUnitPairing.end()) {
      std::string l_chipUnitType;
      rc = p10x_convertCUEnum_to_String(cuPairingIter->chipUnitType, l_chipUnitType);
      if (rc) return rc;
      sdReturn.isChipUnitRelated = true;
      sdReturn.relatedChipUnit.push_back(l_chipUnitType);
      sdReturn.relatedChipUnitShort.push_back(l_chipUnitType);
      cuPairingIter++;
    }
  }
  o_queryData.push_back(sdReturn);

  return rc;
}

uint32_t p10_dllGetScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint64_t data;
  struct pdbg_target *target;
  struct pdbg_target *addr_base;
  std::string pdbgClassString;

  rc = p10x_convertCUString_to_pdbgClassString(i_target.chipUnitType, pdbgClassString);
  if (rc)
  {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                       "Matching pdbg class string not found!");
  }

  pdbg_for_each_class_target(pdbgClassString.c_str() ,target) {

      //for "pu" there is no matching required with chip unit number
      if (i_target.chipUnitType != ""){
          if (pdbg_target_index(target) != i_target.chipUnitNum) 
              continue;
      } 

      const char *path = pdbg_target_path(target);
      uint64_t xlate_addr = i_address;
      addr_base = pdbg_address_absolute(target, &xlate_addr);
      
      // Make sure the pdbg target probe has been done and get the target state
      if (pdbg_target_probe(target) != PDBG_TARGET_ENABLED) {
          continue;
      }

      // Do the read and store the data in the return buffer
      rc = pib_read(target, i_address, &data);
      if (rc) {
          return out.error(EDBG_READ_ERROR, FUNCNAME, 
                 "p%d: pib_read of 0x%016" PRIx64 " = 0x%016" PRIx64 " failed (%s) \n",
                 pdbg_target_index(addr_base), xlate_addr, data, path);
      }
  }
  o_data.setBitLength(64);
  o_data.setDoubleWord(0, data);
  return rc;
}

uint32_t p10_dllPutScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *target;
  struct pdbg_target *addr_base;
  std::string pdbgClassString;

  rc = p10x_convertCUString_to_pdbgClassString(i_target.chipUnitType, pdbgClassString);
  if (rc) {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                       "Matching pdbg class string not found!");
  }
  
  // Get the chip level pdbg target for the call to the pib write
  pdbg_for_each_class_target(pdbgClassString.c_str() ,target) {

      //for "pu" there is no matching required with chip unit number
      if (i_target.chipUnitType != ""){
          if (pdbg_target_index(target) != i_target.chipUnitNum) 
              continue;
      }

      const char *path = pdbg_target_path(target);
      uint64_t xlate_addr = i_address;
      addr_base = pdbg_address_absolute(target, &xlate_addr);

      if (pdbg_target_probe(target) != PDBG_TARGET_ENABLED)
          continue;

      // Write the data to the chip if index matches with chip unit number
      rc = pib_write(target, i_address, i_data.getDoubleWord(0));
      if (rc) {
          return out.error(EDBG_WRITE_ERROR, FUNCNAME, 
                           "pib_write of 0x%" PRIx64 " failed (%s)\n",
                            pdbg_target_index(addr_base), xlate_addr, path);
      }
  }
  return rc;
}

uint32_t p10_dllPutScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t p10_dllDoScomMultiple(const ecmdChipTarget & i_target, std::list<ecmdScomEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_SCOM_FUNCTIONS



