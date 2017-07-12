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

#ifndef C_
#define C_(Ctx, String) String
#endif
#ifndef NC_
#define NC_(Ctx, String) String
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
    { "fpu",     NC_("x86-flag", "Onboard FPU (floating point support)") },
    { "vme",     NC_("x86-flag", "Virtual 8086 mode enhancements") },
    { "de",      NC_("x86-flag", "Debugging Extensions (CR4.DE)") },
    { "pse",     NC_("x86-flag", "Page Size Extensions (4MB memory pages)") },
    { "tsc",     NC_("x86-flag", "Time Stamp Counter (RDTSC)") },
    { "msr",     NC_("x86-flag", "Model-Specific Registers (RDMSR, WRMSR)") },
    { "pae",     NC_("x86-flag", "Physical Address Extensions (support for more than 4GB of RAM)") },
    { "mce",     NC_("x86-flag", "Machine Check Exception") },
    { "cx8",     NC_("x86-flag", "CMPXCHG8 instruction (64-bit compare-and-swap)") },
    { "apic",    NC_("x86-flag", "Onboard APIC") },
    { "sep",     NC_("x86-flag", "SYSENTER/SYSEXIT") },
    { "mtrr",    NC_("x86-flag", "Memory Type Range Registers") },
    { "pge",     NC_("x86-flag", "Page Global Enable (global bit in PDEs and PTEs)") },
    { "mca",     NC_("x86-flag", "Machine Check Architecture") },
    { "cmov",    NC_("x86-flag", "CMOV instructions (conditional move) (also FCMOV)") },
    { "pat",     NC_("x86-flag", "Page Attribute Table") },
    { "pse36",   NC_("x86-flag", "36-bit PSEs (huge pages)") },
    { "pn",      NC_("x86-flag", "Processor serial number") },
    { "clflush", NC_("x86-flag", "Cache Line Flush instruction") },
    { "dts",     NC_("x86-flag", "Debug Store (buffer for debugging and profiling instructions), or alternately: digital thermal sensor") },
    { "acpi",    NC_("x86-flag", "ACPI via MSR (temperature monitoring and clock speed modulation)") },
    { "mmx",     NC_("x86-flag", "Multimedia Extensions") },
    { "fxsr",    NC_("x86-flag", "FXSAVE/FXRSTOR, CR4.OSFXSR") },
    { "sse",     NC_("x86-flag", "Intel SSE vector instructions") },
    { "sse2",    NC_("x86-flag", "SSE2") },
    { "ss",      NC_("x86-flag", "CPU self snoop") },
    { "ht",      NC_("x86-flag", "Hyper-Threading") },
    { "tm",      NC_("x86-flag", "Automatic clock control (Thermal Monitor)") },
    { "ia64",    NC_("x86-flag", "Intel Itanium Architecture 64-bit (not to be confused with Intel's 64-bit x86 architecture with flag x86-64 or \"AMD64\" bit indicated by flag lm)") },
    { "pbe",     NC_("x86-flag", "Pending Break Enable (PBE# pin) wakeup support") },
/* AMD-defined CPU features, CPUID level 0x80000001
 * See also Wikipedia and table 2-23 in Intel Advanced Vector Extensions Programming Reference */
    { "syscall",  NC_("x86-flag", "SYSCALL (Fast System Call) and SYSRET (Return From Fast System Call)") },
    { "mp",       NC_("x86-flag", "Multiprocessing Capable.") },
    { "nx",       NC_("x86-flag", "Execute Disable") },
    { "mmxext",   NC_("x86-flag", "AMD MMX extensions") },
    { "fxsr_opt", NC_("x86-flag", "FXSAVE/FXRSTOR optimizations") },
    { "pdpe1gb",  NC_("x86-flag", "One GB pages (allows hugepagesz=1G)") },
    { "rdtscp",   NC_("x86-flag", "Read Time-Stamp Counter and Processor ID") },
    { "lm",       NC_("x86-flag", "Long Mode (x86-64: amd64, also known as Intel 64, i.e. 64-bit capable)") },
    { "3dnow",    NC_("x86-flag", "3DNow! (AMD vector instructions, competing with Intel's SSE1)") },
    { "3dnowext", NC_("x86-flag", "AMD 3DNow! extensions") },
/* Transmeta-defined CPU features, CPUID level 0x80860001 */
    { "recovery", NC_("x86-flag", "CPU in recovery mode") },
    { "longrun",  NC_("x86-flag", "Longrun power control") },
    { "lrti",     NC_("x86-flag", "LongRun table interface") },
