/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef	__ASM_GENERIC_GBL_DATA_H
#define __ASM_GENERIC_GBL_DATA_H
/*
 * The following data structure is placed in some memory which is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 *
 * Keep it *SMALL* and remember to set GENERATED_GBL_DATA_SIZE > sizeof(gd_t)
 *
 * sjg@chromium.org: Well it would be nice to have a generic one of these
 * since so many fields are similar. But it means that everyone architecture
 * will want to add its own nutty fields. Perhaps that is no bad thing since
 * it shows up inconsistences and might produce downward pressure on the
 * number of fields.
 */

#ifndef __ASSEMBLY__
typedef struct global_data {
	bd_t		*bd;
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	cpu_clk;	/* CPU clock in Hz!		*/
	unsigned long	bus_clk;
#if defined(CONFIG_8xx)
	unsigned long	brg_clk;
#endif
#if defined(CONFIG_CPM2)
	/* There are many clocks on the MPC8260 - see page 9-5 */
	unsigned long	vco_out;
	unsigned long	cpm_clk;
	unsigned long	scc_clk;
	unsigned long	brg_clk;
#ifdef CONFIG_PCI
	unsigned long	pci_clk;
#endif
#endif
	unsigned long   mem_clk;
#if defined(CONFIG_MPC83xx)
	/* There are other clocks in the MPC83XX */
	u32 csb_clk;
#if defined(CONFIG_MPC8308) || defined(CONFIG_MPC831x) || \
	defined(CONFIG_MPC834x) || defined(CONFIG_MPC837x)
	u32 tsec1_clk;
	u32 tsec2_clk;
	u32 usbdr_clk;
#endif
#if defined(CONFIG_MPC834x)
	u32 usbmph_clk;
#endif /* CONFIG_MPC834x */
#if defined(CONFIG_MPC8315)
	u32 tdm_clk;
#endif
	u32 core_clk;
	u32 enc_clk;
	u32 lbiu_clk;
	u32 lclk_clk;
	u32 pci_clk;
#if defined(CONFIG_MPC8308) || defined(CONFIG_MPC831x) || \
	defined(CONFIG_MPC837x)
	u32 pciexp1_clk;
	u32 pciexp2_clk;
#endif
#if defined(CONFIG_MPC837x) || defined(CONFIG_MPC8315)
	u32 sata_clk;
#endif
#if defined(CONFIG_MPC8360)
	u32  mem_sec_clk;
#endif /* CONFIG_MPC8360 */
#endif
#if defined(CONFIG_FSL_ESDHC)
	u32 sdhc_clk;
#endif
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	u32 lbc_clk;
	void *cpu;
#endif /* CONFIG_MPC85xx || CONFIG_MPC86xx */
#if defined(CONFIG_MPC83xx) || defined(CONFIG_MPC85xx) \
		|| defined(CONFIG_MPC86xx)
	u32 i2c1_clk;
	u32 i2c2_clk;
#endif
#if defined(CONFIG_QE)
	u32 qe_clk;
	u32 brg_clk;
	uint mp_alloc_base;
	uint mp_alloc_top;
#endif /* CONFIG_QE */
#if defined(CONFIG_FSL_LAW)
	u32 used_laws;
#endif
#if defined(CONFIG_E500)
	u32 used_tlb_cams[(CONFIG_SYS_NUM_TLBCAMS+31)/32];
#endif
#if defined(CONFIG_MPC5xxx)
	unsigned long	ipb_clk;
	unsigned long	pci_clk;
#endif
#if defined(CONFIG_MPC512X)
	u32 ips_clk;
	u32 csb_clk;
	u32 pci_clk;
#endif /* CONFIG_MPC512X */
#if defined(CONFIG_MPC8220)
	unsigned long   bExtUart;
	unsigned long   inp_clk;
	unsigned long   pci_clk;
	unsigned long   vco_clk;
	unsigned long   pev_clk;
	unsigned long   flb_clk;
#endif
	unsigned long	reset_status;	/* reset status register at boot */
#if defined(CONFIG_MPC83xx)
	unsigned long	arbiter_event_attributes;
	unsigned long	arbiter_event_address;
#endif
#if defined(CONFIG_SYS_ALLOC_DPRAM) || defined(CONFIG_CPM2)
	unsigned int	dp_alloc_base;
	unsigned int	dp_alloc_top;
#endif
#if defined(CONFIG_4xx)
	u32  uart_clk;
#endif /* CONFIG_4xx */
#if defined(CONFIG_SYS_GT_6426x)
	unsigned int	mirror_hack[16];
#endif
#if defined(CONFIG_A3000)	|| \
		defined(CONFIG_HIDDEN_DRAGON)  || \
		defined(CONFIG_MUSENKI)	||  \
		defined(CONFIG_SANDPOINT)
	void *console_addr;
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	unsigned long	fb_base;	/* Base address of framebuffer mem */
#endif
#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
	unsigned long	post_log_word;  /* Record POST activities */
	unsigned long	post_log_res; /* success of POST test */
	unsigned long	post_init_f_time;  /* When post_init_f started */
#endif
#ifdef CONFIG_BOARD_TYPES
	unsigned long	board_type;
#endif
#ifdef CONFIG_MODEM_SUPPORT
	unsigned long do_mdm_init;
	unsigned long be_quiet;
#endif
#if defined(CONFIG_LWMON) || defined(CONFIG_LWMON5)
	unsigned long kbd_status;
#endif
#ifdef CONFIG_SYS_FPGA_COUNT
	unsigned fpga_state[CONFIG_SYS_FPGA_COUNT];
#endif
#if defined(CONFIG_WD_MAX_RATE)
	unsigned long long wdt_last;	/* trace watch-dog triggering rate */
#endif
	unsigned long	have_console;	/* serial_init() was called */
#ifdef CONFIG_PRE_CONSOLE_BUFFER
	unsigned long	precon_buf_idx;	/* Pre-Console buffer index */
#endif
	unsigned long	env_addr;	/* Address  of Environment struct */
	unsigned long	env_valid;	/* Checksum of Environment valid? */
	/* Here begins ARM-specific things. Needs discussion */
#ifdef CONFIG_AT91FAMILY
	/* "static data" needed by at91's clock.c */
	unsigned long	cpu_clk_rate_hz;
	unsigned long	main_clk_rate_hz;
	unsigned long	mck_rate_hz;
	unsigned long	plla_rate_hz;
	unsigned long	pllb_rate_hz;
	unsigned long	at91_pllb_usb_init;
#endif
#ifdef CONFIG_ARM
	/* "static data" needed by most of timer.c on ARM platforms */
	unsigned long	timer_rate_hz;
	unsigned long	tbl;
	unsigned long	tbu;
	unsigned long long	timer_reset_value;
	unsigned long	lastinc;
#endif
#ifdef CONFIG_IXP425
	unsigned long	timestamp;
#endif
	/* TODO: is this the same as relocaddr, or something else? */
	unsigned long	dest_addr;	/* Post-relocation address of U-Boot */
	unsigned long	dest_addr_sp;
	unsigned long	ram_top;	/* Top address of RAM used by U-Boot */

	unsigned long	relocaddr;	/* Start address of U-Boot in RAM */
	phys_size_t	ram_size;	/* RAM size */
	unsigned long	mon_len;	/* monitor len */
	unsigned long	irq_sp;		/* irq stack pointer */
	unsigned long	start_addr_sp;	/* start_addr_stackpointer */
	unsigned long	reloc_off;
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	unsigned long	tlb_addr;
#endif
	struct global_data *new_gd;	/* relocated global data */
	const void	*fdt_blob;	/* Our device tree, NULL if none */
	void		**jt;		/* jump table */
	char		env_buf[32];	/* buffer for getenv() before reloc. */
} gd_t;
#endif

/*
 * Global Data Flags
 */
#define	GD_FLG_RELOC		0x00001	/* Code was relocated to RAM	   */
#define	GD_FLG_DEVINIT		0x00002	/* Devices have been initialized   */
#define	GD_FLG_SILENT		0x00004	/* Silent mode			   */
#define	GD_FLG_POSTFAIL		0x00008	/* Critical POST test failed	   */
#define	GD_FLG_POSTSTOP		0x00010	/* POST seqeunce aborted	   */
#define	GD_FLG_LOGINIT		0x00020	/* Log Buffer has been initialized */
#define GD_FLG_DISABLE_CONSOLE	0x00040	/* Disable console (in & out)	   */
#define GD_FLG_ENV_READY	0x00080	/* Env. imported into hash table   */

#endif /* __ASM_GENERIC_GBL_DATA_H */
