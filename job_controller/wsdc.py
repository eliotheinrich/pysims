from job_controller import config_to_string, submit_jobs
import numpy as np

def wsdc_config(num_judges, num_competitors, num_mutations, num_samples, num_tables, local_permutation=True):
    config = {}
    config["circuit_type"] = "wsdc_score"

    config["num_judges"] = num_judges
    config["num_competitors"] = num_competitors
    config["num_samples"] = num_samples
    config["num_mutations"] = num_mutations
    config["num_tables"] = num_tables
    
    config["local_permutation"] = local_permutation
    config["average_samples"] = True

    return config
    

if __name__ == "__main__":
    config = wsdc_config(num_judges=[3, 5, 7, 9], num_competitors=[3, 4, 5, 6, 7, 8, 9, 10], num_mutations=list(range(0, 200, 2)), num_samples=1000, num_tables=1000, local_permutation=1)
    submit_jobs(config, f"wsdc3", ncores=8, nodes=10, memory="10gb", time="10:00:00", run_local=False)

    config = wsdc_config(num_judges=[3, 5, 7, 9, 11, 13, 15], num_competitors=15, num_mutations=list(range(0, 400, 2)), num_samples=1000, num_tables=1000, local_permutation=1)
    #submit_jobs(config, f"wsdc2", ncores=8, nodes=10, memory="10gb", time="10:00:00", run_local=False)

    config = wsdc_config(num_judges=[3, 5], num_competitors=15, num_mutations=list(range(0, 3000, 2)), num_samples=1000, num_tables=1000, local_permutation=1)
    #submit_jobs(config, f"wsdc_long", ncores=8, nodes=10, memory="10gb", time="10:00:00", run_local=False)
