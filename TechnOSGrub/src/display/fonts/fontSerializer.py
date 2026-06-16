import PIL.Image
import json

import sys

class ImageData:
    def __init__(self, data):
        self.name       = data["name"]
        self.characters = data["characters"]
        self.charset    = len(data["characters"])
        self.font_specs = data["font_specs"]
        self.width      = self.font_specs["width"]
        self.height     = self.font_specs["height"]
        self.before     = self.font_specs["before"]
        self.after      = self.font_specs["after"]
        self.alpha      = self.font_specs["alpha"]
        self.monospace  = self.font_specs["monospace"]

    @property
    def size(self):
        return self.width, self.height

    @size.setter
    def size(self, value):
        self.width, self.height = value

    def __repr__(self):
        return (
            f"ImageData(name={self.name}, charset={self.charset}, "
            f"size={self.size}, font_specs={self.font_specs})"
        )


def serialize(font):
    image = None
    meta = None

    try:
        image = PIL.Image.open(font + ".png")
        if image.mode != "RGB":
            image = image.convert("RGB")
        meta = json.loads(open(font + ".json", "r").read())
    except FileNotFoundError as e:
        print(f"Font png or meta file not found. ({e})")
        return None
    except Exception as e:
        print(f"Font not supported ({e})")
        return None

    data = ImageData(meta)
    print(f"Font {font} decoded: {data!r}")

    if image.size[1] != data.height:
        print("Image height does not match font height.")
        return None

    stride = data.before + data.width + data.after

    name_upper = data.name.upper()
    chars_literal = ",".join(
        f"'\\x{ord(c):02x}'" for c in data.characters
    )

    flat_pixels = []
    for i in range(data.charset):
        startX = i * stride + data.before
        for y in range(data.height):
            for x in range(data.width):
                try:
                    r, g, b = image.getpixel((startX + x, y))
                except IndexError:
                    r, g, b = 0, 0, 0
                flat_pixels.append(str((r + g + b) // 3))

    pixel_literal = ",".join(flat_pixels)

    header = f"""#pragma once
#include "../../include/stdint.h"

#ifndef FONT_{name_upper}
#define FONT_{name_upper}_ID {ord(str(data.name[0]))}{ord(str(data.name[len(data.name) - 1]))}{data.height}{data.width}{data.charset}
#define FONT_{name_upper}

#define FONT_{name_upper}_WIDTH {data.width}
#define FONT_{name_upper}_HEIGHT {data.height}
#define FONT_{name_upper}_CHARSET {data.charset}

static const char FONT_{name_upper}_CHARS[FONT_{name_upper}_CHARSET] = {{{chars_literal}}};

static const uint8_t FONT_{name_upper}_DATA[FONT_{name_upper}_CHARSET * FONT_{name_upper}_HEIGHT * FONT_{name_upper}_WIDTH] = {{{pixel_literal}}};

static inline int16_t font_{data.name}_glyph_idx(char c) {{
    for (int16_t i = 0; i < FONT_{name_upper}_CHARSET; i++) {{
        if (FONT_{name_upper}_CHARS[i] == c) return i;
    }}
    return -1;
}}

static inline const uint8_t* font_{data.name}_glyph(char c) {{
    int16_t idx = font_{data.name}_glyph_idx(c);
    if (idx < 0) return NULL;
    return &FONT_{name_upper}_DATA[(size_t)idx * FONT_{name_upper}_HEIGHT * FONT_{name_upper}_WIDTH];
}}

static inline uint8_t font_{data.name}_pixel(int16_t idx, int x, int y) {{
    return FONT_{name_upper}_DATA[((size_t)idx * FONT_{name_upper}_HEIGHT + y) * FONT_{name_upper}_WIDTH + x];
}}

#endif
"""
    return header


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 fontSerializer.py <font_name>")
        exit()

    open(sys.argv[1]+".h", "w").write(serialize(sys.argv[1]))