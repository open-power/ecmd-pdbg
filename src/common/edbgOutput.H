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

#ifndef edbgOutput_H
#define edbgOutput_H

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>
#include <queue>

#include "ecmdDllCapi.H"  
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
#define PRINT_NONE 2
#define PRINT_DOS 3
#define PRINT_UNIX 4

//---------------------------------------------------------------------
// Class Definition
//---------------------------------------------------------------------
class edbgOutput
{
public:
  edbgOutput();
  edbgOutput(int mode);
  ~edbgOutput();

  void setmode(int mode);
  int  getmode() { return printmode; }

  /* PRINT */
  void print(const char* printMsg, ...);
  void debugPrint(const char* printMsg, ...);

  /* ERROR */
  uint32_t error(uint32_t rc, ecmdChipTarget& i_target, std::string functionName, const char* errMsg, ...);
  uint32_t error(uint32_t rc, std::string functionName, const char* errMsg, ...);
  void error(std::string functionName, const char* errMsg, ...);

  /* WARNING */
  void warning(std::string functionName, const char* warnMsg, ...);

  /* NOTE */
  void note(std::string functionName, const char* noteMsg, ...);


private:  // functions
  edbgOutput(edbgOutput &otherout);
  int operator=(edbgOutput &otherout);
  void print(bool i_debug, const char* printMsg, va_list &arg_ptr);
  uint32_t error(uint32_t rc, ecmdChipTarget* i_target, const char* functionName, const char* errMsg, va_list &arg_ptr);
  void warning(const char* functionName, const char* warnMsg, va_list &arg_ptr);
  void note(const char* functionName, const char* noteMsg, va_list &arg_ptr);

private:  // Data
  int printmode;
};

// The __PRETTY_FUNCTION__ macro isn't all that pretty
// This inline function cleans it up into the class::function desired format
// If not called from a class, it also handles that and gives just the function
inline std::string methodName(const std::string & prettyFunction) {
  size_t begin, end;
  size_t colons = prettyFunction.find("::");
  if (colons == std::string::npos) {
    begin = prettyFunction.find(" ") + 1;
    end = prettyFunction.find("(") - begin;
  } else {
    begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    end = prettyFunction.rfind("(") - begin;
  }

  return prettyFunction.substr(begin,end);
}

#define FUNCNAME methodName(__PRETTY_FUNCTION__)

#endif /* edbgOutput_H */
