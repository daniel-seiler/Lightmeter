import glob
import os
from PIL import Image

output = ""
# Write prefix
output += """#pragma once

#include <Adafruit_SSD1306.h>
#include "stdint.h"
#include "bitmap.hpp"

namespace bitmap {
"""
os.chdir("res")
for img_path in glob.glob("*.png"):
    img = Image.open(img_path)
    img = img.convert(mode="1")
    img_bmp = img.tobitmap()
    img_byte_arr = img.tobytes()
    output += f"""\tstatic const PROGMEM bitmap_t {img_path[:-4].upper()}  = {{
\t\t{img.size[0]},
\t\t{img.size[1]},
\t\t{{"""
    for b in img_byte_arr:
        output += f"{hex(b)}, "
    output = output[:-2]
    output += f"\t}}\n\t}};\n\n"
    img_upper_camel_case = "".join(x.capitalize() for x in img_path[:-4].lower().split("_"))
    output += f"""\tvoid draw{img_upper_camel_case}(Adafruit_SSD1306 *display, uint8_t x, uint8_t y) {{
\t\tdisplay->drawBitmap(x,
\t\t                    y,
\t\t                    {img_path[:-4].upper()}.bmp,
\t\t                    {img_path[:-4].upper()}.width,
\t\t                    {img_path[:-4].upper()}.height,
\t\t                    1);
\t}}

"""

output += f"}}\n"
output_file = open("../src/bitmap_generated.hpp", "w")
output_file.write(output)
output_file.close()
