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

#ifndef p10_edbgEcmdDllScom_H
#define p10_edbgEcmdDllScom_H

#include <p10_scominfo.H>

struct p10_chipUnit_t{
        p10ChipUnits_t ekbChipUnit;
	std::string chipUnitType;
        std::string pdbgClassType;
};

//mapping table for ekb, ecmd and pdbg
const p10_chipUnit_t ChipUnitTable[] =
{
        { P10_NO_CU,        "",     "pib" }, //chip unit not passed. Then,use pib.
        { PU_C_CHIPUNIT,    "c",    "core" },
        { PU_EQ_CHIPUNIT,   "eq",   "eq" },
        { PU_PEC_CHIPUNIT,  "pec",  "pec" },
        { PU_PHB_CHIPUNIT,  "phb",  "phb" },
        { PU_MI_CHIPUNIT,   "mi",   "mi" },
        { PU_MCC_CHIPUNIT,  "mcc",  "mcc" },
        { PU_OMIC_CHIPUNIT, "omic", "omic" },
        { PU_OMI_CHIPUNIT,  "omi",  "omi" },
        { PU_PERV_CHIPUNIT, "perv", "chiplet" },
        { PU_MC_CHIPUNIT,   "mc",   "mc" },
        { PU_NMMU_CHIPUNIT, "nmmu", "nmmu" },
        { PU_IOHS_CHIPUNIT, "iohs", "iohs" },
        { PU_PAU_CHIPUNIT,  "pau",  "pau" },
        { PU_PAUC_CHIPUNIT, "pauc", "pauc" },
        // SMPGROUP is a target type that firmware has had for a while to represent
        // the half-links below a SMP link, e.g. each SMP cable is a SMPGROUP. 
        // In P10, we have added a new IOLINK taget to fapi2 to represent same thing
        // in cronus.
        { P10_NO_CU,        "smpgroup", "smpgroup"}, //Not a scommable target. leaving it NO_CU
};

uint32_t p10_dllQueryScom(const ecmdChipTarget & i_target, std::list<ecmdScomData> & o_queryData, uint64_t i_address, ecmdQueryDetail_t i_detail);

uint32_t p10_dllGetScom(const ecmdChipTarget & i_target, uint64_t i_address, ecmdDataBuffer & o_data);

uint32_t p10_dllPutScom(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data);

uint32_t p10_dllPutScomUnderMask(const ecmdChipTarget & i_target, uint64_t i_address, const ecmdDataBuffer & i_data, const ecmdDataBuffer & i_mask);

uint32_t p10x_convertPDBGClassString_to_CUString(std::string pdbgClassType, std::string &o_chipUnitType);

#endif
