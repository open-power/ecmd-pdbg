/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p10/procedures/hwp/perv/p10_spr_name_map.H $            */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2016,2021                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
///
/// @file p10_spr_name_map.H
/// @brief Utility to map SPR name to SPR number
///
/// *HWP HW Maintainer : Doug Holtsinger <Douglas.Holtsinger@ibm.com>
/// *HWP FW Maintainer : Raja Das        <rajadas2@in.ibm.com>
/// *HWP Consumed by      : None (Cronus test only)

#ifndef _P10_SPR_NAME_MAP_H_
#define _P10_SPR_NAME_MAP_H_

//-----------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <p10_spr_name_map_info.H>

//-----------------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------------
const std::string TID_PATTERN = "<??T>";
const std::string INVALID_PATTERN = "N/A";
const std::string INVALID_PATTERN_FIXME = "FIXME";

//-----------------------------------------------------------------------------------
// Global variable definitions
//-----------------------------------------------------------------------------------
extern std::map<std::string, SPRMapEntry> SPR_MAP;
typedef std::map<std::string, SPRMapEntry>::iterator SPR_MAP_IT;

#define SPR_FLAG_READ_ACCESS(_flag)  ((_flag != FLAG_WRITE_ONLY) && (_flag != FLAG_NO_ACCESS))
#define SPR_FLAG_WRITE_ACCESS(_flag) ((_flag != FLAG_READ_ONLY)  && (_flag != FLAG_NO_ACCESS))

