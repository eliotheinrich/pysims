import subprocess
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("job_name")
parser.add_argument("config")
parser.add_argument("--executable", nargs=1, default=["main"])
parser.add_argument("--mem", nargs=1, default=["5gb"])
parser.add_argument("--time", nargs=1, default=["24:00:00"])
parser.add_argument("--ncores", nargs=1, default=[64], type=int)
parser.add_argument("--ncores-per-task", nargs=1, default=[""])
parser.add_argument("--partition", nargs=1, default=["default"])
parser.add_argument("--ompi", nargs=1, default=[False])
parser.add_argument("--serial", nargs=1, default=[False])
args = parser.parse_args()

job_name = args.job_name
config_path = args.config
memory = args.mem[0]
time = args.time[0]
ncores = args.ncores[0]
ncores_per_task = args.ncores_per_task[0]
executable = args.executable[0]
partition = args.partition[0]
ompi = args.ompi[0]
serial = args.serial[0]

if int(ncores) > 64 and not ompi:
	print("Cannot run a job with more than 64 CPU cores.")
	raise ValueError

if partition == "default":
	if int(ncores) >= 48:
		partition = "exclusive"
	else:
		partition = "shared"


if ompi:
	script = [f"#!/usr/bin/tcsh",
			  f"#SBATCH --partition={partition}",
			  f"#SBATCH --job-name={job_name}    # Job name",
			  f"#SBATCH --nodes=1",
			  f"#SBATCH --ntasks-per-node={ncores}",
			  f"#SBATCH --mem={memory}                     # Job memory request",
			  f"#SBATCH --time={time}               # Time limit hrs:min:sec",
			  f"module load openmpi/4.1.1-intel.2020",
                          f"module load gcc/11.2.0",
			  f"cd ~/cliffordsim/buildt",
			  f"make clean",
			  f"cmake .. -DOMPI=ON -DCMAKE_PREFIX_PATH=/usr/public/openmpi/4.1.1-intel.2020",
			  f"make main",
			  f"cd ../stable",
			  f"rm {executable}",
			  f"cp ../bin/main {executable}",
			  f"/usr/public/openmpi/4.1.1-intel.2020/bin/mpiexec -np {ncores} {executable} /data/heinriea/cliffordsim/{config_path} {ncores_per_task}"]
else:
	script = [f"#!/usr/bin/tcsh",
			  f"#SBATCH --partition={partition}",
			  f"#SBATCH --job-name={job_name}    # Job name",
			  f"#SBATCH --ntasks 1 --cpus-per-task {ncores}",
			  f"#SBATCH --mem={memory}                     # Job memory request",
			  f"#SBATCH --time={time}               # Time limit hrs:min:sec",
                          f"module load gcc/11.2.0",
			  f"cd ~/cliffordsim/stable",
			  f"./{executable} /data/heinriea/cliffordsim/{config_path} {ncores} {ncores_per_task}"]

script = '\n'.join(script)

with open('job_tmp.sl', 'w') as f:
	f.write(script)

# Replace srun?
subprocess.run(["sbatch", "job_tmp.sl"])
subprocess.run(["rm", "job_tmp.sl"])
