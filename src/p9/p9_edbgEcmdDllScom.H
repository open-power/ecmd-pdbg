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
#ifndef p9_edbgEcmdDllScom_H
#define p9_edbgEcmdDllScom_H

uint32_t p9_dllQueryScom(const ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail);

uint32_t p9_dllGetScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data);

uint32_t p9_dllPutScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data);

#endif
