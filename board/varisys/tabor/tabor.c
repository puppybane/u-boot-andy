/*
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <asm/fsl_law.h>
#include <netdev.h>
#include <i2c.h>
#include <hwconfig.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_INITIAL     0x00018000
#define GPIO_OPENDRAIN   0x00018000

int board_early_init_f(void)
{
	volatile ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	volatile ccsr_gpio_t *gpio_3 = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0x200);

	/* Set pmuxcr to allow both i2c1 and i2c2, set 8 bit LBC */
	gur->pmuxcr = 0x40011030;
	/* Enable GPIO3 */
	gur->pmuxcr2 = 0xf7000000;
	/* Disable unused USB2 */
	gur->devdisr = 0x07400000;

	/* Read back the registers to synchronize the write. */
	in_be32(&gur->pmuxcr);
	in_be32(&gur->pmuxcr2);
	in_be32(&gur->devdisr);

	/* Init GPIO Direction */
	setbits_be32(&gpio_3->gpdat, GPIO_INITIAL);
	setbits_be32(&gpio_3->gpodr, GPIO_OPENDRAIN);
	
	/* Set GPIO Direction */
	setbits_be32(&gpio_3->gpdir, GPIO_OPENDRAIN);

	return 0;
}

int checkboard(void)
{

	printf("Board: Tabor, ");

	return 0;
}

#define CONFIG_TFP410_I2C_ADDR	0x38

int misc_init_r(void)
{
	u8 temp;
	size_t arglen;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* For DVI, enable the TFP410 Encoder. */

	temp = 0xBF;
	if (i2c_write(CONFIG_TFP410_I2C_ADDR, 0x08, 1, &temp, sizeof(temp)) < 0)
		return -1;
	if (i2c_read(CONFIG_TFP410_I2C_ADDR, 0x08, 1, &temp, sizeof(temp)) < 0)
		return -1;
	debug("DVI Encoder Read: 0x%02x\n", temp);

	temp = 0x10;
	if (i2c_write(CONFIG_TFP410_I2C_ADDR, 0x0A, 1, &temp, sizeof(temp)) < 0)
		return -1;
	if (i2c_read(CONFIG_TFP410_I2C_ADDR, 0x0A, 1, &temp, sizeof(temp)) < 0)
		return -1;
	debug("DVI Encoder Read: 0x%02x\n",temp);

	return 0;
}

/* Soft I2C for DVI */

#define GPIO_SCL 0x400
#define GPIO_SDA 0x800

void tabor_iic_init(void)
{
	volatile ccsr_gpio_t *gpio_3 = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0x200);

	setbits_be32(&gpio_3->gpdat, GPIO_SCL | GPIO_SDA);
	setbits_be32(&gpio_3->gpodr, GPIO_SCL | GPIO_SDA);
}

int tabor_iic_read(void)
{
	volatile ccsr_gpio_t *gpio_3 = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0x200);

	clrbits_be32(&gpio_3->gpdir, GPIO_SDA);
	return (in_be32(&gpio_3->gpdat) & GPIO_SDA) ? 1 : 0;
}

void tabor_iic_sda(int bit)
{
	volatile ccsr_gpio_t *gpio_3 = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0x200);

	if (bit) {
		setbits_be32(&gpio_3->gpdat, GPIO_SDA );
		clrbits_be32(&gpio_3->gpdir, GPIO_SDA);
	} else {
		setbits_be32(&gpio_3->gpdir, GPIO_SDA);
		clrbits_be32(&gpio_3->gpdat, GPIO_SDA);
	}
}

void tabor_iic_scl(int bit)
{
	volatile ccsr_gpio_t *gpio_3 = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR + 0x200);

	if (bit) {
		setbits_be32(&gpio_3->gpdat, GPIO_SCL);
		clrbits_be32(&gpio_3->gpdir, GPIO_SCL);
	} else {
		setbits_be32(&gpio_3->gpdir, GPIO_SCL);
		clrbits_be32(&gpio_3->gpdat, GPIO_SCL);
	}
}

/*
 * A list of PCI and SATA slots
 */
enum slot_id {
	SLOT_PCIE1 = 1,
	SLOT_PCIE2,
	SLOT_PCIE3,
	SLOT_PCIE4,
	SLOT_PCIE5,
	SLOT_SATA1,
	SLOT_SATA2
};

