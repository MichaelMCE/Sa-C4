

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>
#include <windows.h>	// for Sleep()

#include "libas.h"
#include "ctrl.h"
#include "jas.h"
#include "help.h"
#include "osbf.h"



#define SAC4_NAME					"SA-C4"
#define SAC4_VERSION				"24032023"
#define SAC4_TAG					"A C4 Synth interrogator"

#define MAX_AVAILABLE_PRESETS		(2800)
#define CONSOLE_VARCOL				2

static const uint8_t bitmasks[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};




static void printHex16 (const uint8_t *data, size_t length)
{
	for (int i = 0; i < length; i += 16){
		for (int j = i; j < i+16 && j < length; j++){
			printf("%2.2X ", data[j]);
		}
		printf("\n");
	}
	printf("\n");
}


void printHex32 (const uint8_t *data, size_t length)
{
	for (int i = 0; i < length; i += 32){
		for (int j = i; j < i+32 && j < length; j++){
			printf("%2.2X ", data[j]);
		}
		printf("\n");
	}
	printf("\n");
}
/*
static void printHex (const uint8_t *data, size_t length)
{
	for (int i = 0; i < length; i++)
		printf("%.2X ", data[i]);
	printf("\n");
}
*/
static inline char *rtrim (char *buffer, int len)
{
	while (len > 0 && buffer[--len] == ' ')
		buffer[len] = 0;
	return buffer;
}

static void printStrCol (const char *str, const int col)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	
	SetConsoleTextAttribute(hCon, col);
	printf(str);
	SetConsoleTextAttribute(hCon, 7);
	printf("\n");
}

static void printStrColVar1 (const char *str1, const char *str2, const int col, const int32_t var1)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	
	printf(str1);
	SetConsoleTextAttribute(hCon, col);
	printf(str2, var1);
	SetConsoleTextAttribute(hCon, 7);
	printf("\n");
}

static void printIntColStr (const char *str1, const char *str2, const int col, const int32_t var1, const char *str3)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	
	printf(str1, var1);
	SetConsoleTextAttribute(hCon, col);
	printf(str2, str3);
	SetConsoleTextAttribute(hCon, 7);
	printf("\n");
}

static void printStrStr (const char *str1, const char *str2, const int col, const char *var1)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	
	printf(str1);
	SetConsoleTextAttribute(hCon, col);
	printf(str2, var1);
	SetConsoleTextAttribute(hCon, 7);
	printf("\n");
}

static void printStrStrVar (const char *str1, const char *str2, const int col, const char *var1, const int32_t var2)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	
	printf(str1, var1);
	SetConsoleTextAttribute(hCon, col);
	printf(str2, var2);
	SetConsoleTextAttribute(hCon, 7);
	printf("\n");
}

static void printStrColVar2 (const char *str1, const char *str2, const int col, int32_t var1, int32_t var2)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	
	printf(str1);
	SetConsoleTextAttribute(hCon, col);
	printf(str2, var1, var2);
	SetConsoleTextAttribute(hCon, 7);
	printf("\n");
}

static inline int isEqual (const as_preset_t *preset1, const as_preset_t *preset2)
{
	uint8_t *buffer1 = (uint8_t*)preset1;
	uint8_t *buffer2 = (uint8_t*)preset2;
	
	for (int i = 0; i < AS_PRESET_LENGTH; i++){
		if (buffer1[i] != buffer2[i])
			return 0;
	}
	return 1;
}

static inline int isDefault (const int presetIdx)
{
	return ((presetIdx >= 0) && (presetIdx < AS_PRESET_DEFAULTS_TOTAL));
}

static inline int isValid (const as_preset_t *preset)
{
	uint8_t *buffer = (uint8_t*)preset;
	if ((buffer[AS_PRESET_LENGTH-3] == 0xFF) || (buffer[AS_PRESET_LENGTH-2] == 0xFF))
		return 0;
	else
		return 1;
}

static void json2Preset (as_t *as_ctx, char *jsonData)
{
	presets_t *presets = jas_importJsonBuffer(jsonData, 0);
	if (presets){
		xpre_t xpre;
		xpre_init(&xpre);
		
		// prepare json for processing
		const char *xml = jas_getPresetXml(presets, 0);
		if (!xpre_prepareString(&xpre, xml, -1))
			return;
		
		if (!xpre_hasField(&xpre, "preset_name"))
			xpre_append(&xpre, "preset_name", jas_getPresetName(presets, 0));	


		// process json; extract preset converting to binary 
		uint8_t binOut[AS_PRESET_MAX_LENGTH] = {0};
		int len = xpre_preset2bin(&xpre, binOut, XPRE_FORMAT_NAME, 0);
		
		//printf("json2Preset len %i\n", len);
		
		// convert binary preset to .pre formatted xml
		as_preset_t *preset = (as_preset_t*)binOut;
		uint8_t buffer[512*1024];			// only need around 7k so 512k should be enough
		xpre_init(&xpre);
		xpre_prepareBuffer(&xpre, buffer, sizeof(buffer));
		len = xpre_bin2string(&xpre, preset, sizeof(*preset));
		
		if (len > 100){		// magic number: Something that should represent minimum length of a valid .pre file
			printf("%s\n", buffer);
		}
		jas_free(presets);
	}
}


// acquire a single preset directly from Source Audio's website
void cmd_jsonGetPresetPre (as_t *as_ctx, const int arg)
{
	const int listFrom = arg;
	const int listTotal = 1;
	
	char *buffer = jas_getList(AS_PRODUCT_C4_SYNTH, listTotal, listFrom);
	if (buffer){
		json2Preset(as_ctx, buffer);
		free(buffer);
	}
}

static int sendToPedal (as_t *as_ctx, char *buffer, const int presetIdx, const int action, char *name)
{
	int ret = -1;
	
	presets_t *presets = jas_importJsonBuffer(buffer, 0);
	if (presets){
		xpre_t xpre;
		xpre_init(&xpre);
		xpre_prepareString(&xpre, jas_getPresetXml(presets, 0), -1);
		
		if (!xpre_hasField(&xpre, "preset_name"))
			xpre_append(&xpre, "preset_name", jas_getPresetName(presets, 0));

		printf("%s\nSending to pedal..\n", jas_getPresetName(presets, 0));

		uint8_t binOut[AS_PRESET_MAX_LENGTH] = {0};
		int len = xpre_preset2bin(&xpre, binOut, action, presetIdx);

		if (!(len%AS_PACKET_LENGTH)){
			ret = as_write(as_ctx, binOut, len);
			if (ret == len){
				ret = 1;
				if (name)
					strcpy(name, jas_getPresetName(presets, 0));
			}
		}
		jas_free(presets);
	}
	return ret;
}

