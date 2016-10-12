#include <cstdlib>

#include "Maze.h"

enum {
  RIGHT_WALL = 1 << 0,
  TOP_WALL = 1 << 1,
  LEFT_WALL = 1 << 2,
  BOTTOM_WALL = 1 << 3,
  VISITED = 1 << 4,
};

const char * wallString(int wall) {
#define WALL_STRING(w) \
  case w ## _WALL: \
    return #w "_WALL";
  switch (wall) {
    WALL_STRING(RIGHT)
    WALL_STRING(TOP)
    WALL_STRING(LEFT)
    WALL_STRING(BOTTOM)
  }
  return "UNKNOWN";
}

enum {
  RIGHT = 0,
  TOP,
  LEFT,
  BOTTOM,
};

Maze::Maze(int width, int height)
  : m_width(width), m_height(height)
{
  m_maze = (int *)malloc(sizeof(*m_maze) * width * height);

  m_generateMaze();

  m_printMaze();
}

Maze::~Maze() {
  free(m_maze);
}

Maze::Coord Maze::m_dirOffset(int dir) {
  Coord offset;
  switch (dir) {
    case RIGHT:
      offset.x = 1;
      offset.y = 0;
      break;
    case TOP:
      offset.x = 0;
      offset.y = 1;
      break;
    case LEFT:
      offset.x = -1;
      offset.y = 0;
      break;
    case BOTTOM:
      offset.x = 0;
      offset.y = -1;
      break;
  }
  return offset;
}

Maze::Coord Maze::m_wallOffset(int wall) {
  Coord offset;
  switch (wall) {
    case RIGHT_WALL:
      offset.x = 1;
      offset.y = 0;
      break;
    case TOP_WALL:
      offset.x = 0;
      offset.y = 1;
      break;
    case LEFT_WALL:
      offset.x = -1;
      offset.y = 0;
      break;
    case BOTTOM_WALL:
      offset.x = 0;
      offset.y = -1;
      break;
  }
  return offset;
}

int Maze::m_oppositeDir(int dir) {
  switch (dir) {
    case RIGHT:
      return LEFT;
    case TOP:
      return BOTTOM;
    case LEFT:
      return RIGHT;
    case BOTTOM:
      return TOP;
    default:
      assert(false);
  }
  return -1;
}

int Maze::m_oppositeWall(int wall) {
  switch (wall) {
    case RIGHT_WALL:
      return LEFT_WALL;
    case TOP_WALL:
      return BOTTOM_WALL;
    case LEFT_WALL:
      return RIGHT_WALL;
    case BOTTOM_WALL:
      return TOP_WALL;
    default:
      assert(false);
  }
  return -1;
}

int Maze::m_dirToWall(int dir) {
  return 1 << dir;
}

int Maze::m_ccwWall(int wall) {
  switch (wall) {
    case RIGHT_WALL:
      return TOP_WALL;
    case TOP_WALL:
      return LEFT_WALL;
    case LEFT_WALL:
      return BOTTOM_WALL;
    case BOTTOM_WALL:
      return RIGHT_WALL;
    default:
      assert(false);
  }
  return -1;
}

int Maze::m_cwWall(int wall) {
  switch (wall) {
    case RIGHT_WALL:
      return BOTTOM_WALL;
    case TOP_WALL:
      return RIGHT_WALL;
    case LEFT_WALL:
      return TOP_WALL;
    case BOTTOM_WALL:
      return LEFT_WALL;
    default:
      assert(false);
  }
  return -1;
}

int Maze::m_ccwDir(int dir) {
  switch (dir) {
    case RIGHT:
      return TOP;
      break;
    case TOP:
      return LEFT;
      break;
    case LEFT:
      return BOTTOM;
      break;
    case BOTTOM:
      return RIGHT;
      break;
    default:
      assert(false);
  }
  return -1;
}

int Maze::m_cwDir(int dir) {
  switch (dir) {
    case RIGHT:
      return BOTTOM;
      break;
    case TOP:
      return RIGHT;
      break;
    case LEFT:
      return TOP;
      break;
    case BOTTOM:
      return LEFT;
      break;
    default:
      assert(false);
  }
  return -1;
}

