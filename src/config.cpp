#ifndef GMLIB_CONFIG
#define GMLIB_CONFIG

#include "engine.h"

static config * g_config;

void config::load(const std::string &cfg_file) {
  g_config = new config(cfg_file);
}

config::config(const std::string cfgpath) {
  std::ifstream(cfgpath) >> _data;
}

const config & config::current() {
  return (*g_config);
}

int config::get_driver_index() const {
  //select driver
  if (_data.find("driver") == _data.end())
    return 0;

  std::string driver = _data["driver"];
  int drivers = SDL_GetNumRenderDrivers();
  for(int i = 0; i < drivers; i++) {
      SDL_RendererInfo renderer_info;
      SDL_GetRenderDriverInfo(i, &renderer_info);
      if (driver == std::string(renderer_info.name))
          return i;
  }
  return -1;
}

int config::get_window_flags() const {
  int flags = SDL_WINDOW_SHOWN;
  if (_data.find("fullscreen") != _data.end() && _data["fullscreen"].get<bool>()) {
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
  return flags;
}

#endif //GMLIB_CONFIG
