
#ifndef _LIBAS_H
#define _LIBAS_H

#include "as.h"
#include "osbf.h"

int as_openPedal (as_t *as_ctx, const uint16_t as_productid);
void as_closePedal (as_t *as_ctx);

int as_read (as_t *as_ctx, void *buffer, const int bLength);
int as_write (as_t *as_ctx, const void *buffer, const int bLength);

int as_getHardwareConfig (as_t *as_ctx, as_hw_config_t *cfg);
int as_getPreset (as_t *as_ctx, uint8_t *buffer, const int bufferLen, int preset, int includeName);
int as_getPresetName (as_t *as_ctx, const uint8_t presetIdx, char *name);
int as_setPresetActive (as_t *as_ctx, const int presetIdx);
int as_setPresetName (as_t *as_ctx, char *name, const int presetIdx);
int as_getControlValue (as_t *as_ctx, uint8_t ctrl, uint8_t *value);
int as_setControlValue (as_t *as_ctx, uint8_t ctrl, uint16_t value);
as_product_t as_productLookup (const uint16_t as_productId);
int as_getEEPROM (as_t *as_ctx, uint8_t *buffer, const int bsize);
int as_erase (as_t *as_ctx, const uint8_t presetIdx);
int as_writePreset (as_t *as_ctx, as_preset_t *preset, const int bsize, const int presetIdx);
int as_getActivePreset (as_t *as_ctx);
int as_getVersion (as_t *as_ctx);
int as_dumpFlash (as_t *as_ctx);
int as_getFlash (as_t *as_ctx, const size_t address, const size_t length);
int as_getPresetDefault (as_t *as_ctx, uint8_t *buffer, const size_t bufferLen, int preset);

#endif






