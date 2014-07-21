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
#include <usb.h>

/* maximum amigabootmenu entries */
#define MAX_COUNT	99

struct amigabootmenu_entry {
	unsigned short int num;		/* unique number 0 .. MAX_COUNT */
	char key[3];			/* key identifier of number */
	char *title;			/* title of entry */
	char *command;			/* hush command of entry */
	struct amigabootmenu_data *menu;	/* this bootmenu */
	struct amigabootmenu_entry *next;	/* next menu entry (num+1) */
};

struct amigabootmenu_data {
	int delay;			/* delay for autoboot */
	int active;			/* active menu entry */
	int count;			/* total count of menu entries */
	struct amigabootmenu_entry *first;	/* first menu entry */
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

/* Globals */
int init = 0 ;	/* Display boot logo or not */
int buttonpos = 400 ;
int leftboxpos = 5 ;
int rightboxpos = 405 ;
int centreboxposx = 300 ;
int centreboxposy = 240 ;

static char *amigabootmenu_getoption(unsigned short int n)
{
	if (n > MAX_COUNT)
		return NULL;

	switch(n) {
	case 0 : 
		return("Start AmigaOS = startamigaos") ;
	case 1 :   
		return("Start Classic AmigaOS = startamigaclassic") ;
	case 2 : 
		return("Start Linux = startlinux") ;
	case 3 : 
		return("Boot Options... = bootopt") ;
	case 4 :  
		return("System Info... = sysinfo") ;
	default :
		return NULL;
	}
}

void amigabootmenu_clear_screen( )
{
	ulong addr = 0x10001000 ;
        bmp_image_t *bmp = (bmp_image_t *)addr;
        void *bmp_alloc_addr = NULL;
        unsigned long len;
	int ii, jj; 

        if (!((bmp->header.signature[0]=='B') &&
              (bmp->header.signature[1]=='M')))
                bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

        if (!bmp) {
                printf(" Not bmp - There is no valid bmp file at the given address\n");
                return ;
        }
	
	for (ii = 0; ii < 80; ii++ ) {
		for (jj = 0; jj < 50; jj++) {
        		video_display_bitmap((unsigned long)bmp, ii * 18, jj * 18) ;
		}
	}

	return ;
}

static void amigabootmenu_print_entry(void *data)
{
	struct amigabootmenu_entry *entry = data;
	int reverse = (entry->menu->active == entry->num);
	ulong addr = 0x1000c000 ;
	ulong addr_inv = 0x1001a000 ;
        bmp_image_t *bmp = (bmp_image_t *)addr;
        void *bmp_alloc_addr = NULL;
        unsigned long len;

        if (!((bmp->header.signature[0]=='B') &&
              (bmp->header.signature[1]=='M')))
                bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

        if (!bmp) {
                printf(" Not bmp - There is no valid bmp file at the given address\n");
                return ;
        }

	if (reverse) {
		bmp = (bmp_image_t *) (addr_inv - ((entry->num) * 0x2000)) ;
	} else {
		bmp = (bmp_image_t *) (addr - ((entry->num) * 0x2000)) ;
	}

        video_display_bitmap((unsigned long)bmp, buttonpos, (entry->num * 30) + 190);

}

#ifdef AUTO_BOOT
static void amigabootmenu_autoboot_loop(struct amigabootmenu_data *menu,
				enum amigabootmenu_key *key, int *esc)
{
	int i, c;

	while (menu->delay > 0) {
		for (i = 0; i < 100; ++i) {
			if (!tstc()) {
				WATCHDOG_RESET();
				mdelay(10);
				continue;
			}

			menu->delay = -1;
			c = getc();

			switch (c) {
			case '\e':
				*esc = 1;
				*key = KEY_NONE;
				break;
			case '\r':
				*key = KEY_SELECT;
				break;
			default:
				*key = KEY_NONE;
				break;
			}

			break;
		}

		if (menu->delay < 0)
			break;

		--menu->delay;
	}

