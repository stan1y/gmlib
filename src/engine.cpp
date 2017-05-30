#include <boost/filesystem.hpp>

#include "engine.h"
#include "util.h"
#include "texture.h"
#include "sprite.h"
#include "manager.h"
#include "pyscript.h"

/* Global State */
static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static timer* g_frame_timer = nullptr;
static timer* g_fps_timer = nullptr;
static uint32_t g_counted_frames = 0;
static uint32_t g_screen_ticks_per_frame = 0;

static float g_avg_fps = 0.0f;
static texture g_fps;
static color g_fps_color;

/* Screens */
static sdl_mutex g_screen_lock;
static screen * g_screen_current = nullptr;
static screen * g_screen_next = nullptr;
static bool g_destroy_on_change = false;
static bool g_quit = false;

SDL_Window* GM_GetWindow() {
    if (g_window == nullptr) {
        SDL_Log("%s: not initialized", __METHOD_NAME__);
        return nullptr;
    }
    return g_window;
}

SDL_Renderer* GM_GetRenderer() {
  if (g_renderer == nullptr) {
    SDL_Log("%s: not initialized", __METHOD_NAME__);
    return nullptr;
  }
  return g_renderer;
}

int GM_Init(const std::string & cfg_path, const std::string & name) {

    //check if we're loaded up already
    if (g_window != nullptr || g_renderer != nullptr ) {
        return 0;
    }

    //init RND
    srand((int)time(NULL));

    //init subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        SDL_Log("SDL_Init: Failed to initialize SDL. SDL Error: %s", SDL_GetError());
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == -1) {
        SDL_Log("IMG_Init: Failed to initialize SDL_img. SDL Error: %s", SDL_GetError());
        return -1;
    }
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init: Failed to initialize SDL_ttf. SDL Error: %s", SDL_GetError());
        return -1;;
    }

    // get SDL versions
    SDL_version c_ver;
    SDL_VERSION(&c_ver);
    SDL_version l_ver;
    SDL_GetVersion(&l_ver);

    SDL_Log("loading - version %d.%d.%s.%d, sdl runtime: %d.%d.%d, sdl compilied: %d.%d.%d",
            GM_LIB_MAJOR, GM_LIB_MINOR, GM_LIB_RELESE, GM_LIB_PATCH,
            l_ver.major, l_ver.minor, l_ver.patch,
            c_ver.major, c_ver.minor, c_ver.patch);

    //check cfg path is ok
    if (cfg_path.empty()) {
      SDL_Log("%s: invalid config path", __METHOD_NAME__);
      return -1;
    }
    boost::filesystem::path p_path(cfg_path);
    if (!boost::filesystem::exists(p_path)) {
      SDL_Log("%s: config path does not exist", __METHOD_NAME__);
      return -1;
    }
    boost::filesystem::path abspath = boost::filesystem::absolute(p_path);
    config::load(abspath.string());
    const json & cfg = config::current().get_data();
    
    // init SDL window & renderer
    int flags = SDL_WINDOW_SHOWN;
    if (cfg["fullscreen"].get<bool>())
      flags |= SDL_WINDOW_FULLSCREEN;
    if (cfg["driver"].get<std::string>() == std::string("opengl"))
      flags |= SDL_WINDOW_OPENGL;

    g_window = SDL_CreateWindow(name.c_str(), 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        cfg["display_width"],
        cfg["display_height"],
        flags);
    if ( g_window == nullptr ) {
        SDL_Log("%s: Failed to system window. SDL Error: %s",
                __METHOD_NAME__, SDL_GetError());
        return -1;
    }
    // setup renderer
    int driver = config::current().get_driver_index();
    g_renderer = SDL_CreateRenderer(g_window, driver, config::current().get_window_flags());
    if ( g_renderer == nullptr ) {
        SDL_Log("%s: Failed to create renderer driver_index=%d", __METHOD_NAME__, driver);
        return -1;
    }
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    // log renderer info
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(g_renderer, &renderer_info);

    // setup random
    srand((unsigned int)time(NULL));

    // fps timer
    g_frame_timer = new timer();
    g_screen_ticks_per_frame = 1000 / cfg["fps_cap"].get<int>();
    
    // init UI
    rect display = GM_GetDisplayRect();
    ui::manager::initialize(display, cfg["ui_theme"]);
    
    // setup screens
    g_screen_current = nullptr;
    g_screen_next = nullptr;

    // fps counter    
    if (cfg.find("fps_counter") != cfg.end() &&
        cfg["fps_counter"].get<bool>()) {
      g_fps_timer = new timer();
      g_fps_color = color( 0, 255, 0, 255 );
    }

    return 0;
}

