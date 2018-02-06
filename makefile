# Makefile for ecmd-pdbg BMC tool

# *****************************************************************************
# Include the common base makefile
# *****************************************************************************
include makefile.base

# *****************************************************************************
# The Common Setup stuff
# *****************************************************************************
TARGET_EXE := edbg
TARGET_DLL := edbg.dll

#CXXFLAGS     := ${CXXFLAGS} -Os

# Create a list of subdirectories for each repo where source will be found
# Then use that list to create our include and vpath definitions
ECMD_SRCDIRS := ecmd-core/capi ecmd-core/cmd ecmd-core/dll src_${TARGET_ARCH}
EDBG_SRCDIRS := src/common src/dll src/vpd src/p9 src/p9/ekb
PDBG_SRCDIRS := libpdbg

# Create our includes
CXXFLAGS += $(foreach srcdir, ${ECMD_SRCDIRS}, -I ${ECMD_ROOT}/${srcdir}) -I /usr/include/libxml2
CXXFLAGS += $(foreach srcdir, ${EDBG_SRCDIRS}, -I ${EDBG_ROOT}/${srcdir})
# Need the root pdbg dir too
CXXFLAGS += -I ${PDBG_ROOT} $(foreach srcdir, ${PDBG_SRCDIRS}, -I ${PDBG_ROOT}/${srcdir})

# Create our vpath
VPATH := $(foreach srcdir, ${ECMD_SRCDIRS}, :${ECMD_ROOT}/${srcdir}):
VPATH += $(foreach srcdir, ${EDBG_SRCDIRS}, :${EDBG_ROOT}/${srcdir}):
VPATH += ${PDBG_ROOT}$(foreach srcdir, ${PDBG_SRCDIRS}, :${PDBG_ROOT}/${srcdir})
# Cleanup spaces introduced
VPATH := $(subst ${space},${empty}, ${VPATH})

# *****************************************************************************
# Setup all the files going into the build
# *****************************************************************************
# The INCLUDES_EXE are files provided by eCMD that if changed, we would want to recompile on
INCLUDES_EXE += ecmdClientCapi.H
INCLUDES_EXE += ecmdDataBuffer.H
INCLUDES_EXE += ecmdReturnCodes.H
INCLUDES_EXE += ecmdStructs.H
INCLUDES_EXE += ecmdUtils.H
INCLUDES_EXE += ecmdSharedUtils.H
INCLUDES_EXE += ecmdDefines.H
INCLUDES_EXE += ecmdDllCapi.H
INCLUDES_EXE += ecmdChipTargetCompare.H

# The source files and includes for edbg that are going into the build
INCLUDES_DLL += edbgCommon.H
INCLUDES_DLL += edbgOutput.H
INCLUDES_DLL += edbgReturnCodes.H
INCLUDES_DLL += lhtVpd.H
INCLUDES_DLL += lhtVpdFile.H
INCLUDES_DLL += lhtVpdDevice.H

# Combine all the includes into one variable for the build
INCLUDES := ${INCLUDES_EXE} ${INCLUDES_DLL}

# edbg source files to pull into the build
SOURCES_DLL += edbgEcmdDll.C
SOURCES_DLL += edbgEcmdSimDll.C
SOURCES_DLL += edbgEcmdDllInfo.C
SOURCES_DLL += edbgOutput.C
SOURCES_DLL += lhtVpd.C
SOURCES_DLL += lhtVpdFile.C
SOURCES_DLL += lhtVpdDevice.C
SOURCES_DLL += p9_scominfo.C

# Like the rest of the DLL files, this one is also included in both builds
# However, it needs to have the EXE defines on when it builds
SOURCES_ALL := ecmdDllCapi.C

# eCMD source files to pull in for a static build
SOURCES_EXE += ecmdClientCapi.C
SOURCES_EXE += ecmdClientCapiFunc.C

SOURCES_EXE += ecmdMain.C
SOURCES_EXE += ecmdInterpreter.C
SOURCES_EXE += ecmdExtInterpreter.C
SOURCES_EXE += ecmdCommandUtils.C
SOURCES_EXE += ecmdUtils.C
SOURCES_EXE += ecmdQueryUser.C
SOURCES_EXE += ecmdMiscUser.C
SOURCES_EXE += ecmdScomUser.C
SOURCES_EXE += ecmdSimUser.C
SOURCES_EXE += ecmdVpdUser.C

SOURCES_EXE += ecmdDataBuffer.C
SOURCES_EXE += ecmdDataBufferBase.C
SOURCES_EXE += ecmdStructs.C
SOURCES_EXE += ecmdSharedUtils.C
SOURCES_EXE += ecmdChipTargetCompare.C

