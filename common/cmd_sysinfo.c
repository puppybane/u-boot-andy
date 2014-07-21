/*
 * (C) Copyright 2014 Humboldt
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <ansi.h>
#include <bmp_layout.h>
#include <menu.h>
#include <hush.h>
#include <watchdog.h>
#include <malloc.h>
#include <linux/string.h>
#include <linux/compiler.h>
#include <usb.h>
#include <pci.h>
#include <scsi.h>
#include <sata.h>
#include <video.h>
#include <rtc.h>
#include <i2c.h>
#include <version.h>
#include <fsl_ddr.h>

DECLARE_GLOBAL_DATA_PTR;

extern block_dev_desc_t scsi_dev_desc[CONFIG_SYS_SCSI_MAX_DEVICE];
extern block_dev_desc_t sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];
extern int buttonpos ;
extern int leftboxpos ;
extern int rightboxpos ;
extern char dimm_name[] ;

/* Function Declarations */
void amigabootmenu_clear_screen( void ) ;
extern void video_drawstring(int xx, int yy, unsigned char *s) ;
void sysinfo_draw_titled_box(int x, int y, int w, int h, char * title) ;
extern struct bmp_image *gunzip_bmp(unsigned long addr, unsigned long *lenp, void **alloc_addr);
int do_amigabootmenu(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]) ;
char *usb_get_class_desc(unsigned char dclass) ;
void pci_header_show(pci_dev_t dev);

/*
 * Subroutine:  sysinfo_pciinfo
 *
 * Description: Show information about devices on PCI bus.
 *				 Depending on the define CONFIG_SYS_SHORT_PCI_LISTING
 *				 the output will be more or less exhaustive.
 *
 * Inputs:	 bus_no	  the number of the bus to be scanned.
 *
 * Return:	 None
 *
 */
void sysinfo_pciinfo(int BusNum, int ShortPCIListing)
{
	int Device;
	int Function;
	unsigned char HeaderType;
	unsigned short VendorID;
	pci_dev_t dev;
	char data_buffer[64] ;
	u16 vendor, device;
	u8 class, subclass;

//	printf("Scanning PCI devices on bus %d\n", BusNum);

	if (ShortPCIListing) {
//		printf("BusDevFun  VendorId   DeviceId   Device Class	  Sub-Class\n");
//		printf("_____________________________________________________________\n");
	}

	for (Device = 0; Device < PCI_MAX_PCI_DEVICES; Device++) {
		HeaderType = 0;
		VendorID = 0;
		for (Function = 0; Function < PCI_MAX_PCI_FUNCTIONS; Function++) {
			/*
			 * If this is not a multi-function device, we skip the rest.
			 */
			if (Function && !(HeaderType & 0x80))
				break;

			dev = PCI_BDF(BusNum, Device, Function);

			pci_read_config_word(dev, PCI_VENDOR_ID, &VendorID);
			if ((VendorID == 0xFFFF) || (VendorID == 0x0000))
				continue;

			if (!Function) pci_read_config_byte(dev, PCI_HEADER_TYPE, &HeaderType);
			if (ShortPCIListing)
			{
				sprintf(data_buffer, "%02x.%02x.%02x ", BusNum, Device, Function);
				video_drawstring(420, 50 + (Function * 32), (unsigned char *)data_buffer)  ;

				pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
				pci_read_config_word(dev, PCI_DEVICE_ID, &device);
				pci_read_config_byte(dev, PCI_CLASS_CODE, &class);
				pci_read_config_byte(dev, PCI_CLASS_SUB_CODE, &subclass);

				sprintf(data_buffer, "0x%.4x 0x%.4x %-16s 0x%.2x",
		  				vendor, device,
		  				pci_class_str(class), subclass);
				video_drawstring(420, 66 + (Function * 32), (unsigned char *)data_buffer)  ;
			}
			else
			{
				printf("\nFound PCI device %02x.%02x.%02x:\n",
					  BusNum, Device, Function);
				pci_header_show(dev);
			}
		}
    	}
}

