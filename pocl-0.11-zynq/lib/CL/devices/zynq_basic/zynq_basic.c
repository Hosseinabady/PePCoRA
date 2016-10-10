/* zynq_basic.c - a minimalistic pocl device driver layer implementation

   Copyright (c) 2011-2013 Universidad Rey Juan Carlos and
                 2011-2014 Pekka Jääskeläinen / Tampere University of Technology
   
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


/* sublicense
   File: zynq_basic.c
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

#include <stdbool.h>
#include "zynq_basic.h"
#include "fpga_design.h"
#include "cpuinfo.h"
#include "topology/pocl_topology.h"
#include "common.h"
#include "utlist.h"
#include "devices.h"
#include "fpgacl_device_driver.h"
#include "design_kernel.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dev_image.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>		/* ioctl */
#include <sys/time.h>
#define max(a,b) (((a) > (b)) ? (a) : (b))


#define COMMAND_LENGTH 2048
#define WORKGROUP_STRING_LENGTH 128


double getTimestamp();
struct timeval start_time, end_time;
double t1, t2;

double hardware_start;
double hardware_end;

struct timeval fpga_start_time, fpga_end_time;
double fpga_t1, fpga_t2;

#define DEVICE_NAME_ACP    "/dev/fpgacl_acp"
#define DEVICE_NAME_HP0    "/dev/fpgacl_hp0"
#define DEVICE_NAME_HP1    "/dev/fpgacl_hp1"
#define DEVICE_NAME_HP2    "/dev/fpgacl_hp2"
#define DEVICE_NAME_HP3    "/dev/fpgacl_hp3"

struct data {
  /* Currently loaded kernel. */
  cl_kernel current_kernel;
  /* Loaded kernel dynamic library handle. */
  lt_dlhandle current_dlhandle;
};

int flgacl_dev_acp;
int flgacl_dev_hp0;
int flgacl_dev_hp1;
int flgacl_dev_hp2;
int flgacl_dev_hp3;

const cl_image_format zynq_basic_supported_image_formats[] = {
    { CL_R, CL_SNORM_INT8 },
    { CL_R, CL_SNORM_INT16 },
    { CL_R, CL_UNORM_INT8 },
    { CL_R, CL_UNORM_INT16 },
    { CL_R, CL_UNORM_SHORT_565 }, 
    { CL_R, CL_UNORM_SHORT_555 },
    { CL_R, CL_UNORM_INT_101010 }, 
    { CL_R, CL_SIGNED_INT8 },
    { CL_R, CL_SIGNED_INT16 }, 
    { CL_R, CL_SIGNED_INT32 },
    { CL_R, CL_UNSIGNED_INT8 }, 
    { CL_R, CL_UNSIGNED_INT16 },
    { CL_R, CL_UNSIGNED_INT32 }, 
    { CL_R, CL_HALF_FLOAT },
    { CL_R, CL_FLOAT },
    { CL_Rx, CL_SNORM_INT8 },
    { CL_Rx, CL_SNORM_INT16 },
    { CL_Rx, CL_UNORM_INT8 },
    { CL_Rx, CL_UNORM_INT16 },
    { CL_Rx, CL_UNORM_SHORT_565 }, 
    { CL_Rx, CL_UNORM_SHORT_555 },
    { CL_Rx, CL_UNORM_INT_101010 }, 
    { CL_Rx, CL_SIGNED_INT8 },
    { CL_Rx, CL_SIGNED_INT16 }, 
    { CL_Rx, CL_SIGNED_INT32 },
    { CL_Rx, CL_UNSIGNED_INT8 }, 
    { CL_Rx, CL_UNSIGNED_INT16 },
    { CL_Rx, CL_UNSIGNED_INT32 }, 
    { CL_Rx, CL_HALF_FLOAT },
    { CL_Rx, CL_FLOAT },
    { CL_A, CL_SNORM_INT8 },
    { CL_A, CL_SNORM_INT16 },
    { CL_A, CL_UNORM_INT8 },
    { CL_A, CL_UNORM_INT16 },
    { CL_A, CL_UNORM_SHORT_565 }, 
    { CL_A, CL_UNORM_SHORT_555 },
    { CL_A, CL_UNORM_INT_101010 }, 
    { CL_A, CL_SIGNED_INT8 },
    { CL_A, CL_SIGNED_INT16 }, 
    { CL_A, CL_SIGNED_INT32 },
    { CL_A, CL_UNSIGNED_INT8 }, 
    { CL_A, CL_UNSIGNED_INT16 },
    { CL_A, CL_UNSIGNED_INT32 }, 
    { CL_A, CL_HALF_FLOAT },
    { CL_A, CL_FLOAT },
    { CL_RG, CL_SNORM_INT8 },
    { CL_RG, CL_SNORM_INT16 },
    { CL_RG, CL_UNORM_INT8 },
    { CL_RG, CL_UNORM_INT16 },
    { CL_RG, CL_UNORM_SHORT_565 }, 
    { CL_RG, CL_UNORM_SHORT_555 },
    { CL_RG, CL_UNORM_INT_101010 }, 
    { CL_RG, CL_SIGNED_INT8 },
    { CL_RG, CL_SIGNED_INT16 }, 
    { CL_RG, CL_SIGNED_INT32 },
    { CL_RG, CL_UNSIGNED_INT8 }, 
    { CL_RG, CL_UNSIGNED_INT16 },
    { CL_RG, CL_UNSIGNED_INT32 }, 
    { CL_RG, CL_HALF_FLOAT },
    { CL_RG, CL_FLOAT },
    { CL_RGx, CL_SNORM_INT8 },
    { CL_RGx, CL_SNORM_INT16 },
    { CL_RGx, CL_UNORM_INT8 },
    { CL_RGx, CL_UNORM_INT16 },
    { CL_RGx, CL_UNORM_SHORT_565 }, 
    { CL_RGx, CL_UNORM_SHORT_555 },
    { CL_RGx, CL_UNORM_INT_101010 }, 
    { CL_RGx, CL_SIGNED_INT8 },
    { CL_RGx, CL_SIGNED_INT16 }, 
    { CL_RGx, CL_SIGNED_INT32 },
    { CL_RGx, CL_UNSIGNED_INT8 }, 
    { CL_RGx, CL_UNSIGNED_INT16 },
    { CL_RGx, CL_UNSIGNED_INT32 }, 
    { CL_RGx, CL_HALF_FLOAT },
    { CL_RGx, CL_FLOAT },
    { CL_RA, CL_SNORM_INT8 },
    { CL_RA, CL_SNORM_INT16 },
    { CL_RA, CL_UNORM_INT8 },
    { CL_RA, CL_UNORM_INT16 },
    { CL_RA, CL_UNORM_SHORT_565 }, 
    { CL_RA, CL_UNORM_SHORT_555 },
    { CL_RA, CL_UNORM_INT_101010 }, 
    { CL_RA, CL_SIGNED_INT8 },
    { CL_RA, CL_SIGNED_INT16 }, 
    { CL_RA, CL_SIGNED_INT32 },
    { CL_RA, CL_UNSIGNED_INT8 }, 
    { CL_RA, CL_UNSIGNED_INT16 },
    { CL_RA, CL_UNSIGNED_INT32 }, 
    { CL_RA, CL_HALF_FLOAT },
    { CL_RA, CL_FLOAT },
    { CL_RGBA, CL_SNORM_INT8 },
    { CL_RGBA, CL_SNORM_INT16 },
    { CL_RGBA, CL_UNORM_INT8 },
    { CL_RGBA, CL_UNORM_INT16 },
    { CL_RGBA, CL_UNORM_SHORT_565 }, 
    { CL_RGBA, CL_UNORM_SHORT_555 },
    { CL_RGBA, CL_UNORM_INT_101010 }, 
    { CL_RGBA, CL_SIGNED_INT8 },
    { CL_RGBA, CL_SIGNED_INT16 }, 
    { CL_RGBA, CL_SIGNED_INT32 },
    { CL_RGBA, CL_UNSIGNED_INT8 }, 
    { CL_RGBA, CL_UNSIGNED_INT16 },
    { CL_RGBA, CL_UNSIGNED_INT32 }, 
    { CL_RGBA, CL_HALF_FLOAT },
    { CL_RGBA, CL_FLOAT },
    { CL_INTENSITY, CL_UNORM_INT8 }, 
    { CL_INTENSITY, CL_UNORM_INT16 }, 
    { CL_INTENSITY, CL_SNORM_INT8 }, 
    { CL_INTENSITY, CL_SNORM_INT16 }, 
    { CL_INTENSITY, CL_HALF_FLOAT }, 
    { CL_INTENSITY, CL_FLOAT },
    { CL_LUMINANCE, CL_UNORM_INT8 }, 
    { CL_LUMINANCE, CL_UNORM_INT16 }, 
    { CL_LUMINANCE, CL_SNORM_INT8 }, 
    { CL_LUMINANCE, CL_SNORM_INT16 }, 
    { CL_LUMINANCE, CL_HALF_FLOAT }, 
    { CL_LUMINANCE, CL_FLOAT },
    { CL_RGB, CL_UNORM_SHORT_565 }, 
    { CL_RGB, CL_UNORM_SHORT_555 },
    { CL_RGB, CL_UNORM_INT_101010 }, 
    { CL_RGBx, CL_UNORM_SHORT_565 }, 
    { CL_RGBx, CL_UNORM_SHORT_555 },
    { CL_RGBx, CL_UNORM_INT_101010 }, 
    { CL_ARGB, CL_SNORM_INT8 },
    { CL_ARGB, CL_UNORM_INT8 },
    { CL_ARGB, CL_SIGNED_INT8 },
    { CL_ARGB, CL_UNSIGNED_INT8 }, 
    { CL_BGRA, CL_SNORM_INT8 },
    { CL_BGRA, CL_UNORM_INT8 },
    { CL_BGRA, CL_SIGNED_INT8 },
    { CL_BGRA, CL_UNSIGNED_INT8 }
 };


