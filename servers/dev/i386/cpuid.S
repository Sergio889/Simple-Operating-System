.set EOD, -1 # end of definitions mark

.globl low_cpuid
.code32
.text
low_cpuid:
	pushl $eol
	call low_putstr
	addl $4, %esp
	pushl $cpuinfo_str
	call low_putstr
	addl $4, %esp
	pushl $eol
	call low_putstr
	addl $4, %esp
	pushl $cute_str
	call low_putstr
	addl $4, %esp
	xorl %eax, %eax # first call to cpuid
	cpuid # very first call
	pushl %eax # number of basic leafs returned at %eax
	movl %ecx, str_buf3 # string 2
	pushl $str_buf3
	movl %edx, str_buf2
	pushl $str_buf2
	movl %ebx, str_buf4 # string 1
	pushl $str_buf4
	pushl $cpu_str # string 0
	movb $4, loop_var # loop times
	jmp 2f # start printing
1:
	movb %cl, loop_var
	addl $4, %esp # next string
2:
	call low_putstr # print string
	xorl %ecx, %ecx # might have changed at low_putstr
	movb loop_var, %cl # load loop variable
	loop 1b # loop
	addl $4, %esp # restore stack
	pushl $eol
	call low_putstr
	addl $4, %esp
	popl %ecx # restore number of basic leafs
intel_or_amd:
	pushl $2
	pushl $str_buf4 # Genu or Auth
	pushl $intel_str # Genu
	call strncmp
	addl $12, %esp # restore stack
	testl %eax, %eax # is it Intel?
	jz intel_it_is
	movl $amd_shf_vars, %edi # shift control
	movl $amd_msk_vars, %ebx # mask bit control
	movl $amd_str_ord, %edx
	movl $amd_loop_small, %ebp
	movl $amd_print_or_feature_set, print_or_feature_set_var
	movl $amd_calls, calls_vars
	jmp 1f
intel_it_is:
	movl $1, oh_yeah_it_is_intel
	movl $intel_shf_vars, %edi # shift control
	movl $intel_msk_vars, %ebx # mask bit control
	movl $intel_str_ord, %edx
	movl $intel_loop_small, %ebp
	movl $intel_print_or_feature_set, print_or_feature_set_var
1:
	movl $2, %ecx # XXX TEMP
	xorl %eax, %eax # clear to start a cycle
next_call:
	pushl %ecx # save loops left
	movl calls_vars, %eax
	movl (%eax), %eax
	addl $4, calls_vars
	pusha
	cpuid # call
	movl $intel_defs, %esi # definitions control
	movl %eax, (%esi)
	movl %ebx, 4(%esi)
	movl %ecx, 8(%esi)
	movl %edx, 12(%esi)
	movl $EOD, 16(%esi) # end of definitions for call
	popa
	movl $intel_defs, %esi # definitions control
	movl $4, %ecx # big loop times
loop_big:
	pushl %ecx # save loops
	movl (%ebp), %ecx
	addl $4, %ebp
loop_small:
	pushl %ecx # save loops
	movl (%esi), %eax # work with %eax
	movb (%edi), %cl # shift number of times
	incl %edi # next shift number
	shrl %cl, %eax # shift
	andl (%ebx), %eax # mask
	addl $4, %ebx # next mask
	pushl %eax # save result
	movl print_or_feature_set_var, %eax # print or feature set
	movl (%eax), %eax
	testl %eax, %eax # is it print?
	jnz feature_set # no; feature set
	movl is_feature, %eax # was previous a feature?
	testl %eax, %eax
	jnz it_was # yes
	jmp print
it_was:
	pusha
	pushl $endfeatures_str
	call low_putstr
	addl $4, %esp
	pushl $eol
	call low_putstr
	addl $4, %esp
	popa
	movl $0, is_feature
	pusha
	pushl $eol
	call low_putstr
	addl $4, %esp
	popa
	jmp print
feature_set:
	movl is_feature, %eax
	testl %eax, %eax # continuation of features?
	jz 1f # no
	jmp print # yes
1:
	pusha
	pushl $eol
	call low_putstr
	addl $4, %esp
	pushl $features_str
	call low_putstr
	addl $4, %esp
	popa
	movl $1, is_feature
print:
	movl is_feature, %eax
	testl %eax, %eax
	jnz feature.1
	popl %eax
	jmp available
feature.1:
	popl %eax # restore result
	testl %eax, %eax # is the feature available?
	jnz available # yes
	pusha
	pushl $not_feature
	call low_putstr
	addl $4, %esp
	popa
