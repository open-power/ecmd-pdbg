#!/usr/bin/env python

## IBM_PROLOG_BEGIN_TAG
#  
# eCMD for pdbg Project
#
# Copyright 2017,2018 IBM International Business Machines Corp.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# 	http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
## IBM_PROLOG_END_TAG

# This script will setup a number of variables used through out the make
# Those variables are then written out to a makefile.config
# makefile.config is included by makefile.base

# Python module imports
import os
import sys
import glob
import platform
import textwrap
import re
import argparse
import subprocess

#######################################
# Create the cmdline objects and args #
#######################################

# Add into -h text to describe variable determination via this script
# 1) Script command line args
# 2) From environment variables
# 3) Automatic determination if possible

parser = argparse.ArgumentParser(add_help=False, formatter_class=argparse.RawTextHelpFormatter,
                                 description=textwrap.dedent('''\
                                 This script creates all the variables necessary to build pdbg

                                 It determines the proper values in 3 ways:
                                 1) Command line options to this script
                                 2) Environment variables defined when script is invoked
                                 3) Looking in default locations (i.e. /usr/bin/g++)

                                 For most users building using the default packages of their distro,
                                 no options should be required.
                                 '''), 
                                 epilog=textwrap.dedent('''\
                                 Examples:
                                   ./config.py --ecmd-root /home/user/ecmd --pdbg-root /home/user/pdbg
                                 ''')
)

# Group for required args so the help displays properly
reqgroup = parser.add_argument_group('Required Arguments')

# Add in our required args
# None at this time

# Group for the optional args so the help displays properly
optgroup = parser.add_argument_group('Optional Arguments')

# These args can also be set by declaring their environment variable
# before calling this script.
# If you specify both, the cmdline arg wins over the env variable
# --help
optgroup.add_argument("-h", "--help", help="Show this message and exit", action="help")

# --ecmd-root
optgroup.add_argument("--ecmd-root", help="The location of the eCMD repo to build against\n"
                                          "Default is to use local subrepo")

# --pdbg-root
optgroup.add_argument("--pdbg-root", help="The location of the pdbg repo to build against\n"
                                          "Default is to use local subrepo")

# --install-path
optgroup.add_argument("--install-path", help="Path to install to\n"
                                             "INSTALL_PATH from the environment")

# --host
optgroup.add_argument("--host", help="The host architecture\n"
                                     "HOST_ARCH from the environment")

# --target
optgroup.add_argument("--target", help="The target architecture\n"
                                       "TARGET_ARCH from the environment")

# --cxx
optgroup.add_argument("--cxx", help="The compiler to use\n"
                                    "CXX from the environment")

# --ld
optgroup.add_argument("--ld", help="The linker to use\n"
                                   "LD from the environment")

# --ar
optgroup.add_argument("--ar", help="The archive creator to use\n"
                                   "AR from the environment")

# --strip
optgroup.add_argument("--strip", help="The strip program to use\n"
                                      "STRIP from the environment")

# --sysroot
optgroup.add_argument("--sysroot", help="The system root to us.  Default is /", default='/')

# --swig
optgroup.add_argument("--swig", help="The swig executable to use\n"
                                     "SWIG from the environment")

# --perl
optgroup.add_argument("--perl", help="The perl executable to use\n"
                                     "ECMDPERLBIN from the environment")

# --perlinc
optgroup.add_argument("--perlinc", help="The perl include path to use\n"
                                        "PERLINC from the environment")

# --python
optgroup.add_argument("--python", help="The python executable to use\n"
                                       "ECMDPYTHONBIN from the environment")

# --pythoninc
optgroup.add_argument("--pythoninc", help="The python include path to use\n"
                                          "PYINC from the environment")

# --python3
optgroup.add_argument("--python3", help="The python3 executable to use\n"
                                        "ECMDPYTHON3BIN from the environment")

# --python3inc
optgroup.add_argument("--python3inc", help="The python3 include path to use\n"
                                           "PY3INC from the environment")

# --doxygen
optgroup.add_argument("--doxygen", help="The doxygen executable to use\n"
                                        "DOXYGENBIN from the environment")

# --output-root
optgroup.add_argument("--output-root", help="The location to place build output\n"
                                            "OUTPUT_ROOT from the environment")

