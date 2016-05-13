#include "./Karras.h"

#include <iostream>
#include <algorithm>
#include <memory>
#include <string>

#include "./BoundingBox.h"
#include "opencl/CLWrapper/CLWrapper.h"
// #include "./gpu.h"

extern "C" {
  #include "C/ParallelAlgorithms.h"
	#include "C/BuildOctree.h"
}
using std::cout;
using std::endl;
using std::vector;
using std::shared_ptr;

// kWidth is the quantized width in one dimension.
// Number of bits per dimension is kNumBits = log(kWidth).
// Total number of bits for morton code is kNumBits * DIM.
// static const int kWidth = 8;
// static const int kNumBits = 3;

namespace Karras {
  CLWrapper CL(256, 256);
BigUnsigned* xyz2z(BigUnsigned *result, intn p, const Resln* resln) {
  initBlkBU(result, 0);
	BigUnsigned temp;
  initBlkBU(&temp, 0);
  BigUnsigned *tempb = new BigUnsigned;
  initBlkBU(tempb, 0);

	for (int i = 0; i < resln->bits; ++i) {
    for (int j = 0; j < DIM; ++j) {
			if (p.s[j] & (1 << i)) {
				//ret |= BigUnsigned(1) << (i*DIM + j);
				initBlkBU(&temp, 1);
				shiftBULeft(tempb, &temp, i*DIM + j);
				initBUBU(&temp, result);
				orBU(result, &temp, tempb);
			}
    }
  }
  delete tempb;
  //free(tempb);
  return result;
}

intn z2xyz(BigUnsigned *z, const Resln* resln) {
  intn p = make_intn(0);
	BigUnsigned temp, tempb;
	BigUnsigned zero;
	initBlkBU(&zero, 0);
  for (int i = 0; i < resln->bits; ++i) {
    for (int j = 0; j < DIM; ++j) {
      //if ((z & (BigUnsigned(1) << (i*DIM+j))) > 0)
			initBlkBU(&temp, 1);
			shiftBULeft(&tempb, &temp, i*DIM + j);
			andBU(&temp, z, &tempb);
			if (compareBU(&temp, &zero) > 0) {
				p.s[j] |= (1 << i);
			}
    }
  }
  return p;
}

inline std::string buToString(BigUnsigned bu) {
	std::string representation = "";
	if (bu.len == 0)
	{
    if (bu.isNULL) {
      representation += "[NULL]";
    }
    else {
      representation += "[0]";
    }
	}
	else {
		for (int i = bu.len; i > 0; --i) {
			representation += "[" + std::to_string(bu.blk[i - 1]) + "]";
		}
	}

	return representation;
}

bool lessThanBigUnsigned(BigUnsigned& a, BigUnsigned&b) {
	if (compareBU(&a, &b) == -1) {
		return 1;
	}
	return 0;
}
bool equalsBigUnsigned(BigUnsigned& a, BigUnsigned &b) {
	if (compareBU(&a, &b) == 0) {
		return 1;
	}
	return 0;
}

// Needs to be implemented
void sort_points(BigUnsigned* mpoints, const int n) {
  /*int nextPowerOfTwo = pow(2, ceil(log(n) / log(2)));
  printf("Next power of two is: %d\n", nextPowerOfTwo);
  int difference = nextPowerOfTwo - n;
  BigUnsigned* mpoints = BigUnsigned[n + difference];
  for (int i = 0; i < difference; i++)
  {
    mpoints[n + i]
  }*/
}

// dwidth is passed in for performance reasons. It is equal to
//   float dwidth = bb.max_size();
intn Quantize(
    const floatn& p, const Resln& resln,
    const BoundingBox<floatn>& bb, const float dwidth, const bool clamped) {
  intn q = make_intn(0);
  int effectiveWidth = resln.width-1;
  if (clamped) {
    effectiveWidth = resln.width;
  }
  for (int k = 0; k < DIM; ++k) {
    // const double d =
    //     (resln.width-1) * ((p.s[k] - bb.min().s[k]) / dwidth);
    const double d =
        effectiveWidth * ((p.s[k] - bb.min().s[k]) / dwidth);
    const int v = static_cast<int>(d+0.5);
    if (v < 0) {
      cerr << "Coordinate in dimension " << k << " is less than zero.  d = "
           << d << " v = " << v << endl;
      cerr << "  p[k] = " << p.s[k]
           << " bb.min()[k] = " << bb.min().s[k] << endl;
      cerr << "  dwidth = " << dwidth << " kwidth = " << resln.width << endl;
      throw logic_error("bad coordinate");
    }
    q.s[k] = v;
  }
  return q;
}

vector<intn> Quantize(
    const vector<floatn>& points, const Resln& resln,
    const BoundingBox<floatn>* customBB, const bool clamped) {
  if (points.empty())
    return vector<intn>();

  BoundingBox<floatn> bb;
  if (customBB) {
    bb = *customBB;
  } else {
    for (const floatn& p : points) {
      bb(p);
    }
  }
  const float dwidth = bb.max_size();
  if (dwidth == 0) {
    vector<intn> ret;
    ret.push_back(make_intn(0));
    return ret;
  }

  // Quantize points to integers
  vector<intn> qpoints(points.size());
  for (int i = 0; i < points.size(); ++i) {
    const floatn& p = points[i];
    const intn q = Quantize(p, resln, bb, dwidth, clamped);
    qpoints[i] = q;
  }
  
  return qpoints;
}

vector<OctNode> BuildOctree(
    const vector<intn>& points, const Resln& resln, const bool verbose) {
  if (points.empty())
    throw logic_error("Zero points not supported");

  //Here, we're making a buffer vector containing the input we'd like to sort.
  int n = points.size();
  int nextPowerOfTwo = max( (int) pow( 2, ceil( log( n ) / log( 2 ) ) ), 8 );
  if (verbose)
    cout << "Next Power of Two: " << nextPowerOfTwo << endl;
  if (!CL.isBufferUsable(CL.buffers.bigUnsignedInput, sizeof(BigUnsigned)* (nextPowerOfTwo)))
    CL.buffers.bigUnsignedInput = CL.createBuffer(nextPowerOfTwo*sizeof(BigUnsigned));
  
  //In this case, we're sorting Z-Order points. 
  //We pad with 0's that will be culled using unique.
  BigUnsigned* mpoints = (BigUnsigned*)CL.buffers.bigUnsignedInput->map_buffer();
  for (int i = 0; i < points.size(); ++i) 
    xyz2z(&mpoints[i], points[i], &resln);
  for (int i = points.size(); i < nextPowerOfTwo; i++) 
    initBlkBU(&mpoints[i], 0);  

  CL.buffers.bigUnsignedInput->unmap_buffer();
  CL.RadixSort(resln.mbits);
  n = CL.UniqueSorted();

  //Using the unique, sorted morton numbers, we construct a binary radix tree in parallel.
  CL.buildBrt(n, resln.mbits);
  return CL.BRT2Octree(n);
}

// Debug output
// void OutputOctreeNode(
//     const int node, const std::vector<OctNode>& octree, vector<int> path) {
void OutputOctreeNode(
    const int node, const OctNode* octree, vector<int> path) {
  for (int i = 0; i < 4; ++i) {
    vector<int> p(path);
    p.push_back(i);

    for (int i : p) {
      cout << i;
    }
    cout << endl;

    if (!is_leaf(&octree[node], i))
      OutputOctreeNode(octree[node][i], octree, p);
  }
}

// void OutputOctree(const std::vector<OctNode>& octree) {
void OutputOctree(const OctNode* octree, const int n) {
  // if (!octree.empty()) {
  if (n > 0) {
    cout << endl;
    vector<int> p;
    p.push_back(0);
    OutputOctreeNode(0, octree, p);
  }
}

} // namespace
