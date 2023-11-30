import glob
import os
from PIL import Image

output_header = ""
output_source = ""
# Write prefix
output_header += """#ifndef LIGHTMETER_BITMAP_GENERATED_HPP
#define LIGHTMETER_BITMAP_GENERATED_HPP

#include <Adafruit_SSD1306.h>
#include "stdint.h"

namespace bitmap {
    struct bitmap_t {
        uint16_t width, height;
        uint8_t bmp[];
    };

    const PROGMEM uint16_t ARROW_TL_X     = 20;
    const PROGMEM uint16_t ARROW_TL_Y     = 8;
    const PROGMEM uint16_t ARROW_TR_X     = 20;
    const PROGMEM uint16_t ARROW_TR_Y     = 40;
    const PROGMEM uint16_t ARROW_BL_X     = 95;
    const PROGMEM uint16_t ARROW_BL_Y     = 8;
    const PROGMEM uint16_t ARROW_BR_X     = 95;
    const PROGMEM uint16_t ARROW_BR_Y     = 40;
    const PROGMEM uint16_t CIRCLE_RIGHT_X = 112;
    const PROGMEM uint16_t CIRCLE_RIGHT_Y = 16;
    const PROGMEM uint16_t CIRCLE_LEFT_X  = 0;
    const PROGMEM uint16_t CIRCLE_LEFT_Y  = 22;
    const PROGMEM uint16_t GEAR_RIGHT_X   = 33;
    const PROGMEM uint16_t GEAR_RIGHT_Y   = 0;
    const PROGMEM uint16_t GEAR_LEFT_X    = 71;
    const PROGMEM uint16_t GEAR_LEFT_Y    = 0;
"""
output_source += """#include "bitmap_generated.hpp"

"""
os.chdir("res")
for img_path in glob.glob("*.png"):
    img = Image.open(img_path)
    img = img.convert(mode="1")
    img_bmp = img.tobitmap()
    img_byte_arr = img.tobytes()
    output_header += f"""\tconst PROGMEM bitmap_t {img_path[:-4].upper()}  = {{
\t\t{img.size[0]},
\t\t{img.size[1]},
\t\t{{"""
    for b in img_byte_arr:
        output_header += f"{hex(b)}, "
    output_header = output_header[:-2]
    output_header += f"\t}}\n\t}};\n\n"
    img_upper_camel_case = "".join(x.capitalize() for x in img_path[:-4].lower().split("_"))
    output_header += f"""\textern void draw{img_upper_camel_case}(Adafruit_SSD1306 *display, uint8_t x, uint8_t y);
    
"""
    output_source += f"""void bitmap::draw{img_upper_camel_case}(Adafruit_SSD1306 *display, uint8_t x, uint8_t y) {{
\tdisplay->drawBitmap(x,
\t                    y,
\t                    {img_path[:-4].upper()}.bmp,
\t                    {img_path[:-4].upper()}.width,
\t                    {img_path[:-4].upper()}.height,
\t                    1);
}}

"""

output_header += f"""}}

#endif //LIGHTMETER_BITMAP_GENERATED_HPP
"""
output_header_file = open("../src/bitmap_generated.hpp", "w")
output_header_file.write(output_header)
output_header_file.close()
output_source_file = open("../src/bitmap_generated.cpp", "w")
output_source_file.write(output_source)
output_source_file.close()
