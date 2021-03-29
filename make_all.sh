#!/bin/csh -f

# LIBRARIES
cd libraries
cd DSelector
echo info: \"make all\" in libraries/DSelector
make all
cd ../../

# PROGRAMS
echo os = $BMS_OSNAME
cd programs/MakeDSelector
make all
cd ../tree_to_amptools/
make all
cd ../MakePROOFPackage
./build.sh
cd ../..
