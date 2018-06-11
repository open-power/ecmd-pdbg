//IBM_PROLOG_BEGIN_TAG
/*
 * Copyright 2017,2017 IBM International Business Machines Corp.
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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <yaml.h>
#include <libgen.h>
#include <assert.h>
#include <map>

// Headers from eCMD
#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBuffer.H>
#include <ecmdSharedUtils.H>
#include <ecmdChipTargetCompare.H>

// Headers from pdbg
extern "C" {
#include <libpdbg.h>
}

// Headers from ecmd-pdbg
#include <edbgCommon.H>
#include <edbgOutput.H>
#include <lhtVpdFile.H>
#include <lhtVpdDevice.H>
#include <p9_scominfo.H>

// TODO: This needs to not be hardcoded and set from the command-line.
std::string DEVICE_TREE_FILE;

// Store eeprom locations, indexed by target
std::map<ecmdChipTarget, std::string, ecmdChipTargetCompare> eeproms;
// Store if we have init'd the VPD info
bool vpdInit = false;

// For use by dllQueryConfig and dllQueryExist
uint32_t queryConfigExist(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistCages(ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistNodes(ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistSlots(ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChips(ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChipUnits(ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistThreads(ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::list<ecmdThreadData> & o_threadData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);

// Used to translate an ecmdChipTarget to a pdbg target
uint32_t fetchPdbgTarget(ecmdChipTarget & i_target, struct pdbg_target * o_pdbgTarget);

std::string gEDBG_HOME;

/* ################################################################################################# */
/* Static functions used to lookup pdbg targets 						     */
/* ################################################################################################# */
static uint32_t fetchPdbgInterfaceTarget(ecmdChipTarget & i_target, struct pdbg_target **o_target, const char *interface) {
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
static uint32_t fetchPibTarget(ecmdChipTarget & i_target, struct pdbg_target **o_pibTarget) {
  if (fetchPdbgInterfaceTarget(i_target, o_pibTarget, "pib")) {
    printf("Unable to find pib target in p%d\n", i_target.pos);
    return -1;
  }

  return 0;
}

static uint32_t fetchCfamTarget(ecmdChipTarget & i_target, struct pdbg_target **o_pibTarget) {
  if (fetchPdbgInterfaceTarget(i_target, o_pibTarget, "fsi")) {
    printf("Unable to find cfam target in p%d\n", i_target.pos);
    return -1;
  }

  return 0;
}

/* Given a target in i_target this will return the associcated pdbg target that
 * can be passed to pib_read/write() such that full address translation is
 * performed. For example - getscom pu.ex -c6 20000100 would get translated to
 * pib_read(pdbg_target, 0x100) which would get translated by pib_read() to
 * 0x26000100. */
static uint32_t fetchPdbgTarget(ecmdChipTarget & i_target, struct pdbg_target ** o_pdbgTarget) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *chipTarget, *target;
  uint32_t index;

  assert(i_target.cageState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.cage == 0 &&                                  \
         i_target.nodeState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.node == 0 &&                                  \
         i_target.slotState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.slot == 0 &&                                  \
         i_target.posState == ECMD_TARGET_FIELD_VALID);

  *o_pdbgTarget = NULL;
  pdbg_for_each_class_target("pib", chipTarget) {
    const char *p;

    // Don't search for targets not matched to our index/position
    index = pdbg_target_index(chipTarget);
    if (i_target.pos != index)
      continue;

    // Just return the raw pib target if we're not looking for a specific chip unit
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_UNUSED) {
      *o_pdbgTarget = chipTarget;
    } else {
      // Search child nodes of this position to find what we are
      // looking for
      pdbg_for_each_child_target(chipTarget, target) {
	p = (char *) pdbg_get_target_property(target, "ecmd,chip-unit-type", NULL);
        if (p &&
            p == i_target.chipUnitType &&
            pdbg_target_index(target) == i_target.chipUnitNum) {
          *o_pdbgTarget = target;
	  break;
        }
      }
    }

    if (*o_pdbgTarget)
      break;
  }

  if (!*o_pdbgTarget) {
    printf("Unable to find pdbg target!\n");
    return 1;
  }

  return rc;
}

/* Given a target and a target base address return the chip unit type (eg. "ex",
 * "mba", etc.) */
