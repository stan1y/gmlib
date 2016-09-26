#include <boost/filesystem.hpp>

#include "GMLib.h"
#include "GMUtil.h"
#include "GMTexture.h"
#include "GMSprites.h"
#include "RESLib.h"
#include "GMUI.h"
#include "GMData.h"
#include "GMPython.h"

/* Global State */
static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static SDL_GLContext g_glcontext = nullptr;
static timer* g_frame_timer = nullptr;
static timer* g_fps_timer = nullptr;
static uint32_t g_counted_frames = 0;
static uint32_t g_screen_ticks_per_frame = 0;

static float g_avg_fps = 0.0f;
static texture g_fps;
static color g_fps_color;
static TTF_Font* g_fps_font;

/* Screens */
static sdl_mutex g_screen_lock;
static screen * g_screen_current;
static screen * g_screen_next;

SDL_Window* GM_GetWindow() {
    if (g_window == nullptr) {
        SDLEx_LogError("%s: not initialized", __METHOD_NAME__);
        return nullptr;
    }
    return g_window;
}

SDL_Renderer* GM_GetRenderer() {
  if (g_renderer == nullptr) {
    SDLEx_LogError("%s: not initialized", __METHOD_NAME__);
    return nullptr;
  }
  return g_renderer;
}

int GM_Init(const std::string & cfg_path, const std::string & name) {

    //check if we're loaded up already
    if (g_window != nullptr || g_renderer != nullptr ) {
        return 0;
    }

    //init RND
    srand((int)time(NULL));

    //init subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        SDLEx_LogError("SDL_Init: Failed to initialize SDL. SDL Error: %s", SDL_GetError());
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == -1) {
        SDLEx_LogError("IMG_Init: Failed to initialize SDL_img. SDL Error: %s", SDL_GetError());
        return -1;
    }
    if (TTF_Init() == -1) {
        SDLEx_LogError("TTF_Init: Failed to initialize SDL_ttf. SDL Error: %s", SDL_GetError());
        return -1;;
    }

    // get SDL versions
    SDL_version c_ver;
    SDL_VERSION(&c_ver);
    SDL_version l_ver;
    SDL_GetVersion(&l_ver);

    printf("Starting...\n");
    printf("GMLib        %d.%d.%s.%d\n", GM_LIB_MAJOR, GM_LIB_MINOR, GM_LIB_RELESE, GM_LIB_PATCH);
    printf("SDL runtime  %d.%d.%d\n", l_ver.major, l_ver.minor, l_ver.patch);
    printf("SDL compiled %d.%d.%d\n", c_ver.major, c_ver.minor, c_ver.patch);

    //check cfg path is ok
    if (cfg_path.empty()) {
      SDLEx_LogError("%s: invalid config path", __METHOD_NAME__);
      return -1;
    }
    boost::filesystem::path p_path(cfg_path);
    if (!boost::filesystem::exists(p_path)) {
      SDLEx_LogError("GM_Init: config path does not exist");
      return -1;
    }
    boost::filesystem::path abspath = boost::filesystem::absolute(p_path);
    config::load(abspath.string());
    const config * cfg = GM_GetConfig();
    
    //init SDL window & renderer
    const rect srect = cfg->screen_rect();
    g_window = SDL_CreateWindow(name.c_str(), 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        srect.w, srect.h, cfg->window_flags() | SDL_WINDOW_OPENGL);
    if ( g_window == nullptr ) {
        SDLEx_LogError("%s: Failed to system window. SDL Error: %s", __METHOD_NAME__, SDL_GetError());
        return -1;
    }
    // setup renderer
    uint32_t didx = cfg->driver_index();
    g_renderer = SDL_CreateRenderer(g_window, didx, cfg->renderer_flags());
    if ( g_renderer == nullptr ) {
        SDLEx_LogError("%s: Failed to create renderer driver_index=%d", __METHOD_NAME__, didx);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    // log renderer info
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(g_renderer, &renderer_info);
    SDL_Log("%s: screen ready: %s; fullscreen: %s; driver: %s",
      __METHOD_NAME__,
      srect.tostr().c_str(), 
      ( cfg->fullscreen() ? "yes" : "no" ), 
      renderer_info.name);
    
    // resources cache
    resources::initialize(GM_GetConfig()->assets_path());

    //setup random
    srand((unsigned int)time(NULL));

    // init python
    python::initialize(cfg);

    //setup game state
    g_frame_timer = new timer();
    g_screen_ticks_per_frame = 1000 / cfg->fps_cap();
    
    // init UI
    rect display = GM_GetDisplayRect();
    ui::manager::initialize(display);
    
    //setup screens
    if (cfg->calculate_fps()) {
      g_fps_timer = new timer();
    }
    g_screen_current = nullptr;
    g_screen_next = nullptr;

    //fps counter
    g_fps_color = color( 0, 255, 0, 255 );
    g_fps_font = GM_LoadFont(resources::find("terminus.ttf"), 12);

    SDL_Log("%s: ready", __METHOD_NAME__);
    return 0;
}