void
pocl_zynq_basic_init_device_ops(struct pocl_device_ops *ops)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  ops->device_name = "zynq_basic";

  ops->init_device_infos = pocl_zynq_basic_init_device_infos;
  ops->probe = pocl_zynq_basic_probe;
  ops->uninit = pocl_zynq_basic_uninit;
  ops->init = pocl_zynq_basic_init;
  ops->alloc_mem_obj = pocl_zynq_basic_alloc_mem_obj;
  ops->free = pocl_zynq_basic_free;
  ops->read = pocl_zynq_basic_read;
  ops->read_rect = pocl_zynq_basic_read_rect;
  ops->write = pocl_zynq_basic_write;
  ops->write_rect = pocl_zynq_basic_write_rect;
  ops->copy = pocl_zynq_basic_copy;
  ops->copy_rect = pocl_zynq_basic_copy_rect;
  ops->fill_rect = pocl_zynq_basic_fill_rect;
  ops->map_mem = pocl_zynq_basic_map_mem;
  ops->compile_submitted_kernels = pocl_zynq_basic_compile_submitted_kernels;
  ops->run = pocl_zynq_basic_run;
  ops->run_native = pocl_zynq_basic_run_native;
  ops->get_timer_value = pocl_zynq_basic_get_timer_value;
  ops->get_supported_image_formats = pocl_zynq_basic_get_supported_image_formats;
}

void
pocl_zynq_basic_init_device_infos(struct _cl_device_id* dev)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  dev->type = CL_DEVICE_TYPE_ACCELERATOR;
  dev->vendor_id = 0;
  dev->max_compute_units = 4;
  dev->max_work_item_dimensions = 3;
  dev->max_work_item_sizes[0] = CL_INT_MAX;
  dev->max_work_item_sizes[1] = CL_INT_MAX;
  dev->max_work_item_sizes[2] = CL_INT_MAX;
  dev->max_work_group_size = 1024;
  dev->preferred_wg_size_multiple = 8;
  dev->preferred_vector_width_char = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_CHAR;
  dev->preferred_vector_width_short = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_SHORT;
  dev->preferred_vector_width_int = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_INT;
  dev->preferred_vector_width_long = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_LONG;
  dev->preferred_vector_width_float = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_FLOAT;
  dev->preferred_vector_width_double = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_DOUBLE;
  dev->preferred_vector_width_half = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_HALF;
  /* TODO: figure out what the difference between preferred and native widths are */
  dev->native_vector_width_char = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_CHAR;
  dev->native_vector_width_short = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_SHORT;
  dev->native_vector_width_int = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_INT;
  dev->native_vector_width_long = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_LONG;
  dev->native_vector_width_float = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_FLOAT;
  dev->native_vector_width_double = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_DOUBLE;
  dev->native_vector_width_half = POCL_DEVICES_PREFERRED_VECTOR_WIDTH_HALF;
  dev->max_clock_frequency = 0;
  dev->address_bits = POCL_DEVICE_ADDRESS_BITS;

  /* Use the minimum values until we get a more sensible
     upper limit from somewhere. */
  dev->max_read_image_args = dev->max_write_image_args = 128;
  dev->image2d_max_width = dev->image2d_max_height = 8192;
  dev->image3d_max_width = dev->image3d_max_height = dev->image3d_max_depth = 2048;
  dev->max_samplers = 16;
  dev->max_constant_args = 8;

  dev->max_mem_alloc_size = 0;
  dev->image_support = CL_TRUE;
  dev->image_max_buffer_size = 0;
  dev->image_max_array_size = 0;
  dev->max_parameter_size = 1024;
  dev->min_data_type_align_size = dev->mem_base_addr_align = MAX_EXTENDED_ALIGNMENT;
  dev->half_fp_config = 0;
  dev->single_fp_config = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN;
  dev->double_fp_config = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN;
  dev->global_mem_cache_type = CL_NONE;
  dev->global_mem_cacheline_size = 0;
  dev->global_mem_cache_size = 0;
  dev->global_mem_size = 0;
  dev->max_constant_buffer_size = 0;
  dev->local_mem_type = CL_GLOBAL;
  dev->local_mem_size = 0;
  dev->error_correction_support = CL_FALSE;
  dev->host_unified_memory = CL_TRUE;
  dev->profiling_timer_resolution = 0;
  dev->endian_little = !(WORDS_BIGENDIAN);
  dev->available = CL_TRUE;
  dev->compiler_available = CL_TRUE;
  dev->execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL;
  dev->queue_properties = CL_QUEUE_PROFILING_ENABLE;
  dev->platform = 0;
  dev->device_partition_properties[0] = 0;
  dev->printf_buffer_size = 0;
  dev->vendor = "Mohammad Hosseinabady";
  dev->profile = "EMBEDDED_PROFILE";
  /* Note: The specification describes identifiers being delimited by
     only a single space character. Some programs that check the device's
     extension  string assume this rule. Future extension additions should
     ensure that there is no more than a single space between
     identifiers. */

