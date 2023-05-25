
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "hidapi.h"
#include <inttypes.h>

#include "../as.h"
#include "../xpre.h"
#include "../jas.h"


// dumpFunction will get and print requested data
// getFunction will get and return requested data. buffer (if applicable) is handled by caller





static void dumpHex (const unsigned char *data, size_t length)
{
	for (int i = 0; i < length; i++){
		printf("%.2X ", data[i]);
	}
	printf("\n");
}

void dumpNum (const unsigned char *data, size_t length)
{
	for (int i = 0; i < length; i++){
		printf("%i ", data[i]);
	}
	printf("\n");
}

void dumpASCII (const unsigned char *data, size_t length)
{
	for (int i = 0; i < length; i++){
		if (data[i] > 31 && data[i] < 186)
			printf("%c ", data[i]);
		else
			printf("  ");
	}
	printf("\n");
}

void setWorkingPreset (hid_device *device, const int preset)
{
	const int presetIndex = preset&0x7F;
	uint8_t buffer[AS_PACKET_LENGTH] = {0};

	buffer[0] = 0x00;
	buffer[1] = AS_CMD_ACTIVE_SET;
	buffer[2] = presetIndex;
	buffer[3] = 0x00;
	hid_write(device, buffer, AS_PACKET_LENGTH);
}

int getPresetName (hid_device *device, const int preset, char *name)
{

	uint8_t buffer[AS_PACKET_LENGTH] = {0};

	const int presetIndex = preset&0x7F;
	uint32_t addr = AS_PRESET_ADDRESS_BASE_1 + (presetIndex * AS_PRESET_ADDRESS_LENGTH) + AS_PRESET_ADDRESS_NAME;
	
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_PRESET_GET;
	buffer[2] = (addr>>16)&0xFF;
	buffer[3] = (addr>>8)&0xFF;
	buffer[4] = addr&0xFF;
	buffer[5] = 0x00;		

	hid_write(device, buffer, AS_PACKET_LENGTH);
	int readlen = hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);
	if (readlen != AS_REPORT_LENGTH || buffer[0] != AS_CMD_PRESET_GET || buffer[1] == 0xFF)
		return 0;

	buffer[AS_PRESET_NAME_LENGTH] = 0;
	strncpy(name, &buffer[1], AS_PRESET_NAME_LENGTH);
	return 1;
}
           
int getHardwareConfig (hid_device *device, as_hw_config_t *cfg)
{           
	uint8_t buffer[AS_PACKET_LENGTH] = {0};
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_CONFIG_GET;
            
	hid_write(device, buffer, AS_PACKET_LENGTH);
	int readlen = hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);
	if (readlen != AS_REPORT_LENGTH || buffer[0] != 0x32)
		return 0;

	memcpy(cfg, &buffer[1], sizeof(*cfg));
	return 1;
}
	
void dumpHardwareConfig (hid_device *device)
{
	as_hw_config_t cfg;
	if (!getHardwareConfig(device, &cfg))
		return;
	
	printf("Firmware: %i.%i\n", cfg.firmwareVersion&0xFF, cfg.firmwareVersion>>8);
	printf("Device model: %i\n", cfg.deviceModel);
	printf("Number of presets: %i\n", cfg.numberOfPresets);
	printf("Active preset: %i\n", cfg.activePreset+1);
	printf("Preset/WYSIWYG: %i\n", cfg.WYSIWYG);
	printf("Hardware bypass mode: %i\n", cfg.hardwareBypassMode);
	printf("Midi channel (1 to 16): %i \n", cfg.midiChannel+1);
	printf("Ccontrol input option: %i\n", cfg.controlInputOption);
	printf("External loop safe mode: %i\n", cfg.externalLoopSafeMode);
	printf("Power up state: %i\n", cfg.powerUpState);
	printf("Default input routing: %i\n", cfg.defaultInputRouting);
	printf("Quick bank select: %i\n", cfg.quickBankSelect);
	printf("Neuro hub pedel input calibration Min: %i\n", cfg.hubPedelCalMin);
	printf("Neuro hub pedel input calibration Range: %i\n", cfg.hubPedelCalRange);
	printf("Control input calibration Min: %i\n", cfg.inputCalMin);
	printf("control input calibration Range: %i\n", cfg.inputCalRange);
	printf("USB-Midi skip power check: %i\n", cfg.usbMidiSkipPowerCheck);

}
	