available:
	movl %eax, hex_num # number to convert
	pusha
	pushl (%edx) # current string pointer
	call low_putstr
	addl $4, %esp
	popa
	addl $4, %edx # next string
	movl is_feature, %eax
	testl %eax, %eax
	jz it_is_print
	pusha
	pushl $comma_str
	call low_putstr
	addl $4, %esp
	popa
	jmp 3f
it_is_print:
	pusha
	pushl $4
	pushl $1 # number of words argument
	pushl $str_buf # buffer argument
	pushl $hex_num # memory address argument
	call memdump32 # convert to ascii
	addl $16, %esp # restore stack
	popa
	pusha
	pushl $str_buf # print string
	call low_putstr
	addl $4, %esp
	popa
	pusha
	movl $20, %ecx
	movl $0, %eax
	movl $str_buf, %edi
	rep
	stosb
	popa
	pushl %eax # save
	movl is_feature, %eax
	testl %eax, %eax # was it feature?
	jnz 2f # yes
	pusha
	pushl $eol
	call low_putstr
	addl $4, %esp
	popa
2:
	popl %eax
3:
	addl $4, print_or_feature_set_var
	popl %ecx
	decl %ecx
	testl %ecx, %ecx
	jnz loop_small # next register
	popl %ecx
	addl $4, %esi
	decl %ecx
	testl %ecx, %ecx
	jnz loop_big # next register
	popl %ecx
	decl %ecx
	testl %ecx, %ecx
	jnz next_call
	movl is_feature, %eax
	testl %eax, %eax
	jz 1f
	pushl $endfeatures_str
	call low_putstr
	addl $4, %esp
1:
	pushl $eol
	call low_putstr
	addl $4, %esp
	pushl $cute_str
	call low_putstr
	addl $4, %esp
	ret

.data
/* %eax = 0x80000001 */
/* %ecx */
tbm_str:
	.asciz "TBM"
fma4_str:
	.asciz "FMA4"
lwp_str:
	.asciz "LWP"
wdt_str:
	.asciz "WDT"
xop_str:
	.asciz "XOP"
ibs_str:
	.asciz "IBS"
osvw_str:
	.asciz "OSVW"
nowpre_str:
	.asciz "PREFETCH"
misall_str:
	.asciz "MisallignedSSE"
sse4a_str:
	.asciz "SSE4a"
abm_str:
	.asciz "ABM"
cr8_str:
	.asciz "CR8"
extapic_str:
	.asciz "ExtAPIC"
svm_str:
	.asciz "SVM"
cmpleg_str:
	.asciz "CMPLEG"
lasahf_str:
	.asciz "LAHF/SAHF"
/* %edx */
now_str:
	.asciz "3DNow!"
nowext_str:
	.asciz "3DNowExt"
lm_str:
	.asciz "LM"
rdtscp_str:
	.asciz "RDTSCP"
pag1gb_str:
	.asciz "Page1GB"
ffxtr_str:
	.asciz "AMD FXSAVE/FXSTOR"
mmxamd_str:
	.asciz "MMX-AMDEXT"
nx_str:
	.asciz "NX"
amdsyscsysr_str:
	.asciz "SYSCALL/SYSRET"
/* %eax = 0 */
cpu_str:
	.asciz "Origin = "
/* %eax = 1 */
/* %edx */
f16c_str:
	.asciz "F16C"
pclmul_str:
	.asciz "PCLMUL"
pbe_str:
	.asciz "PBE"
thmon_str:
	.asciz "TM"
htt_str:
	.asciz "HT"
selfsnoop_str:
	.asciz "SS"
sse2_str:
	.asciz "SSE2"
sse_str:
	.asciz "SSE"
fxsr_str:
	.asciz "FXSAVE/FXSTOR"
mmx_str:
	.asciz "MMX"
acpi_str:
	.asciz "ACPI"
ds_str:
	.asciz "DS"
clflush_str:
	.asciz "CFLUSH"
pserial_str:
	.asciz "PSN"
pse36_str:
	.asciz "PSE36"
pat_str:
	.asciz "PAT"
cmove_str:
	.asciz "CMOV/CCMP"
mca_str:
	.asciz "MCA"
pge_str:
	.asciz "PTE-GB"
mtrr_str:
	.asciz "MTRR"
sep_str:
	.asciz "SYSENTER/SYSEXIT"
apicon_str:
	.asciz "APIC-OC"
cx8_str:
	.asciz "CMPXCHG8B"
mce_str:
	.asciz "MCE"
pae_str:
	.asciz "PAE"
