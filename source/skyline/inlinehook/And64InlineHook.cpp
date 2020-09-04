/*
 *  @date   : 2018/04/18
 *  @author : Rprop (r_prop@outlook.com)
 *  https://github.com/Rprop/And64InlineHook
 */
/*
 MIT License

 Copyright (c) 2018 Rprop (r_prop@outlook.com)

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
 */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#if defined(__aarch64__)

#include "nn/os.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/utils/cpputils.hpp"

#define A64_MAX_INSTRUCTIONS 5
#define A64_MAX_REFERENCES (A64_MAX_INSTRUCTIONS * 2)
#define A64_NOP 0xd503201fu
typedef uint32_t* __restrict* __restrict instruction;
typedef struct {
    struct fix_info {
        uint32_t* bprx;
        uint32_t* bprw;
        uint32_t ls;  // left-shift counts
        uint32_t ad;  // & operand
    };
    struct insns_info {
        union {
            uint64_t insu;
            int64_t ins;
            void* insp;
        };
        fix_info fmap[A64_MAX_REFERENCES];
    };
    int64_t basep;
    int64_t endp;
    insns_info dat[A64_MAX_INSTRUCTIONS];

   public:
    inline bool is_in_fixing_range(const int64_t absolute_addr) {
        return absolute_addr >= this->basep && absolute_addr < this->endp;
    }
    inline intptr_t get_ref_ins_index(const int64_t absolute_addr) {
        return static_cast<intptr_t>((absolute_addr - this->basep) / sizeof(uint32_t));
    }
    inline intptr_t get_and_set_current_index(uint32_t* __restrict inp, uint32_t* __restrict outp) {
        intptr_t current_idx = this->get_ref_ins_index(reinterpret_cast<int64_t>(inp));
        this->dat[current_idx].insp = outp;
        return current_idx;
    }
    inline void reset_current_ins(const intptr_t idx, uint32_t* __restrict outp) { this->dat[idx].insp = outp; }
    void insert_fix_map(const intptr_t idx, uint32_t* bprw, uint32_t* bprx, uint32_t ls = 0u,
                        uint32_t ad = 0xffffffffu) {
        for (auto& f : this->dat[idx].fmap) {
            if (f.bprw == NULL) {
                f.bprw = bprw;
                f.bprx = bprx;
                f.ls = ls;
                f.ad = ad;
                return;
            }  // if
        }
        // What? GGing..
    }
    void process_fix_map(const intptr_t idx) {
        for (auto& f : this->dat[idx].fmap) {
            if (f.bprw == NULL) break;
            *(f.bprw) =
                *(f.bprx) | (((int32_t(this->dat[idx].ins - reinterpret_cast<int64_t>(f.bprx)) >> 2) << f.ls) & f.ad);
            f.bprw = NULL;
            f.bprx = NULL;
        }
    }
} context;

//-------------------------------------------------------------------------

