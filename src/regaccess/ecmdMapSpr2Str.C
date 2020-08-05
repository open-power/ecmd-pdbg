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
#include <stdio.h>
#include <inttypes.h>
#include <string>

// Headers from pdbg/libipl/libekb 
extern "C" { 
#include <libpdbg.h> 
} 

//ecmd to spr map include
#include <ecmdMapSpr2Str.H>

//edbg includes
#include <ecmdReturnCodes.H>
#include <ecmdDataBuffer.H>
#include <edbgOutput.H>
#include <edbgCommon.H>

//This table has been prepared as per the information available in p10_spr_name_map.H.
ecmdSprId2SprStrTable_t p10_sprId2StrTable[] = {
{1   , "XER" 	   },
{3   , "DSCR_RU"   },
{8   , "LR" 	   },
{9   , "CTR" 	   },
{13  , "UAMR" 	   },
{17  , "DSCR" 	   },
{18  , "DSISR" 	   },
{19  , "DAR" 	   },
{22  , "DEC" 	   },
{26  , "SRR0" 	   },
{27  , "SRR1" 	   },
{28  , "CFAR" 	   },
{29  , "AMR" 	   },
{48  , "PIDR" 	   },
{61  , "IAMR" 	   },
{128 , "TFHAR" 	   },
{129 , "TFIAR" 	   },
{130 , "TEXASR"    },
{131 , "TEXASRU"   },
{136 , "CTRL_RU"   },
{152 , "CTRL" 	   },
{153 , "FSCR" 	   },
{157 , "UAMOR" 	   },
{158 , "GSR" 	   },
{159 , "PSPB" 	   },
{176 , "DPDES" 	   },
{180 , "DAWR0" 	   },
{181 , "DAWR1" 	   },
{186 , "RPR" 	   },
{187 , "CIABR" 	   },
{188 , "DAWRX0"    },
{189 , "DAWRX1"    },
{190 , "HFSCR" 	   },
{256 , "VRSAVE"    },
{259 , "SPRG3_RU"  },
{268 , "TB"        },
{269 , "TBU_RU"    },
{272 , "SPRG0" 	   },
{273 , "SPRG1" 	   },
{274 , "SPRG2" 	   },
{275 , "SPRG3" 	   },
{276 , "SPRC" 	   },
{277 , "SPRD" 	   },
{284 , "TBL" 	   },
{285 , "TBU" 	   },
{286 , "TBU40" 	   },
{287 , "PVR" 	   },
{304 , "HSPRG0"	   },
{305 , "HSPRG1"    },
{306 , "HDSISR"    },
{307 , "HDAR" 	   },
{308 , "SPURR" 	   },
{309 , "PURR" 	   },
{310 , "HDEC" 	   },
{313 , "HRMOR" 	   },
{314 , "HSRR0" 	   },
{315 , "HSRR1" 	   },
{317 , "TFMR" 	   },
{318 , "LPCR" 	   },
{319 , "LPIDR" 	   },
{336 , "HMER" 	   },
{337 , "HMEER" 	   },
{338 , "PCR" 	   },
{339 , "HEIR" 	   },
{349 , "AMOR" 	   },
{446 , "TIR" 	   },
{464 , "PTCR" 	   },
{496 , "USPRG0"    },
{497 , "USPRG1"	   },
{505 , "URMOR" 	   },
{506 , "USRR0" 	   },
{507 , "USRR1" 	   },
{511 , "SMFCTRL"   },
{736 , "SIERA_RU"  },
{737 , "SIERB_RU"  },
{738 , "MMCR3_RU"  },
{752 , "SIERA" 	   },
{753 , "SIERB" 	   },
{754 , "MMCR3" 	   },
{768 , "SIER_RU"   },
{769 , "MMCR2_RU"  },
{770 , "MMCRA_RU"  },
{771 , "PMC1_RU"   },
{772 , "PMC2_RU"   },
{773 , "PMC3_RU"   },
{774 , "PMC4_RU"   },
{775 , "PMC5_RU"   },
{776 , "PMC6_RU"   },
{779 , "MMCR0_RU"  },
{780 , "SIAR_RU"   },
{781 , "SDAR_RU"   },
{782 , "MMCR1_RU"  },
{784 , "SIER" 	   },
{785 , "MMCR2" 	   },
{786 , "MMCRA" 	   },
{787 , "PMC1" 	   },
{788 , "PMC2" 	   },
{789 , "PMC3" 	   },
{790 , "PMC4" 	   },
{791 , "PMC5" 	   },
{792 , "PMC6" 	   },
{795 , "MMCR0" 	   },
{796 , "SIAR" 	   },
{797 , "SDAR" 	   },
{798 , "MMCR1" 	   },
{799 , "IMC" 	   },
{800 , "BESCRS"    },
{801 , "BESCRSU"   },
{802 , "BESCRR"    },
{803 , "BESCRRU"   },
{804 , "EBBHR" 	   },
{805 , "EBBRR" 	   },
{806 , "BESCR" 	   },
{815 , "TAR" 	   },
{816 , "ASDR" 	   },
{823 , "PSSCR_SU"  },
{848 , "IC" 	   },
{849 , "VTB" 	   },
{850 , "LDBAR" 	   },
{851 , "MMCRC" 	   },
{853 , "PMSR" 	   },
{855 , "PSSCR" 	   },
{861 , "L2QOSR"    },
{880 , "TRIG0" 	   },
{881 , "TRIG1" 	   },
{882 , "TRIG2" 	   },
{884 , "PMCR" 	   },
{885 , "RWMR" 	   },
{895 , "WORT" 	   },
{896 , "PPR" 	   },
{898 , "PPR32" 	   },
{921 , "TSCR" 	   },
{922 , "TTR" 	   },
{1006, "TRACE" 	   },
{1008, "HID" 	   },
{1023, "PIR" 	   },
{2000, "NIA" 	   },
{2001, "MSRD" 	   },
{2002, "CR" 	   },
{2003, "FPSCR" 	   },
{2004, "VSCR" 	   },
{2005, "MSR" 	   },
{2006, "MSR_L1"    },
{2007, "MSRD_L1"   },
};

uint32_t ecmdMapSpr2Str(uint32_t &i_sprId,std::string &o_SprStr)
{
  uint32_t rc=ECMD_SUCCESS;
  uint32_t l_LastEntry = 0;
  uint32_t i=0;
  bool l_sprFound= false;
  
  if (pdbg_get_proc() == PDBG_PROC_P10) {
   
      while ((p10_sprId2StrTable[i].sprId != l_LastEntry) && (!l_sprFound)) {
          if (p10_sprId2StrTable[i].sprId == i_sprId) {
              o_SprStr=p10_sprId2StrTable[i].sprStr;    
              l_sprFound=true;
          } else {
              i++;  
          }
      };
  }

  if (!l_sprFound){
    return out.error(EDBG_GENERAL_ERROR, FUNCNAME, "Unable to Map SPR ID. "
              "SPR ID=0x%08x\n", i_sprId);  
  }

  return rc;
}