uint32_t GM_GetFrameTicks()
{
  return g_frame_timer->get_ticks();
}

float GM_CurrentFPS()
{
  return g_avg_fps;
}

void GM_Quit() 
{
  python::shutdown();
  SDL_Quit();
}

void GM_StartFrame()
{
  if (g_fps_timer) {

    //init fps timer first on first frame
    if(!g_fps_timer->is_started()) g_fps_timer->start();

    //update avg fps
    g_avg_fps = g_counted_frames / ( g_fps_timer->get_ticks() / 1000.f );
    if (g_avg_fps > 2000000) {
      g_avg_fps = 0;
    }  
  }

  //clear screen with black
   SDL_SetRenderTarget(g_renderer, NULL);
   SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
   SDL_RenderClear(g_renderer);
}

void GM_UpdateFrame()
{
  //update game state
  {
    mutex_lock guard(g_screen_lock);
    if (g_screen_current != g_screen_next) {
      g_screen_current = g_screen_next;
      SDL_Log("%s - screen %p is now active", 
        __METHOD_NAME__, g_screen_current);
    }
  }
    
  // update global & current screens
  if (g_screen_current != nullptr) {
    g_screen_current->update();  
  }
}

void GM_RenderFrame()
{
  // update current screen
  if (g_screen_current == NULL) {
    SDLEx_LogError("%s - failed to render: no active screen", 
      __METHOD_NAME__);
    throw std::exception("No active screen to render");
  }
  SDL_Renderer * r = GM_GetRenderer();
  
  g_screen_current->render(r);
  
  // render avg fps
  if (g_fps_timer) {
    g_fps.load_text_solid( std::string("fps: ") + std::to_string(float_to_sint32(GM_CurrentFPS())), g_fps_font, g_fps_color);
    g_fps.render(GM_GetRenderer(), point(5, 5));
  }

  //re-start frame timer
  g_frame_timer->start();
}

void GM_EndFrame()
{
  //swap opengl buffers
  SDL_RenderPresent(g_renderer);
  //update counted frames and delay frame end
  ++g_counted_frames;

  uint32_t frame_ticks = GM_GetFrameTicks();
  if (frame_ticks < g_screen_ticks_per_frame) {
    SDL_Delay(g_screen_ticks_per_frame - frame_ticks);
  }
}

/* 
    Game Screens 
*/

static int GM_ScreenEventHandler(void* ptr, SDL_Event* ev)
{
  screen* self = static_cast<screen*> (ptr);
  self->on_event(ev);
  return 0;
}

screen::screen():
  _wnd(GM_GetWindow()),
  _glctx(SDL_GL_CreateContext(_wnd))
{
  SDL_AddEventWatch(GM_ScreenEventHandler, this);
}

screen::screen(SDL_Window* wnd):
  _wnd(wnd),
  _glctx(SDL_GL_CreateContext(_wnd))
{
  SDL_AddEventWatch(GM_ScreenEventHandler, this);
}

screen* screen::current() 
{
  return g_screen_current;
}

void screen::set_current(screen* s)
{
  if (g_screen_current != s && g_screen_next != s) {
      // requested screen becomes next one
      g_screen_next = s;
    }
}

