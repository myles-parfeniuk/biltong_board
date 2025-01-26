import argparse
import os
import shutil
import sys
import time
import subprocess
from bb_py.bb_utility import BBUtCli, BBUtPath, BBUtLog


class BBCleaner:
    def __init__(self):
        self.path_build = None

    def create_clean_command(self):
        BBUtLog.log("Creating clean commmand.")
        build_command = ["ninja", "-C", f"{self.path_build}", "-t", "clean"]
        BBUtLog.log("Clean command created successfully...:")
        BBUtLog.log(
            "     ----------------------------------------------------------")
        for item in build_command:
            BBUtLog.log(f"    {item}")
        BBUtLog.log(
            "     ---------------------------------------------------------")
        return build_command


    def run_clean_command(self, clean_command):
        try:
            BBUtLog.log("Running clean...")
            BBUtCli.run_cli_cmd(clean_command)
            BBUtLog.log("Clean completed successfully...")
        except subprocess.CalledProcessError as e:
            print(f"ERROR running clean: {e.returncode}")
            sys.exit(1)

    def run_clean(self):
        BBUtLog.reset_timer()
        BBUtLog.log(
            "######################## BBoard Clean Begin ########################")
        self.path_build = BBUtPath.get_build_dir()
        clean_command = self.create_clean_command()
        self.run_clean_command(clean_command)
        BBUtLog.log(
            "######################## BBoard Clean Fin ########################")
