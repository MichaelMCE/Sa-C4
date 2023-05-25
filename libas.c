


#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <windows.h>		// for Sleep()

#include "hidapi/hidapi.h"
#include "xpre.h"
#include "jas.h"
#include "libas.h"









as_product_t as_productLookup (const uint16_t as_productId)
{
	as_product_t ret = {0};
	
	for (int i = 0; as_product_list[i].id; i++){
		if (as_product_list[i].id == as_productId){
			ret = as_product_list[i];
		}
	}
	return ret;
}

int as_openPedal (as_t *as_ctx, const uint16_t as_productid)
{

	if (hid_init()){
		printf("as_openPedal(): hid_init() failed\n");
		return -1;
	}

	as_ctx->device = NULL;
	as_product_t asProductIdent = as_productLookup(as_productid);

	if (!asProductIdent.id){
		printf("as_openPedal(): unknown product ident: %i\n", as_productid);
		return -2;
	}

	
	struct hid_device_info *deviceInfo = hid_enumerate(AS_VID, asProductIdent.PID);
	if (deviceInfo)
		as_ctx->device = hid_open_path(deviceInfo->path);

	hid_free_enumeration(deviceInfo);

	if (as_ctx->device){
		uint8_t writerBuffer[AS_PACKET_LENGTH];
		hid_read_timeout(as_ctx->device, writerBuffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	}
	return (as_ctx->device != NULL);
}

void as_closePedal (as_t *as_ctx)
{
	if (as_ctx->device)
		hid_close(as_ctx->device);
	hid_exit();
}

int as_write (as_t *as_ctx, const void *buffer, const int bLength)
{
	if (!as_ctx->device || !bLength) return -1;
	if (bLength%AS_PACKET_LENGTH){
		printf("as_write(): write length not a multiple of packet length\n");
		return -2;
	}

	char tmp[64];
	int writeTotal = 0;
	const uint8_t *buffer8 = buffer;
	
	for (int i = 0; i < bLength; i += AS_PACKET_LENGTH){
		if (hid_write(as_ctx->device, &buffer8[i], AS_PACKET_LENGTH) != AS_PACKET_LENGTH){
			return 0;
		}else{
			writeTotal += AS_PACKET_LENGTH;
			hid_read_timeout(as_ctx->device, (void*)tmp, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
		}
	}
	
	return writeTotal;
}

int as_read (as_t *as_ctx, void *buffer, const int bLength)
{
	if (!as_ctx->device) return -1;
	if (bLength < AS_PACKET_LENGTH) return -2;
	
	int readlen = hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	if (readlen != AS_REPORT_LENGTH)
		return 0;
	else
		return readlen;
}

/*
########################################################################################################
########################################################################################################
########################################################################################################
########################################################################################################
*/

int as_setPresetActive (as_t *as_ctx, const int presetIdx)
{
	if (!as_ctx->device) return 0;
	
	const uint8_t presetIndex = presetIdx&0x7F;
	uint8_t buffer[AS_PACKET_LENGTH];
	memset(buffer, 0, sizeof(buffer));

	buffer[0] = 0x00;
	buffer[1] = AS_CMD_ACTIVE_SET;
	buffer[2] = presetIndex;
	buffer[3] = 0x00;
	
	int ret = hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH);
	hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	return (ret == AS_PACKET_LENGTH);
}

int as_getHardwareConfig (as_t *as_ctx, as_hw_config_t *cfg)
{
	if (!as_ctx->device) return -1;
		
	uint8_t buffer[AS_PACKET_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_CONFIG_GET;
            
	
	if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
		return -2;
	int readlen = hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	if (readlen != AS_REPORT_LENGTH || buffer[0] != 0x32)
		return 0;

	memcpy(cfg, &buffer[1], sizeof(*cfg));
	return 1;
}

int as_getActivePreset (as_t *as_ctx)
{
	as_hw_config_t cfg;
	if (!as_getHardwareConfig(as_ctx, &cfg)){
		//printf("as_getHardwareConfig failed\n");
		return 0;
	}
	//printf("%i", cfg.activePreset+1);
	return cfg.activePreset;
}

int as_getVersion (as_t *as_ctx)
{
	as_hw_config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	
	if (!as_getHardwareConfig(as_ctx, &cfg)){
		//printf("as_getHardwareConfig failed\n");
		return 0;
	}
	//printf("%i", cfg.activePreset+1);
	return cfg.firmwareVersion;
}

int as_getControlValue (as_t *as_ctx, uint8_t ctrl, uint8_t *value)
{
	uint8_t buffer[AS_PACKET_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_CTRL_GET;
	buffer[2] = 0;
	buffer[3] = ctrl;
	buffer[4] = AS_PAYLOAD_LENGTH;
	
	if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
		return -2;
		
	memset(buffer, 0, sizeof(buffer));
	int readlen = hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	if (readlen != AS_REPORT_LENGTH || buffer[0] != AS_CMD_CTRL_GET)
		return 0;

	*value = buffer[1];
	return 1;
}

int as_setControlValue (as_t *as_ctx, uint8_t ctrl, uint16_t value)
{
	uint8_t buffer[AS_PACKET_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_CTRL_SET;
	buffer[2] = ctrl;
	buffer[3] = (value>>8)&0xFF;
	buffer[4] = value&0xFF;
	
	if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
		return -2;

	//hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	
	return 1;
}

/*

 0 56 196 0 16  235 60 0 197 150 20  119 32  110 103 2 0 0 0 0 240 162 143 1 0 0 0 0 0 0 0 0 0 0 0 0 228 24 1
 0 56 144 0 84  236 65 0 197 150 20  119 16  131 98  2 0 0 0 0 240 162 137 1 0 0 0 0 0 0 0 0 0 0 0 0 228 24 251
 0 56 134 0 208 232 20 0 197 150 198 117 128 119 51  2 0 0 0 0 240 162  94 1 0 0 0 0 0 0 0 0 0 0 0 0 228 24 208
 0 56 129 0 28  230 30 0 197 150 198 117 192 215 75  2 0 0 0 0 240 162 109 1 0 0 0 0 0 0 0 0 0 0 0 0 228 24 223
 0 56 135 0 136 229 21 0 197 150 198 117 16  175 115 2 0 0 0 0 240 162 149 1 0 0 0 0 0 0 0 0 0 0 0 0 228 24 7
 0 56 136 0 136 229 21 0 197 150 198 117 16  175 115 2 0 0 0 0 240 162 149 1 0 0 0 0 0 0 0 0 0 0 0 0 228 24 7

*/


int as_erase (as_t *as_ctx, const uint8_t presetIdx)
{
	uint8_t buffer[AS_PACKET_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	
	const int presetIndex = (presetIdx&0x7F) | 0x80;
	
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_PRESET_ERASE;
	buffer[2] = presetIndex;
	buffer[3] = 0;
	buffer[4] = 0;
	
	if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
		return -2;

	// check return for 0x37
	hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	
	return 1;
}

int as_getPresetName (as_t *as_ctx, const uint8_t presetIdx, char *name)
{
	if (!as_ctx->device) return -1;
	
	const int presetIndex = presetIdx&0x7F;
	const uint32_t addr = AS_PRESET_ADDRESS_BASE + (presetIndex * AS_PRESET_ADDRESS_LENGTH) + AS_PRESET_ADDRESS_NAME;
	
	uint8_t buffer[AS_PACKET_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	
	buffer[0] = 0x00;
	buffer[1] = AS_CMD_FLASH_READ;
	buffer[2] = (addr>>16)&0xFF;
	buffer[3] = (addr>>8)&0xFF;
	buffer[4] = addr&0xFF;
	buffer[5] = 0x00;		

	if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
		return -2;

	int readlen = hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
	if (readlen != AS_REPORT_LENGTH || buffer[0] != AS_CMD_FLASH_READ || buffer[1] == 0xFF){
		if (buffer[1] == 0xFF)
			return -1;	//empty
		else
			return 0;
	}

	buffer[AS_PRESET_NAME_LENGTH+1] = 0;
	strcpy(name, (char*)&buffer[1]);
	return 1;
}

// retrieve preset from device
int as_getPreset (as_t *as_ctx, uint8_t *buffer, const int bufferLen, int preset, int includeName)
{
	includeName &= 0x01;
	preset &= 0x7F;

	memset(buffer, 0, bufferLen);
	uint8_t writerBuffer[AS_PACKET_LENGTH];
	memset(writerBuffer, 0, sizeof(writerBuffer));
	
	for (int idx = 1; idx < 5+includeName; idx++){
		uint32_t addr = AS_PRESET_ADDRESS_BASE + (preset * AS_PRESET_ADDRESS_LENGTH) + (idx * AS_PAYLOAD_LENGTH);

		writerBuffer[0] = 0x00;
		writerBuffer[1] = AS_CMD_FLASH_READ;
		writerBuffer[2] = (addr>>16)&0xFF;
		writerBuffer[3] = (addr>>8)&0xFF;
		writerBuffer[4] = addr&0xFF;
		writerBuffer[5] = 0x00;		
		if (hid_write(as_ctx->device, writerBuffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
			return -1;

		writerBuffer[0] = 0;
		int readlen = hid_read_timeout(as_ctx->device, writerBuffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
		if (readlen != AS_REPORT_LENGTH || writerBuffer[0] != AS_CMD_FLASH_READ)
			return 0;
		memcpy(&buffer[(idx-1)*AS_PAYLOAD_LENGTH], &writerBuffer[1], AS_PAYLOAD_LENGTH);
	}
	return 1;
}

int as_getPresetDefault (as_t *as_ctx, uint8_t *buffer, const size_t bufferLen, int preset)
{
	preset &= 0x7F;

	memset(buffer, 0, bufferLen);
	uint8_t writerBuffer[AS_PACKET_LENGTH];
	memset(writerBuffer, 0, sizeof(writerBuffer));
	
	for (int idx = 1; idx < 6; idx++){
		uint32_t addr = AS_PRESET_ADDRESS_DEFAULTS + (preset * AS_PRESET_ADDRESS_LENGTH) + (idx * AS_PAYLOAD_LENGTH);

		writerBuffer[0] = 0x00;
		writerBuffer[1] = AS_CMD_FLASH_READ;
		writerBuffer[2] = (addr>>16)&0xFF;
		writerBuffer[3] = (addr>>8)&0xFF;
		writerBuffer[4] = addr&0xFF;
		writerBuffer[5] = 0x00;		
		if (hid_write(as_ctx->device, writerBuffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
			return -1;

		writerBuffer[0] = 0;
		int readlen = hid_read_timeout(as_ctx->device, writerBuffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
		if (readlen != AS_REPORT_LENGTH || writerBuffer[0] != AS_CMD_FLASH_READ)
			return 0;
		memcpy(&buffer[(idx-1)*AS_PAYLOAD_LENGTH], &writerBuffer[1], AS_PAYLOAD_LENGTH);
	}
	return 1;
}

int as_setPresetName (as_t *as_ctx, char *name, const int presetIdx)
{

	char presetName[AS_PRESET_NAME_LENGTH];
	memset(presetName, ' ', AS_PRESET_NAME_LENGTH);
	
	int slen = strlen((char*)name);
	if (!slen) return 0;
	if (slen > AS_PRESET_NAME_LENGTH)
		slen = AS_PRESET_NAME_LENGTH;
	if (slen < AS_PRESET_NAME_LENGTH){
		memcpy(presetName, name, slen);
		name = presetName;
	}

	int pos = 0;
	uint8_t buffer[AS_PACKET_LENGTH];
	
	buffer[pos++] = 0;
	buffer[pos++] = AS_CMD_ACTIVE_WRITE;
	buffer[pos++] = presetIdx&0x7F;
	buffer[pos++] = 1;
	memset(&buffer[pos], 32, AS_PRESET_NAME_LENGTH);
	memcpy((char*)&buffer[pos], name, AS_PRESET_NAME_LENGTH);
			
	pos += AS_PRESET_NAME_LENGTH;
	buffer[pos++] = 0;
	buffer[pos++] = 0;
	buffer[pos++] = 0;

	return (as_write(as_ctx, buffer, AS_PACKET_LENGTH) > 0);
}

void dumpHex (const unsigned char *data, size_t length)
{
	for (int i = 0; i < length; i++){
		printf("%.2X ", data[i]);
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

int as_dumpFlash (as_t *as_ctx)
{
	printf("\n");
	uint8_t buffer[AS_PACKET_LENGTH];
				
	const uint32_t startAdd = 0x00;
	const uint32_t endAdd = AS_C4_FLASHSIZE;

	for (int i = startAdd; i < endAdd; i += AS_PAYLOAD_LENGTH){
		memset(buffer, 0, sizeof(buffer));

		uint32_t addr = i;
		buffer[0] = 0x00;
		buffer[1] = AS_CMD_FLASH_READ;
		buffer[2] = (addr>>16)&0xFF;
		buffer[3] = (addr>>8)&0xFF;
		buffer[4] = addr&0xFF;
		buffer[5] = 0x00;		
		if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
			return -1;
		//Sleep(1);
		
		memset(buffer, 0, sizeof(buffer));
		int readlen = hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
		if (readlen != AS_REPORT_LENGTH)
			return -2;

		printf("%.6X:  ", i);
		dumpHex(&buffer[1], AS_PAYLOAD_LENGTH);
		//dumpASCII(&buffer[1], AS_PAYLOAD_LENGTH);
	}
	printf("\n");
	return 1;
}

int as_getFlash (as_t *as_ctx, const size_t address, const size_t length)
{
	printf("\n");
	uint8_t buffer[AS_PACKET_LENGTH];
				
	const uint32_t startAdd = address;
	uint32_t endAdd = address+length-1;
	if (endAdd > AS_C4_FLASHSIZE) endAdd = AS_C4_FLASHSIZE;
	if (startAdd > endAdd-(AS_PAYLOAD_LENGTH-1)) return -3;

	for (int i = startAdd; i < endAdd; i += AS_PAYLOAD_LENGTH){
		memset(buffer, 0, sizeof(buffer));

		uint32_t addr = i;
		buffer[0] = 0x00;
		buffer[1] = AS_CMD_FLASH_READ;
		buffer[2] = (addr>>16)&0xFF;
		buffer[3] = (addr>>8)&0xFF;
		buffer[4] = addr&0xFF;
		buffer[5] = 0x00;		
		if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
			return -1;
		//Sleep(1);
		
		memset(buffer, 0, sizeof(buffer));
		int readlen = hid_read_timeout(as_ctx->device, buffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
		if (readlen != AS_REPORT_LENGTH)
			return -2;

		printf("%.6X:  ", i);
		dumpHex(&buffer[1], AS_PAYLOAD_LENGTH);
		//dumpASCII(&buffer[1], AS_PAYLOAD_LENGTH);
	}
	printf("\n");
	return 1;
}

int as_getEEPROM (as_t *as_ctx, uint8_t *buffer, const int bsize)
{
	if (!as_ctx->device) return -1;
	if (bsize < AS_EEPROM_SIZE) return -3;
	
	uint8_t writebuffer[AS_PACKET_LENGTH];
	memset(writebuffer, 0, sizeof(writebuffer));

	for (int i = 0; i < AS_EEPROM_SIZE; i += AS_PAYLOAD_LENGTH){
		writebuffer[0] = 0x00;
		writebuffer[1] = AS_CMD_EEPROM_READ;
		writebuffer[2] = i;
		writebuffer[3] = 0x20;
		writebuffer[4] = 0;
		writebuffer[5] = 0;

		if (hid_write(as_ctx->device, writebuffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH)
			return -2;

		int readlen = hid_read_timeout(as_ctx->device, writebuffer, AS_PACKET_LENGTH, AS_READ_TIMEOUT);
		if (readlen != AS_REPORT_LENGTH || writebuffer[0] != AS_CMD_EEPROM_READ){
			if (buffer[1] == 0xFF)
				return -4;	//empty
			else
				return 0;
		}

		memcpy(buffer, &writebuffer[1], AS_PAYLOAD_LENGTH);
		buffer += AS_PAYLOAD_LENGTH;
	}

	return 1;
}

int as_writePreset (as_t *as_ctx, as_preset_t *preset, const int bsize, const int presetIdx)
{
	if (bsize % AS_PAYLOAD_LENGTH){
		printf("bsize:%i is not a multiple of as_payload_length\n", bsize);
		return -1;
	}

	uint8_t buffer[AS_PACKET_LENGTH];
	void *ppreset = preset;
	const int twrites = AS_PRESET_LENGTH / AS_PAYLOAD_LENGTH;
	
	for (int i = 0; i < twrites; i++){
		memset(buffer, 0, AS_PACKET_LENGTH);
		buffer[0] = 0;
		buffer[1] = AS_CMD_ACTIVE_STORE;
		buffer[2] = (i == twrites-1);
		buffer[3] = i*AS_PAYLOAD_LENGTH;
		buffer[4] = AS_PAYLOAD_LENGTH;
		
		memcpy(&buffer[5], ppreset, AS_PAYLOAD_LENGTH);
		ppreset += AS_PAYLOAD_LENGTH;

		if (hid_write(as_ctx->device, buffer, AS_PACKET_LENGTH) != AS_PACKET_LENGTH){
			printf("write failed %i\n", i);
			return 0;
		}
	}

	return as_setPresetName(as_ctx, (char*)preset->name, presetIdx);
}

