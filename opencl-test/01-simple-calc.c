#include "common.h"

#define VECTOR_SIZE 1024

static int create_program_from_kernel(const char* fname, const cl_context context, cl_program* program)
{
    FILE* fp = fopen(fname, "r");
    if(!fp)
    {
        perror(fname);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    if(-1 == length)
    {
        perror(fname);
        fclose(fp);
        return -1;
    }
    char src[length];
    fseek(fp, 0, SEEK_SET);
    if(1 != fread(src, length, 1, fp))
    {
        perror(fname);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    cl_int err;
    const char* ss = src;
    *program = clCreateProgramWithSource(context
            , 1
            , &ss
            , NULL
            , &err);
    if(err != CL_SUCCESS)
    {
        fprintf(stderr, "clCreateProgramWithSource failed: %d\n", err);
        return -1;
    }
    return 0;
}

int main()
{
    cl_int err;

    // Get the first available device
    cl_device_id gpu;
    err = clGetDeviceIDs(NULL
            , CL_DEVICE_TYPE_GPU
            , 1
            , &gpu
            , NULL);

    if(err != CL_SUCCESS)
    {
        printf("clGetDeviceIDs failed: %d\n", err);
        return 1;
    }

    // Create a context
    cl_context context;
    context = clCreateContext(NULL
            , 1
            , &gpu
            , NULL
            , NULL
            , &err);

    if(err != CL_SUCCESS)
    {
        printf("clCreateContext failed: %d\n", err);
        return 2;
    }

    if(err != CL_SUCCESS)
    {
        clReleaseContext(context);
        fprintf(stderr, "clCreateCommandQueue failed: %d\n", err);
        return 3;
    }

    // Initialize host data
    float host_a[VECTOR_SIZE];
    float host_b[VECTOR_SIZE];
    float host_result[VECTOR_SIZE];

    for(int i = 0; i < VECTOR_SIZE; i++)
    {
        host_a[i] = (float)i / 31.4;
        host_b[i] = (float)i / 15.5;
    }

    // Create buffers
    cl_mem a = clCreateBuffer(context
            , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR
            , sizeof(host_a)
            , host_a
            , NULL);

    cl_mem b = clCreateBuffer(context
            , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR
            , sizeof(host_b)
            , host_b
            , NULL);

    cl_mem result = clCreateBuffer(context
            , CL_MEM_WRITE_ONLY
            , sizeof(host_result)
            , NULL
            , NULL);

    clReleaseContext(context);

    // Create a program
    cl_program program;
    if(create_program_from_kernel("01-kernel.cl", context, &program))
    {
        clReleaseContext(context);
        return 4;
    }

    // Build the program
    err = clBuildProgram(program
            , 1
            , &gpu
            , NULL
            , NULL
            , NULL);

    if(err != CL_SUCCESS)
    {
        size_t logSize;
        clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char log[logSize];
        clGetProgramBuildInfo(program, gpu, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        fprintf(stderr, "%s\n", log);
        clReleaseProgram(program);
        clReleaseContext(context);
        return 5;
    }

    // Create a kernel
    cl_kernel kernel = clCreateKernel(program
            , "vector_add"
            , &err);

    clReleaseProgram(program);

    if(err != CL_SUCCESS)
    {
        fprintf(stderr, "clCreateKernel failed: %d\n", err);
        clReleaseContext(context);
        return 6;
    }

    // Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &a);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &b);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &result);

    // Create a command queue
    cl_command_queue queue = clCreateCommandQueue(context
            , gpu
            , 0
            , &err);

    // Execute the kernel
    size_t global_work_size = VECTOR_SIZE;
    err = clEnqueueNDRangeKernel(queue
            , kernel
            , 1
            , NULL
            , &global_work_size
            , NULL
            , 0
            , NULL
            , NULL);

    clFinish(queue);
    clReleaseKernel(kernel);
    clReleaseMemObject(a);
    clReleaseMemObject(b);
    if(err != CL_SUCCESS)
    {
        fprintf(stderr, "clEnqueueNDRangeKernel failed: %d\n", err);
        clReleaseMemObject(result);
        clReleaseCommandQueue(queue);
        return 7;
    }

    // Read result from buffer
    err = clEnqueueReadBuffer(queue
            , result
            , CL_TRUE
            , 0
            , sizeof(host_result)
            , host_result
            , 0
            , NULL
            , NULL);

    clReleaseCommandQueue(queue);

    if(err != CL_SUCCESS)
    {
        printf("clEnqueueReadBuffer failed: %d\n", err);
        clReleaseMemObject(result);
        return 8;
    }

    // Print result
    for(int i = 0; i < VECTOR_SIZE; i++)
        printf("Result[%d] = %f\n", i, host_result[i]);

    clReleaseMemObject(result);

    return 0;
}

