#ifndef CPU_X86_ASM_MSR_H
#define CPU_X86_ASM_MSR_H

static inline uint64_t rdmsr(unsigned index)
{
	uint64_t result;
	asm volatile (
		"rdmsr"
		: "=A" (result)
		: "c" (index)
		);
	return result;
}

static inline void wrmsr(unsigned index, uint64_t msr)
{
	asm volatile (
		"wrmsr"
		: /* No outputs */
		: "c" (index), "A" (msr)
		);
}

#endif /* CPU_X86_ASM_MSR_H */
