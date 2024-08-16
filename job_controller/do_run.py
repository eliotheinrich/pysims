from pysims import *

from dataframe import ParallelCompute, load_data

import argparse
import dill

import os

class JobContext:
    def __init__(self, name, dir, ext, partition, nodes, ncores, memory, time, cleanup, serialize, metaparams):
        self.name = name
        self.dir = dir
        self.ext = ext
        self.partition = partition
        self.nodes = nodes
        self.ncores = ncores
        self.memory = memory
        self.time = time
        self.cleanup = cleanup
        self.serialize = serialize
        self.metaparams = metaparams

    def execute(self, job_data, *job_args):
        if isinstance(job_data, list):
            params = job_data
            configs = [prepare_config(param, self.serialize) for param in params]
            pc = ParallelCompute(configs, **self.metaparams)
            data = pc.compute()
            return data
        elif isinstance(job_data, str):
            file_name = job_data
            frame = load_data(os.path.join(self.dir, file_name))
            return self.execute(frame, *job_args)
        elif isinstance(job_data, DataFrame):
            data = job_data
            callback, = job_args
            data = resume_run(data, callback, self.metaparams, self.serialize)
            return data


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("job_name", type=str, help="Name of a job to be executed.")
    parser.add_argument("arg_file", type=str, help="Argument file for storing data for executing job.")
    args = parser.parse_args()

    job_name = args.job_name
    arg_file = args.arg_file

    with open(arg_file, "rb") as file:
        context, job_data, job_args = dill.load(file)

    data = context.execute(job_data, job_args)

    data_filename = os.path.join(context.dir, f'{job_name}.eve')
    data.write(data_filename)
