import subprocess
import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument("config")
parser.add_argument("--ncores", nargs=1, default=[1])
parser.add_argument("--output", nargs=1, default=["output.svg"])
parser.add_argument("--compile", nargs=1, default=["True"])
parser.add_argument("--executable", nargs=1, default=["maind"])
parser.add_argument("--cutoff", nargs=1, default=[1.0])
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
cutoff = args.cutoff[0]



compile_script = [
    f"cd buildp",
    f"cmake -DCMAKE_CXX_FLAGS=\"-pg\" -DCMAKE_BUILD_TYPE=Debug ..",
    f"make main -j{ncores}",
    f"cd ..",
]

extension = output_file.split('.')[1]

script = [
        f"./bin/{executable} {config_path} {ncores}",
        f"gprof bin/{executable} | python gprof2dot.py -n {cutoff} -s | dot -T{extension} -o {output_file}",
        "rm gmon.out",
    ]

if compile:
    script = compile_script + script
    if not os.path.exists('buildp'):
        os.mkdir('buildp')


subprocess.run(["bash", "-c", "\n".join(script)])

