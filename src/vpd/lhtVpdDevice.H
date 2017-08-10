//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2015,2017 IBM International Business Machines Corp.
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

#ifndef lhtVpdDevice_H 
#define lhtVpdDevice_H

/**
 * @file lhtVpdDevice.H
 * @brief Sub class to handle talking to VPD via the linux device
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <inttypes.h>
#include <fstream>

#include <lhtVpd.H>
//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------

//--------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------

//----------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------

//--------------------------------------------------------------------
//  Defines                                                
//--------------------------------------------------------------------

//----------------------------------------------------------------------
//  User Types
//----------------------------------------------------------------------


/**
 *  @brief VPD device subclass for LHT VPD
 *     
 */
class lhtVpdDevice : public lhtVpd {
public:

  /** @name lhtVpdDevice Constructors */
  //@{
  /**
   * @brief Default Constructor
   */
  lhtVpdDevice();

  /**
   * @brief Default Destructor
   */
  virtual ~lhtVpdDevice();
  //@}

  /** @name VPD Access Functions */
  //@{
  /**
   * @brief Open the VPD device to access
   */
  uint32_t openDevice(std::string i_device);

  /**
   * @brief Open the VPD device to access
   */
  uint32_t closeDevice();
  //@}

private:
  /** @name VPD Access Functions */
  //@{
  /**
   * @brief Read VPD based on offset and length
   */
  uint32_t read(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & o_data);

  /**
   * @brief Write VPD based on offset and length
   */
  uint32_t write(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & i_data);
  //@}

private:
  std::fstream vpdDevice;

};


#endif //lhtVpdDevice_H

