#include <poplar/Vertex.hpp>

using namespace poplar;


class WaveEquationSimple : public Vertex
{
public:
  WaveEquationSimple();
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> in1;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> in2;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> damp;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> vp;
  Vector<Output<Vector<float, VectorLayout::SPAN, 4, false>>> out;
  const unsigned worker_height;
  const unsigned worker_width;
  const unsigned worker_depth;
  const unsigned padding;
  const float dt;
  unsigned idx(unsigned x, unsigned y, unsigned w)
  {
    /* The index corresponding to [x,y] in for a row-wise flattened 2D variable*/
    return y + x * w;
  }

  bool compute()
  {
    const float r0 = 1.0f/(dt*dt);
    const float r1 = 1.0f/dt;
    auto& t0 = in1;
    auto& t1 = in2;
    auto& t2 = out;
    const unsigned padded_width = worker_width + 2*padding;
    for (std::size_t x = padding; x < worker_height + padding; ++x)
    {
        for (std::size_t y = padding; y < worker_width + padding; ++y)
        {
            for (std::size_t z = padding; z < worker_depth + padding; ++z)
            {
            // out[idx(x-padding,y-padding,worker_width)][z-padding] = in1[idx(x,y,padded_width)][z] + in2[idx(x,y,padded_width)][z] ;

            float r2 = 1.0F/(vp[idx(x,y,padded_width)][z]*vp[idx(x,y,padded_width)][z]); //vp: 2 access + 1 store => 2 ops
            t2[idx(x-padding,y-padding,worker_width)][z-padding] = // 1 store + 18 store (damp and t0 overlaps ,assume not reused) => 18 ops
                    (   
                        r1*damp[idx(x,y,padded_width)][z]*t0[idx(x,y,padded_width)][z] +                         
                        r2*(
                            -r0*(-2.0F*t0[idx(x,y,padded_width)][z]) - 
                            r0*t1[idx(x,y,padded_width)][z]) + 
                        8.33333315e-4F*(
                            - t0[idx(x-2,y,padded_width)][z] - t0[idx(x,y-2,padded_width)][z] - t0[idx(x,y,padded_width)][z-2] - 
                              t0[idx(x,y,padded_width)][z+2] - t0[idx(x,y+2,padded_width)][z] - t0[idx(x+2,y,padded_width)][z]) + 
                        1.3333333e-2F*(
                              t0[idx(x-1,y,padded_width)][z] + t0[idx(x,y-1,padded_width)][z] + t0[idx(x,y,padded_width)][z-1] + 
                              t0[idx(x,y,padded_width)][z+1] + t0[idx(x,y+1,padded_width)][z] + t0[idx(x+1,y,padded_width)][z]) - 
                        7.49999983e-2F*t0[idx(x,y,padded_width)][z]
                    )/(r0*r2 + r1*damp[idx(x,y,padded_width)][z]);
        }
      }
    }
    return true;
  }
};

class WaveEquationOptimised : public Vertex
{
public:
  WaveEquationOptimised();

  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> in1;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> in2;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> damp;
  Vector<Input<Vector<float, VectorLayout::SPAN, 8, true>>> vp;
  Vector<Output<Vector<float, VectorLayout::SPAN, 4, false>>> out;
  const unsigned worker_height;
  const unsigned worker_width;
  const unsigned worker_depth;
  const unsigned padding;
  const float dt;

  unsigned idx(unsigned x, unsigned y, unsigned w)
  {
    /* The index corresponding to [x,y] in for a row-wise flattened 2D variable*/
    return y + x * w;
  }

  bool compute()
  {
    const float r0 = 1.0f/(dt*dt);
    const float r1 = 1.0f/dt;

    auto& t0 = in1;
    auto& t1 = in2;
    auto& t2 = out;

    const unsigned padded_width = worker_width + 2*padding;

    for (std::size_t x = padding; x < worker_height + padding; ++x)
    {
      for (std::size_t y = padding; y < worker_width + padding; ++y)
      {
        for (std::size_t z = padding; z < worker_depth + padding; ++z)
        {
          // out[idx(x-padding,y-padding,worker_width)][z-padding] = in1[idx(x,y,padded_width)][z] + in2[idx(x,y,padded_width)][z] ;
          
          float r2 = 1.0F/(vp[idx(x-padding,y-padding,worker_width)][z-padding]*vp[idx(x-padding,y-padding,worker_width)][z-padding]); // 1 div + 1 mult
          t2[idx(x-padding,y-padding,worker_width)][z-padding] =                                                                       // 1 store
            ( r1*damp[idx(x-padding,y-padding,worker_width)][z-padding]*t0[idx(x,y,padded_width)][z]                                   // 2 mult
              + r2*(                                                                // 1 add + 1 mult
                      - r0*(-2.0F*t0[idx(x,y,padded_width)][z])                     // 2 mult 
                      - r0*t1[idx(x-padding,y-padding,worker_width)][z-padding])    // 1 add + 1 mult 
              + 8.33333315e-4F*(                                                    // 1 add + 1 mult
                - t0[idx(x-2,y,padded_width)][z] - t0[idx(x,y-2,padded_width)][z] - t0[idx(x,y,padded_width)][z-2]  // 2 add
                - t0[idx(x,y,padded_width)][z+2] - t0[idx(x,y+2,padded_width)][z] - t0[idx(x+2,y,padded_width)][z]) // 3 add 
              + 1.3333333e-2F*(                                                     // 1 add + 1 mult
                  t0[idx(x-1,y,padded_width)][z] + t0[idx(x,y-1,padded_width)][z] + t0[idx(x,y,padded_width)][z-1]  // 2 add
                + t0[idx(x,y,padded_width)][z+1] + t0[idx(x,y+1,padded_width)][z] + t0[idx(x+1,y,padded_width)][z]) // 3 add
              - 7.49999983e-2F*t0[idx(x,y,padded_width)][z]                                                         // 1 add + 1 mult
            )
            /(r0*r2 + r1*damp[idx(x-padding,y-padding,worker_width)][z-padding]);                                   // 1 add + 2 mult + 1 div
        }
      }
    }

    return true;
  }
};