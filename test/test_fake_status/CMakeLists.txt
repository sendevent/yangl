add_executable(test_fake_status main.cpp)
target_link_libraries(test_fake_status PRIVATE Qt6::Core)

set_target_properties(test_fake_status PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../tests"
)
