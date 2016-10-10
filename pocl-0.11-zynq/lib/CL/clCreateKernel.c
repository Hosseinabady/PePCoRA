/* OpenCL runtime library: clCreateKernel()

   Copyright (c) 2011 Universidad Rey Juan Carlos and
                 2012 Pekka Jääskeläinen / Tampere Univ. of Technology
   
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
#include "pocl_llvm.h"
#include <string.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#  include <unistd.h>
#else
#  include "vccompat.hpp"
#endif


//#define __MOHAMMAD_DEBUG__

#define COMMAND_LENGTH 1024

CL_API_ENTRY cl_kernel CL_API_CALL
POname(clCreateKernel)(cl_program program,
               const char *kernel_name,
               cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);

	  printf("vendor0 = %s \n", program->devices[0]->vendor);
	  printf("vendor1 = %s \n", program->devices[1]->vendor);
	  printf("vendor2 = %s \n", program->devices[2]->vendor);

#endif// __MOHAMMAD_DEBUG__
  cl_kernel kernel = NULL;
  char device_cachedir[POCL_FILENAME_LENGTH];
  char descriptor_filename[POCL_FILENAME_LENGTH];
  int errcode;
  int error;
  int device_i;

  POCL_GOTO_ERROR_COND((kernel_name == NULL), CL_INVALID_VALUE);

  POCL_GOTO_ERROR_COND((program == NULL), CL_INVALID_VALUE);
  
  POCL_GOTO_ERROR_ON((program->num_devices == 0),
    CL_INVALID_PROGRAM, "Invalid program (has no devices assigned)\n");

  POCL_GOTO_ERROR_ON((program->binaries == NULL || program->binary_sizes == NULL),
    CL_INVALID_PROGRAM_EXECUTABLE, "No binaries in program (perhaps you forgot "
    "to call clBuildProgram first ?\n");
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  kernel = (cl_kernel) malloc(sizeof(struct _cl_kernel));
  if (kernel == NULL)
  {
    errcode = CL_OUT_OF_HOST_MEMORY;
    goto ERROR;
  }
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  POCL_INIT_OBJECT (kernel);
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  for (device_i = 0; device_i < program->num_devices; ++device_i)
    {
      if (device_i > 0)
        POname(clRetainKernel) (kernel);
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
      snprintf (device_cachedir, POCL_FILENAME_LENGTH, "%s/%s",
                program->cache_dir, program->devices[device_i]->cache_dir_name);

      /* If there is no device dir for this device, the program was
         not built for that device in clBuildProgram. This seems to
         be OK by the standard. */
      if (access (device_cachedir, F_OK) != 0) continue;
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s) = dev_id = %d, kernel_name=%s \n", __LINE__, __FILE__, __func__, program->devices[device_i]->dev_id, kernel_name);
#endif// __MOHAMMAD_DEBUG__
      error = pocl_llvm_get_kernel_metadata 
          (program, kernel, program->devices[device_i]->dev_id, kernel_name,
           device_cachedir, descriptor_filename, &errcode);

      if (error)
        {
          POCL_MSG_ERR("Failed to get kernel metadata "
            "for kernel %s on device %s\n", kernel_name,
              program->devices[device_i]->short_name);
          goto ERROR;
        } 
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
      /* when using the API, there is no descriptor file */
    }
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  /* TODO: one of these two could be eliminated?  */
  kernel->function_name = strdup(kernel_name);
  kernel->name = strdup(kernel_name);

  kernel->context = program->context;
  kernel->program = program;
  kernel->next = NULL;

  POCL_LOCK_OBJ (program);
  cl_kernel k = program->kernels;
  program->kernels = kernel;
  POCL_UNLOCK_OBJ (program);
  kernel->next = k;

  POCL_RETAIN_OBJECT(program);

  if (errcode_ret != NULL)
    *errcode_ret = CL_SUCCESS;
  return kernel;
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
ERROR:
  POCL_MEM_FREE(kernel);
  if(errcode_ret != NULL)
  {
    *errcode_ret = errcode;
  }
  return NULL;
}
POsym(clCreateKernel)
