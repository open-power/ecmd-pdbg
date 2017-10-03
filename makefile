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
CXXFLAGS += -I ${EDBG_ROOT}/src/common -I ${EDBG_ROOT}/src/dll
# pdbg includes
CXXFLAGS += -I ${PDBG_ROOT} -I ${PDBG_ROOT}/libpdbg -fpermissive

# eCMD files
VPATH  := ${VPATH}:${ECMD_ROOT}/ecmd-core/capi:${ECMD_ROOT}/ecmd-core/cmd:${ECMD_ROOT}/ecmd-core/dll:${ECMD_ROOT}/src_${TARGET_ARCH}
# edbg files
VPATH  := ${VPATH}:${EDBG_ROOT}/src/common:${EDBG_ROOT}/src/dll
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

# The source files and includes for edbg that are going into the build
INCLUDES_DLL += edbgCommon.H
INCLUDES_DLL += edbgOutput.H
INCLUDES_DLL += edbgReturnCodes.H

# Combine all the includes into one variable for the build
INCLUDES := ${INCLUDES_EXE} ${INCLUDES_DLL}

# edbg source files to pull into the build
SOURCES_DLL += edbgEcmdDll.C
SOURCES_DLL += edbgEcmdDllInfo.C
SOURCES_DLL += edbgOutput.C

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

SOURCES_EXE += ecmdDataBuffer.C
SOURCES_EXE += ecmdDataBufferBase.C
SOURCES_EXE += ecmdStructs.C
SOURCES_EXE += ecmdSharedUtils.C

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
DEFINES_EXE += -DREMOVE_SIM

# Turn off a bunch of functions from eCMD we don't need in this plugin
DEFINES_EXE += -DECMD_REMOVE_SEDC_SUPPORT
DEFINES_EXE += -DECMD_REMOVE_LATCH_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_RING_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_ARRAY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_SPY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_CLOCK_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_REFCLOCK_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_PROCESSOR_FUNCTIONS
#DEFINES_EXE += -DECMD_REMOVE_SCOM_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_GPIO_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_I2C_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_POWER_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_ADAL_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_MEMORY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_JTAG_FUNCTIONS
#DEFINES_EXE += -DECMD_REMOVE_FSI_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_INIT_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_TRACEARRAY_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_SENSOR_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_BLOCK_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_MPIPL_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_PNOR_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_SP_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_VPD_FUNCTIONS
DEFINES_EXE += -DECMD_REMOVE_UNITID_FUNCTIONS

# *****************************************************************************
# The Main Targets
# *****************************************************************************
all: ${TARGET_EXE} ${TARGET_DLL}

clean: objclean
	rm -rf ${OUTPATH}

objclean:
	rm -rf ${OBJPATH}

dir:
	@mkdir -p ${OBJPATH}
	@mkdir -p ${OUTPATH}

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
	${VERBOSE}${LD} ${LDFLAGS} -o ${OUTPATH}/${TARGET_EXE} $^ -L${PDBG_ROOT}/.libs -lpdbg -lfdt -lz

${TARGET_DLL}: ${OBJS_DLL} ${OBJS_ALL} fake.dtb.o
	@echo Linking ${TARGET_DLL}
	${VERBOSE}${LD} ${SLDFLAGS} -o ${OUTPATH}/${TARGET_DLL} $^ -L${PDBG_ROOT}/.libs -lpdbg -lfdt -L${ECMD_ROOT}/out_${TARGET_ARCH}/lib -lecmd -lz

# *****************************************************************************
# Install what we built
# *****************************************************************************
install:
        # Create the install path
	@echo "Creating ${INSTALL_PATH}"
	@mkdir -p ${INSTALL_PATH}
	@echo ""

	@echo "Creating bin dir ..."
	@mkdir -p ${INSTALL_PATH}/bin
	@echo ""

	@echo "Creating help dir ..."
	@mkdir -p ${INSTALL_PATH}/help
	@echo ""

	@echo "Creating ${TARGET_ARCH}/bin dir ..."
	@mkdir -p ${INSTALL_PATH}/${TARGET_ARCH}/bin
	@echo ""

	@echo "Creating ${TARGET_ARCH}/lib dir ..."
	@mkdir -p ${INSTALL_PATH}/${TARGET_ARCH}/lib
	@echo ""

	@echo "Installing plugin ..."
	@cp ${OUTPATH}/${TARGET_DLL} ${INSTALL_PATH}/${TARGET_ARCH}/lib/.
	@${STRIP} ${INSTALL_PATH}/${TARGET_ARCH}/lib/${TARGET_DLL}
	@echo ""

	@echo "Installing exe ..."
	@cp ${OUTPATH}/${TARGET_EXE} ${INSTALL_PATH}/${TARGET_ARCH}/bin/.
	@${STRIP} ${INSTALL_PATH}/${TARGET_ARCH}/bin/${TARGET_EXE}
	@echo ""

	@echo "Installing edbgReturnCodes.H ..."
	@cp src/common/edbgReturnCodes.H ${INSTALL_PATH}/help/.
	@echo ""

	@echo "Installing help text ..."
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getscom.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putscom.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/getcfam.htxt ${INSTALL_PATH}/help/.
	@cp ${ECMD_ROOT}/ecmd-core/cmd/help/putcfam.htxt ${INSTALL_PATH}/help/.
	@echo ""

# *****************************************************************************
# Debug rule for any makefile testing
# *****************************************************************************
# Allows you to print any variable by doing this:
# make print-BUILD_TARGETS
print-%:
	@echo $*=$($*)