static bool __fix_branch_imm(instruction inprwp, instruction inprxp, instruction outprw, instruction outprx,
                             context* ctxp) {
    static constexpr uint32_t mbits = 6u;
    static constexpr uint32_t mask = 0xfc000000u;   // 0b11111100000000000000000000000000
    static constexpr uint32_t rmask = 0x03ffffffu;  // 0b00000011111111111111111111111111
    static constexpr uint32_t op_b = 0x14000000u;   // "b"  ADDR_PCREL26
    static constexpr uint32_t op_bl = 0x94000000u;  // "bl" ADDR_PCREL26

    const uint32_t ins = *(*inprwp);
    const uint32_t opc = ins & mask;
    switch (opc) {
        case op_b:
        case op_bl: {
            intptr_t current_idx = ctxp->get_and_set_current_index(*inprxp, *outprx);
            int64_t absolute_addr = reinterpret_cast<int64_t>(*inprxp) +
                                    (static_cast<int32_t>(ins << mbits) >> (mbits - 2u));  // sign-extended
            int64_t new_pc_offset =
                static_cast<int64_t>(absolute_addr - reinterpret_cast<int64_t>(*outprx)) >> 2;  // shifted
            bool special_fix_type = ctxp->is_in_fixing_range(absolute_addr);
            // whether the branch should be converted to absolute jump
            if (!special_fix_type && llabs(new_pc_offset) >= (rmask >> 1)) {
                bool b_aligned = (reinterpret_cast<uint64_t>(*outprx + 2) & 7u) == 0u;
                if (opc == op_b) {
                    if (b_aligned != true) {
                        (*outprw)[0] = A64_NOP;
                        ctxp->reset_current_ins(current_idx, ++(*outprx));
                        ++(*outprw);
                    }                            // if
                    (*outprw)[0] = 0x58000051u;  // LDR X17, #0x8
                    (*outprw)[1] = 0xd61f0220u;  // BR X17
                    memcpy(*outprw + 2, &absolute_addr, sizeof(absolute_addr));
                    *outprx += 4;
                    *outprw += 4;
                } else {
                    if (b_aligned == true) {
                        (*outprw)[0] = A64_NOP;
                        ctxp->reset_current_ins(current_idx, ++(*outprx));
                        (*outprw)++;
                    }                            // if
                    (*outprw)[0] = 0x58000071u;  // LDR X17, #12
                    (*outprw)[1] = 0x1000009eu;  // ADR X30, #16
                    (*outprw)[2] = 0xd61f0220u;  // BR X17
                    memcpy(*outprw + 3, &absolute_addr, sizeof(absolute_addr));
                    *outprw += 5;
                    *outprx += 5;
                }  // if
            } else {
                if (special_fix_type) {
                    intptr_t ref_idx = ctxp->get_ref_ins_index(absolute_addr);
                    if (ref_idx <= current_idx) {
                        new_pc_offset =
                            static_cast<int64_t>(ctxp->dat[ref_idx].ins - reinterpret_cast<int64_t>(*outprx)) >> 2;
                    } else {
                        ctxp->insert_fix_map(ref_idx, *outprw, *outprx, 0u, rmask);
                        new_pc_offset = 0;
                    }  // if
                }      // if

                (*outprw)[0] = opc | (new_pc_offset & ~mask);
                ++(*outprw);
                ++(*outprx);
            }  // if

            ++(*inprxp);
            ++(*inprwp);
            return ctxp->process_fix_map(current_idx), true;
        }
    }
    return false;
}

//-------------------------------------------------------------------------

static bool __fix_cond_comp_test_branch(instruction inprwp, instruction inprxp, instruction outprw, instruction outprx,
                                        context* ctxp) {
    static constexpr uint32_t lsb = 5u;
    static constexpr uint32_t lmask01 = 0xff00001fu;  // 0b11111111000000000000000000011111
    static constexpr uint32_t mask0 = 0xff000010u;    // 0b11111111000000000000000000010000
    static constexpr uint32_t op_bc = 0x54000000u;    // "b.c"  ADDR_PCREL19
    static constexpr uint32_t mask1 = 0x7f000000u;    // 0b01111111000000000000000000000000
    static constexpr uint32_t op_cbz = 0x34000000u;   // "cbz"  Rt, ADDR_PCREL19
    static constexpr uint32_t op_cbnz = 0x35000000u;  // "cbnz" Rt, ADDR_PCREL19
    static constexpr uint32_t lmask2 = 0xfff8001fu;   // 0b11111111111110000000000000011111
    static constexpr uint32_t mask2 = 0x7f000000u;    // 0b01111111000000000000000000000000
    static constexpr uint32_t op_tbz =
        0x36000000u;  // 0b00110110000000000000000000000000 "tbz"  Rt, BIT_NUM, ADDR_PCREL14
    static constexpr uint32_t op_tbnz =
        0x37000000u;  // 0b00110111000000000000000000000000 "tbnz" Rt, BIT_NUM, ADDR_PCREL14

    const uint32_t ins = *(*inprwp);
    uint32_t lmask = lmask01;
    if ((ins & mask0) != op_bc) {
        uint32_t opc = ins & mask1;
        if (opc != op_cbz && opc != op_cbnz) {
            opc = ins & mask2;
            if (opc != op_tbz && opc != op_tbnz) {
                return false;
            }  // if
            lmask = lmask2;
        }  // if
    }      // if

    intptr_t current_idx = ctxp->get_and_set_current_index(*inprxp, *outprx);
    int64_t absolute_addr = reinterpret_cast<int64_t>(*inprxp) + ((ins & ~lmask) >> (lsb - 2u));
    int64_t new_pc_offset = static_cast<int64_t>(absolute_addr - reinterpret_cast<int64_t>(*outprx)) >> 2;  // shifted
    bool special_fix_type = ctxp->is_in_fixing_range(absolute_addr);
    if (!special_fix_type && llabs(new_pc_offset) >= (~lmask >> (lsb + 1))) {
        if ((reinterpret_cast<uint64_t>(*outprx + 4) & 7u) != 0u) {
            (*outprw)[0] = A64_NOP;
            ctxp->reset_current_ins(current_idx, *outprx);

            (*outprx)++;
            (*outprw)++;
        }                                                               // if
        (*outprw)[0] = (((8u >> 2u) << lsb) & ~lmask) | (ins & lmask);  // B.C #0x8
        (*outprw)[1] = 0x14000005u;                                     // B #0x14
        (*outprw)[2] = 0x58000051u;                                     // LDR X17, #0x8
        (*outprw)[3] = 0xd61f0220u;                                     // BR X17
        memcpy(*outprw + 4, &absolute_addr, sizeof(absolute_addr));
        *outprw += 6;
        *outprx += 6;
    } else {
        if (special_fix_type) {
            intptr_t ref_idx = ctxp->get_ref_ins_index(absolute_addr);
            if (ref_idx <= current_idx) {
                new_pc_offset = static_cast<int64_t>(ctxp->dat[ref_idx].ins - reinterpret_cast<int64_t>(*outprx)) >> 2;
            } else {
                ctxp->insert_fix_map(ref_idx, *outprw, *outprx, lsb, ~lmask);
                new_pc_offset = 0;
            }  // if
        }      // if

        (*outprw)[0] = (static_cast<uint32_t>(new_pc_offset << lsb) & ~lmask) | (ins & lmask);
        ++(*outprw);
        ++(*outprx);
    }  // if

    ++(*inprxp);
    ++(*inprwp);
    return ctxp->process_fix_map(current_idx), true;
}

