#ifndef _SDLEX_H_
#define _SDLEX_H_

#include <math.h>
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#  if defined(DLL_EXPORT) && !defined(SDLEX_DLL_IMPORT)
#    define SDLEX_API __declspec(dllexport)
#  else
#    ifdef SDLEX_DLL_IMPORT
#      define SDLEX_API __declspec(dllimport)
#    endif
#  endif
#endif
#ifndef SDLEX_API
#  define SDLEX_API extern
#endif


  SDLEX_API int SDLEx_UpdateViewport(SDL_Renderer * renderer);

  /* Load pixels from surface into blank texture. The texture must be created before. 
     This allows to have a SDL_STREAMING_TEXTURE loaded from surface or file
  */
  SDLEX_API int SDLEx_LoadSurfacePixels(SDL_Surface* src, SDL_Texture* dst);

  /* Replaces pixels matched by RGBA filter with another RGBA color. */
  SDLEX_API int SDLEx_ReplaceColor(SDL_Texture* tex, SDL_Color* from, SDL_Color* to);

  /* Get pixel color at given position */
  SDLEX_API uint32_t SDLEx_GetPixel(SDL_Surface *surface, int x, int y);

  /* Put pixel color at given coordinates*/
  SDLEX_API void SDLEx_PutPixel(SDL_Surface *surface, int x, int y, uint32_t pixel);

  /* Point with alpha-weight */

  SDLEX_API int SDLEx_RenderDrawPointWeight(SDL_Renderer* renderer, int x, int y, uint32_t weight);

  /* Rectangle */

  SDLEX_API int SDLEx_RenderDrawRect(SDL_Renderer * renderer, int x1, int y1, int x2, int y2);
  SDLEX_API int SDLEx_RenderFillRect(SDL_Renderer * renderer, int x1, int y1, int x2, int y2);

  /* Rounded-Corner Rectangle */

  SDLEX_API int SDLEx_RenderDrawRoundedRect(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int rad);
  SDLEX_API int SDLEx_RenderFillRoundedRect(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int rad);

  /* AA Line */

  SDLEX_API int SDLEx_RenderDrawLine(SDL_Renderer * renderer, int x1, int y1, int x2, int y2);

  /* Thick Line */
  SDLEX_API int SDLEx_RenderDrawThickLine(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, uint8_t width);

  /* Circle */

  SDLEX_API int SDLEx_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int rad);
  SDLEX_API int SDLEx_RenderDrawAACircle(SDL_Renderer * renderer, int x, int y, int rad);
  SDLEX_API int SDLEx_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int rad);

  /* Arc */

  SDLEX_API int SDLEx_RenderDrawArc(SDL_Renderer * renderer, int x, int y, int rad, int start, int end);

  /* Ellipse */

  SDLEX_API int SDLEx_RenderDrawEllipse(SDL_Renderer * renderer, int x, int y, int rx, int ry);
  SDLEX_API int SDLEx_RenderDrawAAEllipse(SDL_Renderer * renderer, int x, int y, int rx, int ry);
  SDLEX_API int SDLEx_RenderFillEllipse(SDL_Renderer * renderer, int x, int y, int rx, int ry);

  /* Pie */

  SDLEX_API int SDLEx_RenderDrawPie(SDL_Renderer * renderer, int x, int y, int rad, int start, int end);
  SDLEX_API int SDLEx_RenderFillPie(SDL_Renderer * renderer, int x, int y, int rad, int start, int end);

  /* Trigon */

  SDLEX_API int SDLEx_RenderDrawTrigon(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int x3, int y3);
  SDLEX_API int SDLEx_RenderDrawAATrigon(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int x3, int y3);
  SDLEX_API int SDLEx_RenderFillTrigon(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int x3, int y3);
  
  /* Polygon */

  SDLEX_API int SDLEx_RenderDrawPolygon(SDL_Renderer * renderer, const int * vx, const int * vy, int n);
  SDLEX_API int SDLEx_RenderDrawAAPolygon(SDL_Renderer * renderer, const int * vx, const int * vy, int n);
  SDLEX_API int SDLEx_RenderFillPolygon(SDL_Renderer * renderer, const int * vx, const int * vy, int n);
  SDLEX_API int SDLEx_RenderDrawTexturedPolygon(SDL_Renderer * renderer, const int * vx, const int * vy, int n, SDL_Surface * texture,int texture_dx,int texture_dy);

  /* Bezier */

  SDLEX_API int SDLEx_RenderDrawBezierCurve(SDL_Renderer * renderer, const int * vx, const int * vy, int n, int s);

#ifdef __cplusplus
}
#endif

#endif //_SDLEX_H_