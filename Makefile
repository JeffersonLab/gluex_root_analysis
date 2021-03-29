all: make_libraries make_programs

make_libraries:
	make -C libraries/DSelector all

make_programs:
	make -C programs/MakeDSelector all
	make -C programs/tree_to_amptools all
	make -C programs/MakePROOFPackage all