void dumpPresetNames (hid_device *device)
{
	uint8_t buffer[AS_PACKET_LENGTH] = {0};
	int altIndex = 0;
					
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		memset(buffer, 0, sizeof(buffer));
		uint32_t addr = AS_PRESET_ADDRESS_BASE_1 + (i * AS_PRESET_ADDRESS_LENGTH) + AS_PRESET_ADDRESS_NAME;
		buffer[0] = 0x00;
		buffer[1] = AS_CMD_PRESET_GET;
		buffer[2] = (addr>>16)&0xFF;
		buffer[3] = (addr>>8)&0xFF;
		buffer[4] = addr&0xFF;
		buffer[5] = 0x00;		

		hid_write(device, buffer, AS_PACKET_LENGTH);
		int readlen = hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);
		if (readlen != AS_REPORT_LENGTH || buffer[0] != AS_CMD_PRESET_GET)
			continue;

		if (buffer[1] == 0xFF){
#if 1
			memset(buffer, 32, sizeof(buffer));
			strcpy(buffer, " --empty--");
			buffer[strlen(buffer)] = 32;
#else
			memset(buffer, 0, sizeof(buffer));
			addr = AS_PRESET_ADDRESS_BASE_2 + (altIndex * AS_PRESET_ADDRESS_LENGTH) + AS_PRESET_ADDRESS_NAME;
			
			buffer[0] = 0x00;
			buffer[1] = AS_CMD_PRESET_GET;
			buffer[2] = (addr>>16)&0xFF;
			buffer[3] = (addr>>8)&0xFF;
			buffer[4] = addr&0xFF;
			buffer[5] = 0x00;
			
			Sleep(1);
			hid_write(device, buffer, AS_PACKET_LENGTH);
			hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);
			if (buffer[0] != AS_CMD_PRESET_GET) continue;
			
			altIndex++;
#endif
		}

		buffer[AS_PRESET_NAME_LENGTH+1] = 0;
		printf("%.3i: '%s'\n", i+1, &buffer[1]);
	}
}

void dumpAllPresets (hid_device *device)
{
	printf("\n");
	uint8_t buffer[AS_PACKET_LENGTH] = {0};
							
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		printf("%i:\n", i+1);
		
		for (int idx = 1; idx < 6; idx++){
			memset(buffer, 0, sizeof(buffer));

			uint32_t addr = AS_PRESET_ADDRESS_BASE_1 + (i * AS_PRESET_ADDRESS_LENGTH) + (idx * AS_DATA_LENGTH);
			buffer[0] = 0x00;
			buffer[1] = AS_CMD_PRESET_GET;
			buffer[2] = (addr>>16)&0xFF;
			buffer[3] = (addr>>8)&0xFF;
			buffer[4] = addr&0xFF;
			buffer[5] = 0x00;		
	
			hid_write(device, buffer, AS_PACKET_LENGTH);
		
			memset(buffer, 0, sizeof(buffer));
			hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);

			if (idx != 5){
				dumpHex(&buffer[1], AS_DATA_LENGTH);
			}else{
				buffer[AS_PRESET_NAME_LENGTH+1] = 0;
				printf("'%s'\n", &buffer[1]);
			}
		}
		printf("\n");
	}
}

