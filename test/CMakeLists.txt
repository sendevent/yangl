enable_testing()

# Add test utility first
add_subdirectory(test_fake_status)

# Then unit tests
add_subdirectory(tests)