//-------------------------------------------------------------------------

static bool __fix_loadlit(instruction inprwp, instruction inprxp, instruction outprw, instruction outprx,
                          context* ctxp) {
    const uint32_t ins = *(*inprwp);

    // memory prefetch("prfm"), just skip it
    // http://infocenter.arm.com/help/topic/com.arm.doc.100069_0608_00_en/pge1427897420050.html
    if ((ins & 0xff000000u) == 0xd8000000u) {
        ctxp->process_fix_map(ctxp->get_and_set_current_index(*inprxp, *outprx));
        ++(*inprwp);
        ++(*inprxp);
        return true;
    }  // if

    static constexpr uint32_t msb = 8u;
    static constexpr uint32_t lsb = 5u;
    static constexpr uint32_t mask_30 = 0x40000000u;   // 0b01000000000000000000000000000000
    static constexpr uint32_t mask_31 = 0x80000000u;   // 0b10000000000000000000000000000000
    static constexpr uint32_t lmask = 0xff00001fu;     // 0b11111111000000000000000000011111
    static constexpr uint32_t mask_ldr = 0xbf000000u;  // 0b10111111000000000000000000000000
    static constexpr uint32_t op_ldr =
        0x18000000u;  // 0b00011000000000000000000000000000 "LDR Wt/Xt, label" | ADDR_PCREL19
    static constexpr uint32_t mask_ldrv = 0x3f000000u;  // 0b00111111000000000000000000000000
    static constexpr uint32_t op_ldrv =
        0x1c000000u;  // 0b00011100000000000000000000000000 "LDR St/Dt/Qt, label" | ADDR_PCREL19
    static constexpr uint32_t mask_ldrsw = 0xff000000u;  // 0b11111111000000000000000000000000
    static constexpr uint32_t op_ldrsw = 0x98000000u;  // "LDRSW Xt, label" | ADDR_PCREL19 | load register signed word
    // LDR S0, #0 | 0b00011100000000000000000000000000 | 32-bit
    // LDR D0, #0 | 0b01011100000000000000000000000000 | 64-bit
    // LDR Q0, #0 | 0b10011100000000000000000000000000 | 128-bit
    // INVALID    | 0b11011100000000000000000000000000 | may be 256-bit

    uint32_t mask = mask_ldr;
    uintptr_t faligned = (ins & mask_30) ? 7u : 3u;
    if ((ins & mask_ldr) != op_ldr) {
        mask = mask_ldrv;
        if (faligned != 7u) faligned = (ins & mask_31) ? 15u : 3u;
        if ((ins & mask_ldrv) != op_ldrv) {
            if ((ins & mask_ldrsw) != op_ldrsw) {
                return false;
            }  // if
            mask = mask_ldrsw;
            faligned = 7u;
        }  // if
    }      // if

    intptr_t current_idx = ctxp->get_and_set_current_index(*inprxp, *outprx);
    int64_t absolute_addr =
        reinterpret_cast<int64_t>(*inprxp) + ((static_cast<int32_t>(ins << msb) >> (msb + lsb - 2u)) & ~3u);
    int64_t new_pc_offset = static_cast<int64_t>(absolute_addr - reinterpret_cast<int64_t>(*outprx)) >> 2;  // shifted
    bool special_fix_type = ctxp->is_in_fixing_range(absolute_addr);
    // special_fix_type may encounter issue when there are mixed data and code
    if (special_fix_type ||
        (llabs(new_pc_offset) + (faligned + 1u - 4u) / 4u) >= (~lmask >> (lsb + 1))) {  // inaccurate, but it works
        while ((reinterpret_cast<uint64_t>(*outprx + 2) & faligned) != 0u) {
            *(*outprw)++ = A64_NOP;
            (*outprx)++;
        }
        ctxp->reset_current_ins(current_idx, *outprx);

        // Note that if memory at absolute_addr is writeable (non-const), we will fail to fetch it.
        // And what's worse, we may unexpectedly overwrite something if special_fix_type is true...
        uint32_t ns = static_cast<uint32_t>((faligned + 1) / sizeof(uint32_t));
        (*outprw)[0] = (((8u >> 2u) << lsb) & ~mask) | (ins & lmask);  // LDR #0x8
        (*outprw)[1] = 0x14000001u + ns;                               // B #0xc
        memcpy(*outprw + 2, reinterpret_cast<void*>(absolute_addr), faligned + 1);
        *outprw += 2 + ns;
        *outprx += 2 + ns;
    } else {
        faligned >>= 2;  // new_pc_offset is shifted and 4-byte aligned
        while ((new_pc_offset & faligned) != 0) {
            *(*outprw)++ = A64_NOP;
            (*outprx)++;
            new_pc_offset = static_cast<int64_t>(absolute_addr - reinterpret_cast<int64_t>(*outprx)) >> 2;
        }
        ctxp->reset_current_ins(current_idx, *outprx);

        (*outprw)[0] = (static_cast<uint32_t>(new_pc_offset << lsb) & ~mask) | (ins & lmask);
        ++(*outprx);
        ++(*outprw);
    }  // if

    ++(*inprxp);
    ++(*inprwp);
    return ctxp->process_fix_map(current_idx), true;
}

