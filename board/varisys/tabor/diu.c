/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Authors: Timur Tabi <timur@freescale.com>
 *
 * FSL DIU Framebuffer driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <video_fb.h>
#include <fsl_diu_fb.h>

/* The CTL register is called 'csr' in the ngpixis_t structure */

#define PMUXCR_ELBCDIU_MASK	0xc0000000
#define PMUXCR_ELBCDIU_NOR16	0x80000000
#define PMUXCR_ELBCDIU_DIU	0x40000000

/*
 * DIU Area Descriptor
 *
 * Note that we need to byte-swap the value before it's written to the AD
 * register.  So even though the registers don't look like they're in the same
 * bit positions as they are on the MPC8610, the same value is written to the
 * AD register on the MPC8610 and on the P1022.
 */
#define AD_BYTE_F		0x10000000
#define AD_ALPHA_C_SHIFT	25
#define AD_BLUE_C_SHIFT		23
#define AD_GREEN_C_SHIFT	21
#define AD_RED_C_SHIFT		19
#define AD_PIXEL_S_SHIFT	16
#define AD_COMP_3_SHIFT		12
#define AD_COMP_2_SHIFT		8
#define AD_COMP_1_SHIFT		4
#define AD_COMP_0_SHIFT		0

#define GPIO_VGA_SWITCH 0x00000001

void diu_set_pixel_clock(unsigned int pixclock)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	unsigned long speed_ccb, temp;
	u32 pixval;

	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	debug("DIU pixval = %u\n", pixval);

	/* Modify PXCLK in GUTS CLKDVDR */
	temp = in_be32(&gur->clkdvdr) & 0x2000FFFF;
	out_be32(&gur->clkdvdr, temp);			/* turn off clock */
	out_be32(&gur->clkdvdr, temp | 0x80000000 | ((pixval & 0x1F) << 16));
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 pixel_format;


	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	printf("DIU:   Initialising @ %ux%u\n", xres, yres);

	/* Set PMUXCR to switch the muxed pins from the LBC to the DIU */
	clrsetbits_be32(&gur->pmuxcr, PMUXCR_ELBCDIU_MASK, PMUXCR_ELBCDIU_DIU);
	in_be32(&gur->pmuxcr);

	if (fsl_diu_init(grd.winSizeX, grd.winSizeY, pixel_format, 0) < 0)
		return NULL;

	/* fill in Graphic device struct */
	sprintf(grd.modeIdent, "%ix%ix%i %ikHz %iHz",
		grd.winSizeX, grd.winSizeY, 32, 64, 60);

	grd.frameAdrs = (unsigned int)diu_get_screen_base();
	grd.plnSizeX = grd.winSizeX;
	grd.plnSizeY = grd.winSizeY;

	grd.gdfBytesPP = 4;
	grd.gdfIndex = GDF_32BIT_X888RGB;

	grd.isaBase = 0;
	grd.pciBase = 0;
	grd.memSize = 800 * 600 * 4;

	/* Cursor Start Address */
	grd.dprBase = 0;
	grd.vprBase = 0;
	grd.cprBase = 0;

	return &grd;
}

void * video_hw_init(void)
{	
	volatile ccsr_gpio_t *gpio_2 = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0x100);
	pci_dev_t vga;
	u32 pins = in_be32(&gpio_2->gpdat);

	printf("Pins = 0x%x\n", pins);
   if ((pins & GPIO_VGA_SWITCH) != 0) {
		printf("Fit jumper to enable VGA\n");
		return 0;
	}

#ifdef CONFIG_ATI
	printf("Looking for VGA\n");
	vga = find_vga();
	//printf("PINS: 0x%08x\n", pins);
	
	/* PostBIOS with x86 emulater */
	if (vga < 0) {
		printf("No VGA device found\n");
		return init_diu();
	} else 
		return init_vga(vga);
#else

	return init_diu();
#endif
}
