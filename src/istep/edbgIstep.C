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
#include <edbgIstep.H>

//Updated this table as per P10_IPL_Flow_v0.89.17.
//HB isteps need to revisit
const edbgIPLTable::edbgIStep_t edbgIPLTable::cv_edbgIStepTable[] =
{
    /****************************************************************************/
    /* !!! --- THIS LIST MUST BE IN ORDER THAT THEY'RE CALLED IN AN IPL --- !!! */
    /****************************************************************************/

    /****************************************************************************/
    /* NOTE: The istep is executed by the self boot engine/the host code/       */
    /*       the attached bmc depending upon the destination type.              */
    /*       Options:                                                           */
    /*       EDBG_ISTEP_HOST - The istep is performed by the host               */
    /*       EDBG_ISTEP_SBE  - The istep is performed by the self boot engine   */
    /*       EDBG_ISTEP_BMC  - The istep is performed by the BMC                */
    /*       EDBG_ISTEP_NOOP - The istep is No OP                               */
    /*                                                                          */
    /****************************************************************************/

    /****************************************************************************/
    /* Warning : The following constants are defined based on the values of this*/
    /*           table. Any changes to this table requires examination of the   */
    /*           values assigned to these constants.                            */
    /*                                                                          */
    /*       EDBG_FIRST_ISTEP_NUM   = 0                                         */
    /*       EDBG_LAST_ISTEP_NUM    = 21                                        */
    /*       EDBG_INVALID_ISTEP_NUM = 0xFFFF                                    */
    /*       EDBG_INVALID_POSITION  = 0xFFFF                                    */
    /****************************************************************************/

//major | minor |                          istep name   |                     destination   |
//number| number|                                       |                                   |
  { 0,   1,                                   "poweron",                   EDBG_ISTEP_NOOP},
  { 0,   2,                                  "startipl",                   EDBG_ISTEP_NOOP},
  { 0,   3,                              "disableattns",                   EDBG_ISTEP_NOOP},
  { 0,   4,                             "updatehwmodel",                   EDBG_ISTEP_NOOP},
  { 0,   5,                           "alignment_check",                   EDBG_ISTEP_NOOP},
  { 0,   6,                             "set_ref_clock",                   EDBG_ISTEP_NOOP},
  { 0,   7,                           "proc_clock_test",                   EDBG_ISTEP_NOOP},
  { 0,   8,                             "proc_prep_ipl",                   EDBG_ISTEP_NOOP},
  { 0,   9,                               "edramrepair",                   EDBG_ISTEP_NOOP},
  { 0,  10,                          "asset_protection",                   EDBG_ISTEP_NOOP},
  { 0,  11,                   "proc_select_boot_master",                   EDBG_ISTEP_NOOP},
  { 0,  12,                          "hb_config_update",                   EDBG_ISTEP_NOOP},
  { 0,  13,                         "sbe_config_update",                   EDBG_ISTEP_NOOP},
  { 0,  14,                                 "sbe_start",                   EDBG_ISTEP_BMC},
  { 0,  15,                                  "startPRD",                   EDBG_ISTEP_NOOP},
  { 0,  16,                          "proc_attn_listen",                   EDBG_ISTEP_NOOP},
  { 1,   1,                   "proc_sbe_enable_seeprom",                   EDBG_ISTEP_NOOP},
  { 1,   2,                         "proc_sbe_pib_init",                   EDBG_ISTEP_NOOP},
  { 1,   3,                          "proc_sbe_measure",                   EDBG_ISTEP_NOOP},
  { 2,   1,                         "proc_sbe_ld_image",                   EDBG_ISTEP_NOOP},
  { 2,   2,                       "proc_sbe_attr_setup",                   EDBG_ISTEP_SBE},
  { 2,   3,                 "proc_sbe_tp_chiplet_reset",                   EDBG_ISTEP_SBE},
  { 2,   4,               "proc_sbe_tp_gptr_time_initf",                   EDBG_ISTEP_SBE},
  { 2,   5,                "proc_sbe_dft_probe_setup_1",                   EDBG_ISTEP_SBE},
  { 2,   6,                       "proc_sbe_npll_initf",                   EDBG_ISTEP_SBE},
  { 2,   7,                        "proc_sbe_rcs_setup",                   EDBG_ISTEP_SBE},
  { 2,   8,                  "proc_sbe_tp_switch_gears",                   EDBG_ISTEP_SBE},
  { 2,   9,                       "proc_sbe_npll_setup",                   EDBG_ISTEP_SBE},
  { 2,  10,                     "proc_sbe_tp_repr_intf",                   EDBG_ISTEP_SBE},
  { 2,  11,                   "proc_sbe_setup_tp_abist",                   EDBG_ISTEP_SBE},
  { 2,  12,                     "proc_sbe_tp_arrayinit",                   EDBG_ISTEP_SBE},
  { 2,  13,                          "proc_sbe_tp_intf",                   EDBG_ISTEP_SBE},
  { 2,  14,                 "proc_sbe_dft_probesetup_2",                   EDBG_ISTEP_SBE},
  { 2,  15,                  "proc_sbe_tp_chiplet_init",                   EDBG_ISTEP_SBE},
  { 3,   1,                    "proc_sbe_chiplet_setup",                   EDBG_ISTEP_SBE},
  { 3,   2,               "proc_sbe_chiplet_clk_config",                   EDBG_ISTEP_SBE},
  { 3,   3,                    "proc_sbe_chiplet_reset",                   EDBG_ISTEP_SBE},
  { 3,   4,                  "proc_sbe_gptr_time_initf",                   EDBG_ISTEP_SBE},
  { 3,   5,                "proc_sbe_chiplet_pll_initf",                   EDBG_ISTEP_SBE},
  { 3,   6,                "proc_sbe_chiplet_pll_setup",                   EDBG_ISTEP_SBE},
  { 3,   7,                        "proc_sbe_repr_intf",                   EDBG_ISTEP_SBE},
  { 3,   8,                      "proc_sbe_abist_setup",                   EDBG_ISTEP_SBE},
  { 3,   9,                        "proc_sbe_arrayinit",                   EDBG_ISTEP_SBE},
  { 3,  10,                            "proc_sbe_lbist",                   EDBG_ISTEP_SBE},
  { 3,  11,                            "proc_sbe_initf",                   EDBG_ISTEP_SBE},
  { 3,  12,                      "proc_sbe_startclocks",                   EDBG_ISTEP_SBE},
  { 3,  13,                     "proc_sbe_chiplet_init",                   EDBG_ISTEP_SBE},
  { 3,  14,                 "proc_sbe_chiplet_fir_init",                   EDBG_ISTEP_SBE},
  { 3,  15,                         "proc_sbe_dts_init",                   EDBG_ISTEP_SBE},
  { 3,  16,                "proc_sbe_skew_adjust_setup",                   EDBG_ISTEP_SBE},
  { 3,  17,                 "proc_sbe_nest_enable_ridi",                   EDBG_ISTEP_SBE},
  { 3,  18,                         "proc_sbe_scominit",                   EDBG_ISTEP_SBE},
  { 3,  19,                              "proc_sbe_lpc",                   EDBG_ISTEP_SBE},
  { 3,  20,                       "proc_sbe_fabricinit",                   EDBG_ISTEP_SBE},
  { 3,  21,                     "proc_sbe_check_master",                   EDBG_ISTEP_SBE},
  { 3,  22,                        "proc_sbe_mcs_setup",                   EDBG_ISTEP_SBE},
  { 3,  23,                        "proc_sbe_select_ex",                   EDBG_ISTEP_SBE},
  { 4,   1,                    "proc_hcd_cache_poweron",                   EDBG_ISTEP_SBE},
  { 4,   2,                      "proc_hcd_cache_reset",                   EDBG_ISTEP_SBE},
  { 4,   3,            "proc_hcd_cache_gptr_time_initf",                   EDBG_ISTEP_SBE},
  { 4,   4,               "proc_hcd_cache_repair_initf",                   EDBG_ISTEP_SBE},
  { 4,   5,                  "proc_hcd_cache_arrayinit",                   EDBG_ISTEP_SBE},
  { 4,   6,                      "proc_hcd_cache_initf",                   EDBG_ISTEP_SBE},
  { 4,   7,                "proc_hcd_cache_startclocks",                   EDBG_ISTEP_SBE},
  { 4,   8,                   "proc_hcd_cache_scominit",                   EDBG_ISTEP_SBE},
  { 4,   9,             "proc_hcd_cache_scom_customize",                   EDBG_ISTEP_SBE},
  { 4,  10,           "proc_hcd_cache_ras_runtime_scom",                   EDBG_ISTEP_SBE},
  { 4,  11,                     "proc_hcd_core_poweron",                   EDBG_ISTEP_SBE},
  { 4,  12,               "proc_hcd_core_chiplet_reset",                   EDBG_ISTEP_SBE},
  { 4,  13,             "proc_hcd_core_gptr_time_initf",                   EDBG_ISTEP_SBE},
  { 4,  14,                "proc_hcd_core_repair_initf",                   EDBG_ISTEP_SBE},
  { 4,  15,                   "proc_hcd_core_arrayinit",                   EDBG_ISTEP_SBE},
  { 4,  16,                       "proc_hcd_core_initf",                   EDBG_ISTEP_SBE},
  { 4,  17,                 "proc_hcd_core_startclocks",                   EDBG_ISTEP_SBE},
  { 4,  18,                    "proc_hcd_core_scominit",                   EDBG_ISTEP_SBE},
  { 4,  19,              "proc_hcd_core_scom_customize",                   EDBG_ISTEP_SBE},
  { 4,  20,            "proc_hcd_core_ras_runtime_scom",                   EDBG_ISTEP_SBE},
  { 5,   1,                  "proc_sbe_load_bootloader",                   EDBG_ISTEP_SBE},
  { 5,   2,                   "proc_sbe_core_spr_setup",                   EDBG_ISTEP_SBE},
  { 5,   3,                   "proc_sbe_instruct_start",                   EDBG_ISTEP_SBE},
  { 6,   1,                           "host_bootloader",                   EDBG_ISTEP_NOOP},
  { 6,   2,                                "host_setup",                   EDBG_ISTEP_NOOP},
  { 6,   3,                         "host_istep_enable",                   EDBG_ISTEP_NOOP},
  { 6,   4,                             "host_init_fsi",                   EDBG_ISTEP_HOST},
  { 6,   5,                        "host_set_ipl_parms",                   EDBG_ISTEP_HOST},
  { 6,   6,                     "host_discover_targets",                   EDBG_ISTEP_HOST},
  { 6,   7,                    "host_update_master_tpm",                   EDBG_ISTEP_HOST},
  { 6,   8,                                 "host_gard",                   EDBG_ISTEP_HOST},
  { 6,   9,                       "host_voltage_config",                   EDBG_ISTEP_HOST},
  { 7,   1,                     "host_mss_attr_cleanup",                   EDBG_ISTEP_HOST},
  { 7,   2,                                  "mss_volt",                   EDBG_ISTEP_HOST},
  { 7,   3,                                  "mss_freq",                   EDBG_ISTEP_HOST},
  { 7,   4,                            "mss_eff_config",                   EDBG_ISTEP_HOST},
  { 7,   5,                           "mss_attr_update",                   EDBG_ISTEP_HOST},
  { 8,   1,                     "host_slave_sbe_config",                   EDBG_ISTEP_HOST},
  { 8,   2,                            "host_setup_sbe",                   EDBG_ISTEP_HOST},
  { 8,   3,                            "host_cbs_start",                   EDBG_ISTEP_HOST},
  { 8,   4,     "proc_check_slave_sbe_seeprom_complete",                   EDBG_ISTEP_HOST},
  { 8,   5,                      "host_attnlisten_proc",                   EDBG_ISTEP_HOST},
  { 8,   6,                       "host_fbc_eff_config",                   EDBG_ISTEP_HOST},
  { 8,   7,                      "hosteff_config_links",                   EDBG_ISTEP_HOST},
  { 8,   8,                          "proc_attr_update",                   EDBG_ISTEP_HOST},
  { 8,   9,              "proc_chiplet_fabric_scominit",                   EDBG_ISTEP_HOST},
  { 8,   10,                        "host_set_voltages",                   EDBG_ISTEP_HOST},
  { 8,   11,                         "proc_io_scominit",                   EDBG_ISTEP_HOST},
  { 8,   12,                          "proc_load_ioppe",                   EDBG_ISTEP_HOST},
  { 8,   13,                          "proc_init_ioppe",                   EDBG_ISTEP_HOST},
  { 8,   14,                    "proc_iohs_enable_ridi",                   EDBG_ISTEP_HOST},
  { 9,   1,                        "proc_io_dccal_done",                   EDBG_ISTEP_HOST},
  { 9,   2,                    "fabric_dl_pre_trainadv",                   EDBG_ISTEP_HOST},
  { 9,   3,                  "fabric_dl_setup_training",                   EDBG_ISTEP_HOST},
  { 9,   4,                    "proc_fabric_link_layer",                   EDBG_ISTEP_HOST},
  { 9,   5,                   "fabric_dl_post_trainadv",                   EDBG_ISTEP_HOST},
  { 9,   6,                       "proc_fabric_iovalid",                   EDBG_ISTEP_HOST},
  { 9,   7,             "proc_fbc_eff_config_aggregate",                   EDBG_ISTEP_HOST},
  {10,   1,                            "proc_build_smp",                   EDBG_ISTEP_HOST},
  {10,   2,                     "host_slave_sbe_update",                   EDBG_ISTEP_HOST},
  {10,   3,                  "host_secureboot_lockdown",                   EDBG_ISTEP_NOOP},
  {10,   4,                     "proc_chiplet_scominit",                   EDBG_ISTEP_HOST},
  {10,   5,                         "proc_pau_scominit",                   EDBG_ISTEP_HOST},
  {10,   6,                        "proc_pcie_scominit",                   EDBG_ISTEP_HOST},
  {10,   7,                "proc_scomoverride_chiplets",                   EDBG_ISTEP_HOST},
  {10,   8,                  "proc_chiplet_enable_ridi",                   EDBG_ISTEP_HOST},
  {10,   9,                             "host_rng_bist",                   EDBG_ISTEP_HOST},
  {11,   1,                       "host_prd_hwreconfig",                   EDBG_ISTEP_HOST},
  {11,   2,                         "host_set_mem_volt",                   EDBG_ISTEP_HOST},
  {11,   3,                          "proc_ocmb_enable",                   EDBG_ISTEP_HOST},
  {11,   4,                      "ocmb_check_for_ready",                   EDBG_ISTEP_HOST},
  {12,   1,                               "mss_getecid",                   EDBG_ISTEP_HOST},
  {12,   2,                           "omi_attr_update",                   EDBG_ISTEP_HOST},
  {12,   3,                        "proc_omi_scom_init",                   EDBG_ISTEP_HOST},
  {12,   4,                         "ocmb_omi_scominit",                   EDBG_ISTEP_HOST},
  {12,   5,                          "omi_pre_trainadv",                   EDBG_ISTEP_HOST},
  {12,   6,                                 "omi_setup",                   EDBG_ISTEP_HOST},
  {12,   7,                       "omi_io_run_training",                   EDBG_ISTEP_HOST},
  {12,   8,                           "omi_train_check",                   EDBG_ISTEP_HOST},
  {12,   9,                         "omi_post_trainadv",                   EDBG_ISTEP_HOST},
  {12,  10,                      "host_attnlisten_memb",                   EDBG_ISTEP_HOST},
  {12,  11,                             "host_omi_init",                   EDBG_ISTEP_HOST},
  {12,  12,                      "update_omi_firmaware",                   EDBG_ISTEP_HOST},
  {13,   1,                              "mss_scominit",                   EDBG_ISTEP_HOST},
  {13,   2,                              "mss_draminit",                   EDBG_ISTEP_HOST},
  {13,   3,                           "mss_draminit_mc",                   EDBG_ISTEP_HOST},
  {14,   1,                               "mss_memdiag",                   EDBG_ISTEP_HOST},
  {14,   2,                          "mss_thermal_init",                   EDBG_ISTEP_HOST},
  {14,   3,                        "proc_load_iop_xram",                   EDBG_ISTEP_HOST},
  {14,   4,                          "proc_pcie_config",                   EDBG_ISTEP_HOST},
  {14,   5,                      "proc_setup_mmio_bars",                   EDBG_ISTEP_HOST},
  {14,   6,                 "proc_exit_cache_contained",                   EDBG_ISTEP_HOST},
  {14,   7,                            "proc_htm_setup",                   EDBG_ISTEP_HOST},
  {14,   8,                        "host_mpipl_service",                   EDBG_ISTEP_HOST},
  {15,   1,                     "host_build_stop_image",                   EDBG_ISTEP_HOST},
  {15,   2,                        "proc_set_homer_bar",                   EDBG_ISTEP_HOST},
  {15,   3,                 "host_establish_ec_chiplet",                   EDBG_ISTEP_HOST},
  {15,   4,                    "host_start_stop_engine",                   EDBG_ISTEP_HOST},
  {16,   1,                      "host_activate_master",                   EDBG_ISTEP_HOST},
  {16,   2,                 "host_activate_slave_cores",                   EDBG_ISTEP_HOST},
  {16,   3,                           "host_secure_rng",                   EDBG_ISTEP_HOST},
  {16,   4,                                 "mss_scrub",                   EDBG_ISTEP_HOST},
  {16,   5,                         "host_ipl_complete",                   EDBG_ISTEP_HOST},
  {17,   1,                           "collect_drawers",                   EDBG_ISTEP_NOOP},
  {18,   1,                 "sys_proc_eff_config_links",                   EDBG_ISTEP_NOOP},
  {18,   2,          "sys_proc_chiplet_fabric_scominit",                   EDBG_ISTEP_NOOP},
  {18,   3,                "sys_fabric_dl_pre_trainadv",                   EDBG_ISTEP_NOOP},
  {18,   4,              "sys_fabric_dl_setup_training",                   EDBG_ISTEP_NOOP},
  {18,   5,                "sys_proc_fabric_link_layer",                   EDBG_ISTEP_NOOP},
  {18,   6,               "sys_fabric_dl_post_trainadv",                   EDBG_ISTEP_NOOP},
  {18,   7,                       "proc_fabric_iovalid",                   EDBG_ISTEP_NOOP},
  {18,   8,             "proc_fbc_eff_config_aggregate",                   EDBG_ISTEP_NOOP},
  {18,   9,                            "proc_tod_setup",                   EDBG_ISTEP_HOST},
  {18,  10,                             "proc_tod_init",                   EDBG_ISTEP_HOST},
  {18,  11,                          "cec_ipl_complete",                   EDBG_ISTEP_NOOP},
  {18,  12,                           "startprd_system",                   EDBG_ISTEP_NOOP},
  {18,  13,                            "attn_listenall",                   EDBG_ISTEP_NOOP},
  {19,   1,                                 "prep_host",                   EDBG_ISTEP_NOOP},
  {20,   1,                         "host_load_payload",                   EDBG_ISTEP_HOST},
  {20,   2,                        "host_load_complete",                   EDBG_ISTEP_HOST},
  {21,   1,                         "host_micro_update",                   EDBG_ISTEP_NOOP},
  {21,   2,                        "host_runtime_setup",                   EDBG_ISTEP_HOST},
  {21,   3,                          "host_verify_hdat",                   EDBG_ISTEP_HOST},
  {21,   4,                        "host_start_payload",                   EDBG_ISTEP_HOST},
  {21,   5,                   "host_post_start_payload",                   EDBG_ISTEP_NOOP},
  {21,   6,                                 "switchbcu",                   EDBG_ISTEP_NOOP},
  {21,   7,                               "completeipl",                   EDBG_ISTEP_NOOP},
}; // end - array initialization