/* Other features, Linux-defined mapping */
    { "cxmmx",        NC_("x86-flag", "Cyrix MMX extensions") },
    { "k6_mtrr",      NC_("x86-flag", "AMD K6 nonstandard MTRRs") },
    { "cyrix_arr",    NC_("x86-flag", "Cyrix ARRs (= MTRRs)") },
    { "centaur_mcr",  NC_("x86-flag", "Centaur MCRs (= MTRRs)") },
    { "constant_tsc", NC_("x86-flag", "TSC ticks at a constant rate") },
    { "up",           NC_("x86-flag", "SMP kernel running on UP") },
    { "art",          NC_("x86-flag", "Always-Running Timer") },
    { "arch_perfmon", NC_("x86-flag", "Intel Architectural PerfMon") },
    { "pebs",         NC_("x86-flag", "Precise-Event Based Sampling") },
    { "bts",          NC_("x86-flag", "Branch Trace Store") },
    { "rep_good",     NC_("x86-flag", "rep microcode works well") },
    { "acc_power",    NC_("x86-flag", "AMD accumulated power mechanism") },
    { "nopl",         NC_("x86-flag", "The NOPL (0F 1F) instructions") },
    { "xtopology",    NC_("x86-flag", "cpu topology enum extensions") },
    { "tsc_reliable", NC_("x86-flag", "TSC is known to be reliable") },
    { "nonstop_tsc",  NC_("x86-flag", "TSC does not stop in C states") },
    { "extd_apicid",  NC_("x86-flag", "has extended APICID (8 bits)") },
    { "amd_dcm",      NC_("x86-flag", "multi-node processor") },
    { "aperfmperf",   NC_("x86-flag", "APERFMPERF") },
    { "eagerfpu",     NC_("x86-flag", "Non lazy FPU restore") },
    { "nonstop_tsc_s3", NC_("x86-flag", "TSC doesn't stop in S3 state") },
    { "mce_recovery",   NC_("x86-flag", "CPU has recoverable machine checks") },
/* Intel-defined CPU features, CPUID level 0x00000001 (ecx)
 * See also Wikipedia and table 2-26 in Intel Advanced Vector Extensions Programming Reference */
    { "pni",       NC_("x86-flag", "SSE-3 (\"Prescott New Instructions\")") },
    { "pclmulqdq", NC_("x86-flag", "Perform a Carry-Less Multiplication of Quadword instruction - accelerator for GCM)") },
    { "dtes64",    NC_("x86-flag", "64-bit Debug Store") },
    { "monitor",   NC_("x86-flag", "Monitor/Mwait support (Intel SSE3 supplements)") },
    { "ds_cpl",    NC_("x86-flag", "CPL Qual. Debug Store") },
    { "vmx",       NC_("x86-flag", "Hardware virtualization, Intel VMX") },
    { "smx",       NC_("x86-flag", "Safer mode TXT (TPM support)") },
    { "est",       NC_("x86-flag", "Enhanced SpeedStep") },
    { "tm2",       NC_("x86-flag", "Thermal Monitor 2") },
    { "ssse3",     NC_("x86-flag", "Supplemental SSE-3") },
    { "cid",       NC_("x86-flag", "Context ID") },
    { "sdbg",      NC_("x86-flag", "silicon debug") },
    { "fma",       NC_("x86-flag", "Fused multiply-add") },
    { "cx16",      NC_("x86-flag", "CMPXCHG16B") },
    { "xtpr",      NC_("x86-flag", "Send Task Priority Messages") },
    { "pdcm",      NC_("x86-flag", "Performance Capabilities") },
    { "pcid",      NC_("x86-flag", "Process Context Identifiers") },
    { "dca",       NC_("x86-flag", "Direct Cache Access") },
    { "sse4_1",    NC_("x86-flag", "SSE-4.1") },
    { "sse4_2",    NC_("x86-flag", "SSE-4.2") },
    { "x2apic",    NC_("x86-flag", "x2APIC") },
    { "movbe",     NC_("x86-flag", "Move Data After Swapping Bytes instruction") },
    { "popcnt",    NC_("x86-flag", "Return the Count of Number of Bits Set to 1 instruction (Hamming weight, i.e. bit count)") },
    { "tsc_deadline_timer", NC_("x86-flag", "Tsc deadline timer") },
    { "aes/aes-ni",  NC_("x86-flag", "Advanced Encryption Standard (New Instructions)") },
    { "xsave",       NC_("x86-flag", "Save Processor Extended States: also provides XGETBY,XRSTOR,XSETBY") },
    { "avx",         NC_("x86-flag", "Advanced Vector Extensions") },
    { "f16c",        NC_("x86-flag", "16-bit fp conversions (CVT16)") },
    { "rdrand",      NC_("x86-flag", "Read Random Number from hardware random number generator instruction") },
    { "hypervisor",  NC_("x86-flag", "Running on a hypervisor") },
