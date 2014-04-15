/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

unsigned long get_board_sys_clk(void) {
	return 66660000;
}

unsigned long get_board_ddr_clk(void) {
	return 66660000;
}
