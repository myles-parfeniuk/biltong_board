"""
========================================================================== 
* @file           : cube_gen_cleanup.py
* @brief          : Script to cleanup codebase after CubeMX code generation.
*
* @details        : This script is to remove unwanted auto-generated code from 
                    CubeMX that cannot be disabled (for ex. unwanted freeRTOS
                    boilerplate code like the default task).
*
* @author         : Myles Parfeniuk
* @date           : 2024-10-23
*
* Usage:
* Execute from root firmware directory after CubeMX code generation session.
* Ensure to run build.bat after executing this script, not before. 
========================================================================== 
"""
import os

def remove_freertos_init(main_filepath, freertos_app_filepath):
    #unwanted CubeMX CMSIS garbage
    lines_to_remove = [
        "void MX_FREERTOS_Init(void);\n",
        "  /* Call init function for freertos objects (in cmsis_os2.c) */\n",
        "  MX_FREERTOS_Init();\n",
        "  /* Start scheduler */\n",
        "  osKernelStart();\n"
    ]

    #open auto-generated main.c
    with open(main_filepath, 'r') as file:
        content = file.readlines()

    #filter lines to remove
    new_content = [line for line in content if line not in lines_to_remove]

    #write the modified content back to the file
    with open(main_filepath, 'w') as file:
        file.writelines(new_content)

    #entire file of unwanted CubeMX CMSIS garbage
    if os.path.exists(freertos_app_filepath):
        os.remove(freertos_app_filepath)

remove_freertos_init(os.path.join('Core', 'Src', 'main.c'), os.path.join('Core', 'Src', 'freertos.c'))
print("cleanup complete.")