void dumpPresetBin (hid_device *device, const int preset, int includeName)
{
	printf("\n");
	uint8_t buffer[AS_PACKET_LENGTH] = {0};

	includeName &= 0x01;
				
	for (int idx = 1; idx < 5+includeName; idx++){
		memset(buffer, 0, sizeof(buffer));

		uint32_t addr = AS_PRESET_ADDRESS_BASE_1 + (preset * AS_PRESET_ADDRESS_LENGTH) + (idx * AS_DATA_LENGTH);
		buffer[0] = 0x00;
		buffer[1] = AS_CMD_PRESET_GET;
		buffer[2] = (addr>>16)&0xFF;
		buffer[3] = (addr>>8)&0xFF;
		buffer[4] = addr&0xFF;
		buffer[5] = 0x00;		
		hid_write(device, buffer, AS_PACKET_LENGTH);
		
		memset(buffer, 0, sizeof(buffer));
		hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);

		if (idx != 5){
			dumpHex(&buffer[1], AS_DATA_LENGTH);
		}else{
			buffer[AS_PRESET_NAME_LENGTH+1] = 0;
			printf("'%s'\n", &buffer[1]);
		}
	}
	printf("\n");

}

void dumpRAM (hid_device *device)
{
	printf("\n");
	uint8_t buffer[AS_PACKET_LENGTH] = {0};
				
	const uint32_t startAdd = 0x00;
	const uint32_t endAdd = 0x0FFFFF;

	for (int i = startAdd; i < endAdd; i += AS_DATA_LENGTH){
		memset(buffer, 0, sizeof(buffer));

		uint32_t addr = i;
		buffer[0] = 0x00;
		buffer[1] = AS_CMD_PRESET_GET;
		buffer[2] = (addr>>16)&0xFF;
		buffer[3] = (addr>>8)&0xFF;
		buffer[4] = addr&0xFF;
		buffer[5] = 0x00;		
		hid_write(device, buffer, AS_PACKET_LENGTH);
		
		memset(buffer, 0, sizeof(buffer));
		hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);
		//Sleep(1);

		printf("%.6X:  ", i);
		//dumpHex(&buffer[1], AS_DATA_LENGTH);
		dumpASCII(&buffer[1], AS_DATA_LENGTH);
	}
	printf("\n");
}


// retrieve preset from device
int getPreset (hid_device *device, uint8_t *buffer, const int bufferLen, int preset, int includeName)
{
	includeName &= 0x01;
	preset &= 0x7F;

	memset(buffer, 0, bufferLen);
	uint8_t writerBuffer[AS_PACKET_LENGTH] = {0};
	
	for (int idx = 1; idx < 5+includeName; idx++){
		uint32_t addr = AS_PRESET_ADDRESS_BASE_1 + (preset * AS_PRESET_ADDRESS_LENGTH) + (idx * AS_DATA_LENGTH);

		writerBuffer[0] = 0x00;
		writerBuffer[1] = AS_CMD_PRESET_GET;
		writerBuffer[2] = (addr>>16)&0xFF;
		writerBuffer[3] = (addr>>8)&0xFF;
		writerBuffer[4] = addr&0xFF;
		writerBuffer[5] = 0x00;		
		hid_write(device, writerBuffer, AS_PACKET_LENGTH);

		writerBuffer[0] = 0;
		int readlen = hid_read_timeout(device, writerBuffer, AS_PACKET_LENGTH, 1000);
		if (readlen != AS_REPORT_LENGTH || writerBuffer[0] != AS_CMD_PRESET_GET)
			return 0;
		memcpy(&buffer[(idx-1)*AS_DATA_LENGTH], &writerBuffer[1], AS_DATA_LENGTH);
	}
	return 1;
}

