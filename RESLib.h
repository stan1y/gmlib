#ifndef _RES_LIB_H
#define _RES_LIB_H

#include "GMLib.h"
#include "GMTexture.h"
#include "GMData.h"

namespace resources
{

/*
 * Resource ID Format
 *
 * A Resource ID is a string with format of [resource_name:resource_postfix].
 * A resource_name segment is a unique filename of the resource in the assets storage.
 * A resource_postfix is a an optional string witch might be used by certain resource
 * loaders, such as fonts.
 * Example:
 * resources::get_texture("cat.png");
 * resources::get_data("hero.json");
 * resources::get_font("comicsans.ttf:22");
 * resources::find("comicsans.ttf:22") 
 *  -> "/path/to/assets/ui/fonts/comicsans.ttf"
 */

/* Get item from cache by path. Returns nullptr if not found */
iresource * get(const std::string & resource);

/* Get texture from cache by path */
const texture & get_texture(const std::string& resource);

/* Get texture from cache by path */
const data & get_data(const std::string& resource);

/* Get texture from cache by path */
const ttf_font & get_font(const std::string& resource);

/* Initialize resources root */
void initialize(const std::string& assets_root);

/* Get path to the assets root */
std::string root_path();

/* Recursively lookup assets to match file names with give resource id */
std::string find(const std::string& resource);

/* Test if resource file present */
bool exists(const std::string& resource);

}

#endif //_RES_LIB_H