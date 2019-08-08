/*
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "types.h"
#include "preprocessor.h"

#define BIT(n)      (1u   << (n))
#define BITL(n)     (1ull << (n))
#define MASK(n)     (BIT(n) - 1)
#define MASKL(n)    (BITL(n) - 1)
#define MASK2(a,b)  (MASK(a) & ~MASK(b))
#define MASK2L(a,b) (MASKL(a) & ~MASKL(b))

#define MAKE_REG32(a)   (*(volatile u32 *)(uintptr_t)(a))

#define ALIGN(m)        __attribute__((aligned(m)))
#define PACKED          __attribute__((packed))

#define ALINLINE        __attribute__((always_inline))

#define TEMPORARY       __attribute__((section(".tempbss")))

static inline void __dsb_sy(void)
{
    __asm__ __volatile__ ("dsb sy" ::: "memory");
}

static inline void __isb(void)
{
    __asm__ __volatile__ ("isb" ::: "memory");
}

static inline void __tlb_invalidate_el1_stage12(void)
{
    __asm__ __volatile__ ("tlbi alle1" ::: "memory");
}

bool overlaps(u64 as, u64 ae, u64 bs, u64 be);

static inline uintptr_t get_physical_address_el1_stage12(bool *valid, const uintptr_t el1_vaddr) {
    // NOTE: interrupt must be disabled when calling this func
    uintptr_t PAR;
    __asm__ __volatile__ ("at s12e1r, %0" :: "r"(el1_vaddr)); // note: we don't care whether it's writable in EL1&0 translation regime
    __asm__ __volatile__ ("mrs %0, par_el1" : "=r"(PAR));
    *valid = (PAR & 1) == 0ull;
    return (PAR & 1) ? 0ull : (PAR & MASK2L(40, 12)) | ((uintptr_t)el1_vaddr & MASKL(12));
}

bool readEl1Memory(void *dst, uintptr_t addr, size_t size);
bool writeEl1Memory(uintptr_t addr, const void *src, size_t size);

static inline u32 read32le(const volatile void *dword, size_t offset) {
    return *(u32 *)((uintptr_t)dword + offset);
}

static inline u32 read32be(const volatile void *dword, size_t offset) {
    return __builtin_bswap32(read32le(dword, offset));
}

static inline u64 read64le(const volatile void *qword, size_t offset) {
    return *(u64 *)((uintptr_t)qword + offset);
}

static inline u64 read64be(const volatile void *qword, size_t offset) {
    return __builtin_bswap64(read64le(qword, offset));
}

static inline void write32le(volatile void *dword, size_t offset, u32 value) {
    *(u32 *)((uintptr_t)dword + offset) = value;
}

static inline void write32be(volatile void *dword, size_t offset, u32 value) {
    write32le(dword, offset, __builtin_bswap32(value));
}

static inline void write64le(volatile void *qword, size_t offset, u64 value) {
    *(u64 *)((uintptr_t)qword + offset) = value;
}

static inline void write64be(volatile void *qword, size_t offset, u64 value) {
    write64le(qword, offset, __builtin_bswap64(value));
}

static inline unsigned int get_core_id(void) {
    u64 core_id;
    __asm__ __volatile__ ("mrs %0, mpidr_el1" : "=r"(core_id));
    return (unsigned int)core_id & 3;
}

static inline u64 get_debug_authentication_status(void) {
    u64 debug_auth;
    __asm__ __volatile__ ("mrs  %0, dbgauthstatus_el1" : "=r"(debug_auth));
    return debug_auth;
}

static inline u32 get_spsr(void) {
    u32 spsr;
    __asm__ __volatile__ ("mrs  %0, spsr_el2" : "=r"(spsr));
    return spsr;
}

static inline bool check_32bit_additive_overflow(u32 a, u32 b) {
    return __builtin_add_overflow_p(a, b, (u32)0);
}

static inline void panic(void) {
    __builtin_trap();
    for (;;);
}
