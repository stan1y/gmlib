#include "GMLib.h"
#include "RESLib.h"

/* Global State */
static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static config* g_config = nullptr;
static timer* g_frame_timer = nullptr;
static timer* g_fps_timer = nullptr;
static int g_counted_frames = 0;
static int g_screen_ticks_per_frame = 0;
static float g_avg_fps = 0.0f;

/* Screens */
static screen* g_screen_current;
static screen* g_screen_next;

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
        SDLEx_LogError("Failed to initialize SDL");
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == -1) {
        SDLEx_LogError("Failed to initialize SDL_img");
        return -1;
    }
    if (TTF_Init() == -1) {
        SDLEx_LogError("Failed to initialize SDL_ttf");
        return -1;;
    }
    SDL_version c_ver;
    SDL_VERSION(&c_ver);
    SDL_version l_ver;
    SDL_GetVersion(&l_ver);
    SDL_Log("Initalized SDL %d.%d.%d. Complied with %d.%d.%d",
        l_ver.major, l_ver.minor, l_ver.patch,
        c_ver.major, c_ver.minor, c_ver.patch);

    //setup game state
    g_config = conf;
    g_frame_timer = new timer();
    g_screen_ticks_per_frame = 1000 / conf->fps_cap;
    g_fps_timer = new timer();
    
    //setup screen
    g_screen_current = nullptr;
    g_screen_next = nullptr;

    //init SDL window & renderer
    g_window = SDL_CreateWindow(name, 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        conf->display_width, conf->display_height, conf->window_flags);
    if ( g_window == nullptr ) {
        SDLEx_LogError("Failed to system window");
        return -1;
    }
    g_renderer = SDL_CreateRenderer(g_window, conf->driver_index, conf->renderer_flags);
    if ( g_renderer == nullptr ) {
        SDLEx_LogError("Failed to create renderer driver_index=%d", conf->driver_index);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    //log renderer info
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(g_renderer, &renderer_info);
    SDL_Log("Screen ready size=%dx%d, fullscreen=%d, driver=%s", conf->display_width, conf->display_height, conf->fullscreen, renderer_info.name);
    
    //resources cache
    RES_Init(conf->assets_path);
    
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
    
  // update current screen
  if (g_screen_current != nullptr) {
    g_screen_current->update();  
  }
}

void GM_RenderFrame()
{
  // update current screen
  if (g_screen_current == NULL) {
    SDLEx_LogError("Failed to render: no active screen");
    throw std::exception("No active screen to update");
  }
  g_screen_current->render();
  //re-start frame timer
  g_frame_timer->start();
}

void GM_EndFrame()
{
  //present rendered screen
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

TTF_Font* GM_LoadFont(const char* font, int ptsize)
{
    TTF_Font* f = TTF_OpenFont(font, ptsize);

    if (!f) {
        SDLEx_LogError("LoadFont: failed to load %s", font);
        return nullptr;
    }
    return f;
}
    

json_t* GM_LoadJSON(const char* file)
{
    json_t* json_data = nullptr;
    json_error_t jerr;

    json_data = json_load_file(file, 0, &jerr);
    if (json_data == nullptr) {
        SDLEx_LogError("LoadJson: failed to load %s. %s", file, jerr.text);
        return NULL;
    }

    return json_data;
}

/* File and folder helpers */

#ifdef _WIN32
#define PATHSEP '\\'
#endif

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

  return cwd;
}

std::string GM_GetPathOfFile(const std::string& filepath)
{
  size_t pos = filepath.find_last_of(PATHSEP);
  return filepath.substr(0, pos);
}

std::string GM_JoinPaths(const std::string& root, const std::string& relative)
{
  return root + PATHSEP + relative;
}

void GM_EnumPath(const std::string& folder, std::vector<std::string>& files, int d_type)
{
  DIR *dir;
  struct dirent *ent;
                
  /* Open directory stream */
  dir = opendir (folder.c_str());
  if (dir != NULL) {
    /* Print all files and directories within the directory */
    while ((ent = readdir (dir)) != NULL) {
      if (ent->d_type == d_type) {
        files.push_back(ent->d_name);
      }
    }
    closedir (dir);
  }
  else {
    /* Could not open directory */
    SDLEx_LogError("GM_EnumPath: failed to open directory %s", folder.c_str());
    throw std::exception("Failed to open directory");
  }
}

void GM_EnumPathFilesEx(const std::string& folder, const std::string& ext, std::vector<std::string>& files)
{
  GM_EnumPath(folder, files, DT_REG);
  std::vector<std::string>::iterator it = files.begin();
  for(; it != files.end(); ++it) {
    size_t pos = it->find_last_of('.');
    if (pos == std::string::npos) {
      it = files.erase(it);
      continue;
    }
    std::string file_ext = it->substr(pos, it->length() - pos);
    if (file_ext != ext) {
      it = files.erase(it);
      continue;
    }
  }
}

void GM_EnumPathFiles(const std::string& folder, std::vector<std::string>& files)
{
  GM_EnumPath(folder, files, DT_REG);
}

void GM_EnumPathFolders(const std::string& folder, std::vector<std::string>& files)
{
  GM_EnumPath(folder, files, DT_DIR);
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