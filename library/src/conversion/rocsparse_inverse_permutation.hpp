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

#pragma once

#include "rocsparse.h"

template <typename I>
rocsparse_status rocsparse_inverse_permutation_permutation_core(rocsparse_handle handle,
                                                                I                n,
                                                                const I* __restrict__ p,
                                                                I* __restrict__ q,
                                                                rocsparse_index_base base_);
template <typename I>
rocsparse_status rocsparse_inverse_permutation_permutation_impl(rocsparse_handle handle,
                                                                I                n,
                                                                const I* __restrict__ p,
                                                                I* __restrict__ q,
                                                                rocsparse_index_base base_);
template <typename I>
rocsparse_status rocsparse_inverse_permutation_permutation_template(rocsparse_handle handle,
                                                                    I                n,
                                                                    const I* __restrict__ p,
                                                                    I* __restrict__ q,
                                                                    rocsparse_index_base base_);
