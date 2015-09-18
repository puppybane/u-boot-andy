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

#include "cyrus.h"

#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_OPENDRAIN 0x30000000
#define GPIO_DIR       0x3c000004
#define GPIO_INITIAL   0x30000000

int checkboard (void)
{
	printf("Board: CYRUS\n");

#ifdef CONFIG_PHYS_64BIT
	puts("36-bit Addressing\n");
#endif

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
	lbc->lcrr = 0x80000000 | CONFIG_SYS_LBC_LCRR | LCRR_EADC_1;	/* 1 clock LALE cycle */
	
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

#include "../../../drivers/bios_emulator/include/biosemu.h"
#include "../../../drivers/bios_emulator/vesa.h"
extern int BootVideoCardBIOS(pci_dev_t pcidev, BE_VGAInfo ** pVGAInfo, int cleanUp);

#define PCI_CLASS_VIDEO             3
#define PCI_CLASS_VIDEO_STD         0
#define PCI_CLASS_VIDEO_PROG_IF_VGA 0

pci_dev_t find_vga()
{
	pci_dev_t bdf;
	int bus, found_multi = 0;
	
	// Find root controller
	struct pci_controller *hose = pci_bus_to_hose(1);
	
	if (! hose) {
		printf("Unable to find root PCI controller\n");
		return -1;
	}
	
	for (bus = hose->first_busno; bus <= hose->last_busno; bus++) {
		
		for (bdf = PCI_BDF(bus, 0, 0);
		     bdf < PCI_BDF(bus + 1, 0, 0);
		     bdf += PCI_BDF(0, 0, 1)) {
			
			unsigned char class;
			     
			     
			if (!PCI_FUNC(bdf)) {
				unsigned char header_type;
				pci_read_config_byte(bdf,
						     PCI_HEADER_TYPE,
						     &header_type);

				found_multi = header_type & 0x80;
			} else {
				if (!found_multi)
					continue;
			}

			pci_read_config_byte(bdf,
					     PCI_CLASS_CODE,
					     &class);
			if (class != PCI_CLASS_VIDEO)
				continue;
			
			pci_read_config_byte(bdf,
					     PCI_CLASS_DEVICE,
					     &class);			
			if (class != PCI_CLASS_VIDEO_STD)
				continue;
			
			pci_read_config_byte(bdf,
					     PCI_CLASS_PROG,
					     &class);
			
			if (class == PCI_CLASS_VIDEO_PROG_IF_VGA)
				return bdf;			
		}
	}
	
	return -1;
}

static GraphicDevice grd;

void * video_hw_init(void)
{	
	// Boot video here once everything else is working
	pci_dev_t vga = find_vga();

	printf("Looking for VGA\n");
	
	/* PostBIOS with x86 emulater */
	if (vga < 0) {
		printf("No VGA device found\n");
		return 0;
	} else if (!BootVideoCardBIOS(vga, NULL, 0)) {
		printf("VGA initialisation failed\n");
		return 0;
	}
	
	printf("Setting VESA Mode\n");
	/* VGA running...? */
	fbi = set_vesa_mode(0x111);
			
	if (fbi)
	{
		u8 *mmio_base;
		int i;
		printf("OK\n");
		printf("XSize = %d YSize = %d Base =0x%x\n", 
			fbi->XSize, fbi->YSize, fbi->BaseAddress);
		
		
		mmio_base = pci_bus_to_virt(vga, 0xffffffffu & (u64)fbi->BaseAddress,
					PCI_REGION_MEM, 0, MAP_NOCACHE);
		printf("mmio_base = 0x%p\n",
		       mmio_base);
		
		for(i = 0; i < 640*480*2; i++)
			*(mmio_base + i) = 0;
		
		
		grd.winSizeX = 640;
		grd.winSizeY = 480;
		grd.gdfBytesPP = 2;
		grd.frameAdrs = (uint)mmio_base;
		grd.gdfIndex = GDF_16BIT_565RGB;
		return &grd;
	}
	else
		printf("ERROR\n");
	
	
	return 0;
}

void ft_board_setup(void *blob, bd_t *bd)
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
}
