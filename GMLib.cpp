#include "GMLib.h"
#include "RESLib.h"

bool operator== (SDL_Rect& a, SDL_Rect& b) {
    return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

bool operator!= (SDL_Rect& a, SDL_Rect& b) {
    return (a.x != b.x || a.y != b.y || a.w != b.w || a.h != b.h);
}

/* Global State */
static SDL_Window* _window = nullptr;
static SDL_Renderer* _renderer = nullptr;
static globals* _globals = nullptr;
static screen* _screen_Current;
static screen* _screen_Next;

SDL_Window* GM_GetWindow() {
    if (_window == nullptr) {
        SDLEx_LogError("GM_GetWindow: not initialized");
        return nullptr;
    }
    return _window;
}

SDL_Renderer* GM_GetRenderer() {
    if (_renderer == nullptr) {
        SDLEx_LogError("GM_GetRenderer: not initialized");
        return nullptr;
    }
    return _renderer;
}

globals* GM_GetGlobals() {
     if (_globals == nullptr) {
         SDLEx_LogError("GM_GetGlobals: not initialized");
         return nullptr;
     }
     return _globals;
}

int GM_Init(config* conf) {

    //check if we're loaded up already
    if ( _globals != nullptr || _window != nullptr || _renderer != nullptr ) {
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
    _globals = (globals*)SDL_malloc(sizeof(globals));
    SDL_memset(_globals, 0, sizeof(globals));
    _globals->conf = conf;

    //setup screens
    _screen_Current = nullptr;
    _screen_Next = nullptr;

    //init SDL window & renderer
    _window = SDL_CreateWindow("WinComplex", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        conf->display_width, conf->display_height, conf->window_flags);
    if ( _window == nullptr ) {
        SDLEx_LogError("Failed to system window");
        return -1;
    }
    _renderer = SDL_CreateRenderer(_window, conf->driver_index, conf->renderer_flags);
    if ( _renderer == nullptr ) {
        SDLEx_LogError("Failed to create renderer driver_index=%d", conf->driver_index);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

    //log renderer info
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(_renderer, &renderer_info);
    SDL_Log("Screen ready size=%dx%d, fullscreen=%d, driver=%s", conf->display_width, conf->display_height, conf->fullscreen, renderer_info.name);
    
    //resources cache
    RES_Init(conf->assets_path);
    
    return 0;
}

void GM_Quit() {
    SDL_Quit();
}

void GM_StartFrame()
{
    //update frame ticks
    _globals->frame_ticks = SDL_GetTicks();
    if (_globals->last_frame_ticks == 0 || _globals->frame_ticks > _globals->last_frame_ticks) {
        _globals->elapsed = _globals->frame_ticks - _globals->last_frame_ticks;
    }

    //clear screen with black
    SDL_SetRenderTarget(_renderer, NULL);
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
    SDL_RenderClear(_renderer);
}

void GM_UpdateFrame()
{
  //update game state
    if (_screen_Current != _screen_Current) {
      
      _screen_Current = _screen_Next;
      SDL_Log("Screen %p is now active", _screen_Current);
    }

    // update running animations
    anim::update_running();
    
    // update current screen
    if (_screen_Current != nullptr) {
      _screen_Current->update();  
    }
}

void GM_RenderFrame()
{
  // update current screen
  if (_screen_Current == NULL) {
    SDLEx_LogError("Failed to render: no active screen");
    throw std::exception("No active screen to update");
  }
  _screen_Current->render();
}

void GM_EndFrame()
{
    SDL_RenderPresent(_renderer);

    //update last frame ticks
    _globals->last_frame_ticks = _globals->frame_ticks;
    // Limit FPS
	if (_globals->elapsed < 1000 / _globals->conf->fps_cap) {
		SDL_Delay(( 1000 / _globals->conf->fps_cap ) - _globals->elapsed);
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
  return _screen_Current;
}

void screen::set_current(screen* s)
{
  if (_screen_Current != s && _screen_Next != s) {
        _screen_Current = s;
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