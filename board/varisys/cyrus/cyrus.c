/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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
#include <command.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>
#include <pci.h>

#include "cyrus.h"
 #include "../common/eeprom.h"

#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_OPENDRAIN 0x30000000
#define GPIO_DIR       0x3c000004
#define GPIO_INITIAL   0x30000000
#define GPIO_VGA_SWITCH 0x00001000

int checkboard (void)
{
	printf("Board: CYRUS\n");

	return 0;
}

int board_early_init_f(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	
	/*
	 * P4080 DS board only uses the DDR1_MCK0/3 and DDR2_MCK0/3
	 * disable the DDR1_MCK1/2/4/5 and DDR2_MCK1/2/4/5 to reduce
	 * the noise introduced by these unterminated and unused clock pairs.
	 */
	setbits_be32(&gur->ddrclkdr, 0x001B001B);

	/* Set GPIO reset lines to open-drain, tristate */
	setbits_be32(&pgpio->gpdat, GPIO_INITIAL);
	setbits_be32(&pgpio->gpodr, GPIO_OPENDRAIN);
	
	/* Set GPIO Direction */
	setbits_be32(&pgpio->gpdir, GPIO_DIR);
	
	return 0;
}

int board_early_init_r(void)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	
	lbc->lbcr = 0;
	lbc->lcrr = 0x80000000 | CONFIG_SYS_LBC_LCRR;	/* 1 clock LALE cycle */
	
	set_liodns();
	
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_portals();
#endif
	print_lbc_regs();
	return 0;
}

extern int read_eeprom(void);
extern void show_eeprom(void);

int misc_init_r(void)
{
    if (read_eeprom())
	printf("Error reading serial number\n");
    else
	show_eeprom();
	
    return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
	fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}

int mac_read_from_eeprom(void)
{
	init_eeprom(CONFIG_SYS_EEPROM_BUS_NUM, 
		CONFIG_SYS_I2C_EEPROM_ADDR, 
		CONFIG_SYS_I2C_EEPROM_ADDR_LEN);
	// Detect board type
#ifndef CONFIG_CYRUS_V2
	u16 boardrev;
	*((volatile u16 *)CONFIG_SYS_LBC0_BASE) = 2;
	boardrev = *((volatile u16 *)(CONFIG_SYS_LBC0_BASE + 0x8000));
	printf("Board Revision %04x\n", boardrev);
	if ((boardrev >> 8) >= 0x22) {
		mac_read_from_fixed_id();
	} 
#endif
	return mac_read_from_eeprom_common();
}
