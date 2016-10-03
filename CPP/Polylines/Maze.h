#ifndef PGVD_POLYLINES_MAZE_H_
#define PGVD_POLYLINES_MAZE_H_

#include "Polylines.h"

class Maze : public PolyLines {
  private:
    int *m_maze;
    int m_width, m_height;

    void m_generateMaze();
    void m_randomWalkMaze(int x, int y);
    bool m_canWalk(int x0, int y0, int x1, int y1);

  public:
    Maze(int width, int height);
    ~Maze();
};

#endif
