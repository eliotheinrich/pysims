import subprocess
import os
import shutil
import json
import dill as pkl

from .combine_data import combine_data
from .do_run import JobContext, load_job_args, dump_job_args

from dataframe import DataFrame, unbundle_param_matrix


WORKING_DIR = os.environ["WORKING_DIR"]
DATA_DIR = os.path.join(WORKING_DIR, "data")


def save_config(config, filename):
    config = json.dumps(config, indent=1)
    config.replace('\\', '')
    with open(filename, 'w') as file:
        file.write(config)


def verify_callbacks(params, callbacks):
    _params = [p.copy() for p in params]
    for p in _params:
        for callback in callbacks:
            try:
                callback(p)
            except:
                raise RuntimeError("There was an error with the provided callback function.")
                raise


def do_run_locally(context, job_data, job_args, metaparams, num_nodes, checkpoint_callbacks):
    for i in range(num_nodes):
        data = context.execute(job_data, job_args)

        for callback in checkpoint_callbacks:
            data = context.execute(data, callback)

        data_filename = os.path.join(context.dir, f"{context.name}_{i}.{context.ext}")
        data.write(data_filename)

    data = combine_data(context.name, context.dir)
    data_filename = os.path.join(context.dir, f"{context.name}.{context.ext}")
    data.write(data_filename)
    subprocess.run(["mv", "-f", data_filename, DATA_DIR])
    mv_script = ["mv", "-f", data_filename, DATA_DIR]

    if context.cleanup:
        shutil.rmtree(context.dir)


def submit_and_get_id(script_path, dependency=None):
    try:
        command = ['sbatch']
        if dependency is not None:
            if isinstance(dependency, str) or isinstance(dependency, int):
                dependency = [dependency]
            dependency = ':'.join(dependency)
            command.append(f'--dependency=afterok:{dependency}')
        command.append(script_path)
        result = subprocess.run(command, capture_output=True, text=True, check=True)

        output = result.stdout.split()
        job_id = output[-1]

        return job_id
    except subprocess.CalledProcessError as e:
        print(f"Error submitting job: {e.stderr}")
        return None


def create_arg_file(job_name, context, job_data, job_args):
    #args = (context, job_data, job_args)
    #filename = os.path.join(context.dir, f"{job_name}.pkl")
    #with open(filename, "wb") as file:
    #    pkl.dump(args, file)
    #return filename
    filename = os.path.join(context.dir, f"{job_name}.pkl")
    dump_job_args(filename, context, job_data, job_args)
    return filename


def generate_run_script(job_name, context, arg_file):
    output_path = os.path.join(WORKING_DIR, f"slurm.{job_name}.%j.out")
    script = [
        f"#!/usr/bin/bash",
        f"#SBATCH --partition={context.partition}",
        f"#SBATCH --job-name={job_name}",
        f"#SBATCH --output={output_path}",
        f"#SBATCH --cpus-per-task={context.ncores}",
        f"#SBATCH --nodes=1 --ntasks=1",
        f"#SBATCH --mem={context.memory}",
        f"#SBATCH --time={context.time}",
        f"module load miniconda3/miniconda",
        f"conda activate test",
        f"cd {context.dir}",
        f"python -c 'from pysims.do_run import main; main(\"{job_name}\", \"{arg_file}\")'"
    ]

    return '\n'.join(script)

def do_run_slurm(context, job_data, job_args, metaparams, num_nodes, checkpoint_callbacks):
    ids = [0]*num_nodes
    for i in range(num_nodes):
        # Create scripts
        job_name_str = f"{context.name}_{i}_0"
        arg_file = create_arg_file(job_name_str, context, job_data, job_args)
        script = generate_run_script(job_name_str, context, arg_file)

        # Write scripts to files
        batch_script_filename = os.path.join(context.dir, f"{job_name_str}.sl")
        with open(batch_script_filename, 'w') as f:
            f.write(script)

        # Run script
        ids[i] = submit_and_get_id(batch_script_filename)

        for j, callback in enumerate(checkpoint_callbacks, start=1):
            job_name_str = f'{context.name}_{i}_{j}'
            checkpoint_file = os.path.join(context.dir, f'{context.name}_{i}_{j-1}.eve')
            arg_file = create_arg_file(job_name_str, context, checkpoint_file, callback)
            script = generate_run_script(job_name_str, context, arg_file)

            batch_script_filename = os.path.join(context.dir, f"{job_name_str}.sl")
            with open(batch_script_filename, 'w') as f:
                f.write(script)

            ids[i] = submit_and_get_id(batch_script_filename, dependency=ids[i])

    output_path = os.path.join(WORKING_DIR, f"slurm.{context.name}.%j.out")
    num_checkpoints = len(checkpoint_callbacks)
    combine_script = [
        f"#!/usr/bin/bash",
        f"#SBATCH --partition=shared",
        f"#SBATCH --job-name={context.name}",
        f"#SBATCH --output={output_path}"
        f"#SBATCH --cpus-per-task=1",
        f"#SBATCH --nodes=1 --ntasks=1",
        f"#SBATCH --mem=50gb",
        f"#SBATCH --time=00:30:00",

        f"module load miniconda3/miniconda",
        f"conda activate test",

        f"python -c 'from pysims.combine_data import main; main(\"{context.name}\", \"{context.dir}\", \"{context.ext}\", {num_checkpoints})'",
        f"mv -f {os.path.join(context.dir, context.name + '.' + context.ext)} {DATA_DIR}",
    ]

    if context.cleanup:
        combine_script.append(f"rm -r {context.dir}")

    combine_script = '\n'.join(combine_script)
    combine_script_batch = os.path.join(context.dir, f'{context.name}_combine.sl')
    with open(combine_script_batch, 'w') as file:
        file.write(combine_script)

    submit_and_get_id(combine_script_batch, dependency=ids)


def submit_jobs(
        job_name,
        config_generator,
        param_bundle=None,
        checkpoint_file=None,
        init_callback=None,
        cleanup=True,
        memory="5gb",
        time="24:00:00",
        ncores=1,
        partition=None,
        nodes=1,
        run_local=False,
        atol=1e-5,
        rtol=1e-5,
        parallelization_type=1,
        ext="eve",
        batch_size=1024,
        checkpoint_callbacks=None,
        verbose=True,
    ):

    if checkpoint_callbacks is None:
        checkpoint_callbacks = []

    metaparams = {
        "num_threads": ncores,

        "atol": atol,
        "rtol": rtol,

        "parallelization_type": parallelization_type,

        "batch_size": batch_size,
        "verbose": verbose,
    }

    if partition is None:
        if int(ncores) >= 48:
            partition = "exclusive"
        else:
            partition = "shared"

    case_dir = os.path.join(WORKING_DIR, f"cases/{job_name}_case")
    if os.path.exists(case_dir):
        shutil.rmtree(case_dir)
    os.mkdir(case_dir)

    context = JobContext(job_name, config_generator, case_dir, ext, partition, nodes, ncores, memory, time, cleanup, metaparams)

    if checkpoint_file is None:
        params = unbundle_param_matrix(param_bundle)
        verify_callbacks(params, checkpoint_callbacks)
        job_data = params
        job_args = None
    elif param_bundle is None:
        job_data = checkpoint_file
        job_args = init_callback

    context.test_config_generator(job_data)

    if run_local:
        do_run_locally(context, job_data, job_args, metaparams, nodes, checkpoint_callbacks)
    else:
        do_run_slurm(context, job_data, job_args, metaparams, nodes, checkpoint_callbacks)