void sysinfo_board_add_ram_info(char *data_buffer)
{
	char tmp[64] ;
	struct ccsr_ddr __iomem *ddr =
		(struct ccsr_ddr __iomem *)(CONFIG_SYS_FSL_DDR_ADDR);

#if	defined(CONFIG_E6500) && (CONFIG_NUM_DDR_CONTROLLERS == 3)
	u32 *mcintl3r = (void *) (CONFIG_SYS_IMMR + 0x18004);
#endif
#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
//	uint32_t cs0_config = ddr_in32(&ddr->cs0_config);
#endif
	uint32_t sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	int cas_lat;
	dimm_params_t *pdimm;
	fsl_ddr_info_t info;

#if CONFIG_NUM_DDR_CONTROLLERS >= 2
	if (!(sdram_cfg & SDRAM_CFG_MEM_EN)) {
		ddr = (void __iomem *)CONFIG_SYS_FSL_DDR2_ADDR;
		sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	}
#endif
#if CONFIG_NUM_DDR_CONTROLLERS >= 3
	if (!(sdram_cfg & SDRAM_CFG_MEM_EN)) {
		ddr = (void __iomem *)CONFIG_SYS_FSL_DDR3_ADDR;
		sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	}
#endif

#ifdef ooo
	fsl_ddr_get_dimm_params(pdimm,
                        controller_number,
                        dimm_number) ;
#endif
//	fsl_ddr_compute(&info, STEP_GET_SPD, 0);

//	strcpy(data_buffer, info.dimm_params[0][0].mpart) ;
//	strcpy(data_buffer, dimm_name) ;

	strcat(data_buffer, "DDR");
	switch ((sdram_cfg & SDRAM_CFG_SDRAM_TYPE_MASK) >>
		SDRAM_CFG_SDRAM_TYPE_SHIFT) {
	case SDRAM_TYPE_DDR1:
		strcat(data_buffer, "1");
		break;
	case SDRAM_TYPE_DDR2:
		strcat(data_buffer, "2");
		break;
	case SDRAM_TYPE_DDR3:
		strcat(data_buffer, "3");
		break;
	default:
		strcat(data_buffer, "?");
		break;
	}

	if (sdram_cfg & SDRAM_CFG_32_BE)
		strcat(data_buffer, " 32-bit");
	else if (sdram_cfg & SDRAM_CFG_16_BE)
		strcat(data_buffer, " 16-bit");
	else
		strcat(data_buffer, " 64-bit");

	/* Calculate CAS latency based on timing cfg values */
	cas_lat = ((ddr_in32(&ddr->timing_cfg_1) >> 16) & 0xf) + 1;
	if ((ddr_in32(&ddr->timing_cfg_3) >> 12) & 1)
		cas_lat += (8 << 1);
	sprintf(tmp, " CL=%d", cas_lat >> 1);
	strcat(data_buffer, tmp) ;
	if (cas_lat & 0x1)
		strcat(data_buffer, ".5");

	if (sdram_cfg & SDRAM_CFG_ECC_EN)
		strcat(data_buffer, " ECC on");
	else
		strcat(data_buffer, " ECC off");

	return  ;
}

static void sysinfo_usb_display_class_sub(unsigned char dclass, unsigned char subclass,
				  unsigned char proto, char *data_buffer)
{
	memset(data_buffer, '\0', 256) ;

	switch (dclass) {
	case USB_CLASS_PER_INTERFACE:
		strcat(data_buffer, "See I/f");
		break;
	case USB_CLASS_HID:
		strcat(data_buffer, "Human I/f, Sub: ");
		switch (subclass) {
		case USB_SUB_HID_NONE:
			strcat(data_buffer, "None");
			break;
		case USB_SUB_HID_BOOT:
			strcat(data_buffer, "Boot ");
			switch (proto) {
			case USB_PROT_HID_NONE:
				strcat(data_buffer, "None");
				break;
			case USB_PROT_HID_KEYBOARD:
				strcat(data_buffer, "Keyboard");
				break;
			case USB_PROT_HID_MOUSE:
				strcat(data_buffer, "Mouse");
				break;
			default:
				strcat(data_buffer, "reserved");
				break;
			}
			break;
		default:
			strcat(data_buffer, "reserved");
			break;
		}
		break;
	case USB_CLASS_MASS_STORAGE:
		strcat(data_buffer, "Mass Storage, ");
		switch (subclass) {
		case US_SC_RBC:
			strcat(data_buffer, "RBC ");
			break;
		case US_SC_8020:
			strcat(data_buffer, "SFF-8020i (ATAPI)");
			break;
		case US_SC_QIC:
			strcat(data_buffer, "QIC-157 (Tape)");
			break;
		case US_SC_UFI:
			strcat(data_buffer, "UFI");
			break;
		case US_SC_8070:
			strcat(data_buffer, "SFF-8070");
			break;
		case US_SC_SCSI:
			strcat(data_buffer, "Transp. SCSI");
			break;
		default:
			strcat(data_buffer, "reserved");
			break;
		}
		strcat(data_buffer, ", ");
		switch (proto) {
		case US_PR_CB:
			strcat(data_buffer, "Command/Bulk");
			break;
		case US_PR_CBI:
			strcat(data_buffer, "Command/Bulk/Int");
			break;
		case US_PR_BULK:
			strcat(data_buffer, "Bulk only");
			break;
		default:
			strcat(data_buffer, "reserved");
			break;
		}
		break;
	default:
		sprintf(data_buffer, "%s", usb_get_class_desc(dclass));
		break;
	}

	return ;
}


