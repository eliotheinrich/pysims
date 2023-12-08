import argparse
import re
import os

from dataframe import DataFrame

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("job_name")
	parser.add_argument("record_error")
	args = parser.parse_args()

	job_name = args.job_name
	record_error = args.record_error
	if record_error == "True":
		record_error = True
	elif record_error == "False":
		record_error = False
	else:
		raise ValueError("record_error must be a bool (True or False).")

	pattern = f'{job_name}_(\d+)\.json'
	cwd = os.getcwd()
	
	data = []
	for file in os.listdir():
		if not os.path.isfile(os.path.join(cwd, file)):
			continue
		m = re.search(pattern, file)
		if m is not None:
			i = m.group(1)
			with open(file, 'r') as f:
				data.append(DataFrame(f.read()))

	df = sum(data, start = DataFrame())
	df.add_metadata('total_time', max([d['total_time'] for d in data]))
	df.add_metadata('num_jobs', sum([d['num_jobs'] for d in data]))
	df.add_metadata('num_threads', sum([d['num_threads'] for d in data]))
	df.write_json(f'{job_name}.json', record_error)