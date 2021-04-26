#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/AravisCamera.o \
	${OBJECTDIR}/src/BaslerCamera.o \
	${OBJECTDIR}/src/PhotonicScienceCamera.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f1

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`pkg-config --cflags aravis-0.8` 
CXXFLAGS=`pkg-config --cflags aravis-0.8` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L${KARABO}/lib -L${KARABO}/extern/lib -Wl,-rpath,${KARABO}/lib -Wl,-rpath,${KARABO}/extern/lib -lkarabo -limageSource `pkg-config --libs karaboDependencies`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaravisCameras.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaravisCameras.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaravisCameras.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} `pkg-config --libs aravis-0.8` -shared -fPIC

${OBJECTDIR}/src/AravisCamera.o: src/AravisCamera.cc 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/AravisCamera.o src/AravisCamera.cc

${OBJECTDIR}/src/BaslerCamera.o: src/BaslerCamera.cc 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/BaslerCamera.o src/BaslerCamera.cc

${OBJECTDIR}/src/PhotonicScienceCamera.o: src/PhotonicScienceCamera.cc 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/PhotonicScienceCamera.o src/PhotonicScienceCamera.cc

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-conf ${TESTFILES}
${TESTDIR}/TestFiles/f1: ${TESTDIR}/src/tests/CameraTest.o ${TESTDIR}/src/tests/test_runner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} `pkg-config --libs aravis-0.8`   `pkg-config cppunit --libs`   


${TESTDIR}/src/tests/CameraTest.o: src/tests/CameraTest.cc 
	${MKDIR} -p ${TESTDIR}/src/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -MMD -MP -MF "$@.d" -o ${TESTDIR}/src/tests/CameraTest.o src/tests/CameraTest.cc


${TESTDIR}/src/tests/test_runner.o: src/tests/test_runner.cc 
	${MKDIR} -p ${TESTDIR}/src/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -MMD -MP -MF "$@.d" -o ${TESTDIR}/src/tests/test_runner.o src/tests/test_runner.cc


${OBJECTDIR}/src/AravisCamera_nomain.o: ${OBJECTDIR}/src/AravisCamera.o src/AravisCamera.cc 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/AravisCamera.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -fPIC  -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/AravisCamera_nomain.o src/AravisCamera.cc;\
	else  \
	    ${CP} ${OBJECTDIR}/src/AravisCamera.o ${OBJECTDIR}/src/AravisCamera_nomain.o;\
	fi

${OBJECTDIR}/src/BaslerCamera_nomain.o: ${OBJECTDIR}/src/BaslerCamera.o src/BaslerCamera.cc 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/BaslerCamera.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -fPIC  -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/BaslerCamera_nomain.o src/BaslerCamera.cc;\
	else  \
	    ${CP} ${OBJECTDIR}/src/BaslerCamera.o ${OBJECTDIR}/src/BaslerCamera_nomain.o;\
	fi

${OBJECTDIR}/src/PhotonicScienceCamera_nomain.o: ${OBJECTDIR}/src/PhotonicScienceCamera.o src/PhotonicScienceCamera.cc 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/PhotonicScienceCamera.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -I${KARABO}/include -I${KARABO}/extern/include -I${KARABO}/extern/include/aravis-0.6 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib64/glib-2.0/include -I/usr/include/glib-2.0 `pkg-config --cflags karaboDependencies` -std=c++11  -fPIC  -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/PhotonicScienceCamera_nomain.o src/PhotonicScienceCamera.cc;\
	else  \
	    ${CP} ${OBJECTDIR}/src/PhotonicScienceCamera.o ${OBJECTDIR}/src/PhotonicScienceCamera_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f1 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaravisCameras.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
