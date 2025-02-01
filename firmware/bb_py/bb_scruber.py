import argparse
import os
import sys
import time
import subprocess
from bb_py.bb_utility import BBUtCli, BBUtPath, BBUtLog, BBUtCodeScrubber

class BBScrubber: 
    def __init__(self):
        self.path_project = None


    def cleanup_main_c(self):
        path_main_c = os.path.join(self.path_project, 'Core', 'Src', 'main.c')
        BBUtCodeScrubber.cleanup_fxn(path_main_c, "osKernelStart")
        BBUtCodeScrubber.cleanup_fxn(path_main_c, "MX_FREERTOS_Init")


    def cleanup_app_freertos_c(self):

        path_app_freertos_c = os.path.join(self.path_project, 'Core', 'Src', 'app_freertos.c')
        BBUtCodeScrubber.cleanup_fxn(path_app_freertos_c, "MX_FREERTOS_Init")
        BBUtCodeScrubber.cleanup_fxn(path_app_freertos_c, "StartDefaultTask")
        BBUtCodeScrubber.cleanup_references(
            path_app_freertos_c, 'defaultTaskHandle')
    
    def delete_ISR_files(self):
        path_stm32xxxx_it_c = os.path.join(self.path_project, 'Core', 'Src', 'stm32g0xx_it.c')
        path_stm32xxxx_it_h = os.path.join(self.path_project, 'Core', 'Inc', 'stm32g0xx_it.h')

        files_to_delete =  [path_stm32xxxx_it_c, path_stm32xxxx_it_h]

        for file_path in files_to_delete:
            try:
                os.remove(file_path)
                BBUtLog.log(f"Successfully deleted: {file_path}")
            except FileNotFoundError:
                BBUtLog.log(f"File not found: {file_path}")
            except PermissionError:
                print(f"Permission denied: {file_path}")
            except Exception as e:
                BBUtLog.log(f"Error deleting {file_path}: {str(e)}")[2]


    def cleanup_cmsis_wrapper(self):
        BBUtLog.log("Removing freeRTOS CMSIS wrapper references from main.c...")
        self.cleanup_main_c()
        BBUtLog.log("Removing freeRTOS CMSIS wrapper references from app_freertos.c...")
        self.cleanup_app_freertos_c()


    def run_cubeHAL_scrub(self):
        BBUtLog.reset_timer()
        BBUtLog.log(
            "######################## BBoard CubeHAL Scrub Begin ########################")
        self.path_project = BBUtPath.get_project_dir()
        BBUtLog.log("Removing freeRTOS CMSIS wrapper...")
        self.cleanup_cmsis_wrapper()
        BBUtLog.log("Deleting stm32xxxx_it.c & .h, ensure any new handlers have been moved to your implementation.")
        self.delete_ISR_files()
        BBUtLog.log(
            "######################## BBoard CubeHAL Scrub Fin ########################")
