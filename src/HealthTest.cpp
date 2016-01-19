#include <cstdio>

#include <cmath>

#include <sstream>
#include <cstring>
#include <string>

#include <iostream>
#include <fstream>
#include "HealthMonitorData.h"
#include <vector>

#include "CL/opencl.h"
#include "threadManagement.h"
#include <signal.h>
#include "Helper.h"
#include "Timer.h"

#include <memory>
using namespace std;

std::vector<cl_command_queue> queuelist;
std::vector <Thread> g_threads;


#define EPSILOND 10e-4
#define EPSILONF 10e-3
//Benchmark sizes



static std::string KernelString("/* Kernel1.cl - Created with AMD CodeXL */                                                                 \n"
  "                                // (3.0f*a+4.0f) * (0.4f*b-2.0f) = 1.0f  =>b = (1.0f/(3.0f*a+4.0f)+2.0f)/0.4f                            \n"
  "                                //(1.5f*a+2.0f) * (1.6f*b-8.0f) = 2.0f                                                                   \n"
  "                                                                                                                                         \n"
  "                                #define LOOPSIZE 200 //=> will do 3*LOOPSIZE*(2+1) mad operations for 2 fetches and 2 writes             \n"
  "                                #define EPSILON 1.0e-2                                                                                   \n"
  "                                                                                                                                         \n"
  "                                __kernel void ComputeB(__global float* A, __global float* B)                                             \n"
  "                                {                                                                                                        \n"
  "                                  int x = get_global_id(0);                                                                              \n"
  "                                  B[x] = (1.0f / (3.0f* A[x] + 4.0f) + 2.0f) / 0.4f;                                                     \n"
  "                                }                                                                                                        \n"
  "                                                                                                                                         \n"
  "                                                                                                                                         \n"
  "                                __kernel void SpecialInverse(__global float* A, __global float* B)                                       \n"
  "                                {                                                                                                        \n"
  "                                  int x = get_global_id(0);                                                                              \n"
  "                                  float a = A[x];                                                                                        \n"
  "                                  float b = B[x];                                                                                        \n"
  "                                                                                                                                         \n"
  "                                  float ErrorA = 0.0f;                                                                                   \n"
  "                                  float ErrorB = 0.0f;                                                                                   \n"
  "                                                                                                                                         \n"
  "                                  for (int i = 0; i<2 * LOOPSIZE; i++)                                                                   \n"
  "                                    ErrorA = mad(mad(3.0f, a, 4.0f), mad(0.4f, b, -2.0f), ErrorA);                                       \n"
  "                                                                                                                                         \n"
  "                                  for (int i = 0; i<LOOPSIZE; i++)                                                                       \n"
  "                                    ErrorB = mad(mad(1.5f, a, 2.0f), mad(1.6f, b, -8.0f), ErrorB);                                       \n"
  "                                                                                                                                         \n"
  "                                                                                                                                         \n"
  "                                  A[x] = ErrorA;                                                                                         \n"
  "                                  B[x] = ErrorB;                                                                                         \n"
  "                                }                                                                                                        \n"
  "                                                                                                                                         \n"
  "                                                                                                                                         \n"
  "                                __kernel void ErrorCount(__global float* A, __global float* B, __global unsigned int* counter)           \n"
  "                                {                                                                                                        \n"
  "                                  int x = get_global_id(0);                                                                              \n"
  "                                  if (fabs(A[x] - B[x])>EPSILON)                                                                         \n"
  "                                    atomic_inc(counter);                                                                                 \n"
  "                                }                                                                                                        \n");



std::size_t ntests = 1;

int NeedToTerminateThread = 0;




struct thread_data
{
  cl_context ctx;
  cl_command_queue queue;
  int NBElem;
  size_t NBRuns;
  cl_kernel InverseKernel;
  cl_kernel Compare;
  cl_kernel ComputeB;
  int NBBuffers;
  int GPUID;
  std::ofstream outputfile;

} ;