void dumpPresetStruct (as_preset_t *preset)
{	
	printf(" Level:\n");
	printf("input1_gain    %i\n", preset->level.input1_gain);
	printf("input2_gain    %i\n", preset->level.input2_gain);
	printf("master_depth   %i\n", preset->level.master_depth);
	printf("mod_source     %i\n", preset->level.mod_source);
	printf("bass           %i\n", preset->level.bass);
	printf("treble         %i\n", preset->level.treble);
	printf("mix            %i\n", preset->level.mix);
	printf("lo_retain      %i\n", preset->level.lo_retain);
	printf("output         %i\n", preset->level.output);
	printf("output_balance %i\n", preset->level.output_balance);
	printf("\n");
	
	for (int i = 0; i < AS_VOICE_TOTAL; i++){
		printf(" Voice %i:\n", i+1);
		printf("level           %i\n", preset->voice[i].level);
		printf("pan             %i\n", preset->voice[i].pan);           
		printf("detune          %i\n", preset->voice[i].detune);        
		printf("tremolo         %i\n", preset->voice[i].tremolo);       
		printf("semitone        %i\n", preset->voice[i].semitone);      
		printf("octave          %i\n", preset->voice[i].octave);        
		printf("envelope        %i\n", preset->voice[i].envelope);      
		printf("source          %i\n", preset->voice[i].source);        
		printf("mode            %i\n", preset->voice[i].mode);          
		printf("enable          %i\n", preset->voice[i].enable);        
		printf("modulate        %i\n", preset->voice[i].modulate);      
		printf("tremolo_source  %i\n", preset->voice[i].tremolo_source);
		printf("destination     %i\n", preset->voice[i].destination);
		printf("\n");
	}

	for (int i = 0; i < AS_FILTER_TOTAL; i++){
		printf(" Filter %i:\n", i+1);
		printf("depth            %i\n", preset->filter[i].depth);
		printf("frequency        %i\n", preset->filter[i].frequency);
		printf("q                %i\n", preset->filter[i].q);
		printf("type             %i\n", preset->filter[i].type);
		printf("envelope         %i\n", preset->filter[i].envelope);
		printf("invert	         %i\n", preset->filter[i].invert);
		printf("enable	         %i\n", preset->filter[i].enable);
		printf("pitch_track      %i\n", preset->filter[i].pitch_track);
		printf("mix_destination  %i\n", preset->filter[i].mix_destination);
		printf("mix_enable       %i\n", preset->filter[i].mix_enable);
		printf("\n");
	}
	
	for (int i = 0; i < AS_ENVELOPE_TOTAL; i++){
		printf(" Envelope %i:\n", i+1);
		printf("sensitivity     %i\n", preset->envelope[i].sensitivity);
		printf("speed           %i\n", preset->envelope[i].speed);
		printf("gate            %i\n", preset->envelope[i].gate);
		printf("type;           %i\n", preset->envelope[i].type);
		printf("input           %i\n", preset->envelope[i].input);      
		printf("\n");
	}

	printf(" Distortion:\n");
	printf("drive              %i\n", preset->distortion.drive); 
	printf("mix                %i\n", preset->distortion.mix);
	printf("output             %i\n", preset->distortion.output);
	printf("type               %i\n", preset->distortion.type);
	printf("enable             %i\n", preset->distortion.enable);
	printf("\n");
	
	printf("fm_sine1           %i\n", preset->fm_sine1);
	printf("fm_sine2           %i\n", preset->fm_sine2);
	printf("fm_sine1_input     %i\n", preset->fm_sine1_input);
	printf("fm_sine2_input     %i\n", preset->fm_sine2_input);
	printf("mono_pitch_filter1 %i\n", preset->mono_pitch_filter1);
	printf("mono_pitch_filter2 %i\n", preset->mono_pitch_filter2);
	printf("\n");


	printf(" LFO:\n");
	printf("speed              %i\n", preset->lfo.speed);
	printf("env_to_speed       %i\n", preset->lfo.env_to_speed);
	printf("env_to_depth       %i\n", preset->lfo.env_to_depth);
	printf("to_phase           %i\n", preset->lfo.to_phase);
	printf("to_multiply        %i\n", preset->lfo.to_multiply);
	printf("shape              %i\n", preset->lfo.shape);
	printf("restart            %i\n", preset->lfo.restart);
	printf("beat_division      %i\n", preset->lfo.beat_division);
	printf("tempo              %i\n", preset->lfo.tempo);
	printf("\n");
	
	for (int i = 0; i < AS_SEQUENCER_TOTAL; i++){
		printf(" Sequencer %i:\n", i+1);
		printf("steps           %i\n", preset->sequencer[i].steps+2);		// don't know why +2 must be added, but it's needed
		
		for (int j = 0; j < preset->sequencer[i].steps+2; j++)
			printf("value %i   \t%i\n", j+1, preset->sequencer[i].value[j]);
		printf("\n");
	}

	printf(" Harmony:\n");
	printf("tuning          %i\n", preset->harmony.tuning);
	printf("key             %i\n", preset->harmony.key);
	printf("interval1       %i\n", preset->harmony.interval1);
	printf("mode            %i\n", preset->harmony.mode);
	printf("interval2       %i\n", preset->harmony.interval2);
	printf("\n");


	printf(" Pitch detect:\n");
	printf("input           %i\n", preset->pitchdetect.input);
	printf("mode            %i\n", preset->pitchdetect.mode);
	printf("low_note        %i\n", preset->pitchdetect.low_note);
	printf("high_note       %i\n", preset->pitchdetect.high_note);
	printf("\n");

	printf(" General:\n");
	printf("knob1_assign         %i\n", preset->knob1_assign);
	printf("knob2_assign         %i\n", preset->knob2_assign);
	printf("routing_option       %i\n", preset->routing_option);
	printf("filter2_correction   %i\n", preset->filter2_correction);
	printf("on_off_status        %i\n", preset->on_off_status);
	printf("ext_control_enable   %i\n", preset->ext_control_enable);
	printf("lfo_midi_clock_sync  %i\n", preset->lfo_midi_clock_sync);
	printf("\n");

	for (int i = 0; i < AS_EXT_TOTAL; i++){
		printf(" Ext: %i\n", i+1);
		printf("destination  %i\n", preset->ext[i].destination);
		printf("source       %i\n", preset->ext[i].source);
		printf("min          %i\n", preset->ext[i].min);
		printf("max          %i\n", preset->ext[i].max);
		printf("\n");
	}
	
	printf("Name: %s\n", preset->name);
}

