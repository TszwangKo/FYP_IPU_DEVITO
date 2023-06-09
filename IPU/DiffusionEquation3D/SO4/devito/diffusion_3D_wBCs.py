# A 3D heat diffusion using Devito
# BC modelling included
# PyVista plotting included

import argparse
import numpy as np

import sys, os
import codecs, json 

os.system('/usr/bin/Xvfb :99 -screen 0 1024x768x24 &')
os.environ['DISPLAY'] = ':99'

import panel as pn
pn.extension('vtk')

from devito import Grid, TimeFunction, Eq, solve, Operator, Constant, norm

parser = argparse.ArgumentParser(description='Process arguments.')

parser.add_argument("-d", "--shape", default=(11, 11, 11), type=int, nargs="+",
                    help="Number of grid points along each axis")
parser.add_argument("-so", "--space_order", default=4,
                    type=int, help="Space order of the simulation")
parser.add_argument("-to", "--time_order", default=1,
                    type=int, help="Time order of the simulation")
parser.add_argument("-nt", "--nt", default=40,
                    type=int, help="Simulation time in millisecond")
parser.add_argument("-bls", "--blevels", default=2, type=int, nargs="+",
                    help="Block levels")
parser.add_argument("-plot", "--plot", default=False, type=bool, help="Plot3D")
args = parser.parse_args()

def dump_json(u,name):
    b = u.data.tolist() # nested lists with same data, indices
    file_path = f"{name}.json" ## your path variable
    json.dump(b, codecs.open(file_path, 'w', encoding='utf-8'), 
            separators=(',', ':'), 
            sort_keys=True, 
            indent=4) ### this saves the array in .json format

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
    plotter.show(screenshot=f'./diffuse_{args.shape[0]}x{args.shape[1]}x{args.shape[2]}_{args.nt}.png') 

# Some variable declarations
nx, ny, nz = args.shape
nt = args.nt
nu = .5
dx = 2. / (nx - 1)
dy = 2. / (ny - 1)
dz = 2. / (nz - 1)
sigma = .25
dt = sigma * dx * dz * dy / nu
so = args.space_order
to = 1

grid = Grid(shape=(nx, ny, nz), extent=(2., 2., 2.))
u = TimeFunction(name='u', grid=grid, space_order=so)
# init_hat(field=u.data[0], dx=dx, dy=dy, value=2.)
u.data[:, :, :, :] = 0
u.data[:, int(3*nx/4), int(ny/2), int(nz/2)] = 1


dump_json(u,"initial_value")

a = Constant(name='a')
# Create an equation with second-order derivatives
eq = Eq(u.dt, a * u.laplace, subdomain=grid.interior)
stencil = solve(eq, u.forward)
eq_stencil = Eq(u.forward, stencil)

# Create boundary condition expressions
x, y, z = grid.dimensions
t = grid.stepping_dim


# Add boundary conditions
# bc = [Eq(u[t+1, x, y, 0], 2.)]  # bottom
# bc += [Eq(u[t+1, x, y, nz-1], 2.)]  # top
# bc += [Eq(u[t+1, 0, y, z], 2.)]  # left
# bc += [Eq(u[t+1, nx-1, y, z], 2.)]  # right

# bc += [Eq(u[t+1, x, 0, z], 2.)]  # front
# bc += [Eq(u[t+1, x, ny-1, z], 2.)]  # back

# Create an operator that updates the forward stencil point
# plus adding boundary conditions
# op = Operator([eq_stencil] + bc, subdomain=grid.interior)

# No BCs
op = Operator([eq_stencil], subdomain=grid.interior)
# print(op.ccode)

print(nt)
# Apply the operator for a number of timesteps
op(time=nt, dt=dt, a=nu)

print(u.shape)
# print("nt+dt/dt gives :",int((nt+dt)/dt))
if args.plot:
    plot_3dfunc(u)

# import pdb;pdb.set_trace()