#ifndef _CL_DISABLE_LONG
#define DOUBLE_EXT "cl_khr_fp64 "
#else
#define DOUBLE_EXT 
#endif

#ifndef _CL_DISABLE_HALF
#define HALF_EXT "cl_khr_fp16 "
#else
#define HALF_EXT
#endif

  dev->extensions = DOUBLE_EXT HALF_EXT "cl_khr_byte_addressable_store";

  dev->llvm_target_triplet = OCL_KERNEL_TARGET;
  dev->llvm_cpu = OCL_KERNEL_TARGET_CPU;
  dev->has_64bit_long = 1;
}




typedef unsigned long int u32;


struct read_write_command_struct {
    u32*    user_data_address;
    u32     value;
    u32     argument_index;
    u32     read_write;
};
typedef struct read_write_command_struct read_write_command_type;


#define MAX_ARGUMENT_NUMBER                           100
u32* device_buffer[MAX_ARGUMENT_NUMBER];
u32* device_memory[MAX_ARGUMENT_NUMBER];
unsigned int device_buffer_index = 0;


unsigned int
pocl_zynq_basic_probe(struct pocl_device_ops *ops)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__

  int env_count = pocl_device_get_env_count(ops->device_name);


  /* No env specified, so pthread will be used instead of zynq_basic */
  if(env_count < 0)
    return 0;

  return env_count;
}

void
pocl_zynq_basic_init (cl_device_id device, const char* parameters)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  struct data *d;
  static int global_mem_id;
  static int first_zynq_basic_init = 1;
  
  if (first_zynq_basic_init)
    {
      first_zynq_basic_init = 0;
      global_mem_id = device->dev_id;
    }

  device->global_mem_id = global_mem_id;

  d = (struct data *) calloc (1, sizeof (struct data));
  
  d->current_kernel = NULL;
  d->current_dlhandle = 0;
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__

  device->data = d;
  pocl_topology_detect_device_info(device);
  pocl_cpuinfo_detect_device_info(device);

  /* The zynq_basic driver represents only one "compute unit"
     it doesn't exploit multiple hardware threads. Multiple
     zynq_basic devices can be still used for task level parallelism 
     using multiple OpenCL devices. */
  device->max_compute_units = 1;

  if(!strcmp(device->llvm_cpu, "(unknown)"))
    device->llvm_cpu = NULL;

  //Open zynq fpga opencl devices
  flgacl_dev_acp = open(DEVICE_NAME_ACP, O_RDWR);
  if (flgacl_dev_acp < 0) {
    printf("Can't open device file :%s\n", DEVICE_NAME_ACP);
    exit(-1);
  }

  flgacl_dev_hp0 = open(DEVICE_NAME_HP0, O_RDWR);
  if (flgacl_dev_hp0 < 0) {
    printf("Can't open device file :%s\n", DEVICE_NAME_HP0);
    exit(-1);
  }

  flgacl_dev_hp1 = open(DEVICE_NAME_HP1, O_RDWR);
  if (flgacl_dev_hp1 < 0) {
    printf("Can't open device file :%s\n", DEVICE_NAME_HP1);
    exit(-1);
  }

  flgacl_dev_hp2 = open(DEVICE_NAME_HP2, O_RDWR);
  if (flgacl_dev_hp2 < 0) {
    printf("Can't open device file :%s\n", DEVICE_NAME_HP2);
    exit(-1);
  }

  flgacl_dev_hp3 = open(DEVICE_NAME_HP3, O_RDWR);
  if (flgacl_dev_hp3 < 0) {
    printf("Can't open device file :%s\n", DEVICE_NAME_HP3);
    exit(-1);
  }
	
  // work-around LLVM bug where sizeof(long)=4
  #ifdef _CL_DISABLE_LONG
  device->has_64bit_long=0;
  #endif

}


void *
pocl_zynq_basic_malloc (void *device_data, cl_mem_flags flags,
		    size_t size, void *host_ptr)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  void *b;

//  printf("check point in zynq malloc size = %lu  buffer address = %p\n", size, host_ptr);

//  u32 dev_physical_address = ioctl ( file_desc, ENPOWER_OPENCL_MALLOC, size);
//  device_buffer[device_buffer_index] = dev_physical_address;
//  device_buffer_index++;
//  return dev_physical_address;
  /*
  if (flags & CL_MEM_COPY_HOST_PTR)
    {
      if (posix_memalign (&b, MAX_EXTENDED_ALIGNMENT, size) == 0)
        {
          memcpy (b, host_ptr, size);
          return b;
        }
      
      return NULL;
    }
  
  if (flags & CL_MEM_USE_HOST_PTR && host_ptr != NULL)
    {
      return host_ptr;
    }

  if (posix_memalign (&b, MAX_EXTENDED_ALIGNMENT, size) == 0)
    return b;
  */
  return NULL;
  
}

cl_int
pocl_zynq_basic_alloc_mem_obj (cl_device_id device, cl_mem mem_obj)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  void *b = NULL;
  unsigned int size = IN_LINE_SIZE;//mem_obj->size;
  b = mem_obj->mem_host_ptr;
