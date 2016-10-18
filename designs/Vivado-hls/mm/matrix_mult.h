#ifndef __MATRIX_MULT_H__
#define __MATRIX_MULT_H__

#define DIM  2000

#define A_WIDTH    DIM//1920
#define A_HEIGHT   DIM//1088

#define B_WIDTH    DIM//1088
#define B_HEIGHT   DIM//1920

#define GROUP_SIZE  16
typedef unsigned long u32;

#define WORK_ITEM_SIZE  4

void matrix_mult(volatile float* memory, u32 a_offset, u32 b_offset, u32 c_offset, u32 newGroupFlag);

#endif //__MATRIX_MULT_H__
