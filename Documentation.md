
# GMLib Overview
This document is a brief overview of the GMLib classes and functions. The document encourage you to read both haeders and sources of your interest to see details of implementation for yourself.

## Components
The overview list of GMLib features:
* Components (of a screen) architecture and frame rendering (`GMLib.h`).
* Generic resources caches with dynamic and static update (`RESLib.h`)..
* Sprites, sprite sheets and animations (`GMSprites.h`)..
* Textures, drawing and pixel access (`GMTexture.h`)..
* Comprehensive GUI manager and controls library (`GMUI.h`, `GMUIControl.h`, +control specific headers).
* Native JSON as internal and external structured data format (`GMData.h`).
* Embedded python runtime and API (`GMPython.h`).

## Configuration
The GMLib must be initialized before it can be used by an app. In order to do so, applications need to provide a path to a configuration file and a name to the `GM_Init` function. Later the configuration of the GMLib is accessible to application via `config::current()` method. Configuration of the GMLib contains the following information used to initialize SDL and OpenGL for
library functionality.

    {
        "driver": <string>,
        "fps_cap": <int>,
        "display_width": <int>,
        "display_height": <int>,
        "fullscreen": <bool>,
        "assets": [
          <string>,
          ...
        ],
        "ui_theme": <string>,
        "python_home": <string>
    }

### Options
* __driver__ - name of the driver to use: "opengl", "directx", "software"
* __fps_cap__ - max number of frames per second
* __display_width__ - width for the main screen
* __display_height__ - height for the main screen
* __fullscreen__ - enable full screen
* __assets__ - a list of strings with paths to resource roots, folders or archive files.
* __ui_theme__ - a name of the UI theme to use
* __python_home__ - a path to the main python runtime libraries.

## Screens
GMLibs main goal is to manage frame rendering for an application. To achieve that goal GMLib is running frame loop with given speed and let application render frame contents. The application itself is represented to GMLib as one or several `screen` instances. The `screen` is an interface which should be implemented by an app in order to render frames in GMLib frame loop and own a frame at any given moment. Each screen represents a state of an app, such as game, as start menu, the game map, overview, scores screens and etc.

    class myscreen: public screen {
        virtual void update(SDL_Event* ev);
        virtual void on_event(SDL_Event* ev);
        virtual void render(SDL_Renderer * r);
    }
_The `screen` interface to be implemented by an application._

### Screen Components
The screen can and should have several (_reusable_) `screen::component` implementations, shared across different `screen` implementations. This design encourage reuse and abstraction of a application logic to provide different perspectives on the same data (_and internal state, objects, etc_). A component interface allows implementation of this classes to provide utilities api to a `screen` and call back into `screen`.

However `screen::component` interfaces are completely optional, since the interface of `screen` lets implementation do anything to generate a frame with provided instance of `SDL_Renderer` ready to draw to real screen.The components should be added to a screen in the screen constructor with `screen::add_component` method. Later during state update or frame rendering, the application can get a shared component instance with `get_component()` template function and specify type of the required component.


## Resources
GMLib provider a shared cache of resources available to applications. The resource sortable in the cache is an implementation of `iresource` interface. The cache is based on [PhysicsFS](https://icculus.org/physfs/). Several classes in GMLib implements this interface and thus can be loaded from the cache. The following classes are supported by `RESLib.h`:
* `ttf_font` (_GMLib.h_)
* `data` (_GMData.h_)
* `python::script` (_GMPython.h_)
* `texture` (_GMTexture.h_)

The resource cache interface provide functions in `resources` namespace from `RESLib.h` header. Each supported class type has a corresponding function to use, as well as generic `iresource` variant.

    /* 
     * Get item from resources cache by identifier.
     *  Returns nullptr if not found
     */
    iresource const * get(const resource_id & resource);
    
    /* Get texture from resources by identifier */
    texture const * get_texture(const std::string & filename);
    
    /* Get texture as sprites_sheet from resources by identifier */
    sprites_sheet const * get_sprites_sheet(const std::string& filename, 
                                            size_t sprite_width,
                                            size_t sprite_height);
    
    /* Get data from resources by identifier */
    data const * get_data(const std::string & filename);
    
    /* Get font from resources by identifier */
    ttf_font const * get_font(const std::string & filename,
                              size_t pt_size);
    
    /* Get script from resources by identifier */
    python::script const * get_script(const std::string& filename);





































