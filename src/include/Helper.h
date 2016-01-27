// 
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "CL/opencl.h"
#include <string>
#include <fstream>


#ifdef WIN32
#define THREAD_EXIT data->outputfile.close(); return 0;
#else
#define THREAD_EXIT data->outputfile.close(); pthread_exit(0);
#endif




inline const char*  getOpenCLErrorCodeStr(int errorCode)
{

  switch(errorCode)
  {
  case CL_DEVICE_NOT_FOUND:
    return "CL_DEVICE_NOT_FOUND";
  case CL_DEVICE_NOT_AVAILABLE:
    return "CL_DEVICE_NOT_AVAILABLE";               
  case CL_COMPILER_NOT_AVAILABLE:
    return "CL_COMPILER_NOT_AVAILABLE";           
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
    return "CL_MEM_OBJECT_ALLOCATION_FAILURE";      
  case CL_OUT_OF_RESOURCES:
    return "CL_OUT_OF_RESOURCES";                    
  case CL_OUT_OF_HOST_MEMORY:
    return "CL_OUT_OF_HOST_MEMORY";                 
  case CL_PROFILING_INFO_NOT_AVAILABLE:
    return "CL_PROFILING_INFO_NOT_AVAILABLE";        
  case CL_MEM_COPY_OVERLAP:
    return "CL_MEM_COPY_OVERLAP";                    
  case CL_IMAGE_FORMAT_MISMATCH:
    return "CL_IMAGE_FORMAT_MISMATCH";               
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:
    return "CL_IMAGE_FORMAT_NOT_SUPPORTED";         
  case CL_BUILD_PROGRAM_FAILURE:
    return "CL_BUILD_PROGRAM_FAILURE";              
  case CL_MAP_FAILURE:
    return "CL_MAP_FAILURE";                         
  case CL_MISALIGNED_SUB_BUFFER_OFFSET:
    return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
    return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
  case CL_INVALID_VALUE:
    return "CL_INVALID_VALUE";                      
  case CL_INVALID_DEVICE_TYPE:
    return "CL_INVALID_DEVICE_TYPE";               
  case CL_INVALID_PLATFORM:
    return "CL_INVALID_PLATFORM";                   
  case CL_INVALID_DEVICE:
    return "CL_INVALID_DEVICE";                    
  case CL_INVALID_CONTEXT:
    return "CL_INVALID_CONTEXT";                    
  case CL_INVALID_QUEUE_PROPERTIES:
    return "CL_INVALID_QUEUE_PROPERTIES";           
  case CL_INVALID_COMMAND_QUEUE:
    return "CL_INVALID_COMMAND_QUEUE";              
  case CL_INVALID_HOST_PTR:
    return "CL_INVALID_HOST_PTR";                   
  case CL_INVALID_MEM_OBJECT:
    return "CL_INVALID_MEM_OBJECT";                  
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";    
  case CL_INVALID_IMAGE_SIZE:
    return "CL_INVALID_IMAGE_SIZE";                 
  case CL_INVALID_SAMPLER:
    return "CL_INVALID_SAMPLER";                    
  case CL_INVALID_BINARY:
    return "CL_INVALID_BINARY";                     
  case CL_INVALID_BUILD_OPTIONS:
    return "CL_INVALID_BUILD_OPTIONS";              
  case CL_INVALID_PROGRAM:
    return "CL_INVALID_PROGRAM";                    
  case CL_INVALID_PROGRAM_EXECUTABLE:
    return "CL_INVALID_PROGRAM_EXECUTABLE";          
  case CL_INVALID_KERNEL_NAME:
    return "CL_INVALID_KERNEL_NAME";                
  case CL_INVALID_KERNEL_DEFINITION:
    return "CL_INVALID_KERNEL_DEFINITION";          
  case CL_INVALID_KERNEL:
    return "CL_INVALID_KERNEL";                     
  case CL_INVALID_ARG_INDEX:
    return "CL_INVALID_ARG_INDEX";                   
  case CL_INVALID_ARG_VALUE:
    return "CL_INVALID_ARG_VALUE";                   
  case CL_INVALID_ARG_SIZE:
    return "CL_INVALID_ARG_SIZE";                    
  case CL_INVALID_KERNEL_ARGS:
    return "CL_INVALID_KERNEL_ARGS";                
  case CL_INVALID_WORK_DIMENSION:
    return "CL_INVALID_WORK_DIMENSION";              
  case CL_INVALID_WORK_GROUP_SIZE:
    return "CL_INVALID_WORK_GROUP_SIZE";             
  case CL_INVALID_WORK_ITEM_SIZE:
    return "CL_INVALID_WORK_ITEM_SIZE";             
  case CL_INVALID_GLOBAL_OFFSET:
    return "CL_INVALID_GLOBAL_OFFSET";              
  case CL_INVALID_EVENT_WAIT_LIST:
    return "CL_INVALID_EVENT_WAIT_LIST";             
  case CL_INVALID_EVENT:
    return "CL_INVALID_EVENT";                      
  case CL_INVALID_OPERATION:
    return "CL_INVALID_OPERATION";                 
  case CL_INVALID_GL_OBJECT:
    return "CL_INVALID_GL_OBJECT";                  
  case CL_INVALID_BUFFER_SIZE:
    return "CL_INVALID_BUFFER_SIZE";                 
  case CL_INVALID_MIP_LEVEL:
    return "CL_INVALID_MIP_LEVEL";                   
  case CL_INVALID_GLOBAL_WORK_SIZE:
    return "CL_INVALID_GLOBAL_WORK_SIZE";            
  case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
    return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
  case CL_PLATFORM_NOT_FOUND_KHR:
    return "CL_PLATFORM_NOT_FOUND_KHR";
    //case CL_INVALID_PROPERTY_EXT:
    //    return "CL_INVALID_PROPERTY_EXT";
  case CL_DEVICE_PARTITION_FAILED_EXT:
    return "CL_DEVICE_PARTITION_FAILED_EXT";
  case CL_INVALID_PARTITION_COUNT_EXT:
    return "CL_INVALID_PARTITION_COUNT_EXT"; 
  default:
    return "unknown error code";
  }
}

inline std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return contents;
  }
  throw new std::string("File not found!");
}

inline double gflops(std::size_t M, std::size_t N, std::size_t K, double tsec){
  return 2.0*M*N*K*1e-9/tsec;
}

inline void check_err(int err, const char * fctname, cl_context* ctx)
{
  if (err != CL_SUCCESS)
  {
    const char* error=getOpenCLErrorCodeStr(err);

    printf( "%s failed with error : %s\n", fctname, error );
    if(ctx)
      clReleaseContext(*ctx);
    exit(EXIT_FAILURE);
  }
}


inline void check_err_thread(int error, const char * fctname)
{
  if (error != CL_SUCCESS)
  {
    const char* cstr_error = getOpenCLErrorCodeStr(error);

    printf("%s failed with error : %s\n", fctname, cstr_error);
    
  }
}
