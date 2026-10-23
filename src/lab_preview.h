#ifndef SOTN_LAB_PREVIEW_H
#define SOTN_LAB_PREVIEW_H

#include <SDL3/SDL_events.h>
#include <sotn/game_data.h>

typedef struct LabPreview LabPreview;

/** @brief Create a graphics-only room viewer borrowing an open core-data snapshot. */
bool LabPreview_Create(const SotnGameData *data, const char *directory, LabPreview **out);
/** @brief Handle camera/room selection keys; false reports a loading/rendering error. */
bool LabPreview_Event(LabPreview *preview, const SDL_Event *event);
/** @brief Refresh changed room pixels and present through the viewer's owned OpenGL context. */
bool LabPreview_Draw(LabPreview *preview);
/** @brief Destroy viewer resources before closing its borrowed core-data snapshot. */
void LabPreview_Destroy(LabPreview *preview);

#endif
