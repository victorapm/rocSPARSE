/*! \file */
/* ************************************************************************
* Copyright (C) 2018-2023 Advanced Micro Devices, Inc. All rights Reserved.
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

#include <algorithm>

#include "../conversion/rocsparse_csr2coo.hpp"
#include "rocsparse_csrmm.hpp"

#include "definitions.h"
#include "utility.h"

template <>
inline bool rocsparse_enum_utils::is_invalid(rocsparse_csrmm_alg value_)
{
    switch(value_)
    {
    case rocsparse_csrmm_alg_default:
    case rocsparse_csrmm_alg_row_split:
    case rocsparse_csrmm_alg_merge:
    {
        return false;
    }
    }
    return true;
};

template <typename T, typename I, typename J, typename A>
rocsparse_status rocsparse_csrmm_buffer_size_template_merge(rocsparse_handle          handle,
                                                            rocsparse_operation       trans_A,
                                                            rocsparse_csrmm_alg       alg,
                                                            J                         m,
                                                            J                         n,
                                                            J                         k,
                                                            I                         nnz,
                                                            const rocsparse_mat_descr descr,
                                                            const A*                  csr_val,
                                                            const I*                  csr_row_ptr,
                                                            const J*                  csr_col_ind,
                                                            size_t*                   buffer_size);

template <typename T, typename I, typename J, typename A>
rocsparse_status rocsparse_csrmm_buffer_size_template(rocsparse_handle          handle,
                                                      rocsparse_operation       trans_A,
                                                      rocsparse_csrmm_alg       alg,
                                                      J                         m,
                                                      J                         n,
                                                      J                         k,
                                                      I                         nnz,
                                                      const rocsparse_mat_descr descr,
                                                      const A*                  csr_val,
                                                      const I*                  csr_row_ptr,
                                                      const J*                  csr_col_ind,
                                                      size_t*                   buffer_size)
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

    // Logging
    log_trace(handle,
              "rocsparse_csrmm_buffer_size",
              trans_A,
              m,
              n,
              k,
              nnz,
              (const void*&)descr,
              (const void*&)csr_val,
              (const void*&)csr_row_ptr,
              (const void*&)csr_col_ind,
              (const void*&)buffer_size);

    if(rocsparse_enum_utils::is_invalid(trans_A))
    {
        return rocsparse_status_invalid_value;
    }

    if(rocsparse_enum_utils::is_invalid(alg))
    {
        return rocsparse_status_invalid_value;
    }

    // Check matrix type
    if(descr->type != rocsparse_matrix_type_general)
    {
        return rocsparse_status_not_implemented;
    }

    // Check matrix sorting mode
    if(descr->storage_mode != rocsparse_storage_mode_sorted)
    {
        return rocsparse_status_not_implemented;
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

    // Check pointer arguments
    if(csr_row_ptr == nullptr || buffer_size == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    // value arrays and column indices arrays must both be null (zero matrix) or both not null
    if((csr_val == nullptr && csr_col_ind != nullptr)
       || (csr_val != nullptr && csr_col_ind == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    if(nnz != 0 && (csr_col_ind == nullptr && csr_val == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    switch(alg)
    {
    case rocsparse_csrmm_alg_merge:
    {
        return rocsparse_csrmm_buffer_size_template_merge<T>(handle,
                                                             trans_A,
                                                             alg,
                                                             m,
                                                             n,
                                                             k,
                                                             nnz,
                                                             descr,
                                                             csr_val,
                                                             csr_row_ptr,
                                                             csr_col_ind,
                                                             buffer_size);
    }

    case rocsparse_csrmm_alg_default:
    case rocsparse_csrmm_alg_row_split:
    {
        *buffer_size = 0;
        return rocsparse_status_success;
    }
    }
    return rocsparse_status_invalid_value;
}

template <typename T, typename I, typename J, typename A>
rocsparse_status rocsparse_csrmm_analysis_template_merge(rocsparse_handle          handle,
                                                         rocsparse_operation       trans_A,
                                                         rocsparse_csrmm_alg       alg,
                                                         J                         m,
                                                         J                         n,
                                                         J                         k,
                                                         I                         nnz,
                                                         const rocsparse_mat_descr descr,
                                                         const A*                  csr_val,
                                                         const I*                  csr_row_ptr,
                                                         const J*                  csr_col_ind,
                                                         void*                     temp_buffer);

template <typename T, typename I, typename J, typename A>
rocsparse_status rocsparse_csrmm_analysis_template(rocsparse_handle          handle,
                                                   rocsparse_operation       trans_A,
                                                   rocsparse_csrmm_alg       alg,
                                                   J                         m,
                                                   J                         n,
                                                   J                         k,
                                                   I                         nnz,
                                                   const rocsparse_mat_descr descr,
                                                   const A*                  csr_val,
                                                   const I*                  csr_row_ptr,
                                                   const J*                  csr_col_ind,
                                                   void*                     temp_buffer)
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

    // Logging
    log_trace(handle,
              "rocsparse_csrmm_analysis",
              trans_A,
              m,
              n,
              k,
              nnz,
              (const void*&)descr,
              (const void*&)csr_val,
              (const void*&)csr_row_ptr,
              (const void*&)csr_col_ind,
              (const void*&)temp_buffer);

    if(rocsparse_enum_utils::is_invalid(trans_A))
    {
        return rocsparse_status_invalid_value;
    }

    if(rocsparse_enum_utils::is_invalid(alg))
    {
        return rocsparse_status_invalid_value;
    }

    // Check matrix type
    if(descr->type != rocsparse_matrix_type_general)
    {
        return rocsparse_status_not_implemented;
    }

    // Check matrix sorting mode
    if(descr->storage_mode != rocsparse_storage_mode_sorted)
    {
        return rocsparse_status_not_implemented;
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

    // Check pointer arguments
    if(csr_row_ptr == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    // value arrays and column indices arrays must both be null (zero matrix) or both not null
    if((csr_val == nullptr && csr_col_ind != nullptr)
       || (csr_val != nullptr && csr_col_ind == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    if(nnz != 0 && (csr_col_ind == nullptr && csr_val == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    switch(alg)
    {
    case rocsparse_csrmm_alg_merge:
    {
        return rocsparse_csrmm_analysis_template_merge<T>(handle,
                                                          trans_A,
                                                          alg,
                                                          m,
                                                          n,
                                                          k,
                                                          nnz,
                                                          descr,
                                                          csr_val,
                                                          csr_row_ptr,
                                                          csr_col_ind,
                                                          temp_buffer);
    }

    case rocsparse_csrmm_alg_default:
    case rocsparse_csrmm_alg_row_split:
    {
        return rocsparse_status_success;
    }
    }
    return rocsparse_status_invalid_value;
}

template <typename T, typename I, typename J, typename A, typename B, typename C, typename U>
rocsparse_status rocsparse_csrmm_template_general(rocsparse_handle    handle,
                                                  rocsparse_operation trans_A,
                                                  rocsparse_operation trans_B,
                                                  rocsparse_order     order,
                                                  J                   m,
                                                  J                   n,
                                                  J                   k,
                                                  I                   nnz,
                                                  J                   batch_count_A,
                                                  I                   offsets_batch_stride_A,
                                                  I                   columns_values_batch_stride_A,
                                                  U                   alpha_device_host,
                                                  const rocsparse_mat_descr descr,
                                                  const A*                  csr_val,
                                                  const I*                  csr_row_ptr,
                                                  const J*                  csr_col_ind,
                                                  const B*                  dense_B,
                                                  J                         ldb,
                                                  J                         batch_count_B,
                                                  I                         batch_stride_B,
                                                  U                         beta_device_host,
                                                  C*                        dense_C,
                                                  J                         ldc,
                                                  J                         batch_count_C,
                                                  I                         batch_stride_C,
                                                  bool                      force_conj_A);

template <typename T, typename I, typename J, typename A, typename B, typename C, typename U>
rocsparse_status rocsparse_csrmm_template_row_split(rocsparse_handle          handle,
                                                    rocsparse_operation       trans_A,
                                                    rocsparse_operation       trans_B,
                                                    rocsparse_order           order,
                                                    J                         m,
                                                    J                         n,
                                                    J                         k,
                                                    I                         nnz,
                                                    U                         alpha_device_host,
                                                    const rocsparse_mat_descr descr,
                                                    const A*                  csr_val,
                                                    const I*                  csr_row_ptr,
                                                    const J*                  csr_col_ind,
                                                    const B*                  dense_B,
                                                    J                         ldb,
                                                    U                         beta_device_host,
                                                    C*                        dense_C,
                                                    J                         ldc,
                                                    bool                      force_conj_A);

template <typename T, typename I, typename J, typename A, typename B, typename C, typename U>
rocsparse_status rocsparse_csrmm_template_merge(rocsparse_handle          handle,
                                                rocsparse_operation       trans_A,
                                                rocsparse_operation       trans_B,
                                                rocsparse_order           order,
                                                J                         m,
                                                J                         n,
                                                J                         k,
                                                I                         nnz,
                                                U                         alpha_device_host,
                                                const rocsparse_mat_descr descr,
                                                const A*                  csr_val,
                                                const I*                  csr_row_ptr,
                                                const J*                  csr_col_ind,
                                                const B*                  dense_B,
                                                J                         ldb,
                                                U                         beta_device_host,
                                                C*                        dense_C,
                                                J                         ldc,
                                                void*                     temp_buffer,
                                                bool                      force_conj_A);

template <typename T, typename I, typename J, typename A, typename B, typename C, typename U>
rocsparse_status rocsparse_csrmm_template_dispatch(rocsparse_handle    handle,
                                                   rocsparse_operation trans_A,
                                                   rocsparse_operation trans_B,
                                                   rocsparse_order     order,
                                                   rocsparse_csrmm_alg alg,
                                                   J                   m,
                                                   J                   n,
                                                   J                   k,
                                                   I                   nnz,
                                                   J                   batch_count_A,
                                                   I                   offsets_batch_stride_A,
                                                   I columns_values_batch_stride_A,
                                                   U alpha_device_host,
                                                   const rocsparse_mat_descr descr,
                                                   const A*                  csr_val,
                                                   const I*                  csr_row_ptr,
                                                   const J*                  csr_col_ind,
                                                   const B*                  dense_B,
                                                   J                         ldb,
                                                   J                         batch_count_B,
                                                   I                         batch_stride_B,
                                                   U                         beta_device_host,
                                                   C*                        dense_C,
                                                   J                         ldc,
                                                   J                         batch_count_C,
                                                   I                         batch_stride_C,
                                                   void*                     temp_buffer,
                                                   bool                      force_conj_A)
{
    switch(alg)
    {

    case rocsparse_csrmm_alg_default:
    {
        return rocsparse_csrmm_template_general<T>(handle,
                                                   trans_A,
                                                   trans_B,
                                                   order,
                                                   m,
                                                   n,
                                                   k,
                                                   nnz,
                                                   batch_count_A,
                                                   offsets_batch_stride_A,
                                                   columns_values_batch_stride_A,
                                                   alpha_device_host,
                                                   descr,
                                                   csr_val,
                                                   csr_row_ptr,
                                                   csr_col_ind,
                                                   dense_B,
                                                   ldb,
                                                   batch_count_B,
                                                   batch_stride_B,
                                                   beta_device_host,
                                                   dense_C,
                                                   ldc,
                                                   batch_count_C,
                                                   batch_stride_C,
                                                   force_conj_A);
    }

    case rocsparse_csrmm_alg_merge:
    {
        switch(trans_A)
        {
        case rocsparse_operation_none:
        {
            return rocsparse_csrmm_template_merge<T>(handle,
                                                     trans_A,
                                                     trans_B,
                                                     order,
                                                     m,
                                                     n,
                                                     k,
                                                     nnz,
                                                     alpha_device_host,
                                                     descr,
                                                     csr_val,
                                                     csr_row_ptr,
                                                     csr_col_ind,
                                                     dense_B,
                                                     ldb,
                                                     beta_device_host,
                                                     dense_C,
                                                     ldc,
                                                     temp_buffer,
                                                     force_conj_A);
        }
        case rocsparse_operation_transpose:
        case rocsparse_operation_conjugate_transpose:
        {
            return rocsparse_csrmm_template_general<T>(handle,
                                                       trans_A,
                                                       trans_B,
                                                       order,
                                                       m,
                                                       n,
                                                       k,
                                                       nnz,
                                                       batch_count_A,
                                                       offsets_batch_stride_A,
                                                       columns_values_batch_stride_A,
                                                       alpha_device_host,
                                                       descr,
                                                       csr_val,
                                                       csr_row_ptr,
                                                       csr_col_ind,
                                                       dense_B,
                                                       ldb,
                                                       batch_count_B,
                                                       batch_stride_B,
                                                       beta_device_host,
                                                       dense_C,
                                                       ldc,
                                                       batch_count_C,
                                                       batch_stride_C,
                                                       force_conj_A);
        }
        }
    }

    case rocsparse_csrmm_alg_row_split:
    {
        switch(trans_A)
        {
        case rocsparse_operation_none:
        {
            return rocsparse_csrmm_template_row_split<T>(handle,
                                                         trans_A,
                                                         trans_B,
                                                         order,
                                                         m,
                                                         n,
                                                         k,
                                                         nnz,
                                                         alpha_device_host,
                                                         descr,
                                                         csr_val,
                                                         csr_row_ptr,
                                                         csr_col_ind,
                                                         dense_B,
                                                         ldb,
                                                         beta_device_host,
                                                         dense_C,
                                                         ldc,
                                                         force_conj_A);
        }
        case rocsparse_operation_transpose:
        case rocsparse_operation_conjugate_transpose:
        {
            return rocsparse_csrmm_template_general<T>(handle,
                                                       trans_A,
                                                       trans_B,
                                                       order,
                                                       m,
                                                       n,
                                                       k,
                                                       nnz,
                                                       batch_count_A,
                                                       offsets_batch_stride_A,
                                                       columns_values_batch_stride_A,
                                                       alpha_device_host,
                                                       descr,
                                                       csr_val,
                                                       csr_row_ptr,
                                                       csr_col_ind,
                                                       dense_B,
                                                       ldb,
                                                       batch_count_B,
                                                       batch_stride_B,
                                                       beta_device_host,
                                                       dense_C,
                                                       ldc,
                                                       batch_count_C,
                                                       batch_stride_C,
                                                       force_conj_A);
        }
        }
    }
    }
    return rocsparse_status_invalid_value;
}

template <typename T, typename I, typename J, typename A, typename B, typename C>
rocsparse_status rocsparse_csrmm_template(rocsparse_handle          handle,
                                          rocsparse_operation       trans_A,
                                          rocsparse_operation       trans_B,
                                          rocsparse_order           order_B,
                                          rocsparse_order           order_C,
                                          rocsparse_csrmm_alg       alg,
                                          J                         m,
                                          J                         n,
                                          J                         k,
                                          I                         nnz,
                                          J                         batch_count_A,
                                          I                         offsets_batch_stride_A,
                                          I                         columns_values_batch_stride_A,
                                          const T*                  alpha_device_host,
                                          const rocsparse_mat_descr descr,
                                          const A*                  csr_val,
                                          const I*                  csr_row_ptr,
                                          const J*                  csr_col_ind,
                                          const B*                  dense_B,
                                          J                         ldb,
                                          J                         batch_count_B,
                                          I                         batch_stride_B,
                                          const T*                  beta_device_host,
                                          C*                        dense_C,
                                          J                         ldc,
                                          J                         batch_count_C,
                                          I                         batch_stride_C,
                                          void*                     temp_buffer,
                                          bool                      force_conj_A)
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
              replaceX<T>("rocsparse_Xcsrmm"),
              trans_A,
              trans_B,
              m,
              n,
              k,
              nnz,
              batch_count_A,
              offsets_batch_stride_A,
              columns_values_batch_stride_A,
              LOG_TRACE_SCALAR_VALUE(handle, alpha_device_host),
              (const void*&)descr,
              (const void*&)csr_val,
              (const void*&)csr_row_ptr,
              (const void*&)csr_col_ind,
              (const void*&)dense_B,
              ldb,
              batch_count_B,
              batch_stride_B,
              LOG_TRACE_SCALAR_VALUE(handle, beta_device_host),
              (const void*&)dense_C,
              ldc,
              batch_count_C,
              batch_stride_C);

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

    // Check matrix type
    if(descr->type != rocsparse_matrix_type_general)
    {
        return rocsparse_status_not_implemented;
    }

    // Check matrix sorting mode
    if(descr->storage_mode != rocsparse_storage_mode_sorted)
    {
        return rocsparse_status_not_implemented;
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
    if(csr_row_ptr == nullptr || dense_B == nullptr || dense_C == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    // value arrays and column indices arrays must both be null (zero matrix) or both not null
    if((csr_val == nullptr && csr_col_ind != nullptr)
       || (csr_val != nullptr && csr_col_ind == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    if(nnz != 0 && (csr_col_ind == nullptr && csr_val == nullptr))
    {
        return rocsparse_status_invalid_pointer;
    }

    // Check leading dimension of matrices
    static constexpr J s_one = static_cast<J>(1);
    switch(trans_A)
    {
    case rocsparse_operation_none:
    {
        // Check leading dimension of C
        if(ldc < std::max(s_one, ((order_C == rocsparse_order_column) ? m : n)))
        {
            return rocsparse_status_invalid_size;
        }

        // Check leading dimension of B
        switch(trans_B)
        {
        case rocsparse_operation_none:
        {
            if(ldb < std::max(s_one, ((order_B == rocsparse_order_column) ? k : n)))
            {
                return rocsparse_status_invalid_size;
            }
            break;
        }
        case rocsparse_operation_transpose:
        case rocsparse_operation_conjugate_transpose:
        {
            if(ldb < std::max(s_one, ((order_B == rocsparse_order_column) ? n : k)))
            {
                return rocsparse_status_invalid_size;
            }
            break;
        }
        }
        break;
    }
    case rocsparse_operation_transpose:
    case rocsparse_operation_conjugate_transpose:
    {
        // Check leading dimension of C
        if(ldc < std::max(s_one, ((order_C == rocsparse_order_column) ? k : n)))
        {
            return rocsparse_status_invalid_size;
        }

        switch(trans_B)
        {
        case rocsparse_operation_none:
        {
            if(ldb < std::max(s_one, ((order_B == rocsparse_order_column) ? m : n)))
            {
                return rocsparse_status_invalid_size;
            }
            break;
        }
        case rocsparse_operation_transpose:
        case rocsparse_operation_conjugate_transpose:
        {
            if(ldb < std::max(s_one, ((order_B == rocsparse_order_column) ? n : m)))
            {
                return rocsparse_status_invalid_size;
            }
            break;
        }
        }
        break;
    }
    }

    // Check batch parameters of matrices
    bool Ci_A_Bi  = (batch_count_A == 1 && batch_count_B == batch_count_C);
    bool Ci_Ai_B  = (batch_count_B == 1 && batch_count_A == batch_count_C);
    bool Ci_Ai_Bi = (batch_count_A == batch_count_C && batch_count_A == batch_count_B);

    if(!Ci_A_Bi && !Ci_Ai_B && !Ci_Ai_Bi)
    {
        return rocsparse_status_invalid_value;
    }

    if(handle->pointer_mode == rocsparse_pointer_mode_device)
    {
        return rocsparse_csrmm_template_dispatch<T>(handle,
                                                    trans_A,
                                                    trans_B,
                                                    order_B,
                                                    alg,
                                                    m,
                                                    n,
                                                    k,
                                                    nnz,
                                                    batch_count_A,
                                                    offsets_batch_stride_A,
                                                    columns_values_batch_stride_A,
                                                    alpha_device_host,
                                                    descr,
                                                    csr_val,
                                                    csr_row_ptr,
                                                    csr_col_ind,
                                                    dense_B,
                                                    ldb,
                                                    batch_count_B,
                                                    batch_stride_B,
                                                    beta_device_host,
                                                    dense_C,
                                                    ldc,
                                                    batch_count_C,
                                                    batch_stride_C,
                                                    temp_buffer,
                                                    force_conj_A);
    }
    else
    {
        return rocsparse_csrmm_template_dispatch<T>(handle,
                                                    trans_A,
                                                    trans_B,
                                                    order_B,
                                                    alg,
                                                    m,
                                                    n,
                                                    k,
                                                    nnz,
                                                    batch_count_A,
                                                    offsets_batch_stride_A,
                                                    columns_values_batch_stride_A,
                                                    *alpha_device_host,
                                                    descr,
                                                    csr_val,
                                                    csr_row_ptr,
                                                    csr_col_ind,
                                                    dense_B,
                                                    ldb,
                                                    batch_count_B,
                                                    batch_stride_B,
                                                    *beta_device_host,
                                                    dense_C,
                                                    ldc,
                                                    batch_count_C,
                                                    batch_stride_C,
                                                    temp_buffer,
                                                    force_conj_A);
    }
}

#define INSTANTIATE_BUFFER_SIZE(TTYPE, ITYPE, JTYPE, ATYPE)                \
    template rocsparse_status rocsparse_csrmm_buffer_size_template<TTYPE>( \
        rocsparse_handle          handle,                                  \
        rocsparse_operation       trans_A,                                 \
        rocsparse_csrmm_alg       alg,                                     \
        JTYPE                     m,                                       \
        JTYPE                     n,                                       \
        JTYPE                     k,                                       \
        ITYPE                     nnz,                                     \
        const rocsparse_mat_descr descr,                                   \
        const ATYPE*              csr_val,                                 \
        const ITYPE*              csr_row_ptr,                             \
        const JTYPE*              csr_col_ind,                             \
        size_t*                   buffer_size);

// Uniform precisions
INSTANTIATE_BUFFER_SIZE(float, int32_t, int32_t, float);
INSTANTIATE_BUFFER_SIZE(float, int64_t, int32_t, float);
INSTANTIATE_BUFFER_SIZE(float, int64_t, int64_t, float);
INSTANTIATE_BUFFER_SIZE(double, int32_t, int32_t, double);
INSTANTIATE_BUFFER_SIZE(double, int64_t, int32_t, double);
INSTANTIATE_BUFFER_SIZE(double, int64_t, int64_t, double);
INSTANTIATE_BUFFER_SIZE(rocsparse_float_complex, int32_t, int32_t, rocsparse_float_complex);
INSTANTIATE_BUFFER_SIZE(rocsparse_float_complex, int64_t, int32_t, rocsparse_float_complex);
INSTANTIATE_BUFFER_SIZE(rocsparse_float_complex, int64_t, int64_t, rocsparse_float_complex);
INSTANTIATE_BUFFER_SIZE(rocsparse_double_complex, int32_t, int32_t, rocsparse_double_complex);
INSTANTIATE_BUFFER_SIZE(rocsparse_double_complex, int64_t, int32_t, rocsparse_double_complex);
INSTANTIATE_BUFFER_SIZE(rocsparse_double_complex, int64_t, int64_t, rocsparse_double_complex);

// Mixed precisions
INSTANTIATE_BUFFER_SIZE(int32_t, int32_t, int32_t, int8_t);
INSTANTIATE_BUFFER_SIZE(int32_t, int64_t, int32_t, int8_t);
INSTANTIATE_BUFFER_SIZE(int32_t, int64_t, int64_t, int8_t);
INSTANTIATE_BUFFER_SIZE(float, int32_t, int32_t, int8_t);
INSTANTIATE_BUFFER_SIZE(float, int64_t, int32_t, int8_t);
INSTANTIATE_BUFFER_SIZE(float, int64_t, int64_t, int8_t);
#undef INSTANTIATE_BUFFER_SIZE

#define INSTANTIATE_ANALYSIS(TTYPE, ITYPE, JTYPE, ATYPE)                \
    template rocsparse_status rocsparse_csrmm_analysis_template<TTYPE>( \
        rocsparse_handle          handle,                               \
        rocsparse_operation       trans_A,                              \
        rocsparse_csrmm_alg       alg,                                  \
        JTYPE                     m,                                    \
        JTYPE                     n,                                    \
        JTYPE                     k,                                    \
        ITYPE                     nnz,                                  \
        const rocsparse_mat_descr descr,                                \
        const ATYPE*              csr_val,                              \
        const ITYPE*              csr_row_ptr,                          \
        const JTYPE*              csr_col_ind,                          \
        void*                     temp_buffer);

// Uniform precisions
INSTANTIATE_ANALYSIS(float, int32_t, int32_t, float);
INSTANTIATE_ANALYSIS(float, int64_t, int32_t, float);
INSTANTIATE_ANALYSIS(float, int64_t, int64_t, float);
INSTANTIATE_ANALYSIS(double, int32_t, int32_t, double);
INSTANTIATE_ANALYSIS(double, int64_t, int32_t, double);
INSTANTIATE_ANALYSIS(double, int64_t, int64_t, double);
INSTANTIATE_ANALYSIS(rocsparse_float_complex, int32_t, int32_t, rocsparse_float_complex);
INSTANTIATE_ANALYSIS(rocsparse_float_complex, int64_t, int32_t, rocsparse_float_complex);
INSTANTIATE_ANALYSIS(rocsparse_float_complex, int64_t, int64_t, rocsparse_float_complex);
INSTANTIATE_ANALYSIS(rocsparse_double_complex, int32_t, int32_t, rocsparse_double_complex);
INSTANTIATE_ANALYSIS(rocsparse_double_complex, int64_t, int32_t, rocsparse_double_complex);
INSTANTIATE_ANALYSIS(rocsparse_double_complex, int64_t, int64_t, rocsparse_double_complex);

// Mixed precisions
INSTANTIATE_ANALYSIS(int32_t, int32_t, int32_t, int8_t);
INSTANTIATE_ANALYSIS(int32_t, int64_t, int32_t, int8_t);
INSTANTIATE_ANALYSIS(int32_t, int64_t, int64_t, int8_t);
INSTANTIATE_ANALYSIS(float, int32_t, int32_t, int8_t);
INSTANTIATE_ANALYSIS(float, int64_t, int32_t, int8_t);
INSTANTIATE_ANALYSIS(float, int64_t, int64_t, int8_t);
#undef INSTANTIATE_ANALYSIS

#define INSTANTIATE(TTYPE, ITYPE, JTYPE, ATYPE, BTYPE, CTYPE)                                      \
    template rocsparse_status rocsparse_csrmm_template(rocsparse_handle    handle,                 \
                                                       rocsparse_operation trans_A,                \
                                                       rocsparse_operation trans_B,                \
                                                       rocsparse_order     order_B,                \
                                                       rocsparse_order     order_C,                \
                                                       rocsparse_csrmm_alg alg,                    \
                                                       JTYPE               m,                      \
                                                       JTYPE               n,                      \
                                                       JTYPE               k,                      \
                                                       ITYPE               nnz,                    \
                                                       JTYPE               batch_count_A,          \
                                                       ITYPE               offsets_batch_stride_A, \
                                                       ITYPE        columns_values_batch_stride_A, \
                                                       const TTYPE* alpha_device_host,             \
                                                       const rocsparse_mat_descr descr,            \
                                                       const ATYPE*              csr_val,          \
                                                       const ITYPE*              csr_row_ptr,      \
                                                       const JTYPE*              csr_col_ind,      \
                                                       const BTYPE*              dense_B,          \
                                                       JTYPE                     ldb,              \
                                                       JTYPE                     batch_count_B,    \
                                                       ITYPE                     batch_stride_B,   \
                                                       const TTYPE*              beta_device_host, \
                                                       CTYPE*                    dense_C,          \
                                                       JTYPE                     ldc,              \
                                                       JTYPE                     batch_count_C,    \
                                                       ITYPE                     batch_stride_C,   \
                                                       void*                     temp_buffer,      \
                                                       bool                      force_conj_A);

// Uniform precisions
INSTANTIATE(float, int32_t, int32_t, float, float, float);
INSTANTIATE(float, int64_t, int32_t, float, float, float);
INSTANTIATE(float, int64_t, int64_t, float, float, float);
INSTANTIATE(double, int32_t, int32_t, double, double, double);
INSTANTIATE(double, int64_t, int32_t, double, double, double);
INSTANTIATE(double, int64_t, int64_t, double, double, double);
INSTANTIATE(rocsparse_float_complex,
            int32_t,
            int32_t,
            rocsparse_float_complex,
            rocsparse_float_complex,
            rocsparse_float_complex);
INSTANTIATE(rocsparse_float_complex,
            int64_t,
            int32_t,
            rocsparse_float_complex,
            rocsparse_float_complex,
            rocsparse_float_complex);
INSTANTIATE(rocsparse_float_complex,
            int64_t,
            int64_t,
            rocsparse_float_complex,
            rocsparse_float_complex,
            rocsparse_float_complex);
INSTANTIATE(rocsparse_double_complex,
            int32_t,
            int32_t,
            rocsparse_double_complex,
            rocsparse_double_complex,
            rocsparse_double_complex);
INSTANTIATE(rocsparse_double_complex,
            int64_t,
            int32_t,
            rocsparse_double_complex,
            rocsparse_double_complex,
            rocsparse_double_complex);
INSTANTIATE(rocsparse_double_complex,
            int64_t,
            int64_t,
            rocsparse_double_complex,
            rocsparse_double_complex,
            rocsparse_double_complex);

// Mixed precisions
INSTANTIATE(int32_t, int32_t, int32_t, int8_t, int8_t, int32_t);
INSTANTIATE(int32_t, int64_t, int32_t, int8_t, int8_t, int32_t);
INSTANTIATE(int32_t, int64_t, int64_t, int8_t, int8_t, int32_t);
INSTANTIATE(float, int32_t, int32_t, int8_t, int8_t, float);
INSTANTIATE(float, int64_t, int32_t, int8_t, int8_t, float);
INSTANTIATE(float, int64_t, int64_t, int8_t, int8_t, float);
#undef INSTANTIATE

/*
* ===========================================================================
*    C wrapper
* ===========================================================================
*/

#define C_IMPL(NAME, TYPE)                                                  \
    extern "C" rocsparse_status NAME(rocsparse_handle          handle,      \
                                     rocsparse_operation       trans_A,     \
                                     rocsparse_operation       trans_B,     \
                                     rocsparse_int             m,           \
                                     rocsparse_int             n,           \
                                     rocsparse_int             k,           \
                                     rocsparse_int             nnz,         \
                                     const TYPE*               alpha,       \
                                     const rocsparse_mat_descr descr,       \
                                     const TYPE*               csr_val,     \
                                     const rocsparse_int*      csr_row_ptr, \
                                     const rocsparse_int*      csr_col_ind, \
                                     const TYPE*               dense_B,     \
                                     rocsparse_int             ldb,         \
                                     const TYPE*               beta,        \
                                     TYPE*                     dense_C,     \
                                     rocsparse_int             ldc)         \
    try                                                                     \
    {                                                                       \
        return rocsparse_csrmm_template(handle,                             \
                                        trans_A,                            \
                                        trans_B,                            \
                                        rocsparse_order_column,             \
                                        rocsparse_order_column,             \
                                        rocsparse_csrmm_alg_default,        \
                                        m,                                  \
                                        n,                                  \
                                        k,                                  \
                                        nnz,                                \
                                        1,                                  \
                                        0,                                  \
                                        0,                                  \
                                        alpha,                              \
                                        descr,                              \
                                        csr_val,                            \
                                        csr_row_ptr,                        \
                                        csr_col_ind,                        \
                                        dense_B,                            \
                                        ldb,                                \
                                        1,                                  \
                                        0,                                  \
                                        beta,                               \
                                        dense_C,                            \
                                        ldc,                                \
                                        1,                                  \
                                        0,                                  \
                                        nullptr,                            \
                                        false);                             \
    }                                                                       \
    catch(...)                                                              \
    {                                                                       \
        return exception_to_rocsparse_status();                             \
    }

C_IMPL(rocsparse_scsrmm, float);
C_IMPL(rocsparse_dcsrmm, double);
C_IMPL(rocsparse_ccsrmm, rocsparse_float_complex);
C_IMPL(rocsparse_zcsrmm, rocsparse_double_complex);

#undef C_IMPL
