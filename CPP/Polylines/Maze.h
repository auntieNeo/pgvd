#ifndef PGVD_POLYLINES_MAZE_H_
#define PGVD_POLYLINES_MAZE_H_

#include "Polylines.h"

class Maze : public PolyLines {
  private:
    typedef struct Coord {
      int x, y;

      struct Coord operator+(const Coord &other) const {
        Coord result;
        result.x = this->x + other.x;
        result.y = this->y + other.y;
        return result;
      }

      bool operator==(const Coord &other) const {
        return (this->x == other.x) && (this->y == other.y);
      }

      bool operator!=(const Coord &other) const {
        return !(*this == other);
      }
    } Coord;

    int *m_maze;
    int m_width, m_height;
    Coord m_initial;

    static Coord m_dirOffset(int dir);
    static Coord m_wallOffset(int wall);
    static int m_oppositeDir(int dir);
    static int m_oppositeWall(int wall);
    static int m_dirToWall(int dir);
    static int m_ccwWall(int wall);
    static int m_cwWall(int wall);
    static int m_ccwDir(int dir);
    static int m_cwDir(int dir);

    bool m_isValidCell(const Coord &cell) const;
    int &m_cellValue(const Coord &cell) const;
    bool m_checkWall(const Coord &cell, int wall) const;
    bool m_isOuterWall(const Coord &cell, int wall) const;

    void m_generateMaze();
    void m_randomWalkMaze(const Coord &pos);
    bool m_canWalk(const Coord &origin, int direction) const;

    void m_followWall(PolyLines *pl) const;

    void m_printMaze(const Coord *marker = NULL, int direction = -1) const;

  public:
    Maze(int width, int height);
    ~Maze();

    void drawLines(PolyLines *pl) const;
};


#endif
