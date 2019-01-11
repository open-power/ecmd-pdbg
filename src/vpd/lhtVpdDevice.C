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

#include <lhtVpdDevice.H>
#include <edbgCommon.H>
//---------------------------------------------------------------------
//  Constructors
//---------------------------------------------------------------------
lhtVpdDevice::lhtVpdDevice()  // Default constructor
  : lhtVpd()
{
}

//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
lhtVpdDevice::~lhtVpdDevice()
{
}

//---------------------------------------------------------------------
//  Public Member Function Specifications
//---------------------------------------------------------------------
uint32_t lhtVpdDevice::openDevice(std::string i_device) {
  uint32_t rc = 0;

  // Open the device basd on the string passed in
  vpdDevice.open(i_device.c_str(), std::ios_base::in|std::ios_base::out);

  // If the open failed, error
  if (vpdDevice.fail()) {
    return out.error(LHT_VPD_OPEN_FAILED, FUNCNAME, "The open of %s failed!\n", i_device.c_str());
  }

  // Make sure we are at the start of the file, then we are done
  vpdDevice.seekp(std::ios::beg);

  return rc;
}

uint32_t lhtVpdDevice::closeDevice() {
  uint32_t rc = 0;

  vpdDevice.close();

  return rc;
}

uint32_t lhtVpdDevice::read(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & o_data) {
  uint32_t rc = 0;

  // Advance to the current offset
  vpdDevice.seekp(io_offset);

  // Set the output size
  rc = o_data.setByteLength(i_length);
  if (rc) {
    return rc;
  }

  // Copy the data
  rc = o_data.readFileStream(vpdDevice, (i_length * 8));
  if (rc) {
    return rc;
  }

  // Increment the offset after the read
  io_offset += i_length;

  return rc;
}

uint32_t lhtVpdDevice::write(uint32_t & io_offset, uint32_t i_length, const ecmdDataBuffer & i_data) {
  uint32_t rc = 0;

  // Advance to the current offset
  vpdDevice.seekp(io_offset);

  // Get the data out of the buffer
  char buffer[i_length];
  i_data.memCopyOut((uint8_t*)buffer, (i_length * 8));

  // Insert the data
  vpdDevice.write(buffer, i_length);
  if (vpdDevice.fail()) {
    return out.error(LHT_VPD_WRITE_FAILED, FUNCNAME, "Write of data to the device failed!\n");
  }

  // Increment the offset after the write
  io_offset += i_length;

  return rc;
}
