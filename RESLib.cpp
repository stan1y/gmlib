#include "RESLib.h"

namespace fs = boost::filesystem;

namespace resources
{

/* item in cache */
struct resource_descriptor 
{
  iresource const * resource;
  fs::path filename;
  std::string resource_postfix;

  void init(const std::string & resource_id,
            const fs::path & p = fs::path(),
            iresource const * r = nullptr)
  {
    resource = r;
    resource_postfix = std::string("");

    if (resource_id.find(':') != std::string::npos) {
      strings_list parts;
      split_string(resource_id, ':', parts);
      filename = parts[0];
      resource_postfix = parts[1];
    }
    else {
      filename = resource_id;
    }
  }

  resource_descriptor(const std::string & resource_id,
                      const fs::path & p = fs::path(),
                      iresource const * r = nullptr)
  {
    init(resource_id, p, r);
  }


  std::string tostr()
  {
    std::stringstream ss;
    ss << "<resource " << resource << ", from: \"" << filename <<\
      "\", postfix: "<< resource_postfix << ">";
    return ss.str();
  }
};

typedef std::map<std::string, resource_descriptor> cache;

/* Static pointer to assets folder */
typedef std::map<fs::path, fs::path> assets_root;
static assets_root g_assets_root;
static cache* g_cache = nullptr;

/* Add to resources cache */
void add(const resource_id & resource, const fs::path & found_at, iresource const * r)
{
  g_cache->insert(std::make_pair(resource, resource_descriptor(resource, found_at, r)));
#ifdef GM_DEBUG
  SDL_Log("%s \"%s\"", __METHOD_NAME__, resource.c_str());
#endif
}

static char* progress_str[] = { "|", "/", "-", "\\" };
static int max_progress = 3;

static void load_assets_directory(const fs::path & directory)
{
  // cache files in this assets path
  int progress = 0;
  for(auto entry = fs::directory_iterator(directory);
    entry != fs::directory_iterator();
    ++entry) {
      const fs::path & p = *entry;
      fs::path filename = p.filename();

      if (filename.string()[0] == '.') {
        continue;
      }
      if (fs::is_directory(p)) {
        load_assets_directory(p);
      }
      if (fs::is_regular_file(p)) {
        // cache regular files
        assets_root::iterator it = g_assets_root.find(filename);
        if (it != g_assets_root.end()) {
          SDLEx_LogError("%s - found conflict with name \"%s\". Existing pointer is \"%s\", new is \"%s\"",
            __METHOD_NAME__,
            filename.string().c_str(),
            it->second.string().c_str(),
            p.string().c_str());
          throw std::exception("Resource key confict detected.");
        }
        // report progress and loop
        g_assets_root[filename] = p;
        printf("\r%s", progress_str[progress]);
        progress++;
        if (progress > max_progress)
          progress = 0;
      }
  }
}

void initialize(const strings_list & given)
{
  // init cache for loaded items
  g_cache = new cache();

  // init items location folders and resource archives
  strings_list::const_iterator ait = given.begin();
  for(; ait != given.end(); ++ait) {
    fs::path assets_path(*ait);

    if ( !assets_path.is_absolute() ) {
      assets_path = fs::path(fs::current_path() / assets_path);
    }

    if (fs::is_directory(assets_path)) {
      printf ("Loading resources @ %s\n", assets_path.string().c_str());
      load_assets_directory(assets_path);
      printf("\n");
    }

    if (fs::is_regular(assets_path)) {
      // load from archive file one day
    }
  }
  
  printf("Done: %d files collected\n", g_assets_root.size());
}

bool find_file(const fs::path & filename, fs::path & output)
{
  assets_root::iterator asset = g_assets_root.find(filename);
  if (asset != g_assets_root.end()) {
    output = asset->second;
    return true;
  }
  return false;
}

fs::path find_file(const fs::path & filename)
{
  fs::path found;
  if (!find_file(filename, found)) {
    SDLEx_LogError("%s - filename was not found in cache \"%s\"",
      __METHOD_NAME__, filename.string().c_str());
    throw std::exception("Filename was not found in cache");
  }
  return found;
}

bool find_resource(const resource_id & resource, fs::path & output)
{
  resource_descriptor d(resource);
  return find_file(d.filename, output);
}

fs::path find_resource(const resource_id & resource)
{
  fs::path found;
  if (!find_resource(resource, found)) {
    SDLEx_LogError("%s - %s was not found in cache",
      __METHOD_NAME__, 
      resource.c_str());
    throw std::exception("Resource was not found in cache");
  }
  return found;
}


/* Get files with matching extensions */
size_t find_files(const strings_list & ext_list, paths_list & output)
{
  if (!g_cache || g_assets_root.size() == 0) {
      SDLEx_LogError("%s - no assets root initialized",
        __METHOD_NAME__);
      throw std::exception("no assets root");
  }

  auto asset_root_it = g_assets_root.begin();
  for(; asset_root_it != g_assets_root.end(); ++asset_root_it) {
    std::string ext = asset_root_it->first.extension().string();
    auto found = std::find(ext_list.begin(), ext_list.end(), ext);
    if ( found != ext_list.end() ) {
      output.push_back(asset_root_it->second);
    }
  }
  return output.size();
}

/* Get item from cache by path. Returns nullptr if not found */
iresource const * get(const std::string & resource)
{
  if (!g_cache || g_assets_root.size() == 0) {
      SDLEx_LogError("%s - no assets root initialized",
        __METHOD_NAME__);
      throw std::exception("No assets root defined");
  }

  cache::iterator it = g_cache->find(resource);
  if (it == g_cache->end()) {
    // nothing found
    return nullptr;
  }
  return it->second.resource;
}

/* Get texture from cache by path. Returns nullptr if not found */
texture const * get_texture(const std::string& filename)
{
  iresource const * res = get(filename);
  if (res == nullptr) {
    fs::path found_at = find_file(filename);
    if (fs::exists(found_at)) {
      res = new texture(found_at.string());
      add(filename, found_at, res);
    }
  }

  texture const * tx = dynamic_cast<texture const *> (res);
  if (tx == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return tx;
}

/* Get texture as sprites_sheet from cache by path. Returns nullptr if not found */
sprites_sheet const * get_sprites_sheet(const std::string& filename,
                                  size_t sprite_width,
                                  size_t sprite_height)
{
  iresource const * res = get(filename);
  if (res == nullptr) {
    fs::path found_at = find_file(filename);
    if (fs::exists(found_at)) {
      res = new sprites_sheet(found_at.string(), sprite_width, sprite_height);
      add(filename, found_at, res);
    }
  }
  sprites_sheet const * ss = dynamic_cast<sprites_sheet const *> (res);
  if (ss == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return ss;
}

/* Get data from cache by filename. Returns nullptr if not found */
data const * get_data(const std::string& filename)
{
  iresource const * res = get(filename);
  if (res == nullptr) {
    fs::path found_at = find_file(filename);
    if (fs::exists(found_at)) {
      res = new data(found_at.string());
      add(filename, found_at, res);
    }
  }

  data const * d = dynamic_cast<data const *> (res);
  if (d == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return d;
}

ttf_font const * get_font(const std::string& filename, size_t pt_size)
{
  // build font id with pt size postfix
  std::stringstream font_id;
  font_id << filename << ":" << pt_size;

  iresource const * res = get(font_id.str());
  if (res == nullptr) {
    fs::path found_at = find_file(filename);
    if (fs::exists(found_at)) {
      res = new ttf_font(found_at.string(), pt_size);
      
      add(font_id.str(), found_at, res);
    }
  }

  ttf_font const * f = dynamic_cast<ttf_font const*> (res);
  if (f == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return f;
}

python::script const * get_script(const std::string& filename)
{
  iresource const * res = get(filename);
  if (res == nullptr) {
    fs::path found_at = find_file(filename);
    if (fs::exists(found_at)) {
      res = new python::script(found_at.string());
      add(filename, found_at, res);
    }
  }

  python::script const * s = dynamic_cast<python::script const*> (res);
  if (s == nullptr) {
    SDLEx_LogError("%s - failed to cast iresource",
      __METHOD_NAME__);
    throw std::exception("Failed to cast iresource");
  }
  return s;
}

bool lookup_file(const fs::path & directory, 
                 const fs::path & file_name,
                 fs::path & output)
{
  const fs::recursive_directory_iterator end;
  const auto it = std::find_if(fs::recursive_directory_iterator(directory), end,
                          [&file_name](const fs::directory_entry& e) {
                            return e.path().filename() == file_name;
                          });
  if (it == end) {
    return false;
  } else {
    output = it->path();
    return true;
  }
}

bool loaded(const resource_id & resource)
{
  if (!g_cache || g_assets_root.size() == 0) {
      SDLEx_LogError("%s - no assets root initialized",
        __METHOD_NAME__);
      throw std::exception("no assets root");
  }
  return g_cache->find(resource) != g_cache->end() ? true : false;
}

}; //namespace resources