static int cmd_sendToPedal (as_t *as_ctx, const int arg1, const int arg2, const int action)
{
	int listFrom = arg1;
	int listTotal = 1;
	int presetIdx = arg2;
	
	if (presetIdx == -1){
		as_hw_config_t cfg = {0};
		if (!as_getHardwareConfig(as_ctx, &cfg))
			return 0;
		presetIdx = cfg.activePreset;
	}

	if (presetIdx < 0 || presetIdx >= AS_PRESET_TOTAL)
		return 0;

	printf("Requesting preset %i.. :", listFrom+1);

	char *buffer = jas_getList(AS_PRODUCT_C4_SYNTH, listTotal, listFrom);
	if (buffer){
		as_setPresetActive(as_ctx, presetIdx);
		char name[AS_PRESET_NAME_LENGTH+1] = {0};
		
		if (sendToPedal(as_ctx, buffer, presetIdx, action&0x03, name) == 1){
			printf("Success\n");

			if (action == XPRE_FORMAT_STORE)
				printf("To save, use: sa-c4.exe --saveas %i \"%s\"\n", presetIdx+1, name);
			
		}
		free(buffer);
		return 1;
	}
	
	printf("\nFailed\n");
	return 0;
}

void cmd_jsonSendToPedalStore (as_t *as_ctx, const int arg1, const int arg2)
{
	cmd_sendToPedal(as_ctx, arg1, arg2, XPRE_FORMAT_STORE);
}

void cmd_jsonSendToPedalSave (as_t *as_ctx, const int arg1, const int arg2)
{
	cmd_sendToPedal(as_ctx, arg1, arg2, XPRE_FORMAT_SAVE);
}

static void printJsonPresetList (as_t *as_ctx, void *buffer, const int listFrom)
{
	const int c = CONSOLE_VARCOL;
	
	presets_t *presets = jas_importJsonBuffer(buffer, -1);
	if (presets){
		const int total = jas_getTotalPresets(presets);
		for (int i = 0; i < total; i++){
			printStrColVar1("\n","%i:", c, listFrom+i+1);
			printStrStr("PresetName: ", "%s", c, jas_getPresetName(presets, i));
			printStrStr("Desc: ", "%s", c, jas_getPresetDesc(presets, i));
			printStrStr("EffectName: ", "%s", c, jas_getPresetEffectName(presets, i));
			printStrStr("UserName: ", "%s", c, jas_getPresetUserName(presets, i));
		}
		jas_free(presets);
	}
}

int cmd_jsonGetTotalAvailable (as_t *as_ctx, const int arg)
{
	int count = jas_getAvailable(AS_PRODUCT_C4_SYNTH);
	printf("\n%i presets available\n", count);
	return count;
}

void cmd_jsonListAvailable (as_t *as_ctx, const int arg)
{
	int listTotal = 5;
	int listFrom = arg;
	
	char *buffer = jas_getList(AS_PRODUCT_C4_SYNTH, listTotal, listFrom);
	if (!buffer) return;
	
	printJsonPresetList(as_ctx, buffer, listFrom);
	free(buffer);
}

void cmd_jsonGetPresets (as_t *as_ctx, const int listFrom, const int listTotal)
{
	// sanity check
	if (listFrom < 0 || listTotal < 1 || listTotal > MAX_AVAILABLE_PRESETS)
		return;

	char *buffer = jas_getList(AS_PRODUCT_C4_SYNTH, listTotal, listFrom);
	if (!buffer) return;
	
	printf("%s\n", buffer);
	free(buffer);
}

void cmd_retrievePresetName (as_t *as_ctx, const int arg)
{
	const int presetIdx = arg;
	if (presetIdx < 0 || presetIdx >= AS_PRESET_TOTAL)
		return;
	
	char presetName[AS_PRESET_NAME_LENGTH+1] = {32};

	int ret = as_getPresetName(as_ctx, presetIdx, presetName);
	if (ret == 1)
		printStrCol(rtrim(presetName, AS_PRESET_NAME_LENGTH), CONSOLE_VARCOL);
	else if (ret == -1)
		printf("--empty--\n");
}

void cmd_retrievePresetNameRange (as_t *as_ctx, const int fromIdx, const int toIdx)
{
	char presetName[AS_PRESET_NAME_LENGTH+1];

	for (int i = fromIdx; i <= toIdx; i++){
		int ret = as_getPresetName(as_ctx, i, presetName);
		if (ret == 1)
			printf(" %.3i: %s\n", i+1, rtrim(presetName, AS_PRESET_NAME_LENGTH));
		else if (ret == -1)
			printf(" %.3i: --empty--\n", i+1);
	}
}

void cmd_retrievePresetNameAll (as_t *as_ctx, const int arg)
{
	cmd_retrievePresetNameRange(as_ctx, 0, AS_PRESET_TOTAL-1);
}

void cmd_retrieveHardwareConfig (as_t *as_ctx, const int arg)
{
	as_hw_config_t cfg = {0};
	if (!as_getHardwareConfig(as_ctx, &cfg))
		return;


	const int col = CONSOLE_VARCOL;
	printf("\n");
	printStrColVar2("Firmware: ", "%i.%i", col, cfg.firmwareVersion&0xFF, cfg.firmwareVersion>>8);
	printStrColVar1("Device model: ", "%i", col, cfg.deviceModel);
	printStrColVar1("Number of presets: ", "%i", col, cfg.numberOfPresets);
	printStrColVar1("Active preset: ", "%i", col, cfg.activePreset+1);
	printStrColVar1("Preset/WYSIWYG: ", "%i", col, cfg.WYSIWYG);
	printStrColVar1("Hardware bypass mode: ", "%i", col, cfg.hardwareBypassMode);
	printStrColVar1("Midi channel (1 to 16): ", "%i ", col, cfg.midiChannel+1);
	printStrColVar1("Ccontrol input option: ", "%i", col, cfg.controlInputOption);
	printStrColVar1("External loop safe mode: ", "%i", col, cfg.externalLoopSafeMode);
	printStrColVar1("Power up state: ", "%i", col, cfg.powerUpState);
	printStrColVar1("Default input routing: ", "%i", col, cfg.defaultInputRouting);
	printStrColVar1("Quick bank select: ", "%i", col, cfg.quickBankSelect);
	printStrColVar1("Neuro hub pedel input calibration Min: ", "%i", col, cfg.hubPedelCalMin);
	printStrColVar1("Neuro hub pedel input calibration Range: ", "%i", col, cfg.hubPedelCalRange);
	printStrColVar1("Control input calibration Min: ", "%i", col, cfg.inputCalMin);
	printStrColVar1("control input calibration Range: ", "%i", col, cfg.inputCalRange);
	printStrColVar1("USB-Midi skip power check: ", "%i", col, cfg.usbMidiSkipPowerCheck);	
}


