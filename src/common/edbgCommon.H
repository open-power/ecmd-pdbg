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

#ifndef edbgCommon_H
#define edbgCommon_H

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <edbgReturnCodes.H>
#include <edbgOutput.H>

/* --------------------------------------- */
/* Internal global variable for ECMD_HOME  */
/* --------------------------------------- */
extern std::string gECMD_HOME;  // The variable is defined in edbgEcmdDll.C

/* --------------------------------------- */
/* Internal global variable for EDBG_HOME  */
/* --------------------------------------- */
extern std::string gEDBG_HOME;  // The variable is defined in edbgEcmdDll.C

/* -------------------------------------- */
/* Output Class used for all edbg output  */
/* -------------------------------------- */
extern edbgOutput out;  // The variable is defined at the bottom of edbgOutput.C

/* -------------------------------------- */
/* Map ecmd core to a pdbg core target    */
/* -------------------------------------- */
uint32_t mapEcmdCoreToPdbgCoreTarget(const ecmdChipTarget & i_target, struct pdbg_target **o_target);

/* --------------------------------------- */
/* Probe child target based on avilability */ 
/* of parent                               */
/* --------------------------------------- */
uint32_t  probeChildTarget(struct pdbg_target *i_target, 
                           std::string i_pTarget_name, std::string i_cTarget_name);

/* --------------------------------------- */
/* Get Fapi Unit Position for given target */ 
/* --------------------------------------- */
uint8_t getFapiUnitPos(pdbg_target *target);


#endif /* edbgCommon_H */
