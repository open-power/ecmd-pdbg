/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p10/common/scominfo/p10_scominfo.H $                    */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2018,2019                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
///
/// @file p10_scominfo.H
/// @brief P10 chip unit SCOM address platform translation code
///
/// HWP HW Maintainer: Thi Tran <thi@us.ibm.com>
/// HWP FW Maintainer:
/// HWP Consumed by: Cronus, HB, HWSV
///

#ifndef P10_SCOMINFO_H
#define P10_SCOMINFO_H

// includes
#include <stdint.h>
#include <vector>
#include "p10_cu.H"

extern "C"
{
    // Modes of translation
    typedef enum
    {
        P10_DEFAULT_MODE = 0,       // Default platform behavior
        P10_ENGD_BUILD_MODE = 1,    // Apply customization for ENGD build
    } p10TranslationMode_t;

    typedef enum
    {
        FAILED_TRANSLATION = 0xFFFFFFFFFFFFFFF1ull
    } p10TranslationResult_t;

    /// @brief Creates the actual SCOM address based on the chip unit type, instance, and the input SCOM address (relative to chip unit instance 0)
    /// @param[in] i_p10CU        Enumeration of the chip unit type
    /// @param[in] i_ecLevel      Chip EC level represented in HEX digit value.  Example: i_ecLevel = 0x12 --> EC level 1.2
    /// @param[in] i_chipUnitNum  Instance number of the chip unit
    /// @param[in] i_scomAddr     The input SCOM address associated with the chip unit type
    /// @param[in] i_mode         Translation mode, specifying different addr translation methods.
    /// @retval uint64_t Actual SCOM address for the chip unit instance passed in
    uint64_t p10_scominfo_createChipUnitScomAddr(const p10ChipUnits_t i_p10CU,
            const uint8_t i_ecLevel,
            const uint8_t i_chipUnitNum,
            const uint64_t i_scomAddr,
            const uint32_t i_mode = 0);

    /// @brief Determine if the provided SCOM address correlates to any chip units (if so creates a list of chipUnitPairing structures which correspond)
    /// @param[in] i_p10CU              Enumeration of the chip unit type
    /// @param[in] i_ecLevel            Chip EC level represented in HEX digit value.  Example: i_ecLevel = 0x12 --> EC level 1.2
    /// @param[in] i_scomAddr SCOM address to be tested
    /// @param[out] o_chipUnitRelated   Returns true if SCOM address is associated with any chip units
    /// @param[out] o_chipUnitPairing   Collection of chipUnitPairing enums
    /// @param[in] i_mode               Translation mode, specifying different addr translation methods.
    /// @retval uint32_t Return non-zero for error
    uint32_t p10_scominfo_isChipUnitScom(const p10ChipUnits_t i_p10CU,
                                         const uint8_t i_ecLevel,
                                         const uint64_t i_scomAddr,
                                         bool& o_chipUnitRelated,
                                         std::vector<p10_chipUnitPairing_t>& o_chipUnitPairing,
                                         const p10TranslationMode_t i_mode = P10_DEFAULT_MODE);

    /// @brief Alter the unit/unitnum of a target for spys where the clocks-on vs clocks-off targets are different.
    /// @param[in] i_p10CU                Target used for the spy request
    /// @param[in] i_ecLevel              Chip EC level represented in HEX digit value.  Example: i_ecLevel = 0x12 --> EC level 1.2
    /// @param[in] i_targetChipUnitNum    The instance number of the target used for the spy request
    /// @param[in] i_scomaddr             The scom from the clocks-on portion of the spy
    /// @param[out] o_modifiedScomAddr    The translated scom address (none may be needed)
    /// @param[out] o_p10CU               The translated target type
    /// @param[out] o_modifiedChipUnitNum The translated target instance number
    /// @param[in] i_mode                 Translation mode, specifying different addr translation methods.
    /// @retval uint32_t Return non-zero for error
    uint32_t p10_scominfo_fixChipUnitScomAddrOrTarget(const p10ChipUnits_t i_p10CU,
            const uint8_t i_ecLevel,
            const uint32_t i_targetChipUnitNum,
            const uint64_t i_scomaddr,
            uint64_t& o_modifiedScomAddr,
            p10ChipUnits_t& o_p10CU,
            uint32_t& o_modifiedChipUnitNum,
            const uint32_t i_mode = 0);

    /// @brief Validate the chip unit number to be within range
    ///        of a chip unit type.
    /// @param[in] i_chipUnitNum   Value of chip unit number (instance)
    /// @param[in] i_chipUnitType  Chip unit type
    /// @retval Non-zero if error
    uint8_t validateChipUnitNum(const uint8_t i_chipUnitNum,
                                const p10ChipUnits_t i_chipUnitType);

} // extern "C"

#endif /* P10_SCOMINFO_H */
