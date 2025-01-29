import argparse
from bb_py.bb_setup import BBBuildSetup
from bb_py.bb_build_manager import BBBuildMgr
from bb_py.bb_scruber import BBScrubber

class BBoardTools:
    def __init__(self):
        self.bb_bsetup = BBBuildSetup()
        self.bb_bmgr = BBBuildMgr()
        self.bb_scrubber = BBScrubber()

    def add_args(self, parser):
        parser.add_argument("-buildsetup", "--buildsetup", action="store_true",
                            help="Run build setup.")
        parser.add_argument("-build", "--build", action="store_true",
                            help="Run build.")
        parser.add_argument("-clean", "--clean", action="store_true",
                            help="Run clean.")
        parser.add_argument("-cubescrub", "--cubescrub", action="store_true",
                    help="Run clean.")
        
        BBBuildSetup.add_args(parser)


    def execute_command(self, parser):
        args = parser.parse_args()

        if args.cubescrub:
            self.bb_scrubber.run_cubeHAL_scrub()

        if args.buildsetup:
            [build_type, build_tests] = BBBuildSetup.parse_args(args)
            self.bb_bsetup.run_build_setup(build_type, build_tests)
        
        if args.build:
            self.bb_bmgr.run_build()    
        elif args.clean:
            self.bb_bmgr.run_clean()


def main():
    parser = argparse.ArgumentParser()
    bb_tools = BBoardTools()
    bb_tools.add_args(parser)
    bb_tools.execute_command(parser)


if __name__ == "__main__":
    main()