//-------------------------------------------------------------------------

static bool __fix_pcreladdr(instruction inprwp, instruction inprxp, instruction outprw, instruction outprx,
                            context* ctxp) {
    // Load a PC-relative address into a register
    // http://infocenter.arm.com/help/topic/com.arm.doc.100069_0608_00_en/pge1427897645644.html
    static constexpr uint32_t msb = 8u;
    static constexpr uint32_t lsb = 5u;
    static constexpr uint32_t mask = 0x9f000000u;     // 0b10011111000000000000000000000000
    static constexpr uint32_t rmask = 0x0000001fu;    // 0b00000000000000000000000000011111
    static constexpr uint32_t lmask = 0xff00001fu;    // 0b11111111000000000000000000011111
    static constexpr uint32_t fmask = 0x00ffffffu;    // 0b00000000111111111111111111111111
    static constexpr uint32_t max_val = 0x001fffffu;  // 0b00000000000111111111111111111111
    static constexpr uint32_t op_adr = 0x10000000u;   // "adr"  Rd, ADDR_PCREL21
    static constexpr uint32_t op_adrp = 0x90000000u;  // "adrp" Rd, ADDR_ADRP

    const uint32_t ins = *(*inprwp);
    intptr_t current_idx;
    switch (ins & mask) {
        case op_adr: {
            current_idx = ctxp->get_and_set_current_index(*inprxp, *outprx);
            int64_t lsb_bytes = static_cast<uint32_t>(ins << 1u) >> 30u;
            int64_t absolute_addr = reinterpret_cast<int64_t>(*inprxp) +
                                    (((static_cast<int32_t>(ins << msb) >> (msb + lsb - 2u)) & ~3u) | lsb_bytes);
            int64_t new_pc_offset = static_cast<int64_t>(absolute_addr - reinterpret_cast<int64_t>(*outprx));
            bool special_fix_type = ctxp->is_in_fixing_range(absolute_addr);
            if (!special_fix_type && llabs(new_pc_offset) >= (max_val >> 1)) {
                if ((reinterpret_cast<uint64_t>(*outprx + 2) & 7u) != 0u) {
                    (*outprw)[0] = A64_NOP;
                    ctxp->reset_current_ins(current_idx, ++(*outprx));
                    ++*(outprw);
                }  // if

                (*outprw)[0] = 0x58000000u | (((8u >> 2u) << lsb) & ~mask) | (ins & rmask);  // LDR #0x8
                (*outprw)[1] = 0x14000003u;                                                  // B #0xc
                memcpy(*outprw + 2, &absolute_addr, sizeof(absolute_addr));
                *outprw += 4;
                *outprx += 4;
            } else {
                if (special_fix_type) {
                    intptr_t ref_idx = ctxp->get_ref_ins_index(absolute_addr & ~3ull);
                    if (ref_idx <= current_idx) {
                        new_pc_offset =
                            static_cast<int64_t>(ctxp->dat[ref_idx].ins - reinterpret_cast<int64_t>(*outprx));
                    } else {
                        ctxp->insert_fix_map(ref_idx, *outprw, *outprx, lsb, fmask);
                        new_pc_offset = 0;
                    }  // if
                }      // if

                // the lsb_bytes will never be changed, so we can use lmask to keep it
                (*outprw)[0] = (static_cast<uint32_t>(new_pc_offset << (lsb - 2u)) & fmask) | (ins & lmask);
                ++(*outprw);
                ++(*outprx);
            }  // if
        } break;
        case op_adrp: {
            current_idx = ctxp->get_and_set_current_index(*inprxp, *outprw);
            int32_t lsb_bytes = static_cast<uint32_t>(ins << 1u) >> 30u;
            int64_t absolute_addr =
                (reinterpret_cast<int64_t>(*inprxp) & ~0xfffll) +
                ((((static_cast<int32_t>(ins << msb) >> (msb + lsb - 2u)) & ~3u) | lsb_bytes) << 12);
            skyline::logger::s_Instance->LogFormat("[And64InlineHook] ins = 0x%.8X, pc = %p, abs_addr = %p", ins,
                                                   *inprxp, reinterpret_cast<int64_t*>(absolute_addr));
            if (ctxp->is_in_fixing_range(absolute_addr)) {
                intptr_t ref_idx = ctxp->get_ref_ins_index(absolute_addr /* & ~3ull*/);
                if (ref_idx > current_idx) {
                    // the bottom 12 bits of absolute_addr are masked out,
                    // so ref_idx must be less than or equal to current_idx!
                    skyline::logger::s_Instance->Log(
                        "[And64InlineHook] ref_idx must be less than or equal to current_idx!\n");
                }  // if

                // *absolute_addr may be changed due to relocation fixing
                skyline::logger::s_Instance->Log("What is the correct way to fix this?\n");
                *(*outprw)++ = ins;  // 0x90000000u;
                (*outprx)++;
            } else {
                if ((reinterpret_cast<uint64_t>(*outprx + 2) & 7u) != 0u) {
                    (*outprw)[0] = A64_NOP;
                    ctxp->reset_current_ins(current_idx, ++(*outprx));
                    ++*(outprw);
                }  // if

                (*outprw)[0] = 0x58000000u | (((8u >> 2u) << lsb) & ~mask) | (ins & rmask);  // LDR #0x8
                (*outprw)[1] = 0x14000003u;                                                  // B #0xc
                memcpy(*outprw + 2, &absolute_addr, sizeof(absolute_addr));                  // potential overflow?
                *outprw += 4;
                *outprx += 4;
            }  // if
        } break;
        default:
            return false;
    }

    ctxp->process_fix_map(current_idx);
    ++(*inprxp);
    ++(*inprwp);
    return true;
}

