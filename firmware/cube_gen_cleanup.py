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

def copy_cube_it_externs(cube_interrupt_file_path, biltong_it_file_path):
    try:
        # Collect 'extern' lines from the Cube-generated file
        with open(cube_interrupt_file_path, 'r') as input_file:
            extern_lines = [line for line in input_file if 'extern' in line.strip()]

        if not extern_lines:
            print("No 'extern' lines found to copy.")
            return

        # Read the existing content of the biltong_it.c file
        with open(biltong_it_file_path, 'r') as biltong_file:
            biltong_content = biltong_file.readlines()

        
        # delete any externs currently in biltong file before adding more
        biltong_content = [line for line in biltong_content if 'extern' not in line]

        # Find the position after the introductory comment block
        insertion_index = 0
        for i, line in enumerate(biltong_content):
            if line.strip().startswith("*/"):  # Find the end of the comment block
                insertion_index = i + 1
                break

        # Insert 'extern' lines after the comment block
        updated_content = (
            biltong_content[:insertion_index]
            + ["\n"]  # Add a blank line for separation
            + extern_lines
            + ["\n"]  # Add a blank line for readability
            + biltong_content[insertion_index:]
        )

        # Write the updated content back to the file
        with open(biltong_it_file_path, 'w') as biltong_file:
            biltong_file.writelines(updated_content)

        print(f"Successfully added {len(extern_lines)} 'extern' lines to {biltong_it_file_path}.")
    except FileNotFoundError as e:
        print(f"Error: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def delete_cube_it_file(cube_interrupt_file_path):
    if os.path.exists(cube_interrupt_file_path):
        os.remove(cube_interrupt_file_path)

remove_freertos_init(os.path.join('Core', 'Src', 'main.c'), os.path.join('Core', 'Src', 'app_freertos.c'))
copy_cube_it_externs(os.path.join('Core', 'Src', 'stm32g0xx_it.c'), os.path.join('Core', 'app', 'biltong_it.c'))
delete_cube_it_file(os.path.join('Core', 'Src', 'stm32g0xx_it.c'))
print("cleanup complete.")