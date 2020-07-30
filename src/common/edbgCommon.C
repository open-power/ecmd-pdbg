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