#define __flush_cache(c, n) __builtin___clear_cache(reinterpret_cast<char*>(c), reinterpret_cast<char*>(c) + n)

//-------------------------------------------------------------------------

static void __fix_instructions(uint32_t* __restrict inprw, uint32_t* __restrict inprx, int32_t count,
                               uint32_t* __restrict outrwp, uint32_t* __restrict outrxp) {
    context ctx;
    ctx.basep = reinterpret_cast<int64_t>(inprx);
    ctx.endp = reinterpret_cast<int64_t>(inprx + count);
    memset(ctx.dat, 0, sizeof(ctx.dat));
    static_assert(sizeof(ctx.dat) / sizeof(ctx.dat[0]) == A64_MAX_INSTRUCTIONS, "please use A64_MAX_INSTRUCTIONS!");
#ifndef NDEBUG
    if (count > A64_MAX_INSTRUCTIONS) {
        skyline::logger::s_Instance->Log("[And64InlineHook] too many fixing instructions!\n");
    }   // if
#endif  // NDEBUG

    uint32_t* const outprx_base = outrxp;
    uint32_t* const outprw_base = outrwp;

    while (--count >= 0) {
        if (__fix_branch_imm(&inprw, &inprx, &outrwp, &outrxp, &ctx)) continue;
        if (__fix_cond_comp_test_branch(&inprw, &inprx, &outrwp, &outrxp, &ctx)) continue;
        if (__fix_loadlit(&inprw, &inprx, &outrwp, &outrxp, &ctx)) continue;
        if (__fix_pcreladdr(&inprw, &inprx, &outrwp, &outrxp, &ctx)) continue;

        // without PC-relative offset
        ctx.process_fix_map(ctx.get_and_set_current_index(inprx, outrxp));
        *(outrwp++) = *(inprw++);
        outrxp++;
        inprx++;
    }

    static constexpr uint_fast64_t mask = 0x03ffffffu;  // 0b00000011111111111111111111111111
    auto callback = reinterpret_cast<int64_t>(inprx);
    auto pc_offset = static_cast<int64_t>(callback - reinterpret_cast<int64_t>(outrxp)) >> 2;
    if (llabs(pc_offset) >= (mask >> 1)) {
        if ((reinterpret_cast<uint64_t>(outrxp + 2) & 7u) != 0u) {
            outrwp[0] = A64_NOP;
            ++outrxp;
            ++outrwp;
        }                         // if
        outrwp[0] = 0x58000051u;  // LDR X17, #0x8
        outrwp[1] = 0xd61f0220u;  // BR X17
        *reinterpret_cast<int64_t*>(outrwp + 2) = callback;
        outrwp += 4;
        outrxp += 4;
    } else {
        outrwp[0] = 0x14000000u | (pc_offset & mask);  // "B" ADDR_PCREL26
        ++outrwp;
        ++outrxp;
    }  // if

    const uintptr_t total = (outrxp - outprx_base) * sizeof(uint32_t);
    __flush_cache(outprx_base, total);  // necessary
    __flush_cache(outprw_base, total);
}

