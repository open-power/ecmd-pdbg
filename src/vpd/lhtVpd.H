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

#ifndef lhtVpd_H 
#define lhtVpd_H

/**
 * @file lhtVpd.H
 * @brief Top level class to handle VPD in the Linux Host Tool
*/

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <inttypes.h>
#include <string>
#include <map>

#include <ecmdDataBuffer.H>
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
 *  @brief  Base class for LHT VPD
 *     
 */
class lhtVpd {
public:

  /** @name lhtVpd Constructors */
  //@{
  /**
   * @brief Default Constructor
   */
  lhtVpd();

  /**
   * @brief Default Destructor
   */
  virtual ~lhtVpd();
  //@}

  // Member Functions
  /** @name VPD Access Functions */
  //@{
  /**
   * @brief Get VPD data by keyword
   */
  uint32_t getKeyword(std::string i_recordName, std::string i_keyword, ecmdDataBuffer & o_data);

  /**
   * @brief Put VPD data by keyword
   */
  uint32_t putKeyword(std::string i_recordName, std::string i_keyword, const ecmdDataBuffer & i_data);

private:
  /**
   * @brief Structure representing the format of keyword
   */
  struct keywordInfo {
    uint16_t length;
    uint32_t dataOffset;  // Where in the VPD the data starts, used for writes
  };

  /**
   * @brief Structure representing the format of each record 
   *        in a table of contents entry
   */
  struct recordInfo {
    // All uint16 fields are stored in little endian.
    uint16_t recordType;
    uint16_t recordOffset;
    uint16_t recordLength;
    uint16_t eccOffset;
    uint16_t eccLength;
    uint16_t lastKeywordOffset;
    std::map<std::string, keywordInfo> keywordCache;
    std::map<std::string, ecmdDataBuffer> keywordDataCache;
  };

  /**
   * @brief Read the TOC and generate a cache of the records contained
   */
  uint32_t recordCacheInit(void);

  /**
   * @brief Find information about keyword in VPD
   */
  uint32_t findKeywordInfo(const std::string & i_recordName, const std::string & i_keyword, keywordInfo & o_keywordEntry);

  /**
   * @brief Find VPD data by keyword
   */
  uint32_t findKeywordData(const std::string & i_recordName, const std::string & i_keyword, ecmdDataBuffer & o_data);

  /**
   * @brief Read VPD based on offset and processing entire TOC
   */
  uint32_t readToc(uint32_t & io_offset, std::string i_recordName);

  /**
   * @brief Read VPD based on offset and processing entire record
   */
  uint32_t readRecord(const std::string & i_recordName, recordInfo & io_recordEntry, const std::string & i_keyword, keywordInfo & o_keywordEntry);

  /**
   * @brief Read VPD based on offset and processing entire keyword
   */
  uint32_t readKeyword(uint32_t & io_offset, std::string & o_keyword, keywordInfo & o_keywordEntry, ecmdDataBuffer * o_data);

  /**
   * @brief Updated the cached keyword data
   */
  uint32_t updateKeywordCache(std::string i_recordName, std::string i_keyword, const ecmdDataBuffer & i_data);

  /**
   * @brief Regenerate ECC of the record
   */
  uint32_t updateRecordEcc(std::string & i_recordName);

  /**
   * @brief Read VPD based on offset and length
   */
  virtual uint32_t read(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & o_data);

  /**
   * @brief Write VPD based on offset and length
   */
  virtual uint32_t write(uint32_t & io_offset, uint32_t i_length, const ecmdDataBuffer & i_data);

  /**
   * @brief Generate ecc based upon input data
   */
  virtual uint32_t createEcc(const ecmdDataBuffer & i_data, ecmdDataBuffer & io_ecc);
  //@}

  /**
   * @brief Definition of VPD field sizes
   */
  enum {
    RECORD_BYTE_SIZE             = 4,
    RECORD_ADDR_BYTE_SIZE        = 2,
    KEYWORD_BYTE_SIZE            = 2,
    KEYWORD_LENGTH_BYTE_SIZE     = 1,
    HASHKEYWORD_LENGTH_BYTE_SIZE = 2,
    RECORD_TOC_UNUSED            = 2,
    RT_SKIP_BYTES                = 3,
    VHDR_ECC_DATA_SIZE           = 11,
    VHDR_RESOURCE_ID_SIZE        = 1,
  };

  std::map<std::string, recordInfo> recordCache;
};


#endif //lhtVpd_H

