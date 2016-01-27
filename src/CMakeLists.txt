# ########################################################################
# Copyright 2016 Advanced Micro Devices, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ########################################################################

# We require 2.8.12
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

# We require OpenCL
# A standard FindOpenCL.cmake module ships with cmake 3.1, but we supply our own until 3.1 becomes more prevalent
SET( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/../cmake;${CMAKE_MODULE_PATH} )
MESSAGE( STATUS "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}" )
FIND_PACKAGE( OpenCL REQUIRED )
MESSAGE( STATUS "OPENCL_INCLUDE_DIRS=${OPENCL_INCLUDE_DIRS}" )
IF(CMAKE_COMPILER_IS_GNUCXX)
  FIND_PACKAGE( Threads REQUIRED )
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET( HealthMonitor.src
  HealthMonitorData.cpp
  HealthTest.cpp
  threadManagement.cpp
  timer.cpp
)

SET( HealthMonitor.hdr
  include/HealthMonitorData.h
  include/Helper.h
  include/threadManagement.h
  include/Timer.h
  )

INCLUDE_DIRECTORIES(
  ${OPENCL_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src/include
)

# We do use OpenMP
OPTION (USE_OpenMP "Use OpenMP" ON)
IF(USE_OpenMP)
  FIND_PACKAGE(OpenMP)
  IF(OPENMP_FOUND)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  ENDIF(OPENMP_FOUND)
ENDIF(USE_OpenMP)

MESSAGE( STATUS "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}" )
MESSAGE( STATUS "PROJECT_SOURCE_DIR/include=${PROJECT_SOURCE_DIR}/include" )
MESSAGE( STATUS "PROJECT_SOURCE_DIR/src/include=${PROJECT_SOURCE_DIR}/src/include" )

ADD_EXECUTABLE( HealthMonitor  ${HealthMonitor.src} ${HealthMonitor.hdr} )
IF(CMAKE_COMPILER_IS_GNUCXX)
  TARGET_LINK_LIBRARIES( HealthMonitor ${OPENCL_LIBRARIES} pthread )
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  TARGET_LINK_LIBRARIES( HealthMonitor ${OPENCL_LIBRARIES} )
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
SET_TARGET_PROPERTIES( HealthMonitor PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

