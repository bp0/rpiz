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
    { "fpu",	 "Onboard FPU (floating point support)" },
    { "vme",	 "Virtual 8086 mode enhancements" },
    { "de", 	 "Debugging Extensions (CR4.DE)" },
    { "pse",	 "Page Size Extensions (4MB memory pages)" },
    { "tsc",	 "Time Stamp Counter (RDTSC)" },
    { "msr",	 "Model-Specific Registers (RDMSR, WRMSR)" },
    { "pae",	 "Physical Address Extensions (support for more than 4GB of RAM)" },
    { "mce",	 "Machine Check Exception" },
    { "cx8",	 "CMPXCHG8 instruction (64-bit compare-and-swap)" },
    { "apic",	 "Onboard APIC" },
    { "sep",	 "SYSENTER/SYSEXIT" },
    { "mtrr",	 "Memory Type Range Registers" },
    { "pge",	 "Page Global Enable (global bit in PDEs and PTEs)" },
    { "mca",	 "Machine Check Architecture" },
    { "cmov",	 "CMOV instructions (conditional move) (also FCMOV)" },
    { "pat",	 "Page Attribute Table" },
    { "pse36",	 "36-bit PSEs (huge pages)" },
    { "pn", 	 "Processor serial number" },
    { "clflush",	 "Cache Line Flush instruction" },
    { "dts",	 "Debug Store (buffer for debugging and profiling instructions)" },
    { "acpi",	 "ACPI via MSR (temperature monitoring and clock speed modulation)" },
    { "mmx",	 "Multimedia Extensions" },
    { "fxsr",	 "FXSAVE/FXRSTOR, CR4.OSFXSR" },
    { "sse",	 "Intel SSE vector instructions" },
    { "sse2",	 "SSE2" },
    { "ss", 	 "CPU self snoop" },
    { "ht", 	 "Hyper-Threading" },
    { "tm", 	 "Automatic clock control (Thermal Monitor)" },
    { "ia64",	 "Intel Itanium Architecture 64-bit (not to be confused with Intel's 64-bit x86 architecture with flag x86-64 or \"AMD64\" bit indicated by flag lm)" },
    { "pbe",	 "Pending Break Enable (PBE# pin) wakeup support" },
/* AMD-defined CPU features, CPUID level 0x80000001
 * See also Wikipedia and table 2-23 in Intel Advanced Vector Extensions Programming Reference */
    { "syscall",	 "SYSCALL (Fast System Call) and SYSRET (Return From Fast System Call)" },
    { "mp", 	 "Multiprocessing Capable." },
    { "nx", 	 "Execute Disable" },
    { "mmxext",	 "AMD MMX extensions" },
    { "fxsr_opt",	 "FXSAVE/FXRSTOR optimizations" },
    { "pdpe1gb",	 "One GB pages (allows hugepagesz=1G)" },
    { "rdtscp",	 "Read Time-Stamp Counter and Processor ID" },
    { "lm", 	 "Long Mode (x86-64: amd64, also known as Intel 64, i.e. 64-bit capable)" },
    { "3dnow",	 "3DNow! (AMD vector instructions, competing with Intel's SSE1)" },
    { "3dnowext",	 "AMD 3DNow! extensions" },
/* Transmeta-defined CPU features, CPUID level 0x80860001 */
    { "recovery",	 "CPU in recovery mode" },
    { "longrun",	 "Longrun power control" },
    { "lrti",	 "LongRun table interface" },
/* Other features, Linux-defined mapping */
    { "cxmmx",	 "Cyrix MMX extensions" },
    { "k6_mtrr",	 "AMD K6 nonstandard MTRRs" },
    { "cyrix_arr",	 "Cyrix ARRs (= MTRRs)" },
    { "centaur_mcr",	 "Centaur MCRs (= MTRRs)" },
    { "constant_tsc",	 "TSC ticks at a constant rate" },
    { "up", 	 "SMP kernel running on UP" },
    { "art",	 "Always-Running Timer" },
    { "arch_perfmon",	 "Intel Architectural PerfMon" },
    { "pebs",	 "Precise-Event Based Sampling" },
    { "bts",	 "Branch Trace Store" },
    { "rep_good",	 "rep microcode works well" },
    { "acc_power",	 "AMD accumulated power mechanism" },
    { "nopl",	 "The NOPL (0F 1F) instructions" },
    { "xtopology",	 "cpu topology enum extensions" },
    { "tsc_reliable",	 "TSC is known to be reliable" },
    { "nonstop_tsc",	 "TSC does not stop in C states" },
    { "extd_apicid",	 "has extended APICID (8 bits)" },
    { "amd_dcm",	 "multi-node processor" },
    { "aperfmperf",	 "APERFMPERF" },
    { "eagerfpu",	 "Non lazy FPU restore" },
    { "nonstop_tsc_s3",	 "TSC doesn't stop in S3 state" },
    { "mce_recovery",	 "CPU has recoverable machine checks" },
