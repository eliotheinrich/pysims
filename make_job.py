import subprocess
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("job_name")
parser.add_argument("mem")
parser.add_argument("time")
parser.add_argument("config")
args = parser.parse_args()

job_name = args.job_name
memory = args.mem
time = args.time
config_path = args.config

script = [f"#!/usr/bin/tcsh",
		  f"#SBATCH --partition=full_nodes64",
		  f"#SBATCH --job-name={job_name}    # Job name",
		  f"#SBATCH  --ntasks 1 --cpus-per-task 30",
		  f"#SBATCH --mem={memory}                     # Job memory request",
		  f"#SBATCH --time={time}               # Time limit hrs:min:sec",
		  f"cd ~/cliffordsim/bin/tmp",
		  f"./main_var /data/heinriea/cliffordsim/{config_path} 30"]

script = '\n'.join(script)

with open('job_tmp.sl', 'w') as f:
	f.write(script)

subprocess.run(["sbatch", "job_tmp.sl"])
subprocess.run(["rm", "job_tmp.sl"])
