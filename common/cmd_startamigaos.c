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

#include <video.h>

/* Function Declarations */
void amigabootmenu_clear_screen( void ) ;
extern void video_drawstring(int xx, int yy, unsigned char *s) ;
extern void sysinfo_draw_titled_box(int x, int y, int w, int h, char * title) ;
extern int do_amigabootmenu(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]) ;

static void startamigaos_show(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	amigabootmenu_clear_screen() ;

	video_drawstring(160, 20, (unsigned char *)"<<<<<< AmigaONE running >>>>>>")  ;

	getc() ;

	/* Return to Boot Menu */
	do_amigabootmenu(cmdtp, flag, argc, argv); 

}


int do_startamigaos(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	startamigaos_show(cmdtp, flag, argc, argv) ;

	return 0;
}

U_BOOT_CMD(
	startamigaos, 2, 1, do_startamigaos,
	"AmigaONE Start",
	""
	"    - start AmigaONE"
);
