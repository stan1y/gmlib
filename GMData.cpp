#include "GMData.h"
#include "GMUtil.h"
#include "RESLib.h"

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

data::data(const std::string & json, uint32_t parser_flags)
{
  json_error_t jerr;
  _p = json_loadb(json.c_str(), json.length(), parser_flags, &jerr);
  if (_p == NULL) {
    SDLEx_LogError("data: failed to parse. %s", jerr.text);
    throw std::exception("failed to parse data");
  }
}

data::data(const std::string & json_file)
{
  load(json_file);
}

void data::load(const std::string & json_file)
{
  SDL_Log("data::load - reading %s\n", json_file.c_str()); 
  json_t * p = GM_LoadJSON(json_file);
  if (p == NULL) throw std::exception("failed to load data");
  set_owner_of(p);
  json_decref(p);
}

void data::unpack(const char *fmt, ...) const
{
  json_error_t jerr;
  int ret;
  va_list ap;

  va_start(ap, fmt);
  ret = json_vunpack_ex(_p, &jerr, 0, fmt, ap);
  va_end(ap);
  if (ret != 0)
  {
      SDLEx_LogError("Failed to unpack data, error at line: %d, column: %d, position: %d. %s",
          jerr.line, jerr.column, jerr.position, jerr.text);
      throw std::exception("failed to unpack data");
  }
}
