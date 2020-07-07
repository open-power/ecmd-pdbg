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
#include <assert.h>

// Headers from pdbg
extern "C" {
#include <libpdbg.h>
}

// Headers from ecmd-pdbg
#include <edbgCommon.H>
#include <edbgOutput.H>
#include <p9_scominfo.H>

#ifndef ECMD_REMOVE_SCOM_FUNCTIONS
/* Given a target in i_target this will return the associcated pdbg target that
 * can be passed to pib_read/write() such that full address translation is
 * performed. For example - getscom pu.ex -c6 20000100 would get translated to
 * pib_read(pdbg_target, 0x100) which would get translated by pib_read() to
 * 0x26000100. */
static uint32_t fetchPdbgTarget(const ecmdChipTarget & i_target, struct pdbg_target ** o_pdbgTarget) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *chipTarget, *target;
  uint32_t index;

  assert(i_target.cageState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.cage == 0 &&                                  \
         i_target.nodeState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.node == 0 &&                                  \
         i_target.slotState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.slot == 0 &&                                  \
         i_target.posState == ECMD_TARGET_FIELD_VALID);

  std::string cuString;
  
  *o_pdbgTarget = NULL;
  pdbg_for_each_class_target("pib", chipTarget) {
    const char *p;

    // Don't search for targets not matched to our index/position
    index = pdbg_target_index(chipTarget);
    if (i_target.pos != index)
      continue;

    // Just return the raw pib target if we're not looking for a specific chip unit
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_UNUSED) {
      *o_pdbgTarget = chipTarget;
    } else {
      // Search child nodes of this position to find what we are
      // looking for
      pdbg_for_each_child_target(chipTarget, target) {
	p = (char *) pdbg_get_target_property(target, "ecmd,chip-unit-type", NULL);
        if (p &&
            p == i_target.chipUnitType &&
            pdbg_target_index(target) == i_target.chipUnitNum) {
          *o_pdbgTarget = target;
	  break;
        }
      }
    }
    if (*o_pdbgTarget)
      break;
  }

  if (!*o_pdbgTarget) {
    return out.error(1, FUNCNAME, "Unable to find pdbg target!\n");
  }

  return rc;
}

//convert the enum to string for use in code
uint32_t p9n_convertCUEnum_to_String(p9ChipUnits_t i_P9CU, std::string &o_chipUnitType) {
  uint32_t rc = ECMD_SUCCESS;

  if (i_P9CU == PU_C_CHIPUNIT)            o_chipUnitType = "c";
  else if (i_P9CU == PU_EQ_CHIPUNIT)      o_chipUnitType = "eq";
  else if (i_P9CU == PU_EX_CHIPUNIT)      o_chipUnitType = "ex";
  else if (i_P9CU == PU_XBUS_CHIPUNIT)    o_chipUnitType = "xbus";
  else if (i_P9CU == PU_OBUS_CHIPUNIT)    o_chipUnitType = "obus";
  else if (i_P9CU == PU_NV_CHIPUNIT)      o_chipUnitType = "nv";
  else if (i_P9CU == PU_PEC_CHIPUNIT)     o_chipUnitType = "pec";
  else if (i_P9CU == PU_PHB_CHIPUNIT)     o_chipUnitType = "phb";
  else if (i_P9CU == PU_MI_CHIPUNIT)      o_chipUnitType = "mi";
  else if (i_P9CU == PU_DMI_CHIPUNIT)     o_chipUnitType = "dmi";
  else if (i_P9CU == PU_MCS_CHIPUNIT)     o_chipUnitType = "mcs";
  else if (i_P9CU == PU_MCA_CHIPUNIT)     o_chipUnitType = "mca";
  else if (i_P9CU == PU_MCBIST_CHIPUNIT)  o_chipUnitType = "mcbist";
  else if (i_P9CU == PU_PERV_CHIPUNIT)    o_chipUnitType = "perv";
  else if (i_P9CU == PU_PPE_CHIPUNIT)     o_chipUnitType = "ppe";
  else if (i_P9CU == PU_SBE_CHIPUNIT)     o_chipUnitType = "sbe";
  else if (i_P9CU == PU_CAPP_CHIPUNIT)    o_chipUnitType = "capp";
  else if (i_P9CU == PU_MC_CHIPUNIT)      o_chipUnitType = "mc";
  else {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown chip unit enum:%d\n", i_P9CU);
  }

  return rc;
}