/* Intel-defined CPU features, CPUID level 0x00000001 (ecx)
 * See also Wikipedia and table 2-26 in Intel Advanced Vector Extensions Programming Reference */
    { "pni",	 "SSE-3 (“Prescott New Instructions”)" },
    { "pclmulqdq",	 "Perform a Carry-Less Multiplication of Quadword instruction — accelerator for GCM)" },
    { "dtes64",	 "64-bit Debug Store" },
    { "monitor",	 "Monitor/Mwait support (Intel SSE3 supplements)" },
    { "ds_cpl",	 "CPL Qual. Debug Store" },
    { "vmx: Hardware virtualization",	 "Intel VMX" },
    { "smx: Safer mode",	 "TXT (TPM support)" },
    { "est",	 "Enhanced SpeedStep" },
    { "tm2",	 "Thermal Monitor 2" },
    { "ssse3",	 "Supplemental SSE-3" },
    { "cid",	 "Context ID" },
    { "sdbg",	 "silicon debug" },
    { "fma",	 "Fused multiply-add" },
    { "cx16",	 "CMPXCHG16B" },
    { "xtpr",	 "Send Task Priority Messages" },
    { "pdcm",	 "Performance Capabilities" },
    { "pcid",	 "Process Context Identifiers" },
    { "dca",	 "Direct Cache Access" },
    { "sse4_1",	 "SSE-4.1" },
    { "sse4_2",	 "SSE-4.2" },
    { "x2apic",	 "x2APIC" },
    { "movbe",	 "Move Data After Swapping Bytes instruction" },
    { "popcnt",	 "Return the Count of Number of Bits Set to 1 instruction (Hamming weight, i.e. bit count)" },
    { "tsc_deadline_timer",	 "Tsc deadline timer" },
    { "aes/aes-ni",	 "Advanced Encryption Standard (New Instructions)" },
    { "xsave",	 "Save Processor Extended States: also provides XGETBY,XRSTOR,XSETBY" },
    { "avx",	 "Advanced Vector Extensions" },
    { "f16c",	 "16-bit fp conversions (CVT16)" },
    { "rdrand",	 "Read Random Number from hardware random number generator instruction" },
    { "hypervisor",	 "Running on a hypervisor" },
/* VIA/Cyrix/Centaur-defined CPU features, CPUID level 0xC0000001 */
    { "rng",	 "Random Number Generator present (xstore)" },
    { "rng_en",	 "Random Number Generator enabled" },
    { "ace",	 "on-CPU crypto (xcrypt)" },
    { "ace_en",	 "on-CPU crypto enabled" },
    { "ace2",	 "Advanced Cryptography Engine v2" },
    { "ace2_en",	 "ACE v2 enabled" },
    { "phe",	 "PadLock Hash Engine" },
    { "phe_en",	 "PHE enabled" },
    { "pmm",	 "PadLock Montgomery Multiplier" },
    { "pmm_en",	 "PMM enabled" },
