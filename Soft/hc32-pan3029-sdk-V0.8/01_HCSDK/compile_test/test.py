import csv
import datetime
import os
from pathlib import Path
from subprocess import PIPE, Popen

KEIL_PATH = 'C:/keil_v5/UV4/UV4.exe'


def write_result_file(file_name, row):
    headers = ["examples", 'complie result', 'errors', 'warning']
    file_exsit = True
    try:
        if not os.path.exists("./" + file_name):
            file_exsit = False
        with open( file_name, 'a', newline='') as f:
            f_csv = csv.writer(f)
            if not file_exsit:
                f_csv.writerow(headers)
            f_csv.writerow(row)
    except Exception as e:
        print('e', e)


def save_log(log_file_name,sample_name, p):
    with p.open() as f:
        lines = f.readlines()
    com_result = ''
    err_count = '0'
    warn_count = '0'
    is_result_return = False
    for line in lines:
        # print('line', line)
        if 'Error(s)' in line:
            strs = line.split(' ')
            err_count = strs[2]
            warn_count = strs[4]
            is_result_return = True
    is_success = is_result_return and err_count == '0'
    com_result = '成功' if is_success else '失败'
    if sample_name and len(sample_name) > 0:
        write_result_file(log_file_name, [sample_name, com_result, err_count, warn_count])
    return is_success,sample_name


def run_build_cmd():
    try:
        currPath = Path.cwd().parent
        os.chdir(currPath)
        path_list = list(currPath.rglob("*.uvprojx"))
        finish_count = 0;
        success_count = 0
        csv_file_name = 'result_' + datetime.datetime.now().strftime('%m_%d_%H_%M_%S_%f') + '.csv'
        total_count = len(path_list)
        for r_path in path_list:
            prj_path = str(Path(r_path).resolve()).replace('\\', '/')
            cmd = "\"" + KEIL_PATH + "\" -j4 -r \"" + prj_path + "\" -o ./build_log.txt "
            print("start to compile:" + prj_path);
            process = Popen(cmd, shell=False, close_fds=True, stdin=PIPE)
            process.wait()

            finish_count += 1

            log_file = Path(r_path).parent.joinpath('build_log.txt')
            if not log_file.exists():
                log_file.touch()

            is_success ,sample_name= save_log(csv_file_name,Path(r_path).parent.parent.name, log_file)
            
            result_str = '失败'
            if is_success:
                success_count += 1
                result_str = '成功'
                
            print('result:'+result_str+'，success:' + str(success_count) + '，finished:' + str(finish_count) + '，total:' + str(total_count)+"，"+sample_name)

    except Exception as e:
        print('e', e)


if __name__ == '__main__':
    run_build_cmd()
