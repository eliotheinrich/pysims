from pysims.pysimulators import *
import subprocess
import os
import shutil
import json

def save_config(config, filename):
    config = json.loads(config)
    config["filename"] = 'default_data.json'
    config = json.dumps(config)
    config.replace('\\', '') 
    with open(filename, 'w') as file:
        file.write(config)

def submit_jobs(
        params, 
        job_name,
        cleanup=True,
        memory="5gb", 
        time="24:00:00", 
        ncores=64, 
        ncores_per_task=1, 
        partition="default",
        nodes=1,
        run_local=False,
        atol=1e-5,
        rtol=1e-5,
        serialize=False,
        average_congruent_runs=True,
        record_error=True,
        parallelization_type=0
    ):

    metaparams = {
        "num_threads": ncores,
        "num_threads_per_task": ncores_per_task,
        
        "atol": atol,
        "rtol": rtol,
         
        "average_congruent_runs": average_congruent_runs,
        "parallelization_type": parallelization_type,
        
        "serialize": serialize,
        "record_error": record_error
    }
    
    
    if partition == "default":
        if int(ncores) >= 48:
            partition = "exclusive"
        else:
            partition = "shared"

    if isinstance(params, str):
        config = params
    else:
        config = write_config(params)
    
    
    config = "".join(config.split())
    
    # Setup
    cwd = os.getcwd()
    case_dir = os.path.join(cwd, f'{job_name}_case')
    if os.path.exists(case_dir):
        shutil.rmtree(case_dir)
    os.mkdir(case_dir)
    
    if run_local:
        os.chdir(f'{case_dir}')
        for i in range(nodes):
            script = ["python", "../do_run.py", f"{job_name}_{i}", f"{json.dumps(metaparams)}", f"{json.dumps(config)}"]

            subprocess.run(script)
        
        

        combine_script = ["python", "../combine_data.py", job_name, str(record_error)]
        subprocess.run(combine_script)
        subprocess.run(["mv", "-f", f"{job_name}.json", ".."])
        if cleanup:
            shutil.rmtree(case_dir)
    else:
        dependencies = []
        for i in range(nodes):
            script = [
                f"#!/usr/bin/tcsh",
                f"#SBATCH --partition={partition}",
                f"#SBATCH --job-name={job_name}",
                f"#SBATCH --cpus-per-task={ncores}",
                f"#SBATCH --nodes=1 --ntasks=1",
                f"#SBATCH --mem={memory}",
                f"#SBATCH --time={time}",
                
                f"module load anaconda/2021.11-p3.9",
                f"conda activate test",
                f"cd {case_dir}",
                
                f"python ../do_run.py {job_name}_{i} '{json.dumps(metaparams)}' '{json.dumps(config)}'",
            ]

            script = '\n'.join(script)
            job_name_str = os.path.join(case_dir, f'{job_name}_{i}.sl')
            with open(job_name_str, 'w') as f:
                f.write(script)

            # Replace s
            sbatch_process = subprocess.run(["sbatch", job_name_str], capture_output=True, text=True)
            sbatch_output = sbatch_process.stdout
            jobid = sbatch_output.strip().split()[-1]
            dependencies.append(jobid)
            subprocess.run(["rm", job_name_str])

        combine_script = [
            f"#!/usr/bin/tcsh",
            f"#SBATCH --partition=shared",
            f"#SBATCH --job-name={job_name}",
            f"#SBATCH --cpus-per-task=1",
            f"#SBATCH --nodes=1 --ntasks=1",
            f"#SBATCH --mem=50gb",
            f"#SBATCH --time=00:30:00",

            f"module load anaconda/2021.11-p3.9",
            f"conda activate test",
            f"cd {case_dir}",
            
            f"python ../combine_data.py {job_name} {record_error}",
            f"mv -f {job_name}.json ..",
        ]
        
        if cleanup:
            combine_script.append(f"rm -r {case_dir}")
        
        combine_script = '\n'.join(combine_script)
        job_name_str = os.path.join(case_dir, f'{job_name}.sl')
        with open(job_name_str, 'w') as f:
            f.write(combine_script)

        subprocess.run(["sbatch", f"--dependency=singleton", job_name_str])

def field_to_string(field) -> str:
    if isinstance(field, str):
        return f'"{field}"'
    elif isinstance(field, bool):
        return 'true' if field else 'false'
    else:
        try:
            iterator = iter(field)
            return '[' + ', '.join([field_to_string(i) for i in iterator]) + ']'
        except TypeError:
            pass
        
        return str(field)

def config_to_string(config: dict) -> str:
    s = "{\n"
    lines = []
    for key, val in config.items():
        if key[:7] == 'zparams':
            v = '[' + ', '.join([config_to_string(p).replace('\n', '').replace('\t', '').replace(',', ', ').replace('\'', '"') for p in val]) + ']'
        else:
            v = field_to_string(val)
        lines.append(f"\t\"{key}\": {v}")
    s += ',\n'.join(lines) + '\n}'
    
    return s.replace('\'', '"')