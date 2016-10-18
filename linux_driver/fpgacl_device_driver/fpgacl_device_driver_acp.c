/*
author: Mohamamd Hosseinabady
email:  mohammad@hosseinabady.com, m.hosseinabady@bristol.ac.uk
*/


#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/page.h>
#include <linux/dma-mapping.h>
#include "fpgacl_device_driver.h"

MODULE_LICENSE("Dual BSD/GPL");




static long unsigned  int  enpower_opencl_device_address_base_acp = FPGACL_DEVICE_BASE_ADDRESS_ACP;
static void               *kernel_enpower_opencl_device_address_base_acp;
 

dev_t        dev_numbers_acp;
struct cdev* enpower_opencl_mohammad_cdev_acp;
static int   enpower_opencl_mohammad_device_open_acp = 0;


//==========================================================
// struct which defines the linux kernel buffer infromation
// for each argument of the FPGA design
//==========================================================
struct fpga_buffer_struct {
	void*        linux_kernel_memory;
	u32          index;
	u32          size;
	u32          phys_add;
//	u32          type;                         //0 for pointer otherwise non-zero
};
typedef struct fpga_buffer_struct fpga_buffer_type;

//==========================================================
// struct which keeps the fpga design argument infromation
//==========================================================
struct fpga_argument_struct {
	void*            kernel_mapped_fpga_address;
	u32              fpga_address_offset;
	u32              type;                         //0 for pointer otherwise non-zero
	fpga_buffer_type fpga_buffer;

};
typedef struct fpga_argument_struct fpga_argument_type;

struct kernel_argument_list_struct{
	struct kernel_argument_list_struct *previous;
	struct kernel_argument_list_struct *next;
	fpga_argument_type fpga_arguments_acp;
};

typedef struct kernel_argument_list_struct kernel_argument_list_type;


static kernel_argument_list_type *fpga_arguments_list_acp;


//==========================================================

//==========================================================
// struct for different tasks in write function
//==========================================================
struct read_write_command_struct {
    void*    user_data_address;
    u32     value;
    u32     argument_index;
    u32     read_write;
};
typedef struct read_write_command_struct read_write_command_type;

//==========================================================


static int fpga_buffer_allocation_acp(fpga_buffer_type* buffer, u32 size);

static ssize_t  enpower_opencl_mohammad_read_acp(struct file * file,	// see include/linux/fs.h
                                                 char __user * buffer,	// buffer to be filled with data 
                                                 size_t        length,	        // length of the buffer
                                                 loff_t *      offset) {
	unsigned long status;
	read_write_command_type* read_write_command = (read_write_command_type *)buffer;
	u32 arg_index = read_write_command->argument_index;
	int i = 0;
	kernel_argument_list_type *arg;

	if (read_write_command->read_write == 0) {
		arg= fpga_arguments_list_acp;
		for (i = 0; i < arg_index; i++) {
			arg = arg->next;
		}
		status = copy_to_user(read_write_command->user_data_address, (arg->fpga_arguments_acp.fpga_buffer.linux_kernel_memory), length);

		if(status){
			printk(KERN_ALERT "ACP: lengh to be read = %d, remain length = %lu \n", length, status);
			printk(KERN_ERR "Error: Reading bitstream from userspace failed ! \n\r");
		}
	} else {
		printk(KERN_ALERT "This is read function not write function ! \n\r");
	}

    return 0;
}



