# ==============================================================================
# Enersion Tests CMake Configuration
# ==============================================================================

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test)

# Protocol Tests
add_executable(test_protocol
    test_protocol.cpp
)

target_include_directories(test_protocol PRIVATE
    ${CMAKE_SOURCE_DIR}/src/protocol
    ${CMAKE_SOURCE_DIR}/src/hal
)

target_link_libraries(test_protocol PRIVATE
    Qt6::Test
    enersion_protocol
)

add_test(NAME ProtocolTests COMMAND test_protocol)

# Service Tests
add_executable(test_service
    test_service.cpp
)

target_include_directories(test_service PRIVATE
    ${CMAKE_SOURCE_DIR}/src/service
    ${CMAKE_SOURCE_DIR}/src/protocol
    ${CMAKE_SOURCE_DIR}/src/hal
)

target_link_libraries(test_service PRIVATE
    Qt6::Test
    Qt6::Core
    enersion_service
)

add_test(NAME ServiceTests COMMAND test_service)



