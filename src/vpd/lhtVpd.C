// COPYRIGHT_START
// ****************************************
// File: lhtVpd.C
//
// (C) Copyright IBM Corporation 2014, 2014
// ****************************************
// COPYRIGHT_END

//----------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------
#include <stdio.h>
#include <algorithm>

#include <lhtVpd.H>
#include <lhtCommon.H>
//---------------------------------------------------------------------
//  Constructors
//---------------------------------------------------------------------
lhtVpd::lhtVpd()  // Default constructor
{
}

//---------------------------------------------------------------------
//  Destructor
//---------------------------------------------------------------------
lhtVpd::~lhtVpd()
{
}

//---------------------------------------------------------------------
//  Public Member Function Specifications
//---------------------------------------------------------------------
uint32_t lhtVpd::getKeyword(std::string i_recordName, std::string i_keyword, ecmdDataBuffer & o_data) {
  uint32_t rc = 0;
  keywordInfo keywordEntry;

  // Force case
  transform(i_recordName.begin(), i_recordName.end(), i_recordName.begin(), toupper);
  transform(i_keyword.begin(), i_keyword.end(), i_keyword.begin(), toupper);

  // Find the keyword in VPD
  rc = findKeyword(i_recordName, i_keyword, keywordEntry);
  if (rc) {
    return rc;
  }

  // Set the output buffer big enough for all the data
  // The caller can manipulate it from there
  o_data.setByteLength(keywordEntry.length);

  // Copy over our data
  o_data.insert(keywordEntry.data, 0, (keywordEntry.length * 8));

  return rc;
}

uint32_t lhtVpd::putKeyword(std::string i_recordName, std::string i_keyword, ecmdDataBuffer & i_data) {
  uint32_t rc = 0;

  keywordInfo keywordEntry;

  // Force case
  transform(i_recordName.begin(), i_recordName.end(), i_recordName.begin(), toupper);
  transform(i_keyword.begin(), i_keyword.end(), i_keyword.begin(), toupper);

  // Find the keyword in VPD
  rc = findKeyword(i_recordName, i_keyword, keywordEntry);
  if (rc) {
    return rc;
  }

  // Make sure we aren't trying write more than the keyword can handle
  if (i_data.getByteLength() > keywordEntry.length) {
    out.note(FUNCNAME, "Given data length (%d) is longer than keyword (%d).  The write will be truncated to keyword length\n", i_data.getByteLength(), keywordEntry.length);
    i_data.shrinkBitLength((keywordEntry.length * 8));
  }

  // Do the write here - this is why we need the offset stored in the keyword
  rc = write(keywordEntry.dataOffset, i_data.getByteLength(), i_data);
  if (rc) {
    return rc;
  }

  // Copy new data to keyword cache
  rc = updateKeywordCache(i_recordName, i_keyword, i_data);
  if (rc) {
    return rc;
  }

  // Update ECC for record
  rc = updateRecordEcc(i_recordName);
  if (rc) {
    return rc;
  }

  return rc;
}

uint32_t lhtVpd::recordCacheInit(void) {
  uint32_t rc = 0;
  std::map<std::string, recordInfo>::iterator findRecordIter;

  // The cache will either be empty, or full populated with record entries from the TOC
  // There is no partial read of the table of contents up to just the recordName we need
  // So, if the cache is empty, we need to populate it
  if (recordCache.empty()) {

    uint32_t offset = 0;

    // **************
    // VHDR
    // **************

    // Skip the starting ECC, we don't care about that for now
    offset += VHDR_ECC_DATA_SIZE;

    // Read the VHDR
    rc = readToc(offset, "VHDR");
    if (rc) {
      return out.error(rc, FUNCNAME, "Unable to read the VHDR\n");
    }

    // **************
    // VTOC
    // **************
    findRecordIter = recordCache.find("VTOC");
    if (findRecordIter == recordCache.end()) {
      return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Unable to find VTOC in recordCache\n");
    }

    // Get the VHDR out of the tocRecords instead of as a returned value
    // Jump ahead to the VTOC
    offset = findRecordIter->second.recordOffset;

    // Read the VTOC
    rc = readToc(offset, "VTOC");
    if (rc) {
      return out.error(rc, FUNCNAME, "Unable to read the VTOC\n");
    }
  }

  return rc;
}