uint32_t GM_GetFrameTicks()
{
  return g_frame_timer->get_ticks();
}

float GM_CurrentFPS()
{
  return g_avg_fps;
}

void GM_Quit() 
{
  python::shutdown();
  SDL_Quit();
}

void GM_PostQuit()
{
  SDL_Event ev;
  ev.type = SDL_QUIT;
  SDL_PushEvent(&ev);
}

void GM_StartFrame()
{
  mutex_lock guard(g_screen_lock);

  if (g_fps_timer != nullptr) {

    // init fps timer first on first frame
    if(!g_fps_timer->is_started())
      g_fps_timer->start();

    g_avg_fps = g_counted_frames / ( g_fps_timer->get_ticks() / 1000.f );
    if (g_avg_fps > 2000000) {
      g_avg_fps = 0;
    }  
  }

  // clear screen with black
  SDL_SetRenderTarget(g_renderer, NULL);
  SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
  SDL_RenderClear(g_renderer);
}

void GM_UpdateFrame()
{
  mutex_lock guard(g_screen_lock);

  // update game state
  if (g_screen_current != g_screen_next) {
    if (g_screen_current != nullptr && g_destroy_on_change)
      delete g_screen_current;
    g_screen_current = g_screen_next;
  }
    
  // update global & current screens
  if (g_screen_current != nullptr) {
    g_screen_current->update();  
  }

  // process events
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_QUIT) {
      g_quit = true;
      break;
    }

    g_screen_current->on_event(&ev);
  }
}

void GM_RenderFrame()
{
  mutex_lock guard(g_screen_lock);

  // update current screen
  if (g_screen_current == NULL) {
    SDL_Log("%s - failed to render: no active screen", 
      __METHOD_NAME__);
    throw std::runtime_error("No active screen to render");
  }
  SDL_Renderer * r = GM_GetRenderer();

  // reset renderer
  SDL_SetRenderTarget(r, NULL);
  color::black().apply(r);
  SDL_RenderClear(r);

  g_screen_current->render(r);
  
  // render avg fps
  if (g_fps_timer) {
    SDL_Surface *s = ui::manager::instance()->get_font("fps_counter")->print_solid(
      std::string("fps: ") + std::to_string(float_to_sint32(GM_CurrentFPS())),
      ui::manager::instance()->get_idle_color("fps_counter"));
    g_fps.set_surface(s);
    SDL_FreeSurface(s);
    g_fps.render(GM_GetRenderer(), point(5, 5));
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
  if (g_screen_ticks_per_frame > 0) {
    uint32_t frame_ticks = GM_GetFrameTicks();
    if (frame_ticks < g_screen_ticks_per_frame) {
      SDL_Delay(g_screen_ticks_per_frame - frame_ticks);
    }
  }
}

void GM_Loop()
{
  while (!SDL_QuitRequested() && !g_quit) {
    GM_StartFrame();
    GM_UpdateFrame();
    GM_RenderFrame();
    GM_EndFrame();
  }
}

/* 
    Game Screens 
*/

screen::screen():
  _wnd(GM_GetWindow())
{
}

screen::screen(SDL_Window* wnd):
  _wnd(wnd)
{
}

screen::~screen()
{
  lock_container(_components);
  container<screen::component*>::iterator it = _components.begin();
  for(; it != _components.end(); ++it) {
    if (*it == ui::manager::instance())
      continue;
    delete *it;
  }
}

void screen::update()
{
  lock_container(_components);
  // update UI first
  ui::manager::instance()->on_update(this);
  container<screen::component*>::iterator it = _components.begin();
  for(; it != _components.end(); ++it) {
    (*it)->on_update(this);
  }
}

/* Screen Render */
void screen::render(SDL_Renderer * r)
{
  lock_container(_components);
  // render components first
  container<screen::component*>::iterator it = _components.begin();
  for(; it != _components.end(); ++it) {
    (*it)->render(r);
  }
  // render UI on top of others
  ui::manager::instance()->render(r);
}

/* Screen On Event Callback */
void screen::on_event(SDL_Event* ev)
{
  lock_container(_components);
  // handle event by UI first
  ui::manager::instance()->on_event(ev);
  container<screen::component*>::iterator it = _components.begin();
  for(; it != _components.end(); ++it) {
    (*it)->on_event(ev);
  }
}

void screen::add_component(screen::component* c) {
  _components.push_back(c);
}

const screen* screen::current() 
{
  return g_screen_current;
}

void screen::set_current(screen* s, bool destroy_on_change)
{
  mutex_lock guard(g_screen_lock);
  if (g_screen_current != s && g_screen_next != s) {
    // requested screen becomes next one
    g_screen_next = s;
    g_destroy_on_change = destroy_on_change;
  }
}

/* Load helpers */

SDL_Surface* GM_LoadSurface(const std::string& file_path)
{
  SDL_Surface *tmp = IMG_Load(file_path.c_str());
  if (!tmp) {
    SDL_Log("%s: failed to load %s",
      __METHOD_NAME__,
      file_path.c_str());
    throw sdl_exception();
  }

  SDL_Surface* s = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGBA8888, 0);
  if (!s) {
    SDL_Log("%s: failed to convert surface.", __METHOD_NAME__);
    throw sdl_exception();
  }
  SDL_FreeSurface(tmp);

  return s;
}

