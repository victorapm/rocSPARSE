/*! \file */
/* ************************************************************************
* Copyright (c) 2021 Advanced Micro Devices, Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* ************************************************************************ */

#include "rocsparse_coomm.hpp"
#include "definitions.h"
#include "rocsparse_coomm_template_atomic.hpp"
#include "rocsparse_coomm_template_segmented.hpp"
#include "rocsparse_coomm_template_segmented_atomic.hpp"
#include "utility.h"

// Scale kernel for beta != 1.0
template <unsigned int BLOCKSIZE, typename I, typename T>
__device__ void coomm_column_scale_device(I m, I n, T beta, T* __restrict__ data, I ld)
{
    I gid = hipBlockIdx_x * BLOCKSIZE + hipThreadIdx_x;

    if(gid >= m)
    {
        return;
    }

    for(I i = 0; i < n; i++)
    {
        data[gid + ld * i] = data[gid + ld * i] * beta;
    }
}

// Scale kernel for beta != 1.0
template <unsigned int BLOCKSIZE, unsigned int WF_SIZE, typename I, typename T>
__device__ void coomm_row_scale_device(I m, I n, T beta, T* __restrict__ data, I ld)
{
    int tid = hipThreadIdx_x;
    int lid = tid & (WF_SIZE - 1);
    int wid = tid / WF_SIZE;

    I row = wid + (BLOCKSIZE / WF_SIZE) * hipBlockIdx_x;

    if(row >= m)
    {
        return;
    }

    for(I i = lid; i < n; i += WF_SIZE)
    {
        data[i + ld * row] = data[i + ld * row] * beta;
    }
}

template <unsigned int BLOCKSIZE, unsigned int WF_SIZE, typename I, typename T, typename U>
__launch_bounds__(BLOCKSIZE) ROCSPARSE_KERNEL
    void coomm_row_scale(I m, I n, U beta_device_host, T* __restrict__ data, I ld)
{
    auto beta = load_scalar_device_host(beta_device_host);
    if(beta != static_cast<T>(1))
    {
        coomm_row_scale_device<BLOCKSIZE, WF_SIZE>(m, n, beta, data, ld);
    }
}

template <unsigned int BLOCKSIZE, typename I, typename T, typename U>
__launch_bounds__(BLOCKSIZE) ROCSPARSE_KERNEL
    void coomm_column_scale(I m, I n, U beta_device_host, T* __restrict__ data, I ld)
{
    auto beta = load_scalar_device_host(beta_device_host);
    if(beta != static_cast<T>(1))
    {
        coomm_column_scale_device<BLOCKSIZE>(m, n, beta, data, ld);
    }
}

template <>
inline bool rocsparse_enum_utils::is_invalid(rocsparse_coomm_alg value_)
{
    switch(value_)
    {
    case rocsparse_coomm_alg_default:
    case rocsparse_coomm_alg_atomic:
    case rocsparse_coomm_alg_segmented:
    case rocsparse_coomm_alg_segmented_atomic:
    {
        return false;
    }
    }
    return true;
};

