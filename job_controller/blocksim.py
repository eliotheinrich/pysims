from job_controller import config_to_string, submit_jobs, save_config
import numpy as np

def blocksim_samples_config(
        feedback_mode, 
        us, 
        nruns=25, 
        system_sizes=[128], 
        equilibration_timesteps=1000,
        sampling_timesteps=2000,
        measurement_freq=5,
        sample_surface=False
    ):

    config = {}
    config["circuit_type"] = "blocksim"
    config["num_runs"] = nruns

    config["system_size"] = system_sizes

    config["save_samples"] = True

    config["feedback_mode"] = feedback_mode
    
    config["random_sites"] = True
    config["precut"] = [True]
    config["depositing_type"] = [1]

    config["sample_surface"] = sample_surface
    config["sample_surface_avg"] = False
    config["sample_rugosity"] = False
    config["sample_roughness"] = False
    config["sample_avalanche_sizes"] = False
    config["sample_structure_function"] = False
    config["sample_edges"] = True

    probs = []
    for u in us:
        probs_u = {}
        if u < 1.0:
            probs_u['pu'] = u
            probs_u['pm'] = 1.0
        else:
            probs_u['pu'] = 1.0
            probs_u['pm'] = 1.0/u
        probs.append(probs_u)
    
    config["zparams"] = probs

    config["temporal_avg"] = False
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = measurement_freq

    return config

def blocksim_time_evolution(
        feedback_mode, 
        us, 
        nruns=25, 
        system_sizes=[128], 
        avalanche_type=0,
        delta=2.0,
        equilibration_timesteps=1000,
        sampling_timesteps=2000,
        measurement_freq=5,
        sample_surface=False,
    ):

    config = {}
    config["circuit_type"] = "blocksim"
    config["num_runs"] = nruns

    config["system_size"] = system_sizes

    config["feedback_mode"] = feedback_mode
    
    config["random_sites"] = True
    config["precut"] = [True]
    config["depositing_type"] = [1]
    config["avalanche_type"] = avalanche_type
    config["delta"] = delta

    config["sample_surface"] = sample_surface
    config["sample_surface_avg"] = False
    config["sample_rugosity"] = False
    config["sample_roughness"] = False
    config["sample_avalanche_sizes"] = True 
    config["sample_structure_function"] = False
    config["sample_edges"] = True

    probs = []
    for u in us:
        probs_u = {}
        if u < 1.0:
            probs_u['pu'] = u
            probs_u['pm'] = 1.0
        else:
            probs_u['pu'] = 1.0
            probs_u['pm'] = 1.0/u
        probs.append(probs_u)
    
    config["zparams"] = probs

    config["temporal_avg"] = False
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = measurement_freq

    return config

def blocksim_high_fidelity(
        system_size, 
        us, 
        feedback_mode, 
        eq_steps,
        sampling_timesteps=5000,
        temporal_avg=True,
        measurement_freq=10,
        nruns=250,
        avalanche_type=0,
        delta=2.0,
        sample_surface=True,
        sample_surface_avg=True,
        sample_avalanche_sizes=False,
        sample_roughness=False,
        sample_rugosity=False,
        sample_structure_function=False,
        sample_staircases=False
    ):
    config = {"circuit_type": "blocksim"}
    config["num_runs"] = nruns
    
    config["feedback_mode"] = feedback_mode

    probs = []
    for u in us:
        probs_u = {}
        if u < 1.0:
            probs_u['pu'] = u
            probs_u['pm'] = 1.0
        else:
            probs_u['pu'] = 1.0
            probs_u['pm'] = 1.0/u
        probs.append(probs_u)
    
    config["zparams"] = probs
    
    config["random_sites"] = True
    config["precut"] = [True]
    config["depositing_type"] = [1]
    config["avalanche_type"] = avalanche_type
    config["delta"] = delta
    
    config["sample_surface_avg"] = sample_surface_avg
    config["sample_surface"] = sample_surface

    config["temporal_avg"] = temporal_avg
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = eq_steps
    config["measurement_freq"] = measurement_freq

    zparams = []
    for L in system_size:
        zparams.append({
            "system_size": L,
            "num_bins": L//2,
            "max_av": L//2,
        })
    
    config["zparams2"] = zparams

    config["min_av"] = 1
    config["sample_avalanche_sizes"] = sample_avalanche_sizes

    config["sample_roughness"] = sample_roughness
    config["sample_rugosity"] = sample_rugosity
    config["sample_structure_function"] = sample_structure_function
    config["sample_staircases"] = sample_staircases
    
    return config_to_string(config)

# all modes
us = np.linspace(0.01, 2.0, 40)
system_sizes = [128]
eq_steps = [10000, 50000, 100000, 500000, 1000000, 5000000, 10000000]
for mode in [28]:
    config = blocksim_high_fidelity(system_sizes, feedback_mode=mode, us=us, eq_steps=eq_steps, nruns=300, sample_structure_function=False, sample_avalanche_sizes=False)
    #submit_jobs(config, f"blocksim_{mode}_eq", memory="150gb", time="24:00:00", ncores=48, nodes=12, record_error=True)
 
