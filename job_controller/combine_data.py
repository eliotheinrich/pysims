import argparse
import re
import os

from dataframe import DataFrame, load_data

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("job_name")
    parser.add_argument("extension")
    args = parser.parse_args()

    job_name = args.job_name
    ext = args.extension

    pattern = f'{job_name}_(\d+)\.(json|eve)'
    cwd = os.getcwd()

    data = []
    for file in os.listdir():
        if not os.path.isfile(os.path.join(cwd, file)):
            continue
        m = re.search(pattern, file)
        if m is not None:
            i = m.group(1)
            with open(file, 'r') as f:
                data.append(load_data(file))

    df = DataFrame()
    df = sum(data, start=DataFrame())

    df.add_metadata('total_time', max([d['total_time'] for d in data]))
    df.add_metadata('num_jobs', sum([d['num_jobs'] for d in data]))
    df.add_metadata('num_threads', sum([d['num_threads'] for d in data]))
    df.write(f'{job_name}.{ext}')
