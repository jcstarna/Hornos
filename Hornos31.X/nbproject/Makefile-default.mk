#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../src/Hornos-31.c ../src/Modbus.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1360937237/Hornos-31.o ${OBJECTDIR}/_ext/1360937237/Modbus.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1360937237/Hornos-31.o.d ${OBJECTDIR}/_ext/1360937237/Modbus.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1360937237/Hornos-31.o ${OBJECTDIR}/_ext/1360937237/Modbus.o

# Source Files
SOURCEFILES=../src/Hornos-31.c ../src/Modbus.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_CC="C:\Program Files (x86)\PICC\CCSCON.exe"
MP_LD="C:\Program Files (x86)\PICC\CCSCON.exe"
sourceline.device=sourceline="\#device PIC16F886"
sourceline.xccompat=sourceline="\#device ANSI" sourceline="\#device PASS_STRINGS=IN_RAM" sourceline="\#device CONST=READ_ONLY" sourceline="\#case" sourceline="\#TYPE SIGNED" sourceline="\#type INT=16, LONG=32"
sourceline.gcccompat=sourceline="\#device ANSI" sourceline="\#device PASS_STRINGS=IN_RAM" sourceline="\#device CONST=READ_ONLY" sourceline="\#case" sourceline="\#TYPE SIGNED"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1360937237/Hornos-31.o: ../src/Hornos-31.c  nbproject/Makefile-${CND_CONF}.mk 
	${MKDIR} ${OBJECTDIR}/_ext/1360937237 
ifeq (3,1) 
	${MKDIR} ${DISTDIR} 
	${MP_CC}  out="${OBJECTDIR}\_ext\1360937237"  ../src/Hornos-31.c +FM +DF +CC #__DEBUG=1 +ICD +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 
	@mv ${OBJECTDIR}/_ext/1360937237/Hornos-31.cof "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.cof" 
	@mv ${OBJECTDIR}/_ext/1360937237/Hornos-31.hex "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.hex"
else 
	${MP_CC}  out=""${OBJECTDIR}\_ext\1360937237"" ../src/Hornos-31.c +EXPORT +FM +DF +CC #__DEBUG=1 +ICD +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 +EXPORTD="${OBJECTDIR}/_ext/1360937237"  
	
endif 
	
${OBJECTDIR}/_ext/1360937237/Modbus.o: ../src/Modbus.c  nbproject/Makefile-${CND_CONF}.mk 
	${MKDIR} ${OBJECTDIR}/_ext/1360937237 
ifeq (3,1) 
	${MKDIR} ${DISTDIR} 
	${MP_CC}  out="${OBJECTDIR}\_ext\1360937237"  ../src/Modbus.c +FM +DF +CC #__DEBUG=1 +ICD +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 
	@mv ${OBJECTDIR}/_ext/1360937237/Modbus.cof "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.cof" 
	@mv ${OBJECTDIR}/_ext/1360937237/Modbus.hex "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.hex"
else 
	${MP_CC}  out=""${OBJECTDIR}\_ext\1360937237"" ../src/Modbus.c +EXPORT +FM +DF +CC #__DEBUG=1 +ICD +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 +EXPORTD="${OBJECTDIR}/_ext/1360937237"  
	
endif 
	
else
${OBJECTDIR}/_ext/1360937237/Hornos-31.o: ../src/Hornos-31.c  nbproject/Makefile-${CND_CONF}.mk 
	${MKDIR} ${OBJECTDIR}/_ext/1360937237 
ifeq (3,1) 
	${MKDIR} ${DISTDIR} 
	${MP_CC}  out="${OBJECTDIR}\_ext\1360937237"  ../src/Hornos-31.c +FM +DF +CC +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 
	@mv ${OBJECTDIR}/_ext/1360937237/Hornos-31.cof "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.cof" 
	@mv ${OBJECTDIR}/_ext/1360937237/Hornos-31.hex "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.hex"
else 
	${MP_CC}  out=""${OBJECTDIR}\_ext\1360937237"" ../src/Hornos-31.c +EXPORT +FM +DF +CC +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 +EXPORTD="${OBJECTDIR}/_ext/1360937237"  
	
endif 
	
${OBJECTDIR}/_ext/1360937237/Modbus.o: ../src/Modbus.c  nbproject/Makefile-${CND_CONF}.mk 
	${MKDIR} ${OBJECTDIR}/_ext/1360937237 
ifeq (3,1) 
	${MKDIR} ${DISTDIR} 
	${MP_CC}  out="${OBJECTDIR}\_ext\1360937237"  ../src/Modbus.c +FM +DF +CC +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 
	@mv ${OBJECTDIR}/_ext/1360937237/Modbus.cof "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.cof" 
	@mv ${OBJECTDIR}/_ext/1360937237/Modbus.hex "${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.hex"
else 
	${MP_CC}  out=""${OBJECTDIR}\_ext\1360937237"" ../src/Modbus.c +EXPORT +FM +DF +CC +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P #__16F886=1 +EXPORTD="${OBJECTDIR}/_ext/1360937237"  
	
endif 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	${MKDIR} ${DISTDIR} 
ifeq (3,1) 
	
else 
	${MP_LD}   out="${DISTDIR}"  +FM +DF +CC LINK=Hornos31.X.${IMAGE_TYPE}.hex=${OBJECTDIR}/_ext/1360937237/Hornos-31.o,${OBJECTDIR}/_ext/1360937237/Modbus.o +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P
	
endif 
	
else
${DISTDIR}/Hornos31.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	${MKDIR} ${DISTDIR} 
ifeq (3,1) 
	
else 
	${MP_LD}   out="${DISTDIR}"  +FM +DF +CC LINK=Hornos31.X.${IMAGE_TYPE}.hex=${OBJECTDIR}/_ext/1360937237/Hornos-31.o,${OBJECTDIR}/_ext/1360937237/Modbus.o +Y=9 +EA I+="C:\Program Files (x86)\PICC\Devices" I+="C:\Program Files (x86)\PICC\Drivers" +DF +LN +T +A +M +J +Z -P
	
endif 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