static ssize_t enpower_opencl_mohammad_write_acp(	struct file *file,
												const char __user * buffer, 
												size_t length, 
												loff_t * offset) {
	unsigned long status;
	read_write_command_type* read_write_command = (read_write_command_type *)buffer;
	u32 arg_index = read_write_command->argument_index;

	int i = 0;
	kernel_argument_list_type *arg;


	if (read_write_command->read_write == 1) {
//		printk(KERN_ALERT "arg_index for write = %d \n", arg_index);
    	if (length != 0) { // write user data to buffer
    		arg= fpga_arguments_list_acp;
    		for (i = 0; i < arg_index; i++) {
    			arg = arg->next;
    		}
    		if ( arg->fpga_arguments_acp.type == 0) {
    			status = copy_from_user((arg->fpga_arguments_acp.fpga_buffer.linux_kernel_memory), read_write_command->user_data_address, length);
    			if(status){
    				printk(KERN_ALERT "acp lengh to be write = %d, remain length = %lu arg_index=%d\n", length, status,arg_index);
    				printk(KERN_ERR "acp Error:Writing data from userspace failed ! \n\r");
    			}
    		} else if ( arg->fpga_arguments_acp.type == 1) {
    			//printk(KERN_ALERT "data arg_index for write = %d value = %d\n", arg_index, read_write_command->value);
    			iowrite32((unsigned long int)(read_write_command->value), arg->fpga_arguments_acp.kernel_mapped_fpga_address);
    		} else {
    			printk(KERN_ERR "Error:This is not writing data from userspace  ! \n\r");
    		}
    	} else {
    		printk(KERN_ERR "Error:This is not writing data from userspace  ! \n\r");
    	}
    } else {
		printk(KERN_ALERT "This is write function not read function ! \n\r");
	}
	

    return 0;
}



