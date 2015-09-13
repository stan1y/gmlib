#include <limits.h>
#include <boost/filesystem.hpp>

#include "GMLib.h"
#include "GMTexture.h"
#include "GMSprites.h"
#include "RESLib.h"
#include "GMUI.h"

using namespace boost::filesystem;

/* Global State */
static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static SDL_GLContext g_glcontext = nullptr;
static config* g_config = nullptr;
static timer* g_frame_timer = nullptr;
static timer* g_fps_timer = nullptr;
static uint32_t g_counted_frames = 0;
static uint32_t g_screen_ticks_per_frame = 0;
static float g_avg_fps = 0.0f;
static texture g_fps;
static color g_fps_color;
static TTF_Font* g_fps_font;

/* Screens */
static screen * g_screen_current;
static screen * g_screen_next;
static screen * g_screen_global;

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

const config* GM_GetConfig() {
  if (g_config == nullptr) {
    SDLEx_LogError("GM_GetConfig: not initialized");
    return nullptr;
  }
  return g_config;
}

int GM_Init(const char* name, config* conf) {

    //check if we're loaded up already
    if ( g_config != nullptr || g_window != nullptr || g_renderer != nullptr ) {
        return 0;
    }

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
    SDL_Log("Initalized SDL %d.%d.%d. Complied with %d.%d.%d",
        l_ver.major, l_ver.minor, l_ver.patch,
        c_ver.major, c_ver.minor, c_ver.patch);

    //setup random
    srand((unsigned int)time(NULL));

    //setup game state
    g_config = conf;
    g_frame_timer = new timer();
    g_screen_ticks_per_frame = 1000 / conf->fps_cap;
    g_fps_timer = new timer();
    
    //setup screens
    g_screen_current = nullptr;
    g_screen_next = nullptr;
    g_screen_global = new screen();
    
    //init SDL window & renderer
    g_window = SDL_CreateWindow(name, 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        conf->display_width, conf->display_height, conf->window_flags | SDL_WINDOW_OPENGL);
    if ( g_window == nullptr ) {
        SDLEx_LogError("GM_Init: Failed to system window. SDL Error: %s", SDL_GetError());
        return -1;
    }
    // setup renderer
    g_renderer = SDL_CreateRenderer(g_window, conf->driver_index, conf->renderer_flags);
    if ( g_renderer == nullptr ) {
        SDLEx_LogError("GM_Init: Failed to create renderer driver_index=%d", conf->driver_index);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    // log renderer info
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(g_renderer, &renderer_info);
    SDL_Log("Screen ready size=%dx%d, fullscreen=%d, driver=%s", conf->display_width, conf->display_height, conf->fullscreen, renderer_info.name);
    
    // resources cache
    RES_Init(conf->assets_path);

    // init UI
    rect display = GM_GetDisplayRect();
    ui::manager::initialize(display, g_config->ui_flags);
    g_screen_global->add_component(ui::manager::instance());

    //fps counter
    g_fps_color = color( 0, 255, 0, 255 );
    g_fps_font = GM_LoadFont("fonts/terminus.ttf", 12);

    return 0;
}

inline uint32_t GM_GetFrameTicks()
{
  return g_frame_timer->get_ticks();
}

float GM_GetAvgFps()
{
  return g_avg_fps;
}

void GM_Quit() 
{
  SDL_Quit();
}

void GM_StartFrame()
{
  //init fps timer first on first frame
  if (g_config->calculate_fps && !g_fps_timer->is_started()) {
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
  g_screen_global->update();
}

void GM_RenderFrame()
{
  // update current screen
  if (g_screen_current == NULL) {
    SDLEx_LogError("Failed to render: no active screen");
    throw std::exception("No active screen to update");
  }
  SDL_Renderer * r = GM_GetRenderer();
  g_screen_current->render(r);
  g_screen_global->render(r);

  // render avg fps
  if (g_config->calculate_fps) {
    g_fps.load_text_solid( std::string("fps: ") + std::to_string(float_to_sint32(GM_GetAvgFps())), g_fps_font, g_fps_color);
    g_fps.render(GM_GetRenderer(), point(g_config->display_width - g_fps.width() - 5, 5));
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

screen::screen()
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
  path dir(folder);
  if (recursive) {
    EnumPathEx<recursive_directory_iterator>(folder, ext, files);
  }
  else {
    EnumPathEx<directory_iterator>(folder, ext, files);
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

uint32_t timer::get_ticks()
{
  if (!_started) return 0;

  if (_paused) {
    return _paused_ticks;
  }
  else {
    return SDL_GetTicks() - _started_ticks;
  }
}