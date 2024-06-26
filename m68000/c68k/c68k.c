/*  Copyright 2003-2004 Stephane Dallongeville

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

/*! \file c68k.c
    \brief C68K init, interrupt and memory access functions.
*/

/*********************************************************************************
 *
 * C68K (68000 CPU emulator) version 0.80
 * Compiled with Dev-C++
 * Copyright 2003-2004 Stephane Dallongeville
 *
 ********************************************************************************/

#include <stdio.h>
#include <string.h>

#include "c68k.h"

/* shared global variable */

c68k_struc C68K;

/* prototype */

/* Read / Write dummy functions */

static uint32_t FASTCALL C68k_Read_Dummy(const uint32_t adr)
{
    return 0;
}

static int32_t FASTCALL C68k_Interrupt_Ack_Dummy(int32_t level)
{
    /* return vector */
    return (C68K_INTERRUPT_AUTOVECTOR_EX + level);
}

/* Read / Write core functions */
static uint32_t C68k_Read_Long(c68k_struc *cpu, uint32_t adr)
{
#ifdef MSB_FIRST
    return (cpu->Read_Word(adr) << 16) | (cpu->Read_Word(adr + 2) & 0xFFFF);
#else
    return (cpu->Read_Word(adr) << 16) | (cpu->Read_Word(adr + 2) & 0xFFFF);
#endif
}

static void FASTCALL C68k_Reset_Dummy(void) { }
static void FASTCALL C68k_Write_Dummy(const uint32_t adr, uint32_t data) { }

/* core main functions */

void C68k_Init(c68k_struc *cpu, C68K_INT_CALLBACK *int_cb)
{
    memset(cpu, 0, sizeof(c68k_struc));

    C68k_Set_ReadB(cpu, C68k_Read_Dummy);
    C68k_Set_ReadW(cpu, C68k_Read_Dummy);

    C68k_Set_WriteB(cpu, C68k_Write_Dummy);
    C68k_Set_WriteW(cpu, C68k_Write_Dummy);

    if (int_cb)
       cpu->Interrupt_CallBack = int_cb;
    else
       cpu->Interrupt_CallBack = C68k_Interrupt_Ack_Dummy;
    cpu->Reset_CallBack        = C68k_Reset_Dummy;

    /* used to init JumpTable */
    cpu->Status |= C68K_DISABLE;
    C68k_Exec(cpu, 0);
    
    cpu->Status &= ~C68K_DISABLE;
}

int32_t FASTCALL C68k_Reset(c68k_struc *cpu)
{
    memset(cpu, 0, ((uint8_t *)&(cpu->dirty1)) - ((uint8_t *)&(cpu->D[0])));
    
    cpu->flag_notZ = 1;
    cpu->flag_I    = 7;
    cpu->flag_S    = C68K_SR_S;

    cpu->A[7] = C68k_Read_Long(cpu, 0);
    C68k_Set_PC(cpu, C68k_Read_Long(cpu, 4));

    return cpu->Status;
}

void FASTCALL C68k_Set_IRQ(c68k_struc *cpu, int32_t level)
{
    cpu->IRQLine = level;
    if (cpu->Status & C68K_RUNNING)
    {
        cpu->CycleSup = cpu->CycleIO;
        cpu->CycleIO  = 0;
    }
    cpu->Status &= ~(C68K_HALTED | C68K_WAITING);
}

/* setting core functions */

void C68k_Set_Fetch(c68k_struc *cpu, uint32_t low_adr,
      uint32_t high_adr, uintptr_t fetch_adr)
{
    uint32_t i = (low_adr >> C68K_FETCH_SFT) & C68K_FETCH_MASK;
    uint32_t j = (high_adr >> C68K_FETCH_SFT) & C68K_FETCH_MASK;
    fetch_adr -= i << C68K_FETCH_SFT;
    while (i <= j) cpu->Fetch[i++] = fetch_adr;
}

void C68k_Set_ReadB(c68k_struc *cpu, C68K_READ *Func)
{
    cpu->Read_Byte = Func;
}

void C68k_Set_ReadW(c68k_struc *cpu, C68K_READ *Func)
{
    cpu->Read_Word = Func;
}

void C68k_Set_WriteB(c68k_struc *cpu, C68K_WRITE *Func)
{
    cpu->Write_Byte = Func;
}

void C68k_Set_WriteW(c68k_struc *cpu, C68K_WRITE *Func)
{
    cpu->Write_Word = Func;
}

/* externals main functions */

uint32_t C68k_Get_DReg(c68k_struc *cpu, uint32_t num)
{
    return cpu->D[num];
}

uint32_t C68k_Get_AReg(c68k_struc *cpu, uint32_t num)
{
    return cpu->A[num];
}

uint32_t C68k_Get_PC(c68k_struc *cpu)
{
    return (uint32_t)(cpu->PC - cpu->BasePC);
}

uint32_t C68k_Get_SR(c68k_struc *cpu)
{
   c68k_struc *CPU = cpu;
   return ((CPU->flag_S << 0) | (CPU->flag_I << 8) |
         (((CPU->flag_C >> (C68K_SR_C_SFT - 0)) & 1) |
          ((CPU->flag_V >> (C68K_SR_V_SFT - 1)) & 2) |
          (((!CPU->flag_notZ) & 1) << 2) |
          ((CPU->flag_N >> (C68K_SR_N_SFT - 3)) & 8) | 
          ((CPU->flag_X >> (C68K_SR_X_SFT - 4)) & 0x10)));
}

uint32_t C68k_Get_USP(c68k_struc *cpu)
{
    if (cpu->flag_S)
       return cpu->USP;
    return cpu->A[7];
}

uint32_t C68k_Get_MSP(c68k_struc *cpu)
{
    if (cpu->flag_S)
       return cpu->A[7];
    return cpu->USP;
}

void C68k_Set_DReg(c68k_struc *cpu, uint32_t num, uint32_t val)
{
    cpu->D[num] = val;
}

void C68k_Set_AReg(c68k_struc *cpu, uint32_t num, uint32_t val)
{
    cpu->A[num] = val;
}

void C68k_Set_PC(c68k_struc *cpu, uint32_t val)
{
    cpu->BasePC = cpu->Fetch[(val >> C68K_FETCH_SFT) & C68K_FETCH_MASK];
    cpu->PC = val + cpu->BasePC;
}

void C68k_Set_SR(c68k_struc *cpu, uint32_t val)
{
    c68k_struc *CPU = cpu;
    CPU->flag_C = (val) << (C68K_SR_C_SFT - 0);
    CPU->flag_V = (val) << (C68K_SR_V_SFT - 1);
    CPU->flag_notZ = ~(val) & 4;
    CPU->flag_N = (val) << (C68K_SR_N_SFT - 3);
    CPU->flag_X = (val) << (C68K_SR_X_SFT - 4);
    CPU->flag_I = ((val) >> 8) & 7;
    CPU->flag_S = (val) & C68K_SR_S;
}

void C68k_Set_USP(c68k_struc *cpu, uint32_t val)
{
    if (cpu->flag_S) cpu->USP = val;
    else cpu->A[7] = val;
}

void C68k_Set_MSP(c68k_struc *cpu, uint32_t val)
{
    if (cpu->flag_S) cpu->A[7] = val;
    else cpu->USP = val;
}
