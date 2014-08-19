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

#ifndef __CONFIG_CYRUS_H
#define __CONFIG_CYRUS_H

/*
 * P5020 DS board configuration file
 * Also supports P5010 DS
 */
#define CONFIG_CYRUS

#define CONFIG_PHYS_64BIT

#if !defined(CONFIG_PPC_P5020) && !defined(CONFIG_PPC_P3041) && !defined(CONFIG_PPC_P5040)
#error Must call Cyrus CONFIG with a specific CPU enabled.
#endif


#define CONFIG_MMC
#define CONFIG_SDCARD
#define CONFIG_FSL_SATA_V2
#define CONFIG_PCIE3
#define CONFIG_PCIE4
#ifdef CONFIG_PPC_P5020
#define CONFIG_SYS_FSL_RAID_ENGINE
#define CONFIG_SYS_DPAA_RMAN
#endif
#define CONFIG_SYS_DPAA_PME

/* Enable splash screen */
#define CONFIG_PREBOOT

/* Enable Amiga Boot screen */
#define CONFIG_CMD_AMIGABOOT
#define CONFIG_CMD_SYSINFO
#define CONFIG_CMD_BOOTOPTIONS
#define CONFIG_CMD_START_AMIGAOS
#define CONFIG_CMD_START_AMIGACLASSIC
#define CONFIG_CMD_START_LINUX
 /* Start offset on SD card */
#define CONFIG_AMIGABOOT_BLOCK_OFFS                0x4000   
/* #blks to read */     
#define CONFIG_AMIGABOOT_BLOCK_LEN                300 
/* MCC device to load from */               
#define CONFIG_AMIGABOOT_MCC_DEV                0                
/*
 * Corenet DS style board configuration file
 */
#ifdef CONFIG_RAMBOOT_PBL
#define CONFIG_RAMBOOT_TEXT_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_RESET_VECTOR_ADDRESS	0xfffffffc
#define CONFIG_SYS_FSL_PBL_PBI $(SRCTREE)/board/varisys/cyrus/pbi.cfg
#if defined(CONFIG_PPC_P3041)
#define CONFIG_SYS_CLK_FREQ 133000000
#define CONFIG_SYS_FSL_PBL_RCW $(SRCTREE)/board/varisys/cyrus/rcw_p3041.cfg
#elif defined(CONFIG_CYRUS_V2)
#define CONFIG_SYS_CLK_FREQ 133000000
#define CONFIG_SYS_FSL_PBL_RCW $(SRCTREE)/board/varisys/cyrus/rcw_p5020_v2.cfg
#elif defined(CONFIG_PPC_P5020)
#define CONFIG_SYS_CLK_FREQ 133000000
#define CONFIG_SYS_FSL_PBL_RCW $(SRCTREE)/board/varisys/cyrus/rcw_p5020.cfg
#elif defined(CONFIG_PPC_P5040)
#define CONFIG_SYS_CLK_FREQ 100000000
#define CONFIG_SYS_FSL_PBL_RCW $(SRCTREE)/board/varisys/cyrus/rcw_p5040.cfg
#endif
#endif


/* High Level Configuration Options */
#define CONFIG_BOOKE
#define CONFIG_E500			/* BOOKE e500 family */
#define CONFIG_E500MC			/* BOOKE e500mc family */
#define CONFIG_SYS_BOOK3E_HV		/* Category E.HV supported */
#define CONFIG_MP			/* support multiple processors */


#define CONFIG_API
#define CONFIG_SYS_MMC_MAX_DEVICE     1

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xeff80000
#endif

/*
#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif
*/

#define CONFIG_SYS_FSL_CPC		/* Corenet Platform Cache */
#define CONFIG_SYS_NUM_CPC		CONFIG_NUM_DDR_CONTROLLERS
#define CONFIG_FSL_ELBC			/* Has Enhanced localbus controller */
#define CONFIG_PCI			/* Enable PCI/PCIE */
#define CONFIG_PCIE1			/* PCIE controler 1 */
#define CONFIG_PCIE2			/* PCIE controler 2 */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_FSL_LAW			/* Use common FSL init code */

#define CONFIG_BIOSEMU
#define VIDEO_IO_OFFSET		CONFIG_SYS_PCIE1_IO_VIRT

#define CONFIG_ENV_OVERWRITE