	if (menu->delay == 0) {
		*key = KEY_SELECT;
#ifdef CONFIG_ESCAPEBOOTMENU
	} else {
		run_command(CONFIG_ESCAPEBOOTMENU, 0);
		*key = KEY_UP; // Force redraw
	}
#endif
}
#endif

static void amigabootmenu_loop(struct amigabootmenu_data *menu,
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
	case 'a' :
		menu->active = 0 ;
		*key = KEY_SELECT;
		break ;
	case 'b' :
		menu->active = 3 ;
		*key = KEY_SELECT;
		break ;
	case 'c' :
		menu->active = 1 ;
		*key = KEY_SELECT;
		break ;
	case 'l' :
		menu->active = 2 ;
		*key = KEY_SELECT;
		break ;
	case 'm' :
		menu->active = 5 ;
		*key = KEY_SELECT;
		break ;
	case 's' :
		menu->active = 4 ;
		*key = KEY_SELECT;
		break ;
	default : /* Other key... */
		break ;
	}

	if (*key == KEY_SELECT) {
		ulong addr = 0x1000c000 ;
		ulong addr_inv = 0x1001a000 ;
       		bmp_image_t *bmp ;
       		void *bmp_alloc_addr = NULL;
		unsigned long len, delay;
		ulong start = get_timer(0);

		/* Grey out current active button */
		bmp = (bmp_image_t *) (addr - ((current_active) * 0x2000)) ;
		if (!((bmp->header.signature[0]=='B') &&
		      	(bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);
		if (!bmp) {
			printf(" Not bmp - There is no valid bmp file at the given address\n");
			return ;
		}
		video_display_bitmap((unsigned long)bmp, buttonpos, (current_active * 30) + 190);

		/* Highlight selected button and pause slightly before acting */
		bmp = (bmp_image_t *) (addr_inv - ((menu->active) * 0x2000)) ;
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

		start = get_timer(0) ;
		/* Then put original greyed button back before continuing */
		bmp = (bmp_image_t *) (addr - ((menu->active) * 0x2000)) ;
		video_display_bitmap((unsigned long)bmp, buttonpos, (menu->active * 30) + 190);

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

static char *amigabootmenu_choice_entry(void *data)
{
	struct amigabootmenu_data *menu = data;
	struct amigabootmenu_entry *iter;
	enum amigabootmenu_key key = KEY_NONE;
	int esc = 0;
	int i;

	while (1) {
		if (menu->delay >= 0) {
			/* Autoboot was not stopped */
//			amigabootmenu_autoboot_loop(menu, &key, &esc);
			amigabootmenu_loop(menu, &key, &esc);
		} else {
			/* Some key was pressed, so autoboot was stopped */
			amigabootmenu_loop(menu, &key, &esc);
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
	debug("amigabootmenu: this should not happen");
	return NULL;
}

static void amigabootmenu_destroy(struct amigabootmenu_data *menu)
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

static struct amigabootmenu_data *amigabootmenu_create(int delay)
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

	while ((option = amigabootmenu_getoption(i))) {
		sep = strchr(option, '=');
		if (!sep) {
			printf("Invalid amigabootmenu entry: %s\n", option);
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

	/* Add U-Boot console entry at the end */
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

	menu->count = i;
	return menu;

cleanup:
	amigabootmenu_destroy(menu);
	return NULL;
}

static void amigabootmenu_show(int mdelay)
{
	void *choice = NULL;
	char *title = NULL;
	char *command = NULL;
	struct menu *menu;
	struct amigabootmenu_data *amigabootmenu;
	struct amigabootmenu_entry *iter;
	char *option, *sep;
/* Uncomment for main title string */
//	char *main_title ;
	int cols, rows ;
	int ii, jj ;
	ulong addr = 0x11000000 ;
	ulong addr_splash = 0x12000000 ;
	ulong maintitle_addr = 0x1003a000 ;
        bmp_image_t *bmp = (bmp_image_t *)addr;
	ulong start = get_timer(0);
	unsigned char bBootMenu = 0 ;
	int delay ;

	/* Temporarily pop up animated splash screen - first time only */
	if (init == 0) {
		amigabootmenu_clear_screen() ;
		bmp = (bmp_image_t *)(addr_splash) ;
		video_display_bitmap((unsigned long)bmp, 0, 0);

		/* Start USB */
		usb_init() ;

		/* Start SCSI too probably */

		mdelay = 0 ;
		for (jj = 0; jj < 10; jj++) {
			for (ii = 0; ii < 10; ii++) {
				bmp = (bmp_image_t *)(addr_splash + (ii * 0x1000000)) ;
				video_display_bitmap((unsigned long)bmp, 0, 0);
				delay = 100 ;
				while (get_timer(start) < delay) {
					udelay(1);
				}
				init = 1 ;
				start = get_timer(0) ;
				/* If any key has been pressed break out and run menu */
				if (tstc()) {
					ii = 10 ;
					jj = 10 ;
					bBootMenu = 1 ;
					mdelay = 10 ;
					getc() ;
				}
			}
		}
	}


	/* If delay is 0 do not create menu, just run first entry */
 	if ((mdelay == 0) && (bBootMenu == 0)) {
		option = amigabootmenu_getoption(0);
		if (!option) {
			puts("amigabootmenu option 0 was not found\n");
			return;
		}
		sep = strchr(option, '=');
		if (!sep) {
			puts("amigabootmenu option 0 is invalid\n");
			return;
		}
		run_command(sep+1, 0);
		return;
	}

	amigabootmenu = amigabootmenu_create(mdelay);
	if (!amigabootmenu)
		return;

	menu = menu_create(NULL, amigabootmenu->delay, 1, amigabootmenu_print_entry,
			   amigabootmenu_choice_entry, amigabootmenu);
	if (!menu) {
		amigabootmenu_destroy(amigabootmenu);
		return;
	}

	for (iter = amigabootmenu->first; iter; iter = iter->next) {
		if (!menu_item_add(menu, iter->key, iter))
			goto cleanup;
	}

	/* Default menu entry is always first */
	menu_default_set(menu, "0");
	amigabootmenu_clear_screen() ;

	cols = video_get_screen_columns() ;
	rows = video_get_screen_rows() ;

	centreboxposx = ((cols * 8) - 250) / 2 ;
	centreboxposy = ((rows * 15) - 192) / 2 ;
	buttonpos = ((cols * 8) - 194) / 2 ;

/* Uncomment for string title */
//	main_title = strdup("<<<<<< AmigaONE Early Startup Control >>>>>>");
//	video_drawstring((((cols - strlen(main_title)) * 8) / 2), 10, (unsigned char *)main_title)  ;

        bmp = (bmp_image_t *)maintitle_addr ;
	video_display_bitmap((unsigned long)bmp, (((cols * 8) - 250) / 2), 5);

	sysinfo_draw_titled_box(centreboxposx, centreboxposy, 250, 192, "") ;

	if (menu_get_choice(menu, &choice)) {
		iter = choice;
		title = strdup(iter->title);
		command = strdup(iter->command);
	}

cleanup:
	menu_destroy(menu);
	amigabootmenu_destroy(amigabootmenu);

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

#ifdef CONFIG_MENU_SHOW
int amenu_show(int bootdelay)
{
	amigabootmenu_show(bootdelay);
	return -1; /* -1 - abort boot and run monitor code */
}
#endif

int do_amigabootmenu(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *delay_str = NULL;
	int delay = 10;

#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	delay = CONFIG_BOOTDELAY;
#endif

	if (argc >= 2)
		delay_str = argv[1];

	if (!delay_str)
		delay_str = getenv("amigabootmenu_delay");

	if (delay_str)
		delay = (int)simple_strtol(delay_str, NULL, 10);

	amigabootmenu_show(delay);
	return 0;
}

U_BOOT_CMD(
	amigabootmenu, 2, 1, do_amigabootmenu,
	"AmigaONE Boot Menu",
	"[delay]\n"
	"    - show Graphical AmigaONE boot menu with autoboot delay"
);
