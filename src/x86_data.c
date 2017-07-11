/*
 * rpiz - https://github.com/bp0/rpiz
 * Copyright (C) 2017  Burt P. <pburt0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "x86_data.h"

#ifndef _
#define _(String) String
#endif
#ifndef N_
#define N_(String) String
#endif

/* sources:
 *   https://unix.stackexchange.com/a/43540
 *   https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git/tree/arch/x86/include/asm/cpufeatures.h?id=refs/tags/v4.9
 *   hardinfo: modules/devices/x86/processor.c
 */
static struct {
    char *name, *meaning;
} tab_flag_meaning[] = {
/* Intel-defined CPU features, CPUID level 0x00000001 (edx)
 * See also Wikipedia and table 2-27 in Intel Advanced Vector Extensions Programming Reference */
    { "fpu",     N_("Onboard FPU (floating point support)") },
    { "vme",     N_("Virtual 8086 mode enhancements") },
    { "de",      N_("Debugging Extensions (CR4.DE)") },
    { "pse",     N_("Page Size Extensions (4MB memory pages)") },
    { "tsc",     N_("Time Stamp Counter (RDTSC)") },
    { "msr",     N_("Model-Specific Registers (RDMSR, WRMSR)") },
    { "pae",     N_("Physical Address Extensions (support for more than 4GB of RAM)") },
    { "mce",     N_("Machine Check Exception") },
    { "cx8",     N_("CMPXCHG8 instruction (64-bit compare-and-swap)") },
    { "apic",    N_("Onboard APIC") },
    { "sep",     N_("SYSENTER/SYSEXIT") },
    { "mtrr",    N_("Memory Type Range Registers") },
    { "pge",     N_("Page Global Enable (global bit in PDEs and PTEs)") },
    { "mca",     N_("Machine Check Architecture") },
    { "cmov",    N_("CMOV instructions (conditional move) (also FCMOV)") },
    { "pat",     N_("Page Attribute Table") },
    { "pse36",   N_("36-bit PSEs (huge pages)") },
    { "pn",      N_("Processor serial number") },
    { "clflush", N_("Cache Line Flush instruction") },
    { "dts",     N_("Debug Store (buffer for debugging and profiling instructions), or alternately: digital thermal sensor") },
    { "acpi",    N_("ACPI via MSR (temperature monitoring and clock speed modulation)") },
    { "mmx",     N_("Multimedia Extensions") },
    { "fxsr",    N_("FXSAVE/FXRSTOR, CR4.OSFXSR") },
    { "sse",     N_("Intel SSE vector instructions") },
    { "sse2",    N_("SSE2") },
    { "ss",      N_("CPU self snoop") },
    { "ht",      N_("Hyper-Threading") },
    { "tm",      N_("Automatic clock control (Thermal Monitor)") },
    { "ia64",    N_("Intel Itanium Architecture 64-bit (not to be confused with Intel's 64-bit x86 architecture with flag x86-64 or \"AMD64\" bit indicated by flag lm)") },
    { "pbe",     N_("Pending Break Enable (PBE# pin) wakeup support") },
/* AMD-defined CPU features, CPUID level 0x80000001
 * See also Wikipedia and table 2-23 in Intel Advanced Vector Extensions Programming Reference */
    { "syscall",  N_("SYSCALL (Fast System Call) and SYSRET (Return From Fast System Call)") },
    { "mp",       N_("Multiprocessing Capable.") },
    { "nx",       N_("Execute Disable") },
    { "mmxext",   N_("AMD MMX extensions") },
    { "fxsr_opt", N_("FXSAVE/FXRSTOR optimizations") },
    { "pdpe1gb",  N_("One GB pages (allows hugepagesz=1G)") },
    { "rdtscp",   N_("Read Time-Stamp Counter and Processor ID") },
    { "lm",       N_("Long Mode (x86-64: amd64, also known as Intel 64, i.e. 64-bit capable)") },
    { "3dnow",    N_("3DNow! (AMD vector instructions, competing with Intel's SSE1)") },
    { "3dnowext", N_("AMD 3DNow! extensions") },
/* Transmeta-defined CPU features, CPUID level 0x80860001 */
    { "recovery", N_("CPU in recovery mode") },
    { "longrun",  N_("Longrun power control") },
    { "lrti",     N_("LongRun table interface") },
/* Other features, Linux-defined mapping */
    { "cxmmx",        N_("Cyrix MMX extensions") },
    { "k6_mtrr",      N_("AMD K6 nonstandard MTRRs") },
    { "cyrix_arr",    N_("Cyrix ARRs (= MTRRs)") },
    { "centaur_mcr",  N_("Centaur MCRs (= MTRRs)") },
    { "constant_tsc", N_("TSC ticks at a constant rate") },
    { "up",           N_("SMP kernel running on UP") },
    { "art",          N_("Always-Running Timer") },
    { "arch_perfmon", N_("Intel Architectural PerfMon") },
    { "pebs",         N_("Precise-Event Based Sampling") },
    { "bts",          N_("Branch Trace Store") },
    { "rep_good",     N_("rep microcode works well") },
    { "acc_power",    N_("AMD accumulated power mechanism") },
    { "nopl",         N_("The NOPL (0F 1F) instructions") },
    { "xtopology",    N_("cpu topology enum extensions") },
    { "tsc_reliable", N_("TSC is known to be reliable") },
    { "nonstop_tsc",  N_("TSC does not stop in C states") },
    { "extd_apicid",  N_("has extended APICID (8 bits)") },
    { "amd_dcm",      N_("multi-node processor") },
    { "aperfmperf",   N_("APERFMPERF") },
    { "eagerfpu",     N_("Non lazy FPU restore") },
    { "nonstop_tsc_s3", N_("TSC doesn't stop in S3 state") },
    { "mce_recovery",   N_("CPU has recoverable machine checks") },
/* Intel-defined CPU features, CPUID level 0x00000001 (ecx)
 * See also Wikipedia and table 2-26 in Intel Advanced Vector Extensions Programming Reference */
    { "pni",       N_("SSE-3 (“Prescott New Instructions”)") },
    { "pclmulqdq", N_("Perform a Carry-Less Multiplication of Quadword instruction — accelerator for GCM)") },
    { "dtes64",    N_("64-bit Debug Store") },
    { "monitor",   N_("Monitor/Mwait support (Intel SSE3 supplements)") },
    { "ds_cpl",    N_("CPL Qual. Debug Store") },
    { "vmx",       N_("Hardware virtualization, Intel VMX") },
    { "smx",       N_("Safer mode TXT (TPM support)") },
    { "est",       N_("Enhanced SpeedStep") },
    { "tm2",       N_("Thermal Monitor 2") },
    { "ssse3",     N_("Supplemental SSE-3") },
    { "cid",       N_("Context ID") },
    { "sdbg",      N_("silicon debug") },
    { "fma",       N_("Fused multiply-add") },
    { "cx16",      N_("CMPXCHG16B") },
    { "xtpr",      N_("Send Task Priority Messages") },
    { "pdcm",      N_("Performance Capabilities") },
    { "pcid",      N_("Process Context Identifiers") },
    { "dca",       N_("Direct Cache Access") },
    { "sse4_1",    N_("SSE-4.1") },
    { "sse4_2",    N_("SSE-4.2") },
    { "x2apic",    N_("x2APIC") },
    { "movbe",     N_("Move Data After Swapping Bytes instruction") },
    { "popcnt",    N_("Return the Count of Number of Bits Set to 1 instruction (Hamming weight, i.e. bit count)") },
    { "tsc_deadline_timer", N_("Tsc deadline timer") },
    { "aes/aes-ni",  N_("Advanced Encryption Standard (New Instructions)") },
    { "xsave",       N_("Save Processor Extended States: also provides XGETBY,XRSTOR,XSETBY") },
    { "avx",         N_("Advanced Vector Extensions") },
    { "f16c",        N_("16-bit fp conversions (CVT16)") },
    { "rdrand",      N_("Read Random Number from hardware random number generator instruction") },
    { "hypervisor",  N_("Running on a hypervisor") },
/* VIA/Cyrix/Centaur-defined CPU features, CPUID level 0xC0000001 */
    { "rng",     N_("Random Number Generator present (xstore)") },
    { "rng_en",  N_("Random Number Generator enabled") },
    { "ace",     N_("on-CPU crypto (xcrypt)") },
    { "ace_en",  N_("on-CPU crypto enabled") },
    { "ace2",    N_("Advanced Cryptography Engine v2") },
    { "ace2_en", N_("ACE v2 enabled") },
    { "phe",     N_("PadLock Hash Engine") },
    { "phe_en",  N_("PHE enabled") },
    { "pmm",     N_("PadLock Montgomery Multiplier") },
    { "pmm_en",  N_("PMM enabled") },
/* More extended AMD flags: CPUID level 0x80000001, ecx */
    { "lahf_lm",       N_("Load AH from Flags (LAHF) and Store AH into Flags (SAHF) in long mode") },
    { "cmp_legacy",    N_("If yes HyperThreading not valid") },
    { "svm",           N_("\"Secure virtual machine\": AMD-V") },
    { "extapic",       N_("Extended APIC space") },
    { "cr8_legacy",    N_("CR8 in 32-bit mode") },
    { "abm",           N_("Advanced Bit Manipulation") },
    { "sse4a",         N_("SSE-4A") },
    { "misalignsse",   N_("indicates if a general-protection exception (#GP) is generated when some legacy SSE instructions operate on unaligned data. Also depends on CR0 and Alignment Checking bit") },
    { "3dnowprefetch", N_("3DNow prefetch instructions") },
    { "osvw",          N_("indicates OS Visible Workaround, which allows the OS to work around processor errata.") },
    { "ibs",           N_("Instruction Based Sampling") },
    { "xop",           N_("extended AVX instructions") },
    { "skinit",        N_("SKINIT/STGI instructions") },
    { "wdt",           N_("Watchdog timer") },
    { "lwp",           N_("Light Weight Profiling") },
    { "fma4",          N_("4 operands MAC instructions") },
    { "tce",           N_("translation cache extension") },
    { "nodeid_msr",    N_("NodeId MSR") },
    { "tbm",           N_("Trailing Bit Manipulation") },
    { "topoext",       N_("Topology Extensions CPUID leafs") },
    { "perfctr_core",  N_("Core Performance Counter Extensions") },
    { "perfctr_nb",    N_("NB Performance Counter Extensions") },
    { "bpext",         N_("data breakpoint extension") },
    { "ptsc",          N_("performance time-stamp counter") },
    { "perfctr_l2",    N_("L2 Performance Counter Extensions") },
    { "mwaitx",        N_("MWAIT extension (MONITORX/MWAITX)") },
/* Auxiliary flags: Linux defined - For features scattered in various CPUID levels */
    { "cpb",           N_("AMD Core Performance Boost") },
    { "epb",           N_("IA32_ENERGY_PERF_BIAS support") },
    { "hw_pstate",     N_("AMD HW-PState") },
    { "proc_feedback", N_("AMD ProcFeedbackInterface") },
    { "intel_pt",      N_("Intel Processor Tracing") },
/* Virtualization flags: Linux defined */
    { "tpr_shadow",   N_("Intel TPR Shadow") },
    { "vnmi",         N_("Intel Virtual NMI") },
    { "flexpriority", N_("Intel FlexPriority") },
    { "ept",          N_("Intel Extended Page Table") },
    { "vpid",         N_("Intel Virtual Processor ID") },
    { "vmmcall",      N_("prefer VMMCALL to VMCALL") },
/* Intel-defined CPU features, CPUID level 0x00000007:0 (ebx) */
    { "fsgsbase",   N_("{RD/WR}{FS/GS}BASE instructions") },
    { "tsc_adjust", N_("TSC adjustment MSR") },
    { "bmi1",       N_("1st group bit manipulation extensions") },
    { "hle",        N_("Hardware Lock Elision") },
    { "avx2",       N_("AVX2 instructions") },
    { "smep",       N_("Supervisor Mode Execution Protection") },
    { "bmi2",       N_("2nd group bit manipulation extensions") },
    { "erms",       N_("Enhanced REP MOVSB/STOSB") },
    { "invpcid",    N_("Invalidate Processor Context ID") },
    { "rtm",        N_("Restricted Transactional Memory") },
    { "cqm",        N_("Cache QoS Monitoring") },
    { "mpx",        N_("Memory Protection Extension") },
    { "avx512f",    N_("AVX-512 foundation") },
    { "avx512dq",   N_("AVX-512 Double/Quad instructions") },
    { "rdseed",     N_("The RDSEED instruction") },
    { "adx",        N_("The ADCX and ADOX instructions") },
    { "smap",       N_("Supervisor Mode Access Prevention") },
    { "clflushopt", N_("CLFLUSHOPT instruction") },
    { "clwb",       N_("CLWB instruction") },
    { "avx512pf",   N_("AVX-512 Prefetch") },
    { "avx512er",   N_("AVX-512 Exponential and Reciprocal") },
    { "avx512cd",   N_("AVX-512 Conflict Detection") },
    { "sha_ni",     N_("SHA1/SHA256 Instruction Extensions") },
    { "avx512bw",   N_("AVX-512 Byte/Word instructions") },
    { "avx512vl",   N_("AVX-512 128/256 Vector Length extensions") },
/* Extended state features, CPUID level 0x0000000d:1 (eax) */
    { "xsaveopt",   N_("Optimized XSAVE") },
    { "xsavec",     N_("XSAVEC") },
    { "xgetbv1",    N_("XGETBV with ECX = 1") },
    { "xsaves",     N_("XSAVES/XRSTORS") },
/* Intel-defined CPU QoS sub-leaf, CPUID level 0x0000000F:0 (edx) */
    { "cqm_llc",    N_("LLC QoS") },
/* Intel-defined CPU QoS sub-leaf, CPUID level 0x0000000F:1 (edx) */
    { "cqm_occup_llc",  N_("LLC occupancy monitoring") },
    { "cqm_mbm_total",  N_("LLC total MBM monitoring") },
    { "cqm_mbm_local",  N_("LLC local MBM monitoring") },
/* AMD-defined CPU features, CPUID level 0x80000008 (ebx) */
    { "clzero",     N_("CLZERO instruction") },
    { "irperf",     N_("instructions retired performance counter") },
/* Thermal and Power Management leaf, CPUID level 0x00000006 (eax) */
    { "dtherm",     N_("digital thermal sensor") }, /* formerly dts */
    { "ida",        N_("Intel Dynamic Acceleration") },
    { "arat",       N_("Always Running APIC Timer") },
    { "pln",        N_("Intel Power Limit Notification") },
    { "pts",        N_("Intel Package Thermal Status") },
    { "hwp",        N_("Intel Hardware P-states") },
    { "hwp_notify", N_("HWP notification") },
    { "hwp_act_window", N_("HWP Activity Window") },
    { "hwp_epp",        N_("HWP Energy Performance Preference") },
    { "hwp_pkg_req",    N_("HWP package-level request") },
/* AMD SVM Feature Identification, CPUID level 0x8000000a (edx) */
    { "npt",           N_("AMD Nested Page Table support") },
    { "lbrv",          N_("AMD LBR Virtualization support") },
    { "svm_lock",      N_("AMD SVM locking MSR") },
    { "nrip_save",     N_("AMD SVM next_rip save") },
    { "tsc_scale",     N_("AMD TSC scaling support") },
    { "vmcb_clean",    N_("AMD VMCB clean bits support") },
    { "flushbyasid",   N_("AMD flush-by-ASID support") },
    { "decodeassists", N_("AMD Decode Assists support") },
    { "pausefilter",   N_("AMD filtered pause intercept") },
    { "pfthreshold",   N_("AMD pause filter threshold") },
    { "avic",          N_("Virtual Interrupt Controller") },
/* Intel-defined CPU features, CPUID level 0x00000007:0 (ecx) */
    { "pku",       N_("Protection Keys for Userspace") },
    { "ospke",     N_("OS Protection Keys Enable") },
/* AMD-defined CPU features, CPUID level 0x80000007 (ebx) */
    { "overflow_recov", N_("MCA overflow recovery support") },
    { "succor",         N_("uncorrectable error containment and recovery") },
    { "smca",           N_("Scalable MCA") },

/* bug workarounds */
    { "bug:f00f",         N_("Intel F00F bug")    },
    { "bug:fdiv",         N_("FPU FDIV")          },
    { "bug:coma",         N_("Cyrix 6x86 coma")   },
    { "bug:tlb_mmatch",   N_("AMD Erratum 383")   },
    { "bug:apic_c1e",     N_("AMD Erratum 400")   },
    { "bug:11ap",         N_("Bad local APIC aka 11AP")  },
    { "bug:fxsave_leak",  N_("FXSAVE leaks FOP/FIP/FOP") },
    { "bug:clflush_monitor",  N_("AAI65, CLFLUSH required before MONITOR") },
    { "bug:sysret_ss_attrs",  N_("SYSRET doesn't fix up SS attrs") },
    { "bug:espfix",       N_("IRET to 16-bit SS corrupts ESP/RSP high bits") },
    { "bug:null_seg",     N_("Nulling a selector preserves the base") },         /* see: detect_null_seg_behavior() */
    { "bug:swapgs_fence", N_("SWAPGS without input dep on GS") },
    { "bug:monitor",      N_("IPI required to wake up remote CPU") },
    { "bug:amd_e400",     N_("AMD Erratum 400") },
/* power management
 * ... from arch/x86/kernel/cpu/powerflags.h */
    { "pm:ts",            N_("temperature sensor")     },
    { "pm:fid",           N_("frequency id control")   },
    { "pm:vid",           N_("voltage id control")     },
    { "pm:ttp",           N_("thermal trip")           },
    { "pm:tm",            N_("hardware thermal control")   },
    { "pm:stc",           N_("software thermal control")   },
    { "pm:100mhzsteps",   N_("100 MHz multiplier control") },
    { "pm:hwpstate",      N_("hardware P-state control")   },
/*  { "pm:",              N_("tsc invariant mapped to constant_tsc") }, */
    { "pm:cpb",           N_("core performance boost")     },
    { "pm:eff_freq_ro",   N_("Readonly aperf/mperf")       },
    { "pm:proc_feedback", N_("processor feedback interface") },
    { "pm:acc_power",     N_("accumulated power mechanism")  },
    { NULL, NULL},
};

static char all_flags[4096] = "";

#define APPEND_FLAG(f) strcat(all_flags, f); strcat(all_flags, " ");
const char *x86_flag_list() {
    int i = 0, built = 0;
    built = strlen(all_flags);
    if (!built) {
        while(tab_flag_meaning[i].name != NULL) {
            APPEND_FLAG(tab_flag_meaning[i].name);
            i++;
        }
    }
    return all_flags;
}

const char *x86_flag_meaning(const char *flag) {
    int i = 0;
    if (flag)
    while(tab_flag_meaning[i].name != NULL) {
        if (strcmp(tab_flag_meaning[i].name, flag) == 0) {
            if (tab_flag_meaning[i].meaning != NULL)
                return _(tab_flag_meaning[i].meaning);
            else return NULL;
        }
        i++;
    }
    return NULL;
}