//-------------------------------------------------------------------------

#define __attribute __attribute__
#define aligned(x) __aligned__(x)
#define __intval(p) reinterpret_cast<intptr_t>(p)
#define __uintval(p) reinterpret_cast<uintptr_t>(p)
#define __ptr(p) reinterpret_cast<void*>(p)
#define __page_size PAGE_SIZE
#define __page_align(n) __align_up(static_cast<uintptr_t>(n), __page_size)
#define __ptr_align(x) __ptr(__align_down(reinterpret_cast<uintptr_t>(x), __page_size))
#define __align_up(x, n) (((x) + ((n)-1)) & ~((n)-1))
#define __align_down(x, n) ((x) & -(n))
#define __countof(x) static_cast<intptr_t>(sizeof(x) / sizeof((x)[0]))  // must be signed
#define __atomic_increase(p) __sync_add_and_fetch(p, 1)
#define __sync_cmpswap(p, v, n) __sync_bool_compare_and_swap(p, v, n)
typedef uint32_t insns_t[A64_MAX_BACKUPS][A64_MAX_INSTRUCTIONS * 10u];

constexpr size_t inline_hook_handler_size = 0xC;  // correct if handler size changes
struct PACKED inline_hook_entry {
    std::array<uint8_t, inline_hook_handler_size> handler;
    const void* cur_handler;
    const void* callback;
    const void* trampoline;
};

