#define _POSIX_C_SOURCE 200809L
#define START_TIMER(S) struct timeval start_ ## S , end_ ## S ; gettimeofday(&start_ ## S , NULL);
#define STOP_TIMER(S,T) gettimeofday(&end_ ## S, NULL); T->S += (double)(end_ ## S .tv_sec-start_ ## S.tv_sec)+(double)(end_ ## S .tv_usec-start_ ## S .tv_usec)/1000000;

#include "stdlib.h"
#include "math.h"
#include "sys/time.h"
#include "xmmintrin.h"
#include "pmmintrin.h"
#include "omp.h"

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
  double section1;
  double section2;
  double section3;
  double section4;
  double section5;
  double section6;
  double section7;
  double section8;
  double section9;
} ;


int initdamp(struct dataobj *restrict damp_vec, const int abc_x_l_ltkn, const int abc_x_r_rtkn, const int abc_y_l_ltkn, const int abc_y_r_rtkn, const int abc_z_l_ltkn, const int abc_z_r_rtkn, const float h_x, const float h_y, const float h_z, const int x_M, const int x_m, const int y_M, const int y_m, const int z_M, const int z_m, const int nthreads, const int x_size, const int y_size, const int z_size, struct profiler * timers)
{
  float *restrict r0_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r0_vec),64,x_size*sizeof(float));
  float *restrict r1_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r1_vec),64,x_size*sizeof(float));
  float *restrict r10_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r10_vec),64,z_size*sizeof(float));
  float *restrict r11_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r11_vec),64,z_size*sizeof(float));
  float *restrict r2_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r2_vec),64,x_size*sizeof(float));
  float *restrict r3_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r3_vec),64,x_size*sizeof(float));
  float *restrict r4_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r4_vec),64,y_size*sizeof(float));
  float *restrict r5_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r5_vec),64,y_size*sizeof(float));
  float *restrict r6_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r6_vec),64,y_size*sizeof(float));
  float *restrict r7_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r7_vec),64,y_size*sizeof(float));
  float *restrict r8_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r8_vec),64,z_size*sizeof(float));
  float *restrict r9_vec __attribute__ ((aligned (64)));
  posix_memalign((void**)(&r9_vec),64,z_size*sizeof(float));

  float (*restrict damp)[damp_vec->size[1]][damp_vec->size[2]] __attribute__ ((aligned (64))) = (float (*)[damp_vec->size[1]][damp_vec->size[2]]) damp_vec->data;
  float (*restrict r0) __attribute__ ((aligned (64))) = (float (*)) r0_vec;
  float (*restrict r1) __attribute__ ((aligned (64))) = (float (*)) r1_vec;
  float (*restrict r10) __attribute__ ((aligned (64))) = (float (*)) r10_vec;
  float (*restrict r11) __attribute__ ((aligned (64))) = (float (*)) r11_vec;
  float (*restrict r2) __attribute__ ((aligned (64))) = (float (*)) r2_vec;
  float (*restrict r3) __attribute__ ((aligned (64))) = (float (*)) r3_vec;
  float (*restrict r4) __attribute__ ((aligned (64))) = (float (*)) r4_vec;
  float (*restrict r5) __attribute__ ((aligned (64))) = (float (*)) r5_vec;
  float (*restrict r6) __attribute__ ((aligned (64))) = (float (*)) r6_vec;
  float (*restrict r7) __attribute__ ((aligned (64))) = (float (*)) r7_vec;
  float (*restrict r8) __attribute__ ((aligned (64))) = (float (*)) r8_vec;
  float (*restrict r9) __attribute__ ((aligned (64))) = (float (*)) r9_vec;

  /* Flush denormal numbers to zero in hardware */
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

  /* Begin section0 */
  START_TIMER(section0)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for collapse(2) schedule(static,1)
    for (int x = x_m; x <= x_M; x += 1)
    {
      for (int y = y_m; y <= y_M; y += 1)
      {
        #pragma omp simd aligned(damp:32)
        for (int z = z_m; z <= z_M; z += 1)
        {
          damp[x + 4][y + 4][z + 4] = 0.0F;
        }
      }
    }
  }
  STOP_TIMER(section0,timers)
  /* End section0 */

  /* Begin section1 */
  START_TIMER(section1)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(static,1)
    for (int abc_x_l = x_m; abc_x_l <= abc_x_l_ltkn + x_m - 1; abc_x_l += 1)
    {
      float r18 = fabs(-1.0e-1F*abc_x_l + 1.0e-1F*x_m + 1.1F);
      r0[abc_x_l] = r18;
      r1[abc_x_l] = sin(6.28318530717959F*r18);
    }
  }
  STOP_TIMER(section1,timers)
  /* End section1 */

  float r12 = 1.0F/h_x;
  /* Begin section2 */
  START_TIMER(section2)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for collapse(2) schedule(static,1)
    for (int abc_x_l = x_m; abc_x_l <= abc_x_l_ltkn + x_m - 1; abc_x_l += 1)
    {
      for (int y = y_m; y <= y_M; y += 1)
      {
        #pragma omp simd aligned(damp:32)
        for (int z = z_m; z <= z_M; z += 1)
        {
          damp[abc_x_l + 4][y + 4][z + 4] += r12*(1.03616329184732F*r0[abc_x_l] - 1.64910509747871e-1F*r1[abc_x_l]);
        }
      }
    }
  }
  STOP_TIMER(section2,timers)
  /* End section2 */

  /* Begin section3 */
  START_TIMER(section3)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(static,1)
    for (int abc_x_r = -abc_x_r_rtkn + x_M + 1; abc_x_r <= x_M; abc_x_r += 1)
    {
      float r19 = fabs(1.0e-1F*abc_x_r - 1.0e-1F*x_M + 1.1F);
      r2[abc_x_r] = r19;
      r3[abc_x_r] = sin(6.28318530717959F*r19);
    }
  }
  STOP_TIMER(section3,timers)
  /* End section3 */

  float r13 = 1.0F/h_x;

  /* Begin section4 */
  START_TIMER(section4)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for collapse(2) schedule(static,1)
    for (int abc_x_r = -abc_x_r_rtkn + x_M + 1; abc_x_r <= x_M; abc_x_r += 1)
    {
      for (int y = y_m; y <= y_M; y += 1)
      {
        #pragma omp simd aligned(damp:32)
        for (int z = z_m; z <= z_M; z += 1)
        {
          damp[abc_x_r + 4][y + 4][z + 4] += r13*(1.03616329184732F*r2[abc_x_r] - 1.64910509747871e-1F*r3[abc_x_r]);
        }
      }
    }
  }
  STOP_TIMER(section4,timers)
  /* End section4 */

  float r20 = 1.0F/h_z;
  float r17 = r20;
  float r16 = r20;
  float r21 = 1.0F/h_y;
  float r15 = r21;
  float r14 = r21;
  /* Begin section5 */
  START_TIMER(section5)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(static,1)
    for (int abc_z_r = -abc_z_r_rtkn + z_M + 1; abc_z_r <= z_M; abc_z_r += 1)
    {
      float r22 = fabs(1.0e-1F*abc_z_r - 1.0e-1F*z_M + 1.1F);
      r10[abc_z_r] = r22;
      r11[abc_z_r] = sin(6.28318530717959F*r22);
    }
  }
  STOP_TIMER(section5,timers)
  /* End section5 */

  /* Begin section6 */
  START_TIMER(section6)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(static,1)
    for (int abc_z_l = z_m; abc_z_l <= abc_z_l_ltkn + z_m - 1; abc_z_l += 1)
    {
      float r23 = fabs(-1.0e-1F*abc_z_l + 1.0e-1F*z_m + 1.1F);
      r8[abc_z_l] = r23;
      r9[abc_z_l] = sin(6.28318530717959F*r23);
    }
  }
  STOP_TIMER(section6,timers)
  /* End section6 */

  /* Begin section7 */
  START_TIMER(section7)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(static,1)
    for (int abc_y_r = -abc_y_r_rtkn + y_M + 1; abc_y_r <= y_M; abc_y_r += 1)
    {
      float r24 = fabs(1.0e-1F*abc_y_r - 1.0e-1F*y_M + 1.1F);
      r6[abc_y_r] = r24;
      r7[abc_y_r] = sin(6.28318530717959F*r24);
    }
  }
  STOP_TIMER(section7,timers)
  /* End section7 */

  /* Begin section8 */
  START_TIMER(section8)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(static,1)
    for (int abc_y_l = y_m; abc_y_l <= abc_y_l_ltkn + y_m - 1; abc_y_l += 1)
    {
      float r25 = fabs(-1.0e-1F*abc_y_l + 1.0e-1F*y_m + 1.1F);
      r4[abc_y_l] = r25;
      r5[abc_y_l] = sin(6.28318530717959F*r25);
    }
  }
  STOP_TIMER(section8,timers)
  /* End section8 */

  /* Begin section9 */
  START_TIMER(section9)
  #pragma omp parallel num_threads(nthreads)
  {
    #pragma omp for schedule(dynamic,1)
    for (int x = x_m; x <= x_M; x += 1)
    {
      for (int abc_y_l = y_m; abc_y_l <= abc_y_l_ltkn + y_m - 1; abc_y_l += 1)
      {
        #pragma omp simd aligned(damp:32)
        for (int z = z_m; z <= z_M; z += 1)
        {
          damp[x + 4][abc_y_l + 4][z + 4] += r14*(1.03616329184732F*r4[abc_y_l] - 1.64910509747871e-1F*r5[abc_y_l]);
        }
      }
      for (int abc_y_r = -abc_y_r_rtkn + y_M + 1; abc_y_r <= y_M; abc_y_r += 1)
      {
        #pragma omp simd aligned(damp:32)
        for (int z = z_m; z <= z_M; z += 1)
        {
          damp[x + 4][abc_y_r + 4][z + 4] += r15*(1.03616329184732F*r6[abc_y_r] - 1.64910509747871e-1F*r7[abc_y_r]);
        }
      }
      for (int y = y_m; y <= y_M; y += 1)
      {
        #pragma omp simd aligned(damp:32)
        for (int abc_z_l = z_m; abc_z_l <= abc_z_l_ltkn + z_m - 1; abc_z_l += 1)
        {
          damp[x + 4][y + 4][abc_z_l + 4] += r16*(1.03616329184732F*r8[abc_z_l] - 1.64910509747871e-1F*r9[abc_z_l]);
        }
        #pragma omp simd aligned(damp:32)
        for (int abc_z_r = -abc_z_r_rtkn + z_M + 1; abc_z_r <= z_M; abc_z_r += 1)
        {
          damp[x + 4][y + 4][abc_z_r + 4] += r17*(1.03616329184732F*r10[abc_z_r] - 1.64910509747871e-1F*r11[abc_z_r]);
        }
      }
    }
  }
  STOP_TIMER(section9,timers)
  /* End section9 */

  free(r0_vec);
  free(r1_vec);
  free(r10_vec);
  free(r11_vec);
  free(r2_vec);
  free(r3_vec);
  free(r4_vec);
  free(r5_vec);
  free(r6_vec);
  free(r7_vec);
  free(r8_vec);
  free(r9_vec);

  return 0;
}
