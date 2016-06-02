#pragma once
using namespace std;
#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include <CL/opencl.h>
#endif // __APPLE__

#include <vector>
#include "../../C/BigUnsigned.h"
#include <iostream>

/*
  BufferVector. 
    -Handles device memory creation and deletion
  Nate B/V - bitinat2@isu.edu
*/
class Buffer
{
  private:
    cl_context context;
    cl_command_queue queue;
    cl_mem buffer;
    void* pointer;
    size_t size;

  public:
    Buffer(size_t _size, cl_context _context, cl_command_queue _queue) {
      if (_size == 0)
        throw std::invalid_argument("Buffer size must be greater than 0.");
      context = _context;
      queue = _queue;
      size = _size;
      pointer = nullptr;
      buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, NULL);
    }
    size_t getSize() {
      return size;
    }
    cl_mem getBuffer() {
      return buffer;
    }

    //TODO remove this function.
    void* map_buffer() {
      cl_int error;
      if (pointer != nullptr)
        throw std::invalid_argument("Buffer already mapped!");
      pointer = clEnqueueMapBuffer(queue, buffer, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, size, 0, NULL, NULL, &error);
      if (error != CL_SUCCESS) {
        cout << "Can't map buffer!: " << error << endl;
      }
      return pointer;
    }

    cl_int map_buffer(void* &pointer) {
      cl_int error;
      pointer = clEnqueueMapBuffer(queue, buffer, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, size, 0, NULL, NULL, &error);
      return error;
    }

    void* map_buffer(size_t givenSize) {
      cl_int error;
      if (pointer != nullptr)
        throw std::invalid_argument("Buffer already mapped!");
      pointer = clEnqueueMapBuffer(queue, buffer, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, givenSize, 0, NULL, NULL, &error);
      //cout << "map_buffer(size) error: " << error << endl;
      return pointer;
    }
    
    //TODO remove this function.
    void unmap_buffer() {
      cl_int error;
      if (pointer != nullptr) {
        error = clEnqueueUnmapMemObject(queue, buffer, pointer, 0, NULL, NULL);
        pointer = nullptr;
        if (error != CL_SUCCESS) {
          cout << "Can't unmap buffer!: " << error << endl;
        }
      }
    }
    cl_int unmap_buffer(void* &pointer) {
      if (!pointer) return CL_INVALID_HOST_PTR;
      cl_int error = clEnqueueUnmapMemObject(queue, buffer, pointer, 0, NULL, NULL);
      pointer = nullptr;
      return error;
    }
    ~Buffer() {
      if (pointer != nullptr) {
        clEnqueueUnmapMemObject(queue, buffer, pointer, 0, NULL, NULL);
      }
      clReleaseMemObject(buffer);
    }
};