void printPresetStruct (as_preset_t *preset)
{	
	const int c = CONSOLE_VARCOL;
		
	printStrStr("Name: ", "%s", c, (char*)preset->name);
	printf(" Levels:\n");
	
	printStrColVar1("input1_gain    ", "%i", c, preset->level.input1_gain);
	printStrColVar1("input2_gain    ", "%i", c, preset->level.input2_gain);
	printStrColVar1("master_depth   ", "%i", c, preset->level.master_depth);
	printStrColVar1("mod_source     ", "%i", c, preset->level.mod_source);
	printStrColVar1("bass           ", "%i", c, preset->level.bass);
	printStrColVar1("treble         ", "%i", c, preset->level.treble);
	printStrColVar1("mix            ", "%i", c, preset->level.mix);
	printStrColVar1("lo_retain      ", "%i", c, preset->level.lo_retain);
	printStrColVar1("output         ", "%i", c, preset->level.output);
	printStrColVar1("output_balance ", "%i", c, preset->level.output_balance);
	printf("\n");
	
	for (int i = 0; i < AS_VOICE_TOTAL; i++){
		printStrColVar1(" Filter: ", "%i", c, i+1);
		printStrColVar1("depth            ", "%i", c, preset->filter[i].depth);
		printStrColVar1("frequency        ", "%i", c, preset->filter[i].frequency);
		printStrColVar1("q                ", "%i", c, preset->filter[i].q);
		printStrColVar1("type             ", "%i", c, preset->filter[i].type);
		printStrColVar1("envelope         ", "%i", c, preset->filter[i].envelope);
		printStrColVar1("invert	         ", "%i", c, preset->filter[i].invert);
		printStrColVar1("enable	         ", "%i", c, preset->filter[i].enable);
		printStrColVar1("pitch_track      ", "%i", c, preset->filter[i].pitch_track);
		printStrColVar1("mix_destination  ", "%i", c, preset->filter[i].mix_destination);
		printStrColVar1("mix_enable       ", "%i", c, preset->filter[i].mix_enable);
		printf("\n");
	}
	
	for (int i = 0; i < AS_ENVELOPE_TOTAL; i++){
		printStrColVar1(" Envelope: ", "%i", c, i+1);
		printStrColVar1("sensitivity     ", "%i", c, preset->envelope[i].sensitivity);
		printStrColVar1("speed           ", "%i", c, preset->envelope[i].speed);
		printStrColVar1("gate            ", "%i", c, preset->envelope[i].gate);
		printStrColVar1("type;           ", "%i", c, preset->envelope[i].type);
		printStrColVar1("input           ", "%i", c, preset->envelope[i].input);      
		printf("\n");
	}

	printf(" Distortion:\n");
	printStrColVar1("drive              ", "%i", c, preset->distortion.drive); 
	printStrColVar1("mix                ", "%i", c, preset->distortion.mix);
	printStrColVar1("output             ", "%i", c, preset->distortion.output);
	printStrColVar1("type               ", "%i", c, preset->distortion.type);
	printStrColVar1("enable             ", "%i", c, preset->distortion.enable);
	printf("\n");
	
	printStrColVar1("fm_sine1           ", "%i", c, preset->fm_sine1);
	printStrColVar1("fm_sine2           ", "%i", c, preset->fm_sine2);
	printStrColVar1("fm_sine1_input     ", "%i", c, preset->fm_sine1_input);
	printStrColVar1("fm_sine2_input     ", "%i", c, preset->fm_sine2_input);
	printStrColVar1("mono_pitch_filter1 ", "%i", c, preset->mono_pitch_filter1);
	printStrColVar1("mono_pitch_filter2 ", "%i", c, preset->mono_pitch_filter2);
	printf("\n");


	printf(" LFO:\n");
	printStrColVar1("speed              ", "%i", c, preset->lfo.speed);
	printStrColVar1("env_to_speed       ", "%i", c, preset->lfo.env_to_speed);
	printStrColVar1("env_to_depth       ", "%i", c, preset->lfo.env_to_depth);
	printStrColVar1("to_phase           ", "%i", c, preset->lfo.to_phase);
	printStrColVar1("to_multiply        ", "%i", c, preset->lfo.to_multiply);
	printStrColVar1("shape              ", "%i", c, preset->lfo.shape);
	printStrColVar1("restart            ", "%i", c, preset->lfo.restart);
	printStrColVar1("beat_division      ", "%i", c, preset->lfo.beat_division);
	printStrColVar1("tempo              ", "%i", c, preset->lfo.tempo);
	printf("\n");
	
	for (int i = 0; i < AS_SEQUENCER_TOTAL; i++){
		printStrColVar1(" Sequencer: ", "%i", c, i+1);
		printStrColVar1("steps           ", "%i", c, preset->sequencer[i].steps);
		
		for (int j = 0; j < preset->sequencer[i].steps; j++)
			printStrColVar2("value ", "%i   \t%i", c, j+1, preset->sequencer[i].value[j]);
		printf("\n");
	}

	printf(" Harmony:\n");
	printStrColVar1("tuning          ", "%i", c, preset->harmony.tuning);
	printStrColVar1("key             ", "%i", c, preset->harmony.key);
	printStrColVar1("interval1       ", "%i", c, preset->harmony.interval1);
	printStrColVar1("mode            ", "%i", c, preset->harmony.mode);
	printStrColVar1("interval2       ", "%i", c, preset->harmony.interval2);
	printf("\n");


	printf(" Pitch detect:\n");
	printStrColVar1("input           ", "%i", c, preset->pitchdetect.input);
	printStrColVar1("mode            ", "%i", c, preset->pitchdetect.mode);
	printStrColVar1("low_note        ", "%i", c, preset->pitchdetect.low_note);
	printStrColVar1("high_note       ", "%i", c, preset->pitchdetect.high_note);
	printf("\n");

	printf(" General:\n");
	printStrColVar1("knob1_assign         ", "%i", c, preset->knob1_assign);
	printStrColVar1("knob2_assign         ", "%i", c, preset->knob2_assign);
	printStrColVar1("routing_option       ", "%i", c, preset->routing_option);
	printStrColVar1("filter2_correction   ", "%i", c, preset->filter2_correction);
	printStrColVar1("on_off_status        ", "%i", c, preset->on_off_status);
	printStrColVar1("ext_control_enable   ", "%i", c, preset->ext_control_enable);
	printStrColVar1("lfo_midi_clock_sync  ", "%i", c, preset->lfo_midi_clock_sync);
	printf("\n");

	for (int i = 0; i < AS_EXT_TOTAL; i++){
		printStrColVar1(" Ext: ", "%i", c, i+1);
		printStrColVar1("destination  ", "%i", c, preset->ext[i].destination);
		printStrColVar1("source       ", "%i", c, preset->ext[i].source);
		printStrColVar1("min          ", "%i", c, preset->ext[i].min);
		printStrColVar1("max          ", "%i", c, preset->ext[i].max);
		printf("\n");
	}
}