void sysinfo_usb_display_desc(struct usb_device *dev, int dev_no)
{
	char data_buffer[256] ;
	int offset = 204 ;

	if (dev->descriptor.bDescriptorType == USB_DT_DEVICE) {
		sprintf(data_buffer, "%d: %s,  USB Revision %x.%x", dev->devnum,
		usb_get_class_desc(dev->config.if_desc[0].desc.bInterfaceClass),
				   (dev->descriptor.bcdUSB>>8) & 0xff,
				   dev->descriptor.bcdUSB & 0xff);
		video_drawstring(420, offset+(dev_no*80), (unsigned char *)data_buffer) ;

		if (strlen(dev->mf) || strlen(dev->prod) ||
		    strlen(dev->serial)) {
			sprintf(data_buffer, "%s %s %s", dev->mf, dev->prod,
				dev->serial);
			video_drawstring(420, offset+16+(dev_no*80), (unsigned char *)data_buffer) ;
		}
		if (dev->descriptor.bDeviceClass) {
			char tmp_buffer[256] ;
			sysinfo_usb_display_class_sub(dev->descriptor.bDeviceClass,
						 dev->descriptor.bDeviceSubClass,
						 dev->descriptor.bDeviceProtocol,
						 tmp_buffer) ;
			sprintf(data_buffer,"Class: %s", tmp_buffer) ;
			video_drawstring(420, offset+32+(dev_no*80), (unsigned char *)data_buffer) ;
		} else {
			sprintf(data_buffer, "Class: (from I/f) %s",
				  usb_get_class_desc(
				dev->config.if_desc[0].desc.bInterfaceClass));
			video_drawstring(420, offset+32+(dev_no*80), (unsigned char *)data_buffer) ;
		}
		sprintf(data_buffer, "Packet Size: %d Configurations: %d",
			dev->descriptor.bMaxPacketSize0,
			dev->descriptor.bNumConfigurations);
		video_drawstring(420, offset+48+(dev_no*80), (unsigned char *)data_buffer) ;
		sprintf(data_buffer, "Vendor: 0x%04x Prod 0x%04x Ver %d.%d",
			dev->descriptor.idVendor, dev->descriptor.idProduct,
			(dev->descriptor.bcdDevice>>8) & 0xff,
			dev->descriptor.bcdDevice & 0xff);
		video_drawstring(420, offset+64+(dev_no*80), (unsigned char *)data_buffer) ;
	}

}


