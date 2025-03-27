import argparse
import re
import os

from dataframe import DataFrame, load_data


def combine_data(job_name, dir, avg, num_checkpoints=None):
    if num_checkpoints is None:
        pattern = f"{job_name}_(\\d+)(?:_(\\d+))?\\.(json|eve)"
    else:
        pattern = f"{job_name}_(\\d+)(?:_{num_checkpoints})?\\.(json|eve)"

    data = DataFrame()
    total_time = []
    num_jobs = []
    num_threads = []
    num_runs = []
    for file in os.listdir(dir):
        m = re.search(pattern, file)
        if m is not None:
            print(f"{file} matches pattern")
            _data = load_data(os.path.join(dir, file))

            try:
                data += _data
                total_time.append(_data.metadata["total_time"])
                num_jobs.append(_data.metadata["num_jobs"])
                num_runs.append(_data.metadata["num_runs"])
                num_threads.append(_data.metadata["num_threads"])
            except ValueError as e:
                print(f"Could not add data for some reason")

            if avg:
                data.reduce()

    data.add_metadata("total_time", max(total_time))
    data.add_metadata("num_jobs", sum(num_jobs))
    data.add_metadata("num_threads", sum(num_threads))
    data.add_metadata("num_runs", sum(num_runs))

    return data


def main(job_name, directory, extension, avg, num_checkpoints):
    data = combine_data(job_name, directory, avg, num_checkpoints)
    data_filename = os.path.join(directory, f"{job_name}.{extension}")
    data.write(data_filename)
