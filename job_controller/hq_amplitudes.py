from job_controller import config_to_string, submit_jobs
import numpy as np

def generate_test_config(k, num_samples=0, p1=0.5, p2=0.5, p3=0.5, calculation_type=0, num_polynomials=1):
    config = {}
    config["circuit_type"] = "hq_circuit"
    config["num_runs"] = 1
    
    config["k"] = k
    config["p1"] = p1
    config["p2"] = p2
    config["p3"] = p3

    config["num_samples"] = num_samples
    config["calculation_type"] = calculation_type
    config["num_polynomials"] = num_polynomials

    return config

if __name__ == "__main__":
    config = generate_test_config(k=2, num_samples=[0, 300000], calculation_type=[1], p1=0.5, p2=0.5, p3=0.5, num_polynomials=1)
    submit_jobs(config, f"hq_sampling", ncores=1, memory="1gb", time="01:00:00", nodes=1, run_local=True, verbose=False)
