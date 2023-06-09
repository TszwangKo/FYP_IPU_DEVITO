
# Program to plot 2-D Heat map
# using matplotlib.pyplot.imshow() method
import numpy as np
import matplotlib.pyplot as plt

import codecs, json 
import argparse

import os
os.system('/usr/bin/Xvfb :99 -screen 0 1024x768x24 &')
os.environ['DISPLAY'] = ':99'

import panel as pn
pn.extension('vtk')


parser = argparse.ArgumentParser(description='Process arguments.')

parser.add_argument("-name", "--name", default="ipu", type=str, 
                    help="name of grid data")
parser.add_argument("--src", "--src", default="./json/", type=str, 
                    help="name of path to look for file `name` ")

args = parser.parse_args()

#https://gist.github.com/garybradski/66e5f184702fe71398e13b2f4961a31c

def plot_3dfunc(u):
    # Plot a 3D structured grid using pyvista

    import matplotlib.pyplot as plt
    import pyvista as pv
    cmap = plt.colormaps["viridis"]
    values = u
    vistagrid = pv.UniformGrid()
    vistagrid.dimensions = np.array(values.shape) + 1
    vistagrid.spacing = (1, 1, 1)
    vistagrid.origin = (0, 0, 0)  # The bottom left corner of the data set
    vistagrid.cell_data["values"] = values.flatten(order="F")
    vistaslices = vistagrid.slice_orthogonal()

    plotter = pv.Plotter(off_screen=True)
    color_range = abs(max(u.min(), u.max(), key=abs))
    plotter.add_mesh(vistaslices,cmap=cmap,clim=[-color_range,color_range])
    plotter.add_mesh(vistaslices,cmap=cmap)
    plotter.show(screenshot=f'./plot/{args.name}.png') 
    # vistagrid.plot(show_edges=True)
    # vistaslices.plot(cmap=cmap)

file_path = f"{args.src}{args.name}.json"
obj_text = codecs.open(file_path, 'r', encoding='utf-8').read()
data = json.loads(obj_text) #This reads json to list
data_np = np.array(data)      #This converts to numpy

if (data_np.ndim==3):
    data_grid = data_np
else:
    data_grid = data_np[0,:,:,:]
# b = a_new.tolist() # nested lists with same data, indices
#    Obviously, if you already have list, you don't/can't .tolist() it
# file_path = "./iter10_reshaped.json" ## your path variable
# json.dump(b, codecs.open(file_path, 'w', encoding='utf-8'), separators=(',', ':'), sort_keys=True, indent=4)

# print(data_grid)
plot_3dfunc(data_grid)