SDL_Texture* GM_LoadTexture(const std::string& file_path)
{
  SDL_Surface* s = GM_LoadSurface(file_path);
  SDL_Texture *tex = SDL_CreateTextureFromSurface(GM_GetRenderer(), s);
  if (tex == NULL) {
    SDL_Log("%s: failed to create texture from surface of %s",
      __METHOD_NAME__,
      file_path.c_str());
    throw sdl_exception();
  }
  SDL_FreeSurface(s);
  return tex;
}

TTF_Font* GM_LoadFont(const std::string& file_path, int ptsize)
{
  TTF_Font* f = TTF_OpenFont(file_path.c_str(), ptsize);
  if (!f) {
    SDL_Log("%s: failed to load %s",
      __METHOD_NAME__,
      file_path.c_str());
    throw sdl_exception();
  }
  return f;
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

uint32_t timer::get_ticks() const
{
  if (!_started) return 0;

  if (_paused) {
    return _paused_ticks;
  }
  else {
    return SDL_GetTicks() - _started_ticks;
  }
}

/* Core classes */

/* test collision of point and circle */
bool point::collide_circle(const point & center, const int radius) 
{
  int square_dist = double_to_sint32(std::pow(center.x - x, 2)) + 
    double_to_sint32(std::pow(center.y - y, 2));
  return (square_dist <= double_to_sint32(std::pow(radius, 2)));
}

bool operator== (const color& a, const color& b) {
  return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}

bool operator!= (const color& a, const color& b) {
  return !(a == b);
}

bool operator== (const rect& a, const rect& b) {
  return (SDL_RectEquals(&a, &b) == SDL_TRUE);
}

bool operator!= (const rect& a, const rect& b) {
    return (SDL_RectEquals(&a, &b) != SDL_TRUE);
}

rect operator+ (const rect& a, const rect& b) {
  return rect(a.x + b.x, a.y + b.y, a.w + b.w, a.h + b.h);
}

rect operator- (const rect& a, const rect& b) {
  return rect(a.x - b.x, a.y - b.y, a.w - b.w, a.h - b.h);
}

rect operator+= (rect& a, const rect& b) {
  a.x += b.x; a.y += b.y; a.w += b.w; a.h += b.h;
  return a;
}

rect operator-= (rect& a, const rect& b) {
  a.x -= b.x; a.y -= b.y; a.w -= b.w; a.h -= b.h;
  return a;
}

rect operator+ (const rect& r, const point& p) {
  return rect(r.x + p.x, r.y + p.y, r.w, r.h);
}

rect operator- (const rect& r, const point& p) {
  return rect(r.x - p.x, r.y - p.y, r.w, r.h);
}

point operator+ (const point& a, const point& b) {
  return point(a.x + b.x, a.y + b.y);
}

point operator- (const point& a, const point& b) {
  return point(a.x - b.x, a.y - b.y);
}

bool operator== (const point& a, const point& b) {
    return (a.x == b.x && a.y == b.y);
}

bool operator!= (const point& a, const point& b) {
    return (a.x != b.x || a.y != b.y);
}

void operator+= (point& a, const point& b) {
  a.x += b.x; a.y += b.y;
}

void operator-= (point& a, const point& b) {
  a.x -= b.x; a.y -= b.y;
}

void operator+= (rect& r, const point& p) {
  r.x += p.x; r.y += p.y;
}

void operator-= (rect& r, const point& p) {
  r.x -= p.x; r.y -= p.y;
}

void color::apply(SDL_Renderer* rnd) const
{
  SDL_SetRenderDrawColor(rnd, r, g, b, a);
}

void color::apply() const
{
  apply(GM_GetRenderer());
}

color color::from_json(const json & d)
{
  if (d.is_string()) {
    return color::from_string(d);
  }
  if (d.is_array()) {
    return color(
        d.at(0),
        d.at(1),
        d.at(2),
        d.at(3));
  }
  throw std::runtime_error("json is not a color.");
}

color color::from_string(const std::string & sclr)
{
  if (sclr == std::string("red")) {
    return color::red();
  }
  if (sclr == std::string("green")) {
    return color::green();
  }
  if (sclr == std::string("blue")) {
    return color::blue();
  }
  if (sclr == std::string("white")) {
    return color::white();
  }
  if (sclr == std::string("black")) {
    return color::black();
  }
  if (sclr == std::string("dark_red")) {
    return color::dark_red();
  }
  if (sclr == std::string("dark_blue")) {
    return color::dark_blue();
  }
  if (sclr == std::string("dark_gray")) {
    return color::dark_gray();
  }
  if (sclr == std::string("gray")) {
    return color::gray();
  }
  if (sclr == std::string("light_gray")) {
    return color::light_gray();
  }

  SDL_Log("color::from_string - unknown name %s", sclr.c_str());
  throw std::runtime_error("unknown color name");
}

rect rect::scale(const float & fw, const float & fh) const
{
  rect r(*this);
  int dx = double_to_sint32(((w * fw) - w) / 2.0);
  int dy = double_to_sint32(((h * fh) - h) / 2.0);
  r.x += dx;
  r.y += dy;
  r.w -= 2 * dx;
  r.h -= 2 * dy;
  return r;
}

rect rect::center(const uint32_t for_w, const uint32_t for_h) const
{
  return rect(x + (w - for_w) / 2, y + (h - for_h) / 2, for_w, for_h);
}

SDL_Surface * ttf_font::print_solid(const std::string & text,
                          const color & clr) const
{
  return TTF_RenderText_Solid(_f, text.c_str(), clr);
}

SDL_Surface * ttf_font::print_blended(const std::string & text,
                            const color & clr) const
{
  return TTF_RenderText_Blended(_f, text.c_str(), clr);
}

rect ttf_font::get_text_rect(const std::string& text) const
{
  rect r(0, 0, 0, 0);
  TTF_SizeText(_f, text.c_str(), &r.w, &r.h);
  return r;
}

void ttf_font::load(const std::string & file_path, size_t pts)
{
  if (_f != nullptr)
    TTF_CloseFont(_f);
  _pts = pts;
  _f = GM_LoadFont(media_path(file_path), _pts);
  _fname = file_path;
  SDL_Log("ttf_font - loaded %s, pt size: %zu",
          _fname.c_str(), _pts);
}
