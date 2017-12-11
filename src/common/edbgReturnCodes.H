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

#ifndef edbgReturnCodes_H
#define edbgReturnCodes_H

// If not defined from including ecmdReturnCodes.H, define it here
#ifndef ECMD_ERR_EDBG
 #define ECMD_ERR_EDBG                           0x40000000 ///< Error came from edbg
#endif

// Our catch all error code
#define EDBG_GENERAL_ERROR                       (ECMD_ERR_EDBG | 0xFFFFFF)

// Special RC indicating not to use out.error
#define EDBG_OUT_ERROR_DO_NOT_USE                (ECMD_ERR_EDBG | 0xAAAAAA)

/******************************/
/* DLL Interface Functions    */
/******************************/
#define EDBG_INIT_ERROR                          (ECMD_ERR_EDBG | 0x100001)
#define EDBG_CNFG_MISSING                        (ECMD_ERR_EDBG | 0x100003)
#define EDBG_CNFG_FORMAT_ERROR                   (ECMD_ERR_EDBG | 0x100005)
#define EDBG_READ_ERROR                          (ECMD_ERR_EDBG | 0x100007)
#define EDBG_WRITE_ERROR                         (ECMD_ERR_EDBG | 0x100009)

/********************/
/* VPD Functions    */
/********************/
#define LHT_VPD_KEYWORD_NOT_FOUND               (ECMD_ERR_LHT | 0x200001)
#define LHT_VPD_GENERAL_ERROR                   (ECMD_ERR_LHT | 0x200003)
#define LHT_VPD_BAD_TAG                         (ECMD_ERR_LHT | 0x200005)
#define LHT_VPD_BAD_KEYWORD                     (ECMD_ERR_LHT | 0x200007)
#define LHT_VPD_BAD_RECORD                      (ECMD_ERR_LHT | 0x200009)
#define LHT_VPD_OPEN_FAILED                     (ECMD_ERR_LHT | 0x20000B)
#define LHT_VPD_WRITE_FAILED                    (ECMD_ERR_LHT | 0x20000D)
#define LHT_VPD_ECC_UNIMPLEMENTED               (ECMD_ERR_LHT | 0x20000F)

/********************/
/* Object Errors    */
/********************/
#define EDBG_OBJECT_DOESNT_EXIST                (ECMD_ERR_EDBG | 0x300001)
#define EDBG_OBJECT_ALREADY_EXISTS              (ECMD_ERR_EDBG | 0x300003)
#define EDBG_VARIABLE_NOT_SET                   (ECMD_ERR_EDBG | 0x300005)

#endif /* edbgReturnCodes_H */


