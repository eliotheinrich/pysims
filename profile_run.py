import subprocess
import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument("config")
parser.add_argument("--ncores", nargs=1, default=[1])
parser.add_argument("--output", nargs=1, default=["output.png"])
parser.add_argument("--compile", nargs=1, default=["True"])
parser.add_argument("--executable", nargs=1, default=["bin/main"])
args = parser.parse_args()

def parse_bool(p: str) -> bool:
	if p == "True":
		return True
	elif p == "False":
		return False
	else:
		raise ValueError(f"{p} cannot be parsed as a boolean; it must be \"True\" or \"False\".")

config_path = args.config
ncores = args.ncores[0]
output_file = args.output[0]
compile = parse_bool(args.compile[0])
executable = args.executable[0]



compile_script = [
    f"cd buildp",
	f"cmake -DCMAKE_CXX_FLAGS=\"-pg\" ..",
 	f"make {executable.split('/')[1]}",
	f"cd ..",	
]

script = [
	f"./{executable} {config_path} {ncores}",
	f"gprof bin/main | python gprof2dot.py | dot -Tpng -o {output_file}",
	"rm gmon.out",
]

if compile:
	script = compile_script + script
	if not os.path.exists('buildp'):
		os.mkdir('buildp')


subprocess.run(["bash", "-c", "\n".join(script)])