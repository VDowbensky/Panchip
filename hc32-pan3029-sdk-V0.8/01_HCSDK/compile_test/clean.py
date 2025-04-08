import csv
import os
import shutil
from pathlib import Path
from subprocess import PIPE, Popen


def run_build_cmd():
    try:
        currPath = Path.cwd().parent
        os.chdir(currPath)
        path_list = list(currPath.rglob("*.uvprojx"))
        finish_count = 0;
        total_count = len(path_list)
        for r_path in path_list:
            prj_path = str(Path(r_path).resolve()).replace('\\', '/')
            p_dir = Path(r_path).parent
            if p_dir.joinpath('output').exists():
                shutil.rmtree(p_dir.joinpath('output').resolve())
            if p_dir.joinpath('RTE').exists():
                shutil.rmtree(p_dir.joinpath('RTE').resolve())
            files = [file for file in p_dir.rglob("*.*")]
            for file in files:
                if not file.name.endswith('.uvprojx') and not file.name.endswith('.uvoptx'):
                    os.remove(file)
        print('finished:' + str(finish_count) + '，total:' + str(total_count))


    except Exception as e:
        print('e', e)


if __name__ == '__main__':
    run_build_cmd()
