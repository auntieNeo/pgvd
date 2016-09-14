#ifdef __OPENCL_VERSION__ 
#include "./OpenCL/C/BoundingBox/BoundingBox.h"
#else
#include "./BoundingBox.h"
#endif

void BB_initialize(BoundingBox *bb, const floatn* minimum, const floatn* maximum) {
  bb->initialized = true;
  copy_fvfv(&bb->minimum, minimum);
  copy_fvfv(&bb->maximum, maximum);
}

void BB_center(const BoundingBox *bb, floatn *center) {
  add_fvfv(center, &bb->minimum, &bb->maximum);
  div_fvf(center, center, 2.0F);
}

void BB_size(const BoundingBox *bb, floatn *size) {
  subt_fvfv(size, &bb->maximum, &bb->minimum);
}

void BB_max_size(const BoundingBox *bb, float *m) {
  floatn size;
  BB_size(bb, &size);
  max_in_fv(m, &size);
}

bool BB_contains_point(const BoundingBox *bb, floatn *point, const float epsilon) {
  for (int i = 0; i < DIM; ++i)
    if ((point->s[i] <= bb->minimum.s[i] - epsilon) ||
        (point->s[i] >= bb->maximum.s[i] + epsilon)) 
      return false;
  return true;
}

// Returns the smallest square bounding box that contains
// bb and has identical origin.
void BB_make_square(BoundingBox *result, const BoundingBox *bb) {
  floatn size;
  float dwidth;
  
  BB_size(bb, &size);
  max_in_fv(&dwidth, &size);
  BB_initialize(result, &bb->minimum, &bb->minimum);
  add_ffv(&result->maximum, dwidth, &result->maximum);
}

void BB_make_centered_square(BoundingBox *result, const BoundingBox *bb) {
  floatn size;
  float dwidth;
  BB_size(bb, &size);
  max_in_fv(&dwidth, &size);

  BB_initialize(result, &bb->minimum, &bb->minimum);
  add_ffv(&result->maximum, dwidth, &result->maximum);

  for (int i = 0; i < DIM; ++i) {
    result->minimum.s[i] -= (dwidth - size.s[i]) / 2;
    result->maximum.s[i] = result->minimum.s[i] + dwidth;
  }
  result->initialized = true;
}

void BB_scale(BoundingBox *result, const BoundingBox *bb, const float f) {
  floatn size;
  BB_size(bb, &size);
  mult_fvf(&size, &size, f);

  floatn newMax;
  copy_fvfv(&newMax, &bb->minimum);
  add_fvfv(&newMax, &newMax, &size);

  BB_initialize(result, &bb->minimum, &newMax);
}

void BB_scale_centered(BoundingBox *result, const BoundingBox *bb, const float f) {
  floatn s;
  BB_size(bb, &s);
  mult_fvf(&s, &s, f);
  div_fvf(&s, &s, 2.0F);

  floatn c;
  BB_center(bb, &c);

  floatn minp;
  subt_fvfv(&minp, &c, &s);
  
  floatn maxp;
  add_fvfv(&maxp, &c, &s);

  BB_initialize(&result, &minp, &maxp);
}

bool BB_is_square(const BoundingBox *bb) {
  floatn s;
  BB_size(bb, &s);
  float a = s.s[0];
  for (int i = 1; i < DIM; ++i) {
    if (s.s[i] != a) return false;
  }
  return true;
}