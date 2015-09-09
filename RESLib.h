#ifndef _RES_LIB_H
#define _RES_LIB_H

#include "GMLib.h"

/* Get item from cache by path. Returns nullptr if not found */
void* RES_Get(const std::string& id);

/* Append item to cache. Raises exception if item already there */
void RES_Put(const std::string& id, void* res);

/* Initialize resources root */
void RES_Init(const std::string& assets_root);
std::string RES_GetAssetsRoot();

/* Return full path to a resource file */
std::string RES_GetFullPath(const std::string& relPath);

/* Test if resource file present */
bool RES_CheckExists(const std::string& relPath);

#endif //_RES_LIB_H