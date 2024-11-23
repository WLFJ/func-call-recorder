from pyinjector.config import Config
from pyinjector.controller import Controller

import sys
import subprocess
import os


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

    # get current process enviroment, inorder to patch and exec sub process.
    env = dict()
    for k, v in os.environ.items():
        env[k] = v

    # patch env
    env["LD_PRELOAD"] = global_cfg.injector_path

    print("running env:", env)

    # modify command to add LD_PRELOAD hook, then exec it in sub process.
    subprocess.run(command, env=env)
