from pyinjector.config import Config
from pyinjector.controller import Controller

import sys
import subprocess

def main():
    global_cfg = Config()
    print("loading result:")
    print('injector path', global_cfg.injector_path)
    print('controller plugin', dir(Controller))
    print('load done.')

    # parse arguments
    if len(sys.argv) < 2:
        print("Usage: pyinjector <command> [args]")
        return

    command = sys.argv[1:]

    # modify command to add LD_PRELOAD hook, then exec it in sub process.
    subprocess.run(command, env={"LD_PRELOAD": global_cfg.injector_path})
    
