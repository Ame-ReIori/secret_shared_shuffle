if (VERBOSE)
  add_definitions(-DVERBOSE)
endif()

# set(CMAKE_CXX_FLAGS "-maes -msse2 -msse3 -msse4.1 -mpclmul -mavx2 -march=native ${CMAKE_CXX_FLAGS}")

macro(EVAL var)
     if(${ARGN})
         set(${var} ON)
     else()
         set(${var} OFF)
     endif()
endmacro()

option(FETCH_AUTO      "automaticly download and build dependancies" OFF)
option(VERBOSE_FETCH   "Verbose fetch" ON)

if(DEFINED COPROTO_ENABLE_BOOST)
    message("warning, setting SSS_ENABLE_BOOST as COPROTO_ENABLE_BOOST=${COPROTO_ENABLE_BOOST}")
    set(SSS_ENABLE_BOOST ${COPROTO_ENABLE_BOOST})
    unset(COPROTO_ENABLE_BOOST CACHE)
endif()

if(DEFINED COPROTO_ENABLE_OPENSSL)
    message("warning, setting SSS_ENABLE_OPENSSL as COPROTO_ENABLE_OPENSSL=${COPROTO_ENABLE_OPENSSL}")
    set(SSS_ENABLE_OPENSSL ${COPROTO_ENABLE_OPENSSL})
    unset(COPROTO_ENABLE_OPENSSL CACHE)
endif()

if(DEFINED LIBOTE_ENABLE_BITPOLYMUL)
    message("warning, setting SSS_ENABLE_BITPOLYMUL as LIBOTE_ENABLE_BITPOLYMUL=${LIBOTE_ENABLE_BITPOLYMUL}")
    set(SSS_ENABLE_BITPOLYMUL ${LIBOTE_ENABLE_BITPOLYMUL})
    unset(LIBOTE_ENABLE_BITPOLYMUL CACHE)
endif()

if(DEFINED SSS_PIC)
    message("warning, setting SSS_ENABLE_PIC as SSS_PIC=${SSS_PIC}")
    set(SSS_ENABLE_PIC ${SSS_PIC})
    unset(SSS_PIC CACHE)
endif()

EVAL(FETCH_LIBOTE_AUTO 
	  (DEFINED FETCH_LIBOTE AND FETCH_LIBOTE) OR
	  ((NOT DEFINED FETCH_LIBOTE) AND (FETCH_AUTO)))

#########################
# set toolchain version #
#########################
if(NOT DEFINED SSS_STD_VER)
  set(SSS_STD_VER 17)
endif()

if(APPLE)
    set(SSS_ENABLE_SODIUM_DEFAULT OFF)
    set(SSS_ENABLE_RELIC_DEFAULT ON)
else()
    set(SSS_ENABLE_SODIUM_DEFAULT ON)
    set(SSS_ENABLE_RELIC_DEFAULT OFF)
endif()

option(SSS_ENABLE_BOOST         "build coproto with boost support" ON)
option(SSS_ENABLE_OPENSSL       "build coproto with openssl support" OFF)
option(SSS_ENABLE_SODIUM        "use the sodium crypto library" ${SSS_ENABLE_SODIUM_DEFAULT})
option(SSS_SODIUM_MONTGOMERY    "request libOTe to use the modified sodium library (non-standard sodium)." ON)
option(SSS_ENABLE_RELIC         "use the relic crypto library" ${SSS_ENABLE_RELIC_DEFAULT})
option(SSS_ENABLE_SSE           "build the library with SSE intrisics" ON)
option(SSS_ENABLE_AVX           "build the library with AVX intrisics" OFF)
option(SSS_ENABLE_PIC           "build with PIC" OFF)
option(SSS_ENABLE_ASAN          "build with ASAN" OFF)
if(APPLE)
  option(SSS_ENABLE_BITPOLYMUL  "build libOTe with quasiCyclic support" OFF)
else()
  option(SSS_ENABLE_BITPOLYMUL  "build libOTe with quasiCyclic support" ON)
endif()

####################
# add compile flag #
####################
# TODO: use config.in
if(DEFINED USE_EMP)
  add_compile_definitions(USE_EMP)
else(DEFINED USE_LIBOTE)
  add_compile_definitions(USE_LIBOTE)
endif()

message(STATUS "secret shared shuffle options\n=======================================================")

message(STATUS "Option: SSS_NO_SYSTEM_PATH    = ${SSS_NO_SYSTEM_PATH}")
message(STATUS "Option: CMAKE_BUILD_TYPE      = ${CMAKE_BUILD_TYPE}\n")
message(STATUS "Option: FETCH_AUTO            = ${FETCH_AUTO}")
message(STATUS "Option: FETCH_SPARSEHASH      = ${FETCH_SPARSEHASH}")
message(STATUS "Option: FETCH_LIBOTE          = ${FETCH_LIBOTE}")
message(STATUS "Option: FETCH_LIBOTE_AUTO     = ${FETCH_LIBOTE_AUTO}")


message("\n")
message(STATUS "Option: SSS_ENABLE_SSE        = ${SSS_ENABLE_SSE}")
message(STATUS "Option: SSS_ENABLE_PIC        = ${SSS_ENABLE_PIC}")
message(STATUS "Option: SSS_ENABLE_ASAN       = ${SSS_ENABLE_ASAN}")
message(STATUS "Option: SSS_STD_VER           = ${SSS_STD_VER}")
                                                  
message(STATUS "Option: SSS_ENABLE_BOOST      = ${SSS_ENABLE_BOOST}")
message(STATUS "Option: SSS_ENABLE_OPENSSL    = ${SSS_ENABLE_OPENSSL}")
message(STATUS "Option: SSS_ENABLE_BITPOLYMUL = ${SSS_ENABLE_BITPOLYMUL}")
message(STATUS "Option: SSS_ENABLE_SODIUM     = ${SSS_ENABLE_SODIUM}")
message(STATUS "Option: SSS_SODIUM_MONTGOMERY = ${SSS_SODIUM_MONTGOMERY}")
message(STATUS "Option: SSS_ENABLE_RELIC      = ${SSS_ENABLE_RELIC}")
message(STATUS "Backend: Use emp-toolkit      = ${USE_EMP}")
message(STATUS "Backend: Use libOTe           = ${USE_LIBOTE}")
