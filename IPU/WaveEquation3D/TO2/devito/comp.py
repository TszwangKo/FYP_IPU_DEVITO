import codecs, json 
import argparse


parser = argparse.ArgumentParser(description='Process arguments.')

parser.add_argument("-d", "--dimension", default=11, 
                    type=int, help="dimension long each side (assume cube dimension)")
parser.add_argument("-nt", "--nt", default=40,
                    type=int, help="Simulation time in millisecond")
parser.add_argument("-plot", "--plot", default=False, type=bool, help="Plot3D")

def readjson(file_path):
    obj_text = codecs.open(file_path, 'r', encoding='utf-8').read()
    data = json.loads(obj_text) #This reads json to list
    data_np = np.array(data)  


ipu_path = f"./output/res_{args.d}{args.nt}.json"
