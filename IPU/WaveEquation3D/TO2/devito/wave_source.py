# Based on the implementation of the Devito acoustic example implementation
# Not using Devito's source injection abstraction
import numpy as np
from devito import TimeFunction, Eq, Operator, solve, norm
from examples.seismic import RickerSource

from examples.seismic import Model, TimeAxis

import os
import panel as pn
import codecs, json 
import sys

os.system('/usr/bin/Xvfb :99 -screen 0 1024x768x24 &')
os.environ['DISPLAY'] = ':99'

import argparse

parser = argparse.ArgumentParser(description='Process arguments.')

parser.add_argument("-d", "--shape", default=(11, 11, 11), type=int, nargs="+",
                    help="Number of grid points along each axis")
parser.add_argument("-so", "--space_order", default=4,
                    type=int, help="Space order of the simulation")
parser.add_argument("-to", "--time_order", default=2,
                    type=int, help="Time order of the simulation")
parser.add_argument("-nt", "--nt", default=200,
                    type=int, help="Simulation time in millisecond")
parser.add_argument("-bls", "--blevels", default=2, type=int, nargs="+",
                    help="Block levels")
parser.add_argument("-plot", "--plot", default=False, type=bool, help="Plot3D")
args = parser.parse_args()

def plot_3dfunc(u,name):
    pn.extension('vtk')
    # Plot a 3D structured grid using pyvista

    import matplotlib.pyplot as plt
    import pyvista as pv

  
    cmap = plt.colormaps["viridis"]
    values = u.data[0, :, :, :]
    vistagrid = pv.UniformGrid()
    vistagrid.dimensions = np.array(values.shape) + 1
    vistagrid.spacing = (1, 1, 1)
    vistagrid.origin = (0, 0, 0)  # The bottom left corner of the data set
    vistagrid.cell_data["values"] = values.flatten(order="F")
    vistaslices = vistagrid.slice_orthogonal()
    # vistagrid.plot(show_edges=True)
    # vistaslices.plot(cmap=cmap)

    plotter = pv.Plotter(off_screen=True)
    color_range = abs(max(values.min(), values.max(), key=abs))
    plotter.add_mesh(vistaslices,cmap=cmap,clim=[-color_range,color_range])
    plotter.show(screenshot=f'./{name}.png') 

# Define a physical size
nx, ny, nz = args.shape
nt = args.nt

shape = (nx, ny, nz)  # Number of grid point (nx, ny, nz)
spacing = (10., 10., 10.)  # Grid spacing in m. The domain size is now 1km by 1km
origin = (0., 0., 0.)  # What is the location of the top left corner. This is necessary to define
# the absolute location of the source and receivers

# Define a velocity profile. The velocity is in km/s
v = np.empty(shape, dtype=np.float32)
v[:, :, :51] = np.float32(1.5)
v[:, :, 51:] = np.float32(2.5)

# With the velocity and model size defined, we can create the seismic model that
# encapsulates this properties. We also define the size of the absorbing layer as 10 grid points
so = args.space_order
to = args.time_order

model = Model(vp=v, origin=origin, shape=shape, spacing=spacing,
              space_order=so, nbl=10, bcs="damp")

# plot_velocity(model)

t0 = 0.  # Simulation starts a t=0
tn = nt  # Simulation last 1 second (1000 ms)
dt = model.critical_dt  # Time step from model grid spacing

time_range = TimeAxis(start=t0, stop=tn, step=dt)
# Define the wavefield with the size of the model and the time dimension
u = TimeFunction(name="u", grid=model.grid, time_order=to, space_order=so)
# We can now write the PDE
pde = model.m * u.dt2 - u.laplace + model.damp * u.dt
stencil = Eq(u.forward, solve(pde, u.forward))

# The source is positioned at a $20m$ depth and at the middle of the $x$ axis ($x_{src}=500m$),
#  with a peak wavelet frequency of $10Hz$.
f0 = 0.010  # Source peak frequency is 10Hz (0.010 kHz)
src = RickerSource(name='src', grid=model.grid, f0=f0,
                   npoint=1, time_range=time_range)
# First, position source centrally in all dimensions, then set depth
src.coordinates.data[0, :] = np.array(model.domain_size) * .5
# Finally we define the source injection and receiver read function to generate the corresponding code
src_term = src.inject(field=u.forward, expr=src * dt**2 / model.m)

op = Operator([stencil] + src_term, subs=model.spacing_map)
# Run with source and plot
op.apply(time=50, dt=model.critical_dt)

parameters = {
  "dt": float(dt),
  "nt": int(nt),
  "steps": int(np.ceil((nt - t0 + dt)/dt))-1 ,
  "shape": [float(model.damp.shape[0]+so),float(model.damp.shape[1]+so),float(model.damp.shape[2]+so)],
  "damp": model.damp.data.tolist(),
  "vp": model.vp.data.tolist(),
  "u0": u.data[0].tolist(),
  "u1": u.data[1].tolist(),
  "u2": u.data[2].tolist()
}

print(f"STEPS: {time_range.num}")

json_object = json.dumps(parameters, indent=4)
 
# Writing to sample.json
with open(f"parameters_{u.shape[1]+so}_{nt}.json", "w") as outfile:
    outfile.write(json_object)
    
if args.plot:
    plot_3dfunc(u,"init")

op = Operator([stencil], subs=model.spacing_map)

# Run more and pliot again
op.apply(time=time_range.num, dt=model.critical_dt)

print(f"number of steps = ",int(np.ceil((nt - t0 + dt)/dt))-1,file=sys.stderr)
print(f"u (shape)       = ", u.shape,file=sys.stderr)
print(f"norm u[0]       =  {np.linalg.norm(u.data[0,:,:,:]):0,.15f}",file=sys.stderr)
print(f"norm u[1]       =  {np.linalg.norm(u.data[1,:,:,:]):0,.15f}",file=sys.stderr)
print(f"norm u[2]       =  {np.linalg.norm(u.data[2,:,:,:]):0,.15f}",file=sys.stderr)

if args.plot:
    plot_3dfunc(u,"ref")



print(np.linalg.norm(u.data[(time_range.num+1)%3,:,:,:]))