void finds (char *bufferXml)
{
	char buffer[256] = {0};
	
	char *found = strchr(bufferXml, '<');
	while (found){
		found++;
		if (*found != '/'){
			char *to = strchr(found, '>');
			
			int len = (to - found);
			memcpy(buffer, found, len);
			buffer[len] = 0;
		
			//printf("%s\n", buffer);
			//printf("\tbuffer[pos++] = xpre_getInt32(xpre, \"%s\");\n", buffer);
			printf("\txpre_add(xpre, \"%s\", pre->__);\n", buffer);

			found += len + 1;
		}
		found = strchr(found, '<');
	};
}

void preset2xpre (hid_device *device, const int presetIdx)
{
	as_preset_t preset = {0};
	getPreset(device, (void*)&preset, sizeof(preset), presetIdx, 1);
	
	int bufferLen = 512*1024;			// a C4 present is ~6.7K. 512k should never be not enough.
	char *buffer = calloc(1, bufferLen);
	if (!buffer){
		printf("emotional damage\n");
		return;
	}
	
	xpre_t xpre;
	xpre_init(&xpre);
	xpre_prepareBuffer(&xpre, buffer, bufferLen);
	xpre_bin2string(&xpre, &preset, sizeof(preset));
	
	printf("\n%s\n", xpre.buffer);
}

void dumpAllPresets2Xml (hid_device *device)
{
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		preset2xpre(device, i);
		printf("\n\n\n");
	}
}

void importPresetAndSave (hid_device *device, char *path, const int presetDestIdx)
{

	xpre_t xpre;
	xpre_init(&xpre);
	xpre_preparePath(&xpre, path);
	
	uint8_t buffer[256] = {0};
	int len = xpre_string2bin(&xpre, buffer, XPRE_FORMAT_SAVE, presetDestIdx&0x7F);
	
	// still won't save if preset_name is missing		<-----
	
	for (int i = 0; i < len; i += AS_PACKET_LENGTH){
		hid_write(device, &buffer[i], AS_PACKET_LENGTH);
		Sleep(1);
	}

	xpre_cleanup(&xpre);
}

void importPresetDontSave (hid_device *device, char *path)
{

	xpre_t xpre;
	xpre_init(&xpre);
	xpre_preparePath(&xpre, path);
	
	uint8_t buffer[256] = {0};
	int len = xpre_string2bin(&xpre, buffer, XPRE_FORMAT_STORE, 0);
	
	for (int i = 0; i < len; i += AS_PACKET_LENGTH){
		hid_write(device, &buffer[i], AS_PACKET_LENGTH);
		Sleep(1);
	}

	xpre_cleanup(&xpre);
}