msr_str:
	.asciz "RDMSR/WRMSR"
tstmpc_str:
	.asciz "TSC"
pse_str:
	.asciz "PSE"
dbg_str:
	.asciz "DBGEXT"
v86_str:
	.asciz "V86"
x87_str:
	.asciz "X87"
/* %ecx */
avx_str:
	.asciz "AVX"
osx_str:
	.asciz "OSXSAVE"
xsav_str:
	.asciz "XSAVE"
aes_str:
	.asciz "AES"
tsc_str:
	.asciz "TSC"
popcnt_str:
	.asciz "POPC"
movbe_str:
	.asciz "MOVBE"
apicx2_str:
	.asciz "X2APIC"
sse42_str:
	.asciz "SSE4.2"
sse41_str:
	.asciz "SSE4.1"
dca_str:
	.asciz "DCA"
pcid_str:
	.asciz "PCID"
perf_dbg_str:
	.asciz "PERF/DEBUG"
xtpr_str:
	.asciz "XTPR"
cmpxchg16b_str:
	.asciz "CMPXCHG16B"
fma_str:
	.asciz "FMA"
l1cntx_str:
	.asciz "L1ID"
ssse3_str:
	.asciz "SSSE3"
tm2_str:
	.asciz "TM2"
est_str:
	.asciz "Enhanced Intel SpeedStep"
smx_str:
	.asciz "SMEXT"
vmx_str:
	.asciz "VMEXT"
cplqua_str:
	.asciz "CPL QDS"
mon_str:
	.asciz "MONITOR"
dtes64_str:
	.asciz "DS64"
carrless_str:
	.asciz "CM"
sse3_str:
	.asciz "SSE3"
/* %ebx */
apic_str:
	.asciz "Initial APIC = "
cl_str:
	.asciz "CLFLUSH = "
lo_str:
	.asciz "LOGICAL = "
br_str:
	.asciz "Brand Index = "
/* %eax */
id_str:
	.asciz "ID = "
fam_str:
	.asciz "Family = "
mod_str:
	.asciz "Model = "
st_str:
	.asciz "Stepping = "
/* %eax */
extid_str:
	.asciz "Extended ID = "
extfam_str:
	.asciz "Extended Family = "
extmod_str:
	.asciz "Extended Model = "
extst_str:
	.asciz "Extended Stepping = "
unk_str:
	.asciz "Unknown = "
loop_var:
	.byte 0x00
cur_call:
	.long 0x00000000
str_buf:
	.fill 20, 1, 0
str_buf2:
	.fill 20, 1, 0
str_buf3:
	.fill 20, 1, 0
str_buf4:
	.fill 20, 1, 0
hex_num:
	.long 0x00000000
eol:
	.asciz "\n"
/* control arrays */
intel_shf_vars:
	/* call 1 */
	.byte 13, 9, 5, 0 # %eax
	.byte 0, 8, 16, 24 # %ebx
	.byte 28, 27, 26, 25, 24, 23, 22, 21, 20, 19 # %ecx
	.byte 18, 17, 15, 14, 13, 12, 10, 9, 8, 7 # ...
	.byte 6, 5, 4, 3, 2, 1, 0 # ...
	.byte 31, 29, 28, 27, 26, 25, 24, 23, 22, 21 # %edx
	.byte 19, 18, 17, 16, 15, 14, 13, 12, 11, 9, 8 # ...
	.byte 7, 6, 5, 4, 3, 2, 1, 0 # ...
	/* call 2 */
intel_msk_vars:
	.long 3, 15, 15, 3 # %eax
	.long 255, 255, 255, 255 # %ebx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %ecx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # ...
	.long 1
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %edx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # ...
	.long 1, 1, 1
intel_defs:
	.fill 20, 1, 0
intel_str_ord:
	.long id_str, fam_str, mod_str, st_str # %eax
	.long br_str, cl_str, lo_str, apic_str # %ebx
	.long avx_str, osx_str, xsav_str, aes_str # %ecx
	.long tsc_str, popcnt_str, movbe_str, apicx2_str # ...
	.long sse42_str, sse41_str, dca_str, pcid_str # ...
	.long perf_dbg_str, xtpr_str, cmpxchg16b_str, fma_str # ...
	.long l1cntx_str, ssse3_str, tm2_str, est_str # ...
	.long smx_str, vmx_str, cplqua_str, mon_str # ...
	.long dtes64_str, carrless_str, sse3_str # ...
	.long pbe_str, thmon_str, htt_str, selfsnoop_str # %edx
	.long sse2_str, sse_str, fxsr_str, mmx_str # ...
	.long acpi_str, ds_str, clflush_str, pserial_str # ...
	.long pse36_str, pat_str, cmove_str, mca_str # ...
	.long pge_str, mtrr_str, sep_str, apicon_str # ...
	.long cx8_str, mce_str, pae_str, msr_str # ...
	.long tstmpc_str, pse_str, dbg_str, v86_str # ...
	.long x87_str # ...
