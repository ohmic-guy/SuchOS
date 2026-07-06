#include "elf.h"
#include "vga.h"

static void memcpy32(uint8_t *dst, const uint8_t *src, uint32_t n) {
  for (uint32_t i = 0; i < n; i++)
    dst[i] = src[i];
}

static void memset32(uint8_t *dst, uint8_t val, uint32_t n) {
  for (uint32_t i = 0; i < n; i++)
    dst[i] = val;
}

uint32_t elf_load(const uint8_t *data, uint32_t size) {
  (void)size;
  const elf32_hdr_t *hdr = (const elf32_hdr_t *)data;

  if (hdr->e_magic != ELF_MAGIC) {
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("[ELF] Bad magic\n");
    return 0;
  }
  if (hdr->e_type != ET_EXEC || hdr->e_machine != EM_386) {
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("[ELF] Not i386 executable\n");
    return 0;
  }

  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("[ELF] Valid ELF32 i386\n");
  terminal_write("[ELF] Entry : ");
  terminal_writehex(hdr->e_entry);
  terminal_putchar('\n');
  terminal_write("[ELF] PHdrs : ");
  terminal_writedec(hdr->e_phnum);
  terminal_putchar('\n');

  const elf32_phdr_t *ph = (const elf32_phdr_t *)(data + hdr->e_phoff);

  for (uint16_t i = 0; i < hdr->e_phnum; i++) {
    if (ph[i].p_type != PT_LOAD)
      continue;

    terminal_write("[ELF] LOAD  vaddr=");
    terminal_writehex(ph[i].p_vaddr);
    terminal_write(" filesz=");
    terminal_writedec(ph[i].p_filesz);
    terminal_write(" memsz=");
    terminal_writedec(ph[i].p_memsz);
    terminal_putchar('\n');

    uint8_t *dst = (uint8_t *)ph[i].p_vaddr;
    const uint8_t *src = data + ph[i].p_offset;

    memcpy32(dst, src, ph[i].p_filesz);
    if (ph[i].p_memsz > ph[i].p_filesz)
      memset32(dst + ph[i].p_filesz, 0, ph[i].p_memsz - ph[i].p_filesz);
  }

  return hdr->e_entry;
}