set(GIT_REPOSITORY  "https://github.com/osu-crypto/libOTe.git")
set(GIT_TAG         "17b1f9be303cbabd017c832f1f2fe2055e3cfa0f")

set(DEP_NAME  "libOTe")
set(CLONE_DIR "${SSS_THIRDPARTY_CLONE_DIR}/${DEP_NAME}")
set(BUILD_DIR "${CLONE_DIR}/out/build/${SSS_CONFIG}")
set(LOG_FILE  "${CMAKE_CURRENT_LIST_DIR}/log-${DEP_NAME}.log")

include("${CMAKE_CURRENT_LIST_DIR}/fetch.cmake")

option(LIBOTE_DEV "always build libOTe" OFF)

if (NOT ${DEP_NAME}_FOUND OR LIBOTE_DEV)
  string (REPLACE ";" "%" CMAKE_PREFIX_PATH_STR "${CMAKE_PREFIX_PATH}")

  find_program(GIT git REQUIRED)

  set(DOWNLOAD_CMD ${GIT} clone --recursive ${GIT_REPOSITORY})
  set(CHECKOUT_CMD ${GIT} checkout ${GIT_TAG})
  set(SUBMODULE_CMD ${GIT} submodule update --recursive)
  set(CONFIGURE_CMD ${CMAKE_COMMAND} -S ${CLONE_DIR} -B ${BUILD_DIR} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_STR}
                    -DNO_SYSTEM_PATH=OFF
                    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                    -DFETCH_AUTO=${FETCH_AUTO}
                    -DVERBOSE_FETCH=${VERBOSE_FETCH}
                    -DENABLE_ALL_OT=ON
                    -DENABLE_SSE=${SSS_ENABLE_SSE}
                    -DENABLE_BOOST=${SSS_ENABLE_BOOST}
                    -DENABLE_BITPOLYMUL=${SSS_ENABLE_BITPOLYMUL}
                    -DENABLE_OPENSSL=${SSS_ENABLE_OPENSSL}
                    -DENABLE_SODIUM=${SSS_ENABLE_SODIUM}
                    -DENABLE_PIC=${SSS_ENABLE_PIC}
                    -DENABLE_ASAN=${SSS_ENABLE_ASAN}
                    -DENABLE_RELIC=${SSS_ENABLE_RELIC}
                    -DCOPROTO_ENABLE_BOOST=${COPROTO_ENABLE_BOOST} # important option, supporting cp::asioConnect
                    -DOC_THIRDPARTY_CLONE_DIR=${SSS_THIRDPARTY_CLONE_DIR}
                    -DOC_THIRDPARTY_INSTALL_PREFIX=${SSS_THIRDPARTY_DIR}
                    )
  set(BUILD_CMD ${CMAKE_COMMAND} --build ${BUILD_DIR} --config ${CMAKE_BUILD_TYPE})
  set(INSTALL_CMD ${CMAKE_COMMAND} --install ${BUILD_DIR} --config ${CMAKE_BUILD_TYPE} --prefix ${SSS_THIRDPARTY_DIR})

  message("============= Building ${DEP_NAME} =============")
  if (NOT EXISTS ${CLONE_DIR})
    run(NAME "Cloning ${GIT_REPOSITORY}" CMD ${DOWNLOAD_CMD} WD ${SSS_THIRDPARTY_CLONE_DIR})
  endif()

  run(NAME "libOTe Checkout ${GIT_TAG}" CMD ${CHECKOUT_CMD} WD ${CLONE_DIR})
  run(NAME "libOTe submodule" CMD ${SUBMODULE_CMD} WD ${CLONE_DIR})
  run(NAME "libOTe Configure" CMD ${CONFIGURE_CMD} WD ${CLONE_DIR})
  run(NAME "libOTe Build" CMD ${BUILD_CMD} WD ${CLONE_DIR})
  run(NAME "libOTe Install" CMD ${INSTALL_CMD} WD ${CLONE_DIR})

  message("log ${LOG_FILE}\n==========================================")
else()
  message("${DEP_NAME} already fetched.")
endif()

install(CODE "
    if(NOT CMAKE_INSTALL_PREFIX STREQUAL \"${SSS_THIRDPARTY_CLONE_DIR}\")
        execute_process(
            COMMAND ${SUDO} \${CMAKE_COMMAND} --install ${BUILD_DIR}  --config ${CMAKE_BUILD_TYPE} --prefix \${CMAKE_INSTALL_PREFIX}
            WORKING_DIRECTORY ${CLONE_DIR}
            RESULT_VARIABLE RESULT
            COMMAND_ECHO STDOUT
        )
    endif()
")