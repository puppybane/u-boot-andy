/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 * Author: Timur Tabi <timur@freescale.com>
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

/*
 * This file handles the board muxing between the Fman Ethernet MACs and
 * the RGMII/SGMII/XGMII PHYs on a Freescale P3041/P5020 "Hydra" reference
 * board. The RGMII PHYs are the two on-board 1Gb ports.  The SGMII PHYs are
 * provided by the standard Freescale four-port SGMII riser card.  The 10Gb
 * XGMII PHY is provided via the XAUI riser card.  Since there is only one
 * Fman device on a P3041 and P5020, we only support one SGMII card and one
 * RGMII card.
 *
 * Muxing is handled via the PIXIS BRDCFG1 register.  The EMI1 bits control
 * muxing among the RGMII PHYs and the SGMII PHYs.  The value for RGMII is
 * always the same (0).  The value for SGMII depends on which slot the riser is
 * inserted in.  The EMI2 bits control muxing for the the XGMII.  Like SGMII,
 * the value is based on which slot the XAUI is inserted in.
 *
 * The SERDES configuration is used to determine where the SGMII and XAUI cards
 * exist, and also which Fman MACs are routed to which PHYs.  So for a given
 * Fman MAC, there is one and only PHY it connects to.  MACs cannot be routed
 * to PHYs dynamically.
 *
 *
 * This file also updates the device tree in three ways:
 *
 * 1) The status of each virtual MDIO node that is referenced by an Ethernet
 *    node is set to "okay".
 *
 * 2) The phy-handle property of each active Ethernet MAC node is set to the
 *    appropriate PHY node.
 *
 * 3) The "mux value" for each virtual MDIO node is set to the correct value,
 *    if necessary.  Some virtual MDIO nodes do not have configurable mux
 *    values, so those values are hard-coded in the DTS.  On the HYDRA board,
 *    the virtual MDIO node for the SGMII card needs to be updated.
 *
 * For all this to work, the device tree needs to have the following:
 *
 * 1) An alias for each PHY node that an Ethernet node could be routed to.
 *
 * 2) An alias for each real and virtual MDIO node that is disabled by default
 * and might need to be enabled, and also might need to have its mux-value
 * updated.
 */

#include <common.h>
#include <netdev.h>
#include <asm/fsl_serdes.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fdt_support.h>
#include <asm/fsl_dtsec.h>

//#include "../common/ngpixis.h"
#include "../common/fman.h"

#ifdef CONFIG_FMAN_ENET

/*
 * Given the following ...
 *
 * 1) A pointer to an Fman Ethernet node (as identified by the 'compat'
 * compatible string and 'addr' physical address)
 *
 * 2) An Fman port
 *
 * ... update the phy-handle property of the Ethernet node to point to the
 * right PHY.  This assumes that we already know the PHY for each port. 
 *
 * The offset of the Fman Ethernet node is also passed in for convenience, but
 * it is not used, and we recalculate the offset anyway.
 *
 * Note that what we call "Fman ports" (enum fm_port) is really an Fman MAC.
 * Inside the Fman, "ports" are things that connect to MACs.  We only call them
 * ports in U-Boot because on previous Ethernet devices (e.g. Gianfar), MACs
 * and ports are the same thing.
 *
 * Note that this code would be cleaner if had a function called
 * fm_info_get_phy_address(), which returns a value from the fm1_dtsec_info[]
 * array.  That's because all we're doing is figuring out the PHY address for
 * a given Fman MAC and writing it to the device tree.  Well, we already did
 * the hard work to figure that out in board_eth_init(), so it's silly to
 * repeat that here.
 */
void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
	if (port != FM1_DTSEC4) {
		return;
	}

	/* RGMII */
	/* The RGMII PHY is identified by the MAC connected to it */
	fdt_set_phy_handle(fdt, compat, addr, "phy_rgmii_0");

}

#endif /* #ifdef CONFIG_FMAN_ENET */

/*
 * Configure the status for the virtual MDIO nodes
 *
 * Rather than create the virtual MDIO nodes from scratch for each active
 * virtual MDIO, we expect the DTS to have the nodes defined already, and we
 * only enable the ones that are actually active.
 *
 * We assume that the DTS already hard-codes the status for all the
 * virtual MDIO nodes to "disabled", so all we need to do is enable the
 * active ones.
 *
 * For SGMII, we also need to set the mux value in the node.
 */
void fdt_fixup_board_enet(void *fdt)
{
#ifdef CONFIG_FMAN_ENET
		fdt_status_okay_by_alias(fdt, "emi1_rgmii");
#endif
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct fsl_pq_mdio_info dtsec_mdio_info;
	unsigned int i, slot;
	int lane;

	printf("Initializing Fman\n");

	/* Register the real 1G MDIO bus */
	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	
	/*
	 * DTSEC4 is RGMII, but connected to PHY address 3 unlike the development system
	 */
	fm_info_set_phy_address(FM1_DTSEC4, 3);
	fm_info_set_mdio(FM1_DTSEC4,
		miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));

	// Never disable DTSEC1 - it controls MDIO
	for (i = FM1_DTSEC2; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		if (i != FM1_DTSEC4) {
			fm_disable_port(i);
		
		}
	}

	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
