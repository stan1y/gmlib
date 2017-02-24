#include "GMLib.h"
#include "GMData.h"

struct config_private : public data {
  static const int max_driver_name = 32;

  /** Engine Settings **/
  int32_t display_width;
  int32_t display_height;
  uint32_t fps_cap;
  std::string driver_name;
  strings_list assets;
  std::string python_path;
  std::string ui_theme;
  bool fullscreen;

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

const config* config::current() {
  if (g_config == nullptr || g_config->_interface == nullptr) {
    SDL_Log("config::current: not initialized");
    return nullptr;
  }
  return g_config->_interface;
}

/** Public Interface **/

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

const strings_list & config::assets() const
{
  return g_config->assets;
}

const bool config::fullscreen() const
{
  return g_config->fullscreen;
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

const std::string config::python_path() const
{
  return g_config->python_path;
}

const std::string config::ui_theme() const
{
  return g_config->ui_theme;
}

//const config * config::current()
//{
//  if (g_config == nullptr) {
//    SDL_Log("%s - GMLib config was not loaded.");
//    throw std::runtime_error("GMLib config was not loaded.");
//  }
//  return g_config->_interface;
//}

/** Private Interface **/

config_private::config_private()
{
    // Default SDL flags
    display_width = 1024;
    display_height = 768;
    driver_name = "opengl";
    ui_theme = "default";
    fullscreen = false;
    fps_cap = 60;

    driver_index = -1;
    window_flags = 0;
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

    g_config = new config_private();
    g_config->load(cfg_file);
    
    data::json *a = NULL;
    char *driver = NULL, *python_path = NULL, *ui_theme = NULL;
    g_config->unpack("{s:i s:i s:s s?:b s?:s s?:o s?:s}",
        "display_width",    &g_config->display_width,
        "display_height",   &g_config->display_height,
        "driver",           &driver,
        "fullscreen",       &g_config->fullscreen,
        "python_path",      &python_path,
        "assets",           &a,
        "ui_theme",         &ui_theme);
    
    //copy strings
    if (driver)
      g_config->driver_name = std::string(driver);
    if (python_path)
      g_config->python_path = std::string(python_path);
    if (ui_theme)
      g_config->ui_theme = std::string(ui_theme);

    // read assets list
    data assets(a);
    data::array_iterator ait = assets.array_begin();
    for(; ait != assets.array_end(); ++ait) {
      g_config->assets.push_back( (*ait)->as<std::string>() );
    }

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
        g_config->window_flags |= SDL_WINDOW_SHOWN;
    }

    return 0;
}