/* VIA/Cyrix/Centaur-defined CPU features, CPUID level 0xC0000001 */
    { "rng",     NC_("x86-flag", "Random Number Generator present (xstore)") },
    { "rng_en",  NC_("x86-flag", "Random Number Generator enabled") },
    { "ace",     NC_("x86-flag", "on-CPU crypto (xcrypt)") },
    { "ace_en",  NC_("x86-flag", "on-CPU crypto enabled") },
    { "ace2",    NC_("x86-flag", "Advanced Cryptography Engine v2") },
    { "ace2_en", NC_("x86-flag", "ACE v2 enabled") },
    { "phe",     NC_("x86-flag", "PadLock Hash Engine") },
    { "phe_en",  NC_("x86-flag", "PHE enabled") },
    { "pmm",     NC_("x86-flag", "PadLock Montgomery Multiplier") },
    { "pmm_en",  NC_("x86-flag", "PMM enabled") },
/* More extended AMD flags: CPUID level 0x80000001, ecx */
    { "lahf_lm",       NC_("x86-flag", "Load AH from Flags (LAHF) and Store AH into Flags (SAHF) in long mode") },
    { "cmp_legacy",    NC_("x86-flag", "If yes HyperThreading not valid") },
    { "svm",           NC_("x86-flag", "\"Secure virtual machine\": AMD-V") },
    { "extapic",       NC_("x86-flag", "Extended APIC space") },
    { "cr8_legacy",    NC_("x86-flag", "CR8 in 32-bit mode") },
    { "abm",           NC_("x86-flag", "Advanced Bit Manipulation") },
    { "sse4a",         NC_("x86-flag", "SSE-4A") },
    { "misalignsse",   NC_("x86-flag", "indicates if a general-protection exception (#GP) is generated when some legacy SSE instructions operate on unaligned data. Also depends on CR0 and Alignment Checking bit") },
    { "3dnowprefetch", NC_("x86-flag", "3DNow prefetch instructions") },
    { "osvw",          NC_("x86-flag", "indicates OS Visible Workaround, which allows the OS to work around processor errata.") },
    { "ibs",           NC_("x86-flag", "Instruction Based Sampling") },
    { "xop",           NC_("x86-flag", "extended AVX instructions") },
    { "skinit",        NC_("x86-flag", "SKINIT/STGI instructions") },
    { "wdt",           NC_("x86-flag", "Watchdog timer") },
    { "lwp",           NC_("x86-flag", "Light Weight Profiling") },
    { "fma4",          NC_("x86-flag", "4 operands MAC instructions") },
    { "tce",           NC_("x86-flag", "translation cache extension") },
    { "nodeid_msr",    NC_("x86-flag", "NodeId MSR") },
    { "tbm",           NC_("x86-flag", "Trailing Bit Manipulation") },
    { "topoext",       NC_("x86-flag", "Topology Extensions CPUID leafs") },
    { "perfctr_core",  NC_("x86-flag", "Core Performance Counter Extensions") },
    { "perfctr_nb",    NC_("x86-flag", "NB Performance Counter Extensions") },
    { "bpext",         NC_("x86-flag", "data breakpoint extension") },
    { "ptsc",          NC_("x86-flag", "performance time-stamp counter") },
    { "perfctr_l2",    NC_("x86-flag", "L2 Performance Counter Extensions") },
    { "mwaitx",        NC_("x86-flag", "MWAIT extension (MONITORX/MWAITX)") },
/* Auxiliary flags: Linux defined - For features scattered in various CPUID levels */
    { "cpb",           NC_("x86-flag", "AMD Core Performance Boost") },
    { "epb",           NC_("x86-flag", "IA32_ENERGY_PERF_BIAS support") },
    { "hw_pstate",     NC_("x86-flag", "AMD HW-PState") },
    { "proc_feedback", NC_("x86-flag", "AMD ProcFeedbackInterface") },
    { "intel_pt",      NC_("x86-flag", "Intel Processor Tracing") },
/* Virtualization flags: Linux defined */
    { "tpr_shadow",   NC_("x86-flag", "Intel TPR Shadow") },
    { "vnmi",         NC_("x86-flag", "Intel Virtual NMI") },
    { "flexpriority", NC_("x86-flag", "Intel FlexPriority") },
    { "ept",          NC_("x86-flag", "Intel Extended Page Table") },
    { "vpid",         NC_("x86-flag", "Intel Virtual Processor ID") },
    { "vmmcall",      NC_("x86-flag", "prefer VMMCALL to VMCALL") },
