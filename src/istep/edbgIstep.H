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
#ifndef EDBGISTEP_H
#define EDBGISTEP_H

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <string>
#include <map>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <experimental/filesystem>

// Headers from ecmd-pdbg
#include <edbgCommon.H>
#include <edbgOutput.H>

class edbgIPLTable
{
public:

  // The length of istep name is limited to this size.
  // The istep names are known at this time. It has been manually verfied that the 
  // length of all the istep names are below 42. 
  enum { EDBG_FUNC_NAME_SIZE = 42 }; 

  // Starting istep number
  enum { EDBG_FIRST_ISTEP_NUM = 0 };

  // Ending istep number
  // Though istep numbers are from 0 to 26 in the IPL table, istep execution is
  // restricted to 21 as the remaining isteps are meant for debug purposes.
  enum { EDBG_LAST_ISTEP_NUM  = 21 };

  // This istep number is reserved for representing an invalid istep number.
  enum { EDBG_INVALID_ISTEP_NUM = 0xFFFF };

  // This position is used for representing an invalid row number in the 
  // edbgIPLTable
  enum { EDBG_INVALID_POSITION = 0xFFFF };

  typedef enum {
  EDBG_ISTEP_HOST = 0x0,     // IStep is executed by host
  EDBG_ISTEP_SBE ,           // IStep is executed by Self Boot Engine
  EDBG_ISTEP_BMC,            // IStep is executed by BMC  
  EDBG_ISTEP_NOOP,           // This istep is NOOP
  EDBG_ISTEP_INVALID_DESTINATION 
  } edbgIStepDestination_t;

  struct edbgIStep_t
  {
    uint16_t        majorNum ;          //Major Number
    uint16_t        minorNum ;          //Minor Number        
    const  char     istepName[EDBG_FUNC_NAME_SIZE];  // IPL Step Name
    // Indicates whether this IStep is executed by HOST/SBE/SP
    edbgIStepDestination_t  destination;      
  };


  /**
    * @brief Constructor.
    * 
    * Right now, no need to initialize anything in the default constructor.
    * 
    */
  edbgIPLTable() {};

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep number
   *
   * @par Description:
   *     This routine gets the istep number for 
   *     the specified istep name.
   *
   * @param[in]   i_istepName   Name of the istep
   * @param[out]  o_majorNum    Major istep number
   * @param[out]  o_minorNum    Minor istep number
   *
   * @return false if the given istep name is an invalid one
   */

  bool getIStepNumber(  const std::string & i_istepName,
                        uint16_t & o_majorNum,
                        uint16_t & o_minorNum );

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Validate the istep name
   *
   * @par Description:
   *     This routine checks whether the given
   *     the istep name exists.
   *
   * @param[in] i_istepName    Name of the istep
   *
   * @return true if the given istep name exists
   *         false, otherwise.
   */
  bool isValid(const std::string & i_istepName);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Validate the istep number (major)
   *
   * @par Description:
   *     This routine checks whether the given
   *     the istep number (major) exists.
   *
   * @param[in] i_majorNum  Major number of the istep
   *
   * @return true if the given istep major number exists
   *         false, otherwise.
   */

  bool isValid(uint16_t i_majorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep name
   *
   * @par Description:
   *     This routine gets the istep name stored at 
   *     the specified array index.
   *
   * @param[in] i_position      Array index
   * @param[out]o_istepName     Name of the istep
   *
   * @return false if the array index is out of range
   */
  bool getIStepNameOf(uint16_t i_position,std::string & o_istepName);
  
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the array index
   *
   * @par Description:
   *     This routine gets the array index of the first minor
   *     number of the given major number.
   *
   * @param[in] i_majorNum    Major Number
   *
   * @return EDBG_INVALID_POSITION if the given major number is not valid
   */
  uint16_t getPosFirstMinorNumber(uint16_t i_majorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the array index
   *
   * @par Description:
   *     This routine gets the array index of the last minor
   *     number of the given major number.
   *
   * @param[in] i_majorNum     Major Number
   *
   * @return EDBG_INVALID_POSITION if the given major number is not valid
   */
  uint16_t getPosLastMinorNumber(uint16_t i_majorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the array index
   *
   * @par Description:
   *     This routine gets array index of 
   *     the specified istep name.
   *
   * @param[in] i_istepName    Name of the istep
   *
   * @return EDBG_INVALID_POSITION if the given step name is not valid
   */
  uint16_t getPosition(const std::string & i_istepName);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep number
   *
   * @par Description:
   *     This routine gets the istep number stored
   *     at the specified arry index
   *
   * @param[in] i_position    Array index
   *
   * @return EDBG_INVALID_ISTEP_NUM if the array index is out of range
   */
  uint16_t getIStepNumber(uint16_t i_position);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep Minor number
   *
   * @par Description:
   *     This routine gets the istep number stored
   *     at the specified arry index
   *
   * @param[in] i_position    Array index
   *
   * @return EDBG_INVALID_ISTEP_NUM if the array index is out of range
   */
  uint16_t getIStepMinorNumber(uint16_t i_position);
  
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the destination
   *
   * @par Description:
   *     This routine tells who will perform the
   *     the specified istep number
   *
   * @param[in] i_majorNum    Major istep number
   * @param[in] i_minorNum    Minor istep number
   *
   * @return EDBG_ISTEP_INVALID_DESTINATION if the istep number is not valid
   */
  edbgIPLTable::edbgIStepDestination_t getDestination(  uint16_t i_majorNum,
                                                        uint16_t i_minorNum);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the istep name 
   *
   * @par Description:
   *     This routine gets the istep name for the specified istep
   *     major and minor number
   *
   * @param[in]   o_majorNum    Major istep number
   * @param[in]   o_minorNum    Minor istep number
   * @param[out]  i_istepName   Name of the istep
   *
   * @return false if the given major and minor number is invalid
   */

  bool getIStepName(const uint16_t i_majorNum,
                    const uint16_t i_minorNum,
                    std::string & o_istepName);

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Get the destination
   *
   * @par Description:
   *     This routine tells who will perform the
   *     istep available at the position i_position
   *
   * @param[in] i_position    Index to the IPL Table
   *
   * @return EDBG_ISTEP_INVALID_DESTINATION if the position is invalid
   */

  edbgIPLTable::edbgIStepDestination_t 
                                getDestination(uint16_t i_position);
  
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Istep poweron
   *
   * @par Description:
   *     This routine  triggers istep poweron (obmcutil recoveryoff->chassison)
   *     
   *
   * @return errno as return code on failure else ECMD_SUCCESS
   */

  int istepPowerOn();

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  /**
   * @brief Chassis on/off check
   *
   * @par Description:
   *     This routine checks if the chassis power is on / off
   *     
   *
   * @return true if power is on else false
   */

  bool isChassisOn();
private:

    // Disabled copy constructor and assignment operator

    edbgIPLTable(const edbgIPLTable& right);

    edbgIPLTable & operator=(const edbgIPLTable& right);

    static const edbgIStep_t cv_edbgIStepTable[];

    // Number of isteps. 
    // This number represents the number of rows in the IPL table
    // The number of isteps in the IPL Table are calculated based
    // on the size of the table divided by the size of a row
    static const uint16_t EDBG_NUMBER_OF_ISTEPS;
};


#endif  // end of define for edbgIStep_H

