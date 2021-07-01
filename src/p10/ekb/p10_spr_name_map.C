/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p10/procedures/hwp/perv/p10_spr_name_map.C $            */
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
/// @file p10_spr_name_map.C
/// @brief Utility to map SPR name to SPR number
///
/// *HWP HW Maintainer : Doug Holtsinger <Douglas.Holtsinger@ibm.com>
/// *HWP FW Maintainer : Raja Das        <rajadas2@in.ibm.com>
/// *HWP Consumed by      : None (Cronus test only)


//-----------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------
#include <p10_spr_name_map.H>
#include <stdlib.h>

std::map<std::string, SPRMapEntry> SPR_MAP;
static bool spr_map_initialized = false;
static uint32_t spr_map_name_select_index = 0;
static std::vector<std::string> SPR_NAMES_RW;
static std::vector<std::string> SPR_NAMES_RO;
static std::vector<std::string> SPR_NAMES_WO;
static std::vector<std::string> SPR_NAMES_ALL;

//-----------------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------------

// return a random value between min and max
static uint32_t p10_random_uniform(const uint32_t min, const uint32_t max)
{
    return min + static_cast<uint32_t>(rand() % (max - min + 1));
}

// See doxygen comments in header file
bool p10_spr_name_map_init()
{
    if (spr_map_initialized)
    {
        return true;
    }

    if (!SPR_MAP.empty())
    {
        return false;
    }

    LIST_SPR_REG(DO_SPR_MAP)
    spr_map_initialized = true;

    return true;
}

// See doxygen comments in header file
bool p10_spr_name_map_check_flag(Enum_AccessType i_reg_flag, bool i_write)
{
    bool l_access_ok = false;

    switch (i_reg_flag)
    {
        case FLAG_NO_ACCESS:
            break;

        case FLAG_READ_ONLY:
            if (!i_write)
            {
                l_access_ok = true;
            }

            break;

        case FLAG_WRITE_ONLY:
            if (i_write)
            {
                l_access_ok = true;
            }

            break;

        case FLAG_READ_WRITE:
        case FLAG_READ_WRITE_HW:
        case FLAG_READ_WRITE_RESET:
        case FLAG_READ_WRITE_SET:
        case FLAG_ANY_ACCESS:
            /* fall-through */
            l_access_ok = true;
            break;

        default:
            break;
    }

    return l_access_ok;
}

// See doxygen comments in header file
bool p10_spr_name_map(const std::string i_name, const bool i_write, uint32_t& o_number)
{
    bool l_check_flag = false;

    if(SPR_MAP.find(i_name) != SPR_MAP.end())
    {
        l_check_flag = p10_spr_name_map_check_flag(SPR_MAP[i_name].flag, i_write);

        if(l_check_flag)
        {
            o_number = SPR_MAP[i_name].number;
        }
    }

    return l_check_flag;
}

// See doxygen comments in header file
bool p10_get_share_type(const std::string i_name, Enum_ShareType& o_share_type)
{
    bool l_rc = false;

    if(SPR_MAP.find(i_name) != SPR_MAP.end())
    {
        o_share_type = SPR_MAP[i_name].share_type;
        l_rc = true;
    }

    return l_rc;
}

// See doxygen comments in header file
bool p10_get_bit_length(const std::string i_name, uint8_t& o_bit_length)
{
    bool l_rc = false;

    if(SPR_MAP.find(i_name) != SPR_MAP.end())
    {
        o_bit_length = SPR_MAP[i_name].bit_length;
        l_rc = true;
    }

    return l_rc;
}

// See doxygen comments in header file
bool p10_get_spr_entry(const std::string i_name, SPRMapEntry& o_spr_entry)
{
    bool l_rc = false;

    if(SPR_MAP.find(i_name) != SPR_MAP.end())
    {
        o_spr_entry = SPR_MAP[i_name];
        l_rc = true;
    }

    return l_rc;
}

// See doxygen comments in header file
Enum_AccessType p10_spr_name_map_get_access(std::string& i_name)
{
    Enum_AccessType l_reg_access_flag = FLAG_NO_ACCESS;
    SPRMapEntry l_spr_entry;

    if (p10_get_spr_entry(i_name, l_spr_entry))
    {
        l_reg_access_flag = l_spr_entry.flag;
    }
    else
    {
        l_reg_access_flag = FLAG_NO_ACCESS;
    }

    return l_reg_access_flag;
}

// See doxygen comments in header file
Enum_AccessType p10_get_random_spr_name(const Enum_AccessType i_reg_access_flag,
                                        const bool i_sequential_selection,
                                        std::string& o_name)
{
    Enum_AccessType l_reg_access_flag = FLAG_NO_ACCESS;
    SPRMapEntry l_spr_entry;
    const std::vector<std::string>* SPR_NAMES;
    uint32_t l_size = 0;
    uint32_t l_idx = 0;

    switch (i_reg_access_flag)
    {
        case FLAG_READ_ONLY:
            SPR_NAMES = &SPR_NAMES_RO;
            break;

        case FLAG_WRITE_ONLY:
            SPR_NAMES = &SPR_NAMES_WO;
            break;

        case FLAG_READ_WRITE:
            SPR_NAMES = &SPR_NAMES_RW;
            break;

        case FLAG_ANY_ACCESS:
            SPR_NAMES = &SPR_NAMES_ALL;
            break;

        default:
            return FLAG_NO_ACCESS;
            break;
    }

    // Check if there are any SPRs of the desired access type
    l_size = (*SPR_NAMES).size();

    if (l_size == 0)
    {
        return FLAG_NO_ACCESS;
    }

    if (i_sequential_selection)
    {
        l_idx = spr_map_name_select_index++ % l_size;
    }
    else
    {
        l_idx = p10_random_uniform(0, l_size - 1);
    }

    o_name = (*SPR_NAMES)[l_idx];

    l_reg_access_flag = p10_spr_name_map_get_access(o_name);

    return l_reg_access_flag;
}