static void sysinfo_show(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int i;
	struct usb_device *dev ;
	int  ii ;
	char data_buffer[128] ;
	char memory_info[128] ;
	char devtype[64] ,c ;
	typedef struct button6 button ;
	char 	cpu_type[32] ;
	char	cpu_core1[32] ;
	ulong addr = 0x1000e000 ;
	ulong addr_inv = 0x10028000 ;
	bmp_image_t *bmp = (bmp_image_t *)addr;
	void *bmp_alloc_addr = NULL;
	int reverse=0, len ;
	int displayed = 0 ;
	struct rtc_time tm;
	int rcode = 0;
	int old_bus ;
	struct cpu_type *cpu ;
	uint svr, pvr, ver, major, minor ;

	amigabootmenu_clear_screen() ;

	video_drawstring(250, 5, (unsigned char *)"<<<<<< AmigaONE System Information >>>>>>")  ;

	/* CPU */
	sysinfo_draw_titled_box(5, 40, 384, 64, " CPU ") ;

	/* Type : 	Rev: 	Speed: */
	cpu = gd->arch.cpu ;
	sprintf(cpu_type, "%s", cpu->name) ;
	svr = get_svr();
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);
	sprintf(data_buffer, 
		"Type: %s Rev: %d.%d Speed: %4ldMHz", 
		cpu_type, major, minor, (gd->bd->bi_intfreq / 1000000)) ;

	video_drawstring(10, 48, (unsigned char *)data_buffer)  ;

	/* Core1 : 	Core2: 	FSB: */
	pvr = get_pvr();
	ver = PVR_VER(pvr);
	major = PVR_MAJ(pvr);
	minor = PVR_MIN(pvr);

	switch(ver) {
	case PVR_VER_E500_V1:
	case PVR_VER_E500_V2:
		strcpy(cpu_core1, "e500");
		break;
	case PVR_VER_E500MC:
		strcpy(cpu_core1, "e500mc");
		break;
	case PVR_VER_E5500:
		strcpy(cpu_core1, "e5500");
		break;
	case PVR_VER_E6500:
		strcpy(cpu_core1, "e6500");
		break;
	default:
		strcpy(cpu_core1, "Unknown");
		break;
	}
	sprintf(data_buffer, "Core1: %s Ver: %d.%d FSB: %3ldMHz", 
		cpu_core1, major, minor, (gd->bd->bi_busfreq / 1000000)) ;
	video_drawstring(10, 64, (unsigned char *)data_buffer)  ;

	/* Temperature */
//do_dtt() ;
	sprintf(data_buffer, "Temperature: N/A") ;
	video_drawstring(10, 80, (unsigned char *)data_buffer)  ;

	/*SATA Devices */
	sysinfo_draw_titled_box(5, 120, 384, 96, " SATA Devices ") ;

	/* SATA info */
	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; ++i) {
		if (sata_dev_desc[i].type == DEV_TYPE_UNKNOWN)
			continue;
		switch (sata_dev_desc[i].type & 0x1F) {
		case DEV_TYPE_HARDDISK:
			strcpy(devtype, "Hard Disk");
			break;
		case DEV_TYPE_CDROM:
			strcpy(devtype, "CD ROM");
			break;
		case DEV_TYPE_OPDISK:
			strcpy(devtype, "Optical Device");
			break;
		case DEV_TYPE_TAPE:
			strcpy(devtype, "Tape");
			break;
		default:
			sprintf (devtype, "# %02X #", sata_dev_desc[i].type & 0x1F);
			break;
		}

//		sprintf(data_buffer, 
//			"SATA device %d: Model: %s Firm: %s Ser#: %s",
//			"SATA device %d: Model: %s Type: %s",
//			i,
//			sata_dev_desc[i].vendor,
//			sata_dev_desc[i].revision,
//			sata_dev_desc[i].product);
		sprintf(data_buffer, 
			"SATA dev %d: Mod: %s Typ: %s",
			i,
			sata_dev_desc[i].vendor,
			devtype);
		video_drawstring(10, 130 + (i * 16), (unsigned char *)data_buffer) ;
	}

	/* SCSI info */
	for (i=0; i<CONFIG_SYS_SCSI_MAX_DEVICE; ++i) {
		if(scsi_dev_desc[i].type==DEV_TYPE_UNKNOWN)
			continue; /* list only known devices */
		sprintf (data_buffer, "SCSI dev. %d: (%d:%d) Vend: %s", 
			i,
			scsi_dev_desc[i].target,
			scsi_dev_desc[i].lun,
			scsi_dev_desc[i].vendor) ;
		video_drawstring(10, 162 + (i * 32), (unsigned char *)data_buffer) ;
		sprintf (data_buffer, 
			"Prod.: %s Rev: %s",
			scsi_dev_desc[i].product,
			scsi_dev_desc[i].revision);
		video_drawstring(10, 178 + (i * 32), (unsigned char *)data_buffer) ;
	}

	/* Memory */
	sysinfo_draw_titled_box(5, 230, 384, 96, " Memory ") ;

	/* DIMM 0: */