/* Intel-defined CPU features, CPUID level 0x00000007:0 (ebx) */
    { "fsgsbase",   NC_("x86-flag", "{RD/WR}{FS/GS}BASE instructions") },
    { "tsc_adjust", NC_("x86-flag", "TSC adjustment MSR") },
    { "bmi1",       NC_("x86-flag", "1st group bit manipulation extensions") },
    { "hle",        NC_("x86-flag", "Hardware Lock Elision") },
    { "avx2",       NC_("x86-flag", "AVX2 instructions") },
    { "smep",       NC_("x86-flag", "Supervisor Mode Execution Protection") },
    { "bmi2",       NC_("x86-flag", "2nd group bit manipulation extensions") },
    { "erms",       NC_("x86-flag", "Enhanced REP MOVSB/STOSB") },
    { "invpcid",    NC_("x86-flag", "Invalidate Processor Context ID") },
    { "rtm",        NC_("x86-flag", "Restricted Transactional Memory") },
    { "cqm",        NC_("x86-flag", "Cache QoS Monitoring") },
    { "mpx",        NC_("x86-flag", "Memory Protection Extension") },
    { "avx512f",    NC_("x86-flag", "AVX-512 foundation") },
    { "avx512dq",   NC_("x86-flag", "AVX-512 Double/Quad instructions") },
    { "rdseed",     NC_("x86-flag", "The RDSEED instruction") },
    { "adx",        NC_("x86-flag", "The ADCX and ADOX instructions") },
    { "smap",       NC_("x86-flag", "Supervisor Mode Access Prevention") },
    { "clflushopt", NC_("x86-flag", "CLFLUSHOPT instruction") },
    { "clwb",       NC_("x86-flag", "CLWB instruction") },
    { "avx512pf",   NC_("x86-flag", "AVX-512 Prefetch") },
    { "avx512er",   NC_("x86-flag", "AVX-512 Exponential and Reciprocal") },
    { "avx512cd",   NC_("x86-flag", "AVX-512 Conflict Detection") },
    { "sha_ni",     NC_("x86-flag", "SHA1/SHA256 Instruction Extensions") },
    { "avx512bw",   NC_("x86-flag", "AVX-512 Byte/Word instructions") },
    { "avx512vl",   NC_("x86-flag", "AVX-512 128/256 Vector Length extensions") },
/* Extended state features, CPUID level 0x0000000d:1 (eax) */
    { "xsaveopt",   NC_("x86-flag", "Optimized XSAVE") },
    { "xsavec",     NC_("x86-flag", "XSAVEC") },
    { "xgetbv1",    NC_("x86-flag", "XGETBV with ECX = 1") },
    { "xsaves",     NC_("x86-flag", "XSAVES/XRSTORS") },
/* Intel-defined CPU QoS sub-leaf, CPUID level 0x0000000F:0 (edx) */
    { "cqm_llc",    NC_("x86-flag", "LLC QoS") },
/* Intel-defined CPU QoS sub-leaf, CPUID level 0x0000000F:1 (edx) */
    { "cqm_occup_llc",  NC_("x86-flag", "LLC occupancy monitoring") },
    { "cqm_mbm_total",  NC_("x86-flag", "LLC total MBM monitoring") },
    { "cqm_mbm_local",  NC_("x86-flag", "LLC local MBM monitoring") },
/* AMD-defined CPU features, CPUID level 0x80000008 (ebx) */
    { "clzero",         NC_("x86-flag", "CLZERO instruction") },
    { "irperf",         NC_("x86-flag", "instructions retired performance counter") },
/* Thermal and Power Management leaf, CPUID level 0x00000006 (eax) */
    { "dtherm",         NC_("x86-flag", "digital thermal sensor") }, /* formerly dts */
    { "ida",            NC_("x86-flag", "Intel Dynamic Acceleration") },
    { "arat",           NC_("x86-flag", "Always Running APIC Timer") },
    { "pln",            NC_("x86-flag", "Intel Power Limit Notification") },
    { "pts",            NC_("x86-flag", "Intel Package Thermal Status") },
    { "hwp",            NC_("x86-flag", "Intel Hardware P-states") },
    { "hwp_notify",     NC_("x86-flag", "HWP notification") },
    { "hwp_act_window", NC_("x86-flag", "HWP Activity Window") },
    { "hwp_epp",        NC_("x86-flag", "HWP Energy Performance Preference") },
    { "hwp_pkg_req",    NC_("x86-flag", "HWP package-level request") },