bool Maze::m_isValidCell(const Coord &cell) const {
  return
    (cell.x >= 0) &&
    (cell.x < m_width) &&
    (cell.y >= 0) &&
    (cell.y < m_height);
}

int &Maze::m_cellValue(const Coord &cell) const {
  assert(m_isValidCell(cell));
  return m_maze[cell.x + cell.y * m_width];
}

bool Maze::m_checkWall(const Coord &cell, int wall) const {
  assert(m_isValidCell(cell));
  bool result = m_cellValue(cell) & wall;
#ifndef NDEBUG
  /* Make sure the same wall is stored in the opposite cell */
  Coord oppositeCell = cell + m_wallOffset(wall);
  if (m_isValidCell(oppositeCell)) {
    bool oppositeResult = m_cellValue(oppositeCell) & m_oppositeWall(wall);
    assert(result == oppositeResult);
  }
#endif
  return result;
}

bool Maze::m_isOuterWall(const Coord &cell, int wall) const {
  switch (wall) {
    case RIGHT_WALL:
      if (cell.x == m_width - 1)
        return true;
      break;
    case TOP_WALL:
      if (cell.y == m_height - 1)
        return true;
      break;
    case LEFT_WALL:
      if (cell.x == 0)
        return true;
      break;
    case BOTTOM_WALL:
      if (cell.y == 0)
        return true;
      break;
  }
  return false;
}

void Maze::m_generateMaze() {
  /* Fill the maze with walls and unvisited cells */
  for (int i = 0; i < m_width * m_height; ++i) {
    m_maze[i] = RIGHT_WALL | TOP_WALL | LEFT_WALL | BOTTOM_WALL;
  }

  /* Start at an initial random point along the edge of the maze */
  if (rand() % 2) {
    /* Put the random point along the x-axis */
    m_initial.x = rand() % m_width;
    m_initial.y = rand() % 2 ? 0 : m_height - 1;
  } else {
    /* Put the random point along the y-axis */
    m_initial.x = rand() % 2 ? 0 : m_width - 1;
    m_initial.y = rand() % m_height;
  }

  /* Fill the maze with a random walk */
  m_randomWalkMaze(m_initial);

  /* TODO: Make a hole in the outer wall so that the maze can be entered */
}

void Maze::m_randomWalkMaze(const Maze::Coord &pos) {
  /* Mark this tile as visited */
  assert(!(m_cellValue(pos) & VISITED));
  m_cellValue(pos) |= VISITED;
  while (true) {
    /* Determine which dirctions are walkable */
    int walkable[4];
    int numWalkable = 0;
    for (int i = 0; i < 4; ++i) {
      Coord next = pos + m_dirOffset(i);
      if (!m_isValidCell(next))
        continue;
      if (m_cellValue(next) & VISITED)
        continue;
      walkable[numWalkable++] = i;
    }
    if (numWalkable == 0) {
      /* Recurse backwards to find a new path */
      return;
    }
    /* Randomly select a direction */
    int direction = walkable[rand() % numWalkable];
    /* Walk in this direction */
    Coord next = pos + m_dirOffset(direction);
    /* TODO: Remove the wall in this direction */
    m_cellValue(pos) &= ~m_dirToWall(direction);
    m_cellValue(next) &= ~m_dirToWall(m_oppositeDir(direction));
    m_randomWalkMaze(next);
  }
}

bool Maze::m_canWalk(
    const Maze::Coord &origin,
    int direction) const
{
  assert(origin.x >= 0);
  assert(origin.y >= 0);
  assert(origin.x < m_width);
  assert(origin.y < m_height);
  Coord destination = origin + m_dirOffset(direction);
  /* Check if the destination is within the maze */
  if (!m_isValidCell(destination))
    return false;
  /* Check if there is a wall between the destination tile and the
   * current tile */
  int wall = m_dirToWall(direction);
  return !m_checkWall(origin, wall);
}

