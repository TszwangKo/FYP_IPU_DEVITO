#define _POSIX_C_SOURCE 200809L
#define START_TIMER(S)               \
  struct timeval start_##S, end_##S; \
  gettimeofday(&start_##S, NULL);
#define STOP_TIMER(S, T)        \
  gettimeofday(&end_##S, NULL); \
  T->S += (double)(end_##S.tv_sec - start_##S.tv_sec) + (double)(end_##S.tv_usec - start_##S.tv_usec) / 1000000;
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#include "stdlib.h"
#include "math.h"
#include "sys/time.h"
#include "xmmintrin.h"
#include "pmmintrin.h"
#include "omp.h"
#include <stdio.h>

struct dataobj
{
  void *restrict data;
  unsigned long *size;
  unsigned long *npsize;
  unsigned long *dsize;
  int *hsize;
  int *hofs;
  int *oofs;
  void *dmap;
};

struct profiler
{
  double section0;
};

int Kernel(struct dataobj *restrict damp_vec, struct dataobj *restrict u_vec, struct dataobj *restrict vp_vec, const float dt, const int time_M, const int time_m, const int x0_blk0_size, const int x_M, const int x_m, const int y0_blk0_size, const int y_M, const int y_m, const int z_M, const int z_m, const int nthreads, struct profiler *timers)
{
  float(*restrict damp)[damp_vec->size[1]][damp_vec->size[2]] __attribute__((aligned(64))) = (float(*)[damp_vec->size[1]][damp_vec->size[2]])damp_vec->data;
  float(*restrict u)[u_vec->size[1]][u_vec->size[2]][u_vec->size[3]] __attribute__((aligned(64))) = (float(*)[u_vec->size[1]][u_vec->size[2]][u_vec->size[3]])u_vec->data;
  float(*restrict vp)[vp_vec->size[1]][vp_vec->size[2]] __attribute__((aligned(64))) = (float(*)[vp_vec->size[1]][vp_vec->size[2]])vp_vec->data;

  /* Flush denormal numbers to zero in hardware */
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

  float r0 = 1.0F / (dt * dt);
  float r1 = 1.0F / dt;

  int count = 0;
  printf("time_m: %d\n", time_m);
  printf("time_M: %d\n", time_M);
  for (int time = time_m, t0 = (time) % (3), t1 = (time + 2) % (3), t2 = (time + 1) % (3); time <= time_M; time += 1, t0 = (time) % (3), t1 = (time + 2) % (3), t2 = (time + 1) % (3))
  {
    count++;
    /* Begin section0 */
    START_TIMER(section0)
#pragma omp parallel num_threads(nthreads)
    {
#pragma omp for collapse(2) schedule(dynamic, 1)
      for (int x0_blk0 = x_m; x0_blk0 <= x_M; x0_blk0 += x0_blk0_size)
      {
        for (int y0_blk0 = y_m; y0_blk0 <= y_M; y0_blk0 += y0_blk0_size)
        {
          for (int x = x0_blk0; x <= MIN(x_M, x0_blk0 + x0_blk0_size - 1); x += 1)
          {
            for (int y = y0_blk0; y <= MIN(y_M, y0_blk0 + y0_blk0_size - 1); y += 1)
            {
#pragma omp simd aligned(damp, u, vp : 32)
              for (int z = z_m; z <= z_M; z += 1)
              {
                float r2 = 1.0F / (vp[x + 4][y + 4][z + 4] * vp[x + 4][y + 4][z + 4]); // 1 * div + 1 * mult + 1 * store
                u[t2][x + 4][y + 4][z + 4] =                                           // 1 * store
                    (r1 * damp[x + 4][y + 4][z + 4] * u[t0][x + 4][y + 4][z + 4]       // 2 * mult
                     + r2 * (                                                          // 1 * add + 1 * mult
                                -r0 * (-2.0F * u[t0][x + 4][y + 4][z + 4])             // 2 * mult
                                - r0 * u[t1][x + 4][y + 4][z + 4]                      // 1 * add + 1 mult
                                ) +
                     8.33333315e-4F * (                                                                                          // 1 * add + 1 * mult
                                          -u[t0][x + 2][y + 4][z + 4] - u[t0][x + 4][y + 2][z + 4] - u[t0][x + 4][y + 4][z + 2]  // 2 * add
                                          - u[t0][x + 4][y + 4][z + 6] - u[t0][x + 4][y + 6][z + 4] - u[t0][x + 6][y + 4][z + 4] // 3 * add
                                          ) +
                     1.3333333e-2F * (                                                                                          // 1 * add + 1 * mult
                                         u[t0][x + 3][y + 4][z + 4] + u[t0][x + 4][y + 3][z + 4] + u[t0][x + 4][y + 4][z + 3]   // 2 * add
                                         + u[t0][x + 4][y + 4][z + 5] + u[t0][x + 4][y + 5][z + 4] + u[t0][x + 5][y + 4][z + 4] // 3 * add
                                         ) -
                     7.49999983e-2F * u[t0][x + 4][y + 4][z + 4] // 1 * add + 1 * mult
                     ) /
                    (r0 * r2 + r1 * damp[x + 4][y + 4][z + 4]); // 1 * div + 2 * mult + 1 * add 
              }
            }
          }
        }
      }
    }
    STOP_TIMER(section0, timers)
    /* End section0 */
  }
  printf("count: %d\n", count);

  return 0;
}
