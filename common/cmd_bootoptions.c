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

/* maximum amigabootmenu entries */
#define MAX_COUNT       99

struct amigabootmenu_entry {
	unsigned short int num;	 /* unique number 0 .. MAX_COUNT */
	char key[3];		    /* key identifier of number */
	char *title;		    /* title of entry */
	char *command;		  /* hush command of entry */
	struct amigabootmenu_data *menu;	/* this bootmenu */
	struct amigabootmenu_entry *next;       /* next menu entry (num+1) */
};

struct amigabootmenu_data {
	int delay;		      /* delay for autoboot */
	int active;		     /* active menu entry */
	int count;		      /* total count of menu entries */
	struct amigabootmenu_entry *first;      /* first menu entry */
};

enum amigabootmenu_key {
	KEY_NONE = 0,
	KEY_UP,
	KEY_DOWN,
	KEY_SELECT,
};

/* Function Declarations */
void amigabootmenu_clear_screen( void ) ;
extern void video_drawstring(int xx, int yy, unsigned char *s) ;
extern void sysinfo_draw_titled_box(int x, int y, int w, int h, char * title) ;
extern struct bmp_image *gunzip_bmp(unsigned long addr, unsigned long *lenp, void **alloc_addr);
int do_amigabootmenu(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]) ;

extern int buttonpos ;
extern int centreboxposx ;
extern int centreboxposy ;

static char *bootoptionsmenu_getoption(unsigned short int n)
{
	if (n > MAX_COUNT)
		return NULL;

	switch(n) {
	case 0 :
		return("Boot from Hard Disk = echo reboot jam") ;
	case 1 :
		return("Boot from Optical = echo reboot marmalade") ;
	case 2 :
		return("Boot from Ethernet = echo reboot lemon curd") ;
	case 3 :
		return("Boot from Mass Storage = echo reboot mango chutney") ;
	case 4 :
		return("Back to Main Boot Menu = ami") ;
	default :
		return NULL;
	}
}

static void bootoptionsmenu_print_entry(void *data)
{
	struct amigabootmenu_entry *entry = data;
	int reverse = (entry->menu->active == entry->num);
	ulong addr = 0x1002a000 ;
	ulong addr_inv = 0x10032000 ;
	ulong addr_back_button = 0x1000e000 ;
	ulong addr_back_button_inv = 0x10028000 ;
	bmp_image_t *bmp ;
	void *bmp_alloc_addr = NULL;
	unsigned long len;

	if (entry->num == 4) {
		if (reverse) {
			bmp = (bmp_image_t *) (addr_back_button_inv) ;
		} else {
			bmp = (bmp_image_t *) (addr_back_button) ;
		}
	} else {
		if (reverse) {
			bmp = (bmp_image_t *) (addr_inv + ((entry->num) * 0x2000)) ;
		} else {
			bmp = (bmp_image_t *) (addr + ((entry->num) * 0x2000)) ;
		}
	}
	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

	if (!bmp) {
		printf(" Not bmp - There is no valid bmp file at the given address\n");
		return ;
	}


	video_display_bitmap((unsigned long)bmp, buttonpos, (entry->num * 30) + 190);
}

static void bootoptionsmenu_loop(struct amigabootmenu_data *menu,
		enum amigabootmenu_key *key, int *esc)
{
	int c;
	int current_active ;

	while (!tstc()) {
		WATCHDOG_RESET();
		mdelay(10);
	}

	c = getc();

	switch (*esc) {
	case 0:
		/* First char of ANSI escape sequence '\e' */
		if (c == '\e') {
			*esc = 1;
			*key = KEY_NONE;
		}
		break;
	case 1:
		/* Second char of ANSI '[' */
		if (c == '[') {
			*esc = 2;
			*key = KEY_NONE;
		} else {
			*esc = 0;
		}
		break;
	case 2:
	case 3:
		/* Third char of ANSI (number '1') - optional */
		if (*esc == 2 && c == '1') {
			*esc = 3;
			*key = KEY_NONE;
			break;
		}

		*esc = 0;

		*esc = 0;

		/* ANSI 'A' - key up was pressed */
		if (c == 'A')
			*key = KEY_UP;
		/* ANSI 'B' - key down was pressed */
		else if (c == 'B')
			*key = KEY_DOWN;
		/* other key was pressed */
		else
			*key = KEY_NONE;

		break;
	}

	/* Remember currently active menu item so we can grey it out */
	current_active = menu->active ;

	/* Get menu accelerators */
	switch (c) {
	case 'h' :
		menu->active = 0 ;
		*key = KEY_SELECT;
		break ;
	case 'b' :
		menu->active = 4 ;
		*key = KEY_SELECT;
		break ;
	case 'o' :
		menu->active = 1 ;
		*key = KEY_SELECT;
		break ;
	case 'e' :
		menu->active = 2 ;
		*key = KEY_SELECT;
		break ;
	case 'm' :
		menu->active = 3 ;
		*key = KEY_SELECT;
		break ;
	default : /* Other key... */
		break ;
	}