//	sprintf(data_buffer, "DIMM 0: %lx", (unsigned long)(bd->bi_memsize)) ;
	sysinfo_board_add_ram_info(memory_info) ;
	sprintf(data_buffer, "DIMM 0: %1ldGb %s", 
		(unsigned long)(gd->ram_size / 1073741824L), memory_info) ;
	video_drawstring(10, 240, (unsigned char *)data_buffer)  ;

	/* DIMM 1: */
	sprintf(data_buffer, "DIMM 1: %s", "")  ;
	video_drawstring(10, 272, (unsigned char *)data_buffer)  ;

	/* Ethernet */
	sysinfo_draw_titled_box(5, 338, 384, 80, " Ethernet ") ;

	/* MAC Address : */
	sprintf(data_buffer, "MAC Address: %s", getenv("ethaddr")) ;
	video_drawstring(10, 348, (unsigned char *)data_buffer)  ;

	/* IP Address : */
//	sprintf(data_buffer, "IP Address: %u", getenv("ipaddr") ;
	sprintf(data_buffer, "IP Address: %ld", gd->bd->bi_ip_addr) ;
	video_drawstring(10, 364, (unsigned char *)data_buffer)  ;

	/* Net Mask : */
//	sprintf(data_buffer, "Net Mask: %s", getenv("netmask")) ;
	sprintf(data_buffer, "Net Mask: %s", "255.255.255.248") ;
	video_drawstring(10, 380, (unsigned char *)data_buffer)  ;

	/* Speed : */
	sprintf(data_buffer, "Speed: %d", gd->bd->bi_ethspeed) ;
	sprintf(data_buffer, "Speed: %d", gd->bd->bi_baudrate) ;
	video_drawstring(10, 396, (unsigned char *)data_buffer)  ;

	/* Miscellaneous */
	sysinfo_draw_titled_box(5, 430, 384, 64, " Miscelleneous ") ;

	/* Firmware: */
	sprintf(data_buffer, "Firmware: %s", version_string) ;
	data_buffer[29] = '\0' ;
	video_drawstring(10, 440, (unsigned char *)data_buffer)  ;

	/* Vendor : */
	sprintf(data_buffer, "Vendor: %s", "Cyrus") ;
	video_drawstring(10, 456, (unsigned char *)data_buffer)  ;

	/* System Date and Time : */
	/* switch to correct I2C bus */
#ifdef CONFIG_SYS_I2C
	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_RTC_BUS_NUM);
#else
	old_bus = I2C_GET_BUS();
	I2C_SET_BUS(CONFIG_SYS_RTC_BUS_NUM);