/* AMD SVM Feature Identification, CPUID level 0x8000000a (edx) */
    { "npt",            NC_("x86-flag", "AMD Nested Page Table support") },
    { "lbrv",           NC_("x86-flag", "AMD LBR Virtualization support") },
    { "svm_lock",       NC_("x86-flag", "AMD SVM locking MSR") },
    { "nrip_save",      NC_("x86-flag", "AMD SVM next_rip save") },
    { "tsc_scale",      NC_("x86-flag", "AMD TSC scaling support") },
    { "vmcb_clean",     NC_("x86-flag", "AMD VMCB clean bits support") },
    { "flushbyasid",    NC_("x86-flag", "AMD flush-by-ASID support") },
    { "decodeassists",  NC_("x86-flag", "AMD Decode Assists support") },
    { "pausefilter",    NC_("x86-flag", "AMD filtered pause intercept") },
    { "pfthreshold",    NC_("x86-flag", "AMD pause filter threshold") },
    { "avic",           NC_("x86-flag", "Virtual Interrupt Controller") },
/* Intel-defined CPU features, CPUID level 0x00000007:0 (ecx) */
    { "pku",            NC_("x86-flag", "Protection Keys for Userspace") },
    { "ospke",          NC_("x86-flag", "OS Protection Keys Enable") },
/* AMD-defined CPU features, CPUID level 0x80000007 (ebx) */
    { "overflow_recov", NC_("x86-flag", "MCA overflow recovery support") },
    { "succor",         NC_("x86-flag", "uncorrectable error containment and recovery") },
    { "smca",           NC_("x86-flag", "Scalable MCA") },

/* bug workarounds */
    { "bug:f00f",       NC_("x86-flag", "Intel F00F bug")    },
    { "bug:fdiv",       NC_("x86-flag", "FPU FDIV")          },
    { "bug:coma",       NC_("x86-flag", "Cyrix 6x86 coma")   },
    { "bug:tlb_mmatch", NC_("x86-flag", "AMD Erratum 383")   },
    { "bug:apic_c1e",   NC_("x86-flag", "AMD Erratum 400")   },
    { "bug:11ap",       NC_("x86-flag", "Bad local APIC aka 11AP")  },
    { "bug:fxsave_leak",      NC_("x86-flag", "FXSAVE leaks FOP/FIP/FOP") },
    { "bug:clflush_monitor",  NC_("x86-flag", "AAI65, CLFLUSH required before MONITOR") },
    { "bug:sysret_ss_attrs",  NC_("x86-flag", "SYSRET doesn't fix up SS attrs") },
    { "bug:espfix",       NC_("x86-flag", "IRET to 16-bit SS corrupts ESP/RSP high bits") },
    { "bug:null_seg",     NC_("x86-flag", "Nulling a selector preserves the base") },         /* see: detect_null_seg_behavior() */
    { "bug:swapgs_fence", NC_("x86-flag", "SWAPGS without input dep on GS") },
    { "bug:monitor",      NC_("x86-flag", "IPI required to wake up remote CPU") },
    { "bug:amd_e400",     NC_("x86-flag", "AMD Erratum 400") },
/* power management
 * ... from arch/x86/kernel/cpu/powerflags.h */
    { "pm:ts",            NC_("x86-flag", "temperature sensor")     },
    { "pm:fid",           NC_("x86-flag", "frequency id control")   },
    { "pm:vid",           NC_("x86-flag", "voltage id control")     },
    { "pm:ttp",           NC_("x86-flag", "thermal trip")           },
    { "pm:tm",            NC_("x86-flag", "hardware thermal control")   },
    { "pm:stc",           NC_("x86-flag", "software thermal control")   },
    { "pm:100mhzsteps",   NC_("x86-flag", "100 MHz multiplier control") },
    { "pm:hwpstate",      NC_("x86-flag", "hardware P-state control")   },
/*  { "pm:",              NC_("x86-flag", "tsc invariant mapped to constant_tsc") }, */
    { "pm:cpb",           NC_("x86-flag", "core performance boost")     },
    { "pm:eff_freq_ro",   NC_("x86-flag", "Readonly aperf/mperf")       },
    { "pm:proc_feedback", NC_("x86-flag", "processor feedback interface") },
    { "pm:acc_power",     NC_("x86-flag", "accumulated power mechanism")  },
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
                return C_("x86-flag", tab_flag_meaning[i].meaning);
            else return NULL;
        }
        i++;
    }
    return NULL;
}