// List all the SPR registers(name, number, spy_name, flag)
// Reference <P10 SPR List.xls> on the P10 logic box folder
// Note:
// When ram is enabled, the hardware forces HV=1 PR=0
// current spy names are got from <P10 SPR List.xls>
// For the SPY names, <??T> should be replaced with thread ID, <??C> should be
// replaced with core ID.
// N/A means the SPR can be RAMed, but there is no associated Spy for the SPR.
#define LIST_SPR_REG(_op_)\
    _op_(XER      ,1   , ECP.EC.VS.XER_SO_T<??T>                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(DSCR_RU  ,3   , ECP.EC.LS.LS.T<??T>_DSCR                         ,FLAG_READ_ONLY  ,SPR_PER_PT         ,25)\
    _op_(LR       ,8   , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(CTR      ,9   , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(UAMR     ,13  , ECP.EC.LS.LS.T<??T>_AMR                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(DSCR     ,17  , ECP.EC.LS.LS.T<??T>_DSCR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,25)\
    _op_(DSISR    ,18  , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(DAR      ,19  , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(DEC      ,22  , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_VT         ,64)\
    _op_(SRR0     ,26  , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SRR1     ,27  , ECP.EC.SD.SDE.T<??T>_SRR1                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(CFAR     ,28  , ECP.EC.IFU.T<??T>_CFAR                           ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(AMR      ,29  , ECP.EC.LS.LS.T<??T>_AMR                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(PIDR     ,48  , ECP.EC.MU.T<??T>_PID                             ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(IAMR     ,61  , ECP.EC.LS.LS.T<??T>_IAMR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(TFHAR    ,128 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(TFIAR    ,129 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(TEXASR   ,130 , ECP.EC.SD.SDE.T<??T>_TEXASR                      ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(TEXASRU  ,131 , ECP.EC.SD.SDE.T<??T>_TEXASR                      ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(CTRL_RU  ,136 , ECP.EC.PC.COMMON.SPR_COMMON.CTRL                 ,FLAG_READ_ONLY  ,SPR_SHARED         ,32)\
    _op_(CTRL     ,152 , ECP.EC.PC.COMMON.SPR_COMMON.CTRL                 ,FLAG_WRITE_ONLY ,SPR_SHARED         ,32)\
    _op_(FSCR     ,153 , ECP.EC.SD.SDE.T<??T>_FSCR                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(UAMOR    ,157 , ECP.EC.LS.LS.T<??T>_UAMOR                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(GSR      ,158 , N/A                                              ,FLAG_WRITE_ONLY ,SPR_SHARE_NA       ,64)\
    _op_(PSPB     ,159 , ECP.EC.PC.PMU.SPRCOR.V<??T>_PSPB                 ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(DPDES    ,176 , ECP.EC.PC.COMMON.SPR_COMMON.DPDES                ,FLAG_READ_WRITE ,SPR_PER_CORE       ,64)\
    _op_(DAWR0    ,180 , ECP.EC.LS.LS.T<??T>_DAWR0                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(DAWR1    ,181 , ECP.EC.LS.LS.T<??T>_DAWR1                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(RPR      ,186 , ECP.EC.IFU.RPR                                   ,FLAG_READ_WRITE ,SPR_PER_CORE       ,64)\
    _op_(CIABR    ,187 , ECP.EC.IFU.T<??T>_CIABR                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(DAWRX0   ,188 , ECP.EC.LS.LS.T<??T>_DAWRX0                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(DAWRX1   ,189 , ECP.EC.LS.LS.T<??T>_DAWRX1                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(HFSCR    ,190 , ECP.EC.SD.SDE.T<??T>_HFSCR                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(VRSAVE   ,256 , ECP.EC.SD.SDE.T<??T>_VRSAVE                      ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(SPRG3_RU ,259 , ECP.EC.VS.SPRG3_T<??T>                           ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(TB       ,268 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_LPAR_VT    ,64)\
    _op_(TBU_RU   ,269 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_LPAR_VT    ,32)\
    _op_(SPRG0    ,272 , ECP.EC.VS.SPRG0_T<??T>                           ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SPRG1    ,273 , ECP.EC.VS.SPRG1_T<??T>                           ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SPRG2    ,274 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SPRG3    ,275 , ECP.EC.VS.SPRG3_T<??T>                           ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SPRC     ,276 , ECP.EC.PC.COMMON.SPR_COMMON.SPRC<??T>            ,FLAG_READ_WRITE ,SPR_PER_VT         ,64)\
    _op_(SPRD     ,277 , N/A                                              ,FLAG_READ_WRITE ,SPR_SHARE_NA       ,64)\
    _op_(TBL      ,284 , ECP.EC.PC.L<??T>_TBL                             ,FLAG_WRITE_ONLY ,SPR_PER_LPAR_VT    ,32)\
    _op_(TBU      ,285 , ECP.EC.PC.L<??T>_TBU                             ,FLAG_WRITE_ONLY ,SPR_PER_LPAR_VT    ,32)\
    _op_(TBU40    ,286 , ECP.EC.PC.L<??T>_TB40U                           ,FLAG_WRITE_ONLY ,SPR_PER_LPAR_VT    ,64)\
    _op_(PVR      ,287 , ECP.EC.PC.PMU.SPRCOR.PVR                         ,FLAG_READ_ONLY  ,SPR_SHARED         ,32)\
    _op_(HSPRG0   ,304 , ECP.EC.VS.HSPRG0_T<??T>                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(HSPRG1   ,305 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(HDSISR   ,306 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(HDAR     ,307 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SPURR    ,308 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_VT         ,64)\
    _op_(PURR     ,309 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_VT         ,64)\
    _op_(HDEC     ,310 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_LPAR_VT    ,64)\
    _op_(HRMOR    ,313 , ECP.EC.MU.HRMOR                                  ,FLAG_READ_WRITE ,SPR_SHARED         ,64)\
    _op_(HSRR0    ,314 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(HSRR1    ,315 , ECP.EC.SD.SDE.T<??T>_HSRR1                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(TFMR     ,317 , ECP.EC.PC.T<??T>_TFMR                            ,FLAG_READ_WRITE ,SPR_PARTIAL_SHARED ,64)\
    _op_(LPCR     ,318 , ECP.EC.MU.T<??T>_LPCR                            ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(LPIDR    ,319 , ECP.EC.MU.T<??T>_LPIDR                           ,FLAG_READ_WRITE ,SPR_PER_LPAR_PT    ,64)\
    _op_(HMER     ,336 , ECP.EC.PC.COMMON.SPR_COMMON.V<??T>_HMER          ,FLAG_READ_WRITE ,SPR_PER_VT         ,64)\
    _op_(HMEER    ,337 , ECP.EC.PC.COMMON.SPR_COMMON.HMEER                ,FLAG_READ_WRITE ,SPR_SHARED         ,64)\
    _op_(PCR      ,338 , ECP.EC.IFU.T<??T>_PCR                            ,FLAG_READ_WRITE ,SPR_PER_LPAR_PT    ,64)\
    _op_(HEIR     ,339 , ECP.EC.SD.SDE.T<??T>_HEIR                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(AMOR     ,349 , ECP.EC.LS.LS.AMOR<??T>                           ,FLAG_READ_WRITE ,SPR_PER_LPAR_PT    ,64)\
    _op_(TIR      ,446 , ECP.EC.PC.PMU.SPRCOR.TIR                         ,FLAG_READ_ONLY  ,SPR_PER_PT         ,8 )\
    _op_(PTCR     ,464 , ECP.EC.MU.PTCR                                   ,FLAG_READ_WRITE ,SPR_PER_CORE       ,64)\
    _op_(USPRG0   ,496 , ECP.EC.VS.USPRG0_T<??T>                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(USPRG1   ,497 , ECP.EC.VS.USPRG1_T<??T>                          ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(URMOR    ,505 , ECP.EC.MU.URMOR                                  ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(USRR0    ,506 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(USRR1    ,507 , ECP.EC.SD.SDE.T<??T>_USRR1                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SMFCTRL  ,511 , ECP.EC.IFU.T<??T>_SMFCTRL                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SIERA_RU ,736 , ECP.EC.PC.PMU.T<??T>_SIERA                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(SIERB_RU ,737 , ECP.EC.PC.PMU.T<??T>_SIERB                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(MMCR3_RU ,738 , ECP.EC.PC.PMU.T<??T>_MMCR3                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(SIERA    ,752 , ECP.EC.PC.PMU.T<??T>_SIERA                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SIERB    ,753 , ECP.EC.PC.PMU.T<??T>_SIERB                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MMCR3    ,754 , ECP.EC.PC.PMU.T<??T>_MMCR3                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SIER_RU  ,768 , ECP.EC.PC.PMU.T<??T>_SIER                        ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(MMCR2_RU ,769 , ECP.EC.PC.PMU.T<??T>_MMCR2                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(MMCRA_RU ,770 , ECP.EC.PC.PMU.T<??T>_MMCRA                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(PMC1_RU  ,771 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(PMC2_RU  ,772 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(PMC3_RU  ,773 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(PMC4_RU  ,774 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(PMC5_RU  ,775 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(PMC6_RU  ,776 , N/A                                              ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(MMCR0_RU ,779 , ECP.EC.PC.PMU.T<??T>_MMCR0                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(SIAR_RU  ,780 , ECP.EC.IFU.T<??T>_SIAR                           ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(SDAR_RU  ,781 , ECP.EC.LS.LS.T<??T>_SDAR                         ,FLAG_READ_ONLY  ,SPR_PER_PT         ,64)\
    _op_(MMCR1_RU ,782 , ECP.EC.PC.PMU.T<??T>_MMCR1                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(SIER     ,784 , ECP.EC.PC.PMU.T<??T>_SIER                        ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MMCR2    ,785 , ECP.EC.PC.PMU.T<??T>_MMCR2                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MMCRA    ,786 , ECP.EC.PC.PMU.T<??T>_MMCRA                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(PMC1     ,787 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(PMC2     ,788 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(PMC3     ,789 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(PMC4     ,790 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(PMC5     ,791 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(PMC6     ,792 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(MMCR0    ,795 , ECP.EC.PC.PMU.T<??T>_MMCR0                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(SIAR     ,796 , ECP.EC.IFU.T<??T>_SIAR                           ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(SDAR     ,797 , ECP.EC.LS.LS.T<??T>_SDAR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MMCR1    ,798 , ECP.EC.PC.PMU.T<??T>_MMCR1                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(IMC      ,799 , N/A                                              ,FLAG_WRITE_ONLY, SPR_SHARED         ,64)\
    _op_(BESCRS   ,800 , ECP.EC.SD.SDE.T<??T>_BESCR                       ,FLAG_READ_WRITE_SET ,SPR_PER_PT     ,64)\
    _op_(BESCRSU  ,801 , ECP.EC.SD.SDE.T<??T>_BESCR                       ,FLAG_READ_WRITE_SET ,SPR_PER_PT     ,32)\
    _op_(BESCRR   ,802 , ECP.EC.SD.SDE.T<??T>_BESCR                       ,FLAG_READ_WRITE_RESET ,SPR_PER_PT   ,64)\
    _op_(BESCRRU  ,803 , ECP.EC.SD.SDE.T<??T>_BESCR                       ,FLAG_READ_WRITE_RESET ,SPR_PER_PT   ,32)\
    _op_(EBBHR    ,804 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(EBBRR    ,805 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(BESCR    ,806 , ECP.EC.SD.SDE.T<??T>_BESCR                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(TAR      ,815 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(ASDR     ,816 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(PSSCR_SU ,823 , ECP.EC.PC.PMC.V<??T>_PSSCR                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(IC       ,848 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(VTB      ,849 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_LPAR_VT    ,64)\
    _op_(LDBAR    ,850 , ECP.EC.PC.IMA.L<??T>_LDBAR                       ,FLAG_READ_WRITE ,SPR_PER_LPAR_VT    ,64)\
    _op_(MMCRC    ,851 , ECP.EC.PC.PMU.PMUC.MMCRC                         ,FLAG_READ_WRITE ,SPR_SHARED         ,32)\
    _op_(PMSR     ,853 , ECP.EC.PC.COMMON.SPR_COMMON.PMSR                 ,FLAG_READ_ONLY  ,SPR_SHARED         ,32)\
    _op_(PSSCR    ,855 , ECP.EC.PC.PMC.V<??T>_PSSCR                       ,FLAG_READ_WRITE ,SPR_PER_VT         ,64)\
    _op_(L2QOSR   ,861 , ECP.EC.MU.L2QOSR_SHADOW                          ,FLAG_WRITE_ONLY ,SPR_PER_CORE       ,64)\
    _op_(TRIG0    ,880 , N/A                                              ,FLAG_WRITE_ONLY ,SPR_PER_PT         ,64)\
    _op_(TRIG1    ,881 , N/A                                              ,FLAG_WRITE_ONLY ,SPR_PER_PT         ,64)\
    _op_(TRIG2    ,882 , N/A                                              ,FLAG_WRITE_ONLY ,SPR_PER_PT         ,64)\
    _op_(PMCR     ,884 , ECP.EC.PC.COMMON.SPR_COMMON.PMCR                 ,FLAG_READ_WRITE ,SPR_PER_CORE       ,64)\
    _op_(RWMR     ,885 , ECP.EC.PC.RWMR                                   ,FLAG_READ_WRITE ,SPR_SHARED         ,64)\
    _op_(WORT     ,895 , ECP.EC.IFU.T<??T>_WORT                           ,FLAG_READ_WRITE ,SPR_PER_PT         ,18)\
    _op_(PPR      ,896 , ECP.EC.SD.SDE.T<??T>_PPR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(PPR32    ,898 , N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(TSCR     ,921 , ECP.EC.SD.SDE.TSCR                               ,FLAG_READ_WRITE ,SPR_SHARED         ,32)\
    _op_(TTR      ,922 , ECP.EC.SD.SDE.TTR                                ,FLAG_READ_WRITE ,SPR_SHARED         ,64)\
    _op_(TRACE    ,1006, N/A                                              ,FLAG_WRITE_ONLY ,SPR_SHARED         ,64)\
    _op_(HID      ,1008, ECP.EC.PC.PMU.SPRCOR.HID                         ,FLAG_READ_WRITE ,SPR_SHARED         ,64)\
    _op_(PIR      ,1023, ECP.EC.PC.PMU.SPRCOR.PIR                         ,FLAG_READ_ONLY  ,SPR_PER_VT         ,32)\
    _op_(NIA      ,2000, ECP.EC.IFU.T<??T>_NIA                            ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MSRD     ,2001, ECP.EC.SD.SDE.T<??T>_MSR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(CR       ,2002, N/A                                              ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(FPSCR    ,2003, ECP.EC.VS.FPSCR_STICKY_T<??T>                    ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(VSCR     ,2004, ECP.EC.VS.VSCR_T<??T>                            ,FLAG_READ_WRITE ,SPR_PER_PT         ,32)\
    _op_(MSR      ,2005, ECP.EC.SD.SDE.T<??T>_MSR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MSR_L1   ,2006, ECP.EC.SD.SDE.T<??T>_MSR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(MSRD_L1  ,2007, ECP.EC.SD.SDE.T<??T>_MSR                         ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)

#define LIST_SPR_REG_POST_DD1(_op_)\
    _op_(HDEXCR_RU ,455   , ECP.EC.IFU.T<??T>_HDEXCR                      ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(HDEXCR    ,471   , ECP.EC.IFU.T<??T>_HDEXCR                      ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)\
    _op_(DEXCR_RU  ,812   , ECP.EC.IFU.T<??T>_DEXCR                       ,FLAG_READ_ONLY  ,SPR_PER_PT         ,32)\
    _op_(DEXCR     ,828   , ECP.EC.IFU.T<??T>_DEXCR                       ,FLAG_READ_WRITE ,SPR_PER_PT         ,64)

#define DO_SPR_MAP(in_name, in_number, in_spy_name, in_flag, in_share_type, in_bit_length)\
    if (std::string(#in_spy_name).find(INVALID_PATTERN_FIXME) == std::string::npos) { \
        SPRMapEntry entry##in_name; \
        entry##in_name.number = in_number; \
        entry##in_name.spy_name = #in_spy_name; \
        entry##in_name.flag = in_flag; \
        entry##in_name.share_type = in_share_type; \
        entry##in_name.bit_length = in_bit_length; \
        entry##in_name.start_bit = 0; \
        SPR_MAP[#in_name] = entry##in_name; \
        SPR_NAMES_ALL.push_back(#in_name); \
    }


extern "C" {
    //-----------------------------------------------------------------------------------
    // Function prototype
    //-----------------------------------------------------------------------------------
    /// @brief Initialize the map between SPR name and SPR number
    /// @return TRUE if the mapping completes successfully, FALSE if the mapping fails.
    //
    bool p10_spr_name_map_init();

    //-----------------------------------------------------------------------------------
    /// @brief Check read-write mode for a SPR register access
    /// @param[in] i_reg_flag => read-write mode of the SPR register
    /// @param[in] i_write  => indicate true if intending to write to SPR
    //                         indicate false if only intending to read
    /// @return TRUE if the read-write mode check completes successfully
    //
    bool p10_spr_name_map_check_flag(const Enum_AccessType i_reg_flag, const bool i_write);

    //-----------------------------------------------------------------------------------
    /// @brief Map SPR name to SPR number
    /// @param[in] i_name   => SPR name
    /// @param[in] i_write  => indicate write/read SPR
    /// @param[out] o_number => SPR number
    /// @return TRUE if the mapping completes successfully
    //
    bool p10_spr_name_map(const std::string i_name,
                          const bool i_write,
                          uint32_t& o_number);

    //-----------------------------------------------------------------------------------
    /// @brief Get share type of SPR
    /// @param[in] i_name        => SPR name
    /// @param[out] o_share_type => share type
    /// @return TRUE if the o_share_type is valid
    //
    bool p10_get_share_type(const std::string i_name,
                            Enum_ShareType& o_share_type);

    //-----------------------------------------------------------------------------------
    /// @brief Get bit length of SPR
    /// @param[in] i_name        => SPR name
    /// @param[out] o_bit_length => bit length
    /// @return TRUE if the o_bit_length is valid
    //
    bool p10_get_bit_length(const std::string i_name,
                            uint8_t& o_bit_length);

    //-----------------------------------------------------------------------------------
    /// @brief Get SPR map entry
    /// @param[in] i_name        => SPR name
    /// @param[out] o_spr_entry  => SPR map entry
    /// @return TRUE if the o_spr_entry is valid
    //
    bool p10_get_spr_entry(const std::string i_name,
                           SPRMapEntry& o_spr_entry);

    //-----------------------------------------------------------------------------------
    /// @brief Get SPR access type
    /// @param[in] i_name        => SPR name
    /// @return access type flag, return FLAG_NO_ACCESS if SPR name not found.
    //
    Enum_AccessType p10_spr_name_map_get_access(std::string& i_name);

    //-----------------------------------------------------------------------------------
    /// @brief Get a random SPR name
    /// @param[in] i_reg_access_flag      => desired read-write access mode of the random SPR register
    /// @param[in] i_sequential_selection => sequential selection instead of random selection of SPR
    /// @param[out] o_spr_info            => information on the selected random SPR
    /// @return true if o_spr_info is valid, false otherwise
    bool p10_get_random_spr_name(const Enum_AccessType i_reg_access_flag,
                                 const bool i_sequential_selection,
                                 p10_spr_name_map_info_t& o_spr_info);


} //extern"C"

#endif //_P10_SPR_NAME_MAP_H_


