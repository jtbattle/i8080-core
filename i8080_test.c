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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i8080.h"

uint8_t mem[65536];

int rd_func(int addr)
{
    return mem[addr & 0xffff];
}

void wr_func(int addr, int byte)
{
    mem[addr & 0xffff] = byte;
}

int in_func(int addr)
{
    return 0x00;
}

void out_func(int addr, int byte)
{
    /* noop */
}


void load_file(const char* name, uint8_t* load_to)
{
    FILE* f = fopen(name, "r+b");
    int sz;
    if (!f) {
        fprintf(stderr, "Unable to open file \"%s\"\n", name);
        exit(1);
    }
    sz = 0;
    while (!feof(f)) {
        int const read = fread((void *)load_to, 1, 1024, f);
        if (read == 0) break;
        sz += read;
        load_to += read;
    }
    printf("\n*********************************\n");
    printf("File \"%s\" loaded, size %d\n", name, sz);
}

void execute_test(const char* filename, int success_check)
{
    int success = 0;
    int addr;

    i8080 *cpu = i8080_new(rd_func, wr_func, in_func, out_func, 0);

    memset(mem, 0, 0x10000);
    load_file(filename, mem + 0x100);

    mem[5] = 0xC9;  // Inject RET at 0x0005 to handle "CALL 5".
    i8080_reset(cpu);
    i8080_jump(cpu, 0x0100);
    while (1) {
        int const pc = i8080_pc(cpu);
        if (mem[pc] == 0x76) {
            printf("HLT at %04X\n", pc);
            exit(1);
        }
        if (pc == 0x0005) {
            /* intercept CP/M BIOS calls */
            if (i8080_regs_c(cpu) == 9) {
                int i;
                for (i = i8080_regs_de(cpu); mem[i] != '$'; i += 1)
                    putchar(mem[i]);
                success = 1;
            }
            if (i8080_regs_c(cpu) == 2) putchar((char)i8080_regs_e(cpu));
        }
        i8080_exec_one_op(cpu);
        if (i8080_pc(cpu) == 0) {
            printf("\nJump to 0000 from %04X\n", pc);
            if (success_check && !success) {
                exit(1);
            }
            return;
        }
    }
}

int main()
{
    execute_test("CPUTEST.COM", 0);
    execute_test("TEST.COM", 0);
    execute_test("8080PRE.COM", 1);
    execute_test("8080EX1.COM", 0);
    return 0;
}