	if (*key == KEY_SELECT) {
		ulong addr = 0x1002a000 ;
		ulong addr_inv = 0x10032000 ;
		ulong addr_back_button = 0x1000e000 ;
		ulong addr_back_button_inv = 0x10028000 ;
		bmp_image_t *bmp ;
		void *bmp_alloc_addr = NULL;
		unsigned long len, delay;
		ulong start = get_timer(0);

		/* Grey out current active button */
		if (current_active == 4) {
			bmp = (bmp_image_t *) (addr_back_button) ;
		} else {
			bmp = (bmp_image_t *) (addr + (current_active * 0x2000)) ;
		}
		if (!((bmp->header.signature[0]=='B') &&
			(bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);
		if (!bmp) {
			printf(" Not bmp - There is no valid bmp file at the given address\n");
			return ;
		}
		video_display_bitmap((unsigned long)bmp, buttonpos, (current_active * 30) + 190);

		/* Highlight selected button and pause slightly before acting */
		if (menu->active == 4) {
			bmp = (bmp_image_t *) (addr_back_button_inv) ;
		} else {
			bmp = (bmp_image_t *) (addr_inv + ((menu->active) * 0x2000)) ;
		}
		if (!((bmp->header.signature[0]=='B') &&
			(bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(addr_inv, &len, &bmp_alloc_addr);
		if (!bmp) {
			printf(" Not bmp - There is no valid bmp file at the given address\n");
			return ;
		}
		video_display_bitmap((unsigned long)bmp, buttonpos, (menu->active * 30) + 190);

		delay = 1 * CONFIG_SYS_HZ;
		while (get_timer(start) < delay) {
			udelay(1);
		}

		/* Then restore before acting */
		if (menu->active == 4) {
			bmp = (bmp_image_t *) (addr_back_button) ;
		} else {
			bmp = (bmp_image_t *) (addr + (menu->active * 0x2000)) ;
		}
		if (!((bmp->header.signature[0]=='B') &&
			(bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);
		if (!bmp) {
			printf(" Not bmp - There is no valid bmp file at the given address\n");
			return ;
		}
		video_display_bitmap((unsigned long)bmp, buttonpos, (menu->active * 30) + 190);

		start = get_timer(0);
		delay = 1 * CONFIG_SYS_HZ;
		while (get_timer(start) < delay) {
			udelay(1);
		}

		amigabootmenu_clear_screen() ;
	}

	/* enter key was pressed */
	if (c == '\r')
		*key = KEY_SELECT;
	/* Mapped arrow keys on USB - ctrl-P*/
	else if (c == 14 || c == 'n' || c == 'N')
		*key = KEY_DOWN;
	/* ctrl-N */
	else if (c == 16 || c == 'p' || c == 'P')
		*key = KEY_UP;

	return ;
}

static char *bootoptionsmenu_choice_entry(void *data)
{
	struct amigabootmenu_data *menu = data;
	struct amigabootmenu_entry *iter;
	enum amigabootmenu_key key = KEY_NONE;
	int esc = 0;
	int i;

	while (1) {
		if (menu->delay >= 0) {
			/* Autoboot was not stopped */
//		      amigabootmenu_autoboot_loop(menu, &key, &esc);
			bootoptionsmenu_loop(menu, &key, &esc);
		} else {
			/* Some key was pressed, so autoboot was stopped */
			bootoptionsmenu_loop(menu, &key, &esc);
		}

		switch (key) {
		case KEY_UP:
			if (menu->active > 0)
				--menu->active;
			/* no menu key selected, regenerate menu */
			return NULL;
		case KEY_DOWN:
			if (menu->active < menu->count - 1)
				++menu->active;
			/* no menu key selected, regenerate menu */
			return NULL;
		case KEY_SELECT:
			iter = menu->first;
			for (i = 0; i < menu->active; ++i) {
				iter = iter->next;
			}
			/* Display reversed bitmap and wait for an instant? */

			return iter->key;
		default:
			break;
		}
	}

	/* never happens */
	debug("bootoptionsmenu: this should not happen");
	return NULL;
}

static void bootoptionsmenu_destroy(struct amigabootmenu_data *menu)
{
	struct amigabootmenu_entry *iter = menu->first;
	struct amigabootmenu_entry *next;

	while (iter) {
		next = iter->next;
		free(iter->title);
		free(iter->command);
		free(iter);
		iter = next;
	}
	free(menu);
}

static struct amigabootmenu_data *bootoptionsmenu_create(int delay)
{
	unsigned short int i = 0;
	const char *option;
	struct amigabootmenu_data *menu;
	struct amigabootmenu_entry *iter = NULL;

	int len;
	char *sep;
	struct amigabootmenu_entry *entry;

	menu = malloc(sizeof(struct amigabootmenu_data));
	if (!menu)
		return NULL;

	menu->delay = delay;
	menu->active = 0;
	menu->first = NULL;

	while ((option = bootoptionsmenu_getoption(i))) {
		sep = strchr(option, '=');
		if (!sep) {
			printf("Invalid bootoptionsmenu entry: %s\n", option);
			break;
		}

		entry = malloc(sizeof(struct amigabootmenu_entry));
		if (!entry)
			goto cleanup;

		len = sep-option;
		entry->title = malloc(len + 1);
		if (!entry->title) {
			free(entry);
			goto cleanup;
		}
		memcpy(entry->title, option, len);
		entry->title[len] = 0;

		len = strlen(sep + 1);
		entry->command = malloc(len + 1);
		if (!entry->command) {
			free(entry->title);
			free(entry);
			goto cleanup;
		}
		memcpy(entry->command, sep + 1, len);
		entry->command[len] = 0;

		sprintf(entry->key, "%d", i);

		entry->num = i;
		entry->menu = menu;
		entry->next = NULL;

		if (!iter)
			menu->first = entry;
		else
			iter->next = entry;

		iter = entry;
		++i;

		if (i == MAX_COUNT - 1)
			break;
	}

#ifdef extra
	/* Add back to main menu entry at the end  ???? */
	if (i <= MAX_COUNT - 1) {
		entry = malloc(sizeof(struct amigabootmenu_entry));
		if (!entry)
			goto cleanup;

		entry->title = strdup("U-Boot Command Line");
		if (!entry->title) {
			free(entry);
			goto cleanup;
		}

		entry->command = strdup("");
		if (!entry->command) {
			free(entry->title);
			free(entry);
			goto cleanup;
		}

		sprintf(entry->key, "%d", i);

		entry->num = i;
		entry->menu = menu;
		entry->next = NULL;

		if (!iter)
			menu->first = entry;
		else
			iter->next = entry;

		iter = entry;
		++i;
	}
#endif

	menu->count = i;
	return menu;

cleanup:
	bootoptionsmenu_destroy(menu);
	return NULL;
}

static void bootoptions_show(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	ulong delay = 10 ;
	void *choice = NULL;
	char *title = NULL;
	char *command = NULL;
	struct menu *menu;
	struct amigabootmenu_data *bootoptionsmenu;
	struct amigabootmenu_entry *iter;

	bootoptionsmenu = bootoptionsmenu_create(delay);
	if (!bootoptionsmenu)
		return;

	menu = menu_create(NULL, bootoptionsmenu->delay, 1, bootoptionsmenu_print_entry,
			   bootoptionsmenu_choice_entry, bootoptionsmenu);
	if (!menu) {
		bootoptionsmenu_destroy(bootoptionsmenu);
		return;
	}

	for (iter = bootoptionsmenu->first; iter; iter = iter->next) {
		if (!menu_item_add(menu, iter->key, iter))
			goto cleanup;
	}

	/* Default menu entry is always first */
	menu_default_set(menu, "0");

	amigabootmenu_clear_screen() ;

	video_drawstring(270, 10, (unsigned char *)"<<<<<< AmigaONE Boot Options >>>>>>")  ;

	sysinfo_draw_titled_box(centreboxposx, centreboxposy, 248, 160, "") ;

#ifdef Inline

	for (ii = 0; ii < 4; ii++) {
		bmp = (bmp_image_t *) (addr_boot_menu_options + (ii * 0x2000)) ;

		if (!((bmp->header.signature[0]=='B') &&
	      	(bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(addr_boot_menu_options + (ii * 0x2000) , &len, &bmp_alloc_addr);

		if (!bmp) {
			printf(" Not bmp - There is no valid bmp file at the given address\n");
			return ;
		}

		video_display_bitmap((unsigned long)bmp, buttonpos, 190 + (ii * 20));
	}
	
	bmp = (bmp_image_t *)addr_back_button;
	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr_back_button, &len, &bmp_alloc_addr);

	if (!bmp) {
		printf(" Not bmp - There is no valid bmp file at the given address\n");
		return ;
	}

	video_display_bitmap((unsigned long)bmp, buttonpos, 330);

	getc() ;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr_back_button_inv, &len, &bmp_alloc_addr);

	if (!bmp) {
		printf(" Not bmp - There is no valid bmp file at the given address\n");
		return ;
	}

	bmp = (bmp_image_t *) (addr_back_button_inv) ;

	video_display_bitmap((unsigned long)bmp, buttonpos, 330);

	delay = 3 * CONFIG_SYS_HZ;
	while (get_timer(start) < delay) {
		udelay(3);
	}

	/* Return to Boot Menu */
	do_amigabootmenu(cmdtp, flag, argc, argv); 

#endif

	if (menu_get_choice(menu, &choice)) {
		iter = choice;
		title = strdup(iter->title);
		command = strdup(iter->command);
	}

cleanup:
	menu_destroy(menu);
	bootoptionsmenu_destroy(bootoptionsmenu);

	if (title && command) {
		debug("Starting entry '%s'\n", title);
		free(title);
		run_command(command, 0);
		free(command);
	}

#ifdef CONFIG_POSTBOOTMENU
	run_command(CONFIG_POSTBOOTMENU, 0);
#endif
}


int do_bootoptions(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	bootoptions_show(cmdtp, flag, argc, argv);

	return 0;
}

U_BOOT_CMD(
	bootoptions, 2, 1, do_bootoptions,
	"AmigaONE Boot Options",
	""
	"    - show AmigaONE Boot Options"
);