long  enpower_opencl_mohammad_ioctl_acp(struct        file *file,	// ditto
		 	                            unsigned int  ioctl_num,	// number and param for ioctl
		                                unsigned long ioctl_param) {  //ioctl_param is INLS

	u32 data_cntrl = 0;
	u32 return_value = 0;
	int state;

	argument_parameters_type *arg_param = (argument_parameters_type*)ioctl_param;
	unsigned int buffer_size;
	unsigned int fpga_reg_offset_address;
	unsigned int index;
	unsigned int type_size;

	kernel_argument_list_type *arg;
	kernel_argument_list_type *tail;


    switch(ioctl_num) {
    	case FPGACL_ARGUMEN_POINTER_ACP:
    		state = copy_from_user((unsigned int*)&index, (unsigned int*)&(arg_param->index), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for index argument parameter failed \n");
    			return -ENODEV;
    		}
//    		printk(KERN_ALERT "for test: index = %d \n", index);

    		state = copy_from_user((unsigned int*)&buffer_size, (unsigned int*)&(arg_param->size), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for buffer size argument parameter failed \n");
    			return -ENODEV;
    		}
//    		printk(KERN_ALERT "for test: buffer size = %d \n", buffer_size);
    		state = copy_from_user((unsigned int*)&fpga_reg_offset_address, (unsigned int*)&(arg_param->fpga_reg_offset_address), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for offset address argument parameter failed \n");
    			return -ENODEV;
    		}
//    		printk(KERN_ALERT "for test: buffer address = %d \n", fpga_reg_offset_address);

    		state = copy_from_user((unsigned int*)&type_size, (unsigned int*)&(arg_param->type_size), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for offset address argument parameter failed \n");
    			return -ENODEV;
    		}
#ifdef __MOHAMMAD_DEBUG__
    		printk(KERN_ALERT "for test: type_size = %d \n", type_size);
#endif //__MOHAMMAD_DEBUG__
//    		printk(KERN_ALERT "Check point : 1_0  ! \n\r");

    		//allocate an argument in the kernel
    		arg = kmalloc(sizeof(kernel_argument_list_type), GFP_KERNEL);
    		if (fpga_arguments_list_acp == NULL) {
    			fpga_arguments_list_acp = arg;
    			arg->next=NULL;
    			arg->previous = NULL;
    		} else {
    			tail = fpga_arguments_list_acp;
    			while (tail->next != NULL) {
    				tail=tail->next;
    			}
    			tail->next = arg;
    			arg->previous = tail;
    			arg->next = NULL;
    		}

//    		printk(KERN_ALERT "Check point : 1  ! \n\r");
    		arg->fpga_arguments_acp.fpga_address_offset             = fpga_reg_offset_address;
    		arg->fpga_arguments_acp.type                            = 0;                         //the argument is a pointer
    		arg->fpga_arguments_acp.fpga_buffer.linux_kernel_memory = NULL;                      //memory in the kernel not allocated yet
    		arg->fpga_arguments_acp.fpga_buffer.size                = buffer_size;               //size of the memory buffer
    		arg->fpga_arguments_acp.fpga_buffer.index               = index;                     //index of the argument


//    		printk(KERN_ALERT "Check point : 2  ! \n\r");
    		//allocation buffer in the kernel
    		state = fpga_buffer_allocation_acp(&arg->fpga_arguments_acp.fpga_buffer, arg->fpga_arguments_acp.fpga_buffer.size);
    		if (state != 0) {
    			printk(KERN_ALERT "kernel memory allocation failed for argument %d\n", index);
    		    return -ENODEV;
    		}

//    		printk(KERN_ALERT "Check point : 3  ! \n\r");
    		//get the kernel address corresponding to the address offset register of the argument in the FPGA
    		arg->fpga_arguments_acp.kernel_mapped_fpga_address = kernel_enpower_opencl_device_address_base_acp + arg->fpga_arguments_acp.fpga_address_offset;
 //   		printk(KERN_ALERT "Check point : 4  ! \n\r");


    		iowrite32((unsigned long int)(arg->fpga_arguments_acp.fpga_buffer.phys_add/type_size), arg->fpga_arguments_acp.kernel_mapped_fpga_address);
//    		printk(KERN_ALERT "Check point : 5  ! \n\r");

    		break;
       	case FPGACL_ARGUMEN_DATA_ACP:
    		state = copy_from_user((unsigned int*)&index, (unsigned int*)&(arg_param->index), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for index argument parameter failed \n");
    			return -ENODEV;
    		}
//    		printk(KERN_ALERT "for test: index = %d \n", index);

    		state = copy_from_user((unsigned int*)&buffer_size, (unsigned int*)&(arg_param->size), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for buffer size argument parameter failed \n");
    			return -ENODEV;
    		}
//    		printk(KERN_ALERT "for test: buffer size = %d \n", buffer_size);
    		state = copy_from_user((unsigned int*)&fpga_reg_offset_address, (unsigned int*)&(arg_param->fpga_reg_offset_address), sizeof(unsigned int));
    		if (state != 0) {
    			printk(KERN_ALERT "copy from user space for offset address argument parameter failed \n");
    			return -ENODEV;
    		}

       		arg = kmalloc(sizeof(kernel_argument_list_type), GFP_KERNEL);
       		if (fpga_arguments_list_acp == NULL) {
       			fpga_arguments_list_acp = arg;
       			arg->next=NULL;
       			arg->previous = NULL;
       		} else {
       			tail = fpga_arguments_list_acp;
       			while (tail->next != NULL) {
       				tail=tail->next;
       			}
       			tail->next = arg;
       			arg->previous = tail;
       			arg->next = NULL;
       		}
       		arg->fpga_arguments_acp.fpga_address_offset             = fpga_reg_offset_address;
       		arg->fpga_arguments_acp.type                            = 1;
       		arg->fpga_arguments_acp.fpga_buffer.linux_kernel_memory = NULL;
       		arg->fpga_arguments_acp.fpga_buffer.size                = 0;
       		arg->fpga_arguments_acp.fpga_buffer.index               = index;

       		arg->fpga_arguments_acp.kernel_mapped_fpga_address = kernel_enpower_opencl_device_address_base_acp + arg->fpga_arguments_acp.fpga_address_offset;


        	break;
        case FPGACL_START_ACP:
            data_cntrl = ioread32(kernel_enpower_opencl_device_address_base_acp+ioctl_param) & 0x80;
            iowrite32(data_cntrl | 0x01, kernel_enpower_opencl_device_address_base_acp+ioctl_param);
            break;
            
        case  FPGACL_CTRL_ACP:
			data_cntrl = ioread32(kernel_enpower_opencl_device_address_base_acp+ioctl_param);
	        return_value = (data_cntrl >> 1)&0x1;
			break;

	default:
	
		break;

    }
    
	return return_value;

}


static int enpower_opencl_mohammad_open_acp(struct inode *inode, struct file *file) {
    
    printk(KERN_ALERT "enpower_opencl_mohammad device (%p, %p) opened \n", inode, file);
    
    if(enpower_opencl_mohammad_device_open_acp)
        return -EBUSY;
     
    enpower_opencl_mohammad_device_open_acp++;
    fpga_arguments_list_acp = NULL;

    return 0;
}



