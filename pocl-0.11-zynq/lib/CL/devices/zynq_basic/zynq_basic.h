/* zynq_basic.h - a minimalistic pocl device driver layer implementation

   Copyright (c) 2011 Universidad Rey Juan Carlos and
                 2012 Pekka Jääskeläinen / Tampere University of Technology
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/
/**
 * @file zynq_basic.h
 *
 * The purpose of the 'zynq_basic' device driver is to serve as an example of
 * a minimalistic (but still working) device driver for pocl.
 *
 * It is a "native device" without multithreading and uses the malloc
 * directly for buffer allocation etc.
 */

 /* sublicense
   File:zynq_basic.h
 *
 Copyright (c) [2016] [Mohammad Hosseinabady (mohammad@hosseinabady.com)]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
* This file has been modified at University of Bristol
* for the ENPOWER project funded by EPSRC
*
* File name : matrix_mult.h
* author    : Mohammad hosseinabady mohammad@hosseinabady.com
* date      : 1 October 2016
*/
 
#ifndef POCL_ZYNQ_BASIC_H
#define POCL_ZYNQ_BASIC_H

#include "pocl_cl.h"
#include "pocl_icd.h"
#include "config.h"

#ifndef WORDS_BIGENDIAN
#define WORDS_BIGENDIAN 0
#endif

#include "prototypes.inc"
GEN_PROTOTYPES (zynq_basic)


#define ENPOWER_OPENCL_DEVICE_NO_PORT                           1000

#define ENPOWER_OPENCL_IOC_MAGIC                                2000

#define ENPOWER_OPENCL_ARGUMEN_IMAGE_WIDTH                     _IOW(ENPOWER_OPENCL_IOC_MAGIC, 0, int)
#define ENPOWER_OPENCL_ARGUMEN_IMAGE_HEIGHT                    _IOW(ENPOWER_OPENCL_IOC_MAGIC, 1, int)
#define ENPOWER_OPENCL_ARGUMEN_INDEX                           _IOW(ENPOWER_OPENCL_IOC_MAGIC, 2, int)

#define ENPOWER_OPENCL_START                                   _IOW(ENPOWER_OPENCL_IOC_MAGIC, 3, int)
#define ENPOWER_OPENCL_CTRL                                    _IOW(ENPOWER_OPENCL_IOC_MAGIC, 4, int)

//#define __MOHAMMAD_DEBUG__

#endif /* POCL_ZYNQ_BASIC_H */