# --extensions
optgroup.add_argument("--extensions", help="Filter down the list of extensions to build\n"
                                           "EXTENSIONS from the environment")
# --ecmd-repos
optgroup.add_argument("--ecmd-repos", help="Other ecmd extension/plugin repos to include in build\n")

# --remove-sim
optgroup.add_argument("--remove-sim", action='store_true', help="Enable REMOVE_SIM in build")

# --without-swig
optgroup.add_argument("--without-swig", action='store_true', help="Disable all swig actions")

# --without-perl
optgroup.add_argument("--without-perl", action='store_true', help="Disable perl module build\n"
                                                                  "CREATE_PERLAPI from the environment")

# --without-python
optgroup.add_argument("--without-python", action='store_true', help="Disable python module build\n"
                                                                    "CREATE_PYAPI from the environment")

# --without-python3
optgroup.add_argument("--without-python3", action='store_true', help="Disable python3 module build\n"
                                                                     "CREATE_PY3API from the environment")

# --build-verbose
optgroup.add_argument("--build-verbose", action='store_true', help="Enable verbose messaging during builds.\n"
                                                                   "Displays compiler calls, etc..\n"
                                                                   "VERBOSE from the environment")
optgroup.add_argument("--bmc-build", action='store_true', help="Provides an easy way to set the right defaults for OpenBMC build\n"
                                                               "Same as: \n"
                                                               "--target armv5e\n"
                                                               "--ld \"${CXX}\"\n"
                                                               "--sysroot $SDKTARGETSYSROOT\n"
                                                               "--without-sim\n"
                                                               "--without-perl\n"
                                                               "--without-python3")

# Parse the cmdline for the args we just added
args = parser.parse_args()

# Store any variables we wish to write to the makefiles here
buildvars = dict()

print("++++ Configuring edbg ++++")

# First, determine our EDBG_ROOT variable
# EDBG_ROOT is the top level directory of the ecmd-pdbg source repo
# EDBG_ROOT is used to derive a number of variable throughout this script
EDBG_ROOT = os.path.dirname(os.path.realpath(__file__))
buildvars["EDBG_ROOT"] = EDBG_ROOT

usingEcmdSubrepo = False
if (args.ecmd_root):
    ECMD_ROOT = args.ecmd_root
else:
    # Use the ecmd subrepo
    ECMD_ROOT = os.path.join(os.path.dirname(os.path.realpath(__file__)), "ecmd")
    usingEcmdSubrepo = True
buildvars["ECMD_ROOT"] = ECMD_ROOT

usingPdbgSubrepo = False
if (args.pdbg_root):
    PDBG_ROOT = args.pdbg_root
else:
    # Use the pdbg subrepo
    PDBG_ROOT = os.path.join(os.path.dirname(os.path.realpath(__file__)), "pdbg")
    usingPdbgSubrepo = True
buildvars["PDBG_ROOT"] = PDBG_ROOT

###############################################################
# Let's setup up all the info about our build environment     #
###############################################################

# If we are using the subrepo, check to see if the dir is empty
# If it is, call the submodule init and update
if (usingEcmdSubrepo and (not os.listdir(ECMD_ROOT))):
    print("Initializing eCMD git submodule..")

    rc = os.system("git submodule init ecmd")
    if (rc):
        exit(rc)

    rc = os.system("git submodule update ecmd")
    if (rc):
        exit(rc)

# If we are using the subrepo, check to see if the dir is empty
# If it is, call the submodule init and update
if (usingPdbgSubrepo and (not os.listdir(PDBG_ROOT))):
    print("Initializing pdbg git submodule..")

    rc = os.system("git submodule init pdbg")
    if (rc):
        exit(rc)

    rc = os.system("git submodule update pdbg")
    if (rc):
        exit(rc)

print("Determining host and distro..")

# We have a couple args that are short cuts for setting a number of other args
# Process those here and setup up everything so the downstream code does its thing
if (args.bmc_build):
    args.target = "armv5e"
    args.without_perl = True
    args.without_python3 = True
    args.remove_sim = True
    args.ld = os.environ["CXX"]
    args.sysroot = os.environ["SDKTARGETSYSROOT"]

