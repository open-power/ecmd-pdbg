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

# eCMD includes
CXXFLAGS += -I ${ECMD_ROOT}/ecmd-core/capi -I ${ECMD_ROOT}/ecmd-core/cmd -I ${ECMD_ROOT}/ecmd-core/dll -I ${ECMD_ROOT}/src_${TARGET_ARCH}
# edbg includes
CXXFLAGS += -I ${EDBG_ROOT}/src/common -I ${EDBG_ROOT}/src/dll -I ${EDBG_ROOT}/src/vpd
# pdbg includes
CXXFLAGS += -I ${PDBG_ROOT} -I ${PDBG_ROOT}/libpdbg -fpermissive
# libxml2
CXXFLAGS += -I /usr/include/libxml2


# eCMD files
VPATH  := ${VPATH}:${ECMD_ROOT}/ecmd-core/capi:${ECMD_ROOT}/ecmd-core/cmd:${ECMD_ROOT}/ecmd-core/dll:${ECMD_ROOT}/src_${TARGET_ARCH}
# edbg files
VPATH  := ${VPATH}:${EDBG_ROOT}/src/common:${EDBG_ROOT}/src/dll:${EDBG_ROOT}/src/vpd
# pdbg files
VPATH  := ${VPATH}:${PDBG_ROOT}:${PDBG_ROOT}/libpdbg

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
all: | ecmd-full pdbg-full edbg-full

config: | ecmd-config pdbg-config

build: | ecmd-build pdbg-build edbg-build

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

ecmd-config: ecmd-banner
	${VERBOSE} cd ${ECMD_ROOT} && ./config.py --output-root `pwd` --extensions "" --without-swig --target ${TARGET_ARCH} --host ${HOST_ARCH}

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

pdbg-config: pdbg-banner
	${VERBOSE} cd ${PDBG_ROOT} && ./bootstrap.sh && unset LD && CFLAGS="-fPIC" ./configure

pdbg-build: pdbg-banner
	${VERBOSE} make -C ${PDBG_ROOT} --no-print-directory

pdbg-clean: pdbg-banner
	@make -C ${PDBG_ROOT} clean --no-print-directory

####
# edbg build rules
####
# No edbg-config rule here because it's expected the user called it before
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

	@echo "Creating ${TARGET_ARCH}/bin dir ..."
	@mkdir -p ${INSTALL_PATH}/bin

	@echo "Creating ${TARGET_ARCH}/lib dir ..."
	@mkdir -p ${INSTALL_PATH}/lib

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

	@echo ""
	@echo "Stripping bin dir ..."
	@${STRIP} ${INSTALL_PATH}/bin/*

	@echo "Stripping lib dir ..."
	@${STRIP} ${INSTALL_PATH}/lib/*
	@echo ""

	@echo "Installing edbgReturnCodes.H ..."
	@cp src/common/edbgReturnCodes.H ${INSTALL_PATH}/help/.

	@echo "Installing help text ..."
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getscom.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putscom.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getcfam.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putcfam.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getvpdkeyword.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putvpdkeyword.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/ecmdquery.htxt ${INSTALL_PATH}/help/.

	@echo "Installing command wrappers ..."
	@cp ${ECMD_ROOT}/ecmd-core/bin/ecmdWrapper.sh ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/getscom ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/putscom ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/getcfam ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/putcfam ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/getvpdkeyword ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/putvpdkeyword ${INSTALL_PATH}/bin/.
	@cp -P ${ECMD_ROOT}/ecmd-core/bin/ecmdquery ${INSTALL_PATH}/bin/.

	@echo "Creating env.sh setup script ..."
	@echo "export ECMD_EXE=${INSTALL_PATH}/bin/edbg" > ${INSTALL_PATH}/bin/env.sh
	@echo "export ECMD_DLL_FILE=${INSTALL_PATH}/lib/edbg.dll" >> ${INSTALL_PATH}/bin/env.sh
	@echo "export EDBG_HOME=${INSTALL_PATH}" >> ${INSTALL_PATH}/bin/env.sh
	@echo "export PATH=\$$PATH:${INSTALL_PATH}/bin" >> ${INSTALL_PATH}/bin/env.sh
	@echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:${INSTALL_PATH}/lib" >> ${INSTALL_PATH}/bin/env.sh

# *****************************************************************************
# Debug rule for any makefile testing
# *****************************************************************************
# Allows you to print any variable by doing this:
# make print-BUILD_TARGETS
print-%:
	@echo $*=$($*)
