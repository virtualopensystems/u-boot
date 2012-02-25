/*
 * Copyright (c) 2012 Samsung Electronics.
 * Abhilash Kesavan <a.kesavan@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>

int exynos_pinmux_config(int peripheral)
{
	int i;
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();

	switch (peripheral) {
	case EXYNOS_UART0:
		for (i = 0; i < 4; i++) {
			s5p_gpio_set_pull(&gpio1->a0, i, GPIO_PULL_NONE);
			s5p_gpio_cfg_pin(&gpio1->a0, i, GPIO_FUNC(0x2));
		}
		break;
	case EXYNOS_UART1:
		for (i = 4; i < 8; i++) {
			s5p_gpio_set_pull(&gpio1->a0, i, GPIO_PULL_NONE);
			s5p_gpio_cfg_pin(&gpio1->a0, i, GPIO_FUNC(0x2));
		}
		break;
	case EXYNOS_UART2:
		for (i = 0; i < 4; i++) {
			s5p_gpio_set_pull(&gpio1->a1, i, GPIO_PULL_NONE);
			s5p_gpio_cfg_pin(&gpio1->a1, i, GPIO_FUNC(0x2));
		}
		break;
	case EXYNOS_UART3:
		for (i = 4; i < 6; i++) {
			s5p_gpio_set_pull(&gpio1->a1, i, GPIO_PULL_NONE);
			s5p_gpio_cfg_pin(&gpio1->a1, i, GPIO_FUNC(0x2));
		}
		break;
	case EXYNOS_SDMMC0_8BIT:
		for (i = 3; i <= 6; i++) {
			s5p_gpio_cfg_pin(&gpio1->c1, i, GPIO_FUNC(0x3));
			s5p_gpio_set_pull(&gpio1->c1, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(&gpio1->c1, i, GPIO_DRV_4X);
		}
		/* Fall-through */
	case EXYNOS_SDMMC0:
		for (i = 0; i < 2; i++) {
			s5p_gpio_cfg_pin(&gpio1->c0, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c0, i, GPIO_PULL_NONE);
			s5p_gpio_set_drv(&gpio1->c0, i, GPIO_DRV_4X);
		}
		for (i = 3; i <= 6; i++) {
			s5p_gpio_cfg_pin(&gpio1->c0, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c0, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(&gpio1->c0, i, GPIO_DRV_4X);
		}
		break;
	case EXYNOS_SDMMC1:
		for (i = 0; i < 2; i++) {
			s5p_gpio_cfg_pin(&gpio1->c1, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c1, i, GPIO_PULL_NONE);
			s5p_gpio_set_drv(&gpio1->c1, i, GPIO_DRV_4X);
		}
		for (i = 3; i <= 6; i++) {
			s5p_gpio_cfg_pin(&gpio1->c1, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c1, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(&gpio1->c1, i, GPIO_DRV_4X);
		}
		break;
	case EXYNOS_SDMMC2_8BIT:
		for (i = 3; i <= 6; i++) {
			s5p_gpio_cfg_pin(&gpio1->c3, i, GPIO_FUNC(0x3));
			s5p_gpio_set_pull(&gpio1->c3, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(&gpio1->c3, i, GPIO_DRV_4X);
		}
		/* Fall-through */
	case EXYNOS_SDMMC2:
		for (i = 0; i < 2; i++) {
			s5p_gpio_cfg_pin(&gpio1->c2, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c2, i, GPIO_PULL_NONE);
			s5p_gpio_set_drv(&gpio1->c2, i, GPIO_DRV_4X);
		}
		for (i = 3; i <= 6; i++) {
			s5p_gpio_cfg_pin(&gpio1->c2, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c2, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(&gpio1->c2, i, GPIO_DRV_4X);
		}
		break;
	case EXYNOS_SDMMC3:
		for (i = 0; i < 2; i++) {
			s5p_gpio_cfg_pin(&gpio1->c3, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c3, i, GPIO_PULL_NONE);
			s5p_gpio_set_drv(&gpio1->c3, i, GPIO_DRV_4X);
		}
		for (i = 3; i <= 6; i++) {
			s5p_gpio_cfg_pin(&gpio1->c3, i, GPIO_FUNC(0x2));
			s5p_gpio_set_pull(&gpio1->c3, i, GPIO_PULL_UP);
			s5p_gpio_set_drv(&gpio1->c3, i, GPIO_DRV_4X);
		}
		break;
	case EXYNOS_SMC911X:
		/*
		 * SROM:CS1 and EBI
		 *
		 * GPY0[0]	SROM_CSn[0]
		 * GPY0[1]	SROM_CSn[1](2)
		 * GPY0[2]	SROM_CSn[2]
		 * GPY0[3]	SROM_CSn[3]
		 * GPY0[4]	EBI_OEn(2)
		 * GPY0[5]	EBI_EEn(2)
		 *
		 * GPY1[0]	EBI_BEn[0](2)
		 * GPY1[1]	EBI_BEn[1](2)
		 * GPY1[2]	SROM_WAIT(2)
		 * GPY1[3]	EBI_DATA_RDn(2)
		 */
		s5p_gpio_cfg_pin(&gpio1->y0, CONFIG_ENV_SROM_BANK,
					GPIO_FUNC(2));
		s5p_gpio_cfg_pin(&gpio1->y0, 4, GPIO_FUNC(2));
		s5p_gpio_cfg_pin(&gpio1->y0, 5, GPIO_FUNC(2));

		for (i = 0; i < 4; i++)
			s5p_gpio_cfg_pin(&gpio1->y1, i, GPIO_FUNC(2));

		/*
		 * EBI: 8 Addrss Lines
		 *
		 * GPY3[0]	EBI_ADDR[0](2)
		 * GPY3[1]	EBI_ADDR[1](2)
		 * GPY3[2]	EBI_ADDR[2](2)
		 * GPY3[3]	EBI_ADDR[3](2)
		 * GPY3[4]	EBI_ADDR[4](2)
		 * GPY3[5]	EBI_ADDR[5](2)
		 * GPY3[6]	EBI_ADDR[6](2)
		 * GPY3[7]	EBI_ADDR[7](2)
		 *
		 * EBI: 16 Data Lines
		 *
		 * GPY5[0]	EBI_DATA[0](2)
		 * GPY5[1]	EBI_DATA[1](2)
		 * GPY5[2]	EBI_DATA[2](2)
		 * GPY5[3]	EBI_DATA[3](2)
		 * GPY5[4]	EBI_DATA[4](2)
		 * GPY5[5]	EBI_DATA[5](2)
		 * GPY5[6]	EBI_DATA[6](2)
		 * GPY5[7]	EBI_DATA[7](2)
		 *
		 * GPY6[0]	EBI_DATA[8](2)
		 * GPY6[1]	EBI_DATA[9](2)
		 * GPY6[2]	EBI_DATA[10](2)
		 * GPY6[3]	EBI_DATA[11](2)
		 * GPY6[4]	EBI_DATA[12](2)
		 * GPY6[5]	EBI_DATA[13](2)
		 * GPY6[6]	EBI_DATA[14](2)
		 * GPY6[7]	EBI_DATA[15](2)
		 */
		for (i = 0; i < 8; i++) {
			s5p_gpio_cfg_pin(&gpio1->y3, i, GPIO_FUNC(2));
			s5p_gpio_set_pull(&gpio1->y3, i, GPIO_PULL_UP);

			s5p_gpio_cfg_pin(&gpio1->y5, i, GPIO_FUNC(2));
			s5p_gpio_set_pull(&gpio1->y5, i, GPIO_PULL_UP);

			s5p_gpio_cfg_pin(&gpio1->y6, i, GPIO_FUNC(2));
			s5p_gpio_set_pull(&gpio1->y6, i, GPIO_PULL_UP);
		}
		break;

	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	}

	return 0;
}