static void applyPresetSeqStepsFixup (as_preset_t *preset)
{
	for (int i = 0; i < AS_SEQUENCER_TOTAL; i++)
		preset->sequencer[i].steps += 2;
}

void cmd_retrievePresetValues (as_t *as_ctx, const int presetIdx)
{
	if (presetIdx < 0 || presetIdx >= AS_PRESET_TOTAL)
		return;
			
	as_preset_t preset = {0};
	if (as_getPreset(as_ctx, (void*)&preset, sizeof(preset), presetIdx, 1) != 1)
		goto exit;
	
	if (isValid(&preset)){
		applyPresetSeqStepsFixup(&preset);
		printPresetStruct(&preset);
		return;
	}
	
exit:
	printf("Preset %i not available\n", presetIdx+1);
}	

void cmd_retrievePresetBin (as_t *as_ctx, const int arg)
{
	const int presetIdx = arg;
	if (presetIdx < 0 || presetIdx >= AS_PRESET_TOTAL)
		return;
			
	as_preset_t preset = {0};
	as_getPreset(as_ctx, (void*)&preset, sizeof(preset), presetIdx, 1);
	printHex32((void*)&preset, AS_PRESET_LENGTH);
}	

void cmd_retrievePresetPre (as_t *as_ctx, const int presetIdx)
{
	if (presetIdx < 0 || presetIdx >= AS_PRESET_TOTAL)
		return;

	as_preset_t preset = {0};
	if (as_getPreset(as_ctx, (void*)&preset, sizeof(preset), presetIdx, 1) != 1 || !isValid(&preset)){
		printf("Preset %i not available\n", presetIdx+1);
		return;
	}	
	
	int bufferLen = 32*1024;			// a C4 present is ~6.7K. 32k should never not be enough.
	char *buffer = calloc(1, bufferLen);
	if (!buffer){
		printf("emotional damage\n");
		return;
	}

	applyPresetSeqStepsFixup(&preset);
	
	xpre_t xpre;
	xpre_init(&xpre);
	xpre_prepareBuffer(&xpre, buffer, bufferLen);
	xpre_bin2string(&xpre, &preset, sizeof(preset));
	
	printf("\n%s\n", xpre.buffer);
	free(buffer);
}

void cmd_retrievePresetPreAll (as_t *as_ctx, const int arg)
{
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		printf("\n- Preset: %i\n", i+1);
		cmd_retrievePresetPre(as_ctx, i);
	}
}

void cmd_setPresetActive (as_t *as_ctx, const int arg)
{
	const int presetIdx = arg;
	if (presetIdx < 0 || presetIdx >= AS_PRESET_TOTAL)
		return;
		
	if (as_setPresetActive(as_ctx, presetIdx))
		printf("Preset %i selected\n", presetIdx+1);
	else
		printf("Preset not set\n");
}

void cmd_setPresetNext (as_t *as_ctx, const int arg)
{
	
	as_hw_config_t cfg = {0};
	if (!as_getHardwareConfig(as_ctx, &cfg))
		return;

	int nextPreset = cfg.activePreset + 1;
	if (nextPreset < cfg.numberOfPresets){
		//printf("Setting Preset: %i\n", nextPreset+1);
		
		as_setPresetActive(as_ctx, nextPreset);
		as_getHardwareConfig(as_ctx, &cfg);
		//cmd_retrievePresetName(as_ctx, cfg.activePreset);
		
		char presetName[AS_PRESET_NAME_LENGTH+1];
		int ret = as_getPresetName(as_ctx, cfg.activePreset, presetName);
		if (ret == 1)
			printf("%i: %s\n", cfg.activePreset+1, presetName);
		else if (ret == -1)
			printf("%i: --empty--\n", cfg.activePreset+1);
	}
}

void cmd_setPresetPrevious (as_t *as_ctx, const int arg)
{
	
	as_hw_config_t cfg = {0};
	if (!as_getHardwareConfig(as_ctx, &cfg))
		return;

	int previousPreset = cfg.activePreset - 1;
	if (previousPreset >= 0){
		//printf("Setting Preset: %i\n", previousPreset+1);
		
		as_setPresetActive(as_ctx, previousPreset);
		as_getHardwareConfig(as_ctx, &cfg);
		
		char presetName[AS_PRESET_NAME_LENGTH+1];
		int ret = as_getPresetName(as_ctx, cfg.activePreset, presetName);
		if (ret == 1)
			printf("%i: %s\n", cfg.activePreset+1, presetName);
		else if (ret == -1)
			printf("%i: --empty--\n", cfg.activePreset+1);
	}
}

void cmd_retrievePresetActive (as_t *as_ctx, const int arg)
{
	as_hw_config_t cfg;
	if (!as_getHardwareConfig(as_ctx, &cfg)){
		printf("as_getHardwareConfig failed\n");
		return;
	}
	printf("%i", cfg.activePreset+1);
}

void cmd_retrievePresetActiveAndName (as_t *as_ctx, const int arg)
{
	as_hw_config_t cfg;
	if (!as_getHardwareConfig(as_ctx, &cfg)){
		printf("as_getHardwareConfig failed\n");
		return;
	}

	char presetName[AS_PRESET_NAME_LENGTH+1];
	int ret = as_getPresetName(as_ctx, cfg.activePreset, presetName);
	if (ret == 1)
		printIntColStr("%i: ", "%s", CONSOLE_VARCOL, cfg.activePreset+1, presetName);
	else if (ret == -1)
		printf("%i: --empty--\n", cfg.activePreset+1);
}

void cmd_printVersion (as_t *as_ctx, const int arg)
{
	printf("%s: %s\nVersion: %s\n\n", SAC4_NAME, SAC4_TAG, SAC4_VERSION);
}

