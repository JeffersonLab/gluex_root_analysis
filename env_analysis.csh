#!/bin/csh -f

# SET ROOT_ANALYSIS_HOME
set ARGS=($_)
if ("$ARGS" != "") then
	set GRA_DIR="`dirname ${ARGS[2]}`"
	set GRA_DIR="`cd ${GRA_DIR}; pwd`"
else
	set GRA_DIR=`pwd`
	if ( ! -e env_analysis.csh && ! $?ROOT_ANALYSIS_HOME ) then
		echo "Error: Non-interactive usage requires either:"
		echo "1. cd <path_to_gluex_root_analysis>; source env_analysis.csh"
		echo "2. Set ROOT_ANALYSIS_HOME before sourcing this script."
		exit 1
	endif
endif
if ( $?ROOT_ANALYSIS_HOME ) then
	set GRA_DIR=$ROOT_ANALYSIS_HOME
endif
if ( -e ${GRA_DIR}/env_analysis.csh ) then
	setenv ROOT_ANALYSIS_HOME $GRA_DIR
else
	echo "Error: ${GRA_DIR}/env_analysis.csh does not exist."
	echo "Probable error: Non-interactive usage requires either:"
	echo "1. cd <path_to_gluex_root_analysis>; source env_analysis.csh"
	echo "2. Set ROOT_ANALYSIS_HOME before sourcing this script."
	echo "If you already set ROOT_ANALYSIS_HOME before sourcing this script,"
	echo "check that it was set to the correct directory."
	exit 1
endif

# SET BMS_OSNAME, IF NOT SET ALREADY # COPIED FROM sim-recon
if ( ! $?BMS_OSNAME ) then
	setenv BMS_OSNAME `${ROOT_ANALYSIS_HOME}/osrelease.pl`
endif

# SET LIBRARY, PATH
if ( ! $?LD_LIBRARY_PATH ) then
	setenv LD_LIBRARY_PATH
endif
setenv LD_LIBRARY_PATH ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/lib/:$LD_LIBRARY_PATH
setenv PATH ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/bin/:$PATH