intel_loop_small:
	.long 4, 4, 27, 29
intel_print_or_feature_set:
	.long 0, 0, 1, 1

/**************************************************************
 * AMD
 **************************************************************
/* control arrays */
amd_shf_vars:
/* %eax = 0x1 */
	.byte 8, 4, 0 # %eax
	.byte 0, 8, 16, 24 # %ebx
	.byte 29, 28, 27, 26, 25, 23, 20, 19 # %ecx
	.byte 13, 12, 9, 3, 1, 0 # ...
	.byte 28, 26, 25, 24, 23 # %edx
	.byte 19, 17, 16, 15, 14, 13, 12, 11, 9, 8 # ...
	.byte 7, 6, 5, 4, 3, 2, 1, 0 # ...
/* %eax = 0x80000001 */
	.byte 8, 4, 0 # %eax
	.byte 0 # %ebx
	.byte 21, 16, 15, 13, 12, 11, 10 # %ecx
	.byte 9, 8, 7, 6, 5, 4, 3, 1, 0 # ...
	.byte 31, 30, 29, 27, 26, 25, 22, 20 # %edx
	.byte 11
amd_msk_vars:
/* %eax = 1 */
	.long 15, 15, 3 # %eax
	.long 255, 255, 255, 255 # %ebx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %ecx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %edx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # ...
/* %eax = 0x80000001 */
	.long 15, 15, 3 # %eax
	.long 255 # %ebx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %ecx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1 # %edx
amd_defs:
	.fill 20, 1, 0
amd_str_ord:
/* %eax = 0x1 */
	.long fam_str, mod_str, st_str, br_str # %eax
	.long cl_str, lo_str, apic_str
	.long f16c_str # %ecx
	.long avx_str, osx_str, xsav_str, aes_str # %ecx
	.long popcnt_str, sse42_str, sse41_str, cmpxchg16b_str # ...
	.long fma_str, ssse3_str, mon_str, pclmul_str # ...
	.long sse3_str # ...
	.long htt_str, sse2_str, sse_str, fxsr_str # %edx
	.long mmx_str # ...
	.long clflush_str # ...
	.long pse36_str, pat_str, cmove_str, mca_str # ...
	.long pge_str, mtrr_str, sep_str, apicon_str # ...
	.long cx8_str, mce_str, pae_str, msr_str # ...
	.long tstmpc_str, pse_str, dbg_str, v86_str # ...
	.long x87_str
/* %eax = 0x80000001 */
	.long extfam_str, extmod_str, extst_str, unk_str # %eax
	.long tbm_str, fma_str, lwp_str, wdt_str # %ecx
	.long xop_str, ibs_str, osvw_str, nowpre_str # ...
	.long misall_str, sse4a_str, abm_str, cr8_str # ...
	.long extapic_str, svm_str, cmpleg_str, lasahf_str # ...
	.long now_str, nowext_str, lm_str, rdtscp_str # %edx
	.long pag1gb_str, ffxtr_str, mmxamd_str, nx_str, amdsyscsysr_str
amd_loop_small:
	.long 3, 4, 14, 23, 3, 1, 16, 9
amd_print_or_feature_set:
/* %eax = 0x1 */
	.long 0, 0, 0, 0, 0, 0, 0 
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %ecx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %edx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # ...
/* %eax = 0x80000001 */
	.long 0, 0, 0, 0
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 # %ecx
	.long 1, 1, 1, 1, 1, 1, 1, 1, 1 # %edx
amd_calls:
	.long 0x1, 0x80000001
intel_str:
	.asciz "Genu"
amd_str:
	.asciz "Auth"
oh_yeah_it_is_intel:
	.byte 0x00
is_feature:
	.long 0x00000000
features_str:
	.asciz "Start Features <"
endfeatures_str:
	.asciz "> End Features"
not_feature:
	.asciz "~"
print_or_feature_set_var:
	.long 0x00000000
comma_str:
	.asciz ", "
calls_vars:
	.long 0x00000000
cpuinfo_str:
	.asciz "Cpu Information/Features"
cute_str:
	.asciz "======================================================\n"
temp_buf:
	.fill 512, 1, 0