# *****************************************************************************
# Setup all the defines going into the build
# *****************************************************************************
# Push the current git rev into the build so ecmdquery version can return it
DEFINES += -DGIT_COMMIT_REV=\"$(shell git --work-tree=. --git-dir=./.git describe --always --long --dirty || echo unknown)\"
# Push the current date into the build so ecmdquery version can return it as well
DEFINES += -DBUILD_DATE=\"$(shell date +"%Y-%m-%d\ %H:%M:%S\ %Z")\"

# These are options we only need when building the standalone exe
# Turn on eCMD static linking
DEFINES_EXE += -DECMD_STATIC_FUNCTIONS

# Remove debug code in and out of eCMD function calls
DEFINES_EXE += -DECMD_STRIP_DEBUG

# Turn on REMOVE_SIM to shrink the exe as much as possible
#DEFINES_EXE += -DREMOVE_SIM

# Turn off a bunch of functions from eCMD we don't need in this plugin
DEFINES_EXE += -DECMD_REMOVE_SEDC_SUPPORT
DEFINES_EXE += -DECMD_REMOVE_LATCH_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_RING_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_ARRAY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_SPY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_CLOCK_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_REFCLOCK_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_PROCESSOR_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_GPIO_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_I2C_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_POWER_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_ADAL_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_MEMORY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_JTAG_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_INIT_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_TRACEARRAY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_SENSOR_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_BLOCK_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_MPIPL_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_PNOR_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_SP_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_UNITID_FUNCTIONS
#DEFINES_EXE += -DECMD_REMOVE_SCOM_FUNCTIONS
#DEFINES_EXE += -DECMD_REMOVE_FSI_FUNCTIONS
#DEFINES_EXE += -DECMD_REMOVE_VPD_FUNCTIONS

# *****************************************************************************
# The Main Targets
# *****************************************************************************

####
# General build rules
###
# The default action is to do everything to config & build all required components
all: | ecmd-full pdbg-full edbg-full dtb

build: | ecmd-build pdbg-build edbg-build dtb

clean: | ecmd-clean pdbg-clean edbg-clean

####
# eCMD build rules
####
ecmd-full: | ecmd-build

ecmd-banner:
	@printf "\n"
	@echo "++++++++++++++++++++++++++++++++++++++++"
	@echo "+++++ ecmd -- ecmd -- ecmd -- ecmd +++++"
	@echo "++++++++++++++++++++++++++++++++++++++++"
	@printf "\n"

ecmd-build: ecmd-banner
	${VERBOSE} make -C ${ECMD_ROOT} --no-print-directory

ecmd-clean: ecmd-banner
	@make -C ${ECMD_ROOT} clean --no-print-directory

####
# pdbg build rules
####
pdbg-full: | pdbg-build

pdbg-banner:
	@printf "\n"
	@echo "++++++++++++++++++++++++++++++++++++++++"
	@echo "+++++ pdbg -- pdbg -- pdbg -- pdbg +++++"
	@echo "++++++++++++++++++++++++++++++++++++++++"
	@printf "\n"

pdbg-build: pdbg-banner
	${VERBOSE} make -C ${PDBG_ROOT} --no-print-directory

pdbg-clean: pdbg-banner
	@make -C ${PDBG_ROOT} clean --no-print-directory

####
# edbg build rules
####
# invoking this make
edbg-full: | edbg-build

edbg-banner:
	@printf "\n"
	@echo "++++++++++++++++++++++++++++++++++++++++"
	@echo "+++++ edbg -- edbg -- edbg -- edbg +++++"
	@echo "++++++++++++++++++++++++++++++++++++++++"
	@printf "\n"

# This re-call of make instead of having EXE and DLL as dependenicies is required
# The eCMD build creates files EXE and DLL are dependent upon.
# If make isn't completely re-invoked to find the eCMD created files, the build
# will fail saying dependencies are missing - hence edbg-build-actual
edbg-build: edbg-banner
	${VERBOSE} make edbg-build-actual -C ${EDBG_ROOT} --no-print-directory

edbg-build-actual: ${TARGET_EXE} ${TARGET_DLL}

edbg-clean: edbg-banner
	rm -rf ${OBJPATH}
	rm -rf ${OUTPATH}
	rm -rf ${DTBPATH}

dir:
	@mkdir -p ${OBJPATH}
	@mkdir -p ${OUTPATH}
	@mkdir -p ${OUTBIN}
	@mkdir -p ${OUTLIB}
	@mkdir -p ${DTBPATH}

date:
        # Remove the object before each build to force a rebuild to update the date
	@rm -f ${OBJPATH}/edbgEcmdDllInfo.o

