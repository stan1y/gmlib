#include <limits.h>
#include <boost/filesystem.hpp>

#include "GMVersion.h"
#include "GMLib.h"
#include "GMTexture.h"
#include "GMSprites.h"
#include "RESLib.h"
#include "GMUI.h"
#include "GMData.h"

/* Global State */
static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static SDL_GLContext g_glcontext = nullptr;
static timer* g_frame_timer = nullptr;
static timer* g_fps_timer = nullptr;
static uint32_t g_counted_frames = 0;
static uint32_t g_screen_ticks_per_frame = 0;

static bool gframe_calculate_fps = false;
static float g_avg_fps = 0.0f;
static texture g_fps;
static color g_fps_color;
static TTF_Font* g_fps_font;

/* Screens */
static screen * g_screen_current;
static screen * g_screen_next;
static screen * g_screen_ui;

SDL_Window* GM_GetWindow() {
    if (g_window == nullptr) {
        SDLEx_LogError("GM_GetWindow: not initialized");
        return nullptr;
    }
    return g_window;
}

SDL_Renderer* GM_GetRenderer() {
  if (g_renderer == nullptr) {
    SDLEx_LogError("GM_GetRenderer: not initialized");
    return nullptr;
  }
  return g_renderer;
}


int GM_Init(const std::string & cfg_path, const std::string & name) {

    //check if we're loaded up already
    if (g_window != nullptr || g_renderer != nullptr ) {
        return 0;
    }

    //check cfg path is ok
    if (cfg_path.empty()) {
      SDLEx_LogError("GM_Init: invalid config path");
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

    //init RND
    srand((int)time(NULL));

    //init subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        SDLEx_LogError("GM_Init: Failed to initialize SDL. SDL Error: %s", SDL_GetError());
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == -1) {
        SDLEx_LogError("GM_Init: Failed to initialize SDL_img. SDL Error: %s", SDL_GetError());
        return -1;
    }
    if (TTF_Init() == -1) {
        SDLEx_LogError("GM_Init: Failed to initialize SDL_ttf. SDL Error: %s", SDL_GetError());
        return -1;;
    }
    SDL_version c_ver;
    SDL_VERSION(&c_ver);
    SDL_version l_ver;
    SDL_GetVersion(&l_ver);
    SDL_Log("GM_Init: GMLib ver. %d.%d; SDL runtime ver. %d.%d.%d; complied with ver. %d.%d.%d",
      GM_LIB_MAJOR, GM_LIB_MINOR,
      l_ver.major, l_ver.minor, l_ver.patch,
      c_ver.major, c_ver.minor, c_ver.patch);

    //setup random
    srand((unsigned int)time(NULL));

    //setup game state
    g_frame_timer = new timer();
    g_screen_ticks_per_frame = 1000 / cfg->fps_cap();
    g_fps_timer = new timer();
    
    //setup screens
    bool fps_state = cfg->calculate_fps();
    GM_SetFPS(fps_state);
    g_screen_current = nullptr;
    g_screen_next = nullptr;
    g_screen_ui = new screen();
    
    //init SDL window & renderer
    const rect screen = cfg->screen_rect();
    g_window = SDL_CreateWindow(name.c_str(), 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        screen.w, screen.h, cfg->window_flags() | SDL_WINDOW_OPENGL);
    if ( g_window == nullptr ) {
        SDLEx_LogError("GM_Init: Failed to system window. SDL Error: %s", SDL_GetError());
        return -1;
    }
    // setup renderer
    uint32_t didx = cfg->driver_index();
    g_renderer = SDL_CreateRenderer(g_window, didx, cfg->renderer_flags());
    if ( g_renderer == nullptr ) {
        SDLEx_LogError("GM_Init: Failed to create renderer driver_index=%d", didx);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    // log renderer info
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(g_renderer, &renderer_info);
    SDL_Log("GM_Init: screen ready: %s; fullscreen: %s; driver: %s", 
      screen.tostr().c_str(), 
      ( cfg->fullscreen() ? "yes" : "no" ), 
      renderer_info.name);
    
    // resources cache
    RES_Init(GM_GetConfigData().get<std::string>
      ("assets_path",
      "resources")
    );

    // init UI
    rect display = GM_GetDisplayRect();
    ui::manager::initialize(display);
    g_screen_ui->add_component(ui::manager::instance());

    //fps counter
    g_fps_color = color( 0, 255, 0, 255 );
    g_fps_font = GM_LoadFont("fonts/terminus.ttf", 12);

    SDL_Log("GM_Init: done.");
    return 0;
}

uint32_t GM_GetFrameTicks()
{
  return g_frame_timer->get_ticks();
}

float GM_GetAvgFPS()
{
  return g_avg_fps;
}

void GM_SetFPS(bool state)
{
  SDL_Log("GM_SetFPS: fps calc is %s", ( state ? "on" : "off"));
  gframe_calculate_fps = state;
}

void GM_Quit() 
{
  SDL_Quit();
}

void GM_StartFrame()
{
  //init fps timer first on first frame
  if (gframe_calculate_fps && !g_fps_timer->is_started()) {
    g_fps_timer->start();
  }

  //update avg fps
  g_avg_fps = g_counted_frames / ( g_fps_timer->get_ticks() / 1000.f );
  if (g_avg_fps > 2000000) {
    g_avg_fps = 0;
  }

  //clear screen with black
   SDL_SetRenderTarget(g_renderer, NULL);
   SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
   SDL_RenderClear(g_renderer);
}

void GM_UpdateFrame()
{
  //update game state
  if (g_screen_current != g_screen_current) {
    g_screen_current = g_screen_next;
    SDL_Log("Screen %p is now active", g_screen_current);
  }

  // update running animations
  anim::update_running();
    
  // update global & current screens
  if (g_screen_current != nullptr) {
    g_screen_current->update();  
  }
  g_screen_ui->update();
}

void GM_RenderFrame()
{
  // update current screen
  if (g_screen_current == NULL) {
    SDLEx_LogError("Failed to render: no active screen");
    throw std::exception("No active screen to update");
  }
  SDL_Renderer * r = GM_GetRenderer();
  
  g_screen_current->activate();
  g_screen_current->render(r);
  
  g_screen_ui->activate();
  g_screen_ui->render(r);

  // render avg fps
  if (gframe_calculate_fps) {
    g_fps.load_text_solid( std::string("fps: ") + std::to_string(float_to_sint32(GM_GetAvgFPS())), g_fps_font, g_fps_color);
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
        g_screen_current = s;
    }
}


/* Load helpers */

SDL_Surface* GM_LoadSurface(const std::string& image)
{
  std::string path = RES_GetFullPath(image);
  SDL_Surface *tmp = IMG_Load(path.c_str());
  if (!tmp) {
    SDLEx_LogError("GM_LoadSurface: failed to load %s: %s", path.c_str(), SDL_GetError());
    throw std::exception("Failed to load surface from file");
  }

  SDL_Surface* s = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA8888, NULL );
  if (!s) {
    SDLEx_LogError("GM_LoadSurface: failed to convert surface: %s", path.c_str(), SDL_GetError());
    throw std::exception("Failed to convert surface");
  }
  SDL_FreeSurface(tmp);

  return s;
}

