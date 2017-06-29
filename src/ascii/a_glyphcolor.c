#include "ascii.h"

GlyphColor glyphcolor_add(GlyphColor a, GlyphColor b) {
  GlyphColor c = {
    .r = a.r + b.r,
    .g = a.g + b.g,
    .b = a.b + b.b,
  };

  if (c.r < a.r || c.r < b.r) {
    c.r = 255;
  }
  if (c.g < a.g || c.g < b.g) {
    c.g = 255;
  }
  if (c.b < a.b || c.b < b.b) {
    c.b = 255;
  }

  return c;
}

GlyphColor glyphcolor_sub(GlyphColor a, GlyphColor b) {
  GlyphColor c = {
    .r = a.r - b.r,
    .g = a.g - b.g,
    .b = a.b - b.b,
  };

  if (c.r > a.r && c.r > b.r) {
    c.r = 0;
  }
  if (c.g > a.g && c.g > b.g) {
    c.g = 0;
  }
  if (c.b > a.b && c.b > b.b) {
    c.b = 0;
  }

  return c;
}

GlyphColor glyphcolor_adds(GlyphColor c, float s) {
  GlyphColor r = {
    .r = (uint8_t)((float)c.r + s),
    .g = (uint8_t)((float)c.g + s),
    .b = (uint8_t)((float)c.b + s),
  };

  if (r.r < c.r) {
    r.r = 255;
  }
  if (r.g < c.g) {
    r.g = 255;
  }
  if (r.b < c.b) {
    r.b = 255;
  }

  return r;
}

GlyphColor glyphcolor_subs(GlyphColor c, float s) {
  GlyphColor r = {
    .r = (uint8_t)((float)c.r - s),
    .g = (uint8_t)((float)c.g - s),
    .b = (uint8_t)((float)c.b - s),
  };

  if (r.r > c.r) {
    r.r = 0;
  }
  if (r.g > c.g) {
    r.g = 0;
  }
  if (r.b > c.b) {
    r.b = 0;
  }

  return r;
}

GlyphColor glyphcolor_muls(GlyphColor c, float s) {
  GlyphColor r = {
    .r = (uint8_t)((float)c.r * s),
    .g = (uint8_t)((float)c.g * s),
    .b = (uint8_t)((float)c.b * s),
  };

  if (s > 1.0f) {
    if (r.r < c.r) {
      r.r = 255;
    }
    if (r.g < c.g) {
      r.g = 255;
    }
    if (r.b < c.b) {
      r.b = 255;
    }
  }

  return r;
}

GlyphColor glyphcolor_divs(GlyphColor c, float s) {
  GlyphColor r = {
    .r = (uint8_t)((float)c.r / s),
    .g = (uint8_t)((float)c.g / s),
    .b = (uint8_t)((float)c.b / s),
  };

  if (s < 1.0f) {
    if (r.r > c.r) {
      r.r = 0;
    }
    if (r.g > c.g) {
      r.g = 0;
    }
    if (r.b > c.b) {
      r.b = 0;
    }
  }

  return r;
}