#define CONFIG_SYS_NO_FLASH

#if defined(CONFIG_SDCARD)
#define CONFIG_SYS_EXTRA_ENV_RELOC
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		(512 * 1097)
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_SYS_CACHE_STASHING
#define CONFIG_BACKSIDE_L2_CACHE
#define CONFIG_SYS_INIT_L2CSR0		L2CSR0_L2E
#define CONFIG_BTB			/* toggle branch predition */
#define	CONFIG_DDR_ECC
#ifdef CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#endif

#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		64	/* number of TLB1 entries */
#endif

/* test POST memory test */
#define CONFIG_POST CONFIG_SYS_POST_MEMORY
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000
#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_PANIC_HANG	/* do not reset board on panic */

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CONFIG_SYS_INIT_L3_ADDR		CONFIG_RAMBOOT_TEXT_BASE
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	(0xf00000000ull | CONFIG_RAMBOOT_TEXT_BASE)
#else
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	CONFIG_SYS_INIT_L3_ADDR
#endif
#define CONFIG_SYS_L3_SIZE		(1024 << 10)
#define CONFIG_SYS_INIT_L3_END (CONFIG_SYS_INIT_L3_ADDR + CONFIG_SYS_L3_SIZE)

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_DCSRBAR		0xf0000000
#define CONFIG_SYS_DCSRBAR_PHYS		0xf00000000ull
#endif

/*
 * DDR Setup
 */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(4 * CONFIG_DIMM_SLOTS_PER_CTLR)

#define CONFIG_DDR_SPD
#define CONFIG_SYS_FSL_DDR3

#ifdef CONFIG_P3060QDS
#define CONFIG_SYS_SPD_BUS_NUM	0
#else
#define CONFIG_SYS_SPD_BUS_NUM	1
#endif
#if defined(CONFIG_PPC_P5020) || defined(CONFIG_PPC_P5040)
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#else
#define SPD_EEPROM_ADDRESS	0x51
#endif
#define CONFIG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/*
 * Local Bus Definitions
 */

#define CONFIG_SYS_LBC0_BASE		0xe0000000	/* Start of LBC Registers */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_LBC0_BASE_PHYS	0xfe0000000ull
#else
#define CONFIG_SYS_LBC0_BASE_PHYS	CONFIG_SYS_LBC0_BASE
#endif

#define CONFIG_SYS_LBC1_BASE		0xe1000000	/* Start of LBC Registers */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_LBC1_BASE_PHYS	0xfe1000000ull
#else
#define CONFIG_SYS_LBC1_BASE_PHYS	CONFIG_SYS_LBC1_BASE
#endif

/* Set the local bus clock 1/16 of platform clock */
#define CONFIG_SYS_LBC_LCRR		(LCRR_CLKDIV_16 | LCRR_EADC_1)

#define CONFIG_SYS_BR0_PRELIM \
(BR_PHYS_ADDR(CONFIG_SYS_LBC0_BASE_PHYS) | BR_PS_16 | BR_V)
#define CONFIG_SYS_BR1_PRELIM \
(BR_PHYS_ADDR(CONFIG_SYS_LBC1_BASE_PHYS) | BR_PS_16 | BR_V)

#define CONFIG_SYS_OR0_PRELIM	0xfff00010
#define CONFIG_SYS_OR1_PRELIM	0xfff00010


#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if defined(CONFIG_RAMBOOT_PBL)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R	/* call board_early_init_r function */
#define CONFIG_MISC_INIT_R

#define CONFIG_HWCONFIG

/* define to use L1 as initial stack */
#define CONFIG_L1_INIT_RAM
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xfdd00000	/* Initial L1 address */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR
/* The assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS	CONFIG_SYS_INIT_RAM_ADDR /* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR_PHYS
#endif
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000	/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0)/2)

#define CONFIG_SYS_BAUDRATE_TABLE	\
{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x11C500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x11C600)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_CCSRBAR+0x11D500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_CCSRBAR+0x11D600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

/* new uImage format support */
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE	/* enable fit_format_{error,warning}() */

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CONFIG_SYS_FSL_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET		0x118000
#define CONFIG_SYS_FSL_I2C2_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C2_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET		0x118100
#define CONFIG_SYS_FSL_I2C3_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C3_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C3_OFFSET		0x119000
#define CONFIG_SYS_FSL_I2C4_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C4_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C4_OFFSET		0x119100