#ifdef __MOHAMMAD_DEBUG__
  printf("check point in zynq pocl_zynq_basic_alloc_mem_obj size = %d ptr=%p\n", size, b);
#endif// __MOHAMMAD_DEBUG__
//  u32 dev_physical_address = ioctl ( file_desc, ENPOWER_OPENCL_MALLOC, size);
//  device_buffer[device_buffer_index] = dev_physical_address;
  device_memory[device_buffer_index]= b;
  device_buffer_index++;
  mem_obj->device_ptrs[device->global_mem_id].mem_ptr = b;


  /*
  struct data* d = (struct data*)device->data;
  cl_int flags = mem_obj->flags;

 // if memory for this global memory is not yet allocated -> do it
  if (mem_obj->device_ptrs[device->global_mem_id].mem_ptr == NULL)
    {
      if (flags & CL_MEM_USE_HOST_PTR && mem_obj->mem_host_ptr != NULL)
        {
          b = mem_obj->mem_host_ptr;
        }
      else if (posix_memalign (&b, MAX_EXTENDED_ALIGNMENT, 
                               mem_obj->size) != 0)
        return CL_MEM_OBJECT_ALLOCATION_FAILURE;

      if (flags & CL_MEM_COPY_HOST_PTR)
        memcpy (b, mem_obj->mem_host_ptr, mem_obj->size);
    
      mem_obj->device_ptrs[device->global_mem_id].mem_ptr = b;
      mem_obj->device_ptrs[device->global_mem_id].global_mem_id = 
        device->global_mem_id;
    }
  // copy already allocated global mem info to devices own slot
  mem_obj->device_ptrs[device->dev_id] = 
    mem_obj->device_ptrs[device->global_mem_id];
  */
  return CL_SUCCESS;

}

void
pocl_zynq_basic_free (void *data, cl_mem_flags flags, void *ptr)
{
//  printf("check point in zynq free \n");
  if (flags & CL_MEM_USE_HOST_PTR)
    return;
  
  free (ptr);
}

void
//pocl_zynq_basic_read (void *data, void *host_ptr, const void *device_ptr, size_t cb)
pocl_zynq_basic_read (void *data, void *host_ptr, const void *device_ptr, size_t offset, size_t cb)
{
//  printf("from pocl_zynq_basic_read\n");
  if (host_ptr == device_ptr)
    return;

/*  buffer_type* buffer = (buffer_type*)malloc(sizeof(buffer_type));
  buffer->buffer_address = (u32*)host_ptr;
  buffer->buffer_index = 2;
  buffer->read_write = 0;
	
  write(file_desc, buffer, cb);*/
  //memcpy (host_ptr, device_ptr, cb);
}

void
//pocl_zynq_basic_write (void *data, const void *host_ptr, void *device_ptr, size_t cb)
pocl_zynq_basic_write (void *data, const void *host_ptr, void *device_ptr,
                  size_t offset, size_t cb)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
//  printf("from pocl_zynq_basic_write host_ptr = %p and  device_ptr = %p\n", host_ptr,  device_ptr);
  unsigned int i = 0;
  if (host_ptr == device_ptr)
    return;

  for (i = 0; i < device_buffer_index; i++)
    if(device_buffer[i] == device_ptr)
      break;
  if (i ==  device_buffer_index) {
    printf("Error write into buffer \n");
    return;
  }
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
//  printf("check point in zynq write size = %lu\n buffer address = %p\n", cb, host_ptr); 
/*  buffer_type* buffer = (buffer_type*)malloc(sizeof(buffer_type));
  buffer->buffer_address = (u32*)host_ptr;
  buffer->buffer_index = i;
  buffer->read_write = 1;
  write(file_desc, buffer, cb);
  */
  //memcpy (device_ptr, host_ptr, cb);
}



void
pocl_zynq_basic_run 
(void *data, 
 _cl_command_node* cmd)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
	 u32 isIdle;
	 u32 isIdle_acp;
	 u32 isIdle_hp0;
	 u32 isIdle_hp1;
	 u32 isIdle_hp2;
	 u32 isIdle_hp3;