thread_data* g_data = NULL;
cl_context ctx = 0;
cl_program program = 0;


double gflops(int size, double times)
{
  //for now the loop size is hardcoded in the kernel at 200, this will have to change
  if (times != 0.0)
    return size*3.0*200.0*(2.0)*1.0e-9/times;
  else
    return 0.0;
}

/********************************************************************************************************************************/
/*                   TestProcedure is the main thread function, it has to be specific for windows and linux                     */
/********************************************************************************************************************************/
#ifdef WIN32
DWORD WINAPI
#else
void
#endif
TestProcedure(thread_data* data)
{
  HealthMonitorData testdata(data->NBBuffers, data->ctx, data->GPUID);

  CPerfCounter timer;
  timer.Start();
  vector<float> A;

  A.resize(data->NBElem);
  int error = 0;

#pragma omp parallel for
  for (unsigned int i=0; i<A.size(); ++i)
    A[i] = (float)rand()/RAND_MAX;
  


  error = testdata.AllocGPUBuffer( data->queue, data->NBElem);
  if (error)
  {
    THREAD_EXIT;
  }

  timer.Stop();
  double time = timer.GetElapsedTime();

  cout << "It took " << time << " s to allocate initial memory on CPU, fill it with random data and to allocate memory on GPU " << data->GPUID<<  endl;
  cout << "We will start now to run the test on this GPU" << endl;


  size_t CurrentRun = (int)data->NBRuns == 0 ? -1 : data->NBRuns;
  int i = 0;

  timer.Reset();
  timer.Start();

  while ((data->NBRuns == 0 || CurrentRun != 0))
  {
    error = testdata.TransferGPUBuffer(A, data->ComputeB);
    if (error)
    {
      THREAD_EXIT;
    }

    if (NeedToTerminateThread)
    {
      THREAD_EXIT;
    }
      

    double time = testdata.run(data->InverseKernel, data->Compare, error);
    if (error)
    {
      THREAD_EXIT;
    }
  
    data->outputfile <<  i<< "," << gflops(data->NBElem, time) << std::endl;
    i++;
    if (CurrentRun != -1)
      CurrentRun--;
  }

  timer.Stop();
  cout << endl;
  time = timer.GetElapsedTime();
  cout <<  "It took " << time << " s to run the GPU test on GPU " << data->GPUID << endl;



  if (!testdata.WasRunOK())
  {
    ofstream Adata;

    Adata.open("Adata.bin", ofstream::out | ofstream::binary | ofstream::trunc);
    Adata.write((char*)&A[0], data->NBElem*sizeof(A[0]));
    Adata.close();

  }

  THREAD_EXIT;

}

/********************************************************************************************************************************/
/********************************************************************************************************************************/

void Print_BuildLog(cl_program program)
{

  for (size_t i=0; i<g_threads.size(); i++)
  {
    cl_device_id device;
    clGetCommandQueueInfo( 	g_data[i].queue,CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL);


    //here we know program exist
    size_t NBChar = 0; 
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &NBChar);

    char* log = new char[NBChar];
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, NBChar, log, &NBChar);
    std::cout <<log<<std::endl;
    std::cout.flush();

    delete [] log;
  }
}