static uint32_t findChipUnitType(ecmdChipTarget &i_target, uint64_t i_address, struct pdbg_target **pdbgTarget)
{
  struct pdbg_target *pibTarget, *target;

  if (fetchPibTarget(i_target, &pibTarget)) {
    printf("Unable to find PIB target\n");
    return -1;
  }

  /* Need to mask off the indirect address if present */
  i_address &= 0x7fffffff;

  pdbg_for_each_child_target(pibTarget, target) {
    uint64_t addr, size;

    addr = pdbg_get_address(target, &size);
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

//convert the enum to string for use in code
uint32_t p9n_convertCUEnum_to_String(p9ChipUnits_t i_P9CU, std::string &o_chipUnitType) {
  uint32_t rc = ECMD_SUCCESS;
  
  if (i_P9CU == PU_C_CHIPUNIT)            o_chipUnitType = "c";
  else if (i_P9CU == PU_EQ_CHIPUNIT)      o_chipUnitType = "eq";
  else if (i_P9CU == PU_EX_CHIPUNIT)      o_chipUnitType = "ex";
  else if (i_P9CU == PU_XBUS_CHIPUNIT)    o_chipUnitType = "xbus";
  else if (i_P9CU == PU_OBUS_CHIPUNIT)    o_chipUnitType = "obus";
  else if (i_P9CU == PU_NV_CHIPUNIT)      o_chipUnitType = "nv";
  else if (i_P9CU == PU_PEC_CHIPUNIT)     o_chipUnitType = "pec";
  else if (i_P9CU == PU_PHB_CHIPUNIT)     o_chipUnitType = "phb";
  else if (i_P9CU == PU_MI_CHIPUNIT)      o_chipUnitType = "mi";
  else if (i_P9CU == PU_DMI_CHIPUNIT)     o_chipUnitType = "dmi";
  else if (i_P9CU == PU_MCS_CHIPUNIT)     o_chipUnitType = "mcs";
  else if (i_P9CU == PU_MCA_CHIPUNIT)     o_chipUnitType = "mca";
  else if (i_P9CU == PU_MCBIST_CHIPUNIT)  o_chipUnitType = "mcbist";
  else if (i_P9CU == PU_PERV_CHIPUNIT)    o_chipUnitType = "perv";
  else if (i_P9CU == PU_PPE_CHIPUNIT)     o_chipUnitType = "ppe";
  else if (i_P9CU == PU_SBE_CHIPUNIT)     o_chipUnitType = "sbe";
  else if (i_P9CU == PU_CAPP_CHIPUNIT)    o_chipUnitType = "capp";
  else if (i_P9CU == PU_MC_CHIPUNIT)      o_chipUnitType = "mc";
  else {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown chip unit enum:%d\n", i_P9CU);
  }

  return rc;
}

//convert chipunit string to enum, as scominfo does not accept strings
uint32_t p9n_convertCUString_to_enum(std::string cuString, p9ChipUnits_t &o_P9CU) {
  uint32_t rc = ECMD_SUCCESS;
  
  if (cuString == "c")          o_P9CU = PU_C_CHIPUNIT;
  else if (cuString == "eq")    o_P9CU = PU_EQ_CHIPUNIT;
  else if (cuString == "ex")    o_P9CU = PU_EX_CHIPUNIT;
  else if (cuString == "xbus")  o_P9CU = PU_XBUS_CHIPUNIT;
  else if (cuString == "obus")  o_P9CU = PU_OBUS_CHIPUNIT;
  else if (cuString == "nv")    o_P9CU = PU_NV_CHIPUNIT;
  else if (cuString == "pec")   o_P9CU = PU_PEC_CHIPUNIT;
  else if (cuString == "phb")   o_P9CU = PU_PHB_CHIPUNIT;
  else if (cuString == "mi")    o_P9CU = PU_MI_CHIPUNIT;
  else if (cuString == "dmi")   o_P9CU = PU_DMI_CHIPUNIT;
  else if (cuString == "mcs")   o_P9CU = PU_MCS_CHIPUNIT;
  else if (cuString == "mca")   o_P9CU = PU_MCA_CHIPUNIT;
  else if (cuString == "mcbist")  o_P9CU = PU_MCBIST_CHIPUNIT;
  else if (cuString == "perv")  o_P9CU = PU_PERV_CHIPUNIT;
  else if (cuString == "ppe")   o_P9CU = PU_PPE_CHIPUNIT;
  else if (cuString == "sbe")   o_P9CU = PU_SBE_CHIPUNIT;
  else if (cuString == "capp")  o_P9CU = PU_CAPP_CHIPUNIT;
  else if (cuString == "mc")    o_P9CU = PU_MC_CHIPUNIT;
  else {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unknown chip unit:%S\n", cuString.c_str());
  }

  return rc;
}

// Load the device tree and initialise the targets
static int initTargets(void) {
  int fd;
  void *fdt;
  struct stat stat;
  static int done = 0;

  if (!done) {
    done = 1;

    // The user sets their device tree via this variable. If not set, fail
    // Would be nice to also set via --device on the cmdline, but currently
    // there is an order of operations problem.
    // Longer term libpdbg will be able to auto-detect the correct thing to use.
    char * devTree = getenv("EDBG_DTB");
    if (devTree == NULL) {
      fprintf(stderr,"dllLoadDll: EDBG_DTB not set in environment, you must set it\n");
      return ECMD_UNKNOWN_FILE;
    }

    // If set to 'none', skip the rest of what we do to setup the device tree
    // This is assuming we won't be using any functions that use the device tree
    if (!strcmp(devTree, "none")) {
      return ECMD_SUCCESS;
    }

    fd = open(devTree, O_RDONLY);
    if (fd < 0) {
      printf("Unable to open device tree: %s\n", devTree);
      return ECMD_FAILURE;
    }

    if (fstat(fd, &stat) < 0) {
      perror("Unable to read device tree size");
      return ECMD_FAILURE;
    }

    fdt = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fdt == MAP_FAILED) {
      perror("Unable to mmap device tree");
      return ECMD_FAILURE;
    }

    pdbg_targets_init(fdt);

    // TODO: We should do this once we know what targets we want to
    // probe/configure. That way we can enable just the ones we care about
    // which is quicker than probing everything all the time.
    pdbg_target_probe_all(NULL);
  }

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
  uint32_t line;
};


// A function recursive that dumps the config as stored
// The printing is not pretty or formatted - used for debug during development
uint32_t printConfig(configEntry_t &configEntry) {
  uint32_t rc;

  // The only terminal type
  if (configEntry.type == CONFIG_VALUE) {
    //printf("In value\n");
    printf(" %s\n", configEntry.value.c_str());
    return 0;
  }

  // These other two will lead to further recursion
  if (configEntry.type == CONFIG_MAP) {
    //printf("In map\n");
    std::map<std::string, configEntry_t>::iterator iter;
    for (iter = configEntry.map.begin(); iter != configEntry.map.end(); iter++) {
      printf("%s:", iter->first.c_str());
      rc = printConfig(iter->second);
      if (rc) return rc;
    }
  }

  if (configEntry.type == CONFIG_LIST) {
    //printf("In list\n");
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

  ecmdChipTarget nodeTarget, chipTarget;

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
        //printf("No event!\n");
        out.warning(FUNCNAME, "A yaml no event hit during parsing!");
        break;

      // Stream start/end are the first/last things in the parse
      case YAML_STREAM_START_EVENT:
        //printf("STREAM START\n");
        // Set our working pointer to the start of our currently emtpy config
        cur = &config;
        break;

      case YAML_STREAM_END_EVENT:
        //printf("STREAM END\n");
        // Nothing to do for cleanup at the end... yet
        break;

      // Nothing to do for document start/end
      case YAML_DOCUMENT_START_EVENT:
        //printf("<b>Start Document</b>\n");
        break;
      case YAML_DOCUMENT_END_EVENT:
        //printf("<b>End Document</b>\n");
        break;

      // Handle sequence start/end events
      case YAML_SEQUENCE_START_EVENT:
        //printf("<b>Start Sequence</b>\n");
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
        //printf("<b>End Sequence</b>\n");
        // When we are done, move ourselves back up the tree
        cur = cur->parent;
        break;

      // Handle map start/end events
      case YAML_MAPPING_START_EVENT:
        // It would be nice if the enum from yaml (map) matched how we have to store this (list)
        // But it doesn't
        //printf("<b>Start Map</b>\n");
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
        //printf("<b>End Map</b>\n");
        // When we are done, move ourselves back up the tree
        cur = cur->parent;
        break;

      // Data
      case YAML_ALIAS_EVENT:
        //printf("Got alias (anchor %s)\n", event.data.alias.anchor);
        // This does nothing for us
        break;
      case YAML_SCALAR_EVENT:
        //printf("Got scalar (value %s)\n", event.data.scalar.value);
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
  //   system_vpd: /some/path/to/an/eeprom
  // - target: k0:n0:s1
  //   system_vpd: /some/path/to/a/second/eeprom
  // We have to loop through the list and verify we have the contents we need in each map
  std::list<configEntry_t>::iterator iter; // To walk the list
  std::map<std::string, configEntry_t>::iterator findIter; // To find the contents in the map
  for (iter = configEntry.list.begin(); iter != configEntry.list.end(); iter++) {
    // In each list entry there will be map of all the required values
    // Ensure everything we need is there and then populate what the code uses
    // These are "target" and "system_vpd"
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

    // system_vpd:
    findIter = iter->map.find("system_vpd");
    if (findIter == iter->map.end()) {
      out.error(FUNCNAME, "The \'system_vpd:\' entry wasn't found in \'vpd:\'\n");
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

uint32_t dllSyncPluginState(ecmdChipTarget & i_target) {
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
uint32_t dllQueryConfig(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail ) {
  return queryConfigExist(i_target, o_queryData, i_detail, false);
}

uint32_t dllQueryExist(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail ) {
  return queryConfigExist(i_target, o_queryData, i_detail, true);
}

uint32_t queryConfigExist(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
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

uint32_t queryConfigExistCages(ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
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

uint32_t queryConfigExistNodes(ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
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

uint32_t queryConfigExistSlots(ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
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

uint32_t queryConfigExistChips(ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipData chipData;
  struct pdbg_target *chipTarget;

  pdbg_for_each_class_target("pib", chipTarget) {

    // Ignore targets wihout an index
    if (pdbg_target_index(chipTarget) < 0)
      continue;

    // If i_allowDisabled isn't true, make sure it's not disabled
    if (!i_allowDisabled) {
      if (pdbg_target_status(chipTarget) == PDBG_TARGET_DISABLED)
	continue;
    }
    
    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((i_target.posState == ECMD_TARGET_FIELD_VALID) && (pdbg_target_index(chipTarget) != i_target.pos))
      continue;

    // We passed our checks, load up our data
    chipData.chipUnitData.clear();
    chipData.chipType = "pu";
    chipData.pos = pdbg_target_index(chipTarget);

    // If the chipUnitType states are set, see what chipUnitTypes are in this chipType
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID
        || i_target.chipUnitTypeState == ECMD_TARGET_FIELD_WILDCARD) {
      // Look for chipunits
      rc = queryConfigExistChipUnits(i_target, chipTarget, chipData.chipUnitData, i_detail, i_allowDisabled);
      if (rc) return rc;
    }

    // Save what we got from recursing down, or just being happy at this level
    o_chipData.push_front(chipData);
  }

  return rc;
}

uint32_t queryConfigExistChipUnits(ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipUnitData chipUnitData;
  struct pdbg_target *target;

  pdbg_for_each_child_target(i_pTarget, target) {
    char *p;

    p = (char *) pdbg_get_target_property(target, "ecmd,chip-unit-type", NULL);
    if (!p || pdbg_target_index(target) < 0)
      /* Skip targets with no ecmd equivalent */
      continue;

    // If i_allowDisabled isn't true, make sure it's not disabled
    if (!i_allowDisabled)
      if (pdbg_target_status(target) == PDBG_TARGET_DISABLED)
	continue;

    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID) &&
        (pdbg_target_index(target) != i_target.chipUnitNum))
      continue;

    if ((i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) &&
        (p != i_target.chipUnitType))
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

  return rc;
}

uint32_t queryConfigExistThreads(ecmdChipTarget & i_target, struct pdbg_target * i_pTarget, std::list<ecmdThreadData> & o_threadData, ecmdQueryDetail_t i_detail, bool i_allowDisabled) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdThreadData threadData;
  struct pdbg_target *target;

  pdbg_for_each_child_target(i_pTarget, target) {
    // Use the index as the threadId and then store it
    threadData.threadId = pdbg_target_index(target);

    o_threadData.push_back(threadData);
  }

  return rc;
}


uint32_t dllGetConfiguration(ecmdChipTarget & i_target, std::string i_name, ecmdConfigValid_t & o_validOutput, std::string & o_valueAlpha, uint32_t & o_valueNumeric) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetConfigurationComplex(ecmdChipTarget & i_target, std::string i_name, ecmdConfigData & o_configData) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSetConfiguration(ecmdChipTarget & i_target, std::string i_name, ecmdConfigValid_t i_validInput, std::string i_valueAlpha, uint32_t i_valueNumeric) {
	return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllSetConfigurationComplex(ecmdChipTarget & i_target, std::string i_name, ecmdConfigData i_configData) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllDeconfigureTarget(ecmdChipTarget & i_target) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllConfigureTarget(ecmdChipTarget & i_target) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryConnectedTargets(ecmdChipTarget & i_target, const char * i_connectionType, std::list<ecmdConnectionData> & o_connections) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllRelatedTargets(const ecmdChipTarget & i_target, const std::string i_relatedType, std::list<ecmdChipTarget> & o_relatedTargets, const ecmdLoopMode_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

/* ######################################################################################### */
/* Info Query Functions - Info Query Functions - Info Query Functions - Info Query Functions */
/* ######################################################################################### */
uint32_t dllQueryFileLocation(ecmdChipTarget & i_target, ecmdFileType_t i_fileType, std::list<std::pair<std::string,  std::string> > & o_fileLocations, std::string & io_version) {
  uint32_t rc = ECMD_SUCCESS;

  switch (i_fileType) {
    case ECMD_FILE_HELPTEXT:
      o_fileLocations.push_back(make_pair(gEDBG_HOME + "/help/", ""));
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
uint32_t dllEnableRingCache(ecmdChipTarget & i_target) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

uint32_t dllDisableRingCache(ecmdChipTarget & i_target) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

uint32_t dllFlushRingCache(ecmdChipTarget & i_target) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

bool dllIsRingCacheEnabled(ecmdChipTarget & i_target) { return false; }

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

uint32_t dllGetChipData(ecmdChipTarget & i_target, ecmdChipData & o_data) {
  return out.error(ECMD_FUNCTION_NOT_SUPPORTED, FUNCNAME, "Function not supported!\n");
}

uint32_t dllChipCleanup(ecmdChipTarget & i_target, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryMode(ecmdChipTarget & i_target, std::string & o_coreMode, std::string & o_coreChipUnit) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetProcessingUnit(ecmdChipTarget & i_target, std::string & o_processingUnitName) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

std::string dllLastError() {
  return "NOT_SUPPORTED";
}

/* ################################################################# */
/* Scom Functions - Scom Functions - Scom Functions - Scom Functions */
/* ################################################################# */
// i_address is a partially translated address - it will contain a
// chiplet base address but it's up to us to add in the chiplet number
// and the rest of the offset. libpdbg expects either a fully translated
// address or a non-translated address, so we need to remove the partial
// translation so we can pass the non-translated address.
static uint64_t getRawScomAddress(ecmdChipTarget & i_target, uint64_t i_address) {
  //struct pdbg_target *chipUnitTarget;
  //
  //if (!findChipUnitType(i_target, i_address, &chipUnitTarget))
  //  i_address -= dt_get_address(chipUnitTarget->dn, 0, NULL);

  uint64_t o_address = i_address;

  // Only call the conversion function if the target is for a chipunit
  if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_VALID) {
    p9ChipUnits_t l_P9CU = P9N_CHIP; //default is the chip
    p9n_convertCUString_to_enum(i_target.chipUnitType, l_P9CU);

    o_address = p9_scominfo_createChipUnitScomAddr(l_P9CU, i_target.chipUnitNum, i_address);
  }
  
  return o_address;
}

uint32_t dllCreateChipUnitScomAddress(ecmdChipTarget & i_target, uint64_t i_address, uint64_t & o_address) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryScom(ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdScomData sdReturn;

  // Wipe out the data structure provided by the user
  o_queryData.clear();

  sdReturn.address = i_address;
  sdReturn.length = 64;
  sdReturn.isChipUnitRelated = false;
  sdReturn.endianMode = ECMD_BIG_ENDIAN;

  //// Need to work out the related chip unit. This amounts to getting the
  //// chiplet id from i_address and wokring out what name to associate with
  //// it.
  //if (!findChipUnitType(i_target, i_address, &chipUnitTarget)) {
  //  p = dt_find_property(chipUnitTarget->dn, "ecmd,chip-unit-type");
  //  assert(p);
  //  sdReturn.isChipUnitRelated = true;
  //  sdReturn.relatedChipUnit.push_back(p->prop);
  //}

  // per ben
  // l_mode 0x00000000 p9n dd10
  // l_mode 0x00000001 PPE_MODE
  // l_mode 0x00000002 p9n dd20+
  // l_mode 0x00000004 p9c dd10
  // l_mode 0x00000008 p9c dd20+
  uint32_t l_mode = 0x2; // Force it to p9n dd20+ for now

  std::vector<p9_chipUnitPairing_t> l_chipUnitPairing;
  rc = p9_scominfo_isChipUnitScom(i_address, sdReturn.isChipUnitRelated, l_chipUnitPairing, l_mode);
  if (rc) {
    return out.error(rc, FUNCNAME,"Invalid scom addr via scom address lookup via p9_scominfo_isChipUnitScom failed\n");
  }

  //for P9n and all other Pegasus Generation of chips we only have 1 chipUnit per scom addr, the list is for the P9 Generation expansion
  if (sdReturn.isChipUnitRelated) {
    std::vector<p9_chipUnitPairing_t>::iterator cuPairingIter = l_chipUnitPairing.begin();
      
    while(cuPairingIter != l_chipUnitPairing.end()) {
      std::string l_chipUnitType;
      rc = p9n_convertCUEnum_to_String(cuPairingIter->chipUnitType, l_chipUnitType);
      if (rc) return rc;
      sdReturn.isChipUnitRelated = true;
      sdReturn.relatedChipUnit.push_back(l_chipUnitType);
      cuPairingIter++;    
    }
  }

  o_queryData.push_back(sdReturn);

  return rc;
}

uint32_t dllGetScom(ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint64_t data;
  struct pdbg_target *target;

  // Convert the input address to an absolute chip level address
  i_address = getRawScomAddress(i_target, i_address);

  // Now for the call to pdbg, use just the chip level target so the address
  // doesn't get translated again down in pdbg
  // i_target is pass by reference, make a local copy before we modify so we don't break upstream
  ecmdChipTarget l_target = i_target;
  l_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
  
  // Get the chip level pdbg target for the call to the pib read
  if (fetchPdbgTarget(l_target, &target)) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unable to find PIB target\n");
  }

  // Do the read and store the data in the return buffer
  rc = pib_read(target, i_address, &data);
  if (rc) {
    return out.error(EDBG_READ_ERROR, FUNCNAME, "pib_read of 0x%" PRIx64 " failed!\n", i_address);
  }
  o_data.setBitLength(64);
  o_data.setDoubleWord(0, data);

  return rc;
}

uint32_t dllPutScom(ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target *target;

  // Convert the input address to an absolute chip level address
  i_address = getRawScomAddress(i_target, i_address);

  // Now for the call to pdbg, use just the chip level target so the address
  // doesn't get translated again down in pdbg
  // i_target is pass by reference, make a local copy before we modify so we don't break upstream
  ecmdChipTarget l_target = i_target;
  l_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;

  // Get the chip level pdbg target for the call to the pib write
  if (fetchPdbgTarget(l_target, &target)) {
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unable to find PIB target\n");
  }

  // Write the data to the chip
  rc = pib_write(target, i_address, i_data.getDoubleWord(0));
  if (rc) {
    return out.error(EDBG_WRITE_ERROR, FUNCNAME, "pib_write of 0x%" PRIx64 " failed!\n", i_address);
  }

  return rc;
}

uint32_t dllPutScomUnderMask(ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & i_data, ecmdDataBuffer & i_mask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllDoScomMultiple(ecmdChipTarget & i_target, std::list<ecmdScomEntry> & io_entries) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

/* ################################################################# */
/* cfam Functions - cfam Functions - cfam Functions - cfam Functions */
/* ################################################################# */
uint32_t dllGetCfamRegister(ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint32_t data;
  struct pdbg_target * pdbgTarget;

  rc = fetchCfamTarget(i_target, &pdbgTarget);
  if (rc) return rc;

  rc = fsi_read(pdbgTarget, i_address, &data);
  o_data.setBitLength(32);
  o_data.setWord(0, data);

  return rc;
}

uint32_t dllPutCfamRegister(ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  struct pdbg_target * pdbgTarget;

  rc = fetchCfamTarget(i_target, &pdbgTarget);
  if (rc) return rc;

  rc = fsi_write(pdbgTarget, i_address, i_data.getWord(0));

  return rc;
}

uint32_t dllGetEcid(const ecmdChipTarget & i_target, ecmdDataBuffer & o_ecidData) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetEcidVerbose(const ecmdChipTarget & i_target, ecmdDataBuffer & o_ecidData, std::vector<std::string> & o_additionalInfo) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetGpRegister(ecmdChipTarget & i_target, uint32_t i_gpRegister, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutGpRegister(ecmdChipTarget & i_target, uint32_t i_gpRegister, ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutGpRegisterUnderMask(ecmdChipTarget & i_target, uint32_t i_gpRegister, ecmdDataBuffer & i_data, ecmdDataBuffer & i_mask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

/* ############################################################# */
/* VPD Functions - VPD Functions - VPD Functions - VPD Functions */
/* ############################################################# */
uint32_t dllGetModuleVpdKeyword(ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllPutModuleVpdKeyword(ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllGetModuleVpdImage(ecmdChipTarget & i_target, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllPutModuleVpdImage(ecmdChipTarget & i_target, ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllGetModuleVpdKeywordFromImage(ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & i_image_data, ecmdDataBuffer & o_keyword_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllPutModuleVpdKeywordToImage(ecmdChipTarget & i_target, const char * i_record_name, const char * i_keyword, ecmdDataBuffer & io_image_data, ecmdDataBuffer & i_keyword_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllGetFruVpdImage(ecmdChipTarget & i_target, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
} 

uint32_t dllPutFruVpdImage(ecmdChipTarget & i_target, ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetFruVpdKeywordWithRid(uint32_t i_rid, const char * i_record_name, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutFruVpdKeywordWithRid(uint32_t i_rid, const char * i_record_name, const char * i_keyword, ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetFruVpdKeyword(ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & o_data) {
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

uint32_t dllPutFruVpdKeyword(ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, ecmdDataBuffer & i_data) {
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

uint32_t dllGetFruVpdKeywordFromImage(ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, uint32_t i_bytes, ecmdDataBuffer & i_image_data, ecmdDataBuffer & o_data) {
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

uint32_t dllPutFruVpdKeywordToImage(ecmdChipTarget & i_target, const char * i_recordName, const char * i_keyword, ecmdDataBuffer & io_image_data, ecmdDataBuffer & i_data) {
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

/* ################################################################# */
/* Ring Functions - Ring Functions - Ring Functions - Ring Functions */
/* ################################################################# */
uint32_t dllQueryRing(ecmdChipTarget & i_target, std::list<ecmdRingData> & o_queryData, const char * i_ringName, ecmdQueryDetail_t i_detail) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRing(ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutRing(ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & i_data, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRingWithModifier(ecmdChipTarget & i_target, uint32_t i_address, uint32_t i_bitLength, ecmdDataBuffer & o_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutRingWithModifier(ecmdChipTarget & i_target, uint32_t i_address, uint32_t i_bitLength, ecmdDataBuffer & i_data) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRingSparse(ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, ecmdDataBuffer & i_mask, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllPutRingSparse(ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & i_data, ecmdDataBuffer & i_mask, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryRingIgnoreMask(ecmdChipTarget & i_target, const std::string i_ringName, ecmdDataBuffer & o_ignoreMask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryRingInversionMask(ecmdChipTarget & i_target, const std::string i_ringName, ecmdDataBuffer & o_inversionMask) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllGetRingSparseWithTraceMask(ecmdChipTarget & i_target, const char * i_ringName, ecmdDataBuffer & o_data, ecmdDataBuffer & i_mask, ecmdDataBuffer & i_traceMask, uint32_t i_mode) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}