void cmd_lookup (as_t *as_ctx, const int arg)
{
	as_product_t did = as_productLookup(AS_PRODUCT_C4_SYNTH);
	printf("Pedal: %s, VID:0x%X PID:0x%X\n\n\n", did.name, did.VID, did.PID);
}

void cmd_dumpFlash (as_t *as_ctx, const int arg)
{
	as_dumpFlash(as_ctx);
}

void cmd_getFlash (as_t *as_ctx, const int address, const int length)
{
	if (address%AS_PAYLOAD_LENGTH){
		printf("address 0x%X is not a multiple of %i\n", address, AS_PAYLOAD_LENGTH);
		return;
	}
	if (length%AS_PAYLOAD_LENGTH){
		printf("length %i is not a multiple of %i\n", length, AS_PAYLOAD_LENGTH);
		return;
	}
	if (address > (AS_C4_FLASHSIZE - AS_PAYLOAD_LENGTH)+1){
		printf("invalid length: %i\n", length);
		return;
	}
	if ((AS_C4_FLASHSIZE - address)+1 < length){
		printf("invalid length from address: 0x%X\n", address);
		return;
	}

	as_getFlash(as_ctx, address, length);
}

void cmd_listCtrls (as_t *as_ctx)
{
	const int tCtrls = ctrl_c4_total();
	
	for (int i = 0; i < tCtrls; i++){
		ctrl_t *ctrl = ctrl_c4_get(i);
		if (ctrl->getIdx == -1) continue;

		printf("%-22s\n", ctrl->label);
	}
}

void cmd_ctrls (as_t *as_ctx)
{
	const int tCtrls = ctrl_c4_total();
	
	for (int i = 0; i < tCtrls; i++){
		ctrl_t *ctrl = ctrl_c4_get(i);
		if (ctrl->getIdx == -1) continue;
		
		as_getControlValue(as_ctx, ctrl->getIdx, &ctrl->u.val8);
		
		if (ctrl->width < 8){
			ctrl->u.val8 >>= ctrl->bitPosition;
			ctrl->u.val8 &= bitmasks[ctrl->width];
		}

		printStrStrVar("%-22s ", "%-3i", CONSOLE_VARCOL, ctrl->label, ctrl->u.val8);
	}
}

void cmd_getCtrlValue (as_t *as_ctx, const char *name)
{
	ctrl_t *ctrl = ctrl_c4_find(name);
	if (ctrl){
		if (ctrl->getIdx == -1){
			printf("not implemented\n");
			return;
		}
		uint8_t ctrlIdx = ctrl->getIdx&0xFF;
		uint8_t value8 = 0;
		as_getControlValue(as_ctx, ctrlIdx, &value8);

		if (ctrl->width < 8){
			value8 >>= ctrl->bitPosition;
			value8 &= bitmasks[ctrl->width];
		}

		//printf("%s: %i\n", ctrl->label, value8);
		printf("%i\n", value8);
	}else{
		printf("Control '%s' not recognised\n", name);
	}
}

void cmd_setCtrlValue (as_t *as_ctx, const char *name, const int value)
{
	ctrl_t *ctrl = ctrl_c4_find(name);
	if (ctrl){
		if (ctrl->setIdx == -1){
			printf("not implemented\n");
			return;
		}
		
		uint8_t ctrlIdx = ctrl->setIdx&0xFF;
		uint16_t value16 = value&0xFFFF;
		if (ctrl->width < 8)
			value16 &= bitmasks[ctrl->width];

		as_setControlValue(as_ctx, ctrlIdx, value16);

		printf("%s set to %i\n", ctrl->label, value16);
	}else{
		printf("'%s' not recognised\n", name);
	}
}

void cmd_importPresetAndStore (as_t *as_ctx, char *path, const int presetIdx)
{
	xpre_t xpre;
	xpre_init(&xpre);
	int ret = xpre_preparePath(&xpre, path);
	if (!ret){
		printf("file not found or invalid\n");
		return;
	}

	uint8_t buffer[AS_PRESET_MAX_LENGTH] = {0};
	int len = xpre_preset2bin(&xpre, buffer, XPRE_FORMAT_STORE, presetIdx);

	if (!(len%AS_PACKET_LENGTH)){
		as_setPresetActive(as_ctx, presetIdx);
		ret = as_write(as_ctx, buffer, len);
		if (ret == len) ret = 1;
	}

	xpre_cleanup(&xpre);
	
	if (ret == 1)
		printf("Success\n");
	else
		printf("Failed\n");
}

void cmd_pause (as_t *as_ctx, const int arg)
{
	printf("\n");
	fflush(stdout);
	system("pause");
}

void cmd_wait (as_t *as_ctx, const int arg)
{
	int sleepMS = abs(arg);
	if (sleepMS)
		Sleep(sleepMS);
}

void cmd_importPresetAndSave (as_t *as_ctx, char *path, const int presetIdx)
{
	xpre_t xpre;
	xpre_init(&xpre);
	int ret = xpre_preparePath(&xpre, path);
	if (!ret){
		printf("file not found or invalid\n");
		return;
	}
	
	uint8_t buffer[AS_PRESET_MAX_LENGTH] = {0};
	int len = xpre_preset2bin(&xpre, buffer, XPRE_FORMAT_SAVE, presetIdx);

	if (!(len%AS_PACKET_LENGTH)){
		as_setPresetActive(as_ctx, presetIdx);
		ret = as_write(as_ctx, buffer, len);
		if (ret == len) ret = 1;
	}

	xpre_cleanup(&xpre);
	
	if (ret == 1)
		printf("Success\n");
	else
		printf("Failed\n");
}

void cmd_dumpEEPROM (as_t *as_ctx, const int arg)
{
	uint8_t buffer[AS_EEPROM_SIZE] = {0};
	as_getEEPROM(as_ctx, buffer, AS_EEPROM_SIZE);
	printHex16(buffer, AS_EEPROM_SIZE);
}

void osbfImportCB (void *object, const int presetIdx, const int action)
{
	as_backup_t *bku = (as_backup_t*)object;
	
	if (action == OSBF_CB_WRITE){
		printf("Writting: %.3i:%s\n", presetIdx+1, rtrim((char*)bku->preset.name, AS_PRESET_NAME_LENGTH));
		as_writePreset(bku->as_ctx, &bku->preset, AS_PRESET_LENGTH, presetIdx);

	}else if (action == OSBF_CB_ERASE){
		char name[AS_PRESET_NAME_LENGTH+1];
		if (as_getPresetName(bku->as_ctx, presetIdx, name) == 1){
			if (strcmp(name, "--empty--")){				// erase only when not already marked empty
				printf("Erasing: %.3i\n", presetIdx+1);
				as_erase(bku->as_ctx, presetIdx);
			}
		}
	}
}

