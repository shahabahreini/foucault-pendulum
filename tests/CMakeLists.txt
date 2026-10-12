add_executable(test_physics test_physics.cpp)
target_link_libraries(test_physics PRIVATE foucault::foucault)
target_compile_definitions(test_physics PRIVATE
  FOUCAULT_REFERENCE_DIR="${PROJECT_SOURCE_DIR}/data/reference")
if(NOT MSVC)
  target_compile_options(test_physics PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME physics COMMAND test_physics)

add_test(NAME cli_help COMMAND foucault-sim --help)
add_test(NAME cli_runs COMMAND foucault-sim --quiet --duration 20 --out -)
add_test(NAME cli_rejects_bad_latitude COMMAND foucault-sim --lat 120 --duration 1)
set_tests_properties(cli_rejects_bad_latitude PROPERTIES WILL_FAIL TRUE)
