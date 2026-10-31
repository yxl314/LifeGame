cmake_minimum_required(VERSION 3.16)

project(LifeGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIFEGAME_BUILD_TESTS "Build the test suite" ON)
option(LIFEGAME_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(LIFEGAME_NO_EXCEPTIONS "Build application code without C++ exceptions" ON)
option(LIFEGAME_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

find_package(Qt5 5.9 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
message(STATUS "Using Qt ${Qt5Core_VERSION}")
if(LIFEGAME_BUILD_TESTS)
    find_package(Qt5 5.9 REQUIRED COMPONENTS Test)
endif()

function(lifegame_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(LIFEGAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(LIFEGAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
    if(LIFEGAME_NO_EXCEPTIONS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /EHs-c-)
        else()
            target_compile_options(${target} PRIVATE -fno-exceptions)
        endif()
    endif()
    if(LIFEGAME_ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Domain: pure C++, no Qt dependency.
# ---------------------------------------------------------------------------
add_library(lifegame_domain STATIC
    src/domain/grid.h
    src/domain/grid.cpp
    src/domain/rule.h
    src/domain/rule.cpp
    src/domain/pattern.h
    src/domain/pattern.cpp
    src/domain/pattern_library.h
    src/domain/pattern_library.cpp
    src/domain/rle.h
    src/domain/rle.cpp
    src/domain/history.h
    src/domain/history.cpp
    src/domain/board_limits.h
    src/domain/boundary.h
    src/domain/cell.h
)
target_include_directories(lifegame_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
lifegame_set_warnings(lifegame_domain)
target_compile_features(lifegame_domain PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Model: observable application state (QtCore + QtConcurrent, no UI).
# ---------------------------------------------------------------------------
add_library(lifegame_shared INTERFACE)
target_sources(lifegame_shared INTERFACE
    src/shared/life_stats.h
    src/shared/app_settings.h
    src/shared/jump_info.h
)
target_include_directories(lifegame_shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(lifegame_shared INTERFACE cxx_std_17)

add_library(lifegame_model STATIC
    src/model/life_model.h
    src/model/life_model.cpp
    src/model/simulation_engine.h
    src/model/simulation_engine.cpp
    src/model/model_log.h
    src/model/model_log.cpp
    src/model/command_result.h
    src/model/stall_reason.h
)
target_link_libraries(lifegame_model PUBLIC lifegame_domain lifegame_shared Qt5::Core Qt5::Concurrent)
target_include_directories(lifegame_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
lifegame_set_warnings(lifegame_model)
target_compile_features(lifegame_model PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# View: widgets observe the model; no controller/service/app includes.
# ---------------------------------------------------------------------------
add_library(lifegame_view STATIC
    src/view/board_view.h
    src/view/board_view.cpp
    src/view/control_panel.h
    src/view/control_panel.cpp
    src/view/settings_dialog.h
    src/view/settings_dialog.cpp
    src/view/jump_progress_dialog.h
    src/view/jump_progress_dialog.cpp
    src/view/status_bar_view.h
    src/view/status_bar_view.cpp
    src/view/dialogs.h
    src/view/dialogs.cpp
    src/view/interfaces.h
    src/view/message_box_confirm_sink.h
    src/view/message_box_confirm_sink.cpp
    src/view/view_log.h
    src/view/view_log.cpp
    src/view/theme.h
    src/view/theme.cpp
    src/view/rule_catalog.h
    src/view/rule_catalog.cpp
    src/view/pattern_catalog.h
    src/view/pattern_catalog.cpp
    src/view/speed_scale.h
    src/view/speed_scale.cpp
    resources/resources.qrc
)
target_link_libraries(lifegame_view PUBLIC lifegame_domain lifegame_shared Qt5::Widgets)
target_include_directories(lifegame_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
lifegame_set_warnings(lifegame_view)
target_compile_features(lifegame_view PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Service: persistence and file I/O (no UI).
# ---------------------------------------------------------------------------
add_library(lifegame_service STATIC
    src/service/session_state.h
    src/service/service_log.h
    src/service/service_log.cpp
    src/service/file_error.h
    src/service/settings_error.h
    src/service/settings_store.h
    src/service/settings_store.cpp
    src/service/rle_file_service.h
    src/service/rle_file_service.cpp
    src/service/image_file_service.h
    src/service/image_file_service.cpp
)
target_link_libraries(lifegame_service PUBLIC lifegame_domain lifegame_shared Qt5::Core Qt5::Gui)
target_include_directories(lifegame_service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
lifegame_set_warnings(lifegame_service)
target_compile_features(lifegame_service PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Controller: input interpretation; may use model, view and service.
# ---------------------------------------------------------------------------
add_library(lifegame_controller STATIC
    src/controller/line_raster.h
    src/controller/line_raster.cpp
    src/controller/error_text.h
    src/controller/error_text.cpp
    src/controller/controller_log.h
    src/controller/controller_log.cpp
    src/controller/board_controller.h
    src/controller/board_controller.cpp
    src/controller/simulation_controller.h
    src/controller/simulation_controller.cpp
    src/controller/jump_controller.h
    src/controller/jump_controller.cpp
    src/controller/file_controller.h
    src/controller/file_controller.cpp
    src/controller/configuration_controller.h
    src/controller/configuration_controller.cpp
    src/controller/session_controller.h
    src/controller/session_controller.cpp
)
target_link_libraries(lifegame_controller PUBLIC lifegame_model lifegame_view lifegame_service lifegame_shared Qt5::Widgets)
target_include_directories(lifegame_controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
lifegame_set_warnings(lifegame_controller)
target_compile_features(lifegame_controller PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Application: composition root + entry point.
# ---------------------------------------------------------------------------
add_library(lifegame_app STATIC
    src/app/main_window.h
    src/app/main_window.cpp
    src/app/logging.h
    src/app/logging.cpp
)
target_link_libraries(lifegame_app PUBLIC lifegame_controller)
target_include_directories(lifegame_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
lifegame_set_warnings(lifegame_app)
target_compile_features(lifegame_app PUBLIC cxx_std_17)

add_executable(lifegame src/app/main.cpp)
target_link_libraries(lifegame PRIVATE lifegame_app)
lifegame_set_warnings(lifegame)

install(TARGETS lifegame RUNTIME DESTINATION bin)

# ---------------------------------------------------------------------------
# Tests.
# ---------------------------------------------------------------------------
if(LIFEGAME_BUILD_TESTS)
    enable_testing()

    add_executable(domain_tests tests/domain_tests.cpp)
    target_link_libraries(domain_tests PRIVATE lifegame_domain)
    lifegame_set_warnings(domain_tests)
    add_test(NAME domain_tests COMMAND domain_tests)

    add_executable(model_tests tests/model_tests.cpp)
    target_link_libraries(model_tests PRIVATE lifegame_model Qt5::Test)
    lifegame_set_warnings(model_tests)
    add_test(NAME model_tests COMMAND model_tests)

    add_executable(view_tests tests/view_tests.cpp)
    target_link_libraries(view_tests PRIVATE lifegame_view Qt5::Test)
    lifegame_set_warnings(view_tests)
    add_test(NAME view_tests COMMAND view_tests)
    set_tests_properties(view_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(controller_tests tests/controller_tests.cpp)
    target_link_libraries(controller_tests PRIVATE lifegame_controller Qt5::Test)
    lifegame_set_warnings(controller_tests)
    add_test(NAME controller_tests COMMAND controller_tests)
    set_tests_properties(controller_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(ui_tests tests/ui_tests.cpp)
    target_link_libraries(ui_tests PRIVATE lifegame_app Qt5::Test)
    lifegame_set_warnings(ui_tests)
    add_test(NAME ui_tests COMMAND ui_tests)
    set_tests_properties(ui_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(logging_tests tests/logging_tests.cpp)
    target_link_libraries(logging_tests PRIVATE lifegame_app Qt5::Test)
    lifegame_set_warnings(logging_tests)
    add_test(NAME logging_tests COMMAND logging_tests)

    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME layering
             COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_layering.py
                     ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME layering_selftest
             COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/layering_selftest.py)
endif()