#define IMAGE_WIDTH   1920
#define IMAGE_HEIGHT  1080

    //================================ACP===========================================================================
	argument_parameters_type *first_arg_param_acp = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	first_arg_param_acp->size = (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float);
	first_arg_param_acp->type_size = sizeof(float);
	first_arg_param_acp->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_INPUT_IMAGE_OFFSET_DATA;
	first_arg_param_acp->index = 0;
	ioctl ( flgacl_dev_acp, FPGACL_ARGUMEN_POINTER_ACP, (u32*)first_arg_param_acp);


	argument_parameters_type *second_arg_param_acp = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	second_arg_param_acp->size = (IMAGE_WIDTH)*sizeof(float);
	second_arg_param_acp->type_size = sizeof(float);
	second_arg_param_acp->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_OUTPUT_IMAGE_OFFSET_DATA;
	second_arg_param_acp->index = 1;
	ioctl ( flgacl_dev_acp, FPGACL_ARGUMEN_POINTER_ACP, (u32*)second_arg_param_acp);

	argument_parameters_type *third_arg_param_acp = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	third_arg_param_acp->size = sizeof(bool);
	third_arg_param_acp->type_size = sizeof(bool);
	third_arg_param_acp->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_FIRST_FLAG_DATA;
	third_arg_param_acp->index = 2;
	ioctl ( flgacl_dev_acp, FPGACL_ARGUMEN_DATA_ACP, (u32*)third_arg_param_acp);

	//================================HP0===========================================================================
	argument_parameters_type *first_arg_param_hp0 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	first_arg_param_hp0->size = (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float);
	first_arg_param_hp0->type_size = sizeof(float);
	first_arg_param_hp0->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_INPUT_IMAGE_OFFSET_DATA;
	first_arg_param_hp0->index = 0;
	ioctl ( flgacl_dev_hp0, FPGACL_ARGUMEN_POINTER_HP0, (u32*)first_arg_param_hp0);


	argument_parameters_type *second_arg_param_hp0 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	second_arg_param_hp0->size = (IMAGE_WIDTH)*sizeof(float);
	second_arg_param_hp0->type_size = sizeof(float);
	second_arg_param_hp0->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_OUTPUT_IMAGE_OFFSET_DATA;
	second_arg_param_hp0->index = 1;
	ioctl ( flgacl_dev_hp0, FPGACL_ARGUMEN_POINTER_HP0, (u32*)second_arg_param_hp0);

	argument_parameters_type *third_arg_param_hp0 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	third_arg_param_hp0->size = sizeof(bool);
	third_arg_param_hp0->type_size = sizeof(bool);
	third_arg_param_hp0->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_FIRST_FLAG_DATA;
	third_arg_param_hp0->index = 2;
	ioctl ( flgacl_dev_hp0, FPGACL_ARGUMEN_DATA_HP0, (u32*)third_arg_param_hp0);

	//================================HP1===========================================================================
	argument_parameters_type *first_arg_param_hp1 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	first_arg_param_hp1->size = (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float);
	first_arg_param_hp1->type_size = sizeof(float);
	first_arg_param_hp1->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_INPUT_IMAGE_OFFSET_DATA;
	first_arg_param_hp1->index = 0;
	ioctl ( flgacl_dev_hp1, FPGACL_ARGUMEN_POINTER_HP1, (u32*)first_arg_param_hp1);


	argument_parameters_type *second_arg_param_hp1 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	second_arg_param_hp1->size = (IMAGE_WIDTH)*sizeof(float);
	second_arg_param_hp1->type_size = sizeof(float);
	second_arg_param_hp1->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_OUTPUT_IMAGE_OFFSET_DATA;
	second_arg_param_hp1->index = 1;
	ioctl ( flgacl_dev_hp1, FPGACL_ARGUMEN_POINTER_HP1, (u32*)second_arg_param_hp1);

	argument_parameters_type *third_arg_param_hp1 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	third_arg_param_hp1->size = sizeof(bool);
	third_arg_param_hp1->type_size = sizeof(bool);
	third_arg_param_hp1->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_FIRST_FLAG_DATA;
	third_arg_param_hp1->index = 2;
	ioctl ( flgacl_dev_hp1, FPGACL_ARGUMEN_DATA_HP1, (u32*)third_arg_param_hp1);

	//================================HP2===========================================================================
	argument_parameters_type *first_arg_param_hp2 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	first_arg_param_hp2->size = (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float);
	first_arg_param_hp2->type_size = sizeof(float);
	first_arg_param_hp2->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_INPUT_IMAGE_OFFSET_DATA;
	first_arg_param_hp2->index = 0;
	ioctl ( flgacl_dev_hp2, FPGACL_ARGUMEN_POINTER_HP2, (u32*)first_arg_param_hp2);


	argument_parameters_type *second_arg_param_hp2 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	second_arg_param_hp2->size = (IMAGE_WIDTH)*sizeof(float);
	second_arg_param_hp2->type_size = sizeof(float);
	second_arg_param_hp2->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_OUTPUT_IMAGE_OFFSET_DATA;
	second_arg_param_hp2->index = 1;
	ioctl ( flgacl_dev_hp2, FPGACL_ARGUMEN_POINTER_HP2, (u32*)second_arg_param_hp2);

	argument_parameters_type *third_arg_param_hp2 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	third_arg_param_hp2->size = sizeof(bool);
	third_arg_param_hp2->type_size = sizeof(bool);
	third_arg_param_hp2->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_FIRST_FLAG_DATA;
	third_arg_param_hp2->index = 2;
	ioctl ( flgacl_dev_hp2, FPGACL_ARGUMEN_DATA_HP2, (u32*)third_arg_param_hp2);
	//================================HP3===========================================================================
	argument_parameters_type *first_arg_param_hp3 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	first_arg_param_hp3->size = (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float);
	first_arg_param_hp3->type_size = sizeof(float);
	first_arg_param_hp3->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_INPUT_IMAGE_OFFSET_DATA;
	first_arg_param_hp3->index = 0;
	ioctl ( flgacl_dev_hp3, FPGACL_ARGUMEN_POINTER_HP3, (u32*)first_arg_param_hp3);


	argument_parameters_type *second_arg_param_hp3 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	second_arg_param_hp3->size = (IMAGE_WIDTH)*sizeof(float);
	second_arg_param_hp3->type_size = sizeof(float);
	second_arg_param_hp3->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_OUTPUT_IMAGE_OFFSET_DATA;
	second_arg_param_hp3->index = 1;
	ioctl ( flgacl_dev_hp3, FPGACL_ARGUMEN_POINTER_HP3, (u32*)second_arg_param_hp3);

	argument_parameters_type *third_arg_param_hp3 = (argument_parameters_type*)malloc(sizeof(argument_parameters_type));
	third_arg_param_hp3->size = sizeof(bool);
	third_arg_param_hp3->type_size = sizeof(bool);
	third_arg_param_hp3->fpga_reg_offset_address =  XIMGCONV_LITE_ADDR_FIRST_FLAG_DATA;
	third_arg_param_hp3->index = 2;
	ioctl ( flgacl_dev_hp3, FPGACL_ARGUMEN_DATA_HP3, (u32*)third_arg_param_hp3);
//==============================================================================================================
//  printf("from pocl_zynq_basic_run\n");


//  printf("from pocl_zynq_basic_run check point 1\n");

	read_write_command_type *rd_command = (read_write_command_type*)malloc(sizeof(read_write_command_type));
#ifdef POWER_MONITORING
	fpgacl_read_sample_start();
