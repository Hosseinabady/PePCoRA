/* OpenCL runtime library: clGetEventProfilingInfo()

   Copyright (c) 2011 Erik Schnetter
   
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

#include "pocl_cl.h"
#include <string.h>

CL_API_ENTRY cl_int CL_API_CALL
POname(clGetEventProfilingInfo)(cl_event event,
                        cl_profiling_info param_name,
                        size_t param_value_size,
                        void *param_value,
                        size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
  size_t const value_size = sizeof(cl_ulong);

  POCL_RETURN_ERROR_COND((event == NULL), CL_INVALID_EVENT);

  POCL_RETURN_ERROR_ON(((event->queue->properties & CL_QUEUE_PROFILING_ENABLE) == 0),
    CL_PROFILING_INFO_NOT_AVAILABLE, "Cannot return profiling info when profiling "
      "is disabled on the queue\n");
  POCL_RETURN_ERROR_ON((event->status != CL_COMPLETE), CL_PROFILING_INFO_NOT_AVAILABLE,
    "Cannot return profiling info on events not CL_COMPLETE yet\n");

  if (param_value)
  {
    if (param_value_size < value_size) return CL_INVALID_VALUE;
    
    switch (param_name)
    {
    case CL_PROFILING_COMMAND_QUEUED:
      *(cl_ulong*)param_value = event->time_queue;
      break;
    case CL_PROFILING_COMMAND_SUBMIT:
      *(cl_ulong*)param_value = event->time_submit;
      break;
    case CL_PROFILING_COMMAND_START:
      *(cl_ulong*)param_value = event->time_start;
      break;
    case CL_PROFILING_COMMAND_END:
      *(cl_ulong*)param_value = event->time_end;
      break;
    default:
      return CL_INVALID_VALUE;
    }
  }
  
  if (param_value_size_ret)
    *param_value_size_ret = value_size;
  
  return CL_SUCCESS;
}
POsym(clGetEventProfilingInfo)
