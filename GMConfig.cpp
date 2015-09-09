#include "GMLib.h"
#include "GMData.h"

config::config()
{
    //SDL flags
    display_width = 1024;
    display_height = 768;
    driver_name = NULL;
    assets_path = NULL;
    default_ui = NULL;
    ui_flags = 0;
    fullscreen = false;
    calculate_fps = true;
    fps_cap = 60;

    driver_index = -1;
    window_flags = SDL_WINDOW_SHOWN;
    renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC;
}

config::~config()
{
  //if (driver_name) free(driver_name);
  //if (assets_path) free(assets_path);
  //if (default_ui) free(default_ui);
}

int config::load(const std::string& cfg_file)
 {
    SDL_Log("config: loading settings...");
    data cfg(cfg_file);
    
    char* assets = NULL;
    char* driver = NULL;
    char* theme = NULL;
    cfg.unpack("{s:i s:i s:b s:b s:s s:s s:s s:i}",
        "display_width", &display_width,
        "display_height", &display_height,
        "fullscreen", &fullscreen,
        "calculate_fps", &calculate_fps,
        "driver", &driver,
        "assets_path", &assets,
        "default_ui", &theme,
        "ui_flags", &ui_flags);
    
    //copy strings
    driver_name = copy_string(driver);
    assets_path = copy_string(assets);
    default_ui = copy_string(theme);

    //select driver
    if (driver) {
        int drivers = SDL_GetNumRenderDrivers();
        for(int i = 0; i < drivers; i++) {
            SDL_RendererInfo renderer_info;
            SDL_GetRenderDriverInfo(i, &renderer_info);
            if (SDL_strncmp(driver, renderer_info.name, max_driver_name) == 0) {
                driver_index = i;
                break;
            }
        }
    }

    //set window flags
    window_flags = 0;
    if (fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else {
        window_flags |= SDL_WINDOWPOS_CENTERED;
    }

    return 0;
}