from pysims import *

from dataframe import ParallelCompute, load_json

import argparse
import json

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("job_name")
	parser.add_argument("metaparams")
	parser.add_argument("configs")
	args = parser.parse_args()

	job_name = args.job_name
	metaparams = json.loads(args.metaparams)
	configs = json.loads(args.configs)
	params = load_json(configs)
	
	configs = [prepare_config(param) for param in params]
 
	pc = ParallelCompute(configs, **metaparams)

	pc.compute(verbose = True)
	pc.write_json(f'{job_name}.json')