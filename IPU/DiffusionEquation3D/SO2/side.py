import math
import argparse

def side_length( num_ipus, base_length):
  log2_num_ipus = math.log(num_ipus) / math.log(2)
  side = base_length*pow(1.25, log2_num_ipus)
  return int(side)

base_length = 338

parser = argparse.ArgumentParser(description='Process arguments.')

parser.add_argument("-num_ipus", "--num-ipus", default=1, type=int, 
                    help="name of grid data")

args = parser.parse_args()

if __name__ == "__main__":
    print(side_length(args.num_ipus,base_length))
