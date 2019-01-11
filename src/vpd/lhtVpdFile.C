//IBM_PROLOG_BEGIN_TAG
/* 
 * eCMD for pdbg Project
 *
 * Copyright 2015,2018 IBM International Business Machines Corp.
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
#include <stdio.h>

#include <lhtVpdFile.H>

//---------------------------------------------------------------------
//  Constructors
//---------------------------------------------------------------------
lhtVpdFile::lhtVpdFile() : lhtVpd()  // Default constructor
{
}

//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
lhtVpdFile::~lhtVpdFile()
{
}

//---------------------------------------------------------------------
//  Public Member Function Specifications
//---------------------------------------------------------------------


uint32_t lhtVpdFile::read(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & o_data) {
  uint32_t rc = 0;

  // Set the output size
  rc = o_data.setByteLength(i_length);
  if (rc) {
    return rc;
  }

  // Copy the data
  rc = o_data.insert(vpdImage, 0, (i_length * 8), (io_offset * 8));
  if (rc) {
    return rc;
  }

  // Increment the offset after the read
  io_offset += i_length;

  return rc;
}

uint32_t lhtVpdFile::write(uint32_t & io_offset, uint32_t i_length, const ecmdDataBuffer & i_data) {
  uint32_t rc = 0;

  // Insert the data
  rc = vpdImage.insert(i_data, (io_offset * 8), (i_length * 8));
  if (rc) {
    return rc;
  }

  // Increment the offset after the write
  io_offset += i_length;

  return rc;
}

uint32_t lhtVpdFile::setImage(const ecmdDataBuffer & i_vpdImage) {
  uint32_t rc = 0;

  vpdImage = i_vpdImage;

  return rc;
}

uint32_t lhtVpdFile::getImage(ecmdDataBuffer & o_vpdImage) {
  uint32_t rc = 0;

  o_vpdImage = vpdImage;

  return rc;
}
