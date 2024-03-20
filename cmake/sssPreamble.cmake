###########################################
# cope with the problem Threads not found #
###########################################
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

###################
# set build model #
###################
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(COMMON_FLAGS "-Wall -Wfatal-errors -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNODEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -ggdb")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")

    # set default build mode
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()

    # build mode error handle
    if (    NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release"
        AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug"
        AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
        message(WARNING ": Unknown build type - \${CMAKE_BUILD_TYPE}=${CMAKE_BUILD_TYPE}.  Please use one of Debug, Release, or RelWithDebInfo. e.g. call\n\tcmake . -DCMAKE_BUILD_TYPE=Release\n" )
    endif()
endif()


if(MSVC)
    set(SSS_CONFIG_NAME "${CMAKE_BUILD_TYPE}")
    if("${SSS_CONFIG_NAME}" STREQUAL "RelWithDebInfo" )
        set(SSS_CONFIG_NAME "Release")
    endif()
    set(SSS_CONFIG "x64-${FSS_CONFIG_NAME}")
elseif(APPLE)
    set(SSS_CONFIG "osx")
else()
    set(SSS_CONFIG "linux")
endif()

set(SSS_BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/../out/build/${SSS_CONFIG}")
get_filename_component(SSS_BUILD_DIR ${SSS_BUILD_DIR} ABSOLUTE)

if(SSS_BUILD)
    if(NOT (${CMAKE_BINARY_DIR} STREQUAL ${SSS_BUILD_DIR}))
        message(WARNING "unexpected build directory. \n\tCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}\nbut expect\n\tVOLEPSI_BUILD_DIR=${VOLEPSI_BUILD_DIR}")
    endif()
    set(SSS_BUILD_DIR ${CMAKE_BINARY_DIR})
endif()

########################
# setup thirdparty dir #
########################
if(NOT DEFINED SSS_THIRDPARTY_DIR)
    set(SSS_THIRDPARTY_DIR "${CMAKE_CURRENT_LIST_DIR}/../out/install/${SSS_CONFIG}")
    get_filename_component(SSS_THIRDPARTY_DIR ${SSS_THIRDPARTY_DIR} ABSOLUTE)
endif()

if(NOT SSS_THIRDPARTY_CLONE_DIR)
    get_filename_component(SSS_THIRDPARTY_CLONE_DIR "${CMAKE_CURRENT_LIST_DIR}/../out/" ABSOLUTE)
endif()

if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message(STATUS "================= dir config =================")
    message(STATUS "                                           ")
    message(STATUS " Build Dir              = ${SSS_BUILD_DIR}")
    message(STATUS " Thirdparty Dir         = ${SSS_THIRDPARTY_DIR}")
    message(STATUS " Thirdparty Clone Dir   = ${SSS_THIRDPARTY_CLONE_DIR}")
    message(STATUS "                                           ")
    message(STATUS "==============================================")
    message(STATUS "")
endif()