# *****************************************************************************
# Object Build Targets
# *****************************************************************************
OBJS_EXE := $(basename ${SOURCES_EXE})
OBJS_EXE := $(addprefix ${OBJPATH}, ${OBJS_EXE})
OBJS_EXE := $(addsuffix .o, ${OBJS_EXE})
OBJS_DLL := $(basename ${SOURCES_DLL})
OBJS_DLL := $(addprefix ${OBJPATH}, ${OBJS_DLL})
OBJS_DLL := $(addsuffix .o, ${OBJS_DLL})
OBJS_ALL := $(basename ${SOURCES_ALL})
OBJS_ALL := $(addprefix ${OBJPATH}, ${OBJS_ALL})
OBJS_ALL := $(addsuffix .o, ${OBJS_ALL})

# *****************************************************************************
# Compile code for the common C++ objects if their respective
# code has been changed.  Or, compile everything if a header file has changed
# *****************************************************************************
# Create the compile defines needed for each type of source building
${OBJS_EXE} ${OBJS_ALL}: CDEFINES = ${DEFINES} ${DEFINES_EXE}
${OBJS_DLL}: CDEFINES = ${DEFINES}

${OBJS_EXE} ${OBJS_DLL} ${OBJS_ALL}: ${OBJPATH}%.o : %.C ${INCLUDES} | dir date
	@echo Compiling $<
	${VERBOSE}${CXX} -c ${CXXFLAGS} $< -o $@ ${CDEFINES}

# *****************************************************************************
# Create the Target
# *****************************************************************************
${TARGET_EXE}: ${OBJS_DLL} ${OBJS_EXE} ${OBJS_ALL}
	@echo Linking ${TARGET_EXE}
	${VERBOSE}${LD} ${LDFLAGS} -o ${OUTBIN}/${TARGET_EXE} $^ -L${PDBG_ROOT}/.libs -lpdbg -lfdt -lz -lxml2

${TARGET_DLL}: ${OBJS_DLL} ${OBJS_ALL}
	@echo Linking ${TARGET_DLL}
	${VERBOSE}${LD} ${SLDFLAGS} -o ${OUTLIB}/${TARGET_DLL} $^ -L${PDBG_ROOT}/.libs -lpdbg -lfdt -lz -lxml2 -L${ECMD_ROOT}/out_${TARGET_ARCH}/lib -lecmd

dtb:
	@echo Creating p9-fake.dtb
	@${VERBOSE} m4 -I ${EDBG_ROOT}/dt ${EDBG_ROOT}/dt/p9-fake.dts.m4 > ${DTBPATH}/p9-fake.dts
	@${VERBOSE} dtc -I dts ${DTBPATH}/p9-fake.dts -O dtb > ${DTBPATH}/p9-fake.dtb
	@echo Creating 2-socket-p9n.dtb
	@${VERBOSE} m4 -I ${EDBG_ROOT}/dt ${EDBG_ROOT}/dt/2-socket-p9n.dts.m4 > ${DTBPATH}/2-socket-p9n.dts
	@${VERBOSE} dtc -I dts ${DTBPATH}/2-socket-p9n.dts -O dtb > ${DTBPATH}/2-socket-p9n.dtb

# *****************************************************************************
# Install what we built
# *****************************************************************************
install:
	@echo "Creating ${INSTALL_PATH}"
	@mkdir -p ${INSTALL_PATH}

	@echo "Creating bin dir ..."
	@mkdir -p ${INSTALL_PATH}/bin

	@echo "Creating help dir ..."
	@mkdir -p ${INSTALL_PATH}/help

	@echo "Creating lib dir ..."
	@mkdir -p ${INSTALL_PATH}/lib

	@echo "Creating dtb dir ..."
	@mkdir -p ${INSTALL_PATH}/dtb
ifeq (${CREATE_PERLAPI},yes)
	@echo "Creating perl dir ..."
	@mkdir -p ${INSTALL_PATH}/perl
endif
ifneq ($(filter yes,${CREATE_PYAPI} ${CREATE_PY3API}),)
	@echo "Creating python dir ..."
	@mkdir -p ${INSTALL_PATH}/python/ecmd
  ifeq (${CREATE_PYAPI},yes)
	@mkdir -p ${INSTALL_PATH}/python/ecmd/python2
  endif
  ifeq (${CREATE_PY3API},yes)
	@mkdir -p ${INSTALL_PATH}/python/ecmd/python3
  endif
endif

	@echo ""
	@echo "Installing edbg plugin ..."
	@cp ${OUTLIB}/${TARGET_DLL} ${INSTALL_PATH}/lib/.

	@echo "Installing edbg exe ..."
	@cp ${OUTBIN}/${TARGET_EXE} ${INSTALL_PATH}/bin/.

	@echo "Installing libecmd.so ..."
	@cp ${ECMD_ROOT}/out_${TARGET_ARCH}/lib/libecmd.so ${INSTALL_PATH}/lib/.

	@echo "Installing libfdt.so* ..."
	@cp -P ${PDBG_ROOT}/.libs/libfdt.so* ${INSTALL_PATH}/lib/.

	@echo "Installing libpdb.so* ..."
	@cp -P ${PDBG_ROOT}/.libs/libpdbg.so* ${INSTALL_PATH}/lib/.
