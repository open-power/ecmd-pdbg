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

#ifndef CIP_REMOVE_INSTRUCTION_FUNCTIONS
/* ################################################################# */
/* Proc Functions - Proc Functions - Proc Functions - Proc Functions */
/* ################################################################# */
uint32_t p10_dllCipStartInstructions(const ecmdChipTarget & i_target, uint32_t i_thread) {
    
    uint32_t rc = ECMD_SUCCESS;
    struct pdbg_target *thread, *core;
    uint32_t  count = 0;

    rc = mapEcmdCoreToPdbgCoreTarget(i_target, &core);
    if (rc)
    {    
        return out.error(rc, FUNCNAME, 
                         "Failed to map get core mapping!!, rc=%d\n", rc);
    }

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
            return out.error(rc, FUNCNAME, 
                             "Failed to start the given thread!!, rc=%d\n", rc);
        }
        count++;
    }
    
    //Additional check to validate if thread_start() is really called.
    if(count){
        return rc;
    } else {
        return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                         "Thread start not called on any threads!!\n");
    }
}


uint32_t p10_dllCipStartAllInstructions() {
  
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *proc=NULL;
 
  //It is important to probe all the child targets for available
  //proc's before issuing thread startall chipop 
  rc = probeChildTarget(proc, "proc", "pib");
  if (rc != ECMD_SUCCESS)
  {
      return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "Target not configured!\n");
  }
  
  //start all the threads 
  rc = thread_start_all();
  if (rc != 0){
      return out.error(rc, FUNCNAME, 
                       "Failed to start all the threads!!,rc=%d\n", rc);
  }

  return rc;
} 

uint32_t p10_dllCipStopInstructions(const ecmdChipTarget & i_target, uint32_t i_thread) {

    uint32_t rc = ECMD_SUCCESS;
    struct pdbg_target *thread, *core;
    uint32_t count = 0;

    rc = mapEcmdCoreToPdbgCoreTarget(i_target, &core);
    if (rc)
    {    
        return out.error(rc, FUNCNAME, 
                         "Failed to map get core mapping!!, rc=%d\n", rc);
    }
    
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
            return out.error(rc, FUNCNAME, 
                             "Failed to stop the given thread!!, rc=%d\n" ,rc);
        }
        count++;
    }

    //Additional check to validate if thread_stop() is really called.
    if(count) {
        return rc;
    } else {
        return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                         "Thread stop not called on any threads!!\n");
    }
}

uint32_t p10_dllCipStopAllInstructions() {

  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *proc=NULL;

  //It is important to probe all the pib targets for available
  //proc's before issuing thread stopall chipop  
  rc = probeChildTarget(proc, "proc", "pib");
  if (rc != ECMD_SUCCESS)
  {
      return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "Target not configured!\n");
  }

  //stop all the threads 
  rc = thread_stop_all(); 
  if (rc != 0) {
      return out.error(rc, FUNCNAME, 
                       "Failed to stop all the threads!!, rc=%d\n", rc);
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



