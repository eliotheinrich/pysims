from pysims import *

from dataframe import ParallelCompute, parse_config

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
	params = parse_config(configs)
	
	configs = [prepare_config(param) for param in params]
 
	pc = ParallelCompute(configs, **metaparams)

	data = pc.compute()
	data.write(f'{job_name}.eve')