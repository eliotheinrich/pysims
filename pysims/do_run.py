from dataframe import compute, load_data, DataFrame

import dill as pkl
import os
import sys
import subprocess
import inspect


def dump_job_args(filename, *args):
    with open(filename, "wb") as file:
        pkl.dump(args, file)


def load_job_args(filename):
    with open(filename, "rb") as file:
        data = pkl.load(file)
    return data


def make_config(config_generator_script, config_generator_name, params):
    script = [
        f"import numpy as np",
        f"{config_generator_script}",
        f"config = {config_generator_name}({params})"
    ]
    locals = {}
    exec('\n'.join(script), globals(), locals)
    return locals["config"]


def resume_run(config_generator, config_generator_name, frame, callback=None, metaparams=None):
    if callback is None:
        def callback(x):
            return

    configs = []
    for slide in frame.slides:
        params = {**frame.params, **slide.params}
        config = make_config(config_generator, config_generator_name, params)
        callback(config.params)
        buffer = slide._get_buffer()
        config.inject_buffer(buffer)
        configs.append(config)

    if metaparams is None:
        metaparams = frame.metadata

    new_frame = compute(configs, **metaparams)

    num_slides = len(frame.slides)
    for n in range(num_slides):
        new_frame.slides[n].params = frame.slides[n].params
        new_frame.slides[n].combine_data(frame.slides[n])
    return new_frame


class JobContext:
    def __init__(self, name, config_generator, dir, ext, partition, nodes, ncores, memory, time, cleanup, metaparams):
        self.name = name
        self.config_generator = inspect.getsource(config_generator)
        self.config_generator_name = config_generator.__name__
        self.dir = dir
        self.ext = ext
        self.partition = partition
        self.nodes = nodes
        self.ncores = ncores
        self.memory = memory
        self.time = time
        self.cleanup = cleanup
        self.metaparams = metaparams

        if self.config_generator_name == "<lambda>":
            raise RuntimeError("Cannot provide a lambda function as config_generator.")


    def execute(self, job_data, *job_args):
        if isinstance(job_data, list):
            params = job_data
            configs = [make_config(self.config_generator, self.config_generator_name, param) for param in params]
            return compute(configs, **self.metaparams)
        elif isinstance(job_data, str):
            file_name = job_data
            frame = load_data(os.path.join(self.dir, file_name))
            return self.execute(frame, *job_args)
        elif isinstance(job_data, DataFrame):
            # Checkpoint file
            data = job_data
            callback, = job_args
            return resume_run(self.config_generator, self.config_generator_name, data, callback, self.metaparams)

    def get_params(self, job_data):
        if isinstance(job_data, list):
            return job_data
        elif isinstance(job_data, str):
            frame = load_data(os.path.join(self.dir, job_data))
            return self.get_params(frame)
        elif isinstance(job_data, DataFrame):
            return [slide.params for slide in job_data.slides]

    def test_config_generator(self, job_data):
        params = self.get_params(job_data)

        filename = os.path.join(self.dir, f"{self.name}_test.pkl")
        try:
            with open(filename, "wb") as file:
                args = (self.config_generator, self.config_generator_name)
                pkl.dump(args, file)

            unpickle_script = [
                 "import dill as pkl",
                 "import os",
                 "import numpy as np",
                 "from pysims import make_config",
                 "os.chdir(os.path.expanduser('~'))",
                f"with open('{filename}', 'rb') as file:",
                 "  generator, name = pkl.load(file)",
                f"  config = make_config(generator, name, {params[0]})"
            ]

            result = subprocess.run([sys.executable, "-c", "\n".join(unpickle_script)], capture_output=True, text=True)

            if result.returncode != 0:
                error_message = result.stderr.strip()
                print(result.stdout.strip())
                print(error_message)
                raise RuntimeError(f"Error unpickling config_generator. Maybe you forgot to call cloud_pickle.register_pickle_by_value on a module?")
        finally:
            os.remove(filename)


def main(job_name, arg_file):
    context, job_data, job_args = load_job_args(arg_file)

    data = context.execute(job_data, job_args)

    data_filename = os.path.join(context.dir, f"{job_name}.eve")
    data.write(data_filename)