static int enpower_opencl_mohammad_release_acp(struct inode *inode, struct file *file){

	kernel_argument_list_type *arg = fpga_arguments_list_acp;
	kernel_argument_list_type *arg_next;
    while (arg != NULL) {
    	if (arg->fpga_arguments_acp.type == 0) {
#ifdef __MOHAMMAD_DEBUG__
    		printk(KERN_ALERT "memory free for argument index = %d, size = %d, linux_kernel_memory= 0x%lx,  phys_add = 0x%lx\n", arg->fpga_arguments_acp.fpga_buffer.index, arg->fpga_arguments_acp.fpga_buffer.size , arg->fpga_arguments_acp.fpga_buffer.linux_kernel_memory, arg->fpga_arguments_acp.fpga_buffer.phys_add);
#endif// __MOHAMMAD_DEBUG__
    		dma_free_coherent (NULL, arg->fpga_arguments_acp.fpga_buffer.size , arg->fpga_arguments_acp.fpga_buffer.linux_kernel_memory, arg->fpga_arguments_acp.fpga_buffer.phys_add);
    	}

    	arg_next = arg->next;
    	kfree(arg);
    	arg=arg_next;
    }
    fpga_arguments_list_acp = NULL;
#ifdef __MOHAMMAD_DEBUG__
    printk(KERN_ALERT "enpower_opencl_mohammad device (%p, %p) closed \n", inode, file);
#endif// __MOHAMMAD_DEBUG__
    enpower_opencl_mohammad_device_open_acp--;


    return 0;
}


struct file_operations enpower_opencl_mohammad_fops_acp = {
	.read = enpower_opencl_mohammad_read_acp,
	.write = enpower_opencl_mohammad_write_acp,
	.unlocked_ioctl = enpower_opencl_mohammad_ioctl_acp,
	.open = enpower_opencl_mohammad_open_acp,
	.release = enpower_opencl_mohammad_release_acp,
};

static int enpower_opencl_mohammad_init_acp(void) {

    int   state; 

    char  device_name[]="fpgacl_acp";

    state = alloc_chrdev_region(&dev_numbers_acp, 0, 1, device_name);
    if(state!=0) {
        printk(KERN_ALERT "failed to register a region dynamically\n");
    } else {
        printk(KERN_ALERT "major number = %d\n", MAJOR(dev_numbers_acp));
    }

    enpower_opencl_mohammad_cdev_acp        = cdev_alloc();
    enpower_opencl_mohammad_cdev_acp->ops   = &enpower_opencl_mohammad_fops_acp;
    enpower_opencl_mohammad_cdev_acp->owner = THIS_MODULE;

    state = cdev_add(enpower_opencl_mohammad_cdev_acp, dev_numbers_acp , 1);
    if(state < 0) {
        printk(KERN_ALERT "device failed to be added\n");
    }

   

    kernel_enpower_opencl_device_address_base_acp = ioremap(enpower_opencl_device_address_base_acp, FPGACL_DEVICE_NO_PORT);
    if (!kernel_enpower_opencl_device_address_base_acp) {
        printk(KERN_ALERT "kernel remap my_gpio failed 0x%lx\n", enpower_opencl_device_address_base_acp);
        return -ENODEV;
    }


    return 0;
}


static int fpga_buffer_allocation_acp(fpga_buffer_type *buffer, u32 size) {


	buffer->size = size;
	buffer->linux_kernel_memory = dma_alloc_coherent( NULL, size, &buffer->phys_add, GFP_KERNEL );
	if (!buffer->linux_kernel_memory) {
		printk(KERN_ALERT "kernel memory allocation failed for argument \n" );
		return -ENODEV;
	} else {
#ifdef __MOHAMMAD_DEBUG__
		printk(KERN_ALERT "%d kernel memory allocation success for argument %d at address 0x%lx\n", size, buffer->index, buffer->linux_kernel_memory);
#endif // __MOHAMMAD_DEBUG__
	}

	return 0;
}



static void enpower_opencl_mohammad_exit_acp(void) {

    cdev_del(enpower_opencl_mohammad_cdev_acp);
    unregister_chrdev_region(dev_numbers_acp, 1);

    printk(KERN_ALERT "By fpga device driver!\n");
	

}
module_init(enpower_opencl_mohammad_init_acp);
module_exit(enpower_opencl_mohammad_exit_acp);