#endif
	rcode = rtc_get (&tm);

	if (rcode) {
		puts("## Get date failed\n");
	}
	sprintf(data_buffer, "%d", old_bus) ;
	sprintf(data_buffer, "Date: %4d-%02d-%02d ", 
			tm.tm_year, tm.tm_mon, tm.tm_mday) ;
	video_drawstring(10, 472, (unsigned char *)data_buffer)  ;

	sprintf (data_buffer, "Time: %2d:%02d:%02d",
		tm.tm_hour, tm.tm_min, tm.tm_sec);
	video_drawstring(150, 472, (unsigned char *)data_buffer)  ;

	/* PCI(e) Devices */
	sysinfo_draw_titled_box(rightboxpos, 40, 384, 144, " PCI(e) Devices ") ;
	for (ii = 0; ii < 3; ii++) {
		sysinfo_pciinfo(ii, 1) ;
	}

	/* USB Devices */
	sysinfo_draw_titled_box(rightboxpos, 198, 384, 360, " USB Devices ") ;
	displayed = 0 ;
	ii = 0 ;
	while (displayed < 4) {
		dev = usb_get_dev_index(ii);
		if (dev == NULL)
			break;
		if (dev->descriptor.bDeviceClass == USB_CLASS_HUB) {
			sysinfo_usb_display_desc(dev, displayed);
			displayed++ ;
//			usb_display_config(dev);
		} else {
			sysinfo_usb_display_desc(dev, displayed);
			displayed++ ;
//			usb_display_config(dev);
		}
		ii++ ;
	}

	if (reverse) {
		bmp = (bmp_image_t *) (addr_inv) ;
	} else {
		bmp = (bmp_image_t *) (addr) ;
	}

	if (!((bmp->header.signature[0]=='B') &&
		 (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, (long unsigned int *)&len, &bmp_alloc_addr);

	if (!bmp) {
		printf(" Not bmp - There is no valid bmp file at the given address\n");
		return ;
	}

	video_display_bitmap((unsigned long)bmp, buttonpos , 570);

	c = getc() ;
	if ((c == 'b')  || (c == 'B')) {
		unsigned long delay;
		ulong start = get_timer(0);
		bmp = (bmp_image_t *) (addr_inv) ;
		video_display_bitmap((unsigned long)bmp, buttonpos , 570);
		delay = 1 * CONFIG_SYS_HZ;
		while (get_timer(start) < delay) {
			udelay(1);
		}
		start = get_timer(0);
		bmp = (bmp_image_t *) (addr) ;
		video_display_bitmap((unsigned long)bmp, buttonpos , 570);
		delay = 1 * CONFIG_SYS_HZ;
		while (get_timer(start) < delay) {
			udelay(1);
		}
	}

	/* Return to Boot Menu */
	do_amigabootmenu(cmdtp, flag, argc, argv); 

#ifdef CONFIG_POSTBOOTMENU
	run_command(CONFIG_POSTBOOTMENU, 0);
#endif
}


void sysinfo_draw_titled_box(int x, int y, int w, int h, char * title)
{
	ulong addr = 0x1001c000 ;
	bmp_image_t *bmp, 
			*tlbmp =  (bmp_image_t *)addr, *blbmp =  (bmp_image_t *)addr, 
			*brbmp =  (bmp_image_t *)addr, *tlinebmp =  (bmp_image_t *)addr, 
			*blinebmp =  (bmp_image_t *)addr, *slinebmp =  (bmp_image_t *)addr ;
	void *bmp_alloc_addr = NULL;
	unsigned long len;
	int ii, jj; 

	for (ii = 1; ii <= 6; ii++) {
		bmp = (bmp_image_t *)(addr + ((ii-1) * 0x2000)) ;

		if (!((bmp->header.signature[0]=='B') &&
		 	(bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

		if (!bmp) {
			printf(" Not bmp - There is no valid bmp file at the given address\n");
			return ;
		}

		switch (ii) {
		case 1 : 
			tlbmp = bmp ;
			break ;
		case 2 : 
			blbmp = bmp ;
			break ;
		case 3 : 
			brbmp = bmp ;
			break ;
		case 4 : 
			tlinebmp = bmp ;
			break ;
		case 5 : 
			blinebmp = bmp ;
			break ;
		case 6 : 
			slinebmp = bmp ;
			break ;
		default : 
			break ;
		}
	}

	/* Top row */
	video_display_bitmap((unsigned long)tlbmp, x, y) ;
	ii = (w - (strlen(title)*8)) / 2 ;
	for (jj = 2; jj < ii; jj+=8) {
		video_display_bitmap((unsigned long)tlinebmp, x+jj, y) ;
	}
	video_drawstring(x + jj, y - 8, (unsigned char *)title)  ;
	ii = jj + (strlen(title) * 8) ;
	for (jj = ii; jj < w; jj+=8) {
		video_display_bitmap((unsigned long)tlinebmp, x+jj, y) ;
	}
  	video_display_bitmap((unsigned long)blbmp, x+w, y) ;

	/* Sides */
	for (jj = 2; jj < h; jj+=8) {
		video_display_bitmap((unsigned long)slinebmp, x, y+jj) ;
		video_display_bitmap((unsigned long)slinebmp, x+w, y+jj) ;
	}

	/* Bottom row */
//  	video_display_bitmap((unsigned long)brbmp, x, y) ;
	for (jj = 2; jj < w; jj+=8) {
		video_display_bitmap((unsigned long)blinebmp, x+jj, y+h) ;
	}
  	video_display_bitmap((unsigned long)blbmp, x+w, y+h) ;

	return ;
}

int do_sysinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	sysinfo_show(cmdtp, flag, argc, argv);

	return 0;
}

U_BOOT_CMD(
	sysinfo, 2, 1, do_sysinfo,
	"AmigaONE System Information",
	""
	"    - show AmigaONE System Information"
);
