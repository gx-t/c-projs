__kernel void vector_add(__global const float* A,
        __global const float* B,
        __global float* C)
{
    int id = get_global_id(0);
    C[id] = sin(A[id]) + cos(B[id]);
}

