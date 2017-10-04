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

// Headers from eCMD
#include <ecmdDllCapi.H>
#include <ecmdStructs.H>
#include <ecmdReturnCodes.H>
#include <ecmdDataBuffer.H>
#include <ecmdSharedUtils.H>

// Headers from pdbg
extern "C" {
#include <target.h>
#include <operations.h>
}

// Headers from ecmd-pdbg
#include <edbgCommon.H>
#include <edbgOutput.H>

// TODO: This needs to not be hardcoded and set from the command-line. Longer
// term libpdbg will be able to auto-detect the correct thing to use.
std::string DEVICE_TREE_FILE;

// For use by dllQueryConfig and dllQueryExist
uint32_t queryConfigExist(ecmdChipTarget & i_target, ecmdQueryData & o_queryData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistCages(ecmdChipTarget & i_target, std::list<ecmdCageData> & o_cageData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistNodes(ecmdChipTarget & i_target, std::list<ecmdNodeData> & o_nodeData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistSlots(ecmdChipTarget & i_target, std::list<ecmdSlotData> & o_slotData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChips(ecmdChipTarget & i_target, std::list<ecmdChipData> & o_chipData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);
uint32_t queryConfigExistChipUnits(ecmdChipTarget & i_target, struct target * i_pTarget, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled);

// Used to translate an ecmdChipTarget to a pdbg target
uint32_t fetchPdbgTarget(ecmdChipTarget & i_target, struct target * o_pdbgTarget);

std::string gECMD_HOME;
std::string gEDBG_HOME;

/* ################################################################################################# */
/* Static functions used to lookup pdbg targets 						     */
/* ################################################################################################# */
static uint32_t fetchPdbgInterfaceTarget(ecmdChipTarget & i_target, struct target **o_target, const char *interface) {
  struct target *target;

  assert(i_target.cageState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.cage == 0 &&                                  \
         i_target.nodeState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.node == 0 &&                                  \
         i_target.slotState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.slot == 0 &&                                  \
         i_target.posState == ECMD_TARGET_FIELD_VALID);

  *o_target = NULL;
  for_each_class_target(interface, target) {
    if (target->index == i_target.pos) {
      *o_target = target;
      return 0;
    }
  }

  return -1;
}

/* Given a target in i_target this will return the associcated pdbg pib target
 * that can be passed to pib_read/write() such that they will not perform any
 * address translations - ie. "raw" scom access as cronus calls it. */
static uint32_t fetchPibTarget(ecmdChipTarget & i_target, struct target **o_pibTarget) {
  if (fetchPdbgInterfaceTarget(i_target, o_pibTarget, "pib")) {
    printf("Unable to find pib target in p%d\n", i_target.pos);
    return -1;
  }

  return 0;
}

static uint32_t fetchCfamTarget(ecmdChipTarget & i_target, struct target **o_pibTarget) {
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
static uint32_t fetchPdbgTarget(ecmdChipTarget & i_target, struct target ** o_pdbgTarget) {
  uint32_t rc = ECMD_SUCCESS;
  struct target *chipTarget;
  uint32_t index;
  struct dt_node *dn;

  assert(i_target.cageState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.cage == 0 &&                                  \
         i_target.nodeState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.node == 0 &&                                  \
         i_target.slotState == ECMD_TARGET_FIELD_VALID &&       \
         i_target.slot == 0 &&                                  \
         i_target.posState == ECMD_TARGET_FIELD_VALID);

  *o_pdbgTarget = NULL;
  for_each_class_target("pib", chipTarget) {
    const struct dt_property *p;

    // Don't search for targets not matched to our index/position
    index = chipTarget->index;
    if (i_target.pos != index)
      continue;

    // Just return the raw pib target if we're not looking for a specific chip unit
    if (i_target.chipUnitTypeState == ECMD_TARGET_FIELD_UNUSED) {
      *o_pdbgTarget = chipTarget;
    } else {
      // Search child nodes of this position to find what we are
      // looking for
      dt_for_each_node(chipTarget->dn, dn) {
        p = dt_find_property(dn, "ecmd,chip-unit-type");
        if (p &&
            p->prop == i_target.chipUnitType &&
            dn->target->index == i_target.chipUnitNum)
          *o_pdbgTarget = dn->target;
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
static uint32_t findChipUnitType(ecmdChipTarget &i_target, uint64_t i_address, struct target **pdbgTarget)
{
  struct target *pibTarget;
  struct dt_node *dn;

  if (fetchPibTarget(i_target, &pibTarget)) {
    printf("Unable to find PIB target\n");
    return -1;
  }

  dt_for_each_node(pibTarget->dn, dn) {
    uint64_t addr, size;
    struct dt_property *p;

    addr = dt_get_address(dn, 0, &size);
    if (i_address >= addr && i_address < addr+size) {
      p = dt_find_property(dn, "ecmd,chip-unit-type");
      if (p && dn->target) {
        // Found our base target
        *pdbgTarget = dn->target;
        return 0;
      }
    }
  }

  return -1;
}

/* Given a partially translated scom address in i_address turn it into an
 * un-translated address. */
static uint64_t getRawScomAddress(ecmdChipTarget & i_target, uint64_t i_address) {
  struct target *chipUnitTarget;

  if (!findChipUnitType(i_target, i_address, &chipUnitTarget))
    i_address -= dt_get_address(chipUnitTarget->dn, 0, NULL);

  return i_address;
}

uint32_t dllInitDll() {
  uint32_t rc = ECMD_SUCCESS;

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

  int fd;
  void *fdt;
  struct stat stat;

  fd = open(DEVICE_TREE_FILE.c_str(), O_RDONLY);
  if (fd < 0) {
    perror("Unable to open device tree");
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

  targets_init(fdt);

  // TODO: We should do this once we know what targets we want to
  // probe/configure. That way we can enable just the ones we care about
  // which is quicker than probing everything all the time.
  target_probe();

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
  struct target *chipTarget;
  uint32_t index;

  for_each_class_target("pib", chipTarget) {

    // Get the index of the target returned and do some checking on it
    index = chipTarget->index;

    // Ignore targets wihout an index
    if (index < 0)
      continue;

    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((i_target.posState == ECMD_TARGET_FIELD_VALID) && (index != i_target.pos))
      continue;

    // We passed our checks, load up our data
    chipData.chipUnitData.clear();
    chipData.chipType = "pu";
    chipData.pos = index;

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

uint32_t queryConfigExistChipUnits(ecmdChipTarget & i_target, struct target * i_chipTarget, std::list<ecmdChipUnitData> & o_chipUnitData, ecmdQueryDetail_t i_detail, bool i_allowDisabled)  {
  uint32_t rc = ECMD_SUCCESS;
  ecmdChipUnitData chipUnitData;
  //struct target *chipUnitTarget;
  struct dt_node *dn;
  uint32_t index;

  dt_for_each_child(i_chipTarget->dn, dn) {
    struct dt_property *p;
    struct target *target = dn->target;

    index = target->index;
    p = dt_find_property(dn, "ecmd,chip-unit-type");
    if (!p || index < 0)
      /* Skip targets with no ecmd equivalent */
      continue;

    // If posState is set to VALID, check that our values match
    // If posState is set to WILDCARD, we don't care
    if ((i_target.chipUnitNumState == ECMD_TARGET_FIELD_VALID) &&
        (index != i_target.chipUnitNum))
      continue;

    chipUnitData.chipUnitType = p->prop;
    chipUnitData.chipUnitNum = index;

    /* TODO: Handle chips with threads */
    chipUnitData.numThreads = 0;
    chipUnitData.threadData.clear();

    o_chipUnitData.push_front(chipUnitData);
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

/* ######################################################################################### */
/* Info Query Functions - Info Query Functions - Info Query Functions - Info Query Functions */
/* ######################################################################################### */
uint32_t dllQueryFileLocation(ecmdChipTarget & i_target, ecmdFileType_t i_fileType, std::string & o_fileLocation, std::string & io_version) {
  uint32_t rc = ECMD_SUCCESS;

  switch (i_fileType) {
    case ECMD_FILE_HELPTEXT:
      o_fileLocation = gECMD_HOME + "/help/";
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

/* ################################################################# */
/* Scom Functions - Scom Functions - Scom Functions - Scom Functions */
/* ################################################################# */
uint32_t dllCreateChipUnitScomAddress(ecmdChipTarget & i_target, uint64_t i_address, uint64_t & o_address) {
  return ECMD_FUNCTION_NOT_SUPPORTED;
}

uint32_t dllQueryScom(ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail) {
  uint32_t rc = ECMD_SUCCESS;

  // This function goes away with eCMD 15.0 when the hidden function becomes the main function
  // For now, call the hidden function which does the work and map the data back into our
  // return structures
  ecmdScomData sdReturn;
  std::list<ecmdScomDataHidden> sdhList;

  // Wipe out the data structure provided by the user
  o_queryData.clear();

  rc = dllQueryScomHidden(i_target, sdhList, i_address, i_detail);
  if (rc) return rc;

  // Load out the data from the hidden query into this return structure
  std::list<ecmdScomDataHidden>::iterator sdhIter;

  for (sdhIter = sdhList.begin(); sdhIter != sdhList.end(); sdhIter++) {
    sdReturn.address = sdhIter->address;
    sdReturn.length = sdhIter->length;
    sdReturn.isChipUnitRelated = sdhIter->isChipUnitRelated;
    sdReturn.relatedChipUnit = sdhIter->relatedChipUnit.front();
    sdReturn.relatedChipUnitShort = sdhIter->relatedChipUnitShort.front();
    sdReturn.endianMode = sdhIter->endianMode;
    sdReturn.clockDomain = sdhIter->clockDomain;
    sdReturn.clockState = sdhIter->clockState;

    o_queryData.push_back(sdReturn);
  }

  return rc;
}

uint32_t dllQueryScomHidden(ecmdChipTarget & i_target, std::list<ecmdScomDataHidden> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail) {
  uint32_t rc = ECMD_SUCCESS;
  ecmdScomDataHidden sdReturn;
  struct target *chipUnitTarget;
  struct dt_property *p;

  // Wipe out the data structure provided by the user
  o_queryData.clear();

  sdReturn.address = i_address;
  sdReturn.length = 64;
  sdReturn.isChipUnitRelated = false;
  sdReturn.endianMode = ECMD_BIG_ENDIAN;

  // Need to work out the related chip unit. This amounts to getting the
  // chiplet id from i_address and wokring out what name to associate with
  // it.
  if (!findChipUnitType(i_target, i_address, &chipUnitTarget)) {
    p = dt_find_property(chipUnitTarget->dn, "ecmd,chip-unit-type");
    assert(p);
    sdReturn.isChipUnitRelated = true;
    sdReturn.relatedChipUnit.push_back(p->prop);
  }

  o_queryData.push_back(sdReturn);

  return rc;
}

uint32_t dllGetScom(ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint64_t data;
  struct target *target;

  // i_address is a partially translated address - it will contain a
  // chiplet base address but it's up to use to add in the chiplet number
  // and the rest of the offset. libpdbg expects either a fully translated
  // address or a non-translated address, so we need to remove the partial
  // translation so we can pass the non-translated address.
  i_address = getRawScomAddress(i_target, i_address);

  // Get the actual pdbg target so libpdbg can do the correct translation.
  if (fetchPdbgTarget(i_target, &target)) {
    printf("Unable to find PIB target\n");
    return -1;
  }

  rc = pib_read(target, i_address, &data);
  o_data.setBitLength(64);
  o_data.setWord(0, data >> 32);
  o_data.setWord(1, data & 0xffffffff);

  return rc;
}

uint32_t dllPutScom(ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  uint64_t data;
  struct target *target;

  i_address = getRawScomAddress(i_target, i_address);

  // Get the actual pdbg target so libpdbg can do the correct translation.
  if (fetchPdbgTarget(i_target, &target)) {
    printf("Unable to find PIB target\n");
    return -1;
  }

  /* TODO: Did we get this right?? I don't think so... */
  data = (uint64_t) i_data.getWord(0) << 32;
  data |= i_data.getWord(1);
  rc = pib_write(target, i_address, data);

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
  struct target * pdbgTarget;

  rc = fetchCfamTarget(i_target, &pdbgTarget);
  if (rc) return rc;

  rc = fsi_read(pdbgTarget, i_address, &data);
  o_data.setBitLength(32);
  o_data.setWord(0, data);

  return rc;
}

uint32_t dllPutCfamRegister(ecmdChipTarget & i_target, uint32_t i_address, ecmdDataBuffer & i_data) {
  uint32_t rc = ECMD_SUCCESS;
  struct target * pdbgTarget;

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

std::string dllLastError() {
  return "NOT_SUPPORTED";
}
