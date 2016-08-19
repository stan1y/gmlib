#include "GMLib.h"
#include "GMData.h"

struct config_private : public data {
  static const int max_driver_name = 32;

  /** Engine Settings **/
  int32_t display_width;
  int32_t display_height;
  uint32_t fps_cap;
  std::string driver_name;
  std::string assets_path;
  bool fullscreen;
  bool calculate_fps;

  /** 
  Other components can use data access protocol 
  **/

  // SDL settings
  int32_t driver_index;
  uint32_t window_flags;
  uint32_t renderer_flags;

  config_private();
  virtual ~config_private();

  // public interface pointer
  config * _interface;
};
static config_private * g_config = NULL;

const config* GM_GetConfig() {
  if (g_config == nullptr || g_config->_interface == nullptr) {
    SDLEx_LogError("GM_GetConfig: not initialized");
    return nullptr;
  }
  return g_config->_interface;
}

/** Public Interface **/

const data & GM_GetConfigData()
{
  return (*g_config);
}


const rect config::screen_rect() const
{
  rect r = rect(0, 0, g_config->display_width, g_config->display_height);
  return r;
}

const std::string config::driver_name() const
{
  return std::string(g_config->driver_name);
}

const int32_t config::driver_index() const
{
  return g_config->driver_index;
}

const std::string config::assets_path() const
{
  return std::string(g_config->assets_path);
}

const bool config::fullscreen() const
{
  return g_config->fullscreen;
}

const bool config::calculate_fps() const
{
  return g_config->calculate_fps;
}

const int config::fps_cap() const
{
  return g_config->fps_cap;
}

const uint32_t config::window_flags() const
{
  return g_config->window_flags;
}

const uint32_t config::renderer_flags() const
{
  return g_config->renderer_flags;
}

/** Private Interface **/

config_private::config_private()
{
    // Default SDL flags
    display_width = 1024;
    display_height = 768;
    driver_name = "";
    assets_path = "";
    fullscreen = false;
    calculate_fps = true;
    fps_cap = 60;

    driver_index = -1;
    window_flags = SDL_WINDOW_SHOWN;
    renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC;

    _interface = new config();
}

config_private::~config_private()
{
  delete _interface;
}

int config::load(const std::string& cfg_file)
 {
   if (g_config != nullptr && g_config->_interface != nullptr) 
     return 0;

    SDL_Log("config:load - loading %s", cfg_file.c_str());
    g_config = new config_private();
    g_config->load(cfg_file);
    
    char* assets = NULL;
    char* driver = NULL;
    g_config->unpack("{s:i s:i s:b s:b s:s s:s}",
        "display_width",    &g_config->display_width,
        "display_height",   &g_config->display_height,
        "fullscreen",       &g_config->fullscreen,
        "calculate_fps",    &g_config->calculate_fps,
        "driver",           &driver,
        "assets_path",      &assets);
        //"default_ui",       &g_config->theme,
        //"ui_flags",         &g_config->ui_flags);
    
    //copy strings
    g_config->driver_name = std::string(driver);
    g_config->assets_path = std::string(assets);

    //select driver
    if (driver) {
        int drivers = SDL_GetNumRenderDrivers();
        for(int i = 0; i < drivers; i++) {
            SDL_RendererInfo renderer_info;
            SDL_GetRenderDriverInfo(i, &renderer_info);
            if (SDL_strncmp(driver, renderer_info.name, config_private::max_driver_name) == 0) {
                g_config->driver_index = i;
                break;
            }
        }
    }

    //set window flags
    g_config->window_flags = 0;
    if (g_config->fullscreen) {
        g_config->window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else {
        g_config->window_flags |= SDL_WINDOWPOS_CENTERED;
    }

    return 0;
}