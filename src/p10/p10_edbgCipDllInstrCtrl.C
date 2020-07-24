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

#ifndef CIP_REMOVE_INSTRUCTION_FUNCTIONS
/* ################################################################# */
/* Proc Functions - Proc Functions - Proc Functions - Proc Functions */
/* ################################################################# */

static uint32_t mapEcmdCoreToPdbgCoreTarget(const ecmdChipTarget & i_target, struct pdbg_target **o_target) {
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

uint32_t p10_dllCipStartInstructions(const ecmdChipTarget & i_target, uint32_t i_thread) {
    
    uint32_t rc = ECMD_SUCCESS;
    struct pdbg_target *thread, *core;

    rc = mapEcmdCoreToPdbgCoreTarget(i_target, &core);
    if (rc) return rc;    

    pdbg_for_each_target("thread", core, thread) {

        // Check if the requested thread is matching the pdbg target index. then,
        // we bring up the thread on that core.
        if (pdbg_target_index(thread) != i_target.thread)
            continue;

	if (pdbg_target_probe(thread) != PDBG_TARGET_ENABLED)
            continue;

        out.print("Starting the thread on the core c%d t%d\n", 
                  i_target.chipUnitNum, i_target.thread);
  
        //start the individual thread 
        rc = thread_start(thread);
        if (rc != 0)
        {
            return out.error(rc, FUNCNAME, "Failed to start the given thread!!, rc=%d\n", rc);
        }
    }
    return rc; 
}


uint32_t p10_dllCipStartAllInstructions() {
  
  uint32_t rc = ECMD_SUCCESS;
  
  //start all the threads 
  rc = thread_start_all();
  if (rc != 0){
      return out.error(rc, FUNCNAME, "Failed to start all the threads!!,rc=%d\n", rc);
  }

  return rc;
} 

uint32_t p10_dllCipStopInstructions(const ecmdChipTarget & i_target, uint32_t i_thread) {

    uint32_t rc = ECMD_SUCCESS;
    struct pdbg_target *thread, *core;
    ecmdChipTarget l_target = i_target;

    rc = mapEcmdCoreToPdbgCoreTarget(l_target, &core);
    if (rc) return rc;  
    
    pdbg_for_each_target("thread", core, thread) {
        
        // Check if the requested thread is matching the pdbg target index. then,
        // we bring down the thread on that core.
        if (pdbg_target_index(thread) != i_target.thread)
            continue;
        
	if (pdbg_target_probe(thread) != PDBG_TARGET_ENABLED)
            continue;

        out.print("Stopping the thread on the core c%d t%d\n", 
                   i_target.chipUnitNum, i_target.thread);
   
        //stop the individual thread 
        rc = thread_stop(thread);
        if (rc != 0)
        {
            return out.error(rc, FUNCNAME, "Failed to stop the given thread!!, rc=%d\n" ,rc);
        }
    }
    return rc; 
}

uint32_t p10_dllCipStopAllInstructions() {

  uint32_t rc = ECMD_SUCCESS;
  
  //stop all the threads 
  rc = thread_stop_all(); 
  if (rc != 0) {
      return out.error(rc, FUNCNAME, "Failed to stop all the threads!!, rc=%d", rc);
  }
 
  return rc;
} 

uint32_t p10_dllCipSpecialWakeup(const ecmdChipTarget & i_target, bool i_enable, uint32_t i_mode) {
  
  //Dummy implementation of special wakeup. 
  //cipinstruct start calls CipSpecialWakeup which returns SUCCESS as 
  //Special wakeup is handled by SBE chip-op itself.    
  return ECMD_SUCCESS;
}

#endif // CIP_REMOVE_INSTRUCTION_FUNCTIONS



