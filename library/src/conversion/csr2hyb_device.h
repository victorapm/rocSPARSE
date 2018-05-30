/* ************************************************************************
 * Copyright 2018 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once
#ifndef CSR2HYB_DEVICE_H
#define CSR2HYB_DEVICE_H

#include "handle.h"

#include <hip/hip_runtime.h>

// Block reduce kernel computing sum
template <rocsparse_int NB>
__device__ void sum_reduce(rocsparse_int tid, rocsparse_int* data)
{
    __syncthreads();

    for(int i = NB >> 1; i > 0; i >>= 1)
    {
        if(tid < i)
        {
            data[tid] += data[tid + i];
        }

        __syncthreads();
    }
}

// Compute non-zero entries per CSR row and do a block reduction over the sum
// to obtain the number of COO part non-zero entries and COO nnz per row.
// Store the result in a workspace for final reduction on part2
template <rocsparse_int NB>
__global__ void hyb_coo_nnz_part1(rocsparse_int m,
                                  rocsparse_int ell_width,
                                  const rocsparse_int* csr_row_ptr,
                                  rocsparse_int* workspace,
                                  rocsparse_int* coo_row_nnz)
{
    rocsparse_int tid = hipThreadIdx_x;
    rocsparse_int gid = hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x;

    __shared__ rocsparse_int sdata[NB];

    if(gid < m)
    {
        rocsparse_int row_nnz = csr_row_ptr[gid + 1] - csr_row_ptr[gid];

        if(row_nnz > ell_width)
        {
            row_nnz          = row_nnz - ell_width;
            sdata[tid]       = row_nnz;
            coo_row_nnz[gid] = row_nnz;
        }
        else
        {
            sdata[tid]       = 0;
            coo_row_nnz[gid] = 0;
        }
    }
    else
    {
        sdata[tid] = 0;
    }

    sum_reduce<NB>(tid, sdata);

    if(tid == 0)
    {
        workspace[hipBlockIdx_x] = sdata[0];
    }
}

// Part2 kernel for final reduction over the sum of COO non-zero entries
template <rocsparse_int NB>
__global__ void hyb_coo_nnz_part2(rocsparse_int m, rocsparse_int* workspace)
{
    rocsparse_int tid = hipThreadIdx_x;

    __shared__ rocsparse_int sdata[NB];
    sdata[tid] = 0;

    for(rocsparse_int i = tid; i < m; i += NB)
    {
        sdata[tid] += workspace[i];
    }

    __syncthreads();

    if(m < 32)
    {
        if(tid == 0)
        {
            for(rocsparse_int i = 1; i < m; ++i)
            {
                sdata[0] += sdata[i];
            }
        }
    }
    else
    {
        sum_reduce<NB>(tid, sdata);
    }

    if(tid == 0)
    {
        workspace[0] = sdata[0];
    }
}

// CSR to HYB format conversion kernel
template <typename T>
__global__ void csr2hyb_kernel(rocsparse_int m,
                               const T* csr_val,
                               const rocsparse_int* csr_row_ptr,
                               const rocsparse_int* csr_col_ind,
                               rocsparse_int ell_width,
                               rocsparse_int* ell_col_ind,
                               T* ell_val,
                               rocsparse_int* coo_row_ind,
                               rocsparse_int* coo_col_ind,
                               T* coo_val,
                               rocsparse_int* workspace,
                               rocsparse_index_base idx_base)
{
    rocsparse_int ai = hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x;

    if(ai >= m)
    {
        return;
    }

    rocsparse_int p = 0;

    rocsparse_int row_begin = csr_row_ptr[ai] - idx_base;
    rocsparse_int row_end   = csr_row_ptr[ai + 1] - idx_base;
    rocsparse_int coo_idx   = coo_row_ind ? workspace[ai] - idx_base : 0;

    // Fill HYB matrix
    for(rocsparse_int aj = row_begin; aj < row_end; ++aj)
    {
        if(p < ell_width)
        {
            // Fill ELL part
            rocsparse_int idx = ELL_IND(ai, p++, m, ell_width);
            ell_col_ind[idx]  = csr_col_ind[aj];
            ell_val[idx]      = csr_val[aj];
        }
        else
        {
            // Fill COO part
            coo_row_ind[coo_idx] = ai + idx_base;
            coo_col_ind[coo_idx] = csr_col_ind[aj];
            coo_val[coo_idx]     = csr_val[aj];
            ++coo_idx;
        }
    }

    // Pad remaining ELL structure
    for(rocsparse_int aj = row_end - row_begin; aj < ell_width; ++aj)
    {
        rocsparse_int idx = ELL_IND(ai, p++, m, ell_width);
        ell_col_ind[idx]  = -1;
        ell_val[idx]      = static_cast<T>(0);
    }
}

#endif // CSR2HYB_DEVICE_H