// Calculate the number of isteps in the IPL Table
const uint16_t edbgIPLTable::EDBG_NUMBER_OF_ISTEPS = ( sizeof(cv_edbgIStepTable) /
                                                       sizeof(edbgIPLTable::edbgIStep_t));



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool edbgIPLTable::getIStepNumber(const std::string & i_istepName,
                                  uint16_t & o_majorNum,
                                  uint16_t & o_minorNum)
{

  bool l_status = false;
  o_majorNum  = EDBG_INVALID_ISTEP_NUM;
  o_minorNum  = EDBG_INVALID_ISTEP_NUM;

  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    // Check whether the istep name in the IPL table matches with the
    // input istep name
    if ( 0 == i_istepName.compare(cv_edbgIStepTable[l_rowNumber].istepName) )
    {
      o_majorNum = cv_edbgIStepTable[l_rowNumber].majorNum;
      o_minorNum = cv_edbgIStepTable[l_rowNumber].minorNum;
      l_status = true;
      break;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool edbgIPLTable::isValid(const std::string & i_istepName)
{
  bool l_status = false;

  //search if this istep name is available in the IPL table
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber)
  {
    //  Check whether the istep name in the IPL table matches with the
    //  input istep name
    if ( 0 == i_istepName.compare(cv_edbgIStepTable[l_rowNumber].istepName) )
    {
      l_status = true;
      break;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool edbgIPLTable::isValid(uint16_t i_majorNum)
{
  if ( i_majorNum > EDBG_LAST_ISTEP_NUM )
  {
    return false;
  }

  uint16_t l_status = false;
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber)
  {
    if ( cv_edbgIStepTable[l_rowNumber].majorNum == i_majorNum )
    {
      l_status = true;
      break;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
bool edbgIPLTable::getIStepNameOf(uint16_t i_position,std::string & o_istepName)
{
  bool l_status = false;
   // If there are 100 rows in the table, then the index of the last row is 99
   // Hence, the array index should always be 1 less than the table size.
   // If the array index is greater than or equal to the table index, then it is
   // out of range and return false.
  if ( i_position < EDBG_NUMBER_OF_ISTEPS )
  {
    o_istepName = cv_edbgIStepTable[i_position].istepName;
    l_status = true;
  }

  return l_status;
}

/*************************************************************************
 * This function returns the position of the first minor number of the
 * specified major number.
 * Example:
 * Row Number       Major    Minor
 *    0               1        0
 *    1               1        1
 *    2               1        2
 *    3               1        3
 *    4               1        5
 *    5               1        7
 *    6               2        0
 *    7               3        1
 *    8               3        8
 *  Row Number is nothing but an array index.
 *  The position of first minor number of the major number 1 is 0.
 *  The position of first minor number of the major number 2 is 6.
 *  The position of first minor number of the major number 3 is 7.
 ************************************************************************/


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t edbgIPLTable::getPosFirstMinorNumber(uint16_t i_majorNum)
{
  if ( i_majorNum > EDBG_LAST_ISTEP_NUM )
  {
    return EDBG_INVALID_POSITION;
  }

  uint16_t l_position = EDBG_INVALID_POSITION;
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    if ( cv_edbgIStepTable[l_rowNumber].majorNum == i_majorNum )
    {
      l_position = l_rowNumber;
      break;
    }
  }

  return l_position;
}

/*************************************************************************
 * This function returns the position of the last minor number of the
 * specified major number.
 * Example:
 * Row Number       Major    Minor
 *    0               1        0
 *    1               1        1
 *    2               1        2
 *    3               1        3
 *    4               1        5
 *    5               1        7
 *    6               2        0
 *    7               3        1
 *    8               3        8
 *  Row Number is nothing but an array index.
 *  The position of last minor number of the major number 1 is 5.
 *  The position of last minor number of the major number 2 is 6.
 *  The position of last minor number of the major number 3 is 8.
 ************************************************************************/
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t edbgIPLTable::getPosLastMinorNumber(uint16_t i_majorNum)
{
  if ( i_majorNum > EDBG_LAST_ISTEP_NUM )
  {
    return EDBG_INVALID_POSITION;
  }

  uint16_t l_position = EDBG_INVALID_POSITION;
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber)
  {
    if ( cv_edbgIStepTable[l_rowNumber].majorNum == i_majorNum)
    {
      // If the table has 100 rows, then the last row number is 99 as the
      // row number starts from 0.
      // To know whether we are at the last row, add 1 to the current
      // row number and compare it with the table size. If it matches, then it
      // indicates that we are at the last row. The last row's minor number is
      // always the last minor number of the last major number.
      if ( ( (l_rowNumber+1) == EDBG_NUMBER_OF_ISTEPS)           ||
         ( cv_edbgIStepTable[l_rowNumber+1].majorNum > i_majorNum ) )
      {
        l_position = l_rowNumber;
        break;
      }
    }
  }

  return l_position;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t edbgIPLTable::getPosition(const std::string & i_istepName)
{
  uint16_t l_position = EDBG_INVALID_POSITION;

  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    // Check whether the istep name in the IPL table matches
    // with the input istep name
    if ( 0 == i_istepName.compare(cv_edbgIStepTable[l_rowNumber].istepName) )
    {
      l_position = l_rowNumber;
      break;
    }
  }

  return l_position;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t edbgIPLTable::getIStepNumber(uint16_t i_position)
{
  uint16_t l_istepNum = edbgIPLTable::EDBG_INVALID_ISTEP_NUM;

  // check whether the array index is out of range
  if ( i_position < EDBG_NUMBER_OF_ISTEPS )
  {
    l_istepNum = cv_edbgIStepTable[i_position].majorNum;
  }

  return l_istepNum;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
uint16_t edbgIPLTable::getIStepMinorNumber(uint16_t i_position)
{
  uint16_t l_istepMinor = edbgIPLTable::EDBG_INVALID_ISTEP_NUM;

  // check whether the array index is out of range
  if ( i_position < EDBG_NUMBER_OF_ISTEPS )
  {
    l_istepMinor = cv_edbgIStepTable[i_position].minorNum;
  }

  return l_istepMinor;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
edbgIPLTable::edbgIStepDestination_t
          edbgIPLTable::getDestination( uint16_t i_majorNum,
                                        uint16_t i_minorNum)
{

  edbgIStepDestination_t  l_destination = EDBG_ISTEP_INVALID_DESTINATION;

    // iterate through the array and return the destination corresponding to the
    // matching major and minor number
  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    if (  cv_edbgIStepTable[l_rowNumber].majorNum == i_majorNum &&
          cv_edbgIStepTable[l_rowNumber].minorNum == i_minorNum )
    {
      l_destination = cv_edbgIStepTable[l_rowNumber].destination;
      break;
    }
  }

  return l_destination;
}

// modified the parameter passing from reference to value
// for the parameters i_majorNum and i_minorNum to make it
// uniform
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool edbgIPLTable::getIStepName(const uint16_t i_majorNum,
                                const uint16_t i_minorNum,
                                std::string & o_istepName)
{

  bool l_status = false;

  // iterate through the array and return the istep name corresponding to the
  // matching major and minor number

  for ( uint16_t l_rowNumber = 0;
        l_rowNumber < EDBG_NUMBER_OF_ISTEPS;
        ++l_rowNumber )
  {
    if (  (cv_edbgIStepTable[l_rowNumber].majorNum == i_majorNum) &&
          (cv_edbgIStepTable[l_rowNumber].minorNum == i_minorNum) )
    {
      o_istepName = cv_edbgIStepTable[l_rowNumber].istepName;
      l_status = true;
    }
  }

  return l_status;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
edbgIPLTable::edbgIStepDestination_t edbgIPLTable::getDestination(uint16_t i_position)
{
  edbgIStepDestination_t  l_destination = EDBG_ISTEP_INVALID_DESTINATION;
   // If there are 100 rows in a table, the last position is 99.
   // ie. the last position in the table is table size - 1.
   // If the given position is greater than or equal to the last position,
   // then it is out of range
  if ( i_position < EDBG_NUMBER_OF_ISTEPS )
  {
    l_destination = cv_edbgIStepTable[i_position].destination;
  }

  return l_destination;
}

// Trigger obmcutil recoveryoff->chassison->wait for chassison()
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int edbgIPLTable::istepPowerOn()
{
    std::string recovery_off_cmd = "obmcutil recoveryoff";
    std::string chasis_on_cmd = "obmcutil --wait chassison";
    std::string chasis_state_cmd = "obmcutil chassisstate 2>&1";
    std::string mbox_reset_cmd = "/usr/sbin/mboxctl --reset";
    std::string data;
    std::array<char, 128> buffer;
    FILE * ostream;
    uint16_t count = 0;
    bool chassisOn = false;

    //Triggering recovery off
    int rc = 0;
    rc = system(recovery_off_cmd.c_str());
    if (rc != 0)
    {
       rc = -errno;
       return rc;
    }

    //Triggering Chassis on
    rc = system(chasis_on_cmd.c_str());
    if (rc != 0)
    {
       rc = -errno;
       return rc;
    }

    // Wait until chassis state (Max wait of 3 minutes) is on.
    do
    {
        //checking chassis state
        ostream = popen(chasis_state_cmd.c_str(), "r");
        if (!ostream)
        {
            rc = -errno;
            pclose(ostream);
            return rc;
        }

        //read the pipe
        while (fgets(buffer.data(), 128, ostream) != NULL) {
            data += buffer.data();
        }

        if (data.find("State.Chassis.PowerState.On") != std::string::npos) {
            chassisOn = true;
            pclose(ostream);
            break;
        }
        usleep(1000);
    }while(++count < 180);

    /* Check chassis state before returning */
    if (chassisOn)
    {
        //Triggering mbox reset
        rc = system(mbox_reset_cmd.c_str());
        if (rc != 0)
        {
           rc = -errno;
           return rc;
        }
    }
    else
    {
        return out.error(-1, FUNCNAME, "Chassis on failed\n");
    }
    return rc;
}


