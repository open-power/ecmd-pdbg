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

#ifndef ecmdMapSpr2String_H
#define ecmdMapSpr2String_H

const typedef struct {  
 uint32_t sprId;
 const char sprStr[16]; 
} ecmdSprId2SprStrTable_t ;

/**
 * @brief Returns the SPR register name for the specified HASH ID
 * @param[in] i_sprId ID of the specific SPR register.
 * @param[out] o_SprName SPR register which gets filled for passed in spr id
 */
uint32_t ecmdMapSpr2Str(uint32_t &i_sprId,std::string &o_SprName);

#endif
