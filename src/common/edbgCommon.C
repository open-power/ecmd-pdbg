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

uint32_t mapEcmdCoreToPdbgCoreTarget(const ecmdChipTarget & i_target, struct pdbg_target **o_target) {
  struct pdbg_target *proc, *target;

  assert(i_target.cageState == ECMD_TARGET_FIELD_VALID &&        \
         i_target.cage == 0 &&                                   \
         i_target.nodeState == ECMD_TARGET_FIELD_VALID &&        \
         i_target.node == 0 &&                                   \
         i_target.slotState == ECMD_TARGET_FIELD_VALID &&        \
         i_target.slot == 0 &&                                   \
         i_target.posState == ECMD_TARGET_FIELD_VALID);
         

  *o_target = NULL;
  char path[16];
  
  sprintf(path, "/proc%d", i_target.pos);
  proc = pdbg_target_from_path(NULL, path);

  //Bail out if give proc position not available.
  if (proc == NULL)  return 1;

  //loop through cores under proc and return core target matching chip unit num.
  pdbg_for_each_target("core", proc, target) 
  {
      if (pdbg_target_index(target) == i_target.chipUnitNum)
      {
          *o_target = target;
          return 0;
      }
  }
  return 1;
}

uint32_t probeChildTarget(struct pdbg_target *i_pTarget, std::string i_pTarget_name, std::string i_cTarget_name) {

  pdbg_for_each_class_target(i_pTarget_name.c_str(), i_pTarget) {
    struct pdbg_target *l_cTarget;
    char path[16];

    sprintf(path, 
            "/%s%d/%s",i_pTarget_name.c_str(), pdbg_target_index(i_pTarget), 
            i_cTarget_name.c_str());

    l_cTarget = pdbg_target_from_path(NULL, path);

    //Bail out if give proc position not available.
    if (l_cTarget == NULL)  return 1;

    // Probe the selected target
    if(pdbg_target_probe(l_cTarget) != PDBG_TARGET_ENABLED)
      continue;
  }
  return 0;
}

uint8_t getFapiUnitPos(pdbg_target *target)
{
    uint32_t fapiUnitPos; //chip unit position
    
    //size: uint8 => 1, uint16 => 2. uint32 => 4 uint64=> 8
    //typedef uint32_t ATTR_FAPI_POS_Type;
    if(!pdbg_target_get_attribute(target, "ATTR_FAPI_POS", 4, 1, &fapiUnitPos)){ 
       return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                 "ATTR_FAPI_POS Attribute get failed");
    }

    return fapiUnitPos;
}

bool isOdysseyChip(pdbg_target *target)
{
    constexpr uint8_t ODYSSEY_CHIP_TYPE = 0x4B;
    constexpr uint16_t ODYSSEY_CHIP_ID = 0x60C0;
    
    uint32_t chipID; //chip ID
    if(!pdbg_target_get_attribute(target, "ATTR_CHIP_ID", 4, 1, &chipID)){ 
       return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                 "ATTR_CHIP_ID Attribute get failed");
    }
    uint8_t chipType;
    //size: uint8 => 1, uint16 => 2. uint32 => 4 uint64=> 8
    if(!pdbg_target_get_attribute(target, "ATTR_TYPE", 1, 1, &chipType)){ 
       return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                 "ATTR_TYPE Attribute get failed");
    }
    //both chipid and type shall match for it to return true
    return( (chipID == ODYSSEY_CHIP_ID) && 
        (chipType == ODYSSEY_CHIP_TYPE) );
}
