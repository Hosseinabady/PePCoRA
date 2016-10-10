/*
 * fpga_device_driver.h
 *
 *  Created on: 10 Nov 2014
 *      Author: csxmh
 */

#ifndef FPGACL_DEVICE_DRIVER_H_
#define FPGACL_DEVICE_DRIVER_H_


#define FPGACL_DEVICE_BASE_ADDRESS_ACP                  0x43C00000
#define FPGACL_DEVICE_BASE_ADDRESS_HP0                  0x43C10000
#define FPGACL_DEVICE_BASE_ADDRESS_HP1                  0x43C20000
#define FPGACL_DEVICE_BASE_ADDRESS_HP2                  0x43C30000
#define FPGACL_DEVICE_BASE_ADDRESS_HP3                  0x43C40000

#define FPGACL_DEVICE_NO_PORT                              1000


#define FPGACL_IOC_MAGIC_ACP                              1000
#define FPGACL_ARGUMEN_POINTER_ACP                        _IOW(FPGACL_IOC_MAGIC_ACP, 0, int)
#define FPGACL_ARGUMEN_DATA_ACP                           _IOW(FPGACL_IOC_MAGIC_ACP, 1, int)
#define FPGACL_START_ACP                                  _IOW(FPGACL_IOC_MAGIC_ACP, 2, int)
#define FPGACL_CTRL_ACP                                   _IOW(FPGACL_IOC_MAGIC_ACP, 3, int)


#define FPGACL_IOC_MAGIC_HP0                              2000
#define FPGACL_ARGUMEN_POINTER_HP0                        _IOW(FPGACL_IOC_MAGIC_HP0, 0, int)
#define FPGACL_ARGUMEN_DATA_HP0                           _IOW(FPGACL_IOC_MAGIC_HP0, 1, int)
#define FPGACL_START_HP0                                  _IOW(FPGACL_IOC_MAGIC_HP0, 2, int)
#define FPGACL_CTRL_HP0                                   _IOW(FPGACL_IOC_MAGIC_HP0, 3, int)

#define FPGACL_IOC_MAGIC_HP1                              2100
#define FPGACL_ARGUMEN_POINTER_HP1                        _IOW(FPGACL_IOC_MAGIC_HP1, 0, int)
#define FPGACL_ARGUMEN_DATA_HP1                           _IOW(FPGACL_IOC_MAGIC_HP1, 1, int)
#define FPGACL_START_HP1                                  _IOW(FPGACL_IOC_MAGIC_HP1, 2, int)
#define FPGACL_CTRL_HP1                                   _IOW(FPGACL_IOC_MAGIC_HP1, 3, int)

#define FPGACL_IOC_MAGIC_HP2                              2200
#define FPGACL_ARGUMEN_POINTER_HP2                        _IOW(FPGACL_IOC_MAGIC_HP2, 0, int)
#define FPGACL_ARGUMEN_DATA_HP2                           _IOW(FPGACL_IOC_MAGIC_HP2, 1, int)
#define FPGACL_START_HP2                                  _IOW(FPGACL_IOC_MAGIC_HP2, 2, int)
#define FPGACL_CTRL_HP2                                   _IOW(FPGACL_IOC_MAGIC_HP2, 3, int)

#define FPGACL_IOC_MAGIC_HP3                              2300
#define FPGACL_ARGUMEN_POINTER_HP3                        _IOW(FPGACL_IOC_MAGIC_HP3, 0, int)
#define FPGACL_ARGUMEN_DATA_HP3                           _IOW(FPGACL_IOC_MAGIC_HP3, 1, int)
#define FPGACL_START_HP3                                  _IOW(FPGACL_IOC_MAGIC_HP3, 2, int)
#define FPGACL_CTRL_HP3                                   _IOW(FPGACL_IOC_MAGIC_HP3, 3, int)






struct argument_parameters_struct {
	unsigned int  size;
	unsigned int  type_size;
	unsigned int  fpga_reg_offset_address;
	unsigned int  index;
};
typedef struct argument_parameters_struct argument_parameters_type;





#endif /* FPGACL_DEVICE_DRIVER_H_ */
