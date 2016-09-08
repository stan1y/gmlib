#include "GMLib.h"
#include "GMUI.h"

void render_vertical_gradient(SDL_Renderer* renderer, const color & from, const color& to, const point & start, const point & end, uint8_t alpha)
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
