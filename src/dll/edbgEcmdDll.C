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

//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------
#include <inttypes.h>
#include <stdio.h>
#include <algorithm>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <yaml.h>
#include <libgen.h>
#include <assert.h>
#include <map>
#include <errno.h>

// Headers from eCMD
#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBuffer.H>
#include <ecmdSharedUtils.H>
#include <ecmdChipTargetCompare.H>
#ifdef EDBG_ISTEP_CTRL_FUNCTIONS
#include <edbgIstep.H>
#endif

// Headers from pdbg/libipl/libekb
extern "C" {
#include <libpdbg.h>
}
#ifdef EDBG_ISTEP_CTRL_FUNCTIONS
#include <libipl.H>
#include <libekb.H>
#endif

// Headers from ecmd-pdbg
#include <edbgCommon.H>
#include <edbgOutput.H>
#include <lhtVpdFile.H>
#include <lhtVpdDevice.H>
#include <p9_edbgEcmdDllScom.H>
#include <p10_edbgEcmdDllScom.H>

//Header from spr generic function
#include <ecmdMapSpr2Str.H>

//Header from HWP
#include <p10_spr_name_map.H>

// TODO: This needs to not be hardcoded and set from the command-line.
std::string DEVICE_TREE_FILE;

// Store eeprom locations, indexed by target
std::map<ecmdChipTarget, std::string, ecmdChipTargetCompare> eeproms;
// Store if we have init'd the VPD info
bool vpdInit = false;

// For use by dllQueryConfig and dllQueryExist
uint32_t queryConfigExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistCages(const ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistNodes(const ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistSlots(const ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChips(const ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChipUnits(const ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::string class_type, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistThreads(const ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::list<ecmdThreadData> & o_threadData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);

std::string gEDBG_HOME;

/* ################################################################################################# */
/* Static functions used to lookup pdbg targets 						     */
/* ################################################################################################# */
static uint32_t fetchPdbgInterfaceTarget(const ecmdChipTarget & i_target, struct pdbg_target **o_target, const char *interface) {
  struct pdbg_target *target;

  assert(i_target.cageState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.cage == 0 &&                                  \
         i_target.nodeState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.node == 0 &&                                  \
         i_target.slotState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.slot == 0 &&                                  \
         i_target.posState == ECMD_TARGET_FIELD_VALID);

  *o_target = NULL;
  pdbg_for_each_class_target(interface, target) {
    if (pdbg_target_index(target) == i_target.pos) {
      *o_target = target;
      return 0;
    }
  }

  return -1;
}

/* Given a target in i_target this will return the associcated pdbg pib target
 * that can be passed to pib_read/write() such that they will not perform any
 * address translations - ie. "raw" scom access as cronus calls it. */
static uint32_t fetchPibTarget(const ecmdChipTarget & i_target, struct pdbg_target **o_pibTarget) {
  if (fetchPdbgInterfaceTarget(i_target, o_pibTarget, "pib")) {
    return out.error(-1, FUNCNAME, "Unable to find pib target in p%d\n", i_target.pos);
  }

  return 0;
}

static uint32_t fetchCfamTarget(const ecmdChipTarget & i_target, struct pdbg_target **o_pibTarget) {
  if (fetchPdbgInterfaceTarget(i_target, o_pibTarget, "fsi")) {
    return out.error(-1, FUNCNAME, "Unable to find cfam target in p%d\n", i_target.pos);
  }

  return 0;
}

/* Given a target and a target base address return the chip unit type (eg. "ex",
 * "mba", etc.) */
static uint32_t findChipUnitType(const ecmdChipTarget &i_target, uint64_t i_address, struct pdbg_target **pdbgTarget)
{
  struct pdbg_target *pibTarget, *target;

  if (fetchPibTarget(i_target, &pibTarget)) {
    return out.error(-1, FUNCNAME, "Unable to find PIB target\n");
  }

  /* Need to mask off the indirect address if present */
  i_address &= 0x7fffffff;

  pdbg_for_each_child_target(pibTarget, target) {
    uint64_t addr, size;

    addr = pdbg_target_address(target, &size);
    if (i_address >= addr && i_address < addr+size) {
      if (pdbg_get_target_property(target, "ecmd,chip-unit-type", NULL)) {
        // Found our base target
        *pdbgTarget = target;
        return 0;
      }
    }
  }

  return -1;
}

/**
  * @brief This is helper inline function to set library specific log level.
  *        Initialise with default value incase env value is not set or
  *        error case.
  *
  * @param  char* env - environment varibale name
  * @param  uint8_t - Default log level
  *
  * @return uint8_t - log level
  */
static inline uint8_t getLogLevelFromEnv(const char* env, const uint8_t i_logLevel)
{
    auto l_logLevel = i_logLevel;
    try
    {
         if (const char* env_p = std::getenv(env))
         {
             l_logLevel = std::stoi(env_p);
         }
    }
    catch (std::exception& e)
    {
         out.error(EDBG_GENERAL_ERROR, FUNCNAME,"Conversion Failure env=%s exception=%s",
                                                 env,e.what());
    }
    return l_logLevel;
}

#ifdef EDBG_ISTEP_CTRL_FUNCTIONS

/**
  * @brief This is helper function to set phal libraries log level based on
  *        environment varaibles values.
  *
  * @param None
  * @return None
  */
void setPhalLogLevel()
{
   ipl_set_loglevel(getLogLevelFromEnv("IPL_LOG", IPL_ERROR));

   uint32_t rc = libekb_init();
   if (rc)
   {
        out.error(rc, FUNCNAME, "libekb_init() failed\n");
   }
   else
   {
        libekb_set_loglevel(getLogLevelFromEnv("LIBEKB_LOG", LIBEKB_LOG_ERR));
   }
}

#endif

// Load the device tree and initialise the targets
static int initTargets(void) {

  // If set to 'none', skip the rest of what we do to setup the device tree
  // This is assuming we won't be using any functions that use the device tree
  if (!strcmp(getenv("PDBG_DTB"), "none")) {
      return ECMD_SUCCESS;
  }

  // set pdbg loglvel
  pdbg_set_loglevel(getLogLevelFromEnv("PDBG_LOG", PDBG_ERROR));

  /*  Device tree can also be specified using PDBG_DTB environment variable
   *  pointing to system device tree.  If system device tree is specified using
   *  PDBG_DTB, then it will override the default device tree or the specified
   *  device tree. NULL to use default which is used pdbg. */
  pdbg_targets_init(NULL);

  return ECMD_SUCCESS;
}

// Structure and enum to store the config as read from yaml
// A particular line could require one of three different variable types
// 1) A straight value to a key, the ultimate end of all types
// 2) A key/value map.  The value could be a more complicated type like a list
// 3) A list of entries
// Finally, a pointer to the parent is stored so we can get back up the tree
// when the parsing terminates at a value
typedef enum {
  CONFIG_VALUE,
  CONFIG_MAP,
  CONFIG_LIST,
} configType_t;

struct configEntry_t {
  configType_t type;
  std::string value;
  std::map<std::string, configEntry_t> map;
  std::list<configEntry_t> list;
  configEntry_t *parent;
};


// A function recursive that dumps the config as stored
// The printing is not pretty or formatted - used for debug during development
uint32_t printConfig(configEntry_t &configEntry) {
  uint32_t rc;

  // The only terminal type
  if (configEntry.type == CONFIG_VALUE) {
    out.print(" %s\n", configEntry.value.c_str());
    return 0;
  }

  // These other two will lead to further recursion
  if (configEntry.type == CONFIG_MAP) {
    std::map<std::string, configEntry_t>::iterator iter;
    for (iter = configEntry.map.begin(); iter != configEntry.map.end(); iter++) {
      out.print("%s:", iter->first.c_str());
      rc = printConfig(iter->second);
      if (rc) return rc;
    }
  }

  if (configEntry.type == CONFIG_LIST) {
    std::list<configEntry_t>::iterator iter;
    for (iter = configEntry.list.begin(); iter != configEntry.list.end(); iter++) {
      rc = printConfig(*iter);
      if (rc) return rc;
    }
  }

  return rc;
}

// Find the entry passed in and return all matches
uint32_t findConfigEntryValue(configEntry_t &configEntry, std::string &i_key, std::list<configEntry_t> &o_configList) {
  uint32_t rc = ECMD_SUCCESS;

  if (configEntry.type == CONFIG_MAP) {
    std::map<std::string, configEntry_t>::iterator iter;
    for (iter = configEntry.map.begin(); iter != configEntry.map.end(); iter++) {
      if (i_key == iter->first) {
        o_configList.push_back(iter->second);
      }
      rc = findConfigEntryValue(iter->second, i_key, o_configList);
      if (rc) return rc;
    }
  } else if (configEntry.type == CONFIG_LIST) {
    std::list<configEntry_t>::iterator iter;
    for (iter = configEntry.list.begin(); iter != configEntry.list.end(); iter++) {
      rc = findConfigEntryValue(*iter, i_key, o_configList);
      if (rc) return rc;
    }
  } else {
    // Nothing to do for any other type than return out
    return 0;
  }

  return rc;
}

// We need a search function that at a given level will go thru the children and find any that match
// It should return that as a list
// That function can then be used to find multiple planar, chip, chipunit declarations
uint32_t readCnfg() {
  uint32_t rc = ECMD_SUCCESS;

  std::string cnfgFile;
  char * var = getenv("EDBG_CNFG");
  if (var != NULL) {
    cnfgFile = var;
  } else {
    // Check if /var/lib/misc/edbg.yaml exists.  If it does, set cnfgFile to it
    if (access("/var/lib/misc/edbg.yaml", F_OK) != -1) {
      cnfgFile = "/var/lib/misc/edbg.yaml";
    }
  }

  // If we don't have a config file, bail here and don't fail trying to open it below
  // We'll assume the user isn't doing vpd stuff and doesn't need the config
  if (cnfgFile.empty()) {
    return rc;
  }

  // Most of the understanding for how to parse yaml used here came from
  // https://www.wpsoftware.net/andrew/pages/libyaml.html

  // Variables for parsing
  yaml_parser_t parser;
  yaml_event_t event;

  // The config file is there, lets open and parse it
  FILE *fh = fopen(cnfgFile.c_str(), "r");
  if (fh == NULL)
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Failed to open config file: %s\n", cnfgFile.c_str());

  // Init the parser
  if (!yaml_parser_initialize(&parser))
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Failed to initialize yaml parser!\n");

  // Set the input file
  yaml_parser_set_input_file(&parser, fh);

  // State variables for config file processing
  configEntry_t config; // Stores the entire config intermediate form
  configEntry_t configEntry; // Reused to push on new config nodes
  configEntry_t *cur = NULL; // Where we are in the whole thing
  // The parser only returns scalar, not if it parsed a key or value
  // We have to track that for ourselves as we parse the file
  uint8_t keyOrValue = 0;
  std::string key;

  // Loop processing until content exhausted
  do {
    if (!yaml_parser_parse(&parser, &event)) {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Parser error %d\n", parser.error);
    }

    switch(event.type) {
      // No Event, throw warning for now in case we need to handle it
      case YAML_NO_EVENT:
        out.warning(FUNCNAME, "A yaml no event hit during parsing!");
        break;

      // Stream start/end are the first/last things in the parse
      case YAML_STREAM_START_EVENT:
        // Set our working pointer to the start of our currently emtpy config
        cur = &config;
        break;

      case YAML_STREAM_END_EVENT:
        // Nothing to do for cleanup at the end... yet
        break;

      // Nothing to do for document start/end
      case YAML_DOCUMENT_START_EVENT:
        break;
      case YAML_DOCUMENT_END_EVENT:
        break;

      // Handle sequence start/end events
      case YAML_SEQUENCE_START_EVENT:
        // Sequence starts tell us we have a key out there we can use to create the next level
        // That means where we are is going to be a MAP
        cur->type = CONFIG_MAP;
        // Setup the parent before we insert into the map
        configEntry.parent = cur;
        // Store it in the map
        cur->map.insert(std::make_pair(key, configEntry));
        // Move our cur to what we just created
        cur = &(cur->map[key]);
        // Reset our tic/toc since we created a value
        keyOrValue = 0;
        break;

      case YAML_SEQUENCE_END_EVENT:
        // When we are done, move ourselves back up the tree
        cur = cur->parent;
        break;

      // Handle map start/end events
      case YAML_MAPPING_START_EVENT:
        // It would be nice if the enum from yaml (map) matched how we have to store this (list)
        // But it doesn't
        // Setup that we are a list
        cur->type = CONFIG_LIST;
        // Setup the parent before we push onto the list
        configEntry.parent = cur;
        // Store it in the list
        cur->list.push_back(configEntry);
        // Move our cur to what we just created
        cur = &(cur->list.back());
        break;

      case YAML_MAPPING_END_EVENT:
        // When we are done, move ourselves back up the tree
        cur = cur->parent;
        break;

      // Data
      case YAML_ALIAS_EVENT:
        // This does nothing for us
        break;
      case YAML_SCALAR_EVENT:
        // For a scalar, figure out if we are on the key or value part and act appropriately
        if (!keyOrValue) {
          // Key is easy - store it for later use and flip to the value side
          key = (char*)event.data.scalar.value;
          keyOrValue += 1;
        } else {
          // We have a value, setup that this entry will be for a map
          cur->type = CONFIG_MAP;
          // Setup the parent before we insert into the map
          configEntry.parent = cur;
          // It's a value, setup those states
          configEntry.type = CONFIG_VALUE;
          configEntry.value = (char *)event.data.scalar.value;
          // Store it in the map
          cur->map.insert(std::make_pair(key, configEntry));
          // Reset our tic/toc since we created a value
          keyOrValue = 0;
        }
        break;
    }

    if (event.type != YAML_STREAM_END_EVENT)
      yaml_event_delete(&event);
  } while(event.type != YAML_STREAM_END_EVENT);
  yaml_event_delete(&event);

  // Cleanup
  yaml_parser_delete(&parser);
  fclose(fh);

  // Comment out for now, perhaps enable under a debug later
  //printConfig(config);

  // ***************
  // Done parsing yaml
  // Pull the data we need out of intermediate form
  // ***************

  // First, check our version tag
  std::list<configEntry_t> configList;
  key = "version";
  configList.clear();
  rc = findConfigEntryValue(config, key, configList);
  if (rc) return rc;

  // Make sure the entry was there
  if (configList.size() == 0) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "The version entry was not found in the config!\n");
  }

  // We did have it, so verify it's the right version
  configEntry = configList.front();
  if (configEntry.value != "1") {
    out.print("configEntry.value is: %s\n", configEntry.value.c_str());
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Only \'version: 1\' supported at this time!\n");
  }

  // Now grab our VPD block and load up our eeproms list
  configList.clear();
  key = "vpd";
  rc = findConfigEntryValue(config, key, configList);
  if (rc) return rc;

  // Make sure the entry was there
  if (configList.size() == 0) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "The vpd section was not found in the config!\n");
  }

  // We did have it, now grab the data we need out of it
  configEntry = configList.front();
  // This configEntry will be list of vpd entries in the config.  They look like this:
  // - target: k0:n0:s0
  //   system-vpd: /some/path/to/an/eeprom
  // - target: k0:n0:s1
  //   system-vpd: /some/path/to/a/second/eeprom
  // We have to loop through the list and verify we have the contents we need in each map
  std::list<configEntry_t>::iterator iter; // To walk the list
  std::map<std::string, configEntry_t>::iterator findIter; // To find the contents in the map
  for (iter = configEntry.list.begin(); iter != configEntry.list.end(); iter++) {
    // In each list entry there will be map of all the required values
    // Ensure everything we need is there and then populate what the code uses
    // These are "target" and "system-vpd"
    configEntry_t findTarget, findSysVpd;

    // target:
    findIter = iter->map.find("target");
    if (findIter == iter->map.end()) {
      out.error(FUNCNAME, "The \'target:\' entry wasn't found in \'vpd:\'\n");
      continue;
    } else {
      // Found it, save it for the end
      findTarget = findIter->second;
    }

    // system-vpd:
    findIter = iter->map.find("system-vpd");
    if (findIter == iter->map.end()) {
      out.error(FUNCNAME, "The \'system-vpd:\' entry wasn't found in \'vpd:\'\n");
      continue;
    } else {
      // Found it, save it for the end
      findSysVpd = findIter->second;
    }

    // We made it here, things must be valid
    // Turn the string into a real chipTarget and then load in the eeproms list
    ecmdChipTarget chipTarget;
    rc = ecmdReadTarget(findTarget.value, chipTarget);
    if (rc) return rc;
    eeproms[chipTarget] = findSysVpd.value;
  }

  // Set that we have valid VPD info if one of those functions should be called
  vpdInit = true;

  return rc;
}

uint32_t dllInitDll() {
  uint32_t rc = ECMD_SUCCESS;

  // Setup a couple global environment variables
  // EDBG_HOME
  // Allow the user to specify it, otherwise dynamically determine it via ECMD_EXE
  char *tempptr = getenv("EDBG_HOME");
  if (tempptr != NULL) {
    gEDBG_HOME.insert(0, tempptr);
  } else {
    // The home is one up from the exe install directory
    // Then resolve it into the real path before setting it
    char relativePath[PATH_MAX];
    char realPath[PATH_MAX];
    sprintf(relativePath, "%s/../", dirname(getenv("ECMD_EXE")));
    realpath(relativePath, realPath);
    gEDBG_HOME = realPath;
  }

  rc = initTargets();
  if (rc) return rc;

  rc = readCnfg();
  if (rc) return rc;

  return rc;
}

uint32_t dllFreeDll() {
  uint32_t rc = ECMD_SUCCESS;

  return rc;
}

uint32_t dllSpecificCommandArgs(int* io_argc, char** io_argv[]) {
  uint32_t rc = ECMD_SUCCESS;

  /* Grab the device mode flag */
  char* curArg;
  if ((curArg = ecmdParseOptionWithArgs(io_argc, io_argv, "--device"))) {
    DEVICE_TREE_FILE = curArg;
  } else {
    DEVICE_TREE_FILE = "ecmd.dtb";
  }

  return rc;
}

void dllLoadDllRecovery(std::string i_commandLine, uint32_t & io_rc) {
  return;
}

uint32_t dllSyncPluginState(const ecmdChipTarget & i_target) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

std::string dllSpecificParseReturnCode(uint32_t i_returnCode) {
  std::string ret = "";

  std::string filePath = gEDBG_HOME + "/help/edbgReturnCodes.H";
  std::string line;
  std::vector<std::string> tokens;
  uint32_t comprc;

  /* This is what I am trying to parse from edbgReturnCodes.H */

  /* #define ECMD_ERR_UNKNOWN                        0x00000000 ///< This error code wasn't flagged to which plugin it came from */
  /* #define ECMD_ERR_ECMD                           0x01000000 ///< Error came from eCMD                                        */
  /* #define ECMD_ERR_CRONUS                         0x02000000 ///< Error came from Cronus                                      */
  /* #define ECMD_ERR_IP                             0x04000000 ///< Error came from IP GFW                                      */
  /* #define ECMD_ERR_Z                              0x08000000 ///< Error came from Z GFW                                       */
  /* #define ECMD_INVALID_DLL_VERSION                (ECMD_ERR_ECMD | 0x1000) ///< Dll Version                                   */

  std::ifstream ins(filePath.c_str());

  if (ins.fail()) {
    ret = "ERROR OPENING " + filePath;
    return ret;
  }

  while (!ins.eof()) { /*  && (strlen(str) != 0) */
    getline(ins,line,'\n');
    /* Let's strip off any comments */
    line = line.substr(0, line.find_first_of("/"));
    ecmdParseTokens(line, " \n()|", tokens);

    /* Didn't find anything */
    if (line.size() < 2) continue;

    if (tokens[0] == "#define") {
      if (tokens.size() >= 4) {
        /* This is a standard return code define */
        sscanf(tokens[3].c_str(),"0x%x",&comprc);
        if ((i_returnCode & 0x00FFFFFF) == comprc) {
          /* This matches, we're out */
          ret = tokens[1];
          break;
        }
      }
    }
  }

  ins.close();

  return ret;
}

/* ################################################################################################# */
/* System Query Functions - System Query Functions - System Query Functions - System Query Functions */
/* ################################################################################################# */
/**
  * @brief Get chip unit position
  *        
  * @param  pdbg_target *target - pdbg target pointer
  *
  * @return Upon success, chip unit number will be returned.  A reason code will
  *         be returned if the execution fails.
  */
uint8_t getChipUnitPos(pdbg_target *target)
{
    uint8_t chipUnitPos; //chip unit position
    
    //size: uint8 => 1, uint16 => 2. uint32 => 4 uint64=> 8
    //typedef uint8_t ATTR_CHIP_UNIT_POS_Type;
    if(!pdbg_target_get_attribute(target, "ATTR_CHIP_UNIT_POS", 1, 1, &chipUnitPos)){ 
       return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                 "ATTR_CHIP_UNIT_POS Attribute get failed");
    }

    return chipUnitPos;
}

