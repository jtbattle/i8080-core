// Intel 8080 (KR580VM80A) microprocessor core model
//
// Copyright (C) 2012 Alexander Demin <alexander@demin.ws>
// Changes 2018/11/23, Jim Battle (frustum@pobox.com) (see i8080.c)
//
// Credits
//
// Viacheslav Slavinsky, Vector-06C FPGA Replica
// http://code.google.com/p/vector06cc/
//
// Dmitry Tselikov, Bashrikia-2M and Radio-86RK on Altera DE1
// http://bashkiria-2m.narod.ru/fpga.html
//
// Ian Bartholomew, 8080/8085 CPU Exerciser
// http://www.idb.me.uk/sunhillow/8080.html
//
// Frank Cringle, The original exerciser for the Z80.
//
// Thanks to zx.pk.ru and nedopc.org/forum communities.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef I8080_H
#define I8080_H

/* enable this define if the host CPU is little endian (eg, intel CPUs) */
#define LITTLE_ENDIAN

#ifdef __cplusplus
extern "C" {
#endif

// TODO: just use <stdint.h> ?
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned long int uint32_t;

typedef int  rd_handler(int addr);
typedef void wr_handler(int addr, int byte);
typedef int  in_handler(int addr);
typedef void out_handler(int addr, int byte);

typedef union {
    struct {
#ifdef LITTLE_ENDIAN
        uint8_t l, h;   /* intel style */
#else
        uint8_t h, l;   /* motorola style */
#endif
    } b;
    uint16_t w;
} reg_pair;

// bits 1,3,5 are unused and not represented
typedef struct {
    uint8_t carry_flag;
    uint8_t parity_flag;
    uint8_t half_carry_flag;
    uint8_t zero_flag;
    uint8_t sign_flag;
} flag_reg;

typedef struct {
    /* processor state */
    reg_pair sp, pc;
    reg_pair af, bc, de, hl;
    flag_reg f;
    uint8_t inte;              /* 1=interrupt enable */
    uint8_t halt;              /* 0=running, 1=halted */
    /* external state handlers */
    rd_handler  *rd_func;
    wr_handler  *wr_func;
    in_handler  *in_func;
    out_handler *out_func;
    void        *user;
} i8080;

/* create a new cpu instance
 * 'user' is a pointer that is returned to the handler at callback time;
 * pass a null pointer if your app doesn't need it
 */
extern i8080 *i8080_new(rd_handler  *rd_func,
                        wr_handler  *wr_func,
                        in_handler  *in_func,
                        out_handler *out_func,
                        void        *user
                       );

/* destroy a cpu instance */
extern void i8080_destroy(i8080 *cpu);

/* return user data pointer */
inline void *i8080_get_user(i8080 *cpu) { return cpu->user; }

extern void i8080_reset(i8080 *cpu);

/* execute one instruction and return the number of elapsed clock ticks */
extern int i8080_exec_one_op(i8080 *cpu);

/* non-maskable interrupt request */
extern void i8080_interrupt(i8080 *cpu, uint8_t op);

/* expose registers for test harness */
inline void i8080_jump(i8080 *cpu, int addr) { cpu->pc.w = addr; }
inline int i8080_pc(i8080 *cpu)      { return cpu->pc.w; }
inline int i8080_regs_de(i8080 *cpu) { return cpu->de.w; }
inline int i8080_regs_c(i8080 *cpu)  { return cpu->bc.b.l; }
inline int i8080_regs_e(i8080 *cpu)  { return cpu->de.b.l; }

#ifdef __cplusplus
}
#endif

#endif /* I8080_H */
