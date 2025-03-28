/*! \file */
/* ************************************************************************
 * Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights Reserved.
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
#include "rocsparse_inverse_permutation.hpp"
#include "utility.h"

template <unsigned int BLOCKSIZE, typename I>
ROCSPARSE_KERNEL(BLOCKSIZE)
void kernel1(I n_, const I* p_, I* q_)
{
    I idx = hipBlockIdx_x * BLOCKSIZE + hipThreadIdx_x;
    if(idx >= n_)
    {
        return;
    }

    q_[p_[idx] - 1] = idx + 1;
}

template <unsigned int BLOCKSIZE, typename I>
ROCSPARSE_KERNEL(BLOCKSIZE)
void kernel0(I n_, const I* p_, I* q_)
{
    I idx = hipBlockIdx_x * BLOCKSIZE + hipThreadIdx_x;
    if(idx >= n_)
    {
        return;
    }
    q_[p_[idx]] = idx;
}

template <typename I>
rocsparse_status rocsparse_inverse_permutation_core(rocsparse_handle handle_,
                                                    I                n_,
                                                    const I* __restrict__ p_,
                                                    I* __restrict__ q_,
                                                    rocsparse_index_base base_)
{
    static constexpr unsigned int BLOCKSIZE = 512;
    dim3                          blocks((n_ - 1) / BLOCKSIZE + 1);
    dim3                          threads(BLOCKSIZE);
    if(base_ == rocsparse_index_base_zero)
    {
        hipLaunchKernelGGL((kernel0<BLOCKSIZE>), blocks, threads, 0, handle_->stream, n_, p_, q_);
    }
    else
    {
        hipLaunchKernelGGL((kernel1<BLOCKSIZE>), blocks, threads, 0, handle_->stream, n_, p_, q_);
    }
    return rocsparse_status_success;
}

template <typename I>
rocsparse_status rocsparse_inverse_permutation_template(rocsparse_handle handle_,
                                                        I                n_,
                                                        const I* __restrict__ p_,
                                                        I* __restrict__ q_,
                                                        rocsparse_index_base base_)
{
    // Quick return if possible
    if(n_ == 0)
    {
        return rocsparse_status_success;
    }
    return rocsparse_inverse_permutation_core(handle_, n_, p_, q_, base_);
}

template <typename I>
rocsparse_status rocsparse_inverse_permutation_impl(rocsparse_handle handle_,
                                                    I                n_,
                                                    const I* __restrict__ p_,
                                                    I* __restrict__ q_,
                                                    rocsparse_index_base base_)
{
    // Check for valid handle
    if(handle_ == nullptr)
    {
        return rocsparse_status_invalid_handle;
    }

    // Logging
    log_trace(
        handle_, "rocsparse_inverse_permutation", n_, (const void*&)p_, (const void*&)q_, base_);
    log_bench(handle_, "./rocsparse-bench -f inverse_permutation", "-n", n_);

    // Check sizes
    if(n_ < 0)
    {
        return rocsparse_status_invalid_size;
    }

    // Check pointer arguments
    if(n_ > 0 && p_ == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    if(n_ > 0 && q_ == nullptr)
    {
        return rocsparse_status_invalid_pointer;
    }

    if(rocsparse_enum_utils::is_invalid(base_))
    {
        return rocsparse_status_invalid_value;
    }

    return rocsparse_inverse_permutation_template(handle_, n_, p_, q_, base_);
}

#define INSTANTIATE(ITYPE)                                                                         \
    template rocsparse_status rocsparse_inverse_permutation_template(rocsparse_handle,             \
                                                                     ITYPE,                        \
                                                                     const ITYPE* __restrict__,    \
                                                                     ITYPE* __restrict__,          \
                                                                     rocsparse_index_base);        \
    template rocsparse_status rocsparse_inverse_permutation_impl<ITYPE>(rocsparse_handle,          \
                                                                        ITYPE,                     \
                                                                        const ITYPE* __restrict__, \
                                                                        ITYPE* __restrict__,       \
                                                                        rocsparse_index_base)

INSTANTIATE(int32_t);
INSTANTIATE(int64_t);
#undef INSTANTIATE

/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */
extern "C" rocsparse_status rocsparse_inverse_permutation(rocsparse_handle     handle_,
                                                          rocsparse_int        n_,
                                                          const rocsparse_int* p_,
                                                          rocsparse_int*       q_,
                                                          rocsparse_index_base base_)
try
{
    return rocsparse_inverse_permutation_impl(handle_, n_, p_, q_, base_);
}
catch(...)
{
    return exception_to_rocsparse_status();
}
