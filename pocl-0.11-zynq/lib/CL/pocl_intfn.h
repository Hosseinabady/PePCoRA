/* pocl_intfn.h - local (non-exported) OpenCL functions

   Copyright (c) 2012 Vincent Danjean <Vincent.Danjean@ens-lyon.org>
   
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

#ifndef POCL_INTFN_H
#define POCL_INTFN_H

#ifndef POCL_CL_H
#  error this file must be included through pocl_cl.h, not directly
#endif

POdeclsym(clBuildProgram)
POdeclsym(clCreateBuffer)
POdeclsym(clCreateCommandQueue)
POdeclsym(clCreateContext)
POdeclsym(clCreateContextFromType)
POdeclsym(clCreateFromGLTexture2D)
POdeclsym(clCreateFromGLTexture3D)
POdeclsym(clCreateImage2D) 
POdeclsym(clCreateImage3D)
POdeclsym(clCreateImage)
POdeclsym(clCreateKernel)
POdeclsym(clCreateKernelsInProgram)
POdeclsym(clCreateProgramWithBinary)
POdeclsym(clCreateProgramWithSource)
POdeclsym(clCreateSampler)
POdeclsym(clCreateSubBuffer)
POdeclsym(clCreateSubDevices)
POdeclsym(clCreateUserEvent)
POdeclsym(clEnqueueBarrier)
POdeclsym(clEnqueueCopyBuffer)
POdeclsym(clEnqueueCopyBufferRect)
POdeclsym(clEnqueueCopyBufferToImage) 
POdeclsym(clEnqueueCopyImage)
POdeclsym(clEnqueueCopyImageToBuffer)
POdeclsym(clEnqueueMapBuffer)
POdeclsym(clEnqueueMapImage)
POdeclsym(clEnqueueMarker) 
POdeclsym(clEnqueueNativeKernel)
POdeclsym(clEnqueueNDRangeKernel)
POdeclsym(clEnqueueReadBuffer)
POdeclsym(clEnqueueReadBufferRect)
POdeclsym(clEnqueueReadImage)
POdeclsym(clEnqueueTask)
POdeclsym(clEnqueueUnmapMemObject)
POdeclsym(clEnqueueWaitForEvents)
POdeclsym(clEnqueueMarkerWithWaitList)
POdeclsym(clEnqueueWriteBuffer)
POdeclsym(clEnqueueWriteBufferRect)
POdeclsym(clEnqueueWriteImage)
POdeclsym(clEnqueueFillImage)
POdeclsym(clFinish)
POdeclsym(clFlush)
POdeclsym(clGetCommandQueueInfo)
POdeclsym(clGetContextInfo)
POdeclsym(clGetDeviceIDs)
POdeclsym(clGetDeviceInfo)
POdeclsym(clGetEventInfo)
POdeclsym(clGetEventProfilingInfo)
POdeclsym(clGetExtensionFunctionAddress)
POdeclsym(clGetImageInfo)
POdeclsym(clGetKernelInfo)
POdeclsym(clGetKernelArgInfo)
POdeclsym(clGetKernelWorkGroupInfo)
POdeclsym(clGetMemObjectInfo)
POdeclsym(clGetPlatformIDs)
POdeclsym(clGetPlatformInfo)
POdeclsym(clGetProgramBuildInfo)
POdeclsym(clGetProgramInfo)
POdeclsym(clGetSamplerInfo)
POdeclsym(clGetSupportedImageFormats)
POdeclsymICD(clIcdGetPlatformIDsKHR)
POdeclsym(clReleaseCommandQueue)
POdeclsym(clReleaseContext)
POdeclsym(clReleaseDevice)
POdeclsym(clReleaseEvent)
POdeclsym(clReleaseKernel)
POdeclsym(clReleaseMemObject)
POdeclsym(clReleaseProgram)
POdeclsym(clReleaseSampler)
POdeclsym(clRetainCommandQueue)
POdeclsym(clRetainContext)
POdeclsym(clRetainDevice)
POdeclsym(clRetainEvent)
POdeclsym(clRetainKernel)
POdeclsym(clRetainMemObject)
POdeclsym(clRetainProgram)
POdeclsym(clRetainSampler)
POdeclsym(clSetEventCallback)
POdeclsym(clSetKernelArg)
POdeclsym(clSetMemObjectDestructorCallback)
POdeclsym(clSetUserEventStatus)
POdeclsym(clUnloadCompiler)
POdeclsym(clWaitForEvents)

#endif