cl_int FinCLPlatform(cl_platform_id& platform)
{
  cl_int status = CL_SUCCESS;
  cl_uint numPlatforms;
  status = clGetPlatformIDs(0, NULL, &numPlatforms);
  if(status != CL_SUCCESS)
  {
    cout<<"Error: clGetPlatformIDs failed. Error code : "<< status;

    return status;
  }

  if (0 < numPlatforms)
  {
    // Get selected platform
    cl_platform_id* platforms = new cl_platform_id[numPlatforms];
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if(status != CL_SUCCESS)
    {
      cout<<"Error: clGetPlatformIDs failed. Error code : " <<status;

      return status;
    }

    // Print all platforms
    for (unsigned i = 0; i < numPlatforms; ++i)
    {
      char pbuf[100];
      status = clGetPlatformInfo(platforms[i],
        CL_PLATFORM_VENDOR,
        sizeof(pbuf),
        pbuf,
        NULL);

      if(status != CL_SUCCESS)
      {
        cout<<"Error: clGetPlatformInfo failed. Error code : ";
        return status;
      }

      cout << "Platform " << i << " : " << pbuf << endl;
    }

    // Get AMD platform
    for (unsigned i = 0; i < numPlatforms; ++i)
    {
      char pbuf[100];
      status = clGetPlatformInfo(platforms[i],
        CL_PLATFORM_VENDOR,
        sizeof(pbuf),
        pbuf,
        NULL);

      if(status != CL_SUCCESS)
      {
        cout<<"Error: clGetPlatformInfo failed. Error code : ";
        return status;
      }

      platform = platforms[i];
      if (!strcmp(pbuf, "Advanced Micro Devices, Inc."))
      {
        break;
      }
    }

    // Check for AMD platform
    char pbuf[100];
    status = clGetPlatformInfo(platform,
      CL_PLATFORM_VENDOR,
      sizeof(pbuf),
      pbuf,
      NULL);

    if(status != CL_SUCCESS)
    {
      cout<<"Error: clGetPlatformInfo failed. Error code : ";
      return status;
    }
    if (strcmp(pbuf, "Advanced Micro Devices, Inc."))
    {
      cout << "AMD platform not found" << endl;
      return -1;
    }

  }

  return status;

}


/*************************************************************************************************/

cl_int InitCL(std::vector<unsigned int>& deviceNum, bool deviceAll, cl_context& context, int& mainCL, float& MaxGlobalMemory)
{
  cl_int status = CL_SUCCESS;
  cl_platform_id platform = NULL;
  cl_context_properties properties[3];


  status = FinCLPlatform(platform);

  if(status!=CL_SUCCESS || platform==NULL)
  {
    cout<< "can't find a AMD platform for OpenCL" << endl;
    return status;
  }

  //I only want to test on GPU for the moment,
  unsigned int NBDevices = 0;
  status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &NBDevices);

  if (NBDevices<1)
  {
    cout<< "no AMD GPU devices in the system or can't query them"<<endl;
    return -1;
  }

  //we test on all devices
  unsigned int MaxDevice = (unsigned int)deviceNum.size();
  int NBDeviceToUse =0;
  if (deviceAll)
    NBDeviceToUse = NBDevices;
  else
    NBDeviceToUse = NBDevices<=MaxDevice?NBDevices:MaxDevice;
  
  cl_device_id* devices = new cl_device_id[NBDevices];
  cl_device_id* devicesToUse = NULL;



  status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, NBDevices, devices, NULL);
  if(status!=CL_SUCCESS)
  {

    return status;
  }

  
  if (deviceAll)
  {
    devicesToUse = devices;
  }
  else
  {
    devicesToUse =  new cl_device_id[MaxDevice];
    for(unsigned int i =0; i<MaxDevice; i++)
    {
      if (deviceNum[i]<NBDevices)
        devicesToUse[i]=devices[deviceNum[i]];
    }
  }

   // context properties list - must be terminated with 0
  properties[0]= CL_CONTEXT_PLATFORM;
  properties[1]= (cl_context_properties) platform;
  properties[2]= 0;

  // create a context with the GPU device
  context = clCreateContext(properties,NBDeviceToUse,devicesToUse,NULL,NULL,&status);
  if(status!=CL_SUCCESS)
  {

    return status;
  }

  MaxGlobalMemory = 128.0f;
  mainCL = 2;
