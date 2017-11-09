#define USE_ASCII
#include "main.h"

GlyphColor glyphcolor(uint8_t r, uint8_t g, uint8_t b) {
  return (GlyphColor){ r, g, b };
}

GlyphColor glyphcolor_hex(uint32_t hex) {
  return (GlyphColor){ (hex >> 16) & 0xff, (hex >> 8) & 0xff, hex & 0xff };
}

double hue_to_rgb(double p, double q, double t) {
  if (t < 0.0) {
    t++;
  }
  if (t > 1.0) {
    t--;
  }
  if (t < 1.0 / 6.0) {
    return p + (q - p) * 6.0 * t;
  } else if (t < 0.5) {
    return q;
  } else if (t < 2.0 / 3.0) {
    return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
  }
  return p;
}

GlyphColor glyphcolor_hsl(double h, double s, double l) {
  double r = l;
  double g = l;
  double b = l;

  if (s > 0.0) {
    double q = (l < 0.5 ? l * (1.0 + s) : l + s - l * s);
    double p = 2.0 * l - q;
    r = hue_to_rgb(p, q, h + 1.0 / 3.0);
    g = hue_to_rgb(p, q, h);
    b = hue_to_rgb(p, q, h - 1.0 / 3.0);
  }

  return glyphcolor((uint8_t)(r * 255.0), (uint8_t)(g * 255.0), (uint8_t)(b * 255.0));
}

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

bool glyphcolor_eq(GlyphColor a, GlyphColor b) {
  return (a.r == b.r && a.g == b.g && a.b == b.b);
}

uint32_t glyphcolor_to_uint(GlyphColor c) {
  return (c.r << 16) + (c.g << 8) + c.b;
}
