#!/bin/bash -f

PACKAGENAME="DSelector"
DIRNAME=${PACKAGENAME}

if [  -d "$DIRNAME" ]; then
	echo Need to create temporary directory $DIRNAME but it already exists. Either remove it, or run this script in a different folder. Aborting.
	exit
fi

rm -rf ${DIRNAME}
rm -rf ${ROOT_ANALYSIS_HOME}/${OS_NAME}/packages/${PACKAGENAME}.par
mkdir ${DIRNAME}
mkdir -p ${ROOT_ANALYSIS_HOME}/${OS_NAME}/packages/

#PROOF-INF
mkdir ${DIRNAME}/PROOF-INF
cp SETUP.C ${DIRNAME}/PROOF-INF/

#LIBRARIES
cp ${ROOT_ANALYSIS_HOME}/${OS_NAME}/lib/libDSelector.so ${DIRNAME}/

#HEADERS
cp -r ${ROOT_ANALYSIS_HOME}/${OS_NAME}/include/DSelector ${DIRNAME}/
cp $HALLD_HOME/$BMS_OSNAME/include/particleType.h ${DIRNAME}/

# build archive
tar -czf ${PACKAGENAME}.par ${DIRNAME}

# install
mv ${PACKAGENAME}.par ${ROOT_ANALYSIS_HOME}/${OS_NAME}/packages/

# cleanup
rm -rf ${DIRNAME}

echo ${PACKAGENAME}.par built and installed to ${ROOT_ANALYSIS_HOME}/${OS_NAME}/packages/

