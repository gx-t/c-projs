#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#ifdef PLATFORM_APPLE
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#endif // __COMMON_H__
