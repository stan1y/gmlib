#include "sdl_ex.h"
#include <memory>

int SDLEx_LoadSurfacePixels(SDL_Surface* src, SDL_Texture* dst)
{
  void* pixels = NULL;
  int pitch = 0;
  int result = 0;

  result = SDL_LockTexture(dst, NULL, &pixels, &pitch);
  if (result != 0) {
    return result;
  }
  memcpy(pixels, src->pixels, src->pitch * src->h);
  SDL_UnlockTexture(dst);

  return 0;
}

int SDLEx_ReplaceColor(SDL_Texture* tex, SDL_Color* from, SDL_Color* to)
{
  void* raw_pixels = NULL;
  int pitch = 0, result = 0, access = 0, w = 0, h = 0;
  uint32_t format;

  result |= SDL_QueryTexture(tex, &format, &access, &w, &h);
  result |= SDL_LockTexture(tex, NULL, &raw_pixels, &pitch);
  
  SDL_PixelFormat* fmt = SDL_AllocFormat(format);
  uint32_t f = SDL_MapRGBA(fmt, from->r, from->g, from->b, from->a);
  uint32_t t = SDL_MapRGBA(fmt, to->r, to->g, to->b, to->a);
  SDL_FreeFormat(fmt);

  uint32_t* pixels = (uint32_t*)raw_pixels;
  //int pixel_count = ( pitch / 4 ) * h;
  int pixel_count = ( pitch / fmt->BytesPerPixel ) * h;
  for( int i = 0; i < pixel_count; ++i ) {
      if( pixels[i] == f ) {
          pixels[i] = t;
      }
  }

  SDL_UnlockTexture(tex);
  return result;
}

uint32_t SDLEx_GetPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(uint16_t *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(uint32_t *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void SDLEx_PutPixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(uint16_t *)p = pixel;
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
        *(uint32_t *)p = pixel;
        break;
    }
}

/*
int SDLEx_UpdateViewport(SDL_Renderer * renderer)
{
    GL_RenderData *data = (GL_RenderData *) renderer->driverdata;

    if (SDL_CurrentContext != data->context) {
        // We'll update the viewport after we rebind the context
        return 0;
    }

    if (renderer->target) {
        data->glViewport(renderer->viewport.x, renderer->viewport.y,
                         renderer->viewport.w, renderer->viewport.h);
    } else {
        int w, h;

        SDL_GetRendererOutputSize(renderer, &w, &h);
        data->glViewport(renderer->viewport.x, (h - renderer->viewport.y - renderer->viewport.h),
                         renderer->viewport.w, renderer->viewport.h);
    }

    data->glMatrixMode(GL_PROJECTION);
    data->glLoadIdentity();
    if (renderer->viewport.w && renderer->viewport.h) {
        if (renderer->target) {
            data->glOrtho((GLdouble) 0,
                          (GLdouble) renderer->viewport.w,
                          (GLdouble) 0,
                          (GLdouble) renderer->viewport.h,
                           0.0, 1.0);
        } else {
            data->glOrtho((GLdouble) 0,
                          (GLdouble) renderer->viewport.w,
                          (GLdouble) renderer->viewport.h,
                          (GLdouble) 0,
                           0.0, 1.0);
        }
    }
    return GL_CheckError("", renderer);
}*/