constexpr size_t inline_hook_size = sizeof(inline_hook_entry);
constexpr size_t inline_hook_count = 0x1000;
constexpr size_t inline_hook_pool_size = inline_hook_size * inline_hook_count;

//-------------------------------------------------------------------------

static Jit __insns_jit;
static Jit __inline_hook_jit;
static nn::os::MutexType hookMutex;

//-------------------------------------------------------------------------

void A64HookInit() {
    nn::os::InitializeMutex(&hookMutex, false, 0);

    // allocate normal hook JIT
    Result rc = jitCreate(&__insns_jit, NULL, sizeof(insns_t));
    R_ERRORONFAIL(rc);
    memset(__insns_jit.rw_addr, 0, __insns_jit.size);
    rc = jitTransitionToExecutable(&__insns_jit);
    R_ERRORONFAIL(rc);

    // search for applicable space for inline hook JIT
    auto cur_searching_addr =
        skyline::utils::g_MainTextAddr - inline_hook_pool_size;  // start searching from right before .text

    MemoryInfo mem;
    while (true) {
        u32 page_info;
        if (R_SUCCEEDED(svcQueryMemory(&mem, &page_info, cur_searching_addr)) && mem.type == MemType_Unmapped &&
            mem.size >= ALIGN_UP(inline_hook_pool_size, PAGE_SIZE)) {
            break;
        }
        cur_searching_addr -= PAGE_SIZE;
    }

    // allocate inline hook JIT
    rc = jitCreate(&__inline_hook_jit, (void*)ALIGN_DOWN(mem.addr + mem.size - inline_hook_pool_size, PAGE_SIZE),
                   inline_hook_pool_size);
    R_ERRORONFAIL(rc);
}

//-------------------------------------------------------------------------

static void FastAllocateTrampoline(uint32_t** rx, uint32_t** rw) {
    static_assert((A64_MAX_INSTRUCTIONS * 10 * sizeof(uint32_t)) % 8 == 0, "8-byte align");
    static volatile int32_t __index = -1;

    uint32_t i = __atomic_increase(&__index);
    insns_t* rwptr = (insns_t*)__insns_jit.rw_addr;
    insns_t* rxptr = (insns_t*)__insns_jit.rx_addr;
    *rw = (*rwptr)[i];
    *rx = (*rxptr)[i];
}

//-------------------------------------------------------------------------

void* A64HookFunctionV(void* const symbol, void* const replace, void* const rxtr, void* const rwtr,
                       const uintptr_t rwx_size) {
    static constexpr uint_fast64_t mask = 0x03ffffffu;  // 0b00000011111111111111111111111111

    uint32_t *rxtrampoline = static_cast<uint32_t*>(rxtr), *rwtrampoline = static_cast<uint32_t*>(rwtr),
             *original = static_cast<uint32_t*>(symbol);

    static_assert(A64_MAX_INSTRUCTIONS >= 5, "please fix A64_MAX_INSTRUCTIONS!");
    auto pc_offset = static_cast<int64_t>(__intval(replace) - __intval(symbol)) >> 2;
    if (llabs(pc_offset) >= (mask >> 1)) {
        skyline::inlinehook::ControlledPages control(original, 5 * sizeof(uint32_t));
        control.claim();

        int32_t count = (reinterpret_cast<uint64_t>(original + 2) & 7u) != 0u ? 5 : 4;

        original = (u32*)control.rw;

        if (rxtrampoline) {
            if (rwx_size < count * 10u) {
                skyline::logger::s_Instance->LogFormat(
                    "[And64InlineHook] rwx size is too small to hold %u bytes backup instructions!", count * 10u);
                control.unclaim();
                return NULL;
            }  // if
            __fix_instructions(original, (u32*)control.rx, count, rwtrampoline, rxtrampoline);
        }  // if

        if (count == 5) {
            original[0] = A64_NOP;
            ++original;
        }                           // if
        original[0] = 0x58000051u;  // LDR X17, #0x8
        original[1] = 0xd61f0220u;  // BR X17
        *reinterpret_cast<int64_t*>(original + 2) = __intval(replace);
        __flush_cache(symbol, 5 * sizeof(uint32_t));

        skyline::logger::s_Instance->LogFormat(
            "[And64InlineHook] inline hook %p->%p successfully! %zu bytes overwritten", symbol, replace,
            5 * sizeof(uint32_t));

        control.unclaim();
    } else {
        skyline::inlinehook::ControlledPages control(original, 1 * sizeof(uint32_t));
        control.claim();

        original = (u32*)control.rw;

        if (rwtrampoline) {
            if (rwx_size < 1u * 10u) {
                skyline::logger::s_Instance->LogFormat(
                    "[And64InlineHook] rwx size is too small to hold %u bytes backup instructions!", 1u * 10u);
                control.unclaim();
                return NULL;
            }  // if
            __fix_instructions(original, (u32*)control.rx, 1, rwtrampoline, rxtrampoline);
        }  // if

        __sync_cmpswap(original, *original, 0x14000000u | (pc_offset & mask));  // "B" ADDR_PCREL26
        __flush_cache(symbol, 1 * sizeof(uint32_t));

        skyline::logger::s_Instance->LogFormat(
            "[And64InlineHook] inline hook %p->%p successfully! %zu bytes overwritten", symbol, replace,
            1 * sizeof(uint32_t));

        control.unclaim();
    }  // if

    // if(rwtrampoline)
    //    rwtrampoline[0] = 0xDEADBEEF;
    return rxtrampoline;
}