//convert chipunit string to enum, as scominfo does not accept strings
uint32_t p9n_convertCUString_to_enum(std::string cuString, p9ChipUnits_t &o_P9CU) {
  uint32_t rc = ECMD_SUCCESS;

  if (cuString == "c")          o_P9CU = PU_C_CHIPUNIT;
  else if (cuString == "eq")    o_P9CU = PU_EQ_CHIPUNIT;
  else if (cuString == "ex")    o_P9CU = PU_EX_CHIPUNIT;
  else if (cuString == "xbus")  o_P9CU = PU_XBUS_CHIPUNIT;
  else if (cuString == "obus")  o_P9CU = PU_OBUS_CHIPUNIT;
  else if (cuString == "nv")    o_P9CU = PU_NV_CHIPUNIT;
  else if (cuString == "pec")   o_P9CU = PU_PEC_CHIPUNIT;
  else if (cuString == "phb")   o_P9CU = PU_PHB_CHIPUNIT;
  else if (cuString == "mi")    o_P9CU = PU_MI_CHIPUNIT;
  else if (cuString == "dmi")   o_P9CU = PU_DMI_CHIPUNIT;
  else if (cuString == "mcs")   o_P9CU = PU_MCS_CHIPUNIT;
  else if (cuString == "mca")   o_P9CU = PU_MCA_CHIPUNIT;
  else if (cuString == "mcbist")  o_P9CU = PU_MCBIST_CHIPUNIT;
  else if (cuString == "perv")  o_P9CU = PU_PERV_CHIPUNIT;
  else if (cuString == "ppe")   o_P9CU = PU_PPE_CHIPUNIT;
  else if (cuString == "sbe")   o_P9CU = PU_SBE_CHIPUNIT;
  else if (cuString == "capp")  o_P9CU = PU_CAPP_CHIPUNIT;
  else if (cuString == "mc")    o_P9CU = PU_MC_CHIPUNIT;
  else {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown chip unit:%S\n", cuString.c_str());
  }

  return rc;
}

/* ################################################################# */
/* Scom Functions - Scom Functions - Scom Functions - Scom Functions */
/* ################################################################# */
// i_address is a partially translated address - it will contain a
// chiplet base address but it's up to us to add in the chiplet number
// and the rest of the offset. libpdbg expects either a fully translated
// address or a non-translated address, so we need to remove the partial
// translation so we can pass the non-translated address.
static uint64_t getRawScomAddress(const ecmdChipTarget & i_target, uint64_t i_address) {
 //struct pdbg_target *chipUnitTarget;
 //
 //(!findChipUnitType(i_target, i_address, &chipUnitTarget))
 //  i_address -= dt_get_address(chipUnitTarget->dn, 0, NULL);

  uint64_t o_address = i_address;

  // Only call the conversion function if the target is for a chipunit
  if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) {
    p9ChipUnits_t l_P9CU = P9N_CHIP; //default is the chip
    p9n_convertCUString_to_enum(i_target.chipUnitType, l_P9CU);

    o_address = p9_scominfo_createChipUnitScomAddr(l_P9CU, i_target.chipUnitNum, i_address);
  }

  return o_address;
}

