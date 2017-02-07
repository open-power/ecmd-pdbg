// COPYRIGHT_START
// ****************************************
// File: lhtVpdFile.C
//
// (C) Copyright IBM Corporation 2014, 2014
// ****************************************
// COPYRIGHT_END

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

uint32_t lhtVpdFile::write(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & i_data) {
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

uint32_t lhtVpdFile::setImage(ecmdDataBuffer & i_vpdImage) {
  uint32_t rc = 0;

  vpdImage = i_vpdImage;

  return rc;
}

uint32_t lhtVpdFile::getImage(ecmdDataBuffer & o_vpdImage) {
  uint32_t rc = 0;

  o_vpdImage = vpdImage;

  return rc;
}
