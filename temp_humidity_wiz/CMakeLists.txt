cmake_minimum_required(VERSION 3.12)
# #####################################################################################################
# This cmake config is made for STM32 MCUs and STM32 
# file structure. Edit MCU information up to the next border
# This is based on Matej Blagsic's youtube video: https://www.youtube.com/watch?v=9tWRdnyWGb8&t=123s
# his Github: https://github.com/prtzl/Embedded_videos/tree/master/046_BUILDING-PT5-cmake/sample-f407vg
project(temp_humidity_wiz)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Core/app)

set(MCU_FAMILY STM32F7xx)
set(MCU_MODEL STM32F767xx)
set(CPU_PARAMETERS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard)

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/startup_stm32f767xx.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/STM32F767ZITx_FLASH.ld)
# #####################################################################################################
set(EXECUTABLE ${CMAKE_PROJECT_NAME})
enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Headers
set(CUBEMX_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/app
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include)
set(PROJECT_INCLUDE_DIRECTORIES
    ${PROJECT_DIR}
    ${PROJECT_DIR}/temp_humidity_sensor)
set(THIRD_PARTY_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/third_party/WSEN_HIDS
    ${PROJECT_DIR}/third_party/etl)

# Sources
file(GLOB_RECURSE STM32CUBEMX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/*.c)
file(GLOB_RECURSE THIRD_PARTY_SOURCES
    ${PROJECT_DIR}/third_party/*.c
    ${PROJECT_DIR}/third_party/*.cpp)
file(GLOB_RECURSE PROJECT_SOURCES
    ${PROJECT_DIR}/*.c
    ${PROJECT_DIR}/*.cpp)

# Exclude unwanted files from the sources
file(GLOB_RECURSE EXCLUDE_TEMPLATE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core/*.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/DSP/*.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/NN/*.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/RTOS/*.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/RTOS2/*.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core_A/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/WSEN_HIDS_2525020210002_EXAMPLE.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F7xx_HAL_Driver/stm32f7xx_hal_timebase_rtc_alarm_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F7xx_HAL_Driver/stm32f7xx_hal_timebase_rtc_wakeup_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F7xx_HAL_Driver/stm32f7xx_hal_timebase_tim_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F7xx_HAL_Driver/stm32f7xx_hal_msp_template.c
    )

# Remove the files from STM32CUBEMX_SOURCES
list(REMOVE_ITEM STM32CUBEMX_SOURCES ${EXCLUDE_TEMPLATE_FILES})

file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
    ${PROJECT_DIR}/*.cpp
    ${PROJECT_DIR}/*.c
    ${PROJECT_DIR}/temp_humidity_sensor/*.cpp
    ${PROJECT_DIR}/temp_humidity_sensor/*.c)

# Executable files
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES} 
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT}
    ${THIRD_PARTY_SOURCES})

# Embedded macros(defines)
target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    USE_HAL_DRIVER)

# Add header directories (AFTER add_executable !!)
target_include_directories(${EXECUTABLE} PRIVATE
    ${CUBEMX_INCLUDE_DIRECTORIES}
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${THIRD_PARTY_INCLUDE_DIRECTORIES})

# Compiler and linker options
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    $<$<COMPILE_LANGUAGE:CXX>:
        -Wno-volatile
        -Wold-style-cast
        -Wuseless-cast
        -Wsuggest-override>
    $<$<CONFIG:Debug>:-Og -g3 -ggdb>
    $<$<CONFIG:Release>:-Og -g0>)

target_link_options(${EXECUTABLE} PRIVATE
    -T${MCU_LINKER_SCRIPT}
    ${CPU_PARAMETERS}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    --specs=nosys.specs
    -Wl,--start-group
    -lc
    -lm
    -lstdc++
    -Wl,--end-group
    -Wl,--print-memory-usage)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.bin)