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

/* Texture */

texture::texture()
{
  _texture = NULL;
  _pixels = NULL;
  _pitch = 0;
  _width = 0;
  _height = 0;
}

void texture::release()
{
  if (_texture) 
    SDL_DestroyTexture(_texture);
  _texture = NULL;
}

texture::~texture()
{
  release();
}

void texture::load(const std::string& path)
{
  SDL_Surface* src = GM_LoadSurface(path);
  load_surface(src);
  SDL_FreeSurface(src);
}

void texture::load_text(const std::string& text, TTF_Font* font, SDL_Color& color)
{
  SDL_Surface* s = TTF_RenderText_Blended(font, text.c_str(), color);
  if (!s) {
    SDLEx_LogError("texture::load_text Failed to render text: %s", TTF_GetError());
    throw std::exception("Failed to render text");
  }
  release();
  _texture = SDL_CreateTextureFromSurface(GM_GetRenderer(), s);
  SDL_FreeSurface(s);
}

void texture::load_surface(SDL_Surface* src)
{
  release();
  _texture = GM_CreateTexture(src->w, src->h, SDL_TEXTUREACCESS_STREAMING);
  SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
  if (_texture == NULL) {
    SDLEx_LogError("texture::load_surface - Failed to create blank texture.");
    throw std::exception("Failed to create blank surface");
  }
  //copy pixels from surface to texture
  SDL_LockTexture(_texture, NULL, &_pixels, &_pitch);
  memcpy(_pixels, src->pixels, src->pitch * src->h);
  SDL_UnlockTexture(_texture); //<- this applys _pixles to the texture
  _pixels = NULL;

  _width = src->w;
  _height = src->h;
}

bool texture::lock()
{
  if (_pixels != NULL) {
    SDLEx_LogError("texure::lock - already locked");
    return false;
  }
  if (SDL_LockTexture(_texture, NULL, &_pixels, &_pitch) != 0) {
    SDLEx_LogError("texture::lock - failed to lock texture: %s", SDL_GetError());
    return false;
  }

  return true;
}

bool texture::unlock()
{
  if (_pixels == NULL) {
    SDLEx_LogError("texture::unlock - texture is not locked");
    return false;
  }

  SDL_UnlockTexture(_texture);
  _pixels = NULL;
  _pitch = 0;
  return true;
}

void texture::replace_color(SDL_Color& from, SDL_Color& to)
{
  if (!lock()) return;
  
  SDL_PixelFormat* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
  uint32_t f = SDL_MapRGBA(fmt, from.r, from.g, from.b, from.a);
  uint32_t t = SDL_MapRGBA(fmt, to.r, to.g, to.b, to.a);
  SDL_FreeFormat(fmt);

  uint32_t* pixels = (uint32_t*)_pixels;
  int pixel_count = ( _pitch / 4 ) * _height;
  for( int i = 0; i < pixel_count; ++i ) {
      if( pixels[i] == f ) {
          pixels[i] = t;
      }
  }
  if (!unlock()) return;
}

void texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
  SDL_Rect rect = { x, y, _width, _height };
  if( clip != NULL ) {
    rect.w = clip->w;
    rect.h = clip->h;
  }
  //Render to screen
  SDL_RenderCopyEx( GM_GetRenderer(), _texture, clip, &rect, angle, center, flip );
}

void texture::set_color(uint8_t red, uint8_t green, uint8_t blue)
{
  if (!_texture) return;
  SDL_SetTextureColorMod(_texture, red, green, blue);
}

void texture::set_blend_mode(SDL_BlendMode blending)
{
  if (!_texture) return;
  SDL_SetTextureBlendMode(_texture, blending);
}

void texture::set_alpha(uint8_t alpha)
{
  if (!_texture) return;
  SDL_SetTextureAlphaMod(_texture, alpha);
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
    

json_t* GM_LoadJSON(const std::string& file)
{
    json_t* json_data = nullptr;
    json_error_t jerr;
    std::string path = RES_GetFullPath(file);
    json_data = json_load_file(path.c_str(), 0, &jerr);
    if (json_data == nullptr) {
        SDLEx_LogError("GM_LoadJson: failed to load %s. %s", path.c_str(), jerr.text);
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