/*
 * This array maps the slot identifiers to their names on the P1022DS board.
 */
static const char *slot_names[] = {
	[SLOT_PCIE1] = "Slot 1",
	[SLOT_PCIE2] = "Slot 2",
	[SLOT_PCIE3] = "Slot 3",
	[SLOT_PCIE4] = "Slot 4",
	[SLOT_PCIE5] = "Mini-PCIe",
	[SLOT_SATA1] = "SATA 1",
	[SLOT_SATA2] = "SATA 2",
};

/*
 * This array maps a given SERDES configuration and SERDES device to the PCI or
 * SATA slot that it connects to.  This mapping is hard-coded in the FPGA.
 */
static u8 serdes_dev_slot[][SATA2 + 1] = {
	[0x01] = { [PCIE3] = SLOT_PCIE4, [PCIE2] = SLOT_PCIE5 },
	[0x02] = { [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x09] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE4,
		   [PCIE2] = SLOT_PCIE5 },
	[0x16] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x17] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3 },
	[0x1a] = { [PCIE1] = SLOT_PCIE1, [PCIE2] = SLOT_PCIE3,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1c] = { [PCIE1] = SLOT_PCIE1,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1e] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE3 },
	[0x1f] = { [PCIE1] = SLOT_PCIE1 },
};


/*
 * Returns the name of the slot to which the PCIe or SATA controller is
 * connected
 */
const char *board_serdes_name(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	unsigned int srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	enum slot_id slot = serdes_dev_slot[srds_cfg][device];
	const char *name = slot_names[slot];

	if (name)
		return name;
	else
		return "Nothing";
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif

int board_early_init_r(void)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	
	lbc->lbcr = 0;
	lbc->lcrr = 0x80000000 | CONFIG_SYS_LBC_LCRR;	/* 1 clock LALE cycle */
	
	
	return 0;
}

static void tabor_phy_tuning(int phy)
{
	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 */
	printf("Tuning PHY @ %d\n", phy);
	 
	// sets address 0x104 or reg 260 for writing
	miiphy_write(DEFAULT_MII_NAME, phy, 0xb, 0x8104); 
	// writes to address 0x104 , RXC/TXC to +0.96ns and TX_CTL/RX_CTL to -0.84ns
	miiphy_write(DEFAULT_MII_NAME, phy, 0xc, 0xf0f0); 
	// sets address 0x105 or reg 261 for writing
	miiphy_write(DEFAULT_MII_NAME, phy, 0xb, 0x8105);
	// writes to address 0x105 , RXD[3..0] to -0.
	miiphy_write(DEFAULT_MII_NAME, phy, 0xc, 0x0000);
	// sets address 0x106 or reg 261 for writing
	miiphy_write(DEFAULT_MII_NAME, phy, 0xb, 0x8106);
	// writes to address 0x106 , TXD[3..0] to -0.84ns
	miiphy_write(DEFAULT_MII_NAME, phy, 0xc, 0x0000);
	// force re-negotiation
	miiphy_write(DEFAULT_MII_NAME, phy, 0x0, 0x1340);	
}

/*
 * Initialize on-board and/or PCI Ethernet devices
 *
 * Returns:
 *      <0, error
 *       0, no ethernet devices found
 *      >0, number of ethernet devices initialized
 */
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[2];
	unsigned int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tabor_phy_tuning(TSEC1_PHY_ADDR);
	tabor_phy_tuning(TSEC2_PHY_ADDR);

	return tsec_eth_init(bis, tsec_info, num) + pci_eth_init(bis);
}

#ifdef CONFIG_OF_BOARD_SETUP
/**
 * ft_codec_setup - fix up the clock-frequency property of the codec node
 *
 * Update the clock-frequency property based on the value of the 'audclk'
 * hwconfig option.  If audclk is not specified, then don't write anything
 * to the device tree, because it means that the codec clock is disabled.
 */
static void ft_codec_setup(void *blob, const char *compatible)
{
	do_fixup_by_compat_u32(blob, compatible, "clock-frequency",
			       12288000, 1);
}

void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_HAS_FSL_DR_USB
	fdt_fixup_dr_usb(blob, bd);
#endif

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif

	/* Update the WM8776 node's clock frequency property */
	ft_codec_setup(blob, "wlf,wm8776");
}
#endif
