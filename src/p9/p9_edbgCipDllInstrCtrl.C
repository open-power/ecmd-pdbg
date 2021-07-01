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
//none

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
uint32_t p9_dllCipStartAllInstructions() {

  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *thr_target;
    
  //Get any thread level pdbg target for the call
  //Make sure the pdbg target probe has been done and get the target state
  pdbg_for_each_class_target("thread", thr_target) {
      if (pdbg_target_status(thr_target) != PDBG_TARGET_ENABLED)
          continue;             
  }
 
  //start all the threads 
  rc = thread_start_all();                           
  if (rc < 0)
  {
      return out.error(rc, FUNCNAME, "Failed to start all the threads, rc=%d\n",rc);
  }
  return rc;
} 

uint32_t p9_dllCipStopAllInstructions() {

  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *thr_target;
    
  //Get any thread level pdbg target for the call
  //Make sure the pdbg target probe has been done and get the target state
  pdbg_for_each_class_target("thread", thr_target) {
      if (pdbg_target_status(thr_target) != PDBG_TARGET_ENABLED)
          continue;             
  }
 
  //stop all the threads 
  rc = thread_stop_all();                           
  if (rc < 0)
  {
      return out.error(rc, FUNCNAME, "Failed to stop all the threads, rc=%d\n",rc);
  }
  return rc;
} 

#endif // CIP_REMOVE_INSTRUCTION_FUNCTIONS



