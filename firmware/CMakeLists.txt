cmake_minimum_required(VERSION 3.12)
# #####################################################################################################
# This cmake config is made for STM32 MCUs and STM32 
# file structure. Edit MCU information up to the next border
# This is based on Matej Blagsic's youtube video: https://www.youtube.com/watch?v=9tWRdnyWGb8&t=123s
# his Github: https://github.com/prtzl/Embedded_videos/tree/master/046_BUILDING-PT5-cmake/sample-f407vg
project(biltong_board_fw)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Core/app)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Core/vendor)

set(MCU_FAMILY STM32G0xx)
set(MCU_MODEL STM32G070xx)
set(CPU_PARAMETERS
    -mcpu=cortex-m0plus
    -mthumb
    #-mfpu=fpv5-sp-d16
    -mfloat-abi=soft)

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/startup_stm32g070xx.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/STM32G070CBTx_FLASH.ld)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    # Can add other includes like DSP
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 # May need to change this based on microcontroller
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS) # May need to change this line depending on cmsis version used
set(PROJECT_INCLUDE_DIRECTORIES
    # Can add other folders in the project here    
    ${PROJECT_DIR}
    ${PROJECT_DIR}/backends
    ${PROJECT_DIR}/backends/sh1122_oled)
set(THIRD_PARTY_INCLUDE_DIRECTORIES
    # Can add third party folders here
    ${VENDOR_DIR}
    ${VENDOR_DIR}/WSEN_HIDS)

# Sources
file(GLOB_RECURSE STM32CUBEMX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/*.c)
file(GLOB_RECURSE THIRD_PARTY_SOURCES
    ${VENDOR_DIR}/WSEN_HIDS/*.c
    ${VENDOR_DIR}/WSEN_HIDS/*.cpp)
file(GLOB_RECURSE PROJECT_SOURCES
    ${PROJECT_DIR}/*.c
    ${PROJECT_DIR}/*.cpp)

# Exclude unwanted files from the sources
file(GLOB_RECURSE EXCLUDE_TEMPLATE_FILES
    # Comment out which memory management .c that you WANT to keep
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_3.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c    
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_timebase_rtc_alarm_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_timebase_rtc_wakeup_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_timebase_tim_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_msp_template.c)

# Remove the files from STM32CUBEMX_SOURCES
list(REMOVE_ITEM STM32CUBEMX_SOURCES ${EXCLUDE_TEMPLATE_FILES})

file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
    #can add more files to project sources with FOLLOW_SYMLINKS option here
    ${PROJECT_DIR}/*.cpp
    ${PROJECT_DIR}/*.c
    ${PROJECT_DIR}/backends/*.cpp
    ${PROJECT_DIR}/backends/*.c
    ${PROJECT_DIR}/backends/sh1122_oled/*.cpp
    ${PROJECT_DIR}/backends/sh1122_oled/*.c)
    

# Executable files
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES} 
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT}
    ${THIRD_PARTY_SOURCES})

# Embedded macros(defines)
target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    # Add different defines, such as ARM_MATH_CM7 or __FPU_PRESENT = 1
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
    $<$<CONFIG:Debug>:-Og -g3 -ggdb -O0> #optimizations disabled, remove -O0 to enable
    $<$<CONFIG:Release>:-Og -g0 -O0>)

target_link_options(${EXECUTABLE} PRIVATE
    -T${MCU_LINKER_SCRIPT}
    ${CPU_PARAMETERS}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    #--specs=nosys.specs #this could potentially disable features of newlib we want
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