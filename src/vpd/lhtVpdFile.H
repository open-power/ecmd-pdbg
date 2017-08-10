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

#ifndef lhtVpdFile_H 
#define lhtVpdFile_H

/**
 * @file lhtVpdFile.H
 * @brief Sub class to handle talking to VPD via an image file
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <inttypes.h>

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
 *  @brief  VPD file subclass of LHT VPD
 *     
 */
class lhtVpdFile : public lhtVpd {
public:

  /** @name lhtVpdFile Constructors */
  //@{
  /**
   * @brief Default Constructor
   */
  lhtVpdFile();

  /**
   * @brief Default Destructor
   */
  virtual ~lhtVpdFile();
  //@}

  // Member Functions


  /** @name VPD Access Functions */
  //@{
  /**
   * @brief Set the VPD image into the class
   */
  uint32_t setImage(ecmdDataBuffer & i_data);

  /**
   * @brief Get the  VPD image out of the class
   */
  uint32_t getImage(ecmdDataBuffer & o_data);
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
  ecmdDataBuffer vpdImage;

};


#endif //lhtVpdFile_H