# Determine the HOST_ARCH
HOST_ARCH = ""
if (args.host is not None):
    HOST_ARCH = args.host
else:
    HOST_ARCH = platform.machine()
buildvars["HOST_ARCH"] = HOST_ARCH

# Set the host base arch.  Just happens to be the first 3 characters
HOST_BARCH = HOST_ARCH[0:3]
buildvars["HOST_BARCH"] = HOST_BARCH

# Determine the TARGET_ARCH
TARGET_ARCH = ""
if (args.target is not None):
    TARGET_ARCH = args.target
elif ("TARGET_ARCH" in os.environ):
    TARGET_ARCH = os.environ["TARGET_ARCH"]
else:
    # Not given, default to the HOST_ARCH
    TARGET_ARCH = HOST_ARCH
buildvars["TARGET_ARCH"] = TARGET_ARCH

# Set the target base arch.  Just happens to be the first 3 characters
TARGET_BARCH = TARGET_ARCH[0:3]
buildvars["TARGET_BARCH"] = TARGET_BARCH

################################################
# Set our output locations for build artifacts #
################################################

print("Establishing output locations..")

# If the OUTPUT_ROOT was passed in, use that for base directory for generated
# files. Otherwise use ECMD_ROOT.
# OUTPUT_ROOT establishes the top level of where all build artifacts will go
if (args.output_root is not None):
    OUTPUT_ROOT = args.output_root
elif ("OUTPUT_ROOT" in os.environ):
    OUTPUT_ROOT = os.environ["OUTPUT_ROOT"]
else:
    OUTPUT_ROOT = os.path.join(EDBG_ROOT, "build")
buildvars["OUTPUT_ROOT"] = OUTPUT_ROOT

# All objects from the build go to a common dir at the top level
# This does come with the stipulation that all source must have unique names
OBJPATH = os.path.join(OUTPUT_ROOT, "obj")
OBJPATH += "/" # Tack this on so the .C->.o rules run properly
buildvars["OBJPATH"] = OBJPATH

# Setup the output path info for the created binaries and libraries
# We have one top level output path where all output binaries go
# This could be shared libs, archives or executables
OUTPATH = os.path.join(OUTPUT_ROOT, "out")
buildvars["OUTPATH"] = OUTPATH
buildvars["OUTBIN"] = os.path.join(OUTPATH, "bin")
buildvars["OUTLIB"] = os.path.join(OUTPATH, "lib")

# Create a common place for all dtb files created to go
DTBPATH = os.path.join(OUTPUT_ROOT, "dtb");
buildvars["DTBPATH"] = DTBPATH

##################################################
# Default things we need setup for every compile #
##################################################
# CXX = the compiler
# CXXFLAGS = flags to pass to the compiler
# LD = the linker
# LDFLAGS = flags to pass to the linker when linking exe's
# SLDFLAGS = flags to pass to the linker when linking shared libs
# AR = the archive creator
# DEFINES = -D defines to pass thru

print("Establishing compiler locations..")

# Compiler - CXX
CXX = ""
if (args.cxx is not None):
    CXX = args.cxx
elif ("CXX" in os.environ):
    CXX = os.environ["CXX"]
else:
    CXX = "/usr/bin/g++"
buildvars["CXX"] = CXX

# Linker - LD
LD = ""
if (args.ld is not None):
    LD = args.ld
elif ("LD" in os.environ):
    LD = os.environ["LD"]
else:
    LD = "/usr/bin/g++"
buildvars["LD"] = LD

# Archive - AR
AR = ""
if (args.ar is not None):
    AR = args.ar
elif ("AR" in os.environ):
    AR = os.environ["AR"]
else:
    AR = "/usr/bin/ar"
buildvars["AR"] = AR


# Strip - STRIP
STRIP = ""
if (args.strip is not None):
    STRIP = args.strip
elif ("STRIP" in os.environ):
    STRIP = os.environ["STRIP"]
else:
    STRIP = "/usr/bin/strip"
buildvars["STRIP"] = STRIP

print("Establishing compiler options..")

# Setup the variable defaults
DEFINES = ""
GPATH = ""
CXXFLAGS = ""
LDFLAGS = ""
SLDFLAGS = ""

# Common compile flags across any OS
CXXFLAGS = "-g -I."

