
HealthMonitor is a sample of OpenCL multi-GPU application exercising the GPUs by executing 3 OpenCL featured kernels.
The GPU devices IDs to be tested are defined by the "-iDD" parameter, and the number of times the kernels are executed is defined by the "mode" parameter value.

Prerequisites:
 - cmake 2.8.12 or higher
 - AMD APP SDK 2.9.1 or higher
 - An OpenMP capable compiler

Building procedure:
 - create a build directory
 - cd build
 - cmake Path_to_the_src_directory
 - make (Linux)
   compile the Project.sln project with Visual Studio (Windows)

Program options:
 -m mode
    mode=0 perform 31 iterations,
    mode=1 perform 310 iterations,
    mode=2 perform 2000 iterations

 -iDD Id0,...,Idn
    comma separated list of GPU device Ids
