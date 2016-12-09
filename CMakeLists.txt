cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(gluex_root_analysis)

# more warnings
add_definitions(-D_FORTIFY_SOURCE=2)

# set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Mac OS X special settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
endif(APPLE)

# load ROOT CMake modules
if (ROOT_VERSION VERSION_GREATER 6)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/etc/cmake/")
else()
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
endif()
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

# locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED MODULE COMPONENTS MathCore RIO Hist Tree Net Proof)

# define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# source file globbing
file(GLOB SRCS libraries/DSelector/*.cc)

# header directories
include_directories(libraries)
include_directories(libraries/DSelector)

# check for external sim-recon
if (DEFINED ENV{HALLD_HOME} AND DEFINED ENV{BMS_OSNAME})
  message("-- Found external sim-recon in " $ENV{HALLD_HOME}/$ENV{BMS_OSNAME})
  include_directories($ENV{HALLD_HOME}/$ENV{BMS_OSNAME}/include)
  set(EXT_SIM_RECON 1)
else()
  message("-- Warning: sim-recon not found, using included GlueX headers. Make sure they are up-to-date!")
  include_directories(include)
  set(EXT_SIM_RECON 0)
endif()

# create the dictionary
if (ROOT_VERSION VERSION_GREATER 6)
    ROOT_GENERATE_DICTIONARY(DSelectorDict libraries/DSelector/*.h LINKDEF DSelectorLinkDef.h
                             OPTIONS -rml libDSelector.so -rmf libDSelector.rootmap)
else()
    ROOT_GENERATE_DICTIONARY(DSelectorDict libraries/DSelector/*.h LINKDEF DSelectorLinkDef.h)
endif()

# create the shared library
add_library(DSelector SHARED ${SRCS} DSelectorDict.cxx)
target_link_libraries(DSelector ${ROOT_LIBRARIES})

# create executables
add_executable(MakeDSelector programs/MakeDSelector/MakeDSelector.cc)
target_link_libraries(MakeDSelector DSelector)
add_executable(tree_to_amptools programs/tree_to_amptools/tree_to_amptools.cc)
target_link_libraries(tree_to_amptools ${ROOT_LIBRARIES})

# generate rootmap
if (ROOT_VERSION VERSION_LESS 6)
    add_custom_command(TARGET DSelector POST_BUILD
    COMMAND rlibmap -o libDSelector.rootmap
                    -l libDSelector.so
                    -d ${ROOT_LIBRARIES}
                    -c ${CMAKE_SOURCE_DIR}/libraries/DSelector/DSelectorLinkDef.h
    )
endif()

# installation
install(TARGETS DSelector LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/lib)
install(FILES ${PROJECT_BINARY_DIR}/libDSelector.rootmap DESTINATION ${PROJECT_BINARY_DIR}/lib)
if (ROOT_VERSION VERSION_GREATER 6)
    install(FILES ${PROJECT_BINARY_DIR}/DSelectorDict_rdict.pcm DESTINATION ${PROJECT_BINARY_DIR}/lib)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/libraries/DSelector/ DESTINATION ${PROJECT_BINARY_DIR}/include/DSelector
        FILES_MATCHING PATTERN "*.h")
if (NOT EXT_SIM_RECON)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${PROJECT_BINARY_DIR}/include)
endif()
install(TARGETS MakeDSelector RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)
install(TARGETS tree_to_amptools RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)