void cmd_importOSBF (as_t *as_ctx, const char *path)
{
	printf("Importing device backup: %s\n", path);

	as_backup_t bku;

	osbf_init(&bku, as_ctx, stdout, osbfImportCB);
	osbf_read_open(&bku, path);
	osbf_read(&bku);
	osbf_close(&bku);
}

void cmd_exportOSBF (as_t *as_ctx, const int arg)
{
	as_backup_t bku;

	osbf_init(&bku, as_ctx, stdout, NULL);
	osbf_generate(&bku);
	osbf_close(&bku);
}

void cmd_copy (as_t *as_ctx, const int fromIdx, const int toIdx)
{
	char name[AS_PRESET_NAME_LENGTH+1];
	memset(name, 32, AS_PRESET_NAME_LENGTH);
	name[AS_PRESET_NAME_LENGTH] = 0;

	int ret = as_getPresetName(as_ctx, fromIdx, name);
	if (ret == 1){
		as_setPresetActive(as_ctx, fromIdx);
		as_setPresetName(as_ctx, rtrim(name, AS_PRESET_NAME_LENGTH), toIdx);
		
		printf("Preset %i copied to %i\n", fromIdx+1, toIdx+1);
	}else{
		printf("Copy failed - check input\n");
	}
}

void cmd_move (as_t *as_ctx, const int fromIdx, const int toIdx)
{
	char name[AS_PRESET_NAME_LENGTH+1];
	memset(name, 32, AS_PRESET_NAME_LENGTH);
	name[AS_PRESET_NAME_LENGTH] = 0;

	int ret = as_getPresetName(as_ctx, fromIdx, name);
	if (ret == 1){
		as_setPresetActive(as_ctx, fromIdx);
		as_setPresetName(as_ctx, rtrim(name, AS_PRESET_NAME_LENGTH), toIdx);
		as_erase(as_ctx, fromIdx);
		
		// As Neuro Desktop rewrites default presets (1 to 6) then we too must do this
		if (isDefault(fromIdx)){
			as_preset_t preset;
			if ((as_getPresetDefault(as_ctx, (void*)&preset, sizeof(preset), fromIdx) == 1)){
				if (isValid(&preset))
					as_writePreset(as_ctx, &preset, AS_PRESET_SIZE, fromIdx);
			}
		}
		printf("Preset %i moved to %i\n", fromIdx+1, toIdx+1);
	}else{
		printf("Move failed - check input\n");
	}
}

void cmd_delete (as_t *as_ctx, const int preIdx)
{
	printf("Erasing %i... ", preIdx+1);
	
	int ret = as_erase(as_ctx, preIdx);
	if (ret == 1){
	 	if (isDefault(preIdx)){
			as_preset_t preset;
			if ((as_getPresetDefault(as_ctx, (void*)&preset, sizeof(preset), preIdx) == 1)){
				if (isValid(&preset))
					as_writePreset(as_ctx, &preset, AS_PRESET_SIZE, preIdx);
			}
		}
		printf("done\n");
	}else{
		printf("failed\n");
	}
}

void cmd_rename (as_t *as_ctx, char *name, const int preIdx)
{
	if (strlen(name) > AS_PRESET_NAME_LENGTH)
		name[AS_PRESET_NAME_LENGTH] = 0;

	if (as_setPresetActive(as_ctx, preIdx)){
		if (as_setPresetName(as_ctx, name, preIdx)){
			printf("Preset %i renamed to '%s'\n", preIdx+1, name);
			return;
		}
	}
	printf("Rename failed - check input\n");
}

void cmd_saveas (as_t *as_ctx, char *name, const int preIdx)
{
	if (strlen(name) > AS_PRESET_NAME_LENGTH)
		name[AS_PRESET_NAME_LENGTH] = 0;

	if (as_setPresetName(as_ctx, name, preIdx)){
		printf("Preset saved to %i as '%s'\n", preIdx+1, name);
		return;
	}

	printf("Save failed - check input\n");
}

void cmd_isdup (as_t *as_ctx, const int preIdx1, const int preIdx2)
{
	as_preset_t preset1;
	as_preset_t preset2;


	if (preIdx1 == preIdx2){
		printf("Preset %i == %i\n", preIdx1+1, preIdx2+1);
		return;
	}

	if ((as_getPreset(as_ctx, (void*)&preset1, sizeof(preset1), preIdx1, 0) != 1) || !isValid(&preset1)){
		printf("Preset %i not available\n", preIdx1+1);
		return;
	}

	if ((as_getPreset(as_ctx, (void*)&preset2, sizeof(preset2), preIdx2, 0) != 1) || !isValid(&preset2)){
		printf("Preset %i not available\n", preIdx2+1);
		return;
	}

	if (isEqual(&preset1, &preset2)){
		printf("%i <> %i: identical\n", preIdx1+1, preIdx2+1);
		return;
	}else{
		printf("%i <> %i: different\n", preIdx1+1, preIdx2+1);
		return;
	}
}

void cmd_finddup (as_t *as_ctx, const int preIdx)
{
	as_preset_t preset;
	
	printf("Searching for duplicates of preset %i..\n", preIdx+1);
	
	if ((as_getPreset(as_ctx, (void*)&preset, sizeof(preset), preIdx, 0) != 1) || !isValid(&preset)){
		printf("Preset %i not available\n", preIdx+1);
		return;
	}
	
	int ct = 0;
	as_preset_t hay;
	
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		if (i == preIdx) continue;
		if ((as_getPreset(as_ctx, (void*)&hay, sizeof(hay), i, 0) != 1) || !isValid(&hay))
			continue;
			
		if (isEqual(&preset, &hay)){
			printf("%i <> %i: identical\n", preIdx+1, i+1);
			ct++;
		}
	}
	
	if (!ct)
		printf("Preset %i not duplicated\n", preIdx+1);
	printf("done\n");
}

