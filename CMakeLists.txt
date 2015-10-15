#
# test is actually depends on google test
#

# macros
macro(create_source_groups curdir)
    file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)
    foreach(child ${children})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
        create_source_groups(${curdir}/${child})
    else()
        string(REPLACE "/" "\\" groupname ${curdir})
        source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
    endif()
    endforeach()
endmacro()

# body
cmake_minimum_required (VERSION 2.8.11)
project (PathDefence)

list(APPEND CMAKE_CXX_FLAGS "-std=c++1y")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -O3")


set(BinDir ${PROJECT_SOURCE_DIR}/bin)
set(LibDir ${PROJECT_SOURCE_DIR}/lib)

# archive is for static libraries
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BinDir} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LibDir} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LibDir} )
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BinDir} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LibDir} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LibDir} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


include_directories("/usr/local/include")
include_directories("/usr/include")
include_directories("./include")

find_library(ANT_LIBRARY ant)
if (NOT ANT_LIBRARY)
    message(FATAL_ERROR "ant library not found")
endif()

# names of executables should be caps

file(GLOB PATH_DEFENCE_FILES "./src/*" "./include/*")
add_library(path_defence ${PATH_DEFENCE_FILES})
target_link_libraries(path_defence ${ANT_LIBRARY})

add_executable(path_defence_offline "./app/offline.cpp")
target_link_libraries(path_defence_offline path_defence)

add_executable(path_defence_online "./app/online.cpp")
target_link_libraries(path_defence_online path_defence)

add_executable(path_defence_online_param_setting "./app/online_param_setting.cpp")
target_link_libraries(path_defence_online_param_setting path_defence)

find_library(GTEST_LIB gtest)
if (NOT GTEST_LIB)
    message(FATAL_ERROR "gtest library not found")
endif()

add_executable(path_defence_test "./app/test.cpp")
target_link_libraries(path_defence_test path_defence ${GTEST_LIB} )
