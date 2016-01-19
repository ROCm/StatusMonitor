#include "HealthMonitorData.h"
#include <iostream>


#define CHECK_ERROR_THREAD( ERROR, FCT_NAME) check_err_thread(ERROR, FCT_NAME); if(ERROR){IsRunOk = false; return ERROR;}

using namespace std;

HealthMonitorData::HealthMonitorData(int NBBuffers, cl_context context, int GPUID) :
m_NBBuffers(NBBuffers)
, ErrorCheckBuffer(NULL)
, ErrorCheckPinnedMemory(NULL)
, localError(NULL)
, m_NBElements(0)
, queue(NULL)
, IsRunOk(true)
, ctx(context)
, GPUID(GPUID)

{
}

HealthMonitorData::~HealthMonitorData()
{
  ErrorOnGPU();
  ReleaseData();
}


cl_int HealthMonitorData::ReleaseData()
{
  int error = CL_SUCCESS;
  for (int i = 0; i < m_NBBuffers; i++)
  {
    if (bufA[i])
    {
      error = clReleaseMemObject(bufA[i]);
      CHECK_ERROR_THREAD(error, "clReleaseMemObject");
      bufA[i] = NULL;
    }
    if (bufB[i])
    {
      error = clReleaseMemObject(bufB[i]);
      CHECK_ERROR_THREAD(error, "clReleaseMemObject");

      bufB[i] = NULL;
    }
  }

  if (localError || ErrorCheckPinnedMemory)
  {
    error = clEnqueueUnmapMemObject(queue, ErrorCheckPinnedMemory, localError, 0, NULL, NULL);
    CHECK_ERROR_THREAD(error, "clEnqueueUnmapMemObject");

    error = clFinish(queue);
    CHECK_ERROR_THREAD(error, "clFinish");

    localError = NULL;
  }
  if (ErrorCheckBuffer)
  {
    error = clReleaseMemObject(ErrorCheckBuffer);
    CHECK_ERROR_THREAD(error, "clReleaseMemObject");

    ErrorCheckBuffer = NULL;
  }
  if (ErrorCheckPinnedMemory)
  {
    error = clReleaseMemObject(ErrorCheckPinnedMemory);
    CHECK_ERROR_THREAD(error, "clReleaseMemObject");

    ErrorCheckPinnedMemory = NULL;
  }

  return error;
}

cl_int HealthMonitorData::AllocGPUBuffer(cl_command_queue queue, unsigned int NBElements)
{
  int error = CL_SUCCESS;
  m_NBElements = NBElements;
  if (bufA.size() != m_NBBuffers)
    bufA.resize(m_NBBuffers);

  if (bufB.size() != m_NBBuffers)
    bufB.resize(m_NBBuffers);

  this->queue = queue;
  //need to release before creating new buffers
  //CL is thread safe so it should be fin to call clReleaseMemObjets even if data are used as CL runtime will clean the data only when needed
  error = ReleaseData();
  if (error != CL_SUCCESS)
    return error;
  
  //we can now create new buffers
  for (int i = 0; i < m_NBBuffers; i++)
  {
    bufA[i] = clCreateBuffer(ctx, CL_MEM_READ_WRITE, NBElements*sizeof(float), NULL, &error);
    CHECK_ERROR_THREAD(error, "clCreateBuffer bufA");

    bufB[i] = clCreateBuffer(ctx, CL_MEM_READ_WRITE, NBElements*sizeof(float), NULL, &error);
    CHECK_ERROR_THREAD(error, "clCreateBuffer bufB");
  }

  ErrorCheckBuffer = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &error);
  CHECK_ERROR_THREAD(error, "clCreateBuffer ErrorCheckBuffer");
  ErrorCheckPinnedMemory = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(unsigned int), NULL, &error);
  CHECK_ERROR_THREAD(error, "clCreateBuffer ErrorCheckPinnedMemory");

  localError = (unsigned int*)clEnqueueMapBuffer(queue, ErrorCheckPinnedMemory, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(unsigned int), 0, NULL, NULL, &error);
  CHECK_ERROR_THREAD(error, "clEnqueueMapBuffer ErrorCheckPinnedMemory");

  return CL_SUCCESS;

}


cl_int HealthMonitorData::TransferGPUBuffer( vector<float>& A,  cl_kernel ComputeB)
{
  int error = 0;
  size_t gs[1] = { (size_t) m_NBElements };
  size_t ls[1] = { (size_t) 64 };

  if (m_NBBuffers == 0 || bufA.size()==0 || bufB.size()==0)
  {
    cout << "need to exit the application as no CL buffer has been createad" << endl;
    return 1;
  }
    

  error = clEnqueueWriteBuffer(queue, bufA[0], CL_FALSE, 0, m_NBElements * sizeof(float), &(A[0]), 0, NULL, NULL);
  CHECK_ERROR_THREAD(error, "clEnqueueWriteBuffer A");

  int ArgIndex = 0;
  error = clSetKernelArg(ComputeB, ArgIndex++, sizeof(cl_mem), &bufA[0]);
  error |= clSetKernelArg(ComputeB, ArgIndex++, sizeof(cl_mem), &bufB[0]);
  CHECK_ERROR_THREAD(error, "clSetKernelArg");

  error = clEnqueueNDRangeKernel(queue, ComputeB, 1, NULL, gs, ls, 0, NULL, NULL);

  CHECK_ERROR_THREAD(error, "clEnqueueNDRangeKernel ComputeB");
  error = clFlush(queue);
  CHECK_ERROR_THREAD(error, "clFlush ComputeB");

  for (int i = 1; i < m_NBBuffers; i++)
  {

    error = clEnqueueCopyBuffer(queue, bufA[0], bufA[i], 0, 0, m_NBElements * sizeof(float), 0, NULL, NULL);
    CHECK_ERROR_THREAD(error, "clEnqueueCopyBuffer A");
    error = clEnqueueCopyBuffer(queue, bufB[0], bufB[i], 0, 0, m_NBElements * sizeof(float), 0, NULL, NULL);
    CHECK_ERROR_THREAD(error, "clEnqueueCopyBuffer B");
    error = clFlush(queue);
    CHECK_ERROR_THREAD(error, "clFlush ComputeB");
  }
  error = clFinish(queue);
  CHECK_ERROR_THREAD(error, "clFinish ComputeB");

  return CL_SUCCESS;
}



