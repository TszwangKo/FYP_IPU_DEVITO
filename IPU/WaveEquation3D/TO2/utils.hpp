#pragma once
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
/*
README
To avoid confusion in indexing:
  x goes along heigth
  y goes along width
  z goes along depth

All triple nested loops go:
for x in height
  for y in width
    for z in depth

And the 3D dimension is organized as [h, w, d]
*/

using json = nlohmann::json;

namespace utils {
    
  struct Options {
    // Command line arguments (with default values)
    unsigned num_ipus;
    unsigned num_iterations;
    float dt;
    std::size_t nt;
    std::size_t height;
    std::size_t width;
    std::size_t depth;
    std::size_t padding;
    std::string vertex;
    bool cpu;
    bool save_res;
    bool compile_only;
    bool load_exe;
    // Not command line arguments
    std::size_t side;
    std::size_t tiles_per_ipu = 0;
    std::size_t num_tiles_available = 0;
    std::size_t halo_volume = 0;
    std::vector<std::size_t> splits = {0,0,0};
    std::vector<std::size_t> smallest_slice = {std::numeric_limits<size_t>::max(),1,1};
    std::vector<std::size_t> largest_slice = {0,0,0};
  };

  inline
  Options parseOptions(int argc, char** argv) {
    Options options;
    namespace po = boost::program_options;
    po::options_description desc("Flags");
    desc.add_options()
    ("help", "Show command help.")
    (
      "num-ipus",
      po::value<unsigned>(&options.num_ipus)->default_value(1),
      "Number of IPUs (must be a power of 2)"
    )
    (
      "num-iterations",
      po::value<unsigned>(&options.num_iterations)->default_value(10),
      "PDE: number of iterations to execute on grid."
    )
    (
      "height",
      po::value<std::size_t>(&options.height)->default_value(31),
      "Heigth of a custom 3D grid"
    )
    (
      "width",
      po::value<std::size_t>(&options.width)->default_value(31), 
      "Width of a custom 3D grid"
    )
    (
      "depth",
      po::value<std::size_t>(&options.depth)->default_value(31),
      "Depth of a custom 3D grid"
    )
    (
      "padding",
      po::value<std::size_t>(&options.padding)->default_value(2),
      "Padding of elements around calculation."
    )
    (
      "dt",
      po::value<float>(&options.dt)->default_value(0.5),
      "PDE: update step size given as a float."
    )
    (
      "nt",
      po::value<std::size_t>(&options.nt)->default_value(40),
      "PDE: update terminal time as an int."
    )
    (
      "vertex",
      po::value<std::string>(&options.vertex)->default_value("WaveEquationOptimised"),
      "Name of vertex (from codelets.cpp) to use for the computation."
    )
    (
      "save-result",
      po::bool_switch(&options.save_res)->default_value(false),
      "Save result to file result.txt"
    )
    (
      "cpu",
      po::bool_switch(&options.cpu)->default_value(false),
      "Also perform CPU execution to control results from IPU."
    )    
    (
      "compile-only",
      po::bool_switch(&options.compile_only)->default_value(false),
      "Compile graph object only"
    )
    (
      "load-exe",
      po::bool_switch(&options.load_exe)->default_value(false),
      "directly load exe from file"
    ); // NOTE: remember to remove this semicolon if more options are added in future
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
      std::cout << desc << "\n";
      throw std::runtime_error("Show help");
    }
    po::notify(vm);
    return options;
  }

} // End of namespace Utils

poplar::Device getDevice(utils::Options &options) {
  /* return a Poplar device with the desired number of IPUs */
  std::size_t n = options.num_ipus;
  if (n!=1 && n!=2 && n!=4 && n!=8 && n!=16 && n!=32 && n!=64)
    throw std::runtime_error("Invalid number of IPUs.");

  // Create device manager
  auto manager = poplar::DeviceManager::createDeviceManager();
  auto devices = manager.getDevices(poplar::TargetType::IPU, n);

  // Use the first available device
  for (auto &device : devices)
    if (device.attach()) 
      return std::move(device);

  throw std::runtime_error("No hardware device available.");
}

