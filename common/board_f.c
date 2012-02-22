/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <linux/compiler.h>
#include <version.h>
#include <asm-generic/sections.h>
#include <asm/io.h>
#ifdef CONFIG_MP
#include <asm/mp.h>
#endif
#include <environment.h>
#include <fdtdec.h>
#if defined(CONFIG_CMD_IDE)
#include <ide.h>
#endif
#include <i2c.h>
#include <initcall.h>
#include <logbuff.h>

/* TODO: Can we move these into arch/ headers? */
#ifdef CONFIG_8xx
#include <mpc8xx.h>
#endif
#ifdef CONFIG_5xx
#include <mpc5xx.h>
#endif
#ifdef CONFIG_MPC5xxx
#include <mpc5xxx.h>
#endif

#include <post.h>
#include <spi.h>
#include <watchdog.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it if needed.
 */
#ifdef XTRN_DECLARE_GLOBAL_DATA_PTR
#undef	XTRN_DECLARE_GLOBAL_DATA_PTR
#define XTRN_DECLARE_GLOBAL_DATA_PTR	/* empty = allocate here */
DECLARE_GLOBAL_DATA_PTR = (gd_t *) (CONFIG_SYS_INIT_GD_ADDR);
#else
DECLARE_GLOBAL_DATA_PTR;
#endif

/* TODO: Move to header file */
int print_cpuinfo(void);

/*
 * sjg: IMO this code should be
 * refactored to a single function, something like:
 *
 * void led_set_state(enum led_colour_t colour, int on);
 */
/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
inline void __coloured_LED_init(void) {}
void coloured_LED_init(void)
	__attribute__((weak, alias("__coloured_LED_init")));
inline void __red_led_on(void) {}
void red_led_on(void) __attribute__((weak, alias("__red_led_on")));
inline void __red_led_off(void) {}
void red_led_off(void) __attribute__((weak, alias("__red_led_off")));
inline void __green_led_on(void) {}
void green_led_on(void) __attribute__((weak, alias("__green_led_on")));
inline void __green_led_off(void) {}
void green_led_off(void) __attribute__((weak, alias("__green_led_off")));
inline void __yellow_led_on(void) {}
void yellow_led_on(void) __attribute__((weak, alias("__yellow_led_on")));
inline void __yellow_led_off(void) {}
void yellow_led_off(void) __attribute__((weak, alias("__yellow_led_off")));
inline void __blue_led_on(void) {}
void blue_led_on(void) __attribute__((weak, alias("__blue_led_on")));
inline void __blue_led_off(void) {}
void blue_led_off(void) __attribute__((weak, alias("__blue_led_off")));

/*
 * Why is gd allocated a register? Prior to reloc it might be better to
 * just pass it around to each function in this file?
 *
 * After reloc one could argue that it is hardly used and doesn't need
 * to be in a register. Or if it is it should perhaps hold pointers to all
 * global data for all modules, so that post-reloc we can avoid the massive
 * literal pool we get on ARM. Or perhaps just encourage each module to use
 * a structure...
 */

/*
 * Could the CONFIG_SPL_BUILD infection become a flag in gd?
 */

#if defined(CONFIG_WATCHDOG)
static int init_func_watchdog_init(void)
{
	puts("       Watchdog enabled\n");
	WATCHDOG_RESET();
	return 0;
}

#define INIT_FUNC_WATCHDOG_INIT	init_func_watchdog_init,

int init_func_watchdog_reset(void)
{
	WATCHDOG_RESET();
	return 0;
}

#else
#define INIT_FUNC_WATCHDOG_INIT
#endif /* CONFIG_WATCHDOG */

void __board_add_ram_info(int use_default)
{
	/* please define platform specific board_add_ram_info() */
}

void board_add_ram_info(int)
	__attribute__ ((weak, alias("__board_add_ram_info")));

static int init_baud_rate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