/* Load helpers */

SDL_Surface* GM_LoadSurface(const std::string& file_path)
{
  SDL_Surface *tmp = IMG_Load(file_path.c_str());
  if (!tmp) {
    SDLEx_LogError("GM_LoadSurface: failed to load %s: %s", file_path.c_str(), SDL_GetError());
    throw std::exception("Failed to load surface from file");
  }

  SDL_Surface* s = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA8888, NULL );
  if (!s) {
    SDLEx_LogError("GM_LoadSurface: failed to convert surface: %s", file_path.c_str(), SDL_GetError());
    throw std::exception("Failed to convert surface");
  }
  SDL_FreeSurface(tmp);

  return s;
}

SDL_Texture* GM_LoadTexture(const std::string& file_path)
{
    SDL_Surface* s = GM_LoadSurface(file_path);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(GM_GetRenderer(), s);
    SDL_FreeSurface(s);
    return tex;
}

TTF_Font* GM_LoadFont(const std::string& file_path, int ptsize)
{
  TTF_Font* f = TTF_OpenFont(file_path.c_str(), ptsize);
  if (!f) {
      SDLEx_LogError("GM_LoadFont: failed to load %s", file_path.c_str());
      throw std::exception("Failed to load font from file");
  }
  return f;
}

/* Timer implementation */

timer::timer()
{
  _started_ticks = 0;
  _paused_ticks = 0;
  _started = false;
  _paused = false;
}

void timer::start()
{
  _started = true;
  _paused = false;
  _started_ticks = SDL_GetTicks();
  _paused_ticks = 0;
}

void timer::stop()
{
  _started = false;
  _paused = false;
  _started_ticks = 0;
  _paused_ticks = 0;
}

void timer::pause()
{
  if (_started && !_paused)
  {
    _paused = false;
    _paused_ticks = SDL_GetTicks() - _started_ticks;
    _started_ticks = 0;
  }
}

void timer::unpause()
{
  if (_started && _paused)
  {
    _paused = false;
    _started_ticks = SDL_GetTicks() - _paused_ticks;
    _paused_ticks = 0;
  }
}

uint32_t timer::get_ticks() const
{
  if (!_started) return 0;

  if (_paused) {
    return _paused_ticks;
  }
  else {
    return SDL_GetTicks() - _started_ticks;
  }
}

/* Core classes */

/* test collision of point and circle */
bool point::collide_circle(const point & center, const int radius) 
{
  int square_dist = double_to_sint32(std::pow(center.x - x, 2)) + 
    double_to_sint32(std::pow(center.y - y, 2));
  return (square_dist <= double_to_sint32(std::pow(radius, 2)));
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

rect operator- (const rect& a, const rect& b) {
  return rect(a.x - b.x, a.y - b.y, a.w - b.w, a.h - b.h);
}

rect operator+= (rect& a, const rect& b) {
  a.x += b.x; a.y += b.y; a.w += b.w; a.h += b.h;
  return a;
}

rect operator-= (rect& a, const rect& b) {
  a.x -= b.x; a.y -= b.y; a.w -= b.w; a.h -= b.h;
  return a;
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
  const ui::theme & th = ui::current_theme();
  if (sclr == std::string("idle") || 
      sclr == std::string("normal") || 
      sclr == std::string("default")) {
    return th.color_idle;
  }
  if (sclr == std::string("back")) {
    return th.color_back;
  }
  if (sclr == std::string("highlight")) {
    return th.color_highlight;
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

rect rect::scale(const float & fw, const float & fh) const
{
  rect r(*this);
  int dx = double_to_sint32(((w * fw) - w) / 2.0);
  int dy = double_to_sint32(((h * fh) - h) / 2.0);
  r.x += dx;
  r.y += dy;
  r.w -= 2 * dx;
  r.h -= 2 * dy;
  return r;
}

rect rect::center(const uint32_t for_w, const uint32_t for_h) const
{
  return rect(x + (w - for_w) / 2, y + (h - for_h) / 2, for_w, for_h);
}