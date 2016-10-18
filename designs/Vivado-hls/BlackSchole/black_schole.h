#ifndef __BLACK_SCHOLE__
#define __BLACK_SCHOLE__

typedef unsigned long u32;
#define OPT_N             4000000
#define GROUP_SIZE        (2048)
#define BLOCK_SIZE         32


void black_schole_level3(volatile float* memory, volatile u32 call_offset, volatile u32 put_offset, volatile u32 s_offset, volatile u32 x_offset, volatile u32 t_offset, volatile float r, volatile float v);
#endif //__BLACK_SCHOLE__