void setControlValue (hid_device *device, const int ctrl, const int value)
{
	uint8_t buffer[AS_PACKET_LENGTH] = {0};
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_CTRL_SET;
	buffer[2] = ctrl;
	buffer[3] = (value>>8)&0xFF;
	buffer[4] = value&0xFF;
	
	hid_write(device, buffer, AS_PACKET_LENGTH);
}

int getControlValues (hid_device *device, uint8_t *values, const size_t length)
{
	uint8_t buffer[AS_PACKET_LENGTH] = {0};
	
	for (int i = 0; i < AS_PRESET_LENGTH; i += AS_DATA_LENGTH){
		buffer[0] = 0x00;
		buffer[1] = AS_CMD_CTRL_GET;
		buffer[2] = 0;
		buffer[3] = i;
		buffer[4] = AS_DATA_LENGTH;
	
		hid_write(device, buffer, AS_PACKET_LENGTH);
		
		memset(buffer, 0, sizeof(buffer));
		int readlen = hid_read_timeout(device, buffer, AS_PACKET_LENGTH, 1000);
		if (readlen != AS_REPORT_LENGTH || buffer[0] != AS_CMD_CTRL_GET)
			return 0;
		memcpy(values, &buffer[1], AS_DATA_LENGTH);
		values += AS_DATA_LENGTH;
	}
	return 1;
}

void dumpControlValues (hid_device *device)
{
	const int rows = AS_PRESET_LENGTH / AS_DATA_LENGTH;
	uint8_t buffer[rows][AS_DATA_LENGTH];
	
	if (getControlValues(device, (void*)buffer, sizeof(buffer))){
		for (int i = 0; i < rows; i++)
			dumpHex(buffer[i], AS_DATA_LENGTH);
	}
}

void hidtest (hid_device *device)
{

	// dumpAllPresets(device);
	// dumpHardwareConfig(device);
	// dumpPreset(device, 0, 0);
	 dumpPresetNames(device);
	
	/*as_preset_t preset = {0};
	getPreset(device, (void*)&preset, sizeof(preset), 1, 1);
	dumpPresetStruct(&preset);*/
	
	// preset2xpre(device, 0);
	// dumpRAM(device);
	// setControlValue(device, 1, 100);
	
	// dumpPresetBin(device, 5, 1);
	// setWorkingPreset(device, 5);
	
	
	// dumpControlValues(device);
}


int main (int argc, char *argv[])
{   
	if (hid_init()){
		printf("hid_init() failed\n");
		return 0;
	}else if (sizeof(as_preset_t) != 164){
		printf("emotional times\n");
		return 0;
	}

	struct hid_device_info *deviceInfo = hid_enumerate(AS_VID, AS_PID_C4SYNTH);

	if (deviceInfo){
		hid_device *device = hid_open_path(deviceInfo->path);
		if (device){
			wchar_t product[64];
			if (!hid_get_product_string(device, product, sizeof(product)))
				wprintf(L"Found: 0x%X 0x%X, %s\n\n", deviceInfo->vendor_id, deviceInfo->product_id, product);

			hid_free_enumeration(deviceInfo);

			if (argc > 1 && isdigit(argv[1][0])){
				int preset = atoi(argv[1]);
				if (preset >= 1 && preset <= AS_PRESET_TOTAL){
					printf("Setting preset: %i\n", preset);
					setWorkingPreset(device, preset-1);
					
					uint8_t buffer[64];
					if (getPresetName(device, preset-1, buffer))
						printf(": %s\n", buffer);
					
				}
			}else{
				hidtest(device);
				
				//if (argc > 1)
				//	importPresetAndSave(device, argv[1], presetIdx);
				
				//if (argc > 1)
				//	importPresetDontSave(device, argv[1]);
				
			}
		}

		hid_close(device);
	}

	hid_exit();
	return 1;

}
