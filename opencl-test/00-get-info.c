#include "common.h"

int main()
{
    // Step 1: Get Platform
    cl_uint platformCount = 0;
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, &platformCount);
    if(platformCount == 0)
    {
        printf("No OpenCL platforms found.\n");
        return 1;
    }
    printf("OpenCL platform found.\n");

    // Step 2: Get Device
    cl_uint deviceCount = 0;
    cl_device_id device = 0;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &deviceCount);
    if(deviceCount == 0)
    {
        printf("No GPU devices found, trying CPU.\n");
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, &deviceCount);
        if(deviceCount == 0)
        {
            printf("No CPU devices found either.\n");
            return 2;
        }
    }
    printf("OpenCL device opened (no need to close at the end).\n");

    // If you call clRetainDevice, you must release it using clReleaseDevice. However, this is rare and typically used in multi-device setups where you manage device references manually.

    // Step 3: Display Device Name
    char deviceName[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
    printf("Using device: %s\n", deviceName);

    cl_device_type deviceType;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, NULL);

    if(deviceType == CL_DEVICE_TYPE_GPU)
    {
        printf("The device is a GPU.\n");
    }
    else if(deviceType == CL_DEVICE_TYPE_CPU)
    {
        printf("The device is a CPU.\n");
    }
    else
    {
        printf("The device is of another type.\n");
    }

    cl_uint computeUnits;
    cl_int err = clGetDeviceInfo(device
            , CL_DEVICE_MAX_COMPUTE_UNITS
            , sizeof(computeUnits)
            , &computeUnits
            , NULL);

    if(err == CL_SUCCESS)
    {
        printf("Number of compute units (cores): %u\n", computeUnits);
    }
    else
    {
        printf("Failed to query compute units.\n");
    }


    size_t maxWorkGroupSize = 0;
    cl_ulong localMemSize = 0;
    cl_ulong maxWorkItemsPerComputeUnit = 0;

    // Step 3: Query Maximum Work-Group Size
    clGetDeviceInfo(device
            , CL_DEVICE_MAX_WORK_GROUP_SIZE
            , sizeof(maxWorkGroupSize)
            , &maxWorkGroupSize
            , NULL);

    printf("Maximum work-group size: %zu\n", maxWorkGroupSize);

    // Step 4: Query Local Memory Size
    clGetDeviceInfo(device
            , CL_DEVICE_LOCAL_MEM_SIZE
            , sizeof(localMemSize)
            , &localMemSize
            , NULL);

    printf("Local memory size per compute unit: %llu bytes\n", localMemSize);

    // Step 5: Calculate Theoretical Maximum Work-Items Per Compute Unit
    maxWorkItemsPerComputeUnit = maxWorkGroupSize; // Rough estimate
    printf("Max work-items per compute unit (theoretical): %llu\n", maxWorkItemsPerComputeUnit);

    //    size_t preferredGroupSizeMultiple;
    //    clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(preferredGroupSizeMultiple), &preferredGroupSizeMultiple, NULL);
    //    printf("Preferred work-group size multiple (SIMD width): %zu\n", preferredGroupSizeMultiple);

    return 0;
}

