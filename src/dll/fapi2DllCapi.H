//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2017 IBM International Business Machines Corp.
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

#ifndef fapi2DllCapi_H
#define fapi2DllCapi_H

#include <inttypes.h>

#include <ecmdDefines.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBufferBase.H>

/*
  This is a stripped down version of fapi2DllCapi.H from the eCMD dirs.
  It provides the fapi2InitExtension code and that is it.
  It should be found first in the search path to override the eCMD one.
*/

extern "C" {

/* Extension initialization function - verifies version */
uint32_t dllFapi2InitExtension(const char * i_version);

/* Extension Specific load function - used by Cronus/GFW to see if extension is supported */
uint32_t dllFapi2InitExtensionInPlugin();

} //extern C

#endif