/**
  * @brief Check if the target is functional or not
  *        
  * @param  pdbg_target *target - pdbg target pointer
  *
  * @return Upon success return true else false.
  * 
  */
bool isFunctionalTarget(struct pdbg_target *target)
{
    uint8_t buf[5];
    bool isFunc = false;

    if (!pdbg_target_get_attribute_packed(target, "ATTR_HWAS_STATE", "41", 1, buf)) {
        out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                         "ATTR_HWAS_STATE Attribute get failed");
        isFunc = false;
    }

    //isFuntional bit is stored in 4th byte and bit 3 position in HWAS_STATE
    if (buf[4] & 0x20) {
        isFunc = true;
    }

    return isFunc;    
}

uint32_t dllQueryConfig(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail ) {
  return queryConfigExist(i_target, o_queryData, i_detail, false);
}

uint32_t dllQueryExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail ) {
  return queryConfigExist(i_target, o_queryData, i_detail, true);
}

uint32_t queryConfigExist(const ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
  uint32_t rc = ECMD_SUCCESS;

  // Need to clear out the queryConfig data before pushing stuff in
  // This is in case there is stale data in there
  o_queryData.cageData.clear();

  // From here, we will recursively work our way through all levels of hierarchy in the target
  if (i_target.cageState == ECMD_TARGET_FIELD_VALID || i_target.cageState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistCages(i_target, o_queryData.cageData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  return rc;
}

uint32_t queryConfigExistCages(const ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdCageData cageData;

  // We only have 1 cage for edbg, create that data
  // Then walk down through our nodes
  cageData.cageId = 0;

  // If the node states are set, see what nodes are in this cage
  if (i_target.nodeState == ECMD_TARGET_FIELD_VALID || i_target.nodeState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistNodes(i_target, cageData.nodeData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  // Save what we got from recursing down, or just being happy at this level
  o_cageData.push_back(cageData);

  return rc;
}

uint32_t queryConfigExistNodes(const ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdNodeData nodeData;

  // We only have 1 node for edbg, create that data
  // Then walk down through our slots
  nodeData.nodeId = 0;

  // If the slot states are set, see what slots are in this node
  if (i_target.slotState == ECMD_TARGET_FIELD_VALID || i_target.slotState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistSlots(i_target, nodeData.slotData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  // Save what we got from recursing down, or just being happy at this level
  o_nodeData.push_back(nodeData);

  return rc;
}

uint32_t queryConfigExistSlots(const ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdSlotData slotData;

  // We only have 1 slot for edbg, create that data
  // Then walk down through our chips
  slotData.slotId = 0;

  // If the chipType states are set, see what chipTypes are in this slot
  if (i_target.chipTypeState == ECMD_TARGET_FIELD_VALID || i_target.chipTypeState == ECMD_TARGET_FIELD_WILDCARD) {
    rc = queryConfigExistChips(i_target, slotData.chipData, i_detail, i_allowDisabled);
    if (rc) return rc;
  }

  // Save what we got from recursing down, or just being happy at this level
  o_slotData.push_back(slotData);

  return rc;
}

uint32_t queryConfigExistChips(const ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipData chipData;
  ecmdChipUnitData chipUnitData;
  struct pdbg_target *chipTarget, *pibTarget, *ocmbTarget;
  char pib_path[32];

  // The display order is proc chip followed by memory chip
  // Within the proc/memory chip, sort them by position.
  // To keep in sync with lab and cronus users, order and display
  // the memory chips by FAPI_POS.
  pdbg_for_each_class_target("proc", chipTarget) {

    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((pdbg_target_index(chipTarget) < 0) || ((i_target.posState == ECMD_TARGET_FIELD_VALID) && 
        (pdbg_target_index(chipTarget) != i_target.pos)))
      continue;
      
    //if chip type is not pu then, skip adding.   
    if((i_target.chipType != ECMD_CHIPT_PROCESSOR) && 
       (i_target.chipTypeState != ECMD_TARGET_FIELD_WILDCARD))
      continue;

    // If the target is not functional then do not add to the list
    // FIXME : Enable this once we have way to determine functional state 
    // when the system is in the powered off state
    //if(!isFunctionalTarget(chipTarget)) 
    //    continue;
    
    sprintf(pib_path, "/%s%d/%s", "proc", pdbg_target_index(chipTarget), "pib");

    pibTarget = pdbg_target_from_path(NULL,pib_path);
    if (!pibTarget)
       continue;

    // Probe target to see if it exists (ie. disabled or not)
    pdbg_target_probe(pibTarget);

    // If i_allowDisabled isn't true, make sure it's not disabled
    if (!i_allowDisabled) {
      if (pdbg_target_status(pibTarget) != PDBG_TARGET_ENABLED)
	continue;
    }

    // We passed our checks, load up our data
    chipData.chipUnitData.clear();
    chipData.chipType = "pu";
    chipData.chipShortType = i_target.chipUnitType;
    chipData.pos = pdbg_target_index(chipTarget);

    // If the chipUnitType states are set, see what chipUnitTypes are in this chipType
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID
        || i_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD) {
      // Look for chipunits
      rc = queryConfigExistChipUnits(i_target, chipTarget, chipData.chipType, chipData.chipUnitData, i_detail, i_allowDisabled);
      if (rc) return rc;
    }
    // Save what we got from recursing down, or just being happy at this level
    o_chipData.push_back(chipData);
  }
  
  //Show up explorer only if the explorer scoms are requested
  if ((i_target.chipType == "explorer") ||
      (i_target.chipTypeState == ECMD_TARGET_FIELD_WILDCARD))
  {
      //Next, Fill in explorer targets
      pdbg_for_each_class_target("ocmb", ocmbTarget) {

        // If posState is set to VALID, check that our values match
        // If posState is set to WILDCARD, we don't care
        if ((pdbg_target_index(ocmbTarget) < 0) || ((i_target.posState == ECMD_TARGET_FIELD_VALID) && 
          (getFapiUnitPos(ocmbTarget) != i_target.pos)))
          continue;

        //Add to the data structure only if functional
        if(!isFunctionalTarget(ocmbTarget)) 
          continue;

        pdbg_target_probe(ocmbTarget);

        // If i_allowDisabled isn't true, make sure it's not disabled
        if (!i_allowDisabled) {
          if (pdbg_target_status(ocmbTarget) != PDBG_TARGET_ENABLED)
      	    continue;
        }

        // We passed our checks, load up our data
        chipData.chipUnitData.clear();
        chipData.chipType = "explorer";
        chipData.chipShortType = "exp";

        // Getting the seq id of the chip
        // We use FAPI unit position instead of Chip unit position here.
        // DDIMM populated position comes from TARGETING::ATTR_FAPI_POS
        chipData.pos = getFapiUnitPos(ocmbTarget);

        // If the chipUnitType states are set, see what chipUnitTypes are in this chipType
        if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID
          || i_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD) {
          // Look for chipunits
          rc = queryConfigExistChipUnits(i_target, ocmbTarget, chipData.chipType, chipData.chipUnitData, i_detail, i_allowDisabled);
          if (rc) return rc;
        }
        // Save what we got from recursing down, or just being happy at this level
        o_chipData.push_back(chipData);
      }
  }
  return rc;
}

uint32_t addChipUnits(const ecmdChipTarget & i_target, struct pdbg_target *i_pTarget, std::string class_name, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)
{
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *target;
  ecmdChipUnitData chipUnitData;
  std::string cuString;
  ecmdChipTarget o_target;

  p10x_convertPDBGClassString_to_CUString(class_name, cuString);
  if (rc) return rc;
 
  if (class_name == "explorer")  {
    chipUnitData.chipUnitType = "mp";
    chipUnitData.chipUnitShortType = "mp";
    chipUnitData.chipUnitNum = 0;
    o_chipUnitData.push_back(chipUnitData);
  } else {
   
    pdbg_for_each_target(class_name.c_str(), i_pTarget, target) {
 
      //If posState is set to VALID, check that our values match
      //If posState is set to WILDCARD, we don't care
      if ((i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID) &&
        (pdbg_target_index(target) != i_target.chipUnitNum))
        continue;

      if ((i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
        (cuString != i_target.chipUnitType))
        continue;
      
      // If i_allowDisabled isn't true, make sure it's not disabled
      // check HWAS state to populate the table with only 
      // functional resources
      // Generally, if we don't add a check for the functional state then, 
      // all the targets in the device tree will be marked as available even though
      // they are functionally not enabled based on the HW config.
      // Checking for the Functional state of a target based on the device tree 
      // attribute HWAS_STATE as this attribute value will get populated from MRW 
      // which can be treated as the correct state for the given target. 
      if (!i_allowDisabled  && !isFunctionalTarget(target))
        continue;

      //probe only the functional targets 
      pdbg_target_probe(target);

      uint32_t chipUnitNum = getChipUnitPos(target);
    
      if (pdbg_target_index(target) >= 0) {
        chipUnitData.threadData.clear();
        chipUnitData.chipUnitType = cuString;
        chipUnitData.chipUnitNum = chipUnitNum;
        chipUnitData.numThreads = 4;
      }
    
      // If the thread states are set, see what thread are in this chipUnit
      if (i_target.threadState == ECMD_TARGET_FIELD_VALID
        || i_target.threadState == ECMD_TARGET_FIELD_WILDCARD) {
        // Look for chipunits
        rc = queryConfigExistThreads(i_target, target, chipUnitData.threadData, i_detail, i_allowDisabled);
        if (rc) return rc;
      }
      o_chipUnitData.push_back(chipUnitData);
    }
  }
  return rc;
}

uint32_t queryConfigExistChipUnits(const ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::string class_type, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {

  uint32_t rc = ECMD_SUCCESS;
  ecmdChipUnitData chipUnitData;
  struct pdbg_target *target;
  uint32_t l_index;

  if(pdbg_get_proc() == PDBG_PROC_P10)
  {
      for (l_index = 0;
           l_index < (sizeof(ChipUnitTable) / sizeof(p10_chipUnit_t));
           l_index++)
      {
          // If pdbg class type is pib , don't add the chip unit to the
          // queryConfigExistChipUnits
          if (ChipUnitTable[l_index].pdbgClassType != "pib") {
              rc = addChipUnits(i_target, i_pTarget, ChipUnitTable[l_index].pdbgClassType, 
                                o_chipUnitData, i_detail, i_allowDisabled);
              if (rc) {
                  out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Failed to add chip unit:%s\n",
                            ChipUnitTable[l_index].pdbgClassType.c_str());
              }
          }
      }
      //Add explorer targets if target class type selected is explorer
      if (class_type == "explorer")
      {
          rc = addChipUnits(i_target, i_pTarget, class_type, 
                            o_chipUnitData, i_detail, i_allowDisabled);
          if (rc) {
              return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Failed to add ocmb chip unit\n");
          }
      }
  }
  // FIXME: This logic needs to optimized. Will get the same p10 logic work on 
  // p9 as well. but, for now to not break the things keeping like this. 
  else if (pdbg_get_proc() == PDBG_PROC_P9) {
  
    pdbg_for_each_child_target(i_pTarget, target) {
    char *p;

    p = (char *) pdbg_get_target_property(target, "ecmd,chip-unit-type", NULL);
    if (!p || pdbg_target_index(target) < 0)
      /* Skip targets with no ecmd equivalent */
      continue;

    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID) &&
        (pdbg_target_index(target) != i_target.chipUnitNum))
      continue;

    if ((i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
        (p != i_target.chipUnitType))
      continue;

    pdbg_target_probe(target);

    // If i_allowDisabled isn't true, make sure it's not disabled
    if (!i_allowDisabled)
      if (pdbg_target_status(target) != PDBG_TARGET_ENABLED)
	continue;

    chipUnitData.chipUnitType = p;
    chipUnitData.chipUnitNum = pdbg_target_index(target);

    // If the thread states are set, see what thread are in this chipUnit
    if (i_target.threadState == ECMD_TARGET_FIELD_VALID
        || i_target.threadState == ECMD_TARGET_FIELD_WILDCARD) {
      // Look for chipunits
      rc = queryConfigExistThreads(i_target, target, chipUnitData.threadData, i_detail, i_allowDisabled);
      if (rc) return rc;
    }

    o_chipUnitData.push_back(chipUnitData);
    }
  }
  return rc;
}

uint32_t queryConfigExistThreads(const ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::list<ecmdThreadData> & o_threadData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
  
  uint32_t rc = ECMD_SUCCESS;
  ecmdThreadData threadData;
  struct pdbg_target *target;

  pdbg_for_each_target("thread", i_pTarget, target){

    if  ((i_target.threadState == ECMD_TARGET_FIELD_VALID) &&
         (pdbg_target_index(target) != i_target.thread)){
      continue;
    }
    
    pdbg_target_probe(target);

    //Use the index as the threadId and then store it
    threadData.threadId = pdbg_target_index(target);

    o_threadData.push_back(threadData);
  }
  return rc;
}


uint32_t dllGetConfiguration(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigValid_t & o_validOutput, std::string & o_valueAlpha, uint32_t & o_valueNumeric) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetConfigurationComplex(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigData & o_configData) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSetConfiguration(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigValid_t i_validInput, std::string i_valueAlpha, uint32_t i_valueNumeric) {
	return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSetConfigurationComplex(const ecmdChipTarget & i_target, std::string i_name, ecmdConfigData i_configData) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllDeconfigureTarget(const ecmdChipTarget & i_target) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllConfigureTarget(const ecmdChipTarget & i_target) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryConnectedTargets(const ecmdChipTarget & i_target, const char * i_connectionType, std::list<ecmdConnectionData> & o_connections) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllRelatedTargets(const ecmdChipTarget & i_target, const std::string i_relatedType, std::list<ecmdChipTarget> & o_relatedTargets, const ecmdLoopMode_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

/* ######################################################################################### */
/* Info Query Functions - Info Query Functions - Info Query Functions - Info Query Functions */
/* ######################################################################################### */
uint32_t dllQueryFileLocation(const ecmdChipTarget & i_target, ecmdFileType_t i_fileType, std::list<ecmdFileLocation> & o_fileLocations, std::string & io_version) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdFileLocation location;

  switch (i_fileType) {
    case ECMD_FILE_HELPTEXT:
      location.textFile = gEDBG_HOME + "/help/";
      location.hashFile = "";
      o_fileLocations.push_back(location);
      break;

    default:
      rc = ECMD_INVALID_ARGS;
      break;
  }

  return rc;
}

/* ######################################################################### */
/* Output Functions - Output Functions - Output Functions - Output Functions */
/* ######################################################################### */
void dllOutputError(const char* message) {
  out.error("ECMD", message);
}

void dllOutputWarning(const char* message) {
  out.warning("ECMD", message);
}

void dllOutput(const char* message) {
  out.print(message);
}

/* ######################################################################################### */
/* Ring Cache Functions - Ring Cache Functions - Ring Cache Functions - Ring Cache Functions */
/* ######################################################################################### */
uint32_t dllEnableRingCache(const ecmdChipTarget & i_target) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

uint32_t dllDisableRingCache(const ecmdChipTarget & i_target) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

uint32_t dllFlushRingCache(const ecmdChipTarget & i_target) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

bool dllIsRingCacheEnabled(const ecmdChipTarget & i_target) { return false; }

/* ################################################################# */
/* Misc Functions - Misc Functions - Misc Functions - Misc Functions */
/* ################################################################# */
void dllSetTraceMode(ecmdTraceType_t i_type, bool i_enable) {
  return out.error(FUNCNAME, "Function not supported!\n");
}

bool dllQueryTraceMode(ecmdTraceType_t i_type) {
  return false;
}

uint32_t dllDelay(uint32_t i_simCycles, uint32_t i_msDelay) {
  uint32_t rc = ECMD_SUCCESS;

  rc = usleep(i_msDelay*1000);

  return rc;
}

/**
* @brief Get chip type (p9 or p10)
*         
* @param std::string o_chipType - Chip type as output
*
* @return Upon success, return chip name (p9, p10, etc.)  Unknown will
*         be returned if it fails to determine.
*/
std::string getChipType() {
    
    std::string l_chipType;

    // determine the chip type
    switch (pdbg_get_proc()) {
        case PDBG_PROC_P9:
            l_chipType = "p9";
            break;

        case PDBG_PROC_P10:
            l_chipType = "p10";
            break;

        default:
            l_chipType = "Unknown";
            break;
    }
    return l_chipType;
}

uint32_t dllGetChipData(const ecmdChipTarget & i_target, ecmdChipData & o_data) {
  uint32_t rc = ECMD_SUCCESS;

  if (pdbg_get_proc() == PDBG_PROC_P10) 
  {
      ecmdChipData chipData;
      struct pdbg_target *chipTarget;
      uint32_t index;
     
      //chipEC is 0 if we fail to read via attribute
      uint8_t chipEC = 0;

      pdbg_for_each_class_target("proc", chipTarget) {

        index = pdbg_target_index(chipTarget);

        // If posState is set to VALID, check that our values match
        // If posState is set to WILDCARD, we don't care
        if ((index < 0) || ((i_target.posState == ECMD_TARGET_FIELD_VALID) && (index != i_target.pos)))
          continue;

        // We passed our checks, load up our data
        chipData.chipUnitData.clear();
        chipData.chipType = getChipType();
        chipData.chipShortType = getChipType();
        chipData.chipCommonType = ECMD_CHIPT_PROCESSOR;
        chipData.pos = index;

        //TARGETING::ATTR_EC
        if(!pdbg_target_get_attribute(chipTarget, "ATTR_EC", 1, 1, &chipEC)){
          return out.error(EDBG_GENERAL_ERROR, FUNCNAME,
                    "ATTR_EC Attribute get failed");
        }

        chipData.chipEc = chipEC;

        //Will use chip EC we got from device tree
        chipData.simModelEc = chipEC;
    
        //For FSI, the interface type is CFAM.
        chipData.interfaceType = ECMD_INTERFACE_CFAM;

        //FSI is hardcoded.
        chipData.chipFlags = ECMD_CHIPFLAG_FSI; 
    
        //copy data 
        o_data = chipData;
    }
  } 
  else 
  {
      return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
  }
  return rc;
}

uint32_t dllChipCleanup(const ecmdChipTarget & i_target, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryMode(const ecmdChipTarget & i_target, std::string & o_coreMode, std::string & o_coreChipUnit) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetProcessingUnit(const ecmdChipTarget & i_target, std::string & o_processingUnitName) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

std::string dllLastError() {
  return "NOT_SUPPORTED";
}

uint32_t dllCreateChipUnitScomAddress(const ecmdChipTarget & i_target, uint64_t i_address, uint64_t & o_address) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

#ifndef ECMD_REMOVE_SCOM_FUNCTIONS
uint32_t dllQueryScom(const ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail) {
  uint32_t rc = ECMD_SUCCESS;
  
  if (pdbg_get_proc() == PDBG_PROC_P9) {
      rc = p9_dllQueryScom(i_target, o_queryData, i_address, i_detail);
  } else if (pdbg_get_proc() == PDBG_PROC_P10) {
      rc = p10_dllQueryScom(i_target, o_queryData, i_address, i_detail);
  } else {
     return ECMD_FUNCTION_NOT_SUPPORTED;
  }

  if (rc) {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "queryscom failed!!");
  }

  return rc;
}

uint32_t dllGetScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  
  if (pdbg_get_proc() == PDBG_PROC_P9) {
      rc = p9_dllGetScom(i_target,i_address,o_data);
  } else if (pdbg_get_proc() == PDBG_PROC_P10) {
      rc = p10_dllGetScom(i_target,i_address,o_data);
  } else {
     return ECMD_FUNCTION_NOT_SUPPORTED;
  }

  if (rc) {
      return out.error(EDBG_READ_ERROR, FUNCNAME, "getscom failed!!");
  }

  return rc;
}

uint32_t dllPutScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  
  if (pdbg_get_proc() == PDBG_PROC_P9) {
      rc = p9_dllPutScom(i_target,i_address,i_data);
  } else if (pdbg_get_proc() == PDBG_PROC_P10) {
      rc = p10_dllPutScom(i_target,i_address,i_data);
  } else {
     return ECMD_FUNCTION_NOT_SUPPORTED;
  }

  if (rc) {
      return out.error(EDBG_WRITE_ERROR, FUNCNAME, "putscom failed!!");
  }
  return rc;
}

uint32_t dllPutScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllDoScomMultiple(const ecmdChipTarget & i_target, std::list<ecmdScomEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_SCOM_FUNCTIONS

#ifndef ECMD_REMOVE_FSI_FUNCTIONS
/* ################################################################# */
/* cfam Functions - cfam Functions - cfam Functions - cfam Functions */
/* ################################################################# */
uint32_t dllGetCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint32_t data;
  struct pdbg_target * pdbgTarget;

  rc = fetchCfamTarget(i_target, &pdbgTarget);
  if (rc) return rc;

  // Make sure the pdbg target probe has been done and get the target state
  if (pdbg_target_probe(pdbgTarget) != PDBG_TARGET_ENABLED) {
    return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "Target not configured!\n");
  }

  rc = fsi_read(pdbgTarget, i_address, &data);
  o_data.setBitLength(32);
  o_data.setWord(0, data);

  return rc;
}

uint32_t dllPutCfamRegister(const ecmdChipTarget & i_target, uint32_t i_address, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target * pdbgTarget;

  rc = fetchCfamTarget(i_target, &pdbgTarget);
  if (rc) return rc;

  // Make sure the pdbg target probe has been done and get the target state
  if (pdbg_target_probe(pdbgTarget) != PDBG_TARGET_ENABLED) {
    return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "Target not configured!\n");
  }

  rc = fsi_write(pdbgTarget, i_address, i_data.getWord(0));

  return rc;
}

uint32_t dllGetEcid(const ecmdChipTarget & i_target, ecmdDataBuffer & o_ecidData) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetEcidVerbose(const ecmdChipTarget & i_target, ecmdDataBuffer & o_ecidData, std::vector<std::string> & o_additionalInfo) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetGpRegister(const ecmdChipTarget & i_target, uint32_t i_gpRegister, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutGpRegister(const ecmdChipTarget & i_target, uint32_t i_gpRegister, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutGpRegisterUnderMask(const ecmdChipTarget & i_target, uint32_t i_gpRegister, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_FSI_FUNCTIONS

#ifndef ECMD_REMOVE_VPD_FUNCTIONS
/* ############################################################# */
/* VPD Functions - VPD Functions - VPD Functions - VPD Functions */
/* ############################################################# */
uint32_t dllGetModuleVpdKeyword(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutModuleVpdKeyword(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetModuleVpdImage(const ecmdChipTarget & i_target, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutModuleVpdImage(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetModuleVpdKeywordFromImage(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, const ecmdDataBuffer & i_image_data, ecmdDataBuffer & o_keyword_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutModuleVpdKeywordToImage(const ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, ecmdDataBuffer & io_image_data, const ecmdDataBuffer & i_keyword_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetFruVpdImage(const ecmdChipTarget & i_target, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutFruVpdImage(const ecmdChipTarget & i_target, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetFruVpdKeywordWithRid(const uint32_t i_rid, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutFruVpdKeywordWithRid(const uint32_t i_rid, const char * i_record_name, const char * i_keyword, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetFruVpdKeyword(const ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  lhtVpdDevice vpd;

  if (!vpdInit) {
    return out.error(EDBG_CNFG_MISSING, FUNCNAME, "VPD functions not initialized!  Set EDBG_CNFG or run edbgdetcnfg.\n");
  }

  // Get the path to the eepromFile and error if not there
  std::string eepromFile = eeproms[i_target];

  // Open the device
  rc = vpd.openDevice(eepromFile);
  if (rc) return rc;

  // Get the keyword data
  rc = vpd.getKeyword(i_recordName, i_keyword, o_data);
  if (rc) return rc;

  // If the call returned more than was asked for, shrink it
  if (o_data.getByteLength() > i_bytes) {
    o_data.shrinkBitLength(i_bytes * 8);
  }

  // All done, close the device
  rc = vpd.closeDevice();
  if (rc) return rc;

  return rc;
}

uint32_t dllPutFruVpdKeyword(const ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  lhtVpdDevice vpd;

  if (!vpdInit) {
    return out.error(EDBG_CNFG_MISSING, FUNCNAME, "VPD functions not initialized!  Set EDBG_CNFG or run edbgdetcnfg.\n");
  }

  // Get the path to the eepromFile and error if not there
  std::string eepromFile = eeproms[i_target];

  // Open the device
  rc = vpd.openDevice(eepromFile);
  if (rc) {
    return rc;
  }

  // Put the keyword data
  rc = vpd.putKeyword(i_recordName, i_keyword, i_data);
  if (rc) {
    return rc;
  }

  // All done, close the device
  rc = vpd.closeDevice();
  if (rc) {
    return rc;
  }

  return rc;
}

uint32_t dllGetFruVpdKeywordFromImage(const ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, uint32_t i_bytes, const ecmdDataBuffer & i_image_data, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  lhtVpdFile vpd;

  // Load the image into the class
  rc = vpd.setImage(i_image_data);
  if (rc) {
    return rc;
  }

  // Get the keyword data
  rc = vpd.getKeyword(i_recordName, i_keyword, o_data);
  if (rc) {
    return rc;
  }

  // If the call returned more than was asked for, shrink it
  if (o_data.getByteLength() > i_bytes) {
    o_data.shrinkBitLength(i_bytes * 8);
  }

  return rc;
}

uint32_t dllPutFruVpdKeywordToImage(const ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, ecmdDataBuffer & io_image_data, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  lhtVpdFile vpd;

  // Load the image into the class
  rc = vpd.setImage(io_image_data);
  if (rc) {
    return rc;
  }

  // Put the keyword data
  rc = vpd.putKeyword(i_recordName, i_keyword, i_data);
  if (rc) {
    return rc;
  }

  // Pull out the modified image
  rc = vpd.getImage(io_image_data);

  return rc;
}
#endif // ECMD_REMOVE_VPD_FUNCTIONS

#ifndef ECMD_REMOVE_RING_FUNCTIONS
/* ################################################################# */
/* Ring Functions - Ring Functions - Ring Functions - Ring Functions */
/* ################################################################# */
uint32_t dllQueryRing(const ecmdChipTarget & i_target, std::list<ecmdRingData> & o_queryData, const char * i_ringName, ecmdQueryDetail_t i_detail) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRing(const ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutRing(const ecmdChipTarget & i_target, const char * i_ringName, const ecmdDataBuffer & i_data, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRingWithModifier(const ecmdChipTarget & i_target, uint32_t i_address, uint32_t i_bitLength, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutRingWithModifier(const ecmdChipTarget & i_target, uint32_t i_address, uint32_t i_bitLength, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRingSparse(const ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, const ecmdDataBuffer & i_mask, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutRingSparse(const ecmdChipTarget & i_target, const char * i_ringName, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryRingIgnoreMask(const ecmdChipTarget & i_target, const std::string i_ringName, ecmdDataBuffer & o_ignoreMask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryRingInversionMask(const ecmdChipTarget & i_target, const std::string i_ringName, ecmdDataBuffer & o_inversionMask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRingSparseWithTraceMask(const ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, const ecmdDataBuffer & i_mask, const ecmdDataBuffer & i_traceMask, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_RING_FUNCTIONS

#ifndef ECMD_REMOVE_CLOCK_FUNCTIONS
/* ##################################################################### */
/* Clock Functions - Clock Functions - Clock Functions - Clock Functions */
/* ##################################################################### */
uint32_t dllQueryClockState(const ecmdChipTarget & i_target, const char * i_clockDomain, ecmdClockState_t & o_clockState) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllStartClocks(const ecmdChipTarget & i_target, const char * i_clockDomain, bool i_forceState) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllStopClocks(const ecmdChipTarget & i_target, const char * i_clockDomain, bool i_forceState, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_CLOCK_FUNCTIONS

#ifndef ECMD_REMOVE_MEMORY_FUNCTIONS
/* ############################################################# */
/* Mem Functions - Mem Functions - Mem Functions - Mem Functions */
/* ############################################################# */
uint32_t dllGetMemProc(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data, uint32_t i_mode) {
  uint32_t rc = ECMD_SUCCESS;
  uint8_t *buf;
  struct pdbg_target *mem_target;

  // Get any mem level pdbg target for the call
  // Make sure the pdbg target probe has been done and get the target state
  bool memFound = false;
  pdbg_for_each_class_target("mem", mem_target) {
    if (pdbg_target_probe(mem_target) == PDBG_TARGET_ENABLED) {
      memFound = true;
      break;
    }
  }

  // If we don't have any available chip, gotta bail
  if (!memFound) {
    return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "No chip for getmem found!\n");
  }

  // Check our length
  if (i_bytes == 0) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "i_bytes must be > 0\n");
  }

  // Allocate a buffer to receive the data
  buf = (uint8_t *)malloc(i_bytes);

  // Set the block size
  uint32_t blockSize = 0;

  //default cache inhibit is false
  bool ci = false;

  // Make the right call depending on the mode
  if (i_mode == MEMPROC_CACHE_INHIBIT) {
    ci = true;
    if (i_bytes == 1) {
      blockSize = 1;
    } else if (i_bytes == 2) {
      blockSize = 2;
    } else if (i_bytes == 4) {
      blockSize = 4;
    } else {
      blockSize = 8;
    }
  }
  rc = mem_read(mem_target, i_address, buf, i_bytes, blockSize, ci);
  if (rc) {
    // Cleanup
    free(buf);
    return out.error(rc, FUNCNAME, "Error calling pdbg getmem\n");
  }

  // Extract our data and free our buffer
  o_data.setByteLength(i_bytes);
  o_data.memCopyIn(buf, i_bytes);
  free(buf);

  return rc;
}

uint32_t dllPutMemProc(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data, uint32_t i_mode) {
  uint32_t rc = ECMD_SUCCESS;
  uint8_t *buf;
  struct pdbg_target *mem_target;

  // Get any mem level pdbg target for the call
  // Make sure the pdbg target probe has been done and get the target state
  bool memFound = false;
  pdbg_for_each_class_target("mem", mem_target) {
    if (pdbg_target_probe(mem_target) == PDBG_TARGET_ENABLED) {
      memFound = true;
      break;
    }
  }

  // If we don't have any available chip, gotta bail
  if (!memFound) {
    return out.error(ECMD_TARGET_NOT_CONFIGURED, FUNCNAME, "No chip for putmem found!\n");
  }

  // Check our length
  if (i_bytes == 0) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "i_bytes must be > 0\n");
  }

  // Allocate a buffer and load in the data
  buf = (uint8_t *)malloc(i_bytes);
  i_data.memCopyOut(buf, i_bytes);

  // Set the block size
  uint32_t blockSize = 0;

  //default cache inhibit is false
  bool ci = false;

  // Make the right call depending on the mode
  if (i_mode == MEMPROC_CACHE_INHIBIT) {
    ci = true;
    if (i_bytes == 1) {
      blockSize = 1;
    } else if (i_bytes == 2) {
      blockSize = 2;
    } else if (i_bytes == 4) {
      blockSize = 4;
    } else {
      blockSize = 8;
    }
  }
  rc = mem_write(mem_target, i_address, buf, i_bytes, blockSize, ci);

  // Cleanup and check rc
  free(buf);
  if (rc) {
    return out.error(rc, FUNCNAME, "Error calling pdbg putmem\n");
  }

  return rc;
}

uint32_t dllGetMemDma(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutMemDma(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetMemMemCtrl(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutMemMemCtrl(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetSram(const ecmdChipTarget & i_target, uint32_t i_channel, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutSram(const ecmdChipTarget & i_target, uint32_t i_channel, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryCache(const ecmdChipTarget & i_target, ecmdCacheType_t i_cacheType, ecmdCacheData & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllCacheFlush(const ecmdChipTarget & i_target, ecmdCacheType_t i_cacheType) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetMemPba(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, ecmdDataBuffer & o_data, uint32_t i_mode) {

  uint32_t rc = ECMD_SUCCESS;
  uint8_t *buf;
  struct pdbg_target *mem_target;

  //default cache inhibit is false
  bool ci = false;
  
  // Check our length
  if (i_bytes == 0) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "i_bytes must be > 0\n");
  }
  
  // Allocate a buffer to receive the data
  buf = (uint8_t *)malloc(i_bytes);

  // Get any mem level pdbg target for the call
  // Make sure the pdbg target probe has been done and get the target state
  pdbg_for_each_class_target("pib", mem_target) {
      char mem_path[128];
      struct pdbg_target *mem;
      
      // Set the block size
      uint32_t blockSize = 0;

      //Check for matching target index
      if (pdbg_target_index(mem_target) != i_target.chipUnitNum)
          continue;
      
      sprintf(mem_path, "/%s%u", "mempba", pdbg_target_index(mem_target));

      mem = pdbg_target_from_path(NULL, mem_path);
      if (!mem)
          continue;
   
      if (pdbg_target_probe(mem) != PDBG_TARGET_ENABLED)
          continue;

      // Make the right call depending on the mode
      if (i_mode == PBA_MODE_CACHE_INHIBIT) {
          ci = true;
          if (i_bytes == 1) {
            blockSize = 1;
          } else if (i_bytes == 2) {
            blockSize = 2;
          } else if (i_bytes == 4) {
            blockSize = 4;
          } else {
            blockSize = 8;
          }
      }

      rc = mem_read(mem, i_address, buf, i_bytes, blockSize, ci);
      if (rc) {
          // Cleanup
          free(buf);
          return out.error(EDBG_READ_ERROR, FUNCNAME,"Unable to read memory from %s\n",
				    pdbg_target_path(mem));
      }
  }

  // Extract our data and free our buffer
  o_data.setByteLength(i_bytes);
  o_data.memCopyIn(buf, i_bytes);
  free(buf);

  return rc;
}

uint32_t dllPutMemPba(const ecmdChipTarget & i_target, uint64_t i_address, uint32_t i_bytes, const ecmdDataBuffer & i_data, uint32_t i_mode) {
  
  uint32_t rc = ECMD_SUCCESS;
  uint8_t *buf;
  struct pdbg_target *mem_target;

  //default cache inhibit is false
  bool ci = false;
  
  // Check our length
  if (i_bytes == 0) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "i_bytes must be > 0\n");
  }
  
  // Allocate a buffer to receive the data
  buf = (uint8_t *)malloc(i_bytes);
  i_data.memCopyOut(buf, i_bytes);

  // Get any mem level pdbg target for the call
  // Make sure the pdbg target probe has been done and get the target state
  pdbg_for_each_class_target("pib", mem_target) {
      char mem_path[128];
      struct pdbg_target *mem;

      // Set the block size
      uint32_t blockSize = 0;
      
      //Check for matching target index
      if (pdbg_target_index(mem_target) != i_target.chipUnitNum)
          continue;
      
      sprintf(mem_path, "/%s%u", "mempba", pdbg_target_index(mem_target));

      mem = pdbg_target_from_path(NULL, mem_path);
      if (!mem)
          continue;
   
      if (pdbg_target_probe(mem) != PDBG_TARGET_ENABLED)
          continue;

      // Make the right call depending on the mode
      if (i_mode == PBA_MODE_CACHE_INHIBIT) {
         out.print("Mode is cache inhibit:%d,%d\n", i_mode, i_bytes);
         ci = true;
         if (i_bytes < 128) {
           out.error(EDBG_WRITE_ERROR, FUNCNAME, 
                     "CI mode write needs minimum 128 bytes and above\n"); 
         } else {
           //Block size of 8 bytes default for CI mode.
           blockSize = 8;
         }
      }
      rc = mem_write(mem, i_address, buf, i_bytes, blockSize, ci);
      // Cleanup
      free(buf);
      if (rc) {
          return out.error(EDBG_WRITE_ERROR, FUNCNAME,"Unable to write to memory %s\n",
				    pdbg_target_path(mem));
      }
  }
  return rc;
}

uint32_t dllQueryHostMemInfo( const std::vector<ecmdChipTarget> & i_targets, ecmdChipTarget & o_target,  uint64_t & o_address, uint64_t & o_size, const uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryHostMemInfoRanges( const std::vector<ecmdChipTarget> & i_targets, ecmdChipTarget & o_target, std::vector<std::pair<uint64_t,  uint64_t> > & o_ranges, const uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_MEMORY_FUNCTIONS

#ifdef EDBG_ISTEP_CTRL_FUNCTIONS

static edbgIPLTable g_edbgIPLTable;

/**
  * @brief This is a helper function to call the libipl interface
  *        to execute an istep by range of istep names
  *
  * @param uint16_t i_major_start - Start Major number to execute
  * @param uint16_t i_major_end   - End Major number to execute
  * @param uint16_t i_minor_start - Minor number start
  * @param uint16_t i_minor_end   - Minor number end
  *
  * @return Upon success, ECMD_SUCCESS will be returned.  A reason code will
  *         be returned if the execution fails.
  */
uint32_t iStepsHelper(uint16_t i_major_start,
                      uint16_t i_major_end,
                      uint16_t i_minor_start,
                      uint16_t i_minor_end)
{
  uint32_t rc = ECMD_SUCCESS;
  uint16_t l_minor_start, l_minor_end  = edbgIPLTable::EDBG_INVALID_POSITION;
  uint16_t l_start_index = edbgIPLTable::EDBG_INVALID_POSITION;
  std::string IStepName;

  edbgIPLTable::edbgIStepDestination_t l_destination =
                edbgIPLTable::EDBG_ISTEP_INVALID_DESTINATION;
  if (i_major_start == 0)
  {
      /* istep power on */
      rc = g_edbgIPLTable.istepPowerOn();
      if (!rc)
      {
          //Set IPL mode to interactive
          rc = ipl_init(IPL_HOSTBOOT);
          if(rc)
          {
	      return out.error(rc, FUNCNAME,
                               "Unable to set IPL in interactive mode\n");
          }
      }
      else
      {   //TODO:
          /****************************************************/
          /* Error Handling                                   */
	  /****************************************************/
	  return out.error(rc, FUNCNAME, "FAIL: istepPowerOn\n");
      }
  }

  //Setting phal log level
  setPhalLogLevel();

  /* loop through each major & minor isteps */
  /* kick off isteps */
  for (uint16_t major_istep = i_major_start; major_istep <= i_major_end; major_istep++) {

      // if major number incrementer is < end major number then, execute all
      // minor isteps. else execute only till istep requested.
      if (major_istep < i_major_end) {
          l_minor_end =
          g_edbgIPLTable.getIStepMinorNumber(g_edbgIPLTable.getPosLastMinorNumber(major_istep));
      } else {
          l_minor_end = i_minor_end;
      }

      // if major number incrementer is > major number started with then,
      // update start point. else keep the original.
      if (major_istep > i_major_start) {
          l_minor_start =
          g_edbgIPLTable.getIStepMinorNumber(g_edbgIPLTable.getPosFirstMinorNumber(major_istep));
      } else {
          l_minor_start = i_minor_start;
      }

      for (uint16_t minor_istep = l_minor_start; minor_istep <= l_minor_end; minor_istep++) {

          auto l_istep_index_minor_end = g_edbgIPLTable.getPosLastMinorNumber(major_istep);
          auto l_istep_minor_end = g_edbgIPLTable.getIStepMinorNumber(l_istep_index_minor_end);

          // if minor number becomes greater than the last istep in the
          // major istep then, break the inner loop.
          if (minor_istep > l_istep_minor_end) {
              break;
          }

          g_edbgIPLTable.getIStepName(major_istep, minor_istep, IStepName);
          l_start_index = g_edbgIPLTable.getPosition(IStepName);
          l_destination = g_edbgIPLTable.getDestination(l_start_index);

          //This istep is NOOP
          if ( l_destination == edbgIPLTable::EDBG_ISTEP_NOOP ) {
              out.print("Requested istep %s is NOOP\n", IStepName.c_str());
          }
          else
          {
              /* kick off isteps */
              rc = ipl_run_major_minor(major_istep, minor_istep);
              if (!rc)
              {
                  out.print("PASS: istep %s\n",IStepName.c_str());
	      }
              else
              {   //TODO:
                  /****************************************************/
                  /* Error Handling                                   */
                  /****************************************************/
                  return out.error(rc, FUNCNAME, "FAIL: istep %s  - Check Error\n",
			           IStepName.c_str());
              }
          }
      }
  }
  return rc;
}

/**
  * @brief This is a helper function to call the pdbg interface
  *        to execute an istep
  *
  * @param uint16_t i_major - Major number to execute
  * @param uint16_t i_minor_start - Minor number start
  * @param uint16_t i_minor_end   - Minor number end
  *
  * @return Upon success, ECMD_SUCCESS will be returned.  A reason code will
  *         be returned if the execution fails.
  */
uint32_t iStepsHelper(uint16_t i_major,
                      uint16_t i_minor_start,
                      uint16_t i_minor_end)
{
    uint32_t rc = ECMD_SUCCESS;
    uint16_t l_start_index  = edbgIPLTable::EDBG_INVALID_POSITION;
    std::string IStepName;

    edbgIPLTable::edbgIStepDestination_t l_destination =
                      edbgIPLTable::EDBG_ISTEP_INVALID_DESTINATION;

    // If istep is 0 then, run chassis on and other workaround steps before
    // kick off ipl_run_major_minor() in loop
    if (i_major == 0)
    {
       /* istep power on */
       rc = g_edbgIPLTable.istepPowerOn();
       if (!rc)
       {
           //Set IPL mode to interactive
           rc = ipl_init(IPL_HOSTBOOT);
           if(rc)
           {
	       return out.error(rc, FUNCNAME,
                                "Unable to set IPL in interactive mode\n");
           }
       }
       else
       {    //TODO:
            /****************************************************/
            /* Error Handling                                   */
	    /****************************************************/
	    return out.error(rc, FUNCNAME, "FAIL: istepPowerOn\n");
       }
    }

    //Setting pHAL log level
    setPhalLogLevel();

    /* loop through each isteps */
    for (uint16_t istep = i_minor_start; istep <= i_minor_end ; istep++) {
        g_edbgIPLTable.getIStepName(i_major, istep, IStepName);
        l_start_index = g_edbgIPLTable.getPosition(IStepName);
        l_destination = g_edbgIPLTable.getDestination(l_start_index);

        //This istep is NOOP
        if ( l_destination == edbgIPLTable::EDBG_ISTEP_NOOP ) {
            out.print("Requested istep %s is NOOP\n", IStepName.c_str());
        }
        else
        {
            /* kick off isteps */
            rc = ipl_run_major_minor(i_major, istep);
            if (!rc)
            {
                out.print("PASS: istep %s\n",IStepName.c_str());
	    }
            else
            {   //TODO:
                /****************************************************/
                /* Error Handling                                   */
		/****************************************************/
		return out.error(rc, FUNCNAME, "FAIL: istep %s  - Check Error\n",
				     IStepName.c_str());
            }
        }
    }
    return rc;
} // iStepsHelper

#endif //EDBG_ISTEP_CTRL_FUNCTIONS

#ifndef ECMD_REMOVE_INIT_FUNCTIONS

/* ##################################################################### */
/* istep Functions - istep Functions - istep Functions - istep Functions */
/* ##################################################################### */
uint32_t dllIStepsByNumber(const ecmdDataBuffer & i_steps) {

#ifdef EDBG_ISTEP_CTRL_FUNCTIONS
    uint32_t rc = ECMD_SUCCESS;
    uint16_t l_istep_index_begin,l_minor_start = edbgIPLTable::EDBG_INVALID_POSITION;
    uint16_t l_istep_index_end,l_minor_end  = edbgIPLTable::EDBG_INVALID_POSITION;
    uint16_t l_active_step = edbgIPLTable::EDBG_INVALID_ISTEP_NUM;

    do   // Start of single exit point loop.
    {
      /****************************************************************************************/
      /* First, check to make sure at least 1 bit is active in i_steps databuffer, then....   */
      /* For each valid bit in the i_steps databuffer:                                        */
      /*   1)  find the index entry #s that start that step and end that step                 */
      /*   2)  Call iStepsHelper()  multiple times, starting with starting index entry #    */
      /*        and ending with the ending entry #.                                           */
      /****************************************************************************************/

      //check to see if at least 1 bit in the range is active
      if (!i_steps.getNumBitsSet(
           edbgIPLTable::EDBG_FIRST_ISTEP_NUM,
           edbgIPLTable::EDBG_LAST_ISTEP_NUM-edbgIPLTable::EDBG_FIRST_ISTEP_NUM+1))
      {  //there are no bits in the range set
          rc = ECMD_INVALID_ARGS;
          out.warning(FUNCNAME,
                      "dllIStepsByNumber: No Steps in active range selected. (Range start: %d, end: %d).\n",
                      edbgIPLTable::EDBG_FIRST_ISTEP_NUM,
                      edbgIPLTable::EDBG_LAST_ISTEP_NUM);

          break;  // exit do-loop
      }

      /* Large 'for' loop that goes through i_steps looking for active steps
       * start at begining of range */
      for (l_active_step =  edbgIPLTable::EDBG_FIRST_ISTEP_NUM ;
          /* this step is in the range */
           l_active_step <= edbgIPLTable::EDBG_LAST_ISTEP_NUM &&
           rc == ECMD_SUCCESS;
           ++l_active_step)
      {

          if (i_steps.isBitSet(l_active_step))  //this is an active step
          {
              /*  look IPLTable for the existence of this istep number */
              if ( false == g_edbgIPLTable.isValid(l_active_step) )
              {  /* error - istep number l_active_step not found */

                  /* this is only warning, as the value is in the range, but isn't being used */
                  out.warning(FUNCNAME,
                              "dllIStepsByNumber: Requested iStep Number %d is invalid.\n",
                              l_active_step);
                  continue;

              }
              /* 1a) Lookup first index entry of this active step */
              l_istep_index_begin = g_edbgIPLTable.getPosFirstMinorNumber(l_active_step);
              l_minor_start = g_edbgIPLTable.getIStepMinorNumber(l_istep_index_begin);

              /* 1b) Starting with l_istep_index_begin,
               *     lookup last index entry of this active step */
              l_istep_index_end = g_edbgIPLTable.getPosLastMinorNumber(l_active_step);
              l_minor_end = g_edbgIPLTable.getIStepMinorNumber(l_istep_index_end);

              /* 2) Call iStepsHelper()  multiple times,
               *    starting with starting index entry #
               *    and ending with the ending entry #. */
              /* kick off isteps */
              rc = iStepsHelper(l_active_step,
                                l_minor_start,
                                l_minor_end);

          }  /*  end of 'if' active step check */

      }  /* end of for loop going through active i_steps */

    }while(0);

    return rc;
#else
    return ECMD_FUNCTION_NOT_SUPPORTED;
#endif // EDBG_ISTEP_CTRL_FUNCTIONS
}

uint32_t dllIStepsByName(std::string i_stepName) {
#ifdef EDBG_ISTEP_CTRL_FUNCTIONS
  uint32_t rc = ECMD_SUCCESS;
  uint16_t o_majorNum, o_minorNum;
  uint16_t l_start_index = edbgIPLTable::EDBG_INVALID_POSITION;
  edbgIPLTable::edbgIStepDestination_t l_destination =
                edbgIPLTable::EDBG_ISTEP_INVALID_DESTINATION;

  // make sure i_stepName is lowercase
  transform(  i_stepName.begin(), i_stepName.end(),
              i_stepName.begin(), (int(*)(int)) tolower);

  if ( false == g_edbgIPLTable.isValid(i_stepName) )
  {
    // error - i_stepName not found!
    if ( i_stepName == "list" )
    {
      //ecmdFileLocation file;
      std::list<ecmdFileLocation> l_fileLocs;
      ecmdChipTarget target;
      std::ifstream ins;
      std::string curLine;
      std::string l_versionString = "default";
      std::string l_fileLoc;

      /* Get the path to the help text files */
      rc = dllQueryFileLocation(target, ECMD_FILE_HELPTEXT, l_fileLocs, l_versionString);
      if (rc != ECMD_SUCCESS)
      {
        return out.error(rc, FUNCNAME, "Couldn't find HELP File path\n");
      }
      else
      {
            l_fileLoc = l_fileLocs.begin()->textFile;
            // now set the help file to the istep cmd list
            l_fileLoc += "istep_list.htxt";

            /* Let's go open this file*/
            ins.open(l_fileLoc.c_str());
            if (ins.fail()) {
              return out.error(ECMD_INVALID_ARGS,FUNCNAME,"Error occured opening help file\n");
            }
            else
            {
              while (getline(ins, curLine)) {
              curLine += '\n';
              out.print(curLine.c_str());
              }
              ins.close();
            }
      }
    }
    else
    {
      return out.error(ECMD_INVALID_ARGS, FUNCNAME, "Invalid istep option\n");
    }
  }
  else
  {
    if (i_stepName == "poweron")
    {
       /* istep power on */
       rc = g_edbgIPLTable.istepPowerOn();
       if (!rc)
       {
           //Set IPL mode to interactive
           rc = ipl_init(IPL_HOSTBOOT);
           if(rc)
           {
	       return out.error(rc, FUNCNAME,
                                "Unable to set IPL in interactive mode\n");
           }
       }
       else
       {    //TODO:
            /****************************************************/
            /* Error Handling                                   */
	    /****************************************************/
	    return out.error(rc, FUNCNAME, "FAIL: istepPowerOn\n");
       }
    }

    //Setting pHAL log level
    setPhalLogLevel();

    l_start_index = g_edbgIPLTable.getPosition(i_stepName);
    l_destination = g_edbgIPLTable.getDestination(l_start_index);

    //This istep is NOOP
    if ( l_destination == edbgIPLTable::EDBG_ISTEP_NOOP ) {
        out.print("Requested istep %s is NOOP\n", i_stepName.c_str());
    } else {
        // i_stepName found so execute iSteps
        // Lookup i_stepName in IPL Table
        g_edbgIPLTable.getIStepNumber(i_stepName, o_majorNum, o_minorNum);

        /* kick off isteps */
        rc = ipl_run_major_minor(o_majorNum, o_minorNum);
        if (!rc)
        {
            out.print("PASS: istep %s\n",i_stepName.c_str());
        }
        else
        {   //TODO:
            /****************************************************/
            /* Error Handling                                   */
            /****************************************************/
            return out.error(rc, FUNCNAME, "FAIL: istep %s  - Check Error\n",
		             i_stepName.c_str());
        }
     }
  }
  return rc;
#else
  return ECMD_FUNCTION_NOT_SUPPORTED;
#endif
}

uint32_t dllIStepsByNameMultiple(std::list< std::string > i_stepNames) {
#ifdef EDBG_ISTEP_CTRL_FUNCTIONS
  uint32_t rc = ECMD_SUCCESS;

  // It is assumed that list passed in is a series of iSteps to be called in
  // order
  for (auto l_stepIter = i_stepNames.begin();
       l_stepIter != i_stepNames.end() ;
       l_stepIter++)
  {
    rc = dllIStepsByName(*l_stepIter);
    if (rc) break;
  }
  return rc;
#else
  return ECMD_FUNCTION_NOT_SUPPORTED;
#endif
}

uint32_t dllIStepsByNameRange(std::string i_stepNameBegin, std::string i_stepNameEnd) {
#ifdef EDBG_ISTEP_CTRL_FUNCTIONS
  uint32_t rc = ECMD_SUCCESS;
  uint16_t l_istep_index_begin,l_minor_start = edbgIPLTable::EDBG_INVALID_POSITION;
  uint16_t l_istep_index_end, l_minor_end  = edbgIPLTable::EDBG_INVALID_POSITION;
  uint16_t o_StartMajorNum, o_StartMinorNum, o_EndMajorNum, o_EndMinorNum;

  /**************************************************************************/
  /*  1)  Find the index entry #s for i_stepNameBegin and i_stepNameEnd     */
  /*  2)  Call iStepsHelper for multiple times, starting with               */
  /*      istepNameBegin's entry # and ending with i_stepNameEnd's entry #  */
  /**************************************************************************/

  // make sure i_stepNameBegin and istepNameEnd are lowercase
  transform(  i_stepNameBegin.begin(), i_stepNameBegin.end(),
              i_stepNameBegin.begin(), (int(*)(int)) tolower);
  transform(  i_stepNameEnd.begin(), i_stepNameEnd.end(),
              i_stepNameEnd.begin(), (int(*)(int)) tolower);

  // Lookup i_stepNameBegin in IPLTable
  if ( false == g_edbgIPLTable.isValid(i_stepNameBegin) )
  {  // error - i_stepNameBegin not found!
    rc = ECMD_ISTEPS_INVALID_STEP;
    return out.error(rc, FUNCNAME, "Requested iStep-Begin not recognized:"
                         " %s. Returning rc=0x%x\n", i_stepNameBegin.c_str(), rc);
  }

  // Get the index of the i_stepNameBegin in IPL Table
  g_edbgIPLTable.getIStepNumber(i_stepNameBegin, o_StartMajorNum, o_StartMinorNum);
  l_istep_index_begin = g_edbgIPLTable.getPosFirstMinorNumber(o_StartMajorNum);

  // Lookup i_stepNameEnd in IPLTable
  if ( false == g_edbgIPLTable.isValid(i_stepNameEnd) )
  {  // error - i_stepNameEnd not found!
    rc = ECMD_ISTEPS_INVALID_STEP;
    return out.error(rc, FUNCNAME, "Requested iStep-End not recognized:"
                         " %s. Returning rc=0x%x\n", i_stepNameEnd.c_str(), rc);
  }

  // Get the index of the i_stepNameEnd in IPL Table
  g_edbgIPLTable.getIStepNumber(i_stepNameEnd, o_EndMajorNum, o_EndMinorNum);
  l_istep_index_end = g_edbgIPLTable.getPosFirstMinorNumber(o_EndMajorNum);

  if ( l_istep_index_begin > l_istep_index_end )
  { // error - i_stepName not found!
    rc = ECMD_ISTEPS_INVALID_STEP;
    return out.error(rc, FUNCNAME, "Requested istep range '%s..%s' is invalid."
                         " %s. Returning rc=0x%x\n", i_stepNameBegin.c_str(),
                         i_stepNameEnd.c_str(), rc);
  }

  l_minor_start =
  g_edbgIPLTable.getIStepMinorNumber(g_edbgIPLTable.getPosition(i_stepNameBegin));

  l_minor_end =
  g_edbgIPLTable.getIStepMinorNumber(g_edbgIPLTable.getPosition(i_stepNameEnd));

  /* kick off isteps */
  rc = iStepsHelper(o_StartMajorNum, o_EndMajorNum, l_minor_start, l_minor_end);
  return rc;
#else
  return ECMD_FUNCTION_NOT_SUPPORTED;
#endif
}

uint32_t dllInitChipFromFile(const ecmdChipTarget & i_target, const char* i_initFile, const char* i_initId, const char* i_mode, uint32_t i_ringMode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSyncIplMode(int i_unused) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_INIT_FUNCTIONS

#ifndef ECMD_REMOVE_PROCESSOR_FUNCTIONS
/* ################################################################# */
/* proc Functions - proc Functions - proc Functions - proc Functions */
/* ################################################################# */

// enums for Reg access register type
enum sbeRegAccesRegType
{
    SBE_REG_ACCESS_GPR  = 0x00,
    SBE_REG_ACCESS_SPR  = 0x01,
    SBE_REG_ACCESS_FPR  = 0x02
};

enum ecmdRegAccessOperation {
  ECMD_GET_SPR_ACCESS,
  ECMD_GET_GPR_ACCESS,
  ECMD_GET_FPR_ACCESS,
  ECMD_PUT_SPR_ACCESS,
  ECMD_PUT_GPR_ACCESS,
  ECMD_PUT_FPR_ACCESS
};

uint32_t dllQueryProcRegisterInfo(const ecmdChipTarget & i_target, const char* i_name, ecmdProcRegisterInfo & o_data) {
  // Fill the respective details in "ecmdProcRegisterInfo" structure
  // based on register type
  //Translates to ecmd command: ecmdquery procregs Chip.chipunit [procregistername] 
 
  uint32_t rc = ECMD_SUCCESS;
  sbeRegAccesRegType l_regType;
  std::string l_name ( i_name );

  // Convert i_name to uppercase
  transform( l_name.begin(), l_name.end(), l_name.begin(), (int(*)(int)) toupper );

  if (l_name == "GPR") {
     l_regType = SBE_REG_ACCESS_GPR;
  } else if (l_name == "FPR") {
      l_regType = SBE_REG_ACCESS_FPR;
  } else if (l_name == "SPR") {
      l_regType = SBE_REG_ACCESS_SPR;
  } else{ //spr passed in default to check if string falls under spr category.
      l_regType = SBE_REG_ACCESS_SPR;
  }

  // We will the structures to make sure 
  // ecmdquery procregs Chip.chipunit [procregistername] is populated correctly.
  if (l_regType == SBE_REG_ACCESS_GPR)
  {
      o_data.bitLength = 64;
      o_data.totalEntries = 32;
      o_data.threadReplicated = true;
  }
  else if (l_regType == SBE_REG_ACCESS_FPR)
  {
      o_data.bitLength = 64;
      o_data.totalEntries = 32;
      o_data.threadReplicated = true;
  }
  //Will update the actual values based on hw procedure.
  else if (l_regType == SBE_REG_ACCESS_SPR)
  {
      //Initialize the SPR map.
      bool l_mapinit =  p10_spr_name_map_init();
      if (!l_mapinit)
      {
          return out.error(EDBG_GENERAL_ERROR, FUNCNAME,
                           "Failed in p10_spr_name_map_init() to initialize.\n");
      }
      //Check if the spr register name is valid
      if(SPR_MAP.find(l_name) == SPR_MAP.end())
      {
          return out.error(EDBG_GENERAL_ERROR, FUNCNAME,
                           "Input spr reg name: %s, is not valid\n "
                          ,l_name.c_str());
      }

      SPRMapEntry l_sprMapEntry;
      bool l_EntryValid =  p10_get_spr_entry(l_name,l_sprMapEntry);
      if (!l_EntryValid)
      {
          return out.error(EDBG_GENERAL_ERROR, FUNCNAME,
                           "Failed in p10_get_spr_entry() to fetch the SPR "
                           "entry for the input SPR name %s\n", l_name.c_str());
      }

      o_data.bitLength = l_sprMapEntry.bit_length;
      // totalEntries for each SPR is "1" since each SPR name has
      // only 1 instance.
      o_data.totalEntries = 1;

      //Register is replicated for each thread will be false 
      //unless shared across threads in that core. i.e. 
      //SPR per Physical thread or virtual thread or N/A (All at core level)
      if( (l_sprMapEntry.share_type == SPR_PER_PT) ||  
         (l_sprMapEntry.share_type == SPR_PER_VT) ||
         (l_sprMapEntry.share_type == SPR_SHARE_NA) )
      {
          o_data.threadReplicated = true;
      }
      else
      {
          o_data.threadReplicated = false;
      }
      
      switch ( l_sprMapEntry.flag )
      {
          case FLAG_READ_ONLY: //write is not allowed
              o_data.mode = ECMD_PROCREG_READ_ONLY;
              break;
          case FLAG_WRITE_ONLY: //read is not allowed
              o_data.mode = ECMD_PROCREG_WRITE_ONLY;
              break;
          case FLAG_READ_WRITE: //rw
              o_data.mode = ECMD_PROCREG_READ_AND_WRITE;
              break;
          default:
              o_data.mode = ECMD_PROCREG_UNKNOWN;
              break;
      }
    
      o_data.isChipUnitRelated = true;

      //Will always default chip unit to pu.c irrespective of 
      //fused core or not.    
      o_data.relatedChipUnit = "c";
      o_data.relatedChipUnitShort = "c";
  }
  return rc;
}

uint32_t ecmdRegAccessHelper(ecmdChipTarget &i_target,std::list<ecmdIndexEntry> &io_entries,
                             ecmdRegAccessOperation i_opType)
{
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *thread, *core;
  std::list <ecmdIndexEntry>::iterator l_ecmdIndexEntryIter;
  
  // check to make sure list isn't empty
  if ( io_entries.size() == 0 ) {  
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "io_entries list is empty\n");
  }

  rc = mapEcmdCoreToPdbgCoreTarget(i_target, &core);
  if (rc) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                     "Unable to find core target in p%d\n", i_target.pos);
  }

  pdbg_for_each_target("thread", core, thread) {
      uint64_t data = 0;

      // Check if the requested thread is matching the pdbg target index. then,
      // we operate on that thread. we are getting thread num as always 0 so, 
      // we will get and display value for thread0.
      if (pdbg_target_index(thread) != i_target.thread)
          continue;
      
      if (pdbg_target_probe(thread) != PDBG_TARGET_ENABLED)
          continue;
 
      for(l_ecmdIndexEntryIter = io_entries.begin(); 
                                 l_ecmdIndexEntryIter != io_entries.end(); 
                                 l_ecmdIndexEntryIter++){
    
          switch (i_opType)
          {
              case ECMD_GET_GPR_ACCESS:
                  rc = thread_getgpr(thread, l_ecmdIndexEntryIter->index, &data);
                  if (rc != 0) 
                  {
                      return out.error(EDBG_READ_ERROR, FUNCNAME,
                                       "getgpr of 0x%016" PRIx64 " = 0x%016" PRIx64 " failed \n",
                                        l_ecmdIndexEntryIter->index, data);
                  }
                  l_ecmdIndexEntryIter->buffer.setBitLength(64);
                  l_ecmdIndexEntryIter->buffer.setDoubleWord(0,data);
                  break;
              case ECMD_PUT_GPR_ACCESS: 
                  rc = thread_putgpr(thread, l_ecmdIndexEntryIter->index, 
                                     l_ecmdIndexEntryIter->buffer.getDoubleWord(0));
                  if (rc != 0) 
                  {
                      return out.error(EDBG_WRITE_ERROR, FUNCNAME,
                                       "putgpr of 0x%016" PRIx64 " failed \n",
                                        l_ecmdIndexEntryIter->index);
                  }
                  break;
              case ECMD_GET_SPR_ACCESS:
                  rc = thread_getspr(thread, l_ecmdIndexEntryIter->index, &data);
                  if (rc != 0) 
                  {
                      return out.error(EDBG_READ_ERROR, FUNCNAME,
                                       "getspr of 0x%016" PRIx64 " failed \n",
                                        l_ecmdIndexEntryIter->index);
                  }
                  l_ecmdIndexEntryIter->buffer.setBitLength(64);
                  l_ecmdIndexEntryIter->buffer.setDoubleWord(0,data);
                  break;
              case ECMD_PUT_SPR_ACCESS: 
                  rc = thread_putspr(thread, l_ecmdIndexEntryIter->index, 
                                     l_ecmdIndexEntryIter->buffer.getDoubleWord(0));
                  if (rc != 0) 
                  {
                      return out.error(EDBG_WRITE_ERROR, FUNCNAME,
                                       "putspr of 0x%016" PRIx64 " failed \n",
                                        l_ecmdIndexEntryIter->index);
                  }
                  break;
              default:
                  return out.error(EDBG_GENERAL_ERROR, FUNCNAME,
                                   "Invalid operation requested. \n" );
          }
      }
  }
  return rc;
}  

uint32_t ecmdSPRNameToIndex(const char * i_spr_name,const bool i_flag,uint32_t& o_index)
{
  uint32_t rc = ECMD_SUCCESS;
  bool l_mapInit = true;
    
  //Initialize the map between SPR name and SPR number
  l_mapInit = p10_spr_name_map_init();
  if (!l_mapInit)
  {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                       "SPR name map initialization failed\n");

  }
  l_mapInit = p10_spr_name_map(i_spr_name, i_flag, o_index);
  if(!l_mapInit)
  {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                       "p10_spr_name_map() returned failure\n");
  }
  return rc;
}

uint32_t dllGetSpr(const ecmdChipTarget & i_target, const char * i_sprName, ecmdDataBuffer & o_data) {
  
  uint32_t rc = ECMD_SUCCESS;
  std::string l_spr_name ( i_sprName );
  ecmdNameEntry l_entry;
  std::list<ecmdNameEntry> l_entryList;

  l_entry.name = l_spr_name;
  l_entry.buffer = o_data;
  l_entry.rc = rc;
  l_entryList.push_back(l_entry);

  rc = dllGetSprMultiple(i_target, l_entryList);
  if ( rc ) {
      return rc;
  }
  o_data = l_entryList.begin()->buffer;
  return rc;
}

uint32_t dllGetSprMultiple(const ecmdChipTarget & i_target, std::list<ecmdNameEntry> & io_entries) {
  uint32_t rc = ECMD_SUCCESS;
  std::list<ecmdIndexEntry> l_entryList;
  std::list<ecmdIndexEntry>::iterator l_indexEntryIter;
  std::list<ecmdNameEntry>::iterator l_nameEntryIter;
  ecmdChipTarget l_target = i_target;

  // Loop through ecmdNameEntry list
  // For each entry hash the uppercase of the sprName, and copy that plus
  // ecmdDataBuffer and rc into the local entry;  then push that entry onto the list
  for (l_nameEntryIter = io_entries.begin(); l_nameEntryIter != io_entries.end(); ++l_nameEntryIter) {
    ecmdIndexEntry l_entry;

    // uppercase of SPR name and put it into l_entry.index
    transform( l_nameEntryIter->name.begin(), l_nameEntryIter->name.end(), l_nameEntryIter->name.begin(), (int(*)(int)) toupper );

    uint32_t l_index;
    rc = ecmdSPRNameToIndex(l_nameEntryIter->name.c_str(),false,l_index);
    if(rc)
    {
        return out.error(rc, FUNCNAME, 
                         "GetSpr: Invalid SPR: %s\n", l_nameEntryIter->name.c_str());  
    }
    l_entry.index = l_index;
    l_entry.buffer = l_nameEntryIter->buffer;
    l_entry.rc = l_nameEntryIter->rc;

    l_entryList.push_back(l_entry);
  }

  rc = ecmdRegAccessHelper(l_target, l_entryList, ECMD_GET_SPR_ACCESS);    
  if ( rc != 0) {
      return out.error(EDBG_READ_ERROR, FUNCNAME, "Getspr failed! \n" );;
  }
 
  // Clear io_entries, as we're going to make a new ecmdNameEntry list that maps to l_entryList
  io_entries.clear();
 
  // Now we map back the index to spr name and push back the results 
  ecmdNameEntry l_nameEntry;
  for (l_indexEntryIter = l_entryList.begin(); l_indexEntryIter != l_entryList.end(); ++l_indexEntryIter) {
      l_nameEntry.buffer = l_indexEntryIter->buffer;
      l_nameEntry.rc = l_indexEntryIter->rc;
      rc = ecmdMapSpr2Str((uint32_t&)l_indexEntryIter->index, l_nameEntry.name);
      if (rc != 0) {
          return out.error(rc, FUNCNAME, 
                           "SPR ID to SPR string map failed! rc = 0x%08x, value = 0x%08x\n", 
                           rc, l_indexEntryIter->index);
      }
      io_entries.push_back(l_nameEntry);
  }
  return rc;
}

uint32_t dllPutSpr(const ecmdChipTarget & i_target, const char * i_sprName, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  std::string l_spr_name ( i_sprName );
  std::list<ecmdNameEntry> l_entryList;
  ecmdNameEntry l_entry;
  
  l_entry.name = l_spr_name;
  l_entry.buffer = i_data;
  l_entry.rc = rc;
  l_entryList.push_back(l_entry);

  rc = dllPutSprMultiple(i_target, l_entryList);
  if ( rc ) {
      return rc;
  }
  return rc;
}

uint32_t dllPutSprMultiple(const ecmdChipTarget & i_target, std::list<ecmdNameEntry> & io_entries) {
  uint32_t rc = ECMD_SUCCESS;
  std::list<ecmdIndexEntry> l_entryList;
  std::list<ecmdNameEntry>::iterator l_nameEntryIter;
  ecmdChipTarget l_target = i_target;

  for (l_nameEntryIter = io_entries.begin(); l_nameEntryIter != io_entries.end(); ++l_nameEntryIter) {
    ecmdIndexEntry l_entry;
    
    // uppercase of SPR name and put it into l_entry.index
    transform( l_nameEntryIter->name.begin(), l_nameEntryIter->name.end(), l_nameEntryIter->name.begin(), (int(*)(int)) toupper );
    
    uint32_t l_index;
    rc = ecmdSPRNameToIndex(l_nameEntryIter->name.c_str(),true,l_index);
    if ( rc != 0) {
      return out.error(EDBG_GENERAL_ERROR, FUNCNAME, 
                       "Invalid SPR: %s\n", l_nameEntryIter->name.c_str());
    }
    l_entry.index = l_index;
    l_entry.buffer = l_nameEntryIter->buffer;
    l_entry.rc = l_nameEntryIter->rc;
    l_entryList.push_back(l_entry);

  }
  rc = ecmdRegAccessHelper(l_target, l_entryList, ECMD_PUT_SPR_ACCESS);  
  if ( rc != 0) {
      return out.error(EDBG_WRITE_ERROR, FUNCNAME, "Putspr failed! \n" );;
  }
  return rc;  
}
   
uint32_t dllGetGpr(const ecmdChipTarget & i_target, uint32_t i_gprNum, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  std::list<ecmdIndexEntry> l_entryList;
  ecmdIndexEntry l_entry;
  ecmdChipTarget l_target = i_target;

  l_entry.index = i_gprNum;
  l_entry.buffer = o_data;
  l_entry.rc = rc;
  l_entryList.push_back(l_entry); 

  rc = ecmdRegAccessHelper(l_target, l_entryList, ECMD_GET_GPR_ACCESS);
  if ( rc != 0) {
      return out.error(EDBG_READ_ERROR, FUNCNAME, "Getgpr failed! \n" );;
  }
  o_data = l_entryList.begin()->buffer;
  return rc;
}

uint32_t dllGetGprMultiple(const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries) {
  ecmdChipTarget l_target = i_target;
  return ecmdRegAccessHelper(l_target, io_entries, ECMD_GET_GPR_ACCESS);
}

uint32_t dllPutGpr(const ecmdChipTarget & i_target, uint32_t i_gprNum, const ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  std::list<ecmdIndexEntry> l_entryList;
  ecmdIndexEntry l_entry;
  ecmdChipTarget l_target = i_target;

  l_entry.index = i_gprNum;
  l_entry.buffer = i_data;
  l_entry.rc = rc;
  l_entryList.push_back(l_entry); 

  rc = ecmdRegAccessHelper(l_target, l_entryList, ECMD_PUT_GPR_ACCESS);
  if ( rc != 0) {
      return out.error(EDBG_WRITE_ERROR, FUNCNAME, "Putgpr failed! \n" );;
  }
  return rc;
}

uint32_t dllPutGprMultiple(const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries) {
  ecmdChipTarget l_target = i_target;
  return ecmdRegAccessHelper(l_target, io_entries, ECMD_PUT_GPR_ACCESS);
}

uint32_t dllGetFpr(const ecmdChipTarget & i_target, uint32_t i_fprNum, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetFprMultiple(const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutFpr(const ecmdChipTarget & i_target, uint32_t i_fprNum, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutFprMultiple(const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetSlb(const ecmdChipTarget & i_target, uint32_t i_slbNum, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetSlbMultiple(const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutSlb(const ecmdChipTarget & i_target, uint32_t i_slbNum, const ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutSlbMultiple(const ecmdChipTarget & i_target, std::list<ecmdIndexEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
#endif // ECMD_REMOVE_PROCESSOR_FUNCTIONS
