#include <cstdlib>

#include "Maze.h"

enum {
  RIGHT_WALL = 1 << 0,
  TOP_WALL = 1 << 1,
  LEFT_WALL = 1 << 2,
  BOTTOM_WALL = 1 << 3,
  VISITED = 1 << 4,
};

Maze::Maze(int width, int height)
  : m_width(width), m_height(height)
{
  m_maze = (int *)malloc(sizeof(*m_maze) * width * height);

  m_generateMaze();
}

Maze::~Maze() {
  free(m_maze);
}

void Maze::m_generateMaze() {
  /* Start at an initial random point along the edge of the maze */
  int initial_x, initial_y;
  if (rand() % 2) {
    /* Put the random point along the x-axis */
    initial_x = rand() % m_width;
    initial_y = rand() % 2 ? 0 : m_height - 1;
  } else {
    /* Put the random point along the y-axis */
    initial_x = rand() % 2 ? 0 : m_width - 1;
    initial_y = rand() % m_height;
  }

  m_randomWalkMaze(initial_x, initial_y);
}

void Maze::m_randomWalkMaze(int x, int y) {
  /* Mark this tile as visited */
  assert(!(m_maze[x + y * m_width] & VISITED));
  m_maze[x + y * m_width] |= VISITED;
  while (true) {
    /* Determine which dirctions are walkable */
    int walkable[4];
    int numWalkable = 0;
    for (int i = 0; i < 4; ++i) {
      int next_x, next_y;
      next_x = x + (i & (1 << 0) ? 1 : -1);
      next_y = y + (i & (1 << 1) ? 1 : -1);
      if (m_canWalk(x, y, next_x, next_y)) {
        walkable[numWalkable++] = i;
      }
    }
    if (numWalkable == 0) {
      /* Recurse backwards to find a new path */
      return;
    }
    /* Randomly select a direction */
    int direction = walkable[rand() % numWalkable];
    /* Walk in this direction */
    int next_x = x + (direction & (1 << 0) ? 1 : -1);
    int next_y = y + (direction & (1 << 1) ? 1 : -1);
    m_randomWalkMaze(next_x, next_y);
  }
}

bool Maze::m_canWalk(int x0, int y0, int x1, int y1) {
  assert(x0 >= 0);
  assert(y0 >= 0);
  assert(x0 < m_width);
  assert(y0 < m_height);
  int origin = m_maze[x0 + y0 * m_width];
  /* Check if the destination is within the maze */
  if ((x1 < 0) ||
      (y1 < 0) ||
      (x1 >= m_width) ||
      (y1 >= m_height))
  {
    return false;
  }
  int destination = m_maze[x1 + y1 * m_width];
  /* Check if the destination tile has already been visited */
  if (destination & VISITED) {
    return false;
  }
  /* Check if there is a wall between the destination tile and the
   * current tile */
  int wall;
  if (x1 < x0) {
    assert(y0 == y1);
    wall = LEFT_WALL;
  } else if (x1 > x0) {
    assert(y0 == y1);
    wall = RIGHT_WALL;
  } else if (y1 < y0) {
    assert(x0 == x1);
    wall = BOTTOM_WALL;
  } else if (y1 > y0) {
    assert(x0 == x1);
    wall = TOP_WALL;
  } else {
    assert(false);
  }
#define CHECK_WALL(wall, opposite) \
  if (origin & wall) { \
    assert(destination & opposite); \
    return false; \
  } else { \
    assert(!(destination & opposite)); \
    return true; \
  }
  switch (wall)
  {
    case BOTTOM_WALL:
      CHECK_WALL(BOTTOM_WALL, TOP_WALL)
    case RIGHT_WALL:
      CHECK_WALL(RIGHT_WALL, LEFT_WALL)
    case TOP_WALL:
      CHECK_WALL(TOP_WALL, BOTTOM_WALL)
    case LEFT_WALL:
      CHECK_WALL(LEFT_WALL, RIGHT_WALL)
    default:
        assert(false);
  }
  return false;
}