#endif //POWER_MONITORING
	hardware_start = getTimestamp();

	int block_index;
	int row;
	for ( row = 0; row <= 2*MASK_HEIGHT/2+IMAGE_HEIGHT/5; row++) {
		block_index = 0;
	//=======================ACP write command=======================
		rd_command->user_data_address = (u32*)(device_memory[0] + ((block_index*IMAGE_HEIGHT/5)+row)*(IMAGE_WIDTH+MASK_WIDTH));
		rd_command->value = 0;
		rd_command->argument_index = 0;
		rd_command->read_write = 1;

		write(flgacl_dev_acp, rd_command, (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float));

		if (row == 0) {
			rd_command->value = 1;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_acp, rd_command, 1);
		} else {
			rd_command->value = 0;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_acp, rd_command, 1);
		}

		ioctl ( flgacl_dev_acp, FPGACL_START_ACP, XIMGCONV_LITE_ADDR_AP_CTRL);
	//=======================HP0 write command=======================
		block_index++;
		rd_command->user_data_address = (u32*)(device_memory[0] + ((block_index*IMAGE_HEIGHT/5)+row)*(IMAGE_WIDTH+MASK_WIDTH));
		rd_command->value = 0;
		rd_command->argument_index = 0;
		rd_command->read_write = 1;

		write(flgacl_dev_hp0, rd_command, (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float));

		if (row == 0) {
			rd_command->value = 1;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp0, rd_command, 1);
		} else {
			rd_command->value = 0;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp0, rd_command, 1);
		}
		ioctl ( flgacl_dev_hp0, FPGACL_START_HP0, XIMGCONV_LITE_ADDR_AP_CTRL);
	//=======================HP1 write command=======================
		block_index++;
		rd_command->user_data_address = (u32*)(device_memory[0] + ((block_index*IMAGE_HEIGHT/5)+row)*(IMAGE_WIDTH+MASK_WIDTH));
		rd_command->value = 0;
		rd_command->argument_index = 0;
		rd_command->read_write = 1;

		write(flgacl_dev_hp1, rd_command, (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float));

		if (row == 0) {
			rd_command->value = 1;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp1, rd_command, 1);
		} else {
			rd_command->value = 0;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp1, rd_command, 1);
		}
		ioctl ( flgacl_dev_hp1, FPGACL_START_HP1, XIMGCONV_LITE_ADDR_AP_CTRL);
	//=======================HP2 write command=======================
		block_index++;
		rd_command->user_data_address = (u32*)(device_memory[0] + ((block_index*IMAGE_HEIGHT/5)+row)*(IMAGE_WIDTH+MASK_WIDTH));
		rd_command->value = 0;
		rd_command->argument_index = 0;
		rd_command->read_write = 1;

		write(flgacl_dev_hp2, rd_command, (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float));

		if (row == 0) {
			rd_command->value = 1;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp2, rd_command, 1);
		} else {
			rd_command->value = 0;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp2, rd_command, 1);
		}
		ioctl ( flgacl_dev_hp2, FPGACL_START_HP2, XIMGCONV_LITE_ADDR_AP_CTRL);
	//=======================HP3 write command=======================
		block_index++;
		rd_command->user_data_address = (u32*)(device_memory[0] + ((block_index*IMAGE_HEIGHT/5)+row)*(IMAGE_WIDTH+MASK_WIDTH));
		rd_command->value = 0;
		rd_command->argument_index = 0;
		rd_command->read_write = 1;

		write(flgacl_dev_hp3, rd_command, (IMAGE_WIDTH+MASK_WIDTH)*sizeof(float));

		if (row == 0) {
			rd_command->value = 1;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp3, rd_command, 1);
		} else {
			rd_command->value = 0;
			rd_command->argument_index = 2;
			rd_command->read_write = 1;

			write(flgacl_dev_hp3, rd_command, 1);
		}
		ioctl ( flgacl_dev_hp3, FPGACL_START_HP3, XIMGCONV_LITE_ADDR_AP_CTRL);
		//=========================



		isIdle  =  ioctl ( flgacl_dev_acp, FPGACL_CTRL_ACP, XIMGCONV_LITE_ADDR_AP_CTRL);
		isIdle +=  ioctl ( flgacl_dev_hp0, FPGACL_CTRL_HP0, XIMGCONV_LITE_ADDR_AP_CTRL);
		isIdle +=  ioctl ( flgacl_dev_hp1, FPGACL_CTRL_HP1, XIMGCONV_LITE_ADDR_AP_CTRL);
		isIdle +=  ioctl ( flgacl_dev_hp2, FPGACL_CTRL_HP2, XIMGCONV_LITE_ADDR_AP_CTRL);
		isIdle +=  ioctl ( flgacl_dev_hp3, FPGACL_CTRL_HP3, XIMGCONV_LITE_ADDR_AP_CTRL);

		while (!isIdle) {
			isIdle  =  ioctl ( flgacl_dev_acp, FPGACL_CTRL_ACP, XIMGCONV_LITE_ADDR_AP_CTRL);
			isIdle +=  ioctl ( flgacl_dev_hp0, FPGACL_CTRL_HP0, XIMGCONV_LITE_ADDR_AP_CTRL);
			isIdle +=  ioctl ( flgacl_dev_hp1, FPGACL_CTRL_HP1, XIMGCONV_LITE_ADDR_AP_CTRL);
			isIdle +=  ioctl ( flgacl_dev_hp2, FPGACL_CTRL_HP2, XIMGCONV_LITE_ADDR_AP_CTRL);
			isIdle +=  ioctl ( flgacl_dev_hp3, FPGACL_CTRL_HP3, XIMGCONV_LITE_ADDR_AP_CTRL);
		}


		if (row >= 2*MASK_WIDTH/2 && row <= IMAGE_HEIGHT/5+2*MASK_WIDTH/2) {
			//========================================ACP=====================================================
			block_index = 0;
			rd_command->user_data_address = (u32*)(device_memory[1]+(block_index*IMAGE_HEIGHT/5)*IMAGE_WIDTH+row*IMAGE_WIDTH);
			rd_command->value = 0;
			rd_command->argument_index = 1;
			rd_command->read_write = 0;

			read(flgacl_dev_acp, rd_command, (IMAGE_WIDTH)*sizeof(float));

			//=========================================HP0====================================================
			block_index++;
			rd_command->user_data_address = (u32*)(device_memory[1]+(block_index*IMAGE_HEIGHT/5)*IMAGE_WIDTH+row*IMAGE_WIDTH);
			rd_command->value = 0;
			rd_command->argument_index = 1;
			rd_command->read_write = 0;

			read(flgacl_dev_hp0, rd_command, (IMAGE_WIDTH)*sizeof(float));

			//=========================================HP1====================================================
			block_index++;
			rd_command->user_data_address = (u32*)(device_memory[1]+(block_index*IMAGE_HEIGHT/5)*IMAGE_WIDTH+row*IMAGE_WIDTH);
			rd_command->value = 0;
			rd_command->argument_index = 1;
			rd_command->read_write = 0;

			read(flgacl_dev_hp1, rd_command, (IMAGE_WIDTH)*sizeof(float));

			//=========================================HP2====================================================
			block_index++;
			rd_command->user_data_address = (u32*)(device_memory[1]+(block_index*IMAGE_HEIGHT/5)*IMAGE_WIDTH+row*IMAGE_WIDTH);
			rd_command->value = 0;
			rd_command->argument_index = 1;
			rd_command->read_write = 0;

			read(flgacl_dev_hp2, rd_command, (IMAGE_WIDTH)*sizeof(float));

			//=========================================HP3====================================================
			block_index++;
			rd_command->user_data_address = (u32*)(device_memory[1]+(block_index*IMAGE_HEIGHT/5)*IMAGE_WIDTH+row*IMAGE_WIDTH);
			rd_command->value = 0;
			rd_command->argument_index = 1;
			rd_command->read_write = 0;

			read(flgacl_dev_hp3, rd_command, (IMAGE_WIDTH)*sizeof(float));

			//=============================================================================================
		}
	}
		hardware_end = getTimestamp();
		printf("hardware image convolution execution time from pocl device  %.6lf ms elapsed\n", (hardware_end-hardware_start)/(1000));
  /*
  struct data *d;
  const char *module_fn;
  char workgroup_string[WORKGROUP_STRING_LENGTH];
  unsigned device;
  struct pocl_argument *al;
  size_t x, y, z;
  unsigned i;
  cl_kernel kernel = cmd->command.run.kernel;
  struct pocl_context *pc = &cmd->command.run.pc;

  assert (data != NULL);
  d = (struct data *) data;

  d->current_kernel = kernel;

  // Find which device number within the context correspond
  //   to current device.  
  for (i = 0; i < kernel->context->num_devices; ++i)
    {
      if (kernel->context->devices[i]->data == data)
        {
          device = i;
          break;
        }
    }

  void *arguments[kernel->num_args + kernel->num_locals];

  // Process the kernel arguments. Convert the opaque buffer
  //   pointers to real device pointers, allocate dynamic local 
  //   memory buffers, etc. 
  for (i = 0; i < kernel->num_args; ++i)
    {
      al = &(cmd->command.run.arguments[i]);
      if (kernel->arg_info[i].is_local)
        {
          arguments[i] = malloc (sizeof (void *));
          *(void **)(arguments[i]) = pocl_zynq_basic_malloc(data, 0, al->size, NULL);
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_POINTER)
        {
          // It's legal to pass a NULL pointer to clSetKernelArguments. In 
          //   that case we must pass the same NULL forward to the kernel.
          //   Otherwise, the user must have created a buffer with per device
          //   pointers stored in the cl_mem. 
          if (al->value == NULL)
            {
              arguments[i] = malloc (sizeof (void *));
              *(void **)arguments[i] = NULL;
            }
          else
            arguments[i] = &((*(cl_mem *) (al->value))->device_ptrs[device].mem_ptr);
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_IMAGE)
        {
          dev_image_t di;
          fill_dev_image_t (&di, al, device);

          void* devptr = pocl_zynq_basic_malloc (data, 0, sizeof(dev_image_t), NULL);
          arguments[i] = malloc (sizeof (void *));
          *(void **)(arguments[i]) = devptr; 
          pocl_zynq_basic_write (data, &di, devptr, sizeof(dev_image_t));
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_SAMPLER)
        {
          dev_sampler_t ds;
          
          arguments[i] = malloc (sizeof (void *));
          *(void **)(arguments[i]) = pocl_zynq_basic_malloc 
            (data, 0, sizeof(dev_sampler_t), NULL);
          pocl_zynq_basic_write (data, &ds, *(void**)arguments[i], sizeof(dev_sampler_t));
        }
      else
        {
          arguments[i] = al->value;
        }
    }
  for (i = kernel->num_args;
       i < kernel->num_args + kernel->num_locals;
       ++i)
    {
      al = &(cmd->command.run.arguments[i]);
      arguments[i] = malloc (sizeof (void *));
      *(void **)(arguments[i]) = pocl_zynq_basic_malloc (data, 0, al->size, NULL);
    }

  for (z = 0; z < pc->num_groups[2]; ++z)
    {
      for (y = 0; y < pc->num_groups[1]; ++y)
        {
          for (x = 0; x < pc->num_groups[0]; ++x)
            {
              pc->group_id[0] = x;
              pc->group_id[1] = y;
              pc->group_id[2] = z;

              cmd->command.run.wg (arguments, pc);

            }
        }
    }
  for (i = 0; i < kernel->num_args; ++i)
    {
      if (kernel->arg_info[i].is_local)
        {
          pocl_zynq_basic_free (data, 0, *(void **)(arguments[i]));
          free (arguments[i]);
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_IMAGE)
        {
          pocl_zynq_basic_free (data, 0, *(void **)(arguments[i]));
          free (arguments[i]);            
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_SAMPLER || 
               (kernel->arg_info[i].type == POCL_ARG_TYPE_POINTER && *(void**)arguments[i] == NULL))
        {
          free (arguments[i]);
        }
    }
  for (i = kernel->num_args;
       i < kernel->num_args + kernel->num_locals;
       ++i)
    {
      pocl_zynq_basic_free(data, 0, *(void **)(arguments[i]));
      free (arguments[i]);
    }
	*/
}

