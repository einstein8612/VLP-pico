file(GLOB TEST_SOURCES test_*.c)

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} ransac_line)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()