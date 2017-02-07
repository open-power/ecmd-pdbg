// COPYRIGHT_START
// ****************************************
// File: lhtVpdFile.H
//
// (C) Copyright IBM Corporation 2014, 2014
// ****************************************
// COPYRIGHT_END
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