//-------------------------------------------------------------------------

extern "C" void A64HookFunction(void* const symbol, void* const replace, void** result) {
    nn::os::LockMutex(&hookMutex);

    R_ERRORONFAIL(jitTransitionToWritable(&__insns_jit));

    uint32_t *rxtrampoline = NULL, *rwtrampoline = NULL;
    if (result != NULL) {
        FastAllocateTrampoline(&rxtrampoline, &rwtrampoline);
        *result = rxtrampoline;
        if (rxtrampoline == NULL) {
            *(u64*)rxtrampoline = 0;
            return;
        };
    }  // if

    rxtrampoline =
        (uint32_t*)A64HookFunctionV(symbol, replace, rxtrampoline, rwtrampoline, A64_MAX_INSTRUCTIONS * 10u);
    if (rxtrampoline == NULL && result != NULL) {
        *result = NULL;
    }  // if

    R_ERRORONFAIL(jitTransitionToExecutable(&__insns_jit));

    nn::os::UnlockMutex(&hookMutex);
}

extern const void (*inlineHandlerStart)(void);
extern const void* inlineHandlerEnd;
extern const void (*inlineHandlerImpl)(void);

u64 inline_hook_curridx = 0;

extern "C" void A64InlineHook(void* const address, void* const callback) {
    u64 handler_start_addr = (u64)&inlineHandlerStart;
    u64 handler_end_addr = (u64)&inlineHandlerEnd;

    // make sure inline hook handler constexpr is correct
    if (inline_hook_handler_size != handler_end_addr - handler_start_addr) {
        skyline::logger::s_Instance->LogFormat("[A64InlineHook] invalid handler size, mannual updating required");
        R_ERRORONFAIL(MAKERESULT(Module_Skyline, SkylineError_InlineHookHandlerSizeInvalid));
    }

    // check pool availability
    if (inline_hook_curridx >= inline_hook_count) {
        skyline::logger::s_Instance->LogFormat("[A64InlineHook] inline hook pool exausted");
        R_ERRORONFAIL(MAKERESULT(Module_Skyline, SkylineError_InlineHookPoolExhausted));
    }

    // prepare to hook
    jitTransitionToExecutable(&__inline_hook_jit);
    auto& rx_entries = *reinterpret_cast<std::array<inline_hook_entry, inline_hook_count>*>(__inline_hook_jit.rx_addr);
    auto& rx = rx_entries[inline_hook_curridx];

    // hook to call the handler
    void* trampoline;
    A64HookFunction(address, &rx.handler, &trampoline);

    // populate handler entry
    jitTransitionToWritable(&__inline_hook_jit);
    auto& rw_entries = *reinterpret_cast<std::array<inline_hook_entry, inline_hook_count>*>(__inline_hook_jit.rw_addr);
    auto& rw = rw_entries[inline_hook_curridx];

    memcpy(rw.handler.data(), (void*)handler_start_addr, inline_hook_handler_size);
    rw.cur_handler = &inlineHandlerImpl;
    rw.callback = callback;
    rw.trampoline = trampoline;

    // finalize, make handler executable
    jitTransitionToExecutable(&__inline_hook_jit);

    inline_hook_curridx++;
}

#endif  // defined(__aarch64__)