void
pocl_zynq_basic_run_native 
(void *data, 
 _cl_command_node* cmd)
{
  cmd->command.native.user_func(cmd->command.native.args);
}

void
//pocl_zynq_basic_copy (void *data, const void *src_ptr, void *__restrict__ dst_ptr, size_t cb)
pocl_zynq_basic_copy (void *data, const void *src_ptr, size_t src_offset,
		void *__restrict__ dst_ptr, size_t dst_offset, size_t cb)

{
  if (src_ptr == dst_ptr)
    return;
  
  memcpy (dst_ptr, src_ptr, cb);
}

void
pocl_zynq_basic_copy_rect (void *data,
                      const void *__restrict const src_ptr,
                      void *__restrict__ const dst_ptr,
                      const size_t *__restrict__ const src_origin,
                      const size_t *__restrict__ const dst_origin, 
                      const size_t *__restrict__ const region,
                      size_t const src_row_pitch,
                      size_t const src_slice_pitch,
                      size_t const dst_row_pitch,
                      size_t const dst_slice_pitch)
{
  char const *__restrict const adjusted_src_ptr = 
    (char const*)src_ptr +
    src_origin[0] + src_row_pitch * src_origin[1] + src_slice_pitch * src_origin[2];
  char *__restrict__ const adjusted_dst_ptr = 
    (char*)dst_ptr +
    dst_origin[0] + dst_row_pitch * dst_origin[1] + dst_slice_pitch * dst_origin[2];
  
  size_t j, k;

  /* TODO: handle overlaping regions */
  
  for (k = 0; k < region[2]; ++k)
    for (j = 0; j < region[1]; ++j)
      memcpy (adjusted_dst_ptr + dst_row_pitch * j + dst_slice_pitch * k,
              adjusted_src_ptr + src_row_pitch * j + src_slice_pitch * k,
              region[0]);
}

void
pocl_zynq_basic_write_rect (void *data,
                       const void *__restrict__ const host_ptr,
                       void *__restrict__ const device_ptr,
                       const size_t *__restrict__ const buffer_origin,
                       const size_t *__restrict__ const host_origin, 
                       const size_t *__restrict__ const region,
                       size_t const buffer_row_pitch,
                       size_t const buffer_slice_pitch,
                       size_t const host_row_pitch,
                       size_t const host_slice_pitch)
{
  char *__restrict const adjusted_device_ptr = 
    (char*)device_ptr +
    buffer_origin[0] + buffer_row_pitch * buffer_origin[1] + buffer_slice_pitch * buffer_origin[2];
  char const *__restrict__ const adjusted_host_ptr = 
    (char const*)host_ptr +
    host_origin[0] + host_row_pitch * host_origin[1] + host_slice_pitch * host_origin[2];
  
  size_t j, k;

  /* TODO: handle overlaping regions */
  
  for (k = 0; k < region[2]; ++k)
    for (j = 0; j < region[1]; ++j)
      memcpy (adjusted_device_ptr + buffer_row_pitch * j + buffer_slice_pitch * k,
              adjusted_host_ptr + host_row_pitch * j + host_slice_pitch * k,
              region[0]);
}

