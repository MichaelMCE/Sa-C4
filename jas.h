
#ifndef _JAS_H
#define _JAS_H


#include "preset.h"
#include "xpre.h"





const char *jas_getPresetDesc (presets_t *presets, const int presetIdx);
const char *jas_getPresetName (presets_t *presets, const int presetIdx);
const char *jas_getPresetUserName (presets_t *presets, const int presetIdx);
const char *jas_getPresetEffectName (presets_t *presets, const int presetIdx);
const char *jas_getPresetURL (presets_t *presets, const int presetIdx);
const char *jas_getPresetXml (presets_t *presets, const int presetIdx);
int16_t jas_getPresetProductId (presets_t *presets, const int presetIdx);
int jas_getTotalPresets (presets_t *presets);


presets_t *jas_importJsonPath (const char *path);
presets_t *jas_importJsonBuffer (void *buffer, const size_t length);
void jas_free (presets_t *presets);


int jas_getAvailable (const int productId);
char *jas_getList (const int productId, const int getTotal, const int getFrom);
int xpre_hasField (xpre_t *xpre, const char *field);

#endif