double HealthMonitorData::run(cl_kernel Inverse, cl_kernel Compare, cl_int& error)
{
  double time = 0.0;
  if (IsRunOk)
  {
    error = Runinverse(Inverse, false, time);
    if (error)
      return 0.0;
   
    error = RunControl(Compare);
    if (error)
      return 0.0;

    error = Runinverse(Inverse, true, time);
    if (error)
      return 0.0;
  }

  return time;
}



cl_int HealthMonitorData::Runinverse(cl_kernel Inverse, bool PerformanceMode, double& time)
{
  cl_int error;
  size_t gs[1] = { (size_t) m_NBElements };
  size_t ls[1] = { (size_t) 64 };
  for ( int i = 0; i < m_NBBuffers; i++)
  {
    int ArgIndex = 0;
    error = clSetKernelArg(Inverse, ArgIndex++, sizeof(cl_mem), &bufA[i]);
    error |= clSetKernelArg(Inverse, ArgIndex++, sizeof(cl_mem), &bufB[i]);
    CHECK_ERROR_THREAD(error, "clSetKernelArg");

    error = clEnqueueNDRangeKernel(queue, Inverse, 1, NULL, gs, ls, 0, NULL, NULL);
   // error = 1;
    CHECK_ERROR_THREAD(error, "clEnqueueNDRangeKernel");
  }

  if (PerformanceMode)
  {
    cl_ulong start;
    cl_ulong end;
    cl_event event = NULL;
    int ArgIndex = 0;
    error = clSetKernelArg(Inverse, ArgIndex++, sizeof(cl_mem), &bufA[0]);
    error |= clSetKernelArg(Inverse, ArgIndex++, sizeof(cl_mem), &bufB[0]);
    CHECK_ERROR_THREAD(error, "clSetKernelArg");

    error = clEnqueueNDRangeKernel(queue, Inverse, 1, NULL, gs, ls, 0, NULL, &event);
    CHECK_ERROR_THREAD(error, "clEnqueueNDRangeKernel");

    error = clFinish(queue);
    CHECK_ERROR_THREAD(error, "clFinish");

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    time = (double)(end - start)*1e-9;
  }

  return CL_SUCCESS;
}

cl_int HealthMonitorData::RunControl( cl_kernel Compare)
{
  cl_int error;
  size_t gs[1] = { (size_t) m_NBElements };
  size_t ls[1] = { (size_t) 64 };
  
  error = clSetKernelArg(Compare, 2, sizeof(cl_mem), &ErrorCheckBuffer);
  CHECK_ERROR_THREAD(error, "clSetKernelArg");

  unsigned int NBError = 0;
  for (int i = 0; i < m_NBBuffers; i++)
  {
    int ArgIndex = 0;
    error = clSetKernelArg(Compare, ArgIndex++, sizeof(cl_mem), &bufA[i]);
    error |= clSetKernelArg(Compare, ArgIndex++, sizeof(cl_mem), &bufB[i]);
    CHECK_ERROR_THREAD(error, "clSetKernelArg");

    error = clEnqueueNDRangeKernel(queue, Compare, 1, NULL, gs, ls, 0, NULL, NULL);
    CHECK_ERROR_THREAD(error, "clEnqueueNDRangeKernel");

   // unsigned localError = 0;
    error = clEnqueueReadBuffer(queue, ErrorCheckBuffer, CL_TRUE, 0, sizeof(unsigned int), localError, 0, NULL, NULL);
    CHECK_ERROR_THREAD(error, "clEnqueueReadBuffer");
    NBError += *localError;
  }

  if (NBError != 0)
    IsRunOk = false;

  return CL_SUCCESS;
}


cl_int HealthMonitorData::ErrorOnGPU()
{
  cl_int error = 0;

  if (!IsRunOk)
  {
    cl_device_id LocalDevice = NULL;

    error = clGetCommandQueueInfo(queue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &LocalDevice, NULL);
    CHECK_ERROR_THREAD(error, "clGetCommandQueueInfo");

    cl_device_topology_amd LocalTopoFailingGPU;
    error = clGetDeviceInfo(LocalDevice, CL_DEVICE_TOPOLOGY_AMD, sizeof(LocalTopoFailingGPU), &LocalTopoFailingGPU, NULL);
    CHECK_ERROR_THREAD(error, "clGetDeviceInfo");

    cout << "Error happened on GPU " << GPUID << " plugged on bus " << (unsigned int)LocalTopoFailingGPU.pcie.bus << ". Please check the csv and bin files to know when it failed" << endl;
    return 1;
  }
  else
  {
    cout << "Everything ran fine on GPU " << GPUID << endl;
  }

  return CL_SUCCESS;
}
