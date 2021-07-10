add_executable(mrediscli mrediscli.cc)
target_link_libraries(mrediscli lava_hiredis)

add_executable(hiredispool hiredispool.cc)
target_link_libraries(hiredispool lava_hiredis)
