#include <common.h>
#include <command.h>
#include <elf.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

static unsigned long load_elf_image_phdr(unsigned long addr);
static unsigned long load_elf_image_shdr(unsigned long addr);

extern int valid_elf_image(unsigned long addr);

typedef struct {
	unsigned long long  sdram_base;
	unsigned long long 	sdram_size;

	unsigned long 		bus_clock;
	unsigned long 		cpu_clock;
	void *        		usable_memtop;
} amigaboot_info_t;

unsigned long call_amigaboot(ulong (*entry)(void *), void *abi)
{
	unsigned long ret;

	/*
	 * QNX images require the data cache is disabled.
	 * Data cache is already flushed, so just turn it off.
	 * AmigaBoot should run fine with caches enabled, so we
	 * might want to remove this.
	 */
	int dcache = dcache_status();
	if (dcache)
		dcache_disable();


	ret = entry(abi);

	if (dcache)
		dcache_enable();

	return ret;
}

static int try_load_amigaboot(void *addr) {
	struct mmc *mmc = find_mmc_device(CONFIG_AMIGABOOT_MCC_DEV);
	if (mmc_init(mmc)) {
		puts("boota: MMC init failed\n");
		return -1;
	}

	int n = mmc->block_dev.block_read(CONFIG_AMIGABOOT_MCC_DEV,
			CONFIG_AMIGABOOT_BLOCK_OFFS,
			CONFIG_AMIGABOOT_BLOCK_LEN,
			(uchar *)addr);
	if (n == CONFIG_AMIGABOOT_BLOCK_LEN) return 0;
	return -1;
}

int do_boota(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;		/* Address of the ELF image     */
	unsigned long rc;		/* Return value from user code  */
	char *sload, *saddr;

	/* -------------------------------------------------- */
	static amigaboot_info_t abi;

	abi.bus_clock = gd->bus_clk;
	abi.cpu_clock = gd->cpu_clk;

	abi.sdram_base = gd->bd->bi_memstart;
	abi.sdram_size = gd->bd->bi_memsize;

	unsigned long memtop = (unsigned long)gd - 1024*1024;
	memtop &= ~0xFFFFF;
	abi.usable_memtop = (void *)memtop;

	/* -------------------------------------------------- */
	int rcode = 0;

	sload = saddr = NULL;
	if (argc == 3) {
		sload = argv[1];
		saddr = argv[2];
	} else if (argc == 2) {
		if (argv[1][0] == '-')
			sload = argv[1];
		else
			saddr = argv[1];
	}

	if (saddr)
		addr = simple_strtoul(saddr, NULL, 16);
	else {
		// If no address was given, assume we want to load from SD
		addr = load_addr;
		try_load_amigaboot((void *)addr);
	}

	if (!valid_elf_image(addr))
		return 1;

	if (sload && sload[1] == 'p')
		addr = load_elf_image_phdr(addr);
	else
		addr = load_elf_image_shdr(addr);

	printf("## Starting application at 0x%08lx ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = call_amigaboot((void *)addr, &abi);
	if (rc != 0)
		rcode = 1;

	printf("## Application terminated, rc = 0x%lx\n", rc);
	return rcode;

}

// Blatantly ripped out of an innocent cmd_elf.c

/* ======================================================================
 * A very simple elf loader, assumes the image is valid, returns the
 * entry point address.
 * ====================================================================== */
static unsigned long load_elf_image_phdr(unsigned long addr)
{
	Elf32_Ehdr *ehdr;		/* Elf header structure pointer     */
	Elf32_Phdr *phdr;		/* Program header structure pointer */
	int i;

	ehdr = (Elf32_Ehdr *) addr;
	phdr = (Elf32_Phdr *) (addr + ehdr->e_phoff);

	/* Load each program header */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		void *dst = (void *)(uintptr_t) phdr->p_paddr;
		void *src = (void *) addr + phdr->p_offset;
		debug("Loading phdr %i to 0x%p (%i bytes)\n",
			i, dst, phdr->p_filesz);
		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);
		if (phdr->p_filesz != phdr->p_memsz)
			memset(dst + phdr->p_filesz, 0x00,
				phdr->p_memsz - phdr->p_filesz);
		flush_cache((unsigned long)dst, phdr->p_filesz);
		++phdr;
	}

	return ehdr->e_entry;
}

static unsigned long load_elf_image_shdr(unsigned long addr)
{
	Elf32_Ehdr *ehdr;		/* Elf header structure pointer     */
	Elf32_Shdr *shdr;		/* Section header structure pointer */
	unsigned char *strtab = 0;	/* String table pointer             */
	unsigned char *image;		/* Binary image pointer             */
	int i;				/* Loop counter                     */

	/* -------------------------------------------------- */

	ehdr = (Elf32_Ehdr *) addr;

	/* Find the section header string table for output info */
	shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
			       (ehdr->e_shstrndx * sizeof(Elf32_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *) (addr + shdr->sh_offset);

	/* Load each appropriate section */
	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
				       (i * sizeof(Elf32_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC)
		   || shdr->sh_addr == 0 || shdr->sh_size == 0) {
			continue;
		}

		if (strtab) {
			debug("%sing %s @ 0x%08lx (%ld bytes)\n",
				(shdr->sh_type == SHT_NOBITS) ?
					"Clear" : "Load",
				&strtab[shdr->sh_name],
				(unsigned long) shdr->sh_addr,
				(long) shdr->sh_size);
		}

		if (shdr->sh_type == SHT_NOBITS) {
			memset((void *)(uintptr_t) shdr->sh_addr, 0,
				shdr->sh_size);
		} else {
			image = (unsigned char *) addr + shdr->sh_offset;
			memcpy((void *)(uintptr_t) shdr->sh_addr,
				(const void *) image,
				shdr->sh_size);
		}
		flush_cache(shdr->sh_addr, shdr->sh_size);
	}

	return ehdr->e_entry;
}

U_BOOT_CMD(
        boota,      1,      0,      do_boota,
        "boota - start AmigaOS boot procedure",
        "Write me! I dare you!");
