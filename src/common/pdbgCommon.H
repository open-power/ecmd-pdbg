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

#ifndef pdbgCommon_H
#define pdbgCommon_H

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <pdbgReturnCodes.H>
#include <pdbgOutput.H>

/* --------------------------------------- */
/* Internal global variable for ECMD_HOME  */
/* --------------------------------------- */
extern std::string gECMD_HOME;  // The variable is defined in pdbgEcmdDll.C

/* --------------------------------------- */
/* Internal global variable for PDBG_HOME  */
/* --------------------------------------- */
extern std::string gPDBG_HOME;  // The variable is defined in pdbgEcmdDll.C

/* -------------------------------------- */
/* Output Class used for all pdbg output  */
/* -------------------------------------- */
extern pdbgOutput out;  // The variable is defined at the bottom of pdbgOutput.C

#endif /* pdbgCommon_H */