us = np.linspace(0.01, 2.0, 40)
system_size = [128]
config = blocksim_high_fidelity(system_size, us=us, feedback_mode=28, eq_steps=0, sampling_timesteps=1000, measurement_freq=1, temporal_avg=False, nruns=250, sample_surface=True, sample_avalanche_sizes=False, sample_structure_function=False, sample_rugosity=False, sample_staircases=True)
#submit_jobs(config, f"blocksim_28_stairs3", memory="150gb", time="24:00:00", ncores=48, nodes=4, record_error=True)
 

us = np.linspace(0.1, 0.2, 20)
system_sizes = [256]
for L in system_sizes:
    for mode in [21]:
        config = blocksim_time_evolution(mode, us, nruns=100, system_sizes=L, equilibration_timesteps=0, sampling_timesteps=50000, measurement_freq=10, avalanche_type=1, delta=2.0, sample_surface=False)
        submit_jobs(config, f"blocksim_{mode}_{L}_t", ncores=4, nodes=1, memory="10gb", run_local=True, cleanup=False)



 
    
def rpm_high_fidelity(
        system_size, 
        us, 
        num_runs=50,
        equilibration_timesteps=50000, 
        sampling_timesteps=50000, 
        measurement_freq=10, 
        temporal_avg=True, 
        pbc=False, 
        initial_state=0,
        sample_surface=True,
        sample_surface_avg=True,
        sample_avalanche_sizes=True,
        sample_roughness=True,
        sample_rugosity=False,
        sample_structure_function=False,
    ):
    
    config = {"circuit_type": "rpm"}
    config["num_runs"] = num_runs
    
    config["system_size"] = system_size

    config["pbc"] = pbc
    config["initial_state"] = initial_state

    probs = []
    for u in us:
        probs_u = {}
        if u < 1.0:
            probs_u['pu'] = u
            probs_u['pm'] = 1.0
        else:
            probs_u['pu'] = 1.0
            probs_u['pm'] = 1.0/u
        probs.append(probs_u)
    
    config["zparams"] = probs
    
    config["temporal_avg"] = temporal_avg
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = measurement_freq

    config["sample_surface"] = sample_surface
    config["sample_surface_avg"] = sample_surface_avg
    
    config["num_bins"] = system_size//4
    config["min_av"] = 1
    config["max_av"] = system_size//4
    config["sample_avalanche_sizes"] = sample_avalanche_sizes

    config["sample_roughness"] = sample_roughness
    config["sample_rugosity"] = sample_rugosity
    config["sample_structure_function"] = sample_structure_function

    return config_to_string(config)

#us = np.linspace(0.01, 60, 500)
#us = np.append(us, [1.0, 40.0])
#system_sizes = [10001]
#config = rpm_high_fidelity(system_sizes, us)
#submit_jobs(config, f"rpm_large", memory="20gb", time="24:00:00", ncores=10, record_error=True, nodes=5)

us = np.arange(0.0, 2.0, 0.05)
us = np.concatenate((us, np.linspace(2.0, 60, 20)))
#system_sizes = [64, 128, 256, 512, 1024, 2048, 4096]
#config = rpm_high_fidelity(system_sizes, us)
#submit_jobs(config, f"rpm_scaling_2", memory="100gb", time="24:00:00", ncores=48, nodes=10, record_error=True)

#config = rpm_high_fidelity(8192, [100.0], num_runs=44, equilibration_timesteps=0, sampling_timesteps=20000, measurement_freq=1, temporal_avg=False, pbc=[True, False], initial_state=[0,1], sample_surface=False, sample_avalanche_sizes=True)
#submit_jobs(config, f"rpm_reproduce", memory="150gb", time="24:00:00", ncores=48, nodes=4, record_error=False)

#config = rpm_high_fidelity(2, [100.0], equilibration_timesteps=0, pbc=[True, False], initial_state=[0,1], num_runs=1)
#submit_jobs(config, f"rpm_test", ncores=1, record_error=False, run_local=True)

#config = rpm_high_fidelity(16384/2, [100.0], equilibration_timesteps=0, sampling_timesteps=20000, measurement_freq=100, temporal_avg=False, pbc=True, sample_surface=False)
#submit_jobs(config, f"rpm_reproduce2", memory="150gb", time="24:00:00", ncores=48, nodes=4, record_error=False)

config = rpm_high_fidelity(
    256, 
    us, 
    num_runs=20, 
    equilibration_timesteps=20000, 
    sampling_timesteps=10000, 
    measurement_freq=5, 
    temporal_avg=True, 
    pbc=[True], 
    initial_state=[0], 
    sample_surface=False,
    sample_surface_avg=True,
    sample_avalanche_sizes=False,
    sample_roughness=True,
    sample_rugosity=True,
    sample_structure_function=True
)

#submit_jobs(config, f"rpm_256", memory="10gb", time="2:00:00", ncores=12, nodes=12, record_error=True)
#submit_jobs(config, f"rpm_2048_2", memory="5gb", time="1:00:00", ncores=12, nodes=4, record_error=True)

config = rpm_high_fidelity(
    16, 
    us, 
    num_runs=44, 
    equilibration_timesteps=20000, 
    sampling_timesteps=10000, 
    measurement_freq=5, 
    temporal_avg=True, 
    pbc=[False], 
    initial_state=[0], 
    sample_surface=False,
    sample_surface_avg=False,
    sample_avalanche_sizes=False,
    sample_roughness=False,
    sample_rugosity=False,
    sample_structure_function=True
)

#submit_jobs(config, f"rpm_test", memory="150gb", time="24:00:00", ncores=1, record_error=True, run_local=True, cleanup=False)