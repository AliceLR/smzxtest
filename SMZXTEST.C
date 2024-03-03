/* Super MegaZeux text mode tester (Turbo C 2.01)
 *
 * Copyright (C) 2024 Alice Rowan <petrifiedrowan@gmail.com>
 *
 * Extensively uses code from MegaZeux's EGA renderer:
 * Copyright (C) 2010 Alan Williams <mralert@gmail.com>
 * Copyright (C) 2019 Adrian Siekierka <kontakt@asie.pl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <dos.h>
#include <stdio.h>

static unsigned char far *ptr(unsigned long offset)
{
	return (unsigned char far *)MK_FP(offset >> 4, offset & 0x0f);
}

static void vga_set_16p(void)
{
	asm mov ax,1202h;
	asm mov bl,30h;
	asm int 10h;
	asm mov ax,0003h;
	asm int 10h;
}

static void ega_set_14p_mode_3(void)
{
	asm mov ax,1201h;
	asm mov bl,30h;
	asm int 10h;
	asm mov ax,0003h;
	asm int 10h;
}

static void ega_set_page(void)
{
	asm mov ax,0500h;
	asm int 10h;
}

static void ega_set_smzx(int is_ati)
{
	outportb(0x3C0, 0x10);
	outportb(0x3C0, 0x4C);
	if(is_ati)
	{
		outport(0x3C0, 0x13);
		outport(0x3C0, 0x01);
	}
}

static int ega_detect_ati(void)
{
	static const unsigned char magic[] = "761295520";
	unsigned char far *offset = ptr(0xc0031ul);
	int i;
	for(i = 0; i < 9; i++)
		if(offset[i] != magic[i])
			return 0;
	return 1;
}

static void ega_blink_on(void)
{
	asm mov ax, 1003h;
	asm mov bl, 01h;
	asm int 10h;
}

static void ega_blink_off(void)
{
	asm mov ax, 1003h;
	asm mov bl, 00h;
	asm int 10h;
}

static void ega_cursor_off(void)
{
	asm mov ax, 0103h;
	asm mov cx, 3f00h;
	asm int 10h;
}

static void ega_cursor_on(void)
{
	asm mov ax, 0103h;
	asm mov cx, 0b0ch;
	asm int 10h;
}

static void ega_bank_char(void)
{
	outportb(0x3CE, 0x05);
	outportb(0x3CF, 0x00);
	outportb(0x3CE, 0x06);
	outportb(0x3CF, 0x0C);
	outportb(0x3C4, 0x04);
	outportb(0x3C5, 0x06);
	outportb(0x3C4, 0x02);
	outportb(0x3C5, 0x04);
	outportb(0x3CE, 0x04);
	outportb(0x3CF, 0x02);
}

static void ega_bank_text(void)
{
	outportb(0x3CE, 0x05);
	outportb(0x3CF, 0x10);
	outportb(0x3CE, 0x06);
	outportb(0x3CF, 0x0E);
	outportb(0x3C4, 0x04);
	outportb(0x3C5, 0x02);
	outportb(0x3C4, 0x02);
	outportb(0x3C5, 0x03);
	outportb(0x3CE, 0x04);
	outportb(0x3CF, 0x00);
}

static void ega_vsync(void)
{
	while(inportb(0x3DA) & 0x08)
		;
	while(!(inportb(0x3DA) & 0x08))
		;
}

static void charset(unsigned char ch, const unsigned char value[14])
{
	unsigned char far *offset = ptr(0xb8000ul) + (ch << 5);
	int i;
	for(i = 0; i < 14; i++)
		offset[i] = value[i];
}

static unsigned char far *drawoffset(int x, int y)
{
	return ptr(0xb8000ul) + ((y * 80 + x) << 1);
}

static void drawclear(unsigned char co)
{
	unsigned char far *offset = drawoffset(0, 0);
	int i;
	for(i = 0; i < 2000; i++)
	{
		*(offset++) = 0;
		*(offset++) = co;
	}
}

static void drawchar(int x, int y, unsigned char ch, unsigned char co)
{
	unsigned char far *offset = drawoffset(x, y);
	offset[0] = ch;
	offset[1] = co;
}

static void drawstr(int x, int y, const char *ch, unsigned char co)
{
	unsigned char far *offset = drawoffset(x, y);
	while(*ch)
	{
		*(offset++) = *(ch++);
		*(offset++) = co;
	}
}

static void drawbox(int x, int y, int w, int h, int co)
{
	static const char edge[] = { 218, 196, 191, 179, 0, 179, 192, 196, 217 };
	int i, j;
	for(i = 0; i < h; i++)
	{
	  for(j = 0; j < w; j++)
	  {
	    int idx = ((i > 0) + (i >= h - 1)) * 3 + (j > 0) + (j >= w - 1);
	    drawchar(x + j, y + i, edge[idx], co);
	  }
	}
}

int main(void)
{
	char buf[32];
	int is_ati = ega_detect_ati();
	int i;
	int j;
	int pal;

	ega_set_14p_mode_3();
	ega_set_smzx(is_ati);
	ega_cursor_off();
	ega_blink_off();

	/* Set the EGA palette to the first 16 of the VGA palette. */
	ega_vsync();
	for(i = 0; i < 16; i++)
	{
		outportb(0x3C0, i);
		outportb(0x3C0, i);
	}
	/* Get attribute controller back to normal */
	outportb(0x3C0, 0x20);

	/* Edit palette */
	for(i = 0; i < 256; i++)
	{
		int a = ((i >> 4) << 2) + 0;
		int b = ((i & 15) << 2) + 0;
		int idx = is_ati ? (i >> 4) | ((i & 15) << 4) : i;
		outportb(0x3C8, idx);
		outportb(0x3C9, a);
		outportb(0x3C9, b);
		outportb(0x3C9, b);
	}

	/* Edit char 127 */
	ega_bank_char();
	memset(buf, 0x55, sizeof(buf));
	charset(0x7f, buf);

	/* Draw graphic */
	ega_bank_text();
	drawclear(0x00);

	drawstr(4, 18, "256 colors, cyan right, red bottom: works.", 0x0f);
	drawstr(4, 19, "256 colors, red right, cyan bottom: works (reversed nibbles).", 0x0f);
	drawstr(4, 20, "16 colors: doesn't work (cyan: C&T / NVIDIA) (red: ATI).", 0x0f);
	drawstr(4, 21, "Interlaced bars: doesn't work, no doubling.", 0x0f);
	drawstr(4, 22, "Horizontal bars: doesn't work, doubled left pixel.", 0x0f);
	drawstr(4, 23, "Vertical bars: doesn't work, doubled right pixel.", 0x0f);

	sprintf(buf, "Mode: %s", is_ati ? "ATI" : "C&T / NVIDIA");
	drawstr(4, 1, buf, 0x0f);

	drawstr(63, 4, "Char diagram:", 0x0f);
	drawbox(64, 5, 10, 9, 0x0f);
	for(i = 0; i < 7; i++)
		for(j = 0; j < 8; j += 2)
			drawchar(j + 66, i + 6, 0xdb, 0xff);

	drawstr(57, 1, "0Fh", 0x0f);
	drawstr(20, 16, "F0h", 0x0f);
	drawstr(57, 16, "FFh", 0x0f);

	pal = 0;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 32; j += 2, pal++)
		{
			drawchar(j + 24, i + 1, 0x7f, pal);
			drawchar(j + 25, i + 1, 0x7f, pal);
		}
	}
	ega_set_page();

	/* Wait for user input */
	fgetc(stdin);

	/* Cleanup */
	vga_set_16p();
	ega_blink_on();
	ega_cursor_on();
	drawclear(0x07);

	for(i = 0; i < 16; i++)
	{
		int lvl = !!(i & 8) * 21;
		int r = !!(i & 4) * 42 + lvl;
		int g = !!(i & 2) * 42 + lvl;
		int b = !!(i & 1) * 42 + lvl;
		if(i == 6)
			g = 21;

		outportb(0x3C8, i);
		outportb(0x3C9, r);
		outportb(0x3C9, g);
		outportb(0x3C9, b);
	}
	return 0;
}