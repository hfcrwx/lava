add_library(lava_hiredis Hiredis.cc ConnectionPool.cc Request.cc Connection.cc)
target_link_libraries(lava_hiredis muduo_net muduo_base pthread hiredis)

add_subdirectory(tests)