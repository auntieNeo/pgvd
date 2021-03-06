#ifdef __OPENCL_VERSION__
#include "opencl\C\ParallelAlgorithms.h"
#else
#include "ParallelAlgorithms.h"
#endif

#ifndef __OPENCL_VERSION__
#define __local
#define __global
#endif

//If the bit at the provided unsigned int matches compared with, the predicate buffer at n is set to 1. 0 otherwise.
void BitPredicate( __global BigUnsigned *inputBuffer, __global unsigned int *predicateBuffer, const unsigned int index, const unsigned char comparedWith, const int gid)
{
  BigUnsigned self = inputBuffer[gid];
   unsigned int x = (getBUBit(&self, index) == comparedWith);
   predicateBuffer[gid] = x;
}

//Unique Predication
//Requires input be sorted.
void UniquePredicate( __global BigUnsigned *inputBuffer, __global unsigned int *predicateBuffer, const int gid)
{
  if (gid == 0) {
    predicateBuffer[gid] = 1;
  } else {
    BigUnsigned self = inputBuffer[gid];
    BigUnsigned previous = inputBuffer[gid-1];
    predicateBuffer[gid] = (compareBU(&self, &previous) != 0);
  }
}

void StreamScan_Init(__global unsigned int* buffer, __local unsigned int* localBuffer, __local unsigned int* scratch, const int gid, const int lid)
{
  localBuffer[lid] = scratch[lid] = buffer[gid];
}

void AddAll(__local unsigned int* localBuffer, const int lid, const int powerOfTwo)
{
    if (lid < powerOfTwo) {
      localBuffer[lid] = localBuffer[lid + powerOfTwo] + localBuffer[lid];
    }
}

void HillesSteelScan(__local unsigned int* localBuffer, __local unsigned int* scratch, const int lid, const int powerOfTwo)
{
    if (lid > (powerOfTwo - 1))
      scratch[lid] = localBuffer[lid] + localBuffer[lid - powerOfTwo];
    else
      scratch[lid] = localBuffer[lid];
}

//result buffer MUST be initialized as 0!!!
void BUCompact( __global BigUnsigned *inputBuffer, __global BigUnsigned *resultBuffer, __global unsigned int *lPredicateBuffer, 
	__global unsigned int *leftBuffer, unsigned int size, const int gid)
{
  int a = leftBuffer[gid];
  int b = leftBuffer[size-2];
  int c = lPredicateBuffer[gid];
  int e = lPredicateBuffer[size - 1];

  //Check out http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html figure 39-14
  int t = gid - a + (e + b);
  int d = (!c) ? t : a - 1;

  //This really suffers from poor coalescing
#ifdef __OPENCL_VERSION__
  barrier(CLK_GLOBAL_MEM_FENCE);
#endif
  resultBuffer[d] = inputBuffer[gid];
}

void BUSingleCompact( __global BigUnsigned *inputBuffer, __global BigUnsigned *resultBuffer, __global unsigned int *predicateBuffer, __global unsigned int *addressBuffer, const int gid)
{
  unsigned int index;
  if (predicateBuffer[gid] == 1) {
    index = addressBuffer[gid];
    BigUnsigned temp = inputBuffer[gid];
    resultBuffer[index - 1] = temp;
  } 
}

#ifndef __OPENCL_VERSION__
  #include <stdlib.h>
  #include <stdio.h>
  #include <math.h>
  void StreamScan_SerialKernel(unsigned int* buffer, unsigned int* result, const int size) {
    int nextPowerOfTwo = (int)pow(2, ceil(log(size) / log(2)));
	  int intermediate = -1;
	  unsigned int* localBuffer;
	  unsigned int* scratch;
    unsigned int sum = 0;

	  localBuffer = (unsigned int*) malloc(sizeof(unsigned int)* nextPowerOfTwo);
	  scratch = (unsigned int*) malloc(sizeof(unsigned int)* nextPowerOfTwo);
    //INIT
    for (int i = 0; i < size; i++)
      StreamScan_Init(buffer, localBuffer, scratch, i, i);
    for (int i = size; i < nextPowerOfTwo; ++i) 
      localBuffer[i] = scratch[i] = 0;
    
    //Add not necessary with only one workgroup.
    //Adjacent sync not necessary with only one workgroup.

    //SCAN
    for (unsigned int i = 1; i < nextPowerOfTwo; i <<= 1) {
      for (int j = 0; j < nextPowerOfTwo; ++j) {
        HillesSteelScan(localBuffer, scratch, j, i);
      }
      __local unsigned int *tmp = scratch;
      scratch = localBuffer;
      localBuffer = tmp;
    }
    for (int i = 0; i < size; ++i) {
      result[i] = localBuffer[i];
    }
	  free(localBuffer);
	  free(scratch);
  }
#endif
#ifndef __OPENCL_VERSION__
#undef __local
#undef __global
#endif