std::size_t side_length(std::size_t num_ipus, std::size_t base_length) {
  std::size_t log2_num_ipus = log(num_ipus) / log(2);
  std::size_t side = base_length*pow(1.26, log2_num_ipus);
  return side;
}

inline float randomFloat() {
  return static_cast <float> (rand() / static_cast <float> (RAND_MAX));
}

inline static unsigned index(unsigned x, unsigned y, unsigned z, unsigned width, unsigned depth) { 
  return (z) + (y)*(depth) + (x)*(width)*(depth);
}

inline static unsigned block_low(unsigned id, unsigned p, unsigned n) {
  return (id*n)/p; 
}

inline static unsigned block_high(unsigned id, unsigned p, unsigned n) {
  return block_low(id+1, p, n); 
}

inline static unsigned block_size(unsigned id, unsigned p, unsigned n) {
  return block_high(id, p, n) - block_low(id, p, n); 
}

std::size_t volume(std::vector<std::size_t> shape) {
  // return volume of shape vector (3D)
  return shape[0]*shape[1]*shape[2];
}

void workDivision(utils::Options &options) {
  /* Function UPDATES options.splits
   * 1) all tiles will be used, hence options.num_tiles_available must
   *    previously be updated by using the target object
   * 2) the average resulting slice will have shape [height/nh, width/nw, depth/nd'] and
   *    this function chooses nh, nw, nd, so that the surface area is minimized.
   */
  float smallest_surface_area = std::numeric_limits<float>::max();
  std::size_t height = (options.height - 2);
  std::size_t width = (options.width - 2);
  std::size_t depth = (options.depth - 2) / options.num_ipus;
  std::size_t tile_count = options.num_tiles_available / options.num_ipus;
  for (std::size_t i = 1; i <= tile_count; ++i) {
    if (tile_count % i == 0) { // then i is a factor
      // Further, find two other factors, to obtain exactly three factors
      std::size_t other_factor = tile_count/i;
      for (std::size_t j = 1; j <= other_factor; ++j) {
        if (other_factor % j == 0) { // then j is a second factor
          std::size_t k = other_factor/j; // and k is the third factor
          std::vector<std::size_t> splits = {i,j,k}; 
          if (i*j*k != tile_count) {
            throw std::runtime_error("workDivision(), factorization does not work.");
          }
          for (std::size_t l = 0; l < 3; ++l) {
            for (std::size_t m = 0; m < 3; ++m) {
              for (std::size_t n = 0; n < 3; ++n) {
                if (l != m && l != n && m != n) {
                  float slice_height = float(height)/float(splits[l]);
                  float slice_width = float(width)/float(splits[m]);
                  float slice_depth = float(depth)/float(splits[n]);
                  float surface_area = 2.0*(slice_height*slice_width + slice_depth*slice_width + slice_depth*slice_height);
                  if (surface_area <= smallest_surface_area) {
                    smallest_surface_area = surface_area;
                    options.splits = splits;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void printIfNan(float number, std::string message) {
    if( isnan(number) )
        std::cout << "number" << message << std::endl;
}

std::vector<float> WaveEquationCpu(
  const std::vector<float> initial_values,
  const std::vector<float> damp, 
  const std::vector<float> vp, 
  const utils::Options &options) {
  /*
   * Heat Equation Vertex, computed on the CPU, unparallelized.
   * The purpose of this function is to serve as the "true"
   * solution.
   * NOTE: can be very slow for large grids/large number of iterations
   */
    
  const float dt = options.dt;

  const float r0 = 1.0f/(dt*dt);
  const float r1 = 1.0f/dt;
 

  unsigned h = options.height;
  unsigned w = options.width;
  unsigned d = options.depth;
  unsigned padding = options.padding;
  unsigned iter = options.num_iterations;
  
  std::vector<float> a(initial_values.size());
  std::vector<float> b(initial_values.size());
  std::vector<float> c(initial_values.size());

  // initialise vectors (including edges)
  for (std::size_t i = 0; i < initial_values.size(); ++i)  {
    a[i] = 0.0f; 
    b[i] = initial_values[i]; 
    c[i] = 0.0f; 
  }
  auto& t0 = c;
  auto& t1 = b;
  auto& t2 = a;
  // Wave Equation iterations
  for (std::size_t t = 0; t < iter; ++t) {
    for (std::size_t x = padding; x < h - padding; ++x) {
      for (std::size_t y = padding; y < w - padding; ++y) { 
        for (std::size_t z = padding; z < d - padding; ++z) {
            // a[index(x,y,z,w,d)(x,y,z,w,d)] = b[index(x,y,z,w,d)] + c[index(x,y,z,w,d)];
            float r2 = 1.0F/(vp[index(x,y,z,w,d)]*vp[index(x,y,z,w,d)]);
            t2[index(x,y,z,w,d)] = 
              ( r1*damp[index(x,y,z,w,d)]*t0[index(x,y,z,w,d)] 
                + r2*(
                    - r0*(-2.0F*t0[index(x,y,z,w,d)]) 
                    - r0*t1[index(x,y,z,w,d)]
                  ) 
                  + 8.33333315e-4F*(
                    - t0[index(x-2,y,z,w,d)] - t0[index(x,y-2,z,w,d)] - t0[index(x,y,z-2,w,d)] 
                    - t0[index(x,y,z+2,w,d)] - t0[index(x,y+2,z,w,d)] - t0[index(x+2,y,z,w,d)]
                  ) 
                  + 1.3333333e-2F*(
                      t0[index(x-1,y,z,w,d)] + t0[index(x,y-1,z,w,d)] + t0[index(x,y,z-1,w,d)] 
                    + t0[index(x,y,z+1,w,d)] + t0[index(x,y+1,z,w,d)] + t0[index(x+1,y,z,w,d)]
                  ) 
                  - 7.49999983e-2F*t0[index(x,y,z,w,d)]
              )/(r0*r2 + r1*damp[index(x,y,z,w,d)]);
        }
      }
    }
      
    for (std::size_t x = padding; x < h - padding ; ++x) {
      for (std::size_t y = padding; y < w - padding ; ++y) { 
        for (std::size_t z = padding; z < d - padding ; ++z) {
          b[index(x,y,z,w,d)] = c[index(x,y,z,w,d)];
          c[index(x,y,z,w,d)] = a[index(x,y,z,w,d)];
        }
      }
    }
  }
  return a;
}

void printMeanSquaredError(
  std::vector<float> a, 
  std::vector<float> b, 
  utils::Options &options) {
  /*
   * Compute the MSE, __only the inner elements__, of two 3D grids
   */
  double squared_error = 0, diff;
  std::size_t h = options.height;
  std::size_t w = options.width;
  std::size_t d = options.depth;
  std::size_t padding = options.padding;
  for (std::size_t x = padding; x < h - padding; ++x) {
    for (std::size_t y = padding; y < w - padding; ++y) { 
      for (std::size_t z = padding; z < d - padding; ++z) {
        diff = double(a[index(x,y,z,w,d)] - b[index(x,y,z,w,d)]);
        squared_error += diff*diff;
      }
    }
  }
  double mean_squared_error = squared_error / (double) ((h-4)*(w-4)*(d-4));

  std::cout << "\nMean Squared Error (IPU vs. CPU) = " << mean_squared_error;
  if (mean_squared_error == double(0.0)) {
    std::cout << " (exactly)";
  }
  std::cout << "\n";
}

void saveMatrixToJson (
    std::vector<float> matrix, 
    utils::Options &options,
    std::string file_name) {

    std::vector<std::vector<std::vector<float>>> matrix_3D (options.height-options.padding*2, std::vector<std::vector<float>>(options.width-options.padding*2, std::vector<float>(options.depth-options.padding*2)));
    for ( int x = options.padding; x < options.height-options.padding ; x++ ){
        for( int y = options.padding ; y < options.width-options.padding ; y++ ){
            for (int z = options.padding  ; z < options.depth-options.padding ; z++ ){
                matrix_3D[x-options.padding][y-options.padding][z-options.padding] = matrix[index(x,y,z,options.width,options.depth)];
            }
        }
    }
    std::string path_name = "./output/" + file_name + ".json"; 
    json jsonfile(matrix_3D);

    std::ofstream file(path_name);
    file << jsonfile;
    file.close();
}

inline std::string makeExeFileName(const std::string& name) {
  return name + ".poplar.exe";
}

inline void saveExe(const poplar::Executable& exe, const std::string& name) {
  const auto fileName = makeExeFileName(name);
  auto outf = std::ofstream(fileName);
  exe.serialize(outf);
  std::cerr << "Saved Poplar executable as: " << fileName << std::endl;
}

inline poplar::Executable loadExe(const std::string& name) {
  const auto exeName = makeExeFileName(name);
  std::cerr << "Loading precompiled graph from: " << exeName << std::endl;
  try {
    auto inf = std::ifstream(exeName);
    return poplar::Executable::deserialize(inf);
  } catch (const poplar::poplar_error& e) {
    std::cerr << "Error: Failed to load executable: " << exeName << std::endl; 
    throw;
  }
}

void printNorm(
  std::vector<float> a, 
  utils::Options &options) {
  double norm = 0;
  std::size_t h = options.height;
  std::size_t w = options.width;
  std::size_t d = options.depth;
    
  
  std::cout << "Norms:" << std::endl;
  for (std::size_t x = 0; x < h ; ++x) {
    for (std::size_t y = 0; y < w ; ++y) { 
      for (std::size_t z = 0; z < d ; ++z) {
            norm += a[index(x,y,z,w,d)]*a[index(x,y,z,w,d)];
      }
    }
  }
    std::cout << std::endl;
    std::cout << "\n L2 Norm =" << norm << std::endl;
}

double norm(
  std::vector<float> a, 
  utils::Options &options) {
  double norm = 0;
  std::size_t h = options.height;
  std::size_t w = options.width;
  std::size_t d = options.depth;
  
  for (std::size_t x = 0+options.padding; x < h - options.padding ; ++x) {
    for (std::size_t y = 0 + options.padding; y < w - options.padding; ++y) { 
      for (std::size_t z = 0 + options.padding; z < d - options.padding; ++z) {
            norm += a[index(x,y,z,w,d)]*a[index(x,y,z,w,d)];
      }
    }
  }
  
  return sqrt(norm);
}

void printNorms(
  std::vector<float> a, 
  std::vector<float> b, 
  utils::Options &options) {
    std::cout << std::endl;
    std::cout << "\n IPU L2 Norm =" << norm(a,options) << std::endl;
    std::cout << "\n CPU L2 Norm =" << norm(b,options)  << std::endl;
}

void printResults(utils::Options &options, double wall_time, double stream_time) {

  // Calculate metrics
  double inner_volume = (double) options.height * (double) options.width * (double) options.depth;
  double flops_per_element = 32.0;
  double flops = inner_volume * options.num_iterations * flops_per_element / wall_time;
  double internal_communication_ops = 2.0*(double)options.halo_volume*(double)options.num_ipus*options.padding; // communication ops per layer * layers of padding 
  double external_communication_ops = 4.0*(double)options.height*(double)options.width*(options.num_ipus - 1.0)*options.padding; // 2 load and 2 stores of a slice (partition along depth)
  double avg_load_store_per_element = 20.0;
  double bandwidth = (20.0*inner_volume + internal_communication_ops + external_communication_ops)*(double)options.num_iterations*sizeof(float)/wall_time;
  double tflops = flops*1e-12;
  double bandwidth_TB_s = bandwidth*1e-12;

  std::cout << "3D Isotropic Diffusion"
    << "\n----------------------"
    << "\nVertex             = " << options.vertex
    << "\nNo. IPUs           = " << options.num_ipus
    << "\nNo. Tiles          = " << options.num_tiles_available
    << "\nTotal Grid         = " << options.height << "*" << options.width << "*" << options.depth << " = "
                                 << options.height*options.width*options.depth*1e-6 << " million elements"
    << "\nSmallest Sub-grid  = " << options.smallest_slice[0] << "*" << options.smallest_slice[1] << "*" << options.smallest_slice[2] 
    << "\nLargest Sub-grid   = " << options.largest_slice[0] << "*" << options.largest_slice[1] << "*" << options.largest_slice[2] 
    << "\ndt                 = " << std::setprecision (20) <<  options.dt
    << "\nNo. Iterations     = " << options.num_iterations
    << "\n"
    << "\nLaTeX Tabular Row"
    << "\n-----------------"
    << "\nNo. IPUs & Grid    & No. Iterations & Exec Time [s] & Stream Time [s] & Throughput [TFLOPS] & Minimum Bandwidth [TB/s] \\\\\n" 
    << std::left << std::fixed << std::setprecision(2)
    << std::setw(8) << options.num_ipus << " & "
    << "$" << options.height << "^3" "$ & " 
    << std::setw(14) << options.num_iterations << " & " 
    << std::setw(13) << wall_time << " & " 
    << std::setw(15) << stream_time << " & " 
    << std::setw(19) << tflops << " & " 
    << std::setw(24) << bandwidth_TB_s << " \\\\"
    << "\n";
}

std::vector<std::vector<std::vector<float>>> getValues(std::string key,utils::Options &options){
    std::string file_name = "./devito/parameters_" + std::to_string(options.height) + "_" + std::to_string(options.nt) + ".json";
    std::ifstream ifs(file_name);
    json jf = json::parse(ifs);

    return jf[key].get<std::vector<std::vector<std::vector<float>>>>();
}

void getCpuResult(std::vector<float>& cpu_result, utils::Options &options){
    std::string file_name = "./devito/output/" + std::to_string(options.height) + "x" + std::to_string(options.nt) + ".json";
    std::ifstream ifs(file_name);
    json jf = json::parse(ifs);

    std::vector<std::vector<std::vector<float>>> cpu_result_3D = jf.get<std::vector<std::vector<std::vector<float>>>>();

    for ( int x = 0; x < options.height ; x++ ){
        for( int y = 0 ; y < options.width ; y++ ){
            for (int z = 0 ; z < options.depth ; z++ ){
                cpu_result[index(x,y,z,options.width,options.depth)] = 0;
            }
        }
    }
}

float getDt(utils::Options &options){
    std::string file_name = "./devito/parameters_" + std::to_string(options.height) + "_" + std::to_string(options.nt) + ".json";
    std::ifstream ifs(file_name);
    json jf = json::parse(ifs);

    return jf["dt"].get<float>();

}
int getNt(utils::Options &options){
    std::string file_name = "./devito/parameters_" + std::to_string(options.height) + "_" + std::to_string(options.nt) + ".json";
    std::ifstream ifs(file_name);
    json jf = json::parse(ifs);

    return jf["nt"].get<int>();
}

std::vector<float> getShape(utils::Options &options){
    std::string file_name = "./devito/parameters_" + std::to_string(options.height) + "_" + std::to_string(options.nt) + ".json";
    std::cout << file_name << std::endl;
    std::ifstream ifs(file_name);
    json jf = json::parse(ifs);

    return jf["shape"].get<std::vector<float>>();
}

int getSteps(utils::Options &options){
    std::string file_name = "./devito/parameters_" + std::to_string(options.height) + "_" + std::to_string(options.nt) + ".json";
    std::ifstream ifs(file_name);
    json jf = json::parse(ifs);

    return jf["steps"].get<int>();
}

void printMultiIpuGridInfo(std::size_t base_length) {
  float base_volume = 0;

  std::cout 
    << "\\begin{tabular}{ccc}"
    << "\n\\toprule"
    << "\nNumber of IPUs & Grid Shape & Relative Volume \\\\\\midrule"
    << std::fixed << std::setprecision(2);

  for (std::size_t i = 1; i <= 64; i*=2) {
    std::size_t side = side_length(i, base_length);
    float volume = float(side)*float(side)*float(side);
    if (i == 1) base_volume = volume;
    std::cout << "\n" 
      << i << " & $"
      << side << "\\times " << side << "\\times " << side << "$ & "
      << volume/base_volume << "x \\\\";
  }

  std::cout 
    << "\n\\bottomrule"
    << "\n\\end{tabular}\n";
}
