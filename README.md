# smzxtest
Tests if a VGA-compatible chipset supports MegaZeux's 256 color text mode ("Super MegaZeux").

## Usage

This program requires no 32-bit extenders or other external files; just copy it
to your DOS installation or a FreeDOS USB installer drive and run it from the
DOS prompt.

For best results, submit an issue with your hardware and
*a picture of the screen* and one of us can add it to the
[wiki](https://www.digitalmzx.com/wiki/Super_MegaZeux#Compatibility).
You should provide the specific video adapter model and, if it is
embedded in a motherboard/laptop/CPU, the specific model of the device
it is embedded in.

## Compiling

Requires Borland Turbo C 2.01 and Borland Turbo Assembler.

```bat
tcc -B smzxtest
```

## License

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