# If the user passed thru extra defines, grab them
if "DEFINES" in os.environ:
    DEFINES = os.environ["DEFINES"]

# Setup common variables across distros
if (TARGET_BARCH == "x86" or TARGET_BARCH == "ppc"):
    GPATH += " " + OBJPATH
    CXXFLAGS += " -Wall -fPIC -std=c++11"
    LDFLAGS += " -fPIC"
    SLDFLAGS += " -shared -fPIC"
    if (TARGET_ARCH.find("64") != -1):
        CXXFLAGS += " -m64"
        LDFLAGS += " -m64"
        SLDFLAGS += " -m64"
    else:
        CXXFLAGS += " -m32"
        LDFLAGS += " -m32"
        SLDFLAGS += " -m32"
elif (TARGET_BARCH == "arm"):
    GPATH += " " + OBJPATH
    CXXFLAGS += " -Wall -fPIC"
    LDFLAGS += " -fPIC"
    SLDFLAGS += " -shared -fPIC"
else:
    print("ERROR: Unknown arch \"%\" detected, can't setup compile options" % TARGET_BARCH)
    sys.exit(1)

# See if REMOVE_SIM is enabled from the cmdline
if (args.remove_sim):
    DEFINES += " -DREMOVE_SIM"

# Export everything we defined
buildvars["DEFINES"] = DEFINES
buildvars["GPATH"] = GPATH
buildvars["CXXFLAGS"] = CXXFLAGS
buildvars["LDFLAGS"] = LDFLAGS
buildvars["SLDFLAGS"] = SLDFLAGS

###################################
# Setup for creating SWIG outputs #
###################################

# Check for optional args to disable building of perl and python module
if (args.without_swig or args.without_perl):
    CREATE_PERLAPI = "no"
elif ("CREATE_PERLAPI" in os.environ):
    CREATE_PERLAPI = os.environ["CREATE_PERLAPI"]
else:
    CREATE_PERLAPI = "yes"
buildvars["CREATE_PERLAPI"] = CREATE_PERLAPI

if (args.without_swig or args.without_python):
    CREATE_PYAPI = "no"
elif ("CREATE_PYAPI" in os.environ):
    CREATE_PYAPI = os.environ["CREATE_PYAPI"]
else:
    CREATE_PYAPI = "yes"
buildvars["CREATE_PYAPI"] = CREATE_PYAPI

if (args.without_swig or args.without_python3):
    CREATE_PY3API = "no"
elif ("CREATE_PY3API" in os.environ):
    CREATE_PY3API = os.environ["CREATE_PY3API"]
else:
    CREATE_PY3API = "yes"
buildvars["CREATE_PY3API"] = CREATE_PY3API

# The swig executable to use
if (not args.without_swig):
    SWIG = ""
    if (args.swig is not None):
        SWIG = args.swig
    elif ("SWIG" in os.environ):
        SWIG = os.environ["SWIG"]
    else:
        SWIG = "/usr/bin/swig"
    buildvars["SWIG"] = SWIG

#################################
# Misc. variables for the build #
#################################

# Enable verbose build option
# By default, we want it quiet which is @
VERBOSE = "@"
if (args.build_verbose):
    VERBOSE = ""
elif ("VERBOSE" in os.environ):
    VERBOSE = os.environ["VERBOSE"]
buildvars["VERBOSE"] = VERBOSE

#######################################
# Setup info around doing the install #
#######################################

# See if the user specified it via the script cmdline
# If not, pull it from the env or set the default
if (args.install_path is not None):
    INSTALL_PATH = args.install_path
elif ("INSTALL_PATH" in os.environ):
    INSTALL_PATH = os.environ["INSTALL_PATH"]
else:    
    # If INSTALL_PATH wasn't given, install into our local dir
    INSTALL_PATH = os.path.join(EDBG_ROOT, "install")
buildvars["INSTALL_PATH"] = INSTALL_PATH

