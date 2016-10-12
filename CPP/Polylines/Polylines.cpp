#include <cstdio>

#include "Polylines.h"

void PolyLines::readFile(const char *filename) {
  float x, y;
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Could not open file '%s': %s\n",
        filename,
        strerror(errno));
    return;
  }
  bool first = true;
  while (fscanf(fp, "%f %f", &x, &y) == 2) {
    if (first) {
      this->newLine({ x, y });
      first = false;
    } else {
      this->addPoint({ x, y });
    }
  }
  fclose(fp);
}