#ifdef CL_VERSION_2_0
  cl_queue_properties QueueProp[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
#endif
  queuelist.resize(NBDeviceToUse);
  for ( int i=0; i<NBDeviceToUse; i++)
  {

    if (mainCL!=1)
    {
      int GFXIP = 0;
      //need to check with GFXIP as with driver 14.xxx we only report the first GPU device as a CL2.0 compatible device beucase we don't support SVM on multi devices
      //So we can still compile using CL2.0 compilation flag which uses our new hsail compiler
      clGetDeviceInfo(devicesToUse[i], CL_DEVICE_GFXIP_MAJOR_AMD, sizeof (int), &GFXIP, NULL);
      if (GFXIP<7)
        mainCL=1;

      cl_ulong GlobMem = 0;
      float FloatGloMem = .0f;
      clGetDeviceInfo(devicesToUse[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof (cl_ulong), &GlobMem, NULL);
      FloatGloMem = (float)GlobMem / (1024.0f * 1024.0f * 1024.0f);

      MaxGlobalMemory = MaxGlobalMemory>FloatGloMem ? FloatGloMem : MaxGlobalMemory;

    }

#ifdef CL_VERSION_2_0
    queuelist[i] = clCreateCommandQueueWithProperties(context, devicesToUse[i], QueueProp, &status);
#else
    queuelist[i] = clCreateCommandQueue(context, devicesToUse[i], CL_QUEUE_PROFILING_ENABLE, &status);
#endif
    if(status!=CL_SUCCESS)
    {
      return status;
    }

  }

  delete []devices;
  if (!deviceAll)
    delete []devicesToUse;


  return status;
}



//We will work only with clBLAS column major kernel
void createThread(  cl_context context, cl_program programXgemm,  size_t NBRuns, int NBElems, int NBBuffers)
{
  g_threads.resize(queuelist.size());
  g_data = new thread_data[queuelist.size()];
  

  for (size_t i=0; i<g_threads.size(); i++)
  {
    stringstream ss;
    cl_int err = CL_SUCCESS;
    cl_kernel Inverse;
    cl_kernel Control;
    cl_kernel ComputeB;


    Inverse = clCreateKernel(programXgemm, "SpecialInverse", &err);
    Control = clCreateKernel(programXgemm, "ErrorCount", &err);
    ComputeB = clCreateKernel(programXgemm, "ComputeB", &err);

    
    check_err(err, "clCreateKernel", &context);

    g_data[i].queue=queuelist[i];


    g_data[i].ctx = context;
    g_data[i].NBRuns = NBRuns;
    g_data[i].InverseKernel = Inverse;
    g_data[i].Compare = Control;
    g_data[i].ComputeB = ComputeB;
    g_data[i].NBElem = NBElems;
    g_data[i].NBBuffers = NBBuffers;
    g_data[i].GPUID = (int)i;


    ss << i;
    string GPUID = ss.str();

    string fileName = "Run_GPU_" + GPUID + ".csv";
    g_data[i].outputfile.open(fileName.c_str());

    g_data[i].outputfile << "Run # " << "," << "GFLOPS" << endl;


    g_threads[i].create((THREAD_PROC)TestProcedure,  (void*)&g_data[i]);

  }
}

void cleanup()
{
  //cout << "CLEANING UP..." << endl;
  for (size_t i=0; i<g_threads.size(); i++)
  {   
    clReleaseCommandQueue(g_data[i].queue);
    clReleaseKernel(g_data[i].InverseKernel);
    clReleaseKernel(g_data[i].Compare);
    clReleaseKernel(g_data[i].ComputeB);

  }
  delete [] g_data;

  clReleaseProgram(program);

  clReleaseContext(ctx);
}


#ifdef WIN32
BOOL WINAPI ConsoleHandler(
    DWORD dwCtrlType   //  control signal type
);


BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    //char mesg[128];

    switch(CEvent)
    {
    case CTRL_C_EVENT:
        cout<<"interup event"<<endl;
        NeedToTerminateThread = 1;
        break;

    case CTRL_CLOSE_EVENT:
        cout<<"close event"<<endl;
        NeedToTerminateThread = 1;
        exit(0);
        break;
    case CTRL_LOGOFF_EVENT:
        cout<<"logoff event"<<endl;
        cleanup();
        exit(0);
        break;
    case CTRL_SHUTDOWN_EVENT:
        cout<<"shutdown event"<<endl;
        cleanup();
        exit(0);
        break;

    }
    return TRUE;
}
#else
void my_handler(int s)
{
  cout<<"interup event"<<endl;
  NeedToTerminateThread = 1; 
}