uint32_t lhtVpd::findKeyword(const std::string & i_recordName, const std::string & i_keyword, keywordInfo & o_keywordEntry) {
  uint32_t rc = 0;
  std::map<std::string, recordInfo>::iterator findRecordIter;

  // Initialize the record cache
  rc = recordCacheInit();
  if (rc) {
    return rc;
  }

  // Cache is populated, get the record we need
  findRecordIter = recordCache.find(i_recordName);
  if (findRecordIter == recordCache.end()) {
    return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Unable to find %s in recordCache\n", i_recordName.c_str());
  }

  // I've gotten my record either from the VPD or the cache
  // Now read the record and look for the keyword
  rc = readRecord(i_recordName, findRecordIter->second, i_keyword, o_keywordEntry);
  if (rc) {
    return rc;
  }

  return rc;
}

uint32_t lhtVpd::updateKeywordCache(std::string i_recordName, std::string i_keyword, ecmdDataBuffer & i_data) {
  uint32_t rc = 0;
  std::map<std::string, recordInfo>::iterator findRecordIter;
  std::map<std::string, keywordInfo>::iterator findIter;

  // Initialize the record cache
  rc = recordCacheInit();
  if (rc) {
    return rc;
  }

  // Cache is populated, get the record we need
  findRecordIter = recordCache.find(i_recordName);
  if (findRecordIter == recordCache.end()) {
    return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Unable to find %s in recordCache\n", i_recordName.c_str());
  }
  recordInfo & recordEntry = findRecordIter->second;

  // I've gotten my record either from the VPD or the cache
  // Now see if the cache contains keyword
  findIter = recordEntry.keywordCache.find(i_keyword);

  // We found it in the cache, so update the value
  if (findIter != recordEntry.keywordCache.end()) {
    // Make sure we don't try to write over end of the buffer
    uint32_t writeLength = i_data.getBitLength();
    if ((findIter->second.length * 8) < writeLength) {
      writeLength = findIter->second.length * 8;
    }
    rc = findIter->second.data.insert(i_data, 0, writeLength);
    if (rc) {
      return rc;
    }
  } else {
    return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Unable to find %s in cache for %s\n", i_keyword.c_str(), i_recordName.c_str());
  }

  return rc;
}


uint32_t lhtVpd::readToc(uint32_t & io_offset, std::string i_recordName) {
  uint32_t rc = 0;
  ecmdDataBuffer readData;
  std::string keyword;
  keywordInfo keywordData;

  // Large resource tag, should be 0x84
  rc = read(io_offset, 1, readData);
  if (rc) return rc;

  // Error check
  if (readData.getByte(0) != 0x84) {
    return out.error(LHT_VPD_BAD_TAG, FUNCNAME, "Large resource tag value is bad: 0x%02X\n", readData.getByte(0));
  }

  // Skip the record length for now
  io_offset +=2;

  // Read the RT keyword, make sure we get the VHDR
  rc = readKeyword(io_offset, keyword, keywordData);
  if (rc) {
    return rc;
  }
  if (keywordData.data.genAsciiStr() != i_recordName) {
    return out.error(LHT_VPD_BAD_RECORD, FUNCNAME, "Looking for record %s, but found record %s\n", i_recordName.c_str(), keywordData.data.genAsciiStr().c_str());
  }

  // Skip the version data (VD) in the VHDR for now
  if (i_recordName == "VHDR") {
    io_offset += 5;
  }

  // Read the PT keyword to get the table of contents
  // The data of the keyword gets decomposed down into the TOC entries
  // The max length of the PT is 255.  If more TOC data is needed than that, the PT keyword is repeated
  // Set up the code to handle that

  // Do an initial read to seed the loop
  rc = readKeyword(io_offset, keyword, keywordData);
  if (rc) {
    return rc;
  }

  while (keyword == "PT") {
    // Turn the keywordData returned into a series of record entries
    recordInfo recordEntry;
    // Each record entry is 14 bytes long and are repeated in a row.  Length should be divisible by 14
    for (uint32_t x = 0; x < keywordData.length; x+=14) {
      // init
      recordEntry.recordType = recordEntry.recordOffset = recordEntry.recordLength = recordEntry.eccOffset = recordEntry.eccLength = 0x0;
      // set
      std::string recordName = keywordData.data.genAsciiStr((x*8), (4*8));
      recordEntry.recordType = keywordData.data.getByte((x+4)) | (keywordData.data.getByte((x+5)) << 8);
      recordEntry.recordOffset = keywordData.data.getByte((x+6)) | (keywordData.data.getByte((x+7)) << 8);
      recordEntry.recordLength = keywordData.data.getByte((x+8)) | (keywordData.data.getByte((x+9)) << 8);
      recordEntry.eccOffset = keywordData.data.getByte((x+10)) | (keywordData.data.getByte((x+11)) << 8);
      recordEntry.eccLength = keywordData.data.getByte((x+12)) | (keywordData.data.getByte((x+13)) << 8);
    
      recordCache[recordName] = recordEntry;
    }

    // Seed the next while check.  
    // It will either be another PT keyword or an unused PF keyword
    rc = readKeyword(io_offset, keyword, keywordData);
    if (rc) {
      return rc;
    }
  }

  // We should be at the end of the record, make sure we have the small resource tag
  // Small resource tag, should be 0x78
  rc = read(io_offset, 1, readData);
  if (rc) {
    return rc;
  }
  
  // Error check
  if (readData.getByte(0) != 0x78) {
    return out.error(LHT_VPD_BAD_TAG, FUNCNAME, "Small resource tag value is bad: 0x%02X\n", readData.getByte(0));
  }

  return rc;
}

