# File: /home/kami/vision-guard/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(vision_guard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "/usr/local/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -Wall
        -Wextra
        -Werror=return-type
        -fno-omit-frame-pointer
    )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /home/kami/blackbox-sentinel/include
    /usr/local/include
)

find_package(Threads REQUIRED)
find_library(BLACKBOX_LIB blackbox REQUIRED PATHS /usr/local/lib /usr/lib)
find_library(XINFER_LIB xinfer REQUIRED PATHS /usr/local/lib /usr/lib)

message(STATUS "[vision-guard] Linked Blackbox Essential: ${BLACKBOX_LIB}")
message(STATUS "[vision-guard] Linked Xinfer Essential:   ${XINFER_LIB}")

set(LIB_SOURCES
    src/core/tsc_clock.cpp
    src/protocols/shm_vision.cpp
    src/protocols/rtsp_onvif.cpp
    src/protocols/gige_vision.cpp
    src/defense/privacy_airgap.cpp
    src/pipeline/guard_engine.cpp
)

add_library(vision_guard SHARED ${LIB_SOURCES})
add_library(vision_guard_static STATIC ${LIB_SOURCES})
set_target_properties(vision_guard_static PROPERTIES OUTPUT_NAME vision_guard)

target_include_directories(vision_guard PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:/home/kami/blackbox-sentinel/include>
    $<BUILD_INTERFACE:/usr/local/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(vision_guard_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:/home/kami/blackbox-sentinel/include>
    $<BUILD_INTERFACE:/usr/local/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(vision_guard PUBLIC
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    Threads::Threads
)

target_link_libraries(vision_guard_static PUBLIC
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    Threads::Threads
)

add_executable(vision-guard-ctl tools/vision_guard_ctl/main.cpp)
target_link_libraries(vision-guard-ctl PRIVATE
    vision_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

add_executable(aryo-vision tools/aryo_vision/main.cpp)
target_link_libraries(aryo-vision PRIVATE
    vision_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

add_library(vision_guard_dissector_plugin SHARED
    tools/sentinel_plugin/vision_guard_plugin.cpp
)
target_link_libraries(vision_guard_dissector_plugin PRIVATE
    vision_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)
set_target_properties(vision_guard_dissector_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "vision_guard_dissector_plugin"
)

add_executable(bench_latency benchmarks/bench_latency.cpp)
target_link_libraries(bench_latency PRIVATE
    vision_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

enable_testing()

add_executable(test_shm_vision tests/test_shm_vision.cpp)
target_link_libraries(test_shm_vision PRIVATE vision_guard)
add_test(NAME TestShmVision COMMAND test_shm_vision)

add_executable(test_rtsp_gige tests/test_rtsp_gige.cpp)
target_link_libraries(test_rtsp_gige PRIVATE vision_guard)
add_test(NAME TestRtspGige COMMAND test_rtsp_gige)

add_executable(test_plugin tests/test_sentinel_plugin.cpp)
target_link_libraries(test_plugin PRIVATE
    vision_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    ${CMAKE_DL_LIBS}
)
add_test(NAME TestSentinelPlugin COMMAND test_plugin)

add_executable(test_vision_tier1_tier2_pipeline tests/test_vision_tier1_tier2_pipeline.cpp)
target_link_libraries(test_vision_tier1_tier2_pipeline PRIVATE
    vision_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    ${CMAKE_DL_LIBS}
)
add_test(NAME TestVisionTier1Tier2Pipeline COMMAND test_vision_tier1_tier2_pipeline)

install(TARGETS vision_guard vision_guard_static vision-guard-ctl aryo-vision vision_guard_dissector_plugin
    EXPORT VisionGuardTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS vision_guard_dissector_plugin
    DESTINATION /usr/local/lib/blackbox
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/VisionGuard)
configure_package_config_file(
    cmake/VisionGuardConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/VisionGuardConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/VisionGuardConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/VisionGuardConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/VisionGuardConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
)
install(EXPORT VisionGuardTargets
    FILE VisionGuardTargets.cmake
    NAMESPACE VisionGuard::
    DESTINATION ${CONFIG_INSTALL_DIR}
)

configure_file(
    cmake/vision_guard.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/vision_guard.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vision_guard.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