/* More extended AMD flags: CPUID level 0x80000001, ecx */
    { "lahf_lm",	 "Load AH from Flags (LAHF) and Store AH into Flags (SAHF) in long mode" },
    { "cmp_legacy",	 "If yes HyperThreading not valid" },
    { "svm",	 "\"Secure virtual machine\": AMD-V" },
    { "extapic",	 "Extended APIC space" },
    { "cr8_legacy",	 "CR8 in 32-bit mode" },
    { "abm",	 "Advanced Bit Manipulation" },
    { "sse4a",	 "SSE-4A" },
    { "misalignsse",	 "indicates if a general-protection exception (#GP) is generated when some legacy SSE instructions operate on unaligned data. Also depends on CR0 and Alignment Checking bit" },
    { "3dnowprefetch",	 "3DNow prefetch instructions" },
    { "osvw",	 "indicates OS Visible Workaround, which allows the OS to work around processor errata." },
    { "ibs",	 "Instruction Based Sampling" },
    { "xop",	 "extended AVX instructions" },
    { "skinit",	 "SKINIT/STGI instructions" },
    { "wdt",	 "Watchdog timer" },
    { "lwp",	 "Light Weight Profiling" },
    { "fma4",	 "4 operands MAC instructions" },
    { "tce",	 "translation cache extension" },
    { "nodeid_msr",	 "NodeId MSR" },
    { "tbm",	 "Trailing Bit Manipulation" },
    { "topoext",	 "Topology Extensions CPUID leafs" },
    { "perfctr_core",	 "Core Performance Counter Extensions" },
    { "perfctr_nb",	 "NB Performance Counter Extensions" },
    { "bpext",	 "data breakpoint extension" },
    { "ptsc",	 "performance time-stamp counter" },
    { "perfctr_l2",	 "L2 Performance Counter Extensions" },
    { "mwaitx",	 "MWAIT extension (MONITORX/MWAITX)" },
/* Auxiliary flags: Linux defined - For features scattered in various CPUID levels */
    { "cpb",	 "AMD Core Performance Boost" },
    { "epb",	 "IA32_ENERGY_PERF_BIAS support" },
    { "hw_pstate",	 "AMD HW-PState" },
    { "proc_feedback",	 "AMD ProcFeedbackInterface" },
    { "intel_pt",	 "Intel Processor Tracing" },
/* Virtualization flags: Linux defined */
    { "tpr_shadow",	 "Intel TPR Shadow" },
    { "vnmi",	 "Intel Virtual NMI" },
    { "flexpriority",	 "Intel FlexPriority" },
    { "ept",	 "Intel Extended Page Table" },
    { "vpid",	 "Intel Virtual Processor ID" },
    { "vmmcall",	 "prefer VMMCALL to VMCALL" },
/* Intel-defined CPU features, CPUID level 0x00000007:0 (ebx) */
    { "fsgsbase",	 "{RD/WR}{FS/GS}BASE instructions" },
    { "tsc_adjust",	 "TSC adjustment MSR" },
    { "bmi1",	 "1st group bit manipulation extensions" },
    { "hle",	 "Hardware Lock Elision" },
    { "avx2",	 "AVX2 instructions" },
    { "smep",	 "Supervisor Mode Execution Protection" },
    { "bmi2",	 "2nd group bit manipulation extensions" },
    { "erms",	 "Enhanced REP MOVSB/STOSB" },
    { "invpcid",	 "Invalidate Processor Context ID" },
    { "rtm",	 "Restricted Transactional Memory" },
    { "cqm",	 "Cache QoS Monitoring" },
    { "mpx",	 "Memory Protection Extension" },
    { "avx512f",	 "AVX-512 foundation" },
    { "avx512dq",	 "AVX-512 Double/Quad instructions" },
    { "rdseed",	 "The RDSEED instruction" },
    { "adx",	 "The ADCX and ADOX instructions" },
    { "smap",	 "Supervisor Mode Access Prevention" },
    { "clflushopt",	 "CLFLUSHOPT instruction" },
    { "clwb",	 "CLWB instruction" },
    { "avx512pf",	 "AVX-512 Prefetch" },
    { "avx512er",	 "AVX-512 Exponential and Reciprocal" },
    { "avx512cd",	 "AVX-512 Conflict Detection" },
    { "sha_ni",	 "SHA1/SHA256 Instruction Extensions" },
    { "avx512bw",	 "AVX-512 Byte/Word instructions" },
    { "avx512vl",	 "AVX-512 128/256 Vector Length extensions" },
/* Extended state features, CPUID level 0x0000000d:1 (eax) */
    { "xsaveopt",	 "Optimized XSAVE" },
    { "xsavec",	 "XSAVEC" },
    { "xgetbv1",	 "XGETBV with ECX = 1" },
    { "xsaves",	 "XSAVES/XRSTORS" },
/* Intel-defined CPU QoS sub-leaf, CPUID level 0x0000000F:0 (edx) */
    { "cqm_llc",	 "LLC QoS" },
/* Intel-defined CPU QoS sub-leaf, CPUID level 0x0000000F:1 (edx) */
    { "cqm_occup_llc",	 "LLC occupancy monitoring" },
    { "cqm_mbm_total",	 "LLC total MBM monitoring" },
    { "cqm_mbm_local",	 "LLC local MBM monitoring" },
