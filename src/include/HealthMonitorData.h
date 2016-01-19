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