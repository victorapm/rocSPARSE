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

#include "rocsparse_sctr.hpp"

/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" rocsparse_status rocsparse_ssctr(rocsparse_handle     handle,
                                            rocsparse_int        nnz,
                                            const float*         x_val,
                                            const rocsparse_int* x_ind,
                                            float*               y,
                                            rocsparse_index_base idx_base)
try
{
    return rocsparse_sctr_template(handle, nnz, x_val, x_ind, y, idx_base);
}
catch(...)
{
    return exception_to_rocsparse_status();
}

extern "C" rocsparse_status rocsparse_dsctr(rocsparse_handle     handle,
                                            rocsparse_int        nnz,
                                            const double*        x_val,
                                            const rocsparse_int* x_ind,
                                            double*              y,
                                            rocsparse_index_base idx_base)
try
{
    return rocsparse_sctr_template(handle, nnz, x_val, x_ind, y, idx_base);
}
catch(...)
{
    return exception_to_rocsparse_status();
}

extern "C" rocsparse_status rocsparse_csctr(rocsparse_handle               handle,
                                            rocsparse_int                  nnz,
                                            const rocsparse_float_complex* x_val,
                                            const rocsparse_int*           x_ind,
                                            rocsparse_float_complex*       y,
                                            rocsparse_index_base           idx_base)
try
{
    return rocsparse_sctr_template(handle, nnz, x_val, x_ind, y, idx_base);
}
catch(...)
{
    return exception_to_rocsparse_status();
}

extern "C" rocsparse_status rocsparse_zsctr(rocsparse_handle                handle,
                                            rocsparse_int                   nnz,
                                            const rocsparse_double_complex* x_val,
                                            const rocsparse_int*            x_ind,
                                            rocsparse_double_complex*       y,
                                            rocsparse_index_base            idx_base)
try
{
    return rocsparse_sctr_template(handle, nnz, x_val, x_ind, y, idx_base);
}
catch(...)
{
    return exception_to_rocsparse_status();
}

extern "C" rocsparse_status rocsparse_isctr(rocsparse_handle     handle,
                                            rocsparse_int        nnz,
                                            const rocsparse_int* x_val,
                                            const rocsparse_int* x_ind,
                                            rocsparse_int*       y,
                                            rocsparse_index_base idx_base)
try
{
    return rocsparse_sctr_template(handle, nnz, x_val, x_ind, y, idx_base);
}
catch(...)
{
    return exception_to_rocsparse_status();
}
