from pysims.pysimulators import *
import argparse
import json

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("job_name")
	parser.add_argument("metaparams")
	parser.add_argument("config")
	args = parser.parse_args()

	job_name = args.job_name
	metaparams = json.loads(args.metaparams)
	config = json.loads(args.config)

	pc = build_pc(metaparams, config)
	pc.compute(verbose = True)
	
	pc.write_json(f'{job_name}.json')
	pc.write_serialize_json(f'serialize/{job_name}_serialize.json')