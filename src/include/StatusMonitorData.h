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

#include <vector>
#include "Helper.h"

using namespace std;

class HealthMonitorData
{
public:
  HealthMonitorData(int NBBuffers, cl_context context, int GPUID);
  ~HealthMonitorData(void);

  cl_int AllocGPUBuffer(cl_command_queue queue, unsigned int NBElements);
  cl_int TransferGPUBuffer(vector<float>& A, cl_kernel ComputeB);
  cl_int ReleaseData();

  double run(cl_kernel Inverse, cl_kernel Compare, cl_int& error);
  bool WasRunOK(){ return IsRunOk; };

private:
  cl_int Runinverse( cl_kernel Inverse, bool PerformanceMode, double& time);
  cl_int RunControl(  cl_kernel Compare);
  cl_int ErrorOnGPU();


  int m_NBBuffers;
  vector<cl_mem> bufA;
  vector<cl_mem> bufB;
  cl_mem ErrorCheckBuffer;
  cl_mem ErrorCheckPinnedMemory;
  unsigned int* localError;
  int m_NBElements;
  cl_command_queue queue;
  bool IsRunOk;
  cl_context ctx;
  int GPUID;
};
