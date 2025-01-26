import argparse
import os
import shutil
import sys
import time
import subprocess
from bb_py.bb_utility import BBUtCli, BBUtPath, BBUtLog


class BBBuilder:
    def __init__(self):
        self.path_build = None

    def create_build_command(self):
        BBUtLog.log("Creating build commmand.")
        build_command = ["ninja", "-C", f"{self.path_build}", "-j8"]
        BBUtLog.log("Build command created successfully...:")
        BBUtLog.log(
            "     ----------------------------------------------------------")
        for item in build_command:
            BBUtLog.log(f"    {item}")
        BBUtLog.log(
            "     ---------------------------------------------------------")
        return build_command


    def run_build_command(self, build_command):
        try:
            BBUtLog.log("Running build...")
            BBUtCli.run_cli_cmd(build_command)
            BBUtLog.log("Build completed successfully...")
        except subprocess.CalledProcessError as e:
            print(f"ERROR running build: {e.returncode}")
            sys.exit(1)

    def run_build(self):
        BBUtLog.reset_timer()
        BBUtLog.log(
            "######################## BBoard Build Begin ########################")
        self.path_build = BBUtPath.get_build_dir()
        build_command = self.create_build_command()
        self.run_build_command(build_command)
        BBUtLog.log(
            "######################## BBoard Build Fin ########################")
