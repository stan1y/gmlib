#include "GMLib.h"

config::config()
{
    //SDL flags
    display_width = 1024;
    display_height = 768;
    driver_name = NULL;
    assets_path = NULL;
    fullscreen = false;
    fps_cap = 30;

    driver_index = -1;
    window_flags = SDL_WINDOW_SHOWN;
    renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
}

int config::load_file(const char* cfg_file)
 {
    SDL_Log("Reading configuration from %s", cfg_file);
    json_t* conf_data = GM_LoadJSON(cfg_file);
    if (conf_data == nullptr) {
        return -1;
    }

    json_error_t jerr;
    if (json_unpack_ex(conf_data, &jerr, 0,
        "{s:i, s:i, s:b, s:s, s:s }",
        "display_width", &display_width,
        "display_height", &display_height,
        "fullscreen", &fullscreen,
        "driver_name", &driver_name,
        "assets_path", &assets_path) != 0)
    {
        SDLEx_LogError("Failed to unpack configuration, error at line: %d, column: %d, position: %d. %s",
            jerr.line, jerr.column, jerr.position, jerr.text);
        return -1;
    }

    //select driver
    if (driver_name) {
        int drivers = SDL_GetNumRenderDrivers();
        for(int i = 0; i < drivers; i++) {
            SDL_RendererInfo renderer_info;
            SDL_GetRenderDriverInfo(i, &renderer_info);
            if (SDL_strncmp(driver_name, renderer_info.name, max_driver_name) == 0) {
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