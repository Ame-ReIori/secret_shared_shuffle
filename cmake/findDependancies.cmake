include("${CMAKE_CURRENT_LIST_DIR}/sssPreamble.cmake")

message(STATUS "SSS_THIRDPARTY_DIR=${SSS_THIRDPARTY_DIR}")

set(PUSHED_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "${SSS_THIRDPARTY_DIR};${CMAKE_PREFIX_PATH}")

macro(FIND_LIBOTE)
    set(ARGS ${ARGN})

    #explicitly asked to fetch libOTe
    if(FETCH_LIBOTE)
        list(APPEND ARGS NO_DEFAULT_PATH PATHS ${SSS_THIRDPARTY_CLONE_DIR})
    elseif(${SSS_NO_SYSTEM_PATH})
        list(APPEND ARGS NO_DEFAULT_PATH PATHS ${CMAKE_PREFIX_PATH})
    endif()
    
    set(libOTe_options iknp simplestot)

    if(SSS_ENABLE_BOOST)
        set(libOTe_options ${libOTe_options} boost)
    endif()
    if(SSS_ENABLE_OPENSSL)
        set(libOTe_options ${libOTe_options} openssl)
    endif()
    if(SSS_ENABLE_SODIUM)
        set(libOTe_options ${libOTe_options} sodium)
        if(SSS_SODIUM_MONTGOMERY)
            set(libOTe_options ${libOTe_options} sodium_montgomery)
        else()
            set(libOTe_options ${libOTe_options} no_sodium_montgomery)
        endif()
    endif()
        
    if(SSS_ENABLE_RELIC)
        set(libOTe_options ${libOTe_options} relic)
    endif()

    if(SSS_ENABLE_BITPOLYMUL)
        set(libOTe_options ${libOTe_options} bitpolymul)
    endif()


    if(SSS_ENABLE_SSE)
        set(libOTe_options ${libOTe_options} sse)
    else()
        set(libOTe_options ${libOTe_options} no_sse)
    endif()

    if(SSS_ENABLE_ASAN)
        set(libOTe_options ${libOTe_options} asan)
    else()
        set(libOTe_options ${libOTe_options} no_asan)
    endif()

    if(SSS_ENABLE_PIC)
        set(libOTe_options ${libOTe_options} pic)
    else()
        set(libOTe_options ${libOTe_options} no_pic)
    endif()
    
    
    message("\n\n\nlibOTe_options=${libOTe_options}")
    find_package(libOTe ${ARGS} COMPONENTS  ${libOTe_options})

    if(TARGET oc::libOTe)
        set(libOTe_FOUND ON)
    else()
        set(libOTe_FOUND  OFF)
    endif()
endmacro()

if(FETCH_LIBOTE_AUTO)
    FIND_LIBOTE(QUIET)
    include(${CMAKE_CURRENT_LIST_DIR}/../thirdparty/getLibOTe.cmake)
endif()

FIND_LIBOTE(REQUIRED)

set(CMAKE_PREFIX_PATH ${PUSHED_CMAKE_PREFIX_PATH})