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

#ifndef pdbgReturnCodes_H
#define pdbgReturnCodes_H

// If not defined from including ecmdReturnCodes.H, define it here
#ifndef ECMD_ERR_PDBG
 #define ECMD_ERR_PDBG                           0x40000000 ///< Error came from pdbg
#endif

// Our catch all error code
#define PDBG_GENERAL_ERROR                       (ECMD_ERR_PDBG | 0xFFFFFF)

// Special RC indicating not to use out.error
#define PDBG_OUT_ERROR_DO_NOT_USE                (ECMD_ERR_PDBG | 0xAAAAAA)

/******************************/
/* DLL Interface Functions    */
/******************************/
#define PDBG_INIT_ERROR                          (ECMD_ERR_PDBG | 0x100001)
#define PDBG_CNFG_MISSING                        (ECMD_ERR_PDBG | 0x100003)
#define PDBG_CNFG_FORMAT_ERROR                   (ECMD_ERR_PDBG | 0x100005)

/********************/
/* Object Errors    */
/********************/
#define PDBG_OBJECT_DOESNT_EXIST                (ECMD_ERR_PDBG | 0x300001)
#define PDBG_OBJECT_ALREADY_EXISTS              (ECMD_ERR_PDBG | 0x300003)
#define PDBG_VARIABLE_NOT_SET                   (ECMD_ERR_PDBG | 0x300005)

#endif /* pdbgReturnCodes_H */


