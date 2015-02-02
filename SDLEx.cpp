#include "GMLib.h"

void SDLEx_RenderDrawCircle(SDL_Renderer *renderer, int n_cx, int n_cy, int radius)
{
    // if the first pixel in the screen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:
 
    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
    
    while (x >= y)
    {
        
        SDL_RenderDrawPoint(renderer, (int)(cx + x), (int)(cy + y));
        SDL_RenderDrawPoint(renderer, (int)(cx + y), (int)(cy + x));
        
 
        if (x != 0)
        {
            SDL_RenderDrawPoint(renderer, (int)(cx - x), (int)(cy + y));
            SDL_RenderDrawPoint(renderer, (int)(cx + y), (int)(cy - x));
        }
 
        if (y != 0)
        {
            SDL_RenderDrawPoint(renderer, (int)(cx + x), (int)(cy - y));
            SDL_RenderDrawPoint(renderer, (int)(cx - y), (int)(cy + x));
        }
 
        if (x != 0 && y != 0)
        {
            SDL_RenderDrawPoint(renderer, (int)(cx - x), (int)(cy - y));
            SDL_RenderDrawPoint(renderer, (int)(cx - y), (int)(cy - x));
        }
 
        error += y;
        ++y;
        error += y;
 
        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
}

void SDLEx_RenderVerticalGradient(SDL_Renderer* renderer, SDL_Color& from, SDL_Color& to, SDL_Point& start, SDL_Point& end, uint8_t alpha)
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

bool operator== (SDL_Rect& a, SDL_Rect& b) {
    return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

bool operator!= (SDL_Rect& a, SDL_Rect& b) {
    return (a.x != b.x || a.y != b.y || a.w != b.w || a.h != b.h);
}

bool operator== (SDL_Point& a, SDL_Point& b) {
    return (a.x == b.x && a.y == b.y);
}

bool operator!= (SDL_Point& a, SDL_Point& b) {
    return (a.x != b.x || a.y != b.y);
}

void operator+= (SDL_Point& a, SDL_Point& b) {
  a.x += b.x; a.y += b.y;
}

void operator-= (SDL_Point& a, SDL_Point& b) {
  a.x -= b.x; a.y -= b.y;
}