ifeq (${CREATE_PERLAPI},yes)
	@echo "Installing perl module ..."
	@cp -r ${ECMD_ROOT}/out_${TARGET_ARCH}/perl ${INSTALL_PATH}/.
endif
ifneq ($(filter yes,${CREATE_PYAPI} ${CREATE_PY3API}),)
	@echo "Installing python init ..."
	@cp -P ${ECMD_ROOT}/ecmd-core/pyapi/init/__init__.py ${INSTALL_PATH}/python/ecmd/.
  ifeq (${CREATE_PYAPI},yes)
	@echo "Installing python2 module ..."
	@cp -P ${ECMD_ROOT}/out_${TARGET_ARCH}/pyapi/_ecmd.so ${INSTALL_PATH}/python/ecmd/python2/.
	@cp -P ${ECMD_ROOT}/out_${TARGET_ARCH}/pyapi/ecmd.py ${INSTALL_PATH}/python/ecmd/python2/__init__.py
  endif
  ifeq (${CREATE_PY3API},yes)
	@echo "Installing python3 module ..."
	@cp -P ${ECMD_ROOT}/out_${TARGET_ARCH}/py3api/_ecmd.so ${INSTALL_PATH}/python/ecmd/python3/.
	@cp -P ${ECMD_ROOT}/out_${TARGET_ARCH}/py3api/ecmd.py ${INSTALL_PATH}/python/ecmd/python3/__init__.py
  endif
endif

	@echo ""
	@echo "Stripping bin dir ..."
	@${STRIP} ${INSTALL_PATH}/bin/*

	@echo "Stripping lib dir ..."
	@${STRIP} ${INSTALL_PATH}/lib/*
ifeq (${CREATE_PERLAPI},yes)
	@echo "Stripping perl module ..."
	@${STRIP} ${INSTALL_PATH}/perl/ecmd.so
endif
ifeq (${CREATE_PYAPI},yes)
	@echo "Stripping python2 module ..."
	@${STRIP} ${INSTALL_PATH}/python/ecmd/python2/_ecmd.so
endif
ifeq (${CREATE_PY3API},yes)
	@echo "Stripping python3 module ..."
	@${STRIP} ${INSTALL_PATH}/python/ecmd/python3/_ecmd.so
endif

	@echo ""
	@echo "Installing return code headers ..."
	@cp ${ECMD_ROOT}/ecmd-core/capi/ecmdReturnCodes.H ${INSTALL_PATH}/help/.
	@cp src/common/edbgReturnCodes.H ${INSTALL_PATH}/help/.

	@echo "Installing help text ..."
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getscom.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putscom.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getcfam.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putcfam.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getvpdkeyword.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putvpdkeyword.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/ecmdquery.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/out_${TARGET_ARCH}/bin/ecmd.htxt ${INSTALL_PATH}/help/.

	@echo "Installing command wrappers ..."
	@cp ${ECMD_ROOT}/ecmd-core/bin/ecmdWrapper.sh ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/getscom ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/putscom ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/getcfam ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/putcfam ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/getvpdkeyword ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/putvpdkeyword ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/ecmdquery ${INSTALL_PATH}/bin/.

	@echo "Installing bin scripts ..."
	@cp ${EDBG_ROOT}/bin/* ${INSTALL_PATH}/bin/.

	@echo "Installing device trees ..."
	@cp ${DTBPATH}/p9-fake.dtb ${INSTALL_PATH}/dtb/.
	@cp ${DTBPATH}/2-socket-p9n.dtb ${INSTALL_PATH}/dtb/.

	@echo "Creating env.sh setup script ..."
	@echo "export EDBG_HOME=${INSTALL_PATH}" > ${INSTALL_PATH}/bin/env.sh
	@echo "export ECMD_EXE=\$$EDBG_HOME/bin/edbg" >> ${INSTALL_PATH}/bin/env.sh
	@echo "export ECMD_DLL_FILE=\$$EDBG_HOME/lib/edbg.dll" >> ${INSTALL_PATH}/bin/env.sh
	@echo "export PATH=\$$PATH:\$$EDBG_HOME/bin" >> ${INSTALL_PATH}/bin/env.sh
	@echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:\$$EDBG_HOME/lib" >> ${INSTALL_PATH}/bin/env.sh

# *****************************************************************************
# Debug rule for any makefile testing
# *****************************************************************************
# Allows you to print any variable by doing this:
# make print-BUILD_TARGETS
print-%:
	@echo $*=$($*)