template <typename I, typename T, typename U>
rocsparse_status rocsparse_coomm_template_dispatch(rocsparse_handle          handle,
                                                   rocsparse_operation       trans_A,
                                                   rocsparse_operation       trans_B,
                                                   rocsparse_order           order,
                                                   rocsparse_coomm_alg       alg,
                                                   I                         m,
                                                   I                         n,
                                                   I                         k,
                                                   I                         nnz,
                                                   U                         alpha_device_host,
                                                   const rocsparse_mat_descr descr,
                                                   const T*                  coo_val,
                                                   const I*                  coo_row_ind,
                                                   const I*                  coo_col_ind,
                                                   const T*                  B,
                                                   I                         ldb,
                                                   U                         beta_device_host,
                                                   T*                        C,
                                                   I                         ldc)
{
    // Scale C with beta
    if(order == rocsparse_order_row)
    {
        if(n >= 256)
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 256>),
                               dim3((256 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
        else if(n >= 128)
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 128>),
                               dim3((128 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
        else if(n >= 64)
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 64>),
                               dim3((64 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
        else if(n >= 32)
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 32>),
                               dim3((32 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
        else if(n >= 16)
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 16>),
                               dim3((16 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
        else if(n >= 8)
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 8>),
                               dim3((8 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
        else
        {
            hipLaunchKernelGGL((coomm_row_scale<256, 4>),
                               dim3((4 * m - 1) / 256 + 1),
                               dim3(256),
                               0,
                               handle->stream,
                               m,
                               n,
                               beta_device_host,
                               C,
                               ldc);
        }
    }
    else if(order == rocsparse_order_column)
    {
        hipLaunchKernelGGL((coomm_column_scale<1024>),
                           dim3((m - 1) / 1024 + 1),
                           dim3(1024),
                           0,
                           handle->stream,
                           m,
                           n,
                           beta_device_host,
                           C,
                           ldc);
    }

    switch(alg)
    {
    case rocsparse_coomm_alg_default:
    case rocsparse_coomm_alg_atomic:
    {
        return rocsparse_coomm_template_atomic(handle,
                                               trans_A,
                                               trans_B,
                                               order,
                                               m,
                                               n,
                                               k,
                                               nnz,
                                               alpha_device_host,
                                               descr,
                                               coo_val,
                                               coo_row_ind,
                                               coo_col_ind,
                                               B,
                                               ldb,
                                               beta_device_host,
                                               C,
                                               ldc);
    }

    case rocsparse_coomm_alg_segmented:
    {
        return rocsparse_coomm_template_segmented(handle,
                                                  trans_A,
                                                  trans_B,
                                                  order,
                                                  m,
                                                  n,
                                                  k,
                                                  nnz,
                                                  alpha_device_host,
                                                  descr,
                                                  coo_val,
                                                  coo_row_ind,
                                                  coo_col_ind,
                                                  B,
                                                  ldb,
                                                  beta_device_host,
                                                  C,
                                                  ldc);
    }

    case rocsparse_coomm_alg_segmented_atomic:
    {
        return rocsparse_coomm_template_segmented_atomic(handle,
                                                         trans_A,
                                                         trans_B,
                                                         order,
                                                         m,
                                                         n,
                                                         k,
                                                         nnz,
                                                         alpha_device_host,
                                                         descr,
                                                         coo_val,
                                                         coo_row_ind,
                                                         coo_col_ind,
                                                         B,
                                                         ldb,
                                                         beta_device_host,
                                                         C,
                                                         ldc);
    }
    }
}

template <typename I, typename T>
rocsparse_status rocsparse_coomm_template(rocsparse_handle          handle,
                                          rocsparse_operation       trans_A,
                                          rocsparse_operation       trans_B,
                                          rocsparse_order           order_B,
                                          rocsparse_order           order_C,
                                          rocsparse_coomm_alg       alg,
                                          I                         m,
                                          I                         n,
                                          I                         k,
                                          I                         nnz,
                                          const T*                  alpha_device_host,
                                          const rocsparse_mat_descr descr,
                                          const T*                  coo_val,
                                          const I*                  coo_row_ind,
                                          const I*                  coo_col_ind,
                                          const T*                  B,
                                          I                         ldb,
                                          const T*                  beta_device_host,
                                          T*                        C,
                                          I                         ldc)
{
    // Check for valid handle and matrix descriptor
    if(handle == nullptr)
    {
        return rocsparse_status_invalid_handle;
    }
    else if(descr == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    // Logging TODO bench logging
    log_trace(handle,
              replaceX<T>("rocsparse_Xcoomm"),
              trans_A,
              trans_B,
              m,
              n,
              k,
              nnz,
              LOG_TRACE_SCALAR_VALUE(handle, alpha_device_host),
              (const void*&)descr,
              (const void*&)coo_val,
              (const void*&)coo_row_ind,
              (const void*&)coo_col_ind,
              (const void*&)B,
              ldb,
              LOG_TRACE_SCALAR_VALUE(handle, beta_device_host),
              (const void*&)C,
              ldc);

    if(rocsparse_enum_utils::is_invalid(trans_A))
    {
        return rocsparse_status_invalid_value;
    }

    if(rocsparse_enum_utils::is_invalid(trans_B))
    {
        return rocsparse_status_invalid_value;
    }

    if(rocsparse_enum_utils::is_invalid(alg))
    {
        return rocsparse_status_invalid_value;
    }

    if(rocsparse_enum_utils::is_invalid(order_B))
    {
        return rocsparse_status_invalid_value;
    }

    if(rocsparse_enum_utils::is_invalid(order_C))
    {
        return rocsparse_status_invalid_value;
    }

    if(order_B != order_C)
    {
        return rocsparse_status_invalid_value;
    }

    // Check sizes
    if(m < 0 || n < 0 || k < 0 || nnz < 0)
    {
        return rocsparse_status_invalid_size;
    }

    // Quick return if possible
    if(m == 0 || n == 0 || k == 0)
    {
        return rocsparse_status_success;
    }

    // Check the rest of pointer arguments
    if(alpha_device_host == nullptr || beta_device_host == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    if(handle->pointer_mode == rocsparse_pointer_mode_host
       && *alpha_device_host == static_cast<T>(0) && *beta_device_host == static_cast<T>(1))
    {
        return rocsparse_status_success;
    }

    // Check the rest of pointer arguments
    if(B == nullptr || C == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    // All must be null (zero matrix) or none null
    if(!(coo_val == nullptr && coo_row_ind == nullptr && coo_col_ind == nullptr)
       && !(coo_val != nullptr && coo_row_ind != nullptr && coo_col_ind != nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    if(nnz != 0 && (coo_val == nullptr && coo_row_ind == nullptr && coo_col_ind == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    // Check leading dimension of B
    I one = 1;
    if(trans_B == rocsparse_operation_none)
    {
        if(ldb < std::max(one, order_B == rocsparse_order_column ? k : n))
        {
            return rocsparse_status_invalid_size;
        }
    }
    else
    {
        if(ldb < std::max(one, order_B == rocsparse_order_column ? n : k))
        {
            return rocsparse_status_invalid_size;
        }
    }

    // Check leading dimension of C
    if(ldc < std::max(one, order_C == rocsparse_order_column ? m : n))
    {
        return rocsparse_status_invalid_size;
    }

    if(handle->pointer_mode == rocsparse_pointer_mode_device)
    {
        return rocsparse_coomm_template_dispatch(handle,
                                                 trans_A,
                                                 trans_B,
                                                 order_B,
                                                 alg,
                                                 m,
                                                 n,
                                                 k,
                                                 nnz,
                                                 alpha_device_host,
                                                 descr,
                                                 coo_val,
                                                 coo_row_ind,
                                                 coo_col_ind,
                                                 B,
                                                 ldb,
                                                 beta_device_host,
                                                 C,
                                                 ldc);
    }
    else
    {
        return rocsparse_coomm_template_dispatch(handle,
                                                 trans_A,
                                                 trans_B,
                                                 order_B,
                                                 alg,
                                                 m,
                                                 n,
                                                 k,
                                                 nnz,
                                                 *alpha_device_host,
                                                 descr,
                                                 coo_val,
                                                 coo_row_ind,
                                                 coo_col_ind,
                                                 B,
                                                 ldb,
                                                 *beta_device_host,
                                                 C,
                                                 ldc);
    }

    return rocsparse_status_success;
}

#define INSTANTIATE(ITYPE, TTYPE)                                     \
    template rocsparse_status rocsparse_coomm_template<ITYPE, TTYPE>( \
        rocsparse_handle          handle,                             \
        rocsparse_operation       trans_A,                            \
        rocsparse_operation       trans_B,                            \
        rocsparse_order           order_B,                            \
        rocsparse_order           order_C,                            \
        rocsparse_coomm_alg       alg,                                \
        ITYPE                     m,                                  \
        ITYPE                     n,                                  \
        ITYPE                     k,                                  \
        ITYPE                     nnz,                                \
        const TTYPE*              alpha_device_host,                  \
        const rocsparse_mat_descr descr,                              \
        const TTYPE*              coo_val,                            \
        const ITYPE*              coo_row_ind,                        \
        const ITYPE*              coo_col_ind,                        \
        const TTYPE*              B,                                  \
        ITYPE                     ldb,                                \
        const TTYPE*              beta_device_host,                   \
        TTYPE*                    C,                                  \
        ITYPE                     ldc);

INSTANTIATE(int32_t, float);
INSTANTIATE(int32_t, double);
INSTANTIATE(int32_t, rocsparse_float_complex);
INSTANTIATE(int32_t, rocsparse_double_complex);
INSTANTIATE(int64_t, float);
INSTANTIATE(int64_t, double);
INSTANTIATE(int64_t, rocsparse_float_complex);
INSTANTIATE(int64_t, rocsparse_double_complex);

#undef INSTANTIATE
