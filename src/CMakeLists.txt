add_compile_options(-msse -maes -march=native)

find_package(OpenSSL REQUIRED)

aux_source_directory(. SRCS)

add_executable(test ${SRCS})

target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto)