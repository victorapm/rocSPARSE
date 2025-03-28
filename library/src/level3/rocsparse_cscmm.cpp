/*! \file */
/* ************************************************************************
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights Reserved.
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

#include "rocsparse_cscmm.hpp"

#include "definitions.h"
#include "utility.h"

template <typename T, typename I, typename J, typename A>
rocsparse_status rocsparse_cscmm_buffer_size_template(rocsparse_handle          handle,
                                                      rocsparse_operation       trans_A,
                                                      rocsparse_csrmm_alg       alg,
                                                      J                         m,
                                                      J                         n,
                                                      J                         k,
                                                      I                         nnz,
                                                      const rocsparse_mat_descr descr,
                                                      const A*                  csc_val,
                                                      const I*                  csc_col_ptr,
                                                      const J*                  csc_row_ind,
                                                      size_t*                   buffer_size)
{
    switch(trans_A)
    {
    case rocsparse_operation_none:
    {
        return rocsparse_csrmm_buffer_size_template<T>(handle,
                                                       rocsparse_operation_transpose,
                                                       alg,
                                                       k,
                                                       n,
                                                       m,
                                                       nnz,
                                                       descr,
                                                       csc_val,
                                                       csc_col_ptr,
                                                       csc_row_ind,
                                                       buffer_size);
    }
    case rocsparse_operation_transpose:
    case rocsparse_operation_conjugate_transpose:
    {
        return rocsparse_csrmm_buffer_size_template<T>(handle,
                                                       rocsparse_operation_none,
                                                       alg,
                                                       k,
                                                       n,
                                                       m,
                                                       nnz,
                                                       descr,
                                                       csc_val,
                                                       csc_col_ptr,
                                                       csc_row_ind,
                                                       buffer_size);
    }
    }

    return rocsparse_status_not_implemented;
}

template <typename T, typename I, typename J, typename A>
rocsparse_status rocsparse_cscmm_analysis_template(rocsparse_handle          handle,
                                                   rocsparse_operation       trans_A,
                                                   rocsparse_csrmm_alg       alg,
                                                   J                         m,
                                                   J                         n,
                                                   J                         k,
                                                   I                         nnz,
                                                   const rocsparse_mat_descr descr,
                                                   const A*                  csc_val,
                                                   const I*                  csc_col_ptr,
                                                   const J*                  csc_row_ind,
                                                   void*                     temp_buffer)
{
    switch(trans_A)
    {
    case rocsparse_operation_none:
    {
        return rocsparse_csrmm_analysis_template<T>(handle,
                                                    rocsparse_operation_transpose,
                                                    alg,
                                                    k,
                                                    n,
                                                    m,
                                                    nnz,
                                                    descr,
                                                    csc_val,
                                                    csc_col_ptr,
                                                    csc_row_ind,
                                                    temp_buffer);
    }
    case rocsparse_operation_transpose:
    case rocsparse_operation_conjugate_transpose:
    {
        return rocsparse_csrmm_analysis_template<T>(handle,
                                                    rocsparse_operation_none,
                                                    alg,
                                                    k,
                                                    n,
                                                    m,
                                                    nnz,
                                                    descr,
                                                    csc_val,
                                                    csc_col_ptr,
                                                    csc_row_ind,
                                                    temp_buffer);
    }
    }

    return rocsparse_status_not_implemented;
}

template <typename T, typename I, typename J, typename A, typename B, typename C>
rocsparse_status rocsparse_cscmm_template(rocsparse_handle          handle,
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
                                          I                         rows_values_batch_stride_A,
                                          const T*                  alpha_device_host,
                                          const rocsparse_mat_descr descr,
                                          const A*                  csc_val,
                                          const I*                  csc_col_ptr,
                                          const J*                  csc_row_ind,
                                          const B*                  dense_B,
                                          J                         ldb,
                                          J                         batch_count_B,
                                          I                         batch_stride_B,
                                          const T*                  beta_device_host,
                                          C*                        dense_C,
                                          J                         ldc,
                                          J                         batch_count_C,
                                          I                         batch_stride_C,
                                          void*                     temp_buffer)
{
    switch(trans_A)
    {
    case rocsparse_operation_none:
    {
        return rocsparse_csrmm_template(handle,
                                        rocsparse_operation_transpose,
                                        trans_B,
                                        order_B,
                                        order_C,
                                        alg,
                                        k,
                                        n,
                                        m,
                                        nnz,
                                        batch_count_A,
                                        offsets_batch_stride_A,
                                        rows_values_batch_stride_A,
                                        alpha_device_host,
                                        descr,
                                        csc_val,
                                        csc_col_ptr,
                                        csc_row_ind,
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
                                        false);
    }
    case rocsparse_operation_transpose:
    {
        return rocsparse_csrmm_template<T>(handle,
                                           rocsparse_operation_none,
                                           trans_B,
                                           order_B,
                                           order_C,
                                           alg,
                                           k,
                                           n,
                                           m,
                                           nnz,
                                           batch_count_A,
                                           offsets_batch_stride_A,
                                           rows_values_batch_stride_A,
                                           alpha_device_host,
                                           descr,
                                           csc_val,
                                           csc_col_ptr,
                                           csc_row_ind,
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
                                           false);
    }
    case rocsparse_operation_conjugate_transpose:
    {
        return rocsparse_csrmm_template<T>(handle,
                                           rocsparse_operation_none,
                                           trans_B,
                                           order_B,
                                           order_C,
                                           alg,
                                           k,
                                           n,
                                           m,
                                           nnz,
                                           batch_count_A,
                                           offsets_batch_stride_A,
                                           rows_values_batch_stride_A,
                                           alpha_device_host,
                                           descr,
                                           csc_val,
                                           csc_col_ptr,
                                           csc_row_ind,
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
                                           true);
    }
    }

    return rocsparse_status_not_implemented;
}

#define INSTANTIATE_BUFFER_SIZE(TTYPE, ITYPE, JTYPE, ATYPE)                \
    template rocsparse_status rocsparse_cscmm_buffer_size_template<TTYPE>( \
        rocsparse_handle          handle,                                  \
        rocsparse_operation       trans_A,                                 \
        rocsparse_csrmm_alg       alg,                                     \
        JTYPE                     m,                                       \
        JTYPE                     n,                                       \
        JTYPE                     k,                                       \
        ITYPE                     nnz,                                     \
        const rocsparse_mat_descr descr,                                   \
        const ATYPE*              csc_val,                                 \
        const ITYPE*              csc_col_ptr,                             \
        const JTYPE*              csc_row_ind,                             \
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
    template rocsparse_status rocsparse_cscmm_analysis_template<TTYPE>( \
        rocsparse_handle          handle,                               \
        rocsparse_operation       trans_A,                              \
        rocsparse_csrmm_alg       alg,                                  \
        JTYPE                     m,                                    \
        JTYPE                     n,                                    \
        JTYPE                     k,                                    \
        ITYPE                     nnz,                                  \
        const rocsparse_mat_descr descr,                                \
        const ATYPE*              csc_val,                              \
        const ITYPE*              csc_col_ptr,                          \
        const JTYPE*              csc_row_ind,                          \
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
    template rocsparse_status rocsparse_cscmm_template(rocsparse_handle    handle,                 \
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
                                                       ITYPE        rows_values_batch_stride_A,    \
                                                       const TTYPE* alpha_device_host,             \
                                                       const rocsparse_mat_descr descr,            \
                                                       const ATYPE*              csc_val,          \
                                                       const ITYPE*              csc_col_ptr,      \
                                                       const JTYPE*              csc_row_ind,      \
                                                       const BTYPE*              B,                \
                                                       JTYPE                     ldb,              \
                                                       JTYPE                     batch_count_B,    \
                                                       ITYPE                     batch_stride_B,   \
                                                       const TTYPE*              beta_device_host, \
                                                       CTYPE*                    C,                \
                                                       JTYPE                     ldc,              \
                                                       JTYPE                     batch_count_C,    \
                                                       ITYPE                     batch_stride_C,   \
                                                       void*                     temp_buffer);

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

// #define INSTANTIATE_MIXED_BUFFERSIZE(ITYPE, JTYPE, ATYPE)                      \
//     template rocsparse_status rocsparse_cscmm_buffer_size_template( \
//         rocsparse_handle          handle,                                                \
//         rocsparse_operation       trans_A,                                               \
//         rocsparse_csrmm_alg       alg,                                                   \
//         JTYPE                     m,                                                     \
//         JTYPE                     n,                                                     \
//         JTYPE                     k,                                                     \
//         ITYPE                     nnz,                                                   \
//         const rocsparse_mat_descr descr,                                                 \
//         const ATYPE*              csc_val,                                               \
//         const ITYPE*              csc_col_ptr,                                           \
//         const JTYPE*              csc_row_ind,                                           \
//         size_t*                   buffer_size);

// INSTANTIATE_MIXED_BUFFERSIZE(int32_t, int32_t, int8_t);
// INSTANTIATE_MIXED_BUFFERSIZE(int64_t, int32_t, int8_t);
// INSTANTIATE_MIXED_BUFFERSIZE(int64_t, int64_t, int8_t);
// #undef INSTANTIATE_MIXED_BUFFERSIZE

// #define INSTANTIATE_MIXED_ANALYSIS(ITYPE, JTYPE, ATYPE)      \
//     template rocsparse_status rocsparse_cscmm_analysis_template( \
//         rocsparse_handle          handle,                                             \
//         rocsparse_operation       trans_A,                                            \
//         rocsparse_csrmm_alg       alg,                                                \
//         JTYPE                     m,                                                  \
//         JTYPE                     n,                                                  \
//         JTYPE                     k,                                                  \
//         ITYPE                     nnz,                                                \
//         const rocsparse_mat_descr descr,                                              \
//         const ATYPE*              csc_val,                                            \
//         const ITYPE*              csc_col_ptr,                                        \
//         const JTYPE*              csc_row_ind,                                        \
//         void*                     temp_buffer);

// INSTANTIATE_MIXED_ANALYSIS(int32_t, int32_t, int8_t);
// INSTANTIATE_MIXED_ANALYSIS(int64_t, int32_t, int8_t);
// INSTANTIATE_MIXED_ANALYSIS(int64_t, int64_t, int8_t);
// #undef INSTANTIATE_MIXED_ANALYSIS

// #define INSTANTIATE_MIXED(TTYPE, ITYPE, JTYPE, ATYPE, BTYPE, CTYPE)      \
//     template rocsparse_status rocsparse_cscmm_template( \
//         rocsparse_handle          handle,                                    \
//         rocsparse_operation       trans_A,                                   \
//         rocsparse_operation       trans_B,                                   \
//         rocsparse_order           order_B,                                   \
//         rocsparse_order           order_C,                                   \
//         rocsparse_csrmm_alg       alg,                                       \
//         JTYPE                     m,                                         \
//         JTYPE                     n,                                         \
//         JTYPE                     k,                                         \
//         ITYPE                     nnz,                                       \
//         JTYPE                     batch_count_A,                             \
//         ITYPE                     offsets_batch_stride_A,                    \
//         ITYPE                     rows_values_batch_stride_A,                \
//         const TTYPE*              alpha_device_host,                         \
//         const rocsparse_mat_descr descr,                                     \
//         const ATYPE*              csc_val,                                   \
//         const ITYPE*              csc_col_ptr,                               \
//         const JTYPE*              csc_row_ind,                               \
//         const BTYPE*              B,                                         \
//         JTYPE                     ldb,                                       \
//         JTYPE                     batch_count_B,                             \
//         ITYPE                     batch_stride_B,                            \
//         const TTYPE*              beta_device_host,                          \
//         CTYPE*                    C,                                         \
//         JTYPE                     ldc,                                       \
//         JTYPE                     batch_count_C,                             \
//         ITYPE                     batch_stride_C,                            \
//         void*                     temp_buffer);

// INSTANTIATE_MIXED(int32_t, int32_t, int32_t, int8_t, int8_t, int32_t);
// INSTANTIATE_MIXED(int32_t, int64_t, int32_t, int8_t, int8_t, int32_t);
// INSTANTIATE_MIXED(int32_t, int64_t, int64_t, int8_t, int8_t, int32_t);
// INSTANTIATE_MIXED(float, int32_t, int32_t, int8_t, int8_t, float);
// INSTANTIATE_MIXED(float, int64_t, int32_t, int8_t, int8_t, float);
// INSTANTIATE_MIXED(float, int64_t, int64_t, int8_t, int8_t, float);
// #undef INSTANTIATE_MIXED