#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

#define CONFIG_CMD_DATE			1
#define CONFIG_RTC_MCP7411		1
#define CONFIG_SYS_RTC_BUS_NUM		3
#define CONFIG_SYS_I2C_RTC_ADDR		0x6f

/*
 * eSPI - Enhanced SPI
 */
#define CONFIG_FSL_ESPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED         10000000
#define CONFIG_SF_DEFAULT_MODE          0

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 1, direct to uli, tgtid 3, Base address 20000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xff8000000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xf8000000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 2, Slot 2, tgtid 2, Base address 201000 */
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xf8010000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xff8010000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xf8010000
#endif
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 3, Slot 1, tgtid 1, Base address 202000 */
#define CONFIG_SYS_PCIE3_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_PCIE3_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xf8020000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_IO_PHYS	0xff8020000ull
#else
#define CONFIG_SYS_PCIE3_IO_PHYS	0xf8020000
#endif
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000	/* 64k */

/* controller 4, Base address 203000 */
#define CONFIG_SYS_PCIE4_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE4_MEM_PHYS	0xc60000000ull
#define CONFIG_SYS_PCIE4_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE4_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE4_IO_PHYS	0xff8030000ull
#define CONFIG_SYS_PCIE4_IO_SIZE	0x00010000	/* 64k */

/* Qman/Bman */
#define CONFIG_SYS_DPAA_QBMAN		/* Support Q/Bman */
#define CONFIG_SYS_BMAN_NUM_PORTALS	10
#define CONFIG_SYS_BMAN_MEM_BASE	0xf4000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#else
#define CONFIG_SYS_BMAN_MEM_PHYS	CONFIG_SYS_BMAN_MEM_BASE
#endif
#define CONFIG_SYS_BMAN_MEM_SIZE	0x00200000
#define CONFIG_SYS_QMAN_NUM_PORTALS	10
#define CONFIG_SYS_QMAN_MEM_BASE	0xf4200000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_QMAN_MEM_PHYS	0xff4200000ull
#else
#define CONFIG_SYS_QMAN_MEM_PHYS	CONFIG_SYS_QMAN_MEM_BASE
#endif
#define CONFIG_SYS_QMAN_MEM_SIZE	0x00200000

#define CONFIG_SYS_DPAA_FMAN
/* Default address of microcode for the Linux Fman driver */
#if defined(CONFIG_SPIFLASH)
/*
 * env is stored at 0x100000, sector size is 0x10000, ucode is stored after
 * env, so we got 0x110000.
 */
#define CONFIG_SYS_QE_FW_IN_SPIFLASH
#define CONFIG_SYS_QE_FMAN_FW_ADDR	0x110000
#elif defined(CONFIG_SDCARD)
/*
 * PBL SD boot image should stored at 0x1000(8 blocks), the size of the image is
 * about 545KB (1089 blocks), Env is stored after the image, and the env size is
 * 0x2000 (16 blocks), 8 + 1089 + 16 = 1113, enlarge it to 1130.
 */
#define CONFIG_SYS_QE_FMAN_FW_IN_MMC
#define CONFIG_SYS_QE_FMAN_FW_ADDR	(512 * 1130)
#endif

#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)

#ifdef CONFIG_SYS_DPAA_FMAN
#define CONFIG_FMAN_ENET
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9021
#endif

#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCI_PNP			/* do pci plug-and-play */
#undef CONFIG_E1000
#define CONFIG_NET_MULTI

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_DOS_PARTITION
#endif	/* CONFIG_PCI */

/* SATA */
#ifdef CONFIG_FSL_SATA_V2
#define CONFIG_LIBATA
#define CONFIG_FSL_SATA
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	2
#define CONFIG_SYS_SCSI_MAX_LUN	1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_SYS_SCSI_MAXDEVICE	CONFIG_SYS_SCSI_MAX_DEVICE

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#define CONFIG_LBA48
#define CONFIG_CMD_SATA
#define CONFIG_CMD_SCSI
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION
#define CONFIG_CMD_PART
#define CONFIG_CMD_EXT2
#define CONFIG_FS_ISO9660
#endif

#ifdef CONFIG_FMAN_ENET
#define CONFIG_SYS_FM1_DTSEC4_PHY_ADDR	0x6
#define CONFIG_SYS_FM1_10GEC1_PHY_ADDR	4

