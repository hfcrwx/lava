add_library(lava_hiredis Hiredis.cc HiredisPool.cc HiredisRequest.cc)
target_link_libraries(lava_hiredis muduo_net muduo_base pthread hiredis)

add_subdirectory(tests)