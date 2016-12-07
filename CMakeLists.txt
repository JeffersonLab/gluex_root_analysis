cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
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

# adjust some paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED MODULE COMPONENTS MathCore RIO Hist Tree Net Proof)

# define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# source file globbing
file(GLOB SRCS libraries/DSelector/*.cc)

# header directories
include_directories(libraries)
include_directories(libraries/DSelector)
include_directories(include)
include_directories($ENV{HALLD_HOME}/$ENV{BMS_OSNAME}/include)

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

# generate rootmap and install files
if (ROOT_VERSION VERSION_LESS 6)
    add_custom_command(TARGET DSelector POST_BUILD
    COMMAND rlibmap -o ${PROJECT_BINARY_DIR}/lib/libDSelector.rootmap
                    -l ${PROJECT_BINARY_DIR}/lib/libDSelector.so
                    -d ${ROOT_LIBRARIES}
                    -c ${CMAKE_SOURCE_DIR}/libraries/DSelector/DSelectorLinkDef.h
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/include/DSelector
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/libraries/DSelector/*.h ${PROJECT_BINARY_DIR}/include/DSelector
    )
else()
    add_custom_command(TARGET DSelector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/libDSelector.rootmap ${PROJECT_BINARY_DIR}/lib/libDSelector.rootmap
    COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/DSelectorDict_rdict.pcm ${PROJECT_BINARY_DIR}/lib/DSelectorDict_rdict.pcm
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/include/DSelector
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/libraries/DSelector/*.h ${PROJECT_BINARY_DIR}/include/DSelector
    )
endif()