##########################################
# Define the eCMD aspects we want to use #
##########################################
# We define this here so it get passed down to the ecmd build
# That will allow us to build the smallest code base possible
DEFINES_FUNC = ""
#DEFINES_FUNC += " -DECMD_REMOVE_SCOM_FUNCTIONS"
#DEFINES_FUNC += " -DECMD_REMOVE_FSI_FUNCTIONS"
#DEFINES_FUNC += " -DECMD_REMOVE_VPD_FUNCTIONS"
#DEFINES_FUNC += " -DECMD_REMOVE_RING_FUNCTIONS"
#DEFINES_FUNC += " -DECMD_REMOVE_CLOCK_FUNCTIONS"
#DEFINES_FUNC += " -DECMD_REMOVE_MEMORY_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_SEDC_SUPPORT"
DEFINES_FUNC += " -DECMD_REMOVE_LATCH_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_ARRAY_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_SPY_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_REFCLOCK_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_PROCESSOR_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_GPIO_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_I2C_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_POWER_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_ADAL_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_JTAG_FUNCTIONS"
#DEFINES_FUNC += " -DECMD_REMOVE_INIT_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_TRACEARRAY_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_SENSOR_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_BLOCK_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_MPIPL_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_PNOR_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_SP_FUNCTIONS"
DEFINES_FUNC += " -DECMD_REMOVE_UNITID_FUNCTIONS"

# And these are the cip extension defines
#DEFINES_FUNC += " -DCIP_REMOVE_MEMORY_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_INSTRUCTION_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_BREAKPOINT_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_VR_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_VSR_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_PORE_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_RW_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_MBOX_FUNCTIONS"
DEFINES_FUNC += " -DCIP_REMOVE_PMC_VOLTAGE_FUNCTIONS"

buildvars["DEFINES_FUNC"] = DEFINES_FUNC

##################################################
# Write out all our variables to makefile.config #
##################################################

# Get the makefile.config to use, otherwise use the default
if ("MAKEFILE_CONFIG_NAME" in os.environ):
    MAKEFILE_CONFIG_NAME = os.environ["MAKEFILE_CONFIG_NAME"]
else:
    MAKEFILE_CONFIG_NAME = "makefile.config"

# Now go thru everything that has been setup and write it out to the file
print("Writing %s" % os.path.join(EDBG_ROOT, MAKEFILE_CONFIG_NAME))
config = open(os.path.join(EDBG_ROOT, MAKEFILE_CONFIG_NAME), 'w')
config.write("\n")
config.write("# These variables are generated by config.py\n")
config.write("\n")

# Write out all the variables
for var in sorted(buildvars):
    config.write("%s := %s\n" % (var, buildvars[var]))
config.write("\n")

# Export them so they can be referenced by any scripts used in the build
for var in sorted(buildvars):
    config.write("export %s\n" % var)
config.write("\n")

config.close()


# Our edbg config is done, now call configure on our subrepos via system calls
print("++++ Configuring ecmd ++++");
# Load all the function defines into the env before calling ecmd configure
os.environ["DEFINES"] = DEFINES_FUNC
command =  "cd " + ECMD_ROOT + " && ./config.py --output-root `pwd` --ld \"" + LD
command += "\" --extensions \"cip fapi2\" --target " + TARGET_ARCH + " --host " + HOST_ARCH
command += " --without-pyecmd --build-disable-test"
command += (" --swig %s" % args.swig) if (args.swig) else ""
command += " --remove-sim" if (args.remove_sim) else ""
command += " --without-swig" if (args.without_swig) else ""
command += " --without-perl" if (args.without_perl) else ""
command += " --without-python" if (args.without_python) else ""
command += " --without-python3" if (args.without_python3) else ""
command += (" --sysroot %s" % args.sysroot) if (args.sysroot) else ""
rc = os.system(command)
if (rc):
    exit(rc)

print("++++ Configuring pdbg ++++");
command = "cd " + PDBG_ROOT + " && ./bootstrap.sh && CFLAGS=\"-fPIC\" ./configure"
# If cross building, need to set our HOST_ARCH properly for pdbg
# pdbg's types don't exactly match what ecmd/edbg use (but are probably the correct ones)
if (TARGET_BARCH == "arm"):
    if (HOST_ARCH == "ppc64le"):
        command += " -host arm-openbmc-linux-gnueabi --build powerpc64le-linux-gnu"
    elif (HOST_ARCH == "x86_64"):
        command += " --host arm-openbmc-linux-gnueabi --build x86_64-linux-gnu"
rc = os.system(command)
if (rc):
    exit(rc)
