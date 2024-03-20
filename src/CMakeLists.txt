add_compile_options(-msse -maes -march=native)

find_package(OpenSSL REQUIRED)

aux_source_directory(. SRCS)

add_executable(test ${SRCS})

target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto oc::libOTe)

target_compile_options(test PUBLIC
        "-std=c++${SSS_STD_VER}") 

if(SSS_ENABLE_SSE)
  target_compile_options(test PUBLIC -mavx)
endif()

if(SSS_ENABLE_PIC)
  target_compile_options(test PUBLIC "-fPIC")
endif()

target_include_directories(test PUBLIC
                      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
                      $<INSTALL_INTERFACE:include>)   

target_include_directories(test PUBLIC
                      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                      $<INSTALL_INTERFACE:include>)

target_compile_options(test PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++${SSS_STD_VER}>)