#endif

void Usage(void)
{
    printf("Usage: HealthTest -m mode (=0 use 31 bufferss, =1 use 310 buffers, =2 use 2000 buffers) -iDD devices (comma separated list of device Ids)\n");
    exit (-10);
}

int main( int argc, char *argv[])
{
  cl_int err;
  
  int ret = 0;
  CPerfCounter timer;
  timer.Start();
#ifdef WIN32
  if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
  {
    // unable to install handler... 
    // display message to the user
    printf("Unable to install handler!\n");
    return -1;
  }
#else
   struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
#endif


  bool deviceAll = true;
  std::vector<unsigned int> deviceNum;

  size_t NBRuns = 5;

  while (--argc)
  {
    ++argv;
    if (!strncmp(*argv, "-m", 2))
    {
      ++argv;
      --argc;
      int mode = strtoul((*argv), NULL, 0);
      cout << "mode = "<<mode<<endl;
      if (mode==0)
        NBRuns = 31;
      else if (mode == 1)
        NBRuns = 310;
      else if (mode == 2)
        NBRuns = 20000;
      else
        Usage();

    }
    else if (!strncmp(*argv, "-iDD", 4))
    {
      ++argv;
      --argc;
      char *p = *argv;
      while (p) 
      {
        deviceAll = false;
        int device = 0;
        sscanf(p, "%d",&device);
        cout << "device = "<<device<<endl;
        deviceNum.push_back(device);
        p = strchr(p, ',');
        if (p != NULL)
          ++p;
      }
    }
    else
    {
        Usage();
    }

  }

  int MainCL;
  float MaxGlobalMemory = 0.0f;
  InitCL(deviceNum, deviceAll, ctx, MainCL, MaxGlobalMemory);


  /* Create and build program */
  std::string src;


  //sgemm1
  //std::string srcXgemm;
  //
  //srcXgemm = get_file_contents("HealthMonitorKernels.cl");

  const char * C_KernelString = KernelString.c_str();
  std::size_t C_KernelString_size[] = { strlen(C_KernelString) };



  program = clCreateProgramWithSource(ctx, 1, &C_KernelString, C_KernelString_size, &err);
  check_err(err, "clCreateProgramWithSource", &ctx);


  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

  if (err!=CL_SUCCESS)
  {
    Print_BuildLog(program);
    check_err(err, "clCreateProgramWithSource", &ctx);
  }

  int NBElem = 6400000;
  //MaxGlobalMemory = 15.0f;
  int NBBuffers = (int)(MaxGlobalMemory / (double)(NBElem*2*sizeof(float)/(1024.0*1024.0*1024.0)));
  float totalMemory = NBElem * 2 * sizeof(float) / (1024.0f*1024.0f*1024.0f) * NBBuffers;
  if (totalMemory != 0)
    cout << "we will work with " << NBBuffers << " buffers. This represents " << totalMemory << " GB of data" << endl << endl;
  else
  {
    cout << "OpenCL reports 0 Bytes of memory available for one of the GPU. Please check if one of the GPU is damaged using clinfo and reboot the system" << endl;
    return -1;
  }


  createThread(ctx, program, NBRuns, NBElem, NBBuffers);
  timer.Stop();

  double time = timer.GetElapsedTime();

  cout << "OpenCL setup and thread creation took " << time << "s" << endl<<endl;

  for (unsigned int i=0;i<g_threads.size();i++)	  
  {
    g_threads[i].join();
  }

  cout << endl;
  cleanup();

  return ret;
}






