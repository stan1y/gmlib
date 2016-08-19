#include "GMLib.h"
#include "GMUI.h"

void RenderVerticalGradient(SDL_Renderer* renderer, const color & from, const color& to, const point & start, const point & end, uint8_t alpha)
{
  int l = end.y - start.y;
  int rl = to.r - from.r,
    gl = to.g - from.g,
    bl = to.b - from.b;
  uint8_t r, g, b;
  for(int x = start.x; x < (start.x + end.x); x++) {
    for(int y = start.y; y < (start.y + end.y); y++) {
      float ratio = (l - (float)y) / l;
      r = static_cast<uint8_t>(from.r + rl * ratio);
      g = static_cast<uint8_t>(from.g + gl * ratio);
      b = static_cast<uint8_t>(from.b + bl * ratio);

      SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
      SDL_RenderDrawPoint(renderer, x, y);
    }
  }
}

Uint32 SDLEx_GetPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void SDLEx_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

bool operator== (rect& a, rect& b) {
    return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

bool operator!= (rect& a, rect& b) {
    return (a.x != b.x || a.y != b.y || a.w != b.w || a.h != b.h);
}

rect operator+ (const rect& a, const rect& b) {
  return rect(a.x + b.x, a.y + b.y, a.w + b.w, a.h + b.h);
}

rect operator+ (const rect& r, const point& p) {
  return rect(r.x + p.x, r.y + p.y, r.w, r.h);
}

rect operator- (const rect& r, const point& p) {
  return rect(r.x - p.x, r.y - p.y, r.w, r.h);
}

point operator+ (const point& a, const point& b) {
  return point(a.x + b.x, a.y + b.y);
}

point operator- (const point& a, const point& b) {
  return point(a.x - b.x, a.y - b.y);
}

bool operator== (point& a, point& b) {
    return (a.x == b.x && a.y == b.y);
}

bool operator!= (point& a, point& b) {
    return (a.x != b.x || a.y != b.y);
}

void operator+= (point& a, point& b) {
  a.x += b.x; a.y += b.y;
}

void operator-= (point& a, point& b) {
  a.x -= b.x; a.y -= b.y;
}

void operator+= (rect& r, point& p) {
  r.x += p.x; r.y += p.y;
}

void operator-= (rect& r, point& p) {
  r.x -= p.x; r.y -= p.y;
}

void color::apply(SDL_Renderer* rnd) const
{
  SDL_SetRenderDrawColor(rnd, r, g, b, a);
}

void color::apply() const
{
  apply(GM_GetRenderer());
}

color color::from_string(const std::string & sclr)
{
  const ui::theme & th = UI_GetTheme();
  if (sclr == std::string("front")) {
    return th.color_front;
  }
  if (sclr == std::string("back")) {
    return th.color_back;
  }
  if (sclr == std::string("highlight")) {
    return th.color_highlight;
  }
  if (sclr == std::string("toolbox")) {
    return th.color_toolbox;
  }
  if (sclr == std::string("red")) {
    return color::red();
  }
  if (sclr == std::string("green")) {
    return color::green();
  }
  if (sclr == std::string("blue")) {
    return color::blue();
  }
  if (sclr == std::string("white")) {
    return color::white();
  }
  if (sclr == std::string("black")) {
    return color::black();
  }
  if (sclr == std::string("dark_red")) {
    return color::dark_red();
  }
  if (sclr == std::string("dark_blue")) {
    return color::dark_blue();
  }
  if (sclr == std::string("dark_gray")) {
    return color::dark_gray();
  }
  if (sclr == std::string("gray")) {
    return color::gray();
  }
  if (sclr == std::string("light_gray")) {
    return color::light_gray();
  }

  SDLEx_LogError("color::from_string - unknown name %s", sclr.c_str());
  throw std::exception("unknown color name");
}