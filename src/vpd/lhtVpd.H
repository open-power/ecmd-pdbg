// COPYRIGHT_START
// ****************************************
// File: lhtVpd.H
//
// (C) Copyright IBM Corporation 2014, 2014
// ****************************************
// COPYRIGHT_END
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
#include <list>

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
  uint32_t putKeyword(std::string i_recordName, std::string i_keyword, ecmdDataBuffer & i_data);

private:
  /**
   * @brief Structure representing the format of keyword
   */
  struct keywordInfo {
    std::string keyword;
    uint8_t length;
    ecmdDataBuffer data;  // Probably heavyweight for storage, but maps to the interface
    uint32_t dataOffset;  // Where in the VPD the data starts, used for writes
    int operator==(const keywordInfo& rhs) const { return (keyword == rhs.keyword); } 
  };

  /**
   * @brief Structure representing the format of each record 
   *        in a table of contents entry
   */
  struct recordInfo {
    std::string recordName;
    // All uint16 fields are stored in little endian.
    uint16_t recordType;
    uint16_t recordOffset;
    uint16_t recordLength;
    uint16_t eccOffset;
    uint16_t eccLength;
    std::list<keywordInfo> keywordCache;
    int operator==(const recordInfo& rhs) const { return (recordName == rhs.recordName); } 
  };

  /**
   * @brief Read the TOC and generate a cache of the records contained
   */
  uint32_t recordCacheInit(void);

  /**
   * @brief Find VPD data by keyword
   */
  uint32_t findKeyword(std::string & i_recordName, std::string & i_keyword, keywordInfo & o_keywordEntry);

  /**
   * @brief Read VPD based on offset and processing entire TOC
   */
  uint32_t readToc(uint32_t & io_offset, std::string i_recordName);

  /**
   * @brief Read VPD based on offset and processing entire record
   */
  uint32_t readRecord(recordInfo & i_recordEntry, std::string i_keyword, keywordInfo & o_keywordEntry);

  /**
   * @brief Read VPD based on offset and processing entire keyword
   */
  uint32_t readKeyword(uint32_t & io_offset, keywordInfo & o_keywordEntry);

  /**
   * @brief Updated the cached keyword data
   */
  uint32_t updateKeywordCache(std::string i_recordName, std::string i_keyword, ecmdDataBuffer & i_data);

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
  virtual uint32_t write(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & i_data);

  /**
   * @brief Generate ecc based upon input data
   */
  virtual uint32_t createEcc(ecmdDataBuffer & i_data, ecmdDataBuffer & io_ecc);
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

  std::list<recordInfo> recordCache;
};


#endif //lhtVpd_H

