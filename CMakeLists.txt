# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/aleclanter/development/experimental/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(oled C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(oled oled.c ssd1306.h)

pico_set_program_name(oled "oled")
pico_set_program_version(oled "0.1")

pico_enable_stdio_uart(oled 0)
pico_enable_stdio_usb(oled 1)

# Add the standard library to the build
target_link_libraries(oled pico_stdlib)

# Add any user requested libraries
target_link_libraries(oled
        hardware_i2c
        )

pico_add_extra_outputs(oled)

