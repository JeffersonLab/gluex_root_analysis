# gluex_root_analysis

### Documentation
https://halldweb.jlab.org/wiki/index.php/DSelector

### Setting up the software & environment
Go to the directory where you want the source code to go. Checkout the software here:
```
git clone https://github.com/JeffersonLab/gluex_root_analysis
```

#### Direct building
Set the path to the checked-out gluex_root_analysis directory to be the variable:
```
$ROOT_ANALYSIS_HOME
```
After sourcing your standard GlueX sim-recon environment file, source the environment file appropriate for your shell:
```
source $ROOT_ANALYSIS_HOME/env_analysis.csh
OR
source $ROOT_ANALYSIS_HOME/env_analysis.sh
```
Build and install the software
```
cd $ROOT_ANALYSIS_HOME
./make_all.sh
```

#### CMake building
```
cd gluex_root_analysis
mkdir build
cd build
cmake ..
make
make install
```

