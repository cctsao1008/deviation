/*
    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Deviation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>

#include "common.h"
#include "../common_emu/fltk.h"

static int logical_lcd_width = LCD_WIDTH*2;
/*
 * // since devo10's screen is too small in emulator , we have it zoomed by 2 for both rows and columns
 * color = 0x0 means white, other value means black
 */
void LCD_DrawPixel(unsigned int color)
{
    u8 c;
    int row, col;
    int i, j;
    // for emulator of devo 10, 0x0 means white while others mean black
    c = color ? 0x00 : 0xaa; // 0xaa is grey color(not dot)

    //Fill in 4 dots
    row = 2 * gui.y;
    col = 2 * gui.x;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            gui.image[3*(logical_lcd_width* (row + i) + col + j)] = c;
            gui.image[3*(logical_lcd_width* (row + i) + col + j) + 1] = c;
            gui.image[3*(logical_lcd_width* (row + i) + col + j) + 2] = c;
        }
    }
    gui.x++;
    if(gui.x > gui.xend) {
        gui.x = gui.xstart;
        gui.y += gui.dir;
    }
}

void LCD_Clear(unsigned int color) {
	(void)color;
	memset(gui.image, 0xaa, sizeof(gui.image));

}