SDL_Texture* GM_LoadTexture(const std::string& sheet)
{
    SDL_Surface* s = GM_LoadSurface(sheet);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(GM_GetRenderer(), s);
    SDL_FreeSurface(s);
    return tex;
}

TTF_Font* GM_LoadFont(const std::string& font, int ptsize)
{
  std::string path = RES_GetFullPath(font);
  TTF_Font* f = TTF_OpenFont(path.c_str(), ptsize);
  if (!f) {
      SDLEx_LogError("GM_LoadFont: failed to load %s", path.c_str());
      return nullptr;
  }
  return f;
}

/* File and folder helpers */

std::string GM_GetExecutablePath()
{
  static char exe[MAX_PATH + 1];
  memset(exe, 0, (MAX_PATH + 1) * sizeof(char));

#ifdef _WIN32
  if (GetModuleFileNameA(NULL, exe, MAX_PATH) == 0) {
    SDLEx_LogError("Failed to query executable path");
    throw std::exception("Failed to query executable path");
  }
#endif

#ifdef _linux_
#endif

  return exe;
}

std::string GM_GetCurrentPath()
{
  static char cwd[MAX_PATH + 1];
  memset(cwd, 0, (MAX_PATH + 1) * sizeof(char));
#ifdef _WIN32
  _getcwd(cwd, MAX_PATH * sizeof(char));
#endif

  return std::string(cwd);
}

void GM_EnumPath(const std::string& folder, std::vector<std::string>& files, bool recursive)
{
  GM_EnumPathEx(folder, "", files, recursive);
}

template<typename Iter>
void EnumPathEx(const std::string& folder, const std::string& ext, std::vector<std::string>& files)
{
  Iter iter(folder), eod;
  for (; iter != eod; ++iter) {
    if ( is_regular_file(iter->path())) {
      if (ext.length() > 0 && iter->path().extension().string() != ext)
        continue;
      files.push_back(iter->path().string());
    }
  }
}

void GM_EnumPathEx(const std::string& folder, const std::string& ext, std::vector<std::string>& files, bool recursive)
{
  boost::filesystem::path dir(folder);
  if (recursive) {
    EnumPathEx<boost::filesystem::recursive_directory_iterator>(folder, ext, files);
  }
  else {
    EnumPathEx<boost::filesystem::directory_iterator>(folder, ext, files);
  }
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