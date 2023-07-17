cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(GP2040-CE-MONKEY-TESTER LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(
  -Wall
  -Wextra
)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/bootsel_button.cpp
)

target_link_libraries(${PROJECT_NAME}
  pico_stdlib
)

pico_add_extra_outputs(${PROJECT_NAME})
