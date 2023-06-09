#define _POSIX_C_SOURCE 200809L
#define START_TIMER(S) struct timeval start_ ## S , end_ ## S ; gettimeofday(&start_ ## S , NULL);
#define STOP_TIMER(S,T) gettimeofday(&end_ ## S, NULL); T->S += (double)(end_ ## S .tv_sec-start_ ## S.tv_sec)+(double)(end_ ## S .tv_usec-start_ ## S .tv_usec)/1000000;
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#include "stdlib.h"
#include "math.h"
#include "sys/time.h"
#include "xmmintrin.h"
#include "pmmintrin.h"

struct dataobj
{
  void *restrict data;
  unsigned long * size;
  unsigned long * npsize;
  unsigned long * dsize;
  int * hsize;
  int * hofs;
  int * oofs;
  void * dmap;
} ;

struct profiler
{
  double section0;
} ;


int Kernel(const float a, struct dataobj *restrict u_vec, const float dt, const float h_x, const float h_y, const float h_z, const int time_M, const int time_m, const int x0_blk0_size, const int x_M, const int x_m, const int y0_blk0_size, const int y_M, const int y_m, const int z_M, const int z_m, struct profiler * timers)
{
  float (*restrict u)[u_vec->size[1]][u_vec->size[2]][u_vec->size[3]] __attribute__ ((aligned (64))) = (float (*)[u_vec->size[1]][u_vec->size[2]][u_vec->size[3]]) u_vec->data;

  /* Flush denormal numbers to zero in hardware */
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

  float r0 = 1.0F/dt;
  float r1 = 1.0F/(h_x*h_x);
  float r2 = 1.0F/(h_y*h_y);
  float r3 = 1.0F/(h_z*h_z);

  for (int time = time_m, t0 = (time)%(2), t1 = (time + 1)%(2); time <= time_M; time += 1, t0 = (time)%(2), t1 = (time + 1)%(2))
  {
    /* Begin section0 */
    START_TIMER(section0)
    for (int x0_blk0 = x_m; x0_blk0 <= x_M; x0_blk0 += x0_blk0_size)
    {
      for (int y0_blk0 = y_m; y0_blk0 <= y_M; y0_blk0 += y0_blk0_size)
      {
        for (int x = x0_blk0; x <= MIN(x_M, x0_blk0 + x0_blk0_size - 1); x += 1)
        {
          for (int y = y0_blk0; y <= MIN(y_M, y0_blk0 + y0_blk0_size - 1); y += 1)
          {
            #pragma omp simd aligned(u:32)
            for (int z = z_m; z <= z_M; z += 1)
            {
              float r4 = -2.84722222F*u[t0][x + 8][y + 8][z + 8];
              u[t1][x + 8][y + 8][z + 8] = dt*(a*(
                    r1 * ( r4 + (-1.78571429e-3F)*(u[t0][x + 4][y + 8][z + 8] + u[t0][x + 12][y + 8][z + 8]) 
                        + 2.53968254e-2F*(u[t0][x + 5][y + 8][z + 8] + u[t0][x + 11][y + 8][z + 8]) 
                        + (-2.0e-1F)*(u[t0][x + 6][y + 8][z + 8] + u[t0][x + 10][y + 8][z + 8]) 
                        + 1.6F*(u[t0][x + 7][y + 8][z + 8] + u[t0][x + 9][y + 8][z + 8])
                    ) 
                    + r2 * ( r4 + (-1.78571429e-3F)*(u[t0][x + 8][y + 4][z + 8] + u[t0][x + 8][y + 12][z + 8]) 
                            + 2.53968254e-2F*(u[t0][x + 8][y + 5][z + 8] + u[t0][x + 8][y + 11][z + 8]) 
                            + (-2.0e-1F)*(u[t0][x + 8][y + 6][z + 8] + u[t0][x + 8][y + 10][z + 8]) 
                            + 1.6F*(u[t0][x + 8][y + 7][z + 8] + u[t0][x + 8][y + 9][z + 8])) 
                    + r3 * ( r4 + (-1.78571429e-3F)*(u[t0][x + 8][y + 8][z + 4] + u[t0][x + 8][y + 8][z + 12]) 
                        + 2.53968254e-2F*(u[t0][x + 8][y + 8][z + 5] + u[t0][x + 8][y + 8][z + 11]) 
                        + (-2.0e-1F)*(u[t0][x + 8][y + 8][z + 6] + u[t0][x + 8][y + 8][z + 10]) 
                        + 1.6F*(u[t0][x + 8][y + 8][z + 7] + u[t0][x + 8][y + 8][z + 9])
                    )) + r0*u[t0][x + 8][y + 8][z + 8]);
            }
          }
        }
      }
    }
    STOP_TIMER(section0,timers)
    /* End section0 */
  }

  return 0;
}