#define CONFIG_SYS_FM2_DTSEC4_PHY_ADDR	0x1f
#define CONFIG_SYS_FM2_10GEC1_PHY_ADDR	0

#define CONFIG_SYS_TBIPA_VALUE	8
#define CONFIG_MII		/* MII PHY management */
#define CONFIG_ETHPRIME		"FM1@DTSEC4"
#define CONFIG_PHY_GIGE		/* Include GbE speed/duplex detection */
#endif

/*
 * Environment
 */
#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_ERRATA
#define CONFIG_CMD_GREPENV
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_CMD_NET
#endif

/*
 * USB
 */
#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_HAS_FSL_MPH_USB

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_CMD_EXT2
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_KEYBOARD 
#define CONFIG_SYS_USB_EVENT_POLL
 /* _VIA_CONTROL_EP  */
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#endif

#ifdef CONFIG_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR       CONFIG_SYS_MPC85xx_ESDHC_ADDR
#define CONFIG_SYS_FSL_ESDHC_BROKEN_TIMEOUT
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_FS_GENERIC
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING			/* Command-line editing */
#define CONFIG_AUTO_COMPLETE			/* add autocompletion support */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"X5000> "		/* Monitor Command Prompt */
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY 	10	/* -1 disables auto-boot */
#define CONFIG_CMD_BOOTMENU		/* ANSI terminal Boot Menu */
#define CONFIG_MENU
#undef CONFIG_MENU_SHOW
 #define CONFIG_ESCAPEBOOTMENU "setenv stdout serial,vga"

#define CONFIG_BAUDRATE	115200

#if defined(CONFIG_P4080DS) || defined(CONFIG_P3060QDS)
#define __USB_PHY_TYPE	ulpi
#else
#define __USB_PHY_TYPE	utmi
#endif

/* Video support */
#define CONFIG_VIDEO
#define VIDEO_FB_LITTLE_ENDIAN
#define CONFIG_CFB_CONSOLE
#define CONFIG_CFB_CONSOLE_ANSI
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_CMD_BMP
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE  (640*480*2 + 1024)

#define CONFIG_SYS_CONSOLE_FG_COL	0x00
#define CONFIG_SYS_CONSOLE_BG_COL	170

#define	CONFIG_EXTRA_ENV_SETTINGS \
 "hwconfig=fsl_ddr:ctlr_intlv=cacheline,"		\
"bank_intlv=cs0_cs1;"					\
"usb1:dr_mode=host,phy_type=" __stringify(__USB_PHY_TYPE) ";"\
"usb2:dr_mode=host,phy_type=" __stringify(__USB_PHY_TYPE) "\0"\
"netdev=eth0\0"						\
"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
"ubootaddr=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"			\
"consoledev=ttyS0\0"					\
"ramdiskaddr=2000000\0"					\
"ramdiskfile=p4080ds/ramdisk.uboot\0"			\
"fdtaddr=c00000\0"					\
"fdtfile=p4080ds/p4080ds.dtb\0"				\
"bdev=sda3\0"						\
"c=ffe\0"

#define CONFIG_HDBOOT					\
"setenv bootargs root=/dev/$bdev rw "		\
"console=$consoledev,$baudrate $othbootargs;"	\
"tftp $loadaddr $bootfile;"			\
"tftp $fdtaddr $fdtfile;"			\
"bootm $loadaddr - $fdtaddr"

#define CONFIG_NFSBOOTCOMMAND			\
"setenv bootargs root=/dev/nfs rw "	\
"nfsroot=$serverip:$rootpath "		\
"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
"console=$consoledev,$baudrate $othbootargs;"	\
"tftp $loadaddr $bootfile;"		\
"tftp $fdtaddr $fdtfile;"		\
"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND				\
"setenv bootargs root=/dev/ram rw "		\
"console=$consoledev,$baudrate $othbootargs;"	\
"tftp $ramdiskaddr $ramdiskfile;"		\
"tftp $loadaddr $bootfile;"			\
"tftp $fdtaddr $fdtfile;"			\
"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#ifdef CONFIG_SECURE_BOOT
#include <asm/fsl_secure_boot.h>
#endif

#endif	/* __CONFIG_H */

/* end #include "corenet_ds.h" */

