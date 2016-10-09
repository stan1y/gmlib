#include "GMData.h"
#include "GMUtil.h"
#include "RESLib.h"

data::json* GM_LoadJSON(const std::string& file_path)
{
    data::json * json_data = nullptr;
    json_error_t jerr;
    json_data = (data::json*) json_load_file(file_path.c_str(), 0, &jerr);
    if (json_data == nullptr) {
        SDLEx_LogError("GM_LoadJSON: failed to load %s. %s. line: %d, column: %d, position: %d",
          file_path.c_str(), jerr.text,
          jerr.line, jerr.column, jerr.position);
        throw data::json_exception("Failed to load json from file", &jerr);
    }

    return json_data;
}

data::data(const std::string & json_string, uint32_t parser_flags):
  _json(nullptr),
  _f(parser_flags)
{
  json_error_t jerr;
  _json = (json*) json_loadb(json_string.c_str(), 
                             json_string.length(),
                             parser_flags,
                             &jerr);
  if (_json == NULL) {
    SDLEx_LogError("data: failed to parse. %s", jerr.text);
    throw json_exception("failed to parse data");
  }
}

data::data(const std::string & json_file):
  _json(nullptr),
  _f(0)
{
  load(json_file);
}

std::string data::tostr(int ident) const
{
  if (!_json || json_is_null(_json))
    return std::string("null");

  char * json = json_dumps(_json, JSON_ENCODE_ANY | JSON_INDENT(ident));
  std::string str(json);
  free(json);
  return str;
}

void data::load(const std::string & json_file)
{
#ifdef GM_DEBUG
  SDL_Log("data::load - reading %s\n", json_file.c_str()); 
#endif
  assign(GM_LoadJSON(json_file));
}


void data::unpack(const char *fmt, ...) const
{
  json_error_t jerr;
  int ret;
  va_list ap;

  va_start(ap, fmt);
  ret = json_vunpack_ex(_json, &jerr, 0, fmt, ap);
  va_end(ap);
  if (ret != 0)
  {
      SDLEx_LogError("%s - failed to unpack data, error at line: %d, column: %d, position: %d. %s",
        __METHOD_NAME__,
        jerr.line, jerr.column, jerr.position, jerr.text);
      throw json_exception("Failed to unpack data", &jerr);
  }
}

void data::json::unpack(const char *fmt, ...) const
{
  json_error_t jerr;
  int ret;
  va_list ap;

  va_start(ap, fmt);
  ret = json_vunpack_ex((json_t*)this, &jerr, 0, fmt, ap);
  va_end(ap);
  if (ret != 0)
  {
      SDLEx_LogError("%s - failed to unpack data, error at line: %d, column: %d, position: %d. %s",
        __METHOD_NAME__,
        jerr.line, jerr.column, jerr.position, jerr.text);
      throw json_exception("Failed to unpack data", &jerr);
  }
}

void * data::find_iter_recursive(const char* key, json ** found) const
{
  std::vector<std::string> subkeys = split_string(key, '.');
  auto ikey = subkeys.begin();
  void * it = nullptr;
  json * current = _json;
  for(; ikey != subkeys.end(); ++ikey) {
    it = json_object_iter_at(current, ikey->c_str());
    if (it != nullptr) {
      // found this level
      current = (json*)json_object_iter_value(it);
    }
  }
    
  if (found != nullptr) {
    found = &current;
  }

  return it;
}