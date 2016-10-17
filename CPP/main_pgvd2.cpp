#include <iostream>
#include <getopt.h>
#include "clfw.hpp"
#include "GLUtilities/gl_utils.h"
#include "Events/events.h"
#include "Shaders/Shaders.hpp"
#include "Polylines/Polylines.h"
#include "Polylines/Maze.h"
#include "GlobalData/data.h"
#include "Options/Options.h"

void InitializeGLFWEventCallbacks() {
  using namespace GLUtilities;
  glfwSetKeyCallback(window, key_cb);
  glfwSetMouseButtonCallback(window, mouse_cb);
  glfwSetCursorPosCallback(window, mouse_move_cb);
  glfwSetWindowSizeCallback(window, resize_cb);
  glfwSetWindowFocusCallback(window, focus_cb);
}
void InitializeGLFW(int width = 512, int height = 512) {
  using namespace GLUtilities;
  GLUtilities::window_height = height;
  GLUtilities::window_width = width;
  restart_gl_log();
  start_gl();
  print_error("new a");
  glfwSetWindowTitle(window, "Parallel GVD");
  InitializeGLFWEventCallbacks();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glfwSwapBuffers(window);
}

int main(int argc, char** argv) {
  using namespace std;
  bool batchMode = false;

  InitializeGLFW();
  Shaders::create();
  Data::lines = new PolyLines();

  /* Parse arguments */
  struct option long_options[] = {
    {"batch", no_argument, NULL, 'b'},
    {"maze", required_argument, NULL, 'm'},
    {NULL, 0, NULL, 0},
  };
  while (true) {
    int option_index = 0;
    int opt = getopt_long(
        argc, argv,
        "bm:",  /* optstring */
        long_options,  /* longopts */
        &option_index  /* longindex */
        );
    if (opt == -1) {
      break;
    }
    switch (opt) {
      case 'b':
        /* Enable batch mode */
        batchMode = true;
        break;
      case 'm':
        {
          int mazeWidth, mazeHeight;
          /* Draw a maze with the given dimensions */
          assert(optarg != NULL);
          sscanf(optarg, "%dx%d", &mazeWidth, &mazeHeight);
          Maze maze(mazeWidth, mazeHeight);
          maze.drawLines(Data::lines);
        }
        break;
    }
  }

  /* Initialize */
  CLFW::Initialize(
      false,  /* verbose */
      !batchMode,  /* queryMode */
      2  /* numQueues */
      );

  Options::showObjects = true;
  Options::showOctree = true;
  Options::max_level = 6;

  Data::octree = new Octree2();

  if (batchMode) {
    /* TODO: Output batch mode performance statistics */
    Timer t;
    Data::octree->build(Data::lines);
    t.stop();
    fprintf(stdout, "Number of lines: %d\n",
        Data::lines->lineCount());
  } else {
    /* Event loop */
    while (!glfwWindowShouldClose(GLUtilities::window)) {
      glfwPollEvents();
      refresh();
    }
    glfwTerminate();
  }
  return 0;
}