uint32_t lhtVpd::readRecord(const std::string & i_recordName, recordInfo & io_recordEntry, const std::string & i_keyword, keywordInfo & o_keywordEntry) {
  uint32_t rc = 0;
  ecmdDataBuffer readData;
  uint32_t offset = io_recordEntry.recordOffset;
  std::string keyword;
  keywordInfo keywordEntry;
  std::map<std::string, keywordInfo>::iterator findIter;

  // Look to see if the keyword we need is already in the cache
  // If so, return that.  If not, read the record until we find it
  findIter = io_recordEntry.keywordCache.find(i_keyword);

  // We found it in the cache, so set our return value and bail
  if (findIter != io_recordEntry.keywordCache.end()) {
    o_keywordEntry = findIter->second;
    return 0;    
  }

  // Not found, so read it out and load it into the cache at the end
  
  // Large resource tag, should be 0x84
  rc = read(offset, 1, readData);
  if (rc) {
    return rc;
  }
  
  // Error check
  if (readData.getByte(0) != 0x84) {
    return out.error(LHT_VPD_BAD_TAG, FUNCNAME, "Large resource tag value is bad: 0x%02X\n", readData.getByte(0));
  }

  // Get the record length
  rc = read(offset, 2, readData);
  if (rc) {
    return rc;
  }

  uint16_t recordLength = 0x0;
  recordLength = readData.getByte(0) | readData.getByte(1) << 8;
  uint32_t readStart = offset;
  
  // Error check the RT keyword to make sure we got the right record
  rc = readKeyword(offset, keyword, keywordEntry);
  if (rc) {
    return rc;
  }

  if (keyword == "RT") {
    if (keywordEntry.data.genAsciiStr() != i_recordName) {
      return out.error(LHT_VPD_BAD_KEYWORD, FUNCNAME, "RT keyword record didn't match.  Expected: %s, found: %s\n", i_recordName.c_str(), keywordEntry.data.genAsciiStr().c_str());
    }
  } else {
    return out.error(LHT_VPD_BAD_KEYWORD, FUNCNAME, "Didn't find expected RT keyword\n");
  }

  // FIXME this may cause multiple reads of the same data depending on access order
  // RT checked out, loop over reading keywords until we find the one we need
  bool found = false;
  while (offset < (readStart + recordLength)) {
    rc = readKeyword(offset, keyword, keywordEntry);
    if (rc) {
      return rc;
    }
    
    // Cache what we read
    io_recordEntry.keywordCache[keyword] = keywordEntry;
    
    // If we got the keyword we were looking for, quit parsing and break out
    if (i_keyword == keyword) {
      found = true;
      o_keywordEntry = keywordEntry;
      break;
    }
  }

  // Error check about it not being found here
  if (!found) {
    // If we didn't find our keyword, we should be on the small resource tag.  Let's make sure that's where we are before return the not found error
    // If the code is working right, this small resource tag check isn't really necessary - but do it incase something goes haywire
    // Small resource tag, should be 0x78
    rc = read(offset, 1, readData);
    if (rc) {
      return rc;
    }
    
    // Error check
    if (readData.getByte(0) != 0x78) {
      return out.error(LHT_VPD_BAD_TAG, FUNCNAME, "Small resource tag value is bad: 0x%02X\n", readData.getByte(0));
    }
    
    // Small resource tag checked out, return the not found error
    return out.error(LHT_VPD_KEYWORD_NOT_FOUND, FUNCNAME, "Unable to find keyword %s in record %s!\n", i_keyword.c_str(), i_recordName.c_str());
  }

  return rc;
}