#define SCALE 0.1f

void Maze::m_followWall(PolyLines *pl) const {
  /* I can pretty much start by following any wall. If I end up on the outside,
   * then I know that I need to do one of two things:
   *   - Continue to follow the outer wall if this is a wall follower
   *   - Loop Back if this is a path follower
   */
  Coord pos;
  int dir;
#define WALL (m_cwWall(m_dirToWall(dir)))
  pos = m_initial;
  /* Determine the initial wall and orientation */
  for (dir = 0; dir < 4; ++dir) {
    if (m_canWalk(pos, dir))
      break;
  }
  if (dir >= 4) {
    /* We couldn't seem to get anywhere from the initial cell */
    fprintf(stderr, "This is bad\n");
    return;
  }
  /* Start the maze line */
  pl->newLine({(float)pos.x * SCALE, (float)pos.y * SCALE});
  /* Iterate along the wall */
  /* FIXME: We need a better condition for this, as the maze can return to the
   * initial position more than once. */
  do {
    m_printMaze(&pos, dir);
    fprintf(stderr, "wall: %s\n", wallString(WALL));
    /* Check if we are still following a wall; this is so that we can
     * follow any walls that end abruptly */
    if (!(m_cellValue(pos) & WALL)) {
      fprintf(stderr, "We reached the end of this wall\n");
      /* TODO: Make a point in our corner of the cell */
      pl->addPoint({(float)pos.x * SCALE, (float)pos.y * SCALE});
      /* Make a right turn to follow a wall that has ended */
      dir = m_cwDir(dir);
      Coord next = pos + m_dirOffset(dir);
      assert(m_canWalk(pos, dir));  /* I think this might fail if we wander
                                       outside of the maze */
      pos = next;
      continue;
    }
    if (!m_canWalk(pos, dir)) {
      /* TODO: Generate a point in this corner of the cell */
      pl->addPoint({(float)pos.x * SCALE, (float)pos.y * SCALE});
      /* Turn left so that we can avoid the obstacle */
      dir = m_ccwDir(dir);
      continue;
    }
    /* Continue straight ahead */
    Coord next = pos + m_dirOffset(dir);
    pos = next;
  } while (pos != m_initial);
}

void Maze::m_printMaze(const Coord *marker, int direction) const {
  char cursor;
  switch (direction) {
    case RIGHT:
      cursor = '>';
      break;
    case TOP:
      cursor = '^';
      break;
    case LEFT:
      cursor = '<';
      break;
    case BOTTOM:
      cursor = 'V';
      break;
    default:
      cursor = 'X';
  }
  /* Iterate over the cells and print the visited state */
  Coord pos;
  for (int y = m_height - 1; y >= 0; --y) {
    pos.y = y;
    /* Print the top wall of each cell for this row */
    for (int x = 0; x < m_width; ++x) {
      pos.x = x;
      fprintf(stderr, " %s",
          m_cellValue(pos) & TOP_WALL ?
          "---" : "   ");
    }
    fprintf(stderr, "\n");
    /* Print the left and right walls for each cell in this row */
    for (int i = 0; i < 1; ++i) {
      for (int x = 0; x < m_width; ++x) {
        pos.x = x;
        fprintf(stderr, "%s %c ",
            m_cellValue(pos) & LEFT_WALL ?
            "|" : " ",
            marker && *marker == pos ?
            cursor : ' ');
        if (x == m_width - 1) {
          fprintf(stderr, "%s",
              m_cellValue(pos) & RIGHT_WALL ?
              "|" : " ");
        }
      }
      fprintf(stderr, "\n");
    }
    if (y == 0) {
      /* Print the remaining bottom wall of the last row */
      for (int x = 0; x < m_width; ++x) {
        pos.x = x;
        fprintf(stderr, " %s",
            m_cellValue(pos) & BOTTOM_WALL ?
            "---" : "   ");
      }
      fprintf(stderr, "\n");
    }
  }
}

void Maze::drawLines(PolyLines *pl) const {
  m_followWall(pl);
}
