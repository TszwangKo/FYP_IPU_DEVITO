#include <poplar/Vertex.hpp>

using namespace poplar;

class WaveEquationSimple : public Vertex
{
public:
  WaveEquationSimple();

  Vector<Input<Vector<float, VectorLayout::SPAN, 8, false>>> in;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, false>>> damp;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, false>>> vp;
  Vector<Output<Vector<float, VectorLayout::SPAN, 4, false>>> out;
  const unsigned worker_height;
  const unsigned worker_width;
  const unsigned worker_depth;
  const unsigned padding;
  const float alpha;

  unsigned idx(unsigned x, unsigned y, unsigned w)
  {
    /* The index corresponding to [x,y] in for a row-wise flattened 2D variable*/
    return y + x * w;
  }

  bool compute()
  {
    const float beta{1.0f - 6.0f * alpha};
    const float hx = 0.2f;
    const float hy = 0.2f;
    const float hz = 0.2f;
    const float dt = 0.25f * hx * hy * hz / 0.5f;

    const float r0 = 1.0F / dt;
    const float r1 = 1.0F / (hx * hx);
    const float r2 = 1.0F / (hy * hy);
    const float r3 = 1.0F / (hz * hz);

    for (std::size_t x = padding; x < worker_height + padding; ++x)
    {
      for (std::size_t y = padding; y < worker_width + padding; ++y)
      {
        for (std::size_t z = padding; z < worker_depth + padding; ++z)
        {

          const unsigned padded_width = worker_width + 2*padding;
          // const float r4 = -2.5F * in[idx(x, y, padded_width)][z];
          out[idx(x-padding,y-padding,worker_width)][z-padding] = in[idx(x,y,padded_width)][z] + damp[idx(x,y,padded_width)][z] +  vp[idx(x,y,padded_width)][z];
          // out[idx(x - 2, y - 2, worker_width)][z - 2] = dt * (alpha * ( r1 * (r4 -
          //                                                                    8.33333333e-2F * (in[idx(x - 2, y, padded_width)][z] + in[idx(x + 2, y, padded_width)][z]) +
          //                                                                    1.33333333F * (in[idx(x - 1, y, padded_width)][z] + in[idx(x + 1, y, padded_width)][z])) +
          //                                                               r2 * (r4 -
          //                                                                    8.33333333e-2F * (in[idx(x, y - 2, padded_width)][z] + in[idx(x, y + 2, padded_width)][z]) +
          //                                                                    1.33333333F * (in[idx(x, y - 1, padded_width)][z] + in[idx(x, y + 1, padded_width)][z])) +
          //                                                               r3 * (r4 -
          //                                                                    8.33333333e-2F * (in[idx(x, y, padded_width)][z - 2] + in[idx(x, y, padded_width)][z + 2]) +
          //                                                                    1.33333333F * (in[idx(x, y, padded_width)][z - 1] + in[idx(x, y, padded_width)][z + 1]))) +
          //                                                     r0 * in[idx(x, y, padded_width)][z]);
        }
      }
    }

    return true;
  }
};

// class HeatEquationOptimized : public Vertex {
// public:
//   HeatEquationOptimized();

//   Vector<Input<Vector<float, VectorLayout::SPAN, 8, false>>> in;
//   Vector<Output<Vector<float, VectorLayout::SPAN, 4, false>>> out;
//   const unsigned worker_height;
//   const unsigned worker_width;
//   const unsigned worker_depth;
//   const float alpha;

//   bool compute () {
//     const float beta{1.0f - 6.0f*alpha};
//     const unsigned padded_width = worker_width + 2;
//     const int half_depth = worker_depth/2 + (worker_depth % 2);
//     typedef float float2 __attribute__((ext_vector_type(2)));
//     float2 temp; // Temporary variable
//     float2 front; // Temporary variable
//     float2 back; // Temporary variable

//     // Unoptimized loop
//     for (std::size_t x = 1; x < worker_height + 1; ++x) {
//       for (std::size_t y = 1; y < worker_width + 1; ++y) {
//         const float * __restrict__ top    = &in[(y+0) + (x-1)*padded_width][0];
//         const float * __restrict__ left   = &in[(y-1) + (x+0)*padded_width][0];
//         const float * __restrict__ middle = &in[(y+0) + (x+0)*padded_width][0];
//         const float * __restrict__ right  = &in[(y+1) + (x+0)*padded_width][0];
//         const float * __restrict__ bottom = &in[(y+0) + (x+1)*padded_width][0];
//         float * __restrict__ output = (float *) &out[(y-1) + (x-1)*worker_width][0];

//         // front slice
//         std::size_t z = 1; // corresponds to z=0 in output
//         output[z-1] = beta*middle[z] +
//           alpha*(top[z]+bottom[z]+left[z]+right[z]+middle[z+1]+middle[z-1]);

//         // back slice (if depth is even, otherwise included in float2 loop)
//         if (worker_depth % 2 == 0) {
//           z = worker_depth;
//           output[z-1] = beta*middle[z] +
//             alpha*(top[z]+bottom[z]+left[z]+right[z]+middle[z+1]+middle[z-1]);
//         }
//       }
//     }

//     // Optimized loop
//     for (std::size_t x = 1; x < worker_height + 1; ++x) {
//       for (std::size_t y = 1; y < worker_width + 1; ++y) {
//         const float2 * __restrict__ top    = (float2 *) &in[(y+0) + (x-1)*padded_width][0];
//         const float2 * __restrict__ left   = (float2 *) &in[(y-1) + (x+0)*padded_width][0];
//         const float2 * __restrict__ middle = (float2 *) &in[(y+0) + (x+0)*padded_width][0];
//         const float2 * __restrict__ right  = (float2 *) &in[(y+1) + (x+0)*padded_width][0];
//         const float2 * __restrict__ bottom = (float2 *) &in[(y+0) + (x+1)*padded_width][0];
//         float * __restrict__ output = (float *) &out[(y-1) + (x-1)*worker_width][0];

//         for (std::size_t z = 1; z < half_depth; ++z) { // z=(1,2),(3,4),... in output

//           //[z-1] [z+0] [z+1]
//           //[x y] [x y] [x y]

//           temp.x = middle[z-1].y + middle[z].y; // front + back elem. for stencil 0
//           temp.y = middle[z].x + middle[z+1].x; // front + back elem. for stencil 1

//           temp = beta*middle[z] + alpha*(temp + top[z] + bottom[z] + left[z] + right[z]);
//           output[2*z - 1] = temp.x; // Output for left stencil 0
//           output[2*z - 0] = temp.y; // Output for right stencil 1
//         }
//       }
//     }

//     return true;
//   }
// };