/* AMD-defined CPU features, CPUID level 0x80000008 (ebx) */
    { "clzero",	 "CLZERO instruction" },
    { "irperf",	 "instructions retired performance counter" },
/* Thermal and Power Management leaf, CPUID level 0x00000006 (eax) */
    { "dtherm (formerly dts)",	 "digital thermal sensor" },
    { "ida",	 "Intel Dynamic Acceleration" },
    { "arat",	 "Always Running APIC Timer" },
    { "pln",	 "Intel Power Limit Notification" },
    { "pts",	 "Intel Package Thermal Status" },
    { "hwp",	 "Intel Hardware P-states" },
    { "hwp_notify",	 "HWP notification" },
    { "hwp_act_window",	 "HWP Activity Window" },
    { "hwp_epp",	 "HWP Energy Performance Preference" },
    { "hwp_pkg_req",	 "HWP package-level request" },
/* AMD SVM Feature Identification, CPUID level 0x8000000a (edx) */
    { "npt",	 "AMD Nested Page Table support" },
    { "lbrv",	 "AMD LBR Virtualization support" },
    { "svm_lock",	 "AMD SVM locking MSR" },
    { "nrip_save",	 "AMD SVM next_rip save" },
    { "tsc_scale",	 "AMD TSC scaling support" },
    { "vmcb_clean",	 "AMD VMCB clean bits support" },
    { "flushbyasid",	 "AMD flush-by-ASID support" },
    { "decodeassists",	 "AMD Decode Assists support" },
    { "pausefilter",	 "AMD filtered pause intercept" },
    { "pfthreshold",	 "AMD pause filter threshold" },
    { "avic",	 "Virtual Interrupt Controller" },
/* Intel-defined CPU features, CPUID level 0x00000007:0 (ecx) */
    { "pku",	 "Protection Keys for Userspace" },
    { "ospke",	 "OS Protection Keys Enable" },
/* AMD-defined CPU features, CPUID level 0x80000007 (ebx) */
    { "overflow_recov",	 "MCA overflow recovery support" },
    { "succor",	 "uncorrectable error containment and recovery" },
    { "smca",	 "Scalable MCA" },

/* bug workarounds */
    { "bug:f00f",        "Intel F00F bug"    },
    { "bug:fdiv",        "FPU FDIV"          },
    { "bug:coma",        "Cyrix 6x86 coma"   },
    { "bug:tlb_mmatch",  "AMD Erratum 383"   },
    { "bug:apic_c1e",    "AMD Erratum 400"   },
    { "bug:11ap",        "Bad local APIC aka 11AP"  },
    { "bug:fxsave_leak", "FXSAVE leaks FOP/FIP/FOP" },
    { "bug:clflush_monitor",  "AAI65, CLFLUSH required before MONITOR" },
    { "bug:sysret_ss_attrs",  "SYSRET doesn't fix up SS attrs" },
    { "bug:espfix",      "IRET to 16-bit SS corrupts ESP/RSP high bits" },
    { "bug:null_seg",    "Nulling a selector preserves the base" },         /* see: detect_null_seg_behavior() */
    { "bug:swapgs_fence","SWAPGS without input dep on GS" },
    { "bug:monitor",     "IPI required to wake up remote CPU" },
    { "bug:amd_e400",    "AMD Erratum 400" },
/* power management
 * ... from arch/x86/kernel/cpu/powerflags.h */
    { "pm:ts",            "temperature sensor"     },
    { "pm:fid",           "frequency id control"   },
    { "pm:vid",           "voltage id control"     },
    { "pm:ttp",           "thermal trip"           },
    { "pm:tm",            "hardware thermal control"   },
    { "pm:stc",           "software thermal control"   },
    { "pm:100mhzsteps",   "100 MHz multiplier control" },
    { "pm:hwpstate",      "hardware P-state control"   },
/*  { "pm:",              "tsc invariant mapped to constant_tsc" }, */
    { "pm:cpb",           "core performance boost"     },
    { "pm:eff_freq_ro",   "Readonly aperf/mperf"       },
    { "pm:proc_feedback", "processor feedback interface" },
    { "pm:acc_power",     "accumulated power mechanism"  },
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
        if (strcmp(tab_flag_meaning[i].name, flag) == 0)
            return tab_flag_meaning[i].meaning;
        i++;
    }
    return NULL;
}
