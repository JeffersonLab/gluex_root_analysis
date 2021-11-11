all: make_libraries make_programs

make_libraries:
	make -C libraries/DSelector all

make_programs:
	make -C programs/MakeDSelector all
	make -C programs/ConfigDSelector all
	make -C programs/tree_to_amptools all
	make -C programs/MakePROOFPackage all

clean:
	make -C libraries/DSelector clean
	make -C programs/MakeDSelector clean
	make -C programs/ConfigDSelector clean
	make -C programs/tree_to_amptools clean
	make -C programs/MakePROOFPackage clean