void cmd_finddups (as_t *as_ctx, const int arg)
{
	
	printf("Searching for duplicates..\n");

	
	int ct = 0;
	as_preset_t hay[AS_PRESET_TOTAL];
	
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		if ((as_getPreset(as_ctx, (void*)&hay[i], sizeof(as_preset_t), i, 0) != 1) || !isValid(&hay[i]))
			continue;
	}

	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		as_preset_t *preset = &hay[i];
		if (!isValid(preset)) continue;
		
		for (int j = 0; j < AS_PRESET_TOTAL; j++){
			if (j == i) continue;

			if (isEqual(preset, &hay[j])){
				printf("%i <> %i: identical\n", i+1, j+1);
				ct++;
			}
		}
	}

	if (!ct)
		printf("No duplicates found\n");
	else
		printf("Duplicates found\n");
}

void doCmdLine (as_t *as_ctx, int argc, char *argv[])
{
	int argIdx = 1;
	
	do{
	do{
		char *cmd = argv[argIdx];

		if (!strcmp(cmd, "--listctrls")){
			cmd_listCtrls(as_ctx);

		}else if (!strcmp(cmd, "--finddups")){
			cmd_finddups(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--isdup")){
			int preIdxFrom = -1;
			int preIdxTo = -1;
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%i,%i", &preIdxFrom, &preIdxTo) != 2)
				break;
			else
				argIdx++;

			if (preIdxFrom < 1 || preIdxFrom > AS_PRESET_TOTAL) break;
			if (preIdxTo < 1 || preIdxTo > AS_PRESET_TOTAL) break;
			
			cmd_isdup(as_ctx, preIdxFrom-1, preIdxTo-1);

		}else if (!strcmp(cmd, "--finddup")){
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;
			}
			if (presetNum > 0 && presetNum <= AS_PRESET_TOTAL)
				cmd_finddup(as_ctx, presetNum-1);
			
		}else if (!strcmp(cmd, "--copy")){
			int preIdxFrom = -1;
			int preIdxTo = -1;
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%i,%i", &preIdxFrom, &preIdxTo) != 2)
				break;
			else
				argIdx++;

			if (preIdxFrom < 1 || preIdxFrom > AS_PRESET_TOTAL) break;
			if (preIdxTo < 1 || preIdxTo > AS_PRESET_TOTAL) break;
			
			cmd_copy(as_ctx, preIdxFrom-1, preIdxTo-1);
			
		}else if (!strcmp(cmd, "--move")){
			int preIdxFrom = -1;
			int preIdxTo = -1;
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%i,%i", &preIdxFrom, &preIdxTo) != 2)
				break;
			else
				argIdx++;
			
			if (preIdxFrom < 1 || preIdxFrom > AS_PRESET_TOTAL) break;
			if (preIdxTo < 1 || preIdxTo > AS_PRESET_TOTAL) break;
			cmd_move(as_ctx, preIdxFrom-1, preIdxTo-1);
			
		}else if (!strcmp(cmd, "--delete") | !strcmp(cmd, "--erase")){
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;
			}
			if (presetNum > 0 && presetNum <= AS_PRESET_TOTAL)
				cmd_delete(as_ctx, presetNum-1);
			
		}else if (!strcmp(cmd, "--osbfexport")){
			cmd_exportOSBF(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--osbfimport")){
			if (argIdx+1 >= argc) break;
			char *path = argv[argIdx+1];
			if (!strncmp(path, "--", 2) ) break;

			cmd_importOSBF(as_ctx, path);

			argIdx++;
		}else if (!strcmp(cmd, "--dumpeeprom")){
			cmd_dumpEEPROM(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--importstore")){
			if (argIdx+1 >= argc) break;
			char *path = argv[argIdx+1];
			if (path[0] == '-') break;

			argIdx++;
			if (argIdx+1 >= argc) break;
			
			char *arg = argv[argIdx+1];
			int presetIdx = -1;
			if (!sscanf(arg, "%i", &presetIdx)) break;
			
			if (presetIdx >= 1 && presetIdx <= AS_PRESET_TOTAL)
				cmd_importPresetAndStore(as_ctx, path, presetIdx-1);

			argIdx++;
		}else if (!strcmp(cmd, "--importsave")){
			if (argIdx+1 >= argc) break;
			char *path = argv[argIdx+1];
			if (path[0] == '-') break;

			argIdx++;
			if (argIdx+1 >= argc) break;
			
			char *arg = argv[argIdx+1];
			int presetIdx = -1;
			if (!sscanf(arg, "%i", &presetIdx)) break;
			
			if (presetIdx >= 1 && presetIdx <= AS_PRESET_TOTAL)
				cmd_importPresetAndSave(as_ctx, path, presetIdx-1);

			argIdx++;
		}else if (!strcmp(cmd, "--ctrls")){
			cmd_ctrls(as_ctx);
			
		}else if (!strcmp(cmd, "--getctrl")){
			if (argIdx+1 >= argc) break;
			char *ctrl = argv[argIdx+1];
			if (ctrl[0] == '-') break;

			cmd_getCtrlValue(as_ctx, ctrl);

			argIdx++;
		}else if (!strcmp(cmd, "--setctrl")){
			if (argIdx+1 >= argc) break;
			char *ctrl = argv[argIdx+1];
			if (ctrl[0] == '-') break;

			argIdx++;
			if (argIdx+1 >= argc) break;
			
			char *arg = argv[argIdx+1];
			int value = -1;
			if (!sscanf(arg, "%i", &value)) break;
			
			if (value >= 0 && value <= 255)
				cmd_setCtrlValue(as_ctx, ctrl, value);

			argIdx++;
		}else if (!strcmp(cmd, "--presets")){
			int preIdxFrom = -1;
			int preIdxTo = -1;
			if (argIdx+1 >= argc) goto exit;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%i,%i", &preIdxFrom, &preIdxTo) != 2)
				goto exit;
			else
				argIdx++;

			if (preIdxFrom < 1 || preIdxFrom > AS_PRESET_TOTAL) break;
			if (preIdxTo < 1 || preIdxTo > AS_PRESET_TOTAL) break;

			exit:
			if ((preIdxFrom <= preIdxTo) && preIdxTo >= 0)
				cmd_retrievePresetNameRange(as_ctx, preIdxFrom-1, preIdxTo-1);
			else
				cmd_retrievePresetNameAll(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--getname")){
			if (argIdx+1 >= argc) break;
			
			char *arg = argv[argIdx+1];
			int presetNum = -1;
			if (!sscanf(arg, "%i", &presetNum)) break;
			
			if (presetNum >= 1 && presetNum <= AS_PRESET_TOTAL)
				cmd_retrievePresetName(as_ctx, presetNum-1);
				
			argIdx++;
		}else if (!strcmp(cmd, "--select")){
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			int presetNum = -1;
			if (!sscanf(arg, "%i", &presetNum)) break;
			
			if (presetNum >= 1 && presetNum <= AS_PRESET_TOTAL)
				cmd_setPresetActive(as_ctx, presetNum-1);

			argIdx++;
		}else if (!strcmp(cmd, "--getactive") || !strcmp(cmd, "--working")){
			cmd_retrievePresetActive(as_ctx, 0);
		
		}else if (!strcmp(cmd, "--activename")){
			cmd_retrievePresetActiveAndName(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--hwcfg")){
			cmd_retrieveHardwareConfig(as_ctx, 0);
		
		}else if (!strcmp(cmd, "--next")){
			cmd_setPresetNext(as_ctx, 0);
	
		}else if (!strcmp(cmd, "--previous")){
			cmd_setPresetPrevious(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--reqlist")){
			int from = 1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (arg[0] != '-'){
					if (!sscanf(arg, "%i", &from))
						break;
					else
						argIdx++;
				}
			}
			if (from < 1 || from > MAX_AVAILABLE_PRESETS) break;
			cmd_jsonListAvailable(as_ctx, from-1);

		}else if (!strcmp(cmd, "--reqavailable")){
			cmd_jsonGetTotalAvailable(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--reqstore")){
			int presetIdxReq = -1;
			int presetIdxDes = -1;
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%i,%i", &presetIdxReq, &presetIdxDes) != 2)
				break;
			else
				argIdx++;
			
			if (presetIdxReq < 1 || presetIdxReq > MAX_AVAILABLE_PRESETS) break;
			if (presetIdxDes < 1 || presetIdxDes > AS_PRESET_TOTAL) break;

			cmd_jsonSendToPedalStore(as_ctx, presetIdxReq-1, presetIdxDes-1);
			
		}else if (!strcmp(cmd, "--reqsave")){
			int presetIdxReq = -1;
			int presetIdxDes = -1;
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%i,%i", &presetIdxReq, &presetIdxDes) != 2)
				break;
			else
				argIdx++;
			
			if (presetIdxReq < 1 || presetIdxReq > MAX_AVAILABLE_PRESETS) break;
			if (presetIdxDes < 1 || presetIdxDes > AS_PRESET_TOTAL) break;

			cmd_jsonSendToPedalSave(as_ctx, presetIdxReq-1, presetIdxDes-1);
			
		}else if (!strcmp(cmd, "--reqpreset")){		
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;
			}
			if (presetNum < 1 || presetNum > MAX_AVAILABLE_PRESETS) break;
				cmd_jsonGetPresetPre(as_ctx, presetNum-1);
					
		}else if (!strcmp(cmd, "--reqjson")){		
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;
			}
			if (presetNum < 1 || presetNum > MAX_AVAILABLE_PRESETS) break;			
				cmd_jsonGetPresets(as_ctx, presetNum-1, 1);	

		}else if (!strcmp(cmd, "--preset")){		
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;
			}
			if (presetNum < 1 || presetNum > AS_PRESET_TOTAL)
				break;
			cmd_retrievePresetValues(as_ctx, presetNum-1);
				
		}else if (!strcmp(cmd, "--bin")){		
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;
			}
			if (presetNum < 1 || presetNum > AS_PRESET_TOTAL)
				break;
			cmd_retrievePresetBin(as_ctx, presetNum-1);

		}else if (!strcmp(cmd, "--getxml")){		
			int presetNum = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;

				if (presetNum < 1 || presetNum > AS_PRESET_TOTAL)
					break;
				cmd_retrievePresetPre(as_ctx, presetNum-1);
			}
		}else if (!strcmp(cmd, "--getxmlall")){
			cmd_retrievePresetPreAll(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--getflash")){
			int address = 0;
			int length = 0;
			if (argIdx+1 >= argc) break;

			char *arg = argv[argIdx+1];
			if (sscanf(arg, "%x,%i", &address, &length) != 2)
				break;
			else
				argIdx++;

			cmd_getFlash(as_ctx, address, length);
			
		}else if (!strcmp(cmd, "--dumpflash")){
			cmd_dumpFlash(as_ctx, 0);

		}else if (!strcmp(cmd, "--saveas")){
			int presetNum = 1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;

				if (presetNum < 1 || presetNum > AS_PRESET_TOTAL)
					break;
					
				if (argIdx+1 < argc){
					char *name = argv[argIdx+1];
					if (!strncmp("--", name, 2))
						break;

					cmd_saveas(as_ctx, name, presetNum-1);
					
					argIdx++;
				}else{
					break;
				}
			}			
		}else if (!strcmp(cmd, "--rename")){
			int presetNum = 0;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &presetNum))
					break;
				else
					argIdx++;

				if (presetNum < 1 || presetNum > AS_PRESET_TOTAL)
					break;
					
				if (argIdx+1 < argc){
					char *name = argv[argIdx+1];
					if (!strncmp("--", name, 2))
						break;

					cmd_rename(as_ctx, name, presetNum-1);
					argIdx++;
				}else{
					break;
				}
			}
		}else if (!strcmp(cmd, "--help")){
			cmd_printHelp();
		
		}else if (!strcmp(cmd, "--lookup")){
			cmd_lookup(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--version")){
			cmd_printVersion(as_ctx, 0);
		
		}else if (!strcmp(cmd, "--pause")){
			cmd_pause(as_ctx, 0);
			
		}else if (!strcmp(cmd, "--wait")){
			int waitMs = -1;
			if (argIdx+1 < argc){
				char *arg = argv[argIdx+1];
				if (!sscanf(arg, "%i", &waitMs))
					break;
				else
					argIdx++;
			}
			if (waitMs > 0)
				cmd_wait(as_ctx, waitMs);
		}
	}while(++argIdx < argc);
	}while(++argIdx < argc);
}

int main (int argc, char *argv[])
{
	if (sizeof(as_preset_t) != 164){
		printf("sanity check: sizeof(as_preset_t) != 164\n");
		return EXIT_FAILURE;
	}

	as_t as_ctx = {0};
	if (as_openPedal(&as_ctx, AS_PRODUCT_C4_SYNTH) != 1){
		printf("emotional damage: pedal not found\n");
		cmd_printHelp();
		return EXIT_FAILURE;
	}

	if (!as_getVersion(&as_ctx)){
		printf("Unable to communicate with pedal\n");
		as_closePedal(&as_ctx);
		return EXIT_FAILURE;
	}


#if 1
	if (argc > 1)
		doCmdLine(&as_ctx, argc, argv);
	else
		cmd_printHelp();
#endif

	as_closePedal(&as_ctx);
	return EXIT_SUCCESS;
}


