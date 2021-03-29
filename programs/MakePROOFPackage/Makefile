PACKAGENAME=DSelector
DIRNAME=${PACKAGENAME}

all: install

.directories_made:
	mkdir -p ${DIRNAME}
	mkdir -p ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/packages/
	date > .directories_made

#PROOF-INF
.proof-inf_created: .directories_made
	mkdir -p ${DIRNAME}/PROOF-INF
	cp SETUP.C ${DIRNAME}/PROOF-INF/
	date > $@

#LIBRARIES
.libraries_copied: .directories_made
	cp ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/lib/libDSelector.so ${DIRNAME}/
	date > $@

#HEADERS
.headers_copied: .directories_made
	cp -r ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/include/DSelector ${DIRNAME}/
	cp ${HALLD_RECON_HOME}/${BMS_OSNAME}/include/particleType.h ${DIRNAME}/
	cp ${HALLD_RECON_HOME}/${BMS_OSNAME}/include/GlueX.h ${DIRNAME}/
	date > $@

# build archive
${PACKAGENAME}.par: .proof-inf_created .headers_copied .libraries_copied
	tar -czf ${PACKAGENAME}.par ${DIRNAME}

# install
install: ${PACKAGENAME}.par
	install ${PACKAGENAME}.par ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/packages/
	@echo ${PACKAGENAME}.par built and installed to ${ROOT_ANALYSIS_HOME}/${BMS_OSNAME}/packages/

# cleanup
clean:
	rm -rf ${DIRNAME}


