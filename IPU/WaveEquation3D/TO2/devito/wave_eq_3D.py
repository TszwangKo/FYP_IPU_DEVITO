# Based on the implementation of the Devito acoustic example implementation
# Not using Devito's source injection abstraction
import numpy as np
from devito import TimeFunction, Eq, Operator, solve, norm

from examples.seismic import Model, TimeAxis
import codecs, json 
import argparse

import os
import sys

# os.system('/usr/bin/Xvfb :99 -screen 0 1024x768x24 &')
os.environ['DISPLAY'] = ':99'

import panel as pn
pn.extension('vtk')

import panel as pn




parser = argparse.ArgumentParser(description='Process arguments.')

parser.add_argument("-d", "--shape", default=(11, 11, 11), type=int, nargs="+",
                    help="Number of grid points along each axis")
parser.add_argument("-so", "--space_order", default=4,
                    type=int, help="Space order of the simulation")
parser.add_argument("-to", "--time_order", default=2,
                    type=int, help="Time order of the simulation")
parser.add_argument("-nt", "--nt", default=40,
                    type=int, help="Simulation time in millisecond")
parser.add_argument("-bls", "--blevels", default=2, type=int, nargs="+",
                    help="Block levels")
parser.add_argument("-plot", "--plot", default=False, type=bool, help="Plot3D")
args = parser.parse_args()

def plot_3dfunc(u):
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
    plotter.show(screenshot=f'./ref.png') 


# Define a physical size
nx, ny, nz = args.shape
nt = args.nt

shape = (nx, ny, nz)  # Number of grid point (nx, ny, nz)
spacing = (10., 10., 10.)  # Grid spacing in m. The domain size is now 1km by 1km
origin = (0., 0., 0.)  # What is the location of the top left corner. This is necessary to define
# the absolute location of the source and receivers

# Define a velocity profile. The velocity is in km/s
v = np.empty(shape, dtype=np.float32)
v[:, :, :51] = 1.5
v[:, :, 51:] = 2.5

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

px, py, pz = u.shape[1:]

# set initial value for t=0
u.data[0, int(px/2), int(py/2), 20] = 0.1

export_u = u.data[0,:,:,:].copy()
export_u1 = u.data[1,:,:,:].copy()


# We can now write the PDE
pde = model.m * u.dt2 - u.laplace + model.damp * u.dt

# The PDE representation is as on paper
# print(pde)
# print(solve(pde, u.forward))

stencil = Eq(u.forward, solve(pde, u.forward))
stencil

op = Operator([stencil], subs=model.spacing_map)
op.apply(time=time_range.num-1, dt=model.critical_dt)
# op.apply(time=time_range.num-1, dt=model.critical_dt, **{'x0_blk0_size': 16, 'y0_blk0_size': 8})

# print(f"dt              = {dt:0,.15f}",file=sys.stderr)
print(f"number of steps = ",int(np.ceil((nt - t0 + dt)/dt)),file=sys.stderr)
print(f"u (shape)       = ", u.shape,file=sys.stderr)
print(f"norm u[0]       =  {np.linalg.norm(u.data[0,:,:,:]):0,.15f}",file=sys.stderr)
print(f"norm u[1]       =  {np.linalg.norm(u.data[1,:,:,:]):0,.15f}",file=sys.stderr)
print(f"norm u[2]       =  {np.linalg.norm(u.data[2,:,:,:]):0,.15f}",file=sys.stderr)

b = model.damp.data.tolist() # nested lists with same data, indices
#    Obviously, if you already have list, you don't/can't .tolist() it
file_path = "damp.json" ## your path variable
json.dump(b, codecs.open(file_path, 'w', encoding='utf-8'), separators=(',', ':'), sort_keys=True, indent=4) ### this saves the array in .json format


b = u.data[0].tolist() # nested lists with same data, indices
file_path = f"./output/{u.shape[1]}x{nt}.json" ## your path variable
json.dump(b, codecs.open(file_path, 'w', encoding='utf-8'), 
          separators=(',', ':'), 
          sort_keys=True, 
          indent=4) ### this saves the array in .json format

parameters = {
  "dt": float(dt),
  "nt": int(nt),
  "steps": int(np.ceil((nt - t0 + dt)/dt))+1,
  "shape": [float(model.damp.shape[0]),float(model.damp.shape[1]),float(model.damp.shape[2])],
  "damp": model.damp.data.tolist(),
  "vp": model.vp.data.tolist(),
  "u": export_u.data.tolist()
}

json_object = json.dumps(parameters, indent=4)
 
# Writing to sample.json
with open(f"parameters_{u.shape[1]}_{nt}.json", "w") as outfile:
    outfile.write(json_object)

if args.plot:
    plot_3dfunc(u)

# #plot_image(u.data[0,:,:,10], cmap="viridis")
# #plt.show()