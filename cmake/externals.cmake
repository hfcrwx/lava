include(ExternalProject)

ExternalProject_Add(
  muduo-2_0_3
  GIT_REPOSITORY https://github.com.cnpmjs.org/chenshuo/muduo.git
  GIT_TAG 9aaeda812bde476d231495212e6cf331ae2792b4
  SOURCE_DIR "${PROJECT_BINARY_DIR}/muduo"
  PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/patches/muduo.diff
  CMAKE_COMMAND cmake
  CMAKE_ARGS "-DCMAKE_BUILD_TYPE=Debug"
  CMAKE_ARGS "-DMUDUO_BUILD_EXAMPLES=OFF"
)
ExternalProject_Add_StepDependencies(muduo-2_0_3 build hiredis-0_12_1)

ExternalProject_Add(
  hiredis-0_12_1
  GIT_REPOSITORY https://github.com.cnpmjs.org/redis/hiredis.git
  GIT_TAG v0.12.1
  SOURCE_DIR "${PROJECT_BINARY_DIR}/hiredis"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make OPTIMIZATION=-O0 && make hiredis-example hiredis-example-libevent OPTIMIZATION=-O0
  INSTALL_COMMAND make install LIBRARY_PATH=lib
  BUILD_IN_SOURCE 1
)
