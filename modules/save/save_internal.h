#ifndef SOTN_SAVE_INTERNAL_H
#define SOTN_SAVE_INTERNAL_H

#include <sotn/save.h>

/** @brief Inject a private/test directory without accessing real user save slots.
 * @param directory Existing directory, copied before return. @param out Owned store or NULL.
 * @return Success, or false with SDL_GetError(). */
bool SotnSaveStore_OpenAt(const char *directory, SotnSaveStore **out);

#endif
