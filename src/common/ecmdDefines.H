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

#ifndef ecmdDefines_H 
#define ecmdDefines_H

/*************************************************************************/
// ecmdDefines.H is a file provided by ecmd to anyone building.  It allows
// the project to set their compile defines in a header that is include
// everywhere insted of on the command line
//
// Since we have a number of different build targets in edbg, it would be
// easier to utilize this mechanism in our builds than big long set of -D
// options on every build
//
// Here we can also define our different edbg build target defines and
// include the ecmd build defines that apply to each type
/*************************************************************************/

// Different hardware access functions from eCMD
// These are all the functions we out right don't support in edbg
// If you need to add support to a build target, move it out of here and
// to the section below
// Base eCMD
#define ECMD_REMOVE_SEDC_SUPPORT
#define ECMD_REMOVE_LATCH_FUNCTIONS
#define ECMD_REMOVE_ARRAY_FUNCTIONS
#define ECMD_REMOVE_SPY_FUNCTIONS
#define ECMD_REMOVE_REFCLOCK_FUNCTIONS
//#define ECMD_REMOVE_PROCESSOR_FUNCTIONS
#define ECMD_REMOVE_GPIO_FUNCTIONS
#define ECMD_REMOVE_I2C_FUNCTIONS
#define ECMD_REMOVE_POWER_FUNCTIONS
#define ECMD_REMOVE_ADAL_FUNCTIONS
#define ECMD_REMOVE_JTAG_FUNCTIONS
#define ECMD_REMOVE_TRACEARRAY_FUNCTIONS
#define ECMD_REMOVE_SENSOR_FUNCTIONS
#define ECMD_REMOVE_BLOCK_FUNCTIONS
#define ECMD_REMOVE_MPIPL_FUNCTIONS
#define ECMD_REMOVE_PNOR_FUNCTIONS
#define ECMD_REMOVE_SP_FUNCTIONS
#define ECMD_REMOVE_UNITID_FUNCTIONS
#define ECMD_REMOVE_RING_FUNCTIONS
#define ECMD_REMOVE_CLOCK_FUNCTIONS
// cip extension
//#define CIP_REMOVE_INSTRUCTION_FUNCTIONS
#define CIP_REMOVE_BREAKPOINT_FUNCTIONS
#define CIP_REMOVE_VR_FUNCTIONS
#define CIP_REMOVE_VSR_FUNCTIONS
#define CIP_REMOVE_PORE_FUNCTIONS
#define CIP_REMOVE_RW_FUNCTIONS
#define CIP_REMOVE_MBOX_FUNCTIONS
#define CIP_REMOVE_PMC_VOLTAGE_FUNCTIONS

// These are the functions we conditionally support, depending upon the build target
// We only have to take action to remove things from the build, so if it is not
// required, we will define it in for that build
#ifdef EDBG_BUILD_PETITBOOT
#define ECMD_REMOVE_SCOM_FUNCTIONS
#define ECMD_REMOVE_FSI_FUNCTIONS
//#define ECMD_REMOVE_VPD_FUNCTIONS
#define ECMD_REMOVE_MEMORY_FUNCTIONS
#define CIP_REMOVE_MEMORY_FUNCTIONS
#define ECMD_REMOVE_INIT_FUNCTIONS
#define CIP_REMOVE_INSTRUCTION_FUNCTIONS
#define ECMD_REMOVE_PROCESSOR_FUNCTIONS
#endif // EDBG_BUILD_PETITBOOT

#ifdef EDBG_BUILD_HOST
//#define ECMD_REMOVE_SCOM_FUNCTIONS
#define ECMD_REMOVE_FSI_FUNCTIONS
#define ECMD_REMOVE_VPD_FUNCTIONS
//#define ECMD_REMOVE_MEMORY_FUNCTIONS
#define CIP_REMOVE_MEMORY_FUNCTIONS
#define ECMD_REMOVE_INIT_FUNCTIONS
#define CIP_REMOVE_INSTRUCTION_FUNCTIONS
#define ECMD_REMOVE_PROCESSOR_FUNCTIONS
#endif // EDBG_BUILD_HOST

#ifdef EDBG_BUILD_EBMC
//#define ECMD_REMOVE_SCOM_FUNCTIONS
//#define ECMD_REMOVE_FSI_FUNCTIONS
//#define ECMD_REMOVE_VPD_FUNCTIONS
//#define ECMD_REMOVE_MEMORY_FUNCTIONS
//#define CIP_REMOVE_MEMORY_FUNCTIONS
//#define ECMD_REMOVE_INIT_FUNCTIONS
//#define CIP_REMOVE_INSTRUCTION_FUNCTIONS
//#define ECMD_REMOVE_PROCESSOR_FUNCTIONS
#endif // EDBG_BUILD_HOST

// These are functions we need in all the time
// Nothing to see here

// Remove extra debug code
// Useful for saving space
#define ECMD_STRIP_DEBUG

// Remove all the simulation code in the build
// Useful for saving space, but have to make sure
// everyone else who is using your plugin built with it off
//#define REMOVE_SIM

#endif
