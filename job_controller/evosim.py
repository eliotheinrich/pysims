from job_controller import config_to_string, submit_jobs
import numpy as np

def generate_test_config(num_trees, init_ps, num_runs, aggression_penalty=0.25, team_bonus=0.75, theft_penalty=0.5):
    config = {}
    config["circuit_type"] = "evolution"
    config["num_runs"] = num_runs

    config["num_trees"] = num_trees
    config["initial_proportion"] = init_ps
    config["aggression_penalty"] = aggression_penalty
    config["team_bonus"] = team_bonus
    config["theft_penalty"] = theft_penalty

    config["equilibration_timesteps"] = 0
    config["sampling_timesteps"] = 2000
    config["measurement_freq"] = 5
    config["temporal_avg"] = False
    
    return config
    

if __name__ == "__main__":
    num_trees = [1000]
    init_ps = list(np.linspace(0.0, 1.0, 50))
    aggression_penalty = list(np.linspace(0.1, 0.5, 20))
    aggression_penalty.append(0.25)
    config = generate_test_config(num_trees=num_trees, init_ps=init_ps, num_runs=1000, aggression_penalty=aggression_penalty)
    submit_jobs(config, f"evo2", ncores=48, memory="250gb", time="3:00:00", nodes=4)