uint32_t p9_dllQueryScom(const ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdScomData sdReturn;

  // Wipe out the data structure provided by the user
  o_queryData.clear();

  sdReturn.address = i_address;
  sdReturn.length = 64;
  sdReturn.isChipUnitRelated = false;
  sdReturn.endianMode = ECMD_BIG_ENDIAN;

  //// Need to work out the related chip unit. This amounts to getting the
  //// chiplet id from i_address and wokring out what name to associate with
  //// it.
  //if (!findChipUnitType(i_target, i_address, &chipUnitTarget)) {
  //  p = dt_find_property(chipUnitTarget->dn, "ecmd,chip-unit-type");
  //  assert(p);
  //  sdReturn.isChipUnitRelated = true;
  //  sdReturn.relatedChipUnit.push_back(p->prop);
  //}
  
  // per ben
  // l_mode 0x00000000 p9n dd10
  // l_mode 0x00000001 PPE_MODE
  // l_mode 0x00000002 p9n dd20+
  // l_mode 0x00000004 p9c dd10
  // l_mode 0x00000008 p9c dd20+
  uint32_t l_mode = 0x2; // Force it to p9n dd20+ for now

  std::vector<p9_chipUnitPairing_t> l_chipUnitPairing;
  rc = p9_scominfo_isChipUnitScom(i_address, sdReturn.isChipUnitRelated, l_chipUnitPairing, l_mode);
  if (rc) {
    return out.error(rc, FUNCNAME,"Invalid scom addr via scom address lookup via p9_scominfo_isChipUnitScom failed\n");
  }
  
  //for P9n and all other Pegasus Generation of chips we only have 1 chipUnit per scom addr, the list is for the P9 Generation expansion
  if (sdReturn.isChipUnitRelated) {
    std::vector<p9_chipUnitPairing_t>::const_iterator cuPairingIter = l_chipUnitPairing.begin();

    while(cuPairingIter != l_chipUnitPairing.end()) {
      std::string l_chipUnitType;
      rc = p9n_convertCUEnum_to_String(cuPairingIter->chipUnitType, l_chipUnitType);
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

uint32_t p9_dllGetScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint64_t data;
  struct pdbg_target *target;
  
  // Convert the input address to an absolute chip level address
  i_address = getRawScomAddress(i_target, i_address);

  // Now for the call to pdbg, use just the chip level target so the address
  // doesn't get translated again down in pdbg
  // i_target is pass by reference, make a local copy before we modify so we don't break upstream
  ecmdChipTarget l_target = i_target;
  l_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;

  // Get the chip level pdbg target for the call to the pib read
  if (fetchPdbgTarget(l_target, &target)) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unable to find PIB target\n");
  }

  // Make sure the pdbg target probe has been done and get the target state
  if (pdbg_target_probe(target) != PDBG_TARGET_ENABLED) {
    return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "Target not configured!\n");
  }

  // Do the read and store the data in the return buffer
  rc = pib_read(target, i_address, &data);
  if (rc) {
    return out.error(EDBG_READ_ERROR, FUNCNAME, "pib_read of 0x%" PRIx64 " failed!\n", i_address);
  }
  o_data.setBitLength(64);
  o_data.setDoubleWord(0, data);

  return rc;

}

uint32_t p9_dllPutScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *target;
  
  // Convert the input address to an absolute chip level address
  i_address = getRawScomAddress(i_target, i_address);

  // Now for the call to pdbg, use just the chip level target so the address
  // doesn't get translated again down in pdbg
  // i_target is pass by reference, make a local copy before we modify so we don't break upstream
  ecmdChipTarget l_target = i_target;
  l_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;  

  // Get the chip level pdbg target for the call to the pib read
  if (fetchPdbgTarget(l_target, &target)) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unable to find PIB target\n");
  }

  // Make sure the pdbg target probe has been done and get the target state
  if (pdbg_target_probe(target) != PDBG_TARGET_ENABLED) {
    return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "Target not configured!\n");
  }
  
  // Write the data to the chip
  rc = pib_write(target, i_address, i_data.getDoubleWord(0));
  if (rc) {
    return out.error(EDBG_WRITE_ERROR, FUNCNAME, "pib_write of 0x%" PRIx64 " failed!\n", i_address);
  }

  return rc;
}

uint32_t p9_dllPutScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t p9_dllDoScomMultiple(const ecmdChipTarget & i_target, std::list<ecmdScomEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_SCOM_FUNCTIONS



