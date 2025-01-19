cmake_minimum_required(VERSION 3.12)

option(BUILD_TESTS "Enable testing with Unity" OFF)


#### PROJECT SETUP ####
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
set(EXECUTABLE ${CMAKE_PROJECT_NAME})
enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
#### PROJECT SETUP END ####

#### INCLUDE DIRECTORIES ####
set(CUBEMX_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include)

set(FREERTOS_INCLUDE_DIRECTORIES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 # May need to change this based on microcontroller
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS) # May need to change this line depending on cmsis version used

set(PROJECT_INCLUDE_DIRECTORIES
    # Can add other folders in the project here    
    ${PROJECT_DIR}
    ${PROJECT_DIR}/device
    ${PROJECT_DIR}/backends
    ${PROJECT_DIR}/backends/sh1122_oled
    ${PROJECT_DIR}/backends/switch_driver
    ${PROJECT_DIR}/backends/temp_rh_sens)

#third party includes
set(WSEN_HIDS_INCLUDE_DIRECTORIES
    ${VENDOR_DIR}/WSEN_HIDS)
    
set(ETL_INCLUDE_DIRECTORIES
    ${VENDOR_DIR}/etl)

#### INCLUDE DIRECTORIES END ####

#### SOURCES ####
file(GLOB_RECURSE STM32CUBEMX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/*.c)

file(GLOB_RECURSE FREERTOS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/*.c)

    #third party sources
file(GLOB_RECURSE WSEN_HIDS_SOURCES
    ${VENDOR_DIR}/WSEN_HIDS/*.c)

    #project sources
file(GLOB_RECURSE PROJECT_SOURCES
    ${PROJECT_DIR}/*.c
    ${PROJECT_DIR}/*.cpp)

file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
    #can add more files to project sources with FOLLOW_SYMLINKS option here
    ${PROJECT_DIR}/*.cpp
    ${PROJECT_DIR}/*.c
    ${PROJECT_DIR}/device/*.cpp
    ${PROJECT_DIR}/device/*.c
    ${PROJECT_DIR}/backends/*.cpp
    ${PROJECT_DIR}/backends/*.c
    ${PROJECT_DIR}/backends/sh1122_oled/*.cpp
    ${PROJECT_DIR}/backends/sh1122_oled/*.c
    ${PROJECT_DIR}/backends/switch_driver/*.cpp
    ${PROJECT_DIR}/backends/switch_driver/*.c
    ${PROJECT_DIR}/backends/temp_rh_sens/*.cpp
    ${PROJECT_DIR}/backends/temp_rh_sens/*.c)
#### SOURCES END ####

#### SOURCE EXCLUSIONS ####
file(GLOB_RECURSE EXCLUDE_STM32_CUBEMX_TEMPLATE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_timebase_rtc_alarm_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_timebase_rtc_wakeup_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_timebase_tim_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_msp_template.c)

file(GLOB_RECURSE EXCLUDE_FREERTOS_MEMMANG_FILES 
    # Comment out which memory management .c that you WANT to keep
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_3.c
    #${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c   
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_5.c
    )


list(REMOVE_ITEM STM32CUBEMX_SOURCES ${EXCLUDE_STM32_CUBEMX_TEMPLATE_FILES})
list(REMOVE_ITEM FREERTOS_SOURCES ${EXCLUDE_FREERTOS_MEMMANG_FILES})
### SOURCE EXCLUSIONS END ####
    

### ADD EXECUTABLE ####
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES} 
    ${FREERTOS_SOURCES}
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT}
    ${WSEN_HIDS_SOURCES})
### ADD EXECUTABLE END ####

### COMPILE DEFINITIONS ####
target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    # Add different defines, such as ARM_MATH_CM7 or __FPU_PRESENT = 1
    USE_HAL_DRIVER)
### COMPILE DEFINITIONS END ####

### ADD INCLUDES TO EXECUTABLE ###
target_include_directories(${EXECUTABLE} PRIVATE
    ${CUBEMX_INCLUDE_DIRECTORIES}
    ${FREERTOS_INCLUDE_DIRECTORIES}
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${ETL_INCLUDE_DIRECTORIES}
    ${WSEN_HIDS_INCLUDE_DIRECTORIES})
### ADD INCLUDES TO EXECUTABLE END ###

#### SUBDIRECTORIES ####
add_subdirectory(Core/vendor/OPEEngine)

if(BUILD_TESTS)
    add_subdirectory(Core/test)
endif()
#### SUBDIRECTORIES END ####

#### LIBRARY LINKING ####
target_link_libraries(${EXECUTABLE} PRIVATE OPEEngine)
### LIBRARY LINKING END ####

### COMPILER AND LINKER SETTINGS ####
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
### COMPILER AND LINKER SETTINGS END ####

### CUSTOM COMPILATION SETTINGS AND COMMANDS ####

set_source_files_properties(${STM32CUBEMX_SOURCES} PROPERTIES COMPILE_FLAGS "-w")
set_source_files_properties(${FREERTOS_SOURCES} PROPERTIES COMPILE_FLAGS "-w")
set_source_files_properties(${WSEN_HIDS_SOURCES} PROPERTIES COMPILE_FLAGS "-w")

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.bin)
### CUSTOM COMPILATION SETTINGS AND COMMANDS END ####