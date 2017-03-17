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

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#define pdbgOutput_C

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "pdbgCommon.H"
#include "pdbgOutput.H"
#include "pdbgReturnCodes.H"
#include "ecmdDllCapi.H"  ///< To register errors with the plugin
#include "ecmdSharedUtils.H"

#undef pdbgOutput_C

//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------
// From ecmdDllCapi.C
extern uint32_t ecmdGlobal_quiet;

//---------------------------------------------------------------------
// Member Function Specifications
//---------------------------------------------------------------------
pdbgOutput::pdbgOutput() {
  printmode = PRINT_UNIX;
}

pdbgOutput::pdbgOutput(int mode) {
  printmode = mode;
}

pdbgOutput::~pdbgOutput() {
}

void pdbgOutput::setmode(int mode) {
  printmode = mode;
}

void pdbgOutput::print(const char* printMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, printMsg);

  print(false, printMsg, arg_ptr);

  va_end(arg_ptr);
}

void pdbgOutput::debugPrint(const char* printMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, printMsg);

  print(true, printMsg, arg_ptr);

  va_end(arg_ptr);
}

void pdbgOutput::print(bool i_debug, const char* printMsg, va_list &arg_ptr) {

  if (printMsg == NULL || printMsg[0] == '\0') {
    printf("ERROR: (pdbgOutput:print): ill-formated str\n");
  }

  /* Print to the screen */
  if (printmode == PRINT_UNIX || printmode == PRINT_DOS) {
    vprintf(printMsg, arg_ptr);
  } else if (printmode == PRINT_NONE) {
    /* Printing has been turned off */
  } else {
    printf("ERROR: (output): Undefined Print Mode\n");
  }
}

uint32_t pdbgOutput::error(uint32_t rc, ecmdChipTarget& i_target, std::string functionName, const char* errMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, errMsg);

  error(rc, &i_target, functionName.c_str(), errMsg, arg_ptr);

  va_end(arg_ptr);
  return rc;
}

uint32_t pdbgOutput::error(uint32_t rc, ecmdChipTarget* i_target, const char* functionName, const char* errMsg, va_list &arg_ptr) {

  std::string errString;
  /* We want to make sure that we turn on printing if it was off */
  int oldmode = printmode;
  if (printmode == PRINT_NONE) printmode = PRINT_UNIX;
  /* Build the string */
  errString = "ERROR: (";
  errString += functionName;
  errString += "): ";
  
  // If the target exists, include in the output
  if (i_target != NULL) {
    errString += "(";
    errString += ecmdWriteTarget(*i_target, ECMD_DISPLAY_TARGET_COMPRESSED);
    errString += "): ";
  }

  // Add in the error message
  errString += errMsg;

  /* If we have a unique RC, register errors instead of printing */
  /* If we don't have an RC, just output the error to the screen */
  if (rc == PDBG_OUT_ERROR_DO_NOT_USE) {
    print(false, errString.c_str(), arg_ptr);
  } else {
    int numBytes;
    char* errorBuf = new char[502];
    /* Use vsnprintf so we don't have to worry about blowing a buffer */
    numBytes = vsnprintf(errorBuf, 500, errString.c_str(), arg_ptr);
    
    /* Longer than expected, we need to allocate a properly sized buffer and try again */
    if (numBytes > 500) {
      printf("In numbytes\n");
      delete[] errorBuf;
      errorBuf = new char[numBytes + 10];
      vsprintf(errorBuf, errString.c_str(), arg_ptr);
    }
    
    // Now register the message and cleanup
    dllRegisterErrorMsg(rc, "PDBG", errorBuf);
    delete[] errorBuf;
    
    // Register the target too if we have one
    if (i_target != NULL) {
      dllRegisterErrorTarget(rc, *i_target);
    }

    printmode = oldmode;
  }

  return rc;
}

uint32_t pdbgOutput::error(uint32_t rc, std::string functionName, const char* errMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, errMsg);

  error(rc, NULL, functionName.c_str(), errMsg, arg_ptr);

  va_end(arg_ptr);
  return rc;
}

void pdbgOutput::error(std::string functionName, const char* errMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, errMsg);

  error(PDBG_OUT_ERROR_DO_NOT_USE, NULL, functionName.c_str(), errMsg, arg_ptr);

  va_end(arg_ptr);
}


void pdbgOutput::warning(std::string functionName, const char* warnMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, warnMsg);

  warning(functionName.c_str(), warnMsg, arg_ptr);

  va_end(arg_ptr);
}

void pdbgOutput::warning(const char* functionName, const char* warnMsg, va_list &arg_ptr) {
  std::string warnString;

  /* We want to make sure that we turn on printing if it was off */
  int oldmode = printmode;
  if (printmode == PRINT_NONE) printmode = PRINT_UNIX;

  /* Build the string */
  warnString = "WARN: (";
  warnString += functionName;
  warnString += "): ";
  warnString += warnMsg;

  /* Now print it */
  print(false, warnString.c_str(), arg_ptr);

  printmode = oldmode;
}

void pdbgOutput::note(std::string functionName, const char* noteMsg, ...) {
  va_list arg_ptr;
  va_start(arg_ptr, noteMsg);

  note(functionName.c_str(), noteMsg, arg_ptr);

  va_end(arg_ptr);
}

void pdbgOutput::note(const char* functionName, const char* noteMsg, va_list &arg_ptr) {
  std::string noteString;

  /* Build the string */
  noteString = "NOTE: (";
  noteString += functionName;
  noteString += "): ";
  noteString += noteMsg;

  /* Now print it */
  print(false, noteString.c_str(), arg_ptr);
}

// Declaration of the output class that is used globally
pdbgOutput out;