static int display_text_info(void)
{
	ulong bss_start, bss_end;

#ifdef CONFIG_SYS_SYM_OFFSETS
	bss_start = _bss_start_ofs + _TEXT_BASE;
	bss_end = _bss_end_ofs + _TEXT_BASE;
#else
	bss_start = (ulong)&__bss_start;
	bss_end = (ulong)&__bss_end;
#endif
	debug("U-Boot code: %08X -> %08lX  BSS: -> %08lX\n",
	      CONFIG_SYS_TEXT_BASE, bss_start, bss_end);

#ifdef CONFIG_MODEM_SUPPORT
	debug("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return 0;
}

static int announce_dram_init(void)
{
	puts("DRAM:  ");
	return 0;
}

#ifdef CONFIG_PPC
static int init_func_ram(void)
{
#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#else
	int board_type = 0;	/* use dummy arg */
#endif

	gd->ram_size = initdram(board_type);

	if (gd->ram_size > 0)
		return 0;

	puts("*** failed ***\n");
	return 1;
}
#endif

static int display_dram_config(void)
{
	ulong size;

#ifdef CONFIG_NR_DRAM_BANKS
	int i;

	debug("\nRAM Configuration:\n");
	for (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
		debug("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
#ifdef DEBUG
		print_size(gd->bd->bi_dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");
#else
	size = gd->ram_size;
#endif

	print_size(size, "");
	board_add_ram_info(0);
	putc('\n');

	return 0;
}

ulong get_effective_memsize(void)
{
#ifndef	CONFIG_VERY_BIG_RAM
	return gd->ram_size;
#else
	/* limit stack to what we can reasonable map */
	return ((gd->ram_size > CONFIG_MAX_MEM_MAPPED) ?
		CONFIG_MAX_MEM_MAPPED : gd->ram_size);
#endif
}

void __dram_init_banksize(void)
{
#if defined(CONFIG_NR_DRAM_BANKS) && defined(CONFIG_SYS_SDRAM_BASE)
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();
#endif
}

void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c(void)
{
	puts("I2C:   ");
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts("ready\n");
	return 0;
}
#endif

#if defined(CONFIG_HARD_SPI)
static int init_func_spi(void)
{
	puts("SPI:   ");
	spi_init();
	puts("ready\n");
	return 0;
}
#endif

static int setup_global_data_ptr(void)
{
	/*
	 * Pointer is writable since we allocated a register for it.
	 * Can we choose one of these two methods instead of offering both?
	 */
#ifdef CONFIG_SYS_INIT_SP_ADDR
	gd = (gd_t *) ((CONFIG_SYS_INIT_SP_ADDR) & ~0x07);
#else
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);
#endif

	/* compiler optimization barrier needed for GCC >= 3.4 */
	dmb();

	return 0;
}

__maybe_unused
static int zero_global_data(void)
{
	memset((void *)gd, '\0', sizeof(gd_t));

	return 0;
}

static int setup_mon_len(void)
{
#ifdef CONFIG_SYS_SYM_OFFSETS
	gd->mon_len = _bss_end_ofs;
#else
	/* TODO: use (ulong)&__bss_end - (ulong)&__text_start; ? */
	gd->mon_len = (ulong)&__bss_end__ - CONFIG_SYS_MONITOR_BASE;
#endif
	return 0;
}

static int setup_fdt(void)
{
#ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = _binary_dt_dtb_start;
#elif defined CONFIG_OF_SEPARATE
	/* FDT is at end of image */
	gd->fdt_blob = (void *)(_end_ofs + CONFIG_SYS_TEXT_BASE);
#endif
	/* Allow the early environment to override the fdt address */
	gd->fdt_blob = (void *)getenv_ulong("fdtcontroladdr", 16,
						(uintptr_t)gd->fdt_blob);
	return 0;
}

static int setup_reloc(void)
{
	debug("Monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug("Ram size: %08lX\n", (ulong)gd->ram_size);
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif
#ifdef CONFIG_NR_DRAM_BANKS
	gd->dest_addr = gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;
#else
	gd->dest_addr = CONFIG_SYS_SDRAM_BASE + get_effective_memsize();
#endif
	gd->ram_top = gd->dest_addr;
#if defined(CONFIG_MP) && (defined(CONFIG_MPC86xx) || defined(CONFIG_E500))
	/*
	 * We need to make sure the location we intend to put secondary core
	 * boot code is reserved and not used by any part of u-boot
	 */
	if (gd->dest_addr > determine_mp_bootpg()) {
		gd->dest_addr = determine_mp_bootpg();
		debug("Reserving MP boot page to %08lx\n", gd->dest_addr);
	}
#endif
	gd->dest_addr_sp = gd->dest_addr;
	return 0;
}

#if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
static int reserve_logbuffer(void)
{
	/* reserve kernel log buffer */
	gd->dest_addr -= LOGBUFF_RESERVE;
	debug("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN,
		gd->dest_addr);
	return 0;
}
#endif

#ifdef CONFIG_PRAM
/* reserve protected RAM */
static int reserve_pram(void)
{
	ulong reg;

	reg = getenv_ulong("pram", 10, CONFIG_PRAM);
	gd->dest_addr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->dest_addr);
	return 0;
}
#endif /* CONFIG_PRAM */

/* Round memory pointer down to next 4 kB limit */
static int reserve_round_4k(void)
{
	gd->dest_addr &= ~(4096 - 1);
	return 0;
}

#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
static int reserve_mmu(void)
{
	/* reserve TLB table */
	gd->dest_addr -= (4096 * 4);

	/* round down to next 64 kB limit */
	gd->dest_addr &= ~(0x10000 - 1);

	gd->tlb_addr = gd->dest_addr;
	debug("TLB table at: %08lx\n", gd->tlb_addr);
	return 0;
}
#endif

#ifdef CONFIG_LCD
static int reserve_lcd(void)
{
#ifdef CONFIG_FB_ADDR
	gd->fb_base = CONFIG_FB_ADDR;
#else
	/* reserve memory for LCD display (always full pages) */
	gd->dest_addr = lcd_setmem(gd->dest_addr);
	gd->fb_base = gd->dest_addr;
#endif /* CONFIG_FB_ADDR */
	return 0;
}
#endif /* CONFIG_LCD */

/* TODO: We should be able to just use CONFIG_VIDEO here */
#if defined(CONFIG_VIDEO) && (!defined(CONFIG_PPC) || defined(CONFIG_8xx)) \
		&& !defined(CONFIG_ARM)
static int reserve_video(void)
{
	/* reserve memory for video display (always full pages) */
	gd->dest_addr = video_setmem(gd->dest_addr);
	gd->fb_base = gd->dest_addr;

	return 0;
}
#endif

static int reserve_uboot(void)
{
	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	gd->dest_addr -= gd->mon_len;
	gd->dest_addr &= ~(4096 - 1);
#ifdef CONFIG_E500
	/* round down to next 64 kB limit so that IVPR stays aligned */
	gd->dest_addr &= ~(65536 - 1);
#endif

	debug("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10,
	      gd->dest_addr);
	return 0;
}

#ifndef CONFIG_SPL_BUILD
/* reserve memory for malloc() area */
static int reserve_malloc(void)
{
	gd->dest_addr_sp = gd->dest_addr - TOTAL_MALLOC_LEN;
	debug("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, gd->dest_addr_sp);
	return 0;
}

/* (permanently) allocate a Board Info struct */
static int reserve_board(void)
{
	gd->dest_addr_sp -= sizeof(bd_t);
	gd->bd = (bd_t *)gd->dest_addr_sp;
	memset(gd->bd, '\0', sizeof(bd_t));
	debug("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof(bd_t), gd->dest_addr_sp);
	return 0;
}
#endif

static int setup_machine(void)
{
#ifdef CONFIG_MACH_TYPE
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE; /* board id for Linux */
#endif
	return 0;
}

static int reserve_global_data(void)
{
	gd->dest_addr_sp -= sizeof(gd_t);
	gd->new_gd = (gd_t *)gd->dest_addr_sp;
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof(gd_t), gd->dest_addr_sp);
	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int reserve_stacks(void)
{
#ifdef CONFIG_PPC
	ulong *s;
#endif

	/* setup stack pointer for exceptions */
	gd->dest_addr_sp -= 16;
	gd->dest_addr_sp &= ~0xf;
	gd->irq_sp = gd->dest_addr_sp;
#ifdef CONFIG_USE_IRQ
	gd->dest_addr_sp -= (CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ);
	debug("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ, gd->dest_addr_sp);

	/* 8-byte alignment for ARM ABI compliance */
	gd->dest_addr_sp &= ~0x07;
#endif
	/* leave 3 words for abort-stack, plus 1 for alignment */
	gd->dest_addr_sp -= 16;

#ifdef CONFIG_PPC
	/* Clear initial stack frame */
	s = (ulong *) gd->dest_addr_sp;
	*s-- = 0;
	*s-- = 0;
	gd->dest_addr_sp = (ulong) s;
#endif

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
static int reserve_stacks_spl(void)
{
	/* Why not -= ? */
	gd->dest_addr_sp += 128;	/* leave 32 words for abort-stack */
	gd->irq_sp = gd->dest_addr_sp;
	return 0;
}
#endif

static int display_new_sp(void)
{
	debug("New Stack Pointer is: %08lx\n", gd->dest_addr_sp);

	return 0;
}

#ifdef CONFIG_PPC
static int setup_board_part1(void)
{
	bd_t *bd = gd->bd;

	/*
	 * Save local variables to board info struct
	 */

	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;	/* start of memory */
	bd->bi_memsize = gd->ram_size;			/* size in bytes */

#ifdef CONFIG_SYS_SRAM_BASE
	bd->bi_sramstart = CONFIG_SYS_SRAM_BASE;	/* start of SRAM */
	bd->bi_sramsize = CONFIG_SYS_SRAM_SIZE;		/* size  of SRAM */
#endif

#if defined(CONFIG_8xx) || defined(CONFIG_8260) || defined(CONFIG_5xx) || \
		defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
	bd->bi_immr_base = CONFIG_SYS_IMMR;	/* base  of IMMR register     */
#endif
#if defined(CONFIG_MPC5xxx)
	bd->bi_mbar_base = CONFIG_SYS_MBAR;	/* base of internal registers */
#endif
#if defined(CONFIG_MPC83xx)
	bd->bi_immrbar = CONFIG_SYS_IMMR;
#endif
#if defined(CONFIG_MPC8220)
	bd->bi_mbar_base = CONFIG_SYS_MBAR;	/* base of internal registers */
	bd->bi_inpfreq = gd->inp_clk;
	bd->bi_pcifreq = gd->pci_clk;
	bd->bi_vcofreq = gd->vco_clk;
	bd->bi_pevfreq = gd->pev_clk;
	bd->bi_flbfreq = gd->flb_clk;

	/* store bootparam to sram (backward compatible), here? */
	{
		u32 *sram = (u32 *) CONFIG_SYS_SRAM_BASE;

		*sram++ = gd->ram_size;
		*sram++ = gd->bus_clk;
		*sram++ = gd->inp_clk;
		*sram++ = gd->cpu_clk;
		*sram++ = gd->vco_clk;
		*sram++ = gd->flb_clk;
		*sram++ = 0xb8c3ba11;	/* boot signature */
	}
#endif

	return 0;
}

static int setup_board_part2(void)
{
	bd_t *bd = gd->bd;

	bd->bi_intfreq = gd->cpu_clk;	/* Internal Freq, in Hz */
	bd->bi_busfreq = gd->bus_clk;	/* Bus Freq,      in Hz */
#if defined(CONFIG_CPM2)
	bd->bi_cpmfreq = gd->cpm_clk;
	bd->bi_brgfreq = gd->brg_clk;
	bd->bi_sccfreq = gd->scc_clk;
	bd->bi_vco = gd->vco_out;
#endif /* CONFIG_CPM2 */
#if defined(CONFIG_MPC512X)
	bd->bi_ipsfreq = gd->ips_clk;
#endif /* CONFIG_MPC512X */
#if defined(CONFIG_MPC5xxx)
	bd->bi_ipbfreq = gd->ipb_clk;
	bd->bi_pcifreq = gd->pci_clk;
#endif /* CONFIG_MPC5xxx */

	return 0;
}
#endif

#ifdef CONFIG_SYS_EXTBDINFO
static int setup_board_extra(void)
{
	bd_t *bd = gd->bd;

	strncpy((char *) bd->bi_s_version, "1.2", sizeof(bd->bi_s_version));
	strncpy((char *) bd->bi_r_version, U_BOOT_VERSION,
		sizeof(bd->bi_r_version));

	bd->bi_procfreq = gd->cpu_clk;	/* Processor Speed, In Hz */
	bd->bi_plb_busfreq = gd->bus_clk;
#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || \
		defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
		defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	bd->bi_pci_busfreq = get_PCI_freq();
	bd->bi_opbfreq = get_OPB_freq();
#elif defined(CONFIG_XILINX_405)
	bd->bi_pci_busfreq = get_PCI_freq();
#endif

	return 0;
}
#endif

#ifdef CONFIG_POST
static int init_post(void)
{
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));

	return 0;
}
#endif

static int setup_baud_rate(void)
{
	/* Ick, can we get rid of this line? */
	gd->bd->bi_baudrate = gd->baudrate;

	return 0;
}

static int setup_dram_config(void)
{
	/* Ram is board specific, so move it to board code ... */
	dram_init_banksize();

	return 0;
}

static int jump_to_copy(void)
{
	memcpy(gd->new_gd, (char *)gd, sizeof(gd_t));
	gd->relocaddr = gd->dest_addr;
	gd->reloc_off = gd->dest_addr - CONFIG_SYS_TEXT_BASE;
	debug("Relocation Offset is: %08lx\n", gd->reloc_off);
	debug("Relocating to %08lx, new gd at %p, sp at %08lx\n",
	      gd->dest_addr, gd->new_gd, gd->dest_addr_sp);

	relocate_code(gd->dest_addr_sp, gd->new_gd, gd->dest_addr);

	return 0;
}

static init_fnc_t init_sequence_f[] = {
	setup_global_data_ptr,
#if !defined(CONFIG_CPM2) && !defined(CONFIG_MPC512X) && \
		!defined(CONFIG_MPC83xx) && !defined(CONFIG_MPC85xx) && \
		!defined(CONFIG_MPC86xx)
	zero_global_data,
#endif
	setup_fdt,
	setup_mon_len,
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/* TODO: can this go into arch_cpu_init()? */
	probecpu,
#endif
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
#ifdef CONFIG_OF_CONTROL
	fdtdec_check_fdt,
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	/* TODO: can any of this go into arch_cpu_init()? */
#if defined(CONFIG_PPC) && !defined(CONFIG_8xx_CPUCLK_DEFAULT)
	get_clocks,		/* get CPU and bus clocks (etc.) */
#if defined(CONFIG_TQM8xxL) && !defined(CONFIG_TQM866M) \
		&& !defined(CONFIG_TQM885D)
	adjust_sdram_tbs_8xx,
#endif
	/* TODO: can we rename this to timer_init()? */
	init_timebase,
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
#ifdef CONFIG_ARM
	timer_init,		/* initialize timer */
#endif
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
#ifdef CONFIG_SYS_ALLOC_DPRAM
#if !defined(CONFIG_CPM2)
	dpram_init,
#endif
#endif
#if defined(CONFIG_BOARD_POSTCLK_INIT)
	board_postclk_init,
#endif
	env_init,		/* initialize environment */
#if defined(CONFIG_8xx_CPUCLK_DEFAULT)
	/* get CPU and bus clocks according to the environment variable */
	get_clocks_866,
	/* adjust sdram refresh rate according to the new clock */
	sdram_adjust_866,
	init_timebase,
#endif
	init_baud_rate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_options,	/* say that we are here */
	display_text_info,	/* show debugging info if required */
#if defined(CONFIG_8260)
	prt_8260_rsr,
	prt_8260_clks,
#endif /* CONFIG_8260 */
#if defined(CONFIG_MPC83xx)
	prt_83xx_rsr,
#endif
#ifdef CONFIG_PPC
	checkcpu,
#endif
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_MPC5xxx)
	prt_mpc5xxx_clks,
#endif /* CONFIG_MPC5xxx */
#if defined(CONFIG_MPC8220)
	prt_mpc8220_clks,
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
	INIT_FUNC_WATCHDOG_INIT
#if defined(CONFIG_MISC_INIT_F)
	misc_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
#if defined(CONFIG_HARD_SPI)
	init_func_spi,
#endif
	announce_dram_init,
	/* TODO: unify all these dram functions? */
#ifdef CONFIG_ARM
	dram_init,		/* configure available RAM banks */
	setup_dram_config,
#endif
#ifdef CONFIG_X86
	dram_init_f,		/* configure available RAM banks */
#endif
#ifdef CONFIG_PPC
	init_func_ram,
#endif
	display_dram_config,
#ifdef CONFIG_POST
	post_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_SYS_DRAM_TEST)
	testdram,
#endif /* CONFIG_SYS_DRAM_TEST */
	INIT_FUNC_WATCHDOG_RESET

#ifdef CONFIG_POST
	init_post,
#endif
	INIT_FUNC_WATCHDOG_RESET
	setup_dram_config,

	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - area that won't get touched by U-Boot and Linux (optional)
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
	setup_reloc,
#if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
	reserve_logbuffer,
#endif
#ifdef CONFIG_PRAM
	reserve_pram,
#endif
	reserve_round_4k,
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	reserve_mmu,
#endif
#ifdef CONFIG_LCD
	reserve_lcd,
#endif
	/* TODO: Why the dependency on CONFIG_8xx? */
#if defined(CONFIG_VIDEO) && (!defined(CONFIG_PPC) || defined(CONFIG_8xx)) \
		&& !defined(CONFIG_ARM)
	reserve_video,
#endif
	reserve_uboot,
#ifndef CONFIG_SPL_BUILD
	reserve_malloc,
	reserve_board,
#endif
	setup_machine,
	reserve_global_data,
#ifdef CONFIG_SPL_BUILD
	reserve_stacks_spl,
#else
	reserve_stacks,
#endif
#ifdef CONFIG_PPC
	setup_board_part1,
	INIT_FUNC_WATCHDOG_RESET
	setup_board_part2,
#endif
	setup_baud_rate,
	display_new_sp,
#ifdef CONFIG_SYS_EXTBDINFO
	setup_board_extra,
#endif
	INIT_FUNC_WATCHDOG_RESET
	jump_to_copy,
	NULL,
};

void board_init_f(ulong bootflags)
{
	/* TODO: perhaps declare gd as a local variable */

	/* TODO: save bootflag into gd->flags */
	if (initcall_run_list(init_sequence_f))
		hang();

	/* NOTREACHED - jump_to_copy() does not return */
	while (1)
		;
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}