uint32_t lhtVpd::readKeyword(uint32_t & io_offset, std::string & o_keyword, keywordInfo & o_keywordEntry) {
  uint32_t rc = 0;
  ecmdDataBuffer data;

  // Call read of keyword length
  rc = read(io_offset, KEYWORD_BYTE_SIZE, data);
  if (rc) {
    return rc;
  }
  o_keyword = data.genAsciiStr();

  // Determine how big the length field for the keyword is
  // # keywords are longer than others
  uint32_t keywordLength;
  if (o_keyword.substr(0,1) == "#") {
    keywordLength = HASHKEYWORD_LENGTH_BYTE_SIZE;
  } else {
    keywordLength = KEYWORD_LENGTH_BYTE_SIZE;
  }
  
  // Call read of length
  rc = read(io_offset, keywordLength, data);
  if (rc) {
    return rc;
  }

  // Extract the data length out of the keyword and save it
  if (keywordLength > 1) {
    o_keywordEntry.length = data.getByte(0) | (data.getByte(1) << 8);
  } else {
    o_keywordEntry.length = data.getByte(0);
  }

  // Save away where the data starts
  o_keywordEntry.dataOffset = io_offset;

  // Call read of length of length
  rc = read(io_offset, o_keywordEntry.length, data);
  if (rc) return rc;

  o_keywordEntry.data = data;

 return rc;
}

uint32_t lhtVpd::updateRecordEcc(std::string & i_recordName) {
  uint32_t rc = 0;
  std::map<std::string, recordInfo>::iterator findRecordIter;
  ecmdDataBuffer data;
  ecmdDataBuffer ecc;

  // Initialize the record cache
  rc = recordCacheInit();
  if (rc) {
    return rc;
  }

  // Cache is populated, get the record we need
  findRecordIter = recordCache.find(i_recordName);
  if (findRecordIter == recordCache.end()) {
    return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Unable to find %s in recordCache\n", i_recordName.c_str());
  }
  const recordInfo & recordEntry = findRecordIter->second;

  // Read the entire record for ECC generation
  uint32_t recordOffset = recordEntry.recordOffset;
  rc = read(recordOffset, recordEntry.recordLength, data);
  if (rc) {
    return rc;
  }

  // Set the size of the ECC storage
  rc = ecc.setByteLength(recordEntry.eccLength);
  if (rc) {
    return rc;
  }

  // Create record ECC
  rc = createEcc(data, ecc);
  // If ECC is unimplemented do not write it out
  if (rc == LHT_VPD_ECC_UNIMPLEMENTED) {
    return 0;
  } else if (rc) {
    return rc;
  }

  // Write the ECC to the VPD
  uint32_t eccOffset = recordEntry.eccOffset;
  rc = write(eccOffset, recordEntry.eccLength, ecc);
  if (rc) {
    return rc;
  }

  return rc;
}

uint32_t lhtVpd::read(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & o_data) {
  return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Never call lhtVpd base class read!\n");
}

uint32_t lhtVpd::write(uint32_t & io_offset, uint32_t i_length, ecmdDataBuffer & i_data) {
  return out.error(LHT_VPD_GENERAL_ERROR, FUNCNAME, "Never call lhtVpd base class write!\n");
}

uint32_t lhtVpd::createEcc(ecmdDataBuffer & i_data, ecmdDataBuffer & io_ecc) {
  return LHT_VPD_ECC_UNIMPLEMENTED;
}
