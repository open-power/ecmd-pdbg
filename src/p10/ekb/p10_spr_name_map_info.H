/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p10/procedures/hwp/perv/p10_spr_name_map_info.H $       */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2016,2020                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
//-----------------------------------------------------------------------------------
///
/// @file p10_spr_name_random.H
/// @brief Return a Random SPR for RAMing
///
//-----------------------------------------------------------------------------------
// *HWP HW Maintainer : Doug Holtsinger <Douglas.Holtsinger@ibm.com>
// *HWP FW Maintainer : Raja Das        <rajadas2@in.ibm.com>
// *HWP Consumed by   : None (Cronus test only)
//-----------------------------------------------------------------------------------

#ifndef _P10_SPR_NAME_MAP_INFO_H_
#define _P10_SPR_NAME_MAP_INFO_H_

//-----------------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------------
enum Enum_ShareType
{
    SPR_PER_CORE,
    SPR_PER_LPAR_PT,
    SPR_PER_LPAR_VT,
    SPR_PER_PT,
    SPR_PER_VT,
    SPR_SHARED,
    SPR_PARTIAL_SHARED,
    SPR_SHARE_NA          // unknown, or dependent on certain conditions
};

enum Enum_AccessType
{
    FLAG_NO_ACCESS = 0,  // Not accessible via RAMing
    FLAG_READ_ONLY,      // Read-only
    FLAG_WRITE_ONLY,     // Write-only
    FLAG_READ_WRITE,     // Read/Write
    FLAG_READ_WRITE_FIRST = FLAG_READ_WRITE,
    FLAG_READ_WRITE_HW,  // Read/Write, but hardware can also write to the register.
    FLAG_READ_WRITE_SET,  // Read/Write Set -- writing 1s sets bits
    FLAG_READ_WRITE_RESET,  // Read/Write Clear -- writing 1s resets bits
    FLAG_READ_WRITE_LAST = FLAG_READ_WRITE_RESET,
    FLAG_ANY_ACCESS      // Any access type
};

//-----------------------------------------------------------------------------------
// Structure definitions
//-----------------------------------------------------------------------------------

typedef struct
{
    int number;
    std::string spy_name;
    Enum_AccessType flag;
    Enum_ShareType share_type;
    uint8_t bit_length;
    uint8_t start_bit;   // FIXME. For now, this is always initialized to 0 for all SPRs in p10_spr_name_map.H
} SPRMapEntry;

typedef struct
{
    std::string spr_name;
    SPRMapEntry map;
} p10_spr_name_map_info_t;


#endif //_P10_SPR_NAME_MAP_INFO_H_