void
pocl_zynq_basic_read_rect (void *data,
                      void *__restrict__ const host_ptr,
                      void *__restrict__ const device_ptr,
                      const size_t *__restrict__ const buffer_origin,
                      const size_t *__restrict__ const host_origin, 
                      const size_t *__restrict__ const region,
                      size_t const buffer_row_pitch,
                      size_t const buffer_slice_pitch,
                      size_t const host_row_pitch,
                      size_t const host_slice_pitch)
{
  char const *__restrict const adjusted_device_ptr = 
    (char const*)device_ptr +
    buffer_origin[0] + buffer_row_pitch * (buffer_origin[1] + buffer_slice_pitch * buffer_origin[2]);
  char *__restrict__ const adjusted_host_ptr = 
    (char*)host_ptr +
    host_origin[0] + host_row_pitch * (host_origin[1] + host_slice_pitch * host_origin[2]);
  
  size_t j, k;
  
  /* TODO: handle overlaping regions */
  
  for (k = 0; k < region[2]; ++k)
    for (j = 0; j < region[1]; ++j)
      memcpy (adjusted_host_ptr + host_row_pitch * j + host_slice_pitch * k,
              adjusted_device_ptr + buffer_row_pitch * j + buffer_slice_pitch * k,
              region[0]);
}

/* origin and region must be in original shape unlike in copy/read/write_rect()
 */
void
pocl_zynq_basic_fill_rect (void *data,
                      void *__restrict__ const device_ptr,
                      const size_t *__restrict__ const buffer_origin,
                      const size_t *__restrict__ const region,
                      size_t const buffer_row_pitch,
                      size_t const buffer_slice_pitch,
                      void *fill_pixel,
                      size_t pixel_size)                    
{
  char *__restrict const adjusted_device_ptr = (char*)device_ptr 
    + buffer_origin[0] * pixel_size 
    + buffer_row_pitch * buffer_origin[1] 
    + buffer_slice_pitch * buffer_origin[2];
    
  size_t i, j, k;

  for (k = 0; k < region[2]; ++k)
    for (j = 0; j < region[1]; ++j)
      for (i = 0; i < region[0]; ++i)
        memcpy (adjusted_device_ptr + pixel_size * i 
                + buffer_row_pitch * j 
                + buffer_slice_pitch * k, fill_pixel, pixel_size);
}

void *
pocl_zynq_basic_map_mem (void *data, void *buf_ptr, 
                      size_t offset, size_t size,
                      void *host_ptr) 
{
  /* All global pointers of the pthread/CPU device are in 
     the host address space already, and up to date. */
  if (host_ptr != NULL) return host_ptr;
  return buf_ptr + offset;
}

void
pocl_zynq_basic_uninit (cl_device_id device)
{
#ifdef POWER_MONITORING
	fpgacl_read_sample_start();
#endif //POWER_MONITORING
  struct data *d = (struct data*)device->data;
  free (d);
  device->data = NULL;
}

cl_ulong
pocl_zynq_basic_get_timer_value (void *data) 
{
  struct timeval current;
  gettimeofday(&current, NULL);  
  return (current.tv_sec * 1000000 + current.tv_usec)*1000;
}

cl_int 
pocl_zynq_basic_get_supported_image_formats (cl_mem_flags flags,
                                        const cl_image_format **image_formats,
                                        cl_int *num_img_formats)
{
    if (num_img_formats == NULL || image_formats == NULL)
      return CL_INVALID_VALUE;
  
    *num_img_formats = sizeof(zynq_basic_supported_image_formats)/sizeof(cl_image_format);
    *image_formats = zynq_basic_supported_image_formats;
    
    return CL_SUCCESS; 
}

typedef struct compiler_cache_item compiler_cache_item;
struct compiler_cache_item
{
  char *tmp_dir;
  char *function_name;
  pocl_workgroup wg;
  compiler_cache_item *next;
};

static compiler_cache_item *compiler_cache;
static pocl_lock_t compiler_cache_lock;

void check_compiler_cache_zynq_basic (_cl_command_node *cmd)
{
  char workgroup_string[WORKGROUP_STRING_LENGTH];
  lt_dlhandle dlhandle;
  compiler_cache_item *ci = NULL;
  
  if (compiler_cache == NULL)
    POCL_INIT_LOCK (compiler_cache_lock);

  POCL_LOCK (compiler_cache_lock);
  LL_FOREACH (compiler_cache, ci)
    {
      if (strcmp (ci->tmp_dir, cmd->command.run.tmp_dir) == 0 &&
          strcmp (ci->function_name, 
                  cmd->command.run.kernel->function_name) == 0)
        {
          POCL_UNLOCK (compiler_cache_lock);
          cmd->command.run.wg = ci->wg;
          return;
        }
    }
  ci = malloc (sizeof (compiler_cache_item));
  ci->next = NULL;
  ci->tmp_dir = strdup(cmd->command.run.tmp_dir);
  ci->function_name = strdup (cmd->command.run.kernel->function_name);
  const char* module_fn = llvm_codegen (cmd->command.run.tmp_dir,
                                        cmd->command.run.kernel,
                                        cmd->device);
  dlhandle = lt_dlopen (module_fn);     
  if (dlhandle == NULL)
    {
      printf ("pocl error: lt_dlopen(\"%s\") failed with '%s'.\n", 
              module_fn, lt_dlerror());
      printf ("note: missing symbols in the kernel binary might be" 
              "reported as 'file not found' errors.\n");
      abort();
    }
  snprintf (workgroup_string, WORKGROUP_STRING_LENGTH,
            "_%s_workgroup", cmd->command.run.kernel->function_name);
  cmd->command.run.wg = ci->wg = 
    (pocl_workgroup) lt_dlsym (dlhandle, workgroup_string);

  LL_APPEND (compiler_cache, ci);
  POCL_UNLOCK (compiler_cache_lock);

}

void
pocl_zynq_basic_compile_submitted_kernels (_cl_command_node *cmd)
{
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
  if (cmd->type == CL_COMMAND_NDRANGE_KERNEL)
    check_compiler_cache_zynq_basic (cmd);
#ifdef __MOHAMMAD_DEBUG__
	  printf("Mohammad Comment from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
#endif// __MOHAMMAD_DEBUG__
}

double getTimestamp()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}
