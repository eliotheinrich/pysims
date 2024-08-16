from dataframe import *
data = load_data("../data/_qrpm_test.eve")
L = data.query("system_size")

surface = data.query("surface", num_samples=True)
print(data.params)
print(data.query('equilibration_timesteps'))
print(surface.shape)
surface = surface[0,L//2,:]
print(surface.shape)
print(surface)

for slide in data.slides:
    print(slide.params)
