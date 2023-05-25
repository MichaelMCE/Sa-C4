
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "libas.h"
#include "osbf.h"


#define OSBF_NEWLINE		"\n"
#define OSBF_BUFFERSIZE		(4096)





static inline char *rtrim (char *buffer, int len)
{
	while (len > 0 && buffer[--len] == ' ')
		buffer[len] = 0;
	return buffer;
}

static void write_tag (as_backup_t *bku, const char *tag)
{
	fprintf(bku->fd, "%s;"OSBF_NEWLINE, tag);
}

static void write_tagInt (as_backup_t *bku, const char *tag, const int value)
{
	fprintf(bku->fd, "%s=%i;"OSBF_NEWLINE, tag, value);
}

static void write_tagStr (as_backup_t *bku, const char *tag, const char *value)
{
	fprintf(bku->fd, "%s=%s;"OSBF_NEWLINE, tag, value);
}

static void write_str (as_backup_t *bku, const char *str)
{
	fprintf(bku->fd, "%s"OSBF_NEWLINE, str);
}

static void write_hex (as_backup_t *bku, const int value)
{
	fprintf(bku->fd, "%.2X", value);
}
static void write_startdat (as_backup_t *bku)
{
	fprintf(bku->fd, "START_DATA"OSBF_NEWLINE);
}

static void write_enddat (as_backup_t *bku)
{
	fprintf(bku->fd, "END_DATA"OSBF_NEWLINE);
}

static void write_header (as_backup_t *bku)
{
	write_startdat(bku);
	write_tag(bku, "BACKUP_INFO");
	write_tagInt(bku, "PRODUCT_ID", AS_PRODUCT_C4_SYNTH);
	write_enddat(bku);
}

static void write_newline (as_backup_t *bku)
{
	fprintf(bku->fd, OSBF_NEWLINE);
}

static void write_tagend (as_backup_t *bku)
{
	fprintf(bku->fd, ";"OSBF_NEWLINE);
}

static int write_eeprom (as_backup_t *bku)
{
	uint8_t buffer[AS_EEPROM_SIZE];
	if (as_getEEPROM(bku->as_ctx, buffer, AS_EEPROM_SIZE) != 1)
		return 0;
	
	write_startdat(bku);
	write_tag(bku, "USER_EEPROM");
	write_tagInt(bku, "SIZE", AS_EEPROM_SIZE);

	for (int i = 0; i < AS_EEPROM_SIZE; i++)
		write_hex(bku, buffer[i]);
	
	write_tagend(bku);
	write_enddat(bku);
	
	return 1;
}

void write_presetname (as_backup_t *bku, const int presetIdx)
{
	char presetName[AS_PRESET_NAME_LENGTH+1] = {0};
	if (as_getPresetName(bku->as_ctx, presetIdx, presetName) == 1)
		write_tagStr(bku, "NAME", rtrim(presetName, AS_PRESET_NAME_LENGTH));
}

static void write_presetbin (as_backup_t *bku, const int presetIdx)
{
	as_preset_t preset = {0};
	uint8_t *ppreset = (uint8_t*)&preset;
	as_getPreset(bku->as_ctx, ppreset, sizeof(preset), presetIdx, 1);
	
	for (int i = 0; i < AS_PRESET_LENGTH+AS_PRESET_NAME_LENGTH; i++)
		write_hex(bku, ppreset[i]);
	write_tagend(bku);
}

static int write_preset (as_backup_t *bku, const int presetIdx)
{
	char presetName[AS_PRESET_NAME_LENGTH+1] = {0};
	
	if (as_getPresetName(bku->as_ctx, presetIdx, presetName) == 1){
		write_startdat(bku);
		write_tag(bku, "USER_PRESET");
		write_tagInt(bku, "LOCATION", presetIdx);
		write_tagInt(bku, "SIZE", AS_PRESET_LENGTH);
		write_tagStr(bku, "NAME", rtrim(presetName, AS_PRESET_NAME_LENGTH));
		write_presetbin(bku, presetIdx);
		write_enddat(bku);
	
		return 1;
	}
	return 0;
}

int osbf_write_preset (as_backup_t *bku, const int presetIdx)
{
	return write_preset(bku, presetIdx);
}

int osbf_write_eeprom (as_backup_t *bku)
{
	return write_eeprom(bku);
}

void osbf_write_header (as_backup_t *bku)
{
	write_header(bku);
}

void osbf_write_namelist (as_backup_t *bku)
{
	write_newline(bku);
	write_newline(bku);
	write_str(bku, "Preset Name List");

	char presetName[AS_PRESET_NAME_LENGTH+1] = {0};	
	
	for (int i = 0; i < AS_PRESET_TOTAL; i++){
		if (as_getPresetName(bku->as_ctx, i, presetName) == 1)
			fprintf(bku->fd, "Preset %.3i - %s"OSBF_NEWLINE, i+1, rtrim(presetName, AS_PRESET_NAME_LENGTH));
		else
			fprintf(bku->fd, "Preset %.3i - empty"OSBF_NEWLINE, i+1);
	}
}

void osbf_generate (as_backup_t *bku)
{
	osbf_write_header(bku);
	osbf_write_eeprom(bku);

	for (int i = 0; i < AS_PRESET_TOTAL; i++)
		osbf_write_preset(bku, i);
		
	osbf_write_namelist(bku);
}

int osbf_read_open (as_backup_t *bku, const char *path)
{
	bku->fd = fopen(path, "rbw+");
	return (bku->fd != NULL);	
}

void osbf_close (as_backup_t *bku)
{
	if (bku->fd){
		fclose(bku->fd);
		bku->fd = NULL;
		bku->deviceId = 0;
	}
}

int read_line (as_backup_t *bku, char *str)
{
	*bku->buffer = 0;
	if (fgets(bku->buffer, bku->blen, bku->fd) == NULL)
		return 0;
		
	return (strstr(bku->buffer, str) != NULL);
}

int read_lineInt (as_backup_t *bku, const char *fmt, int *value)
{
	*bku->buffer = 0;
	if (fgets(bku->buffer, bku->blen, bku->fd) == NULL)
		return 0;

	char *str = strchr(bku->buffer, '=');
	if (str++){
		*value = atoi(str);
		return 1;
	}
	return 0;
}

int read_lineStr (as_backup_t *bku, const char *fmt, char *value)
{
	*bku->buffer = 0;
	if (fgets(bku->buffer, bku->blen, bku->fd) == NULL)
		return 0;
	
	char *str = strchr(bku->buffer, '=');
	if (str++){
		strcpy(value, str);
		str = strchr(value, ';');
		if (str){
			*str = 0;
			return 1;
		}
	}
	return 0;
}

int read_header (as_backup_t *bku)
{
	if (!read_line(bku, "START_DATA"))
		return 0;
	if (!read_line(bku, "BACKUP_INFO"))
		return 0;
		
	bku->deviceId = 0;
	if (!read_lineInt(bku, "PRODUCT_ID=", &bku->deviceId))
		return 0;
	
	if (!bku->deviceId)
		return 0;
	else
		return read_line(bku, "END_DATA");
}

int read_eeprom (as_backup_t *bku)
{
	if (!read_line(bku, "START_DATA"))
		return 0;
	
	if (!read_line(bku, "USER_EEPROM"))
		return 0;

	int elength = 0;
	if (!read_lineInt(bku, "SIZE=", &elength))
		return 0;
	
	if (elength != AS_EEPROM_SIZE){
		//printf(".osbf is corrupt\n");
		return 0;
	}

	char buffer[elength*4];
	memset(buffer, 0, sizeof(buffer));
	read_line(bku, "");

	const int blen = strlen(bku->buffer);
	if (blen-2 < elength*2)
		return 0;

	uint8_t *eeprom = (uint8_t*)bku->eeprom;
	int ct = 0;
	
	for (int i = 0; i < (elength*2); i+=2){
		uint32_t value = 0;
		ct += sscanf(&bku->buffer[i], "%2X", &value);
		
		*eeprom = value;
		eeprom++;
	}

	if (ct != elength)
		return 0;
	else
		return read_line(bku, "END_DATA");
}
 
int read_preset (as_backup_t *bku, int *presetIdx)
{

	if (!read_line(bku, "START_DATA"))
		return 0;
	
	if (!read_line(bku, "USER_PRESET"))
		return 0;

	if (!read_lineInt(bku, "LOCATION=", presetIdx))
		return 0;
	
	bku->presetIdx = *presetIdx;
	//printf("location = %i\n", (*presetIdx)+1);
	
	int elength = 0;
	read_lineInt(bku, "SIZE=", &elength);
	//printf("size = %i\n", elength);
	
	if (elength != AS_PRESET_LENGTH){
		//printf(".osbf is corrupt\n");
		return 0;
	}
	
	char name[2*AS_PRESET_NAME_LENGTH+1];
	if (!read_lineStr(bku, "NAME=", name))
		return 0;
	//printf("name = '%s'\n", name);

	elength += AS_PRESET_NAME_LENGTH;

	read_line(bku, "");
	const int blen = strlen(bku->buffer);
	if (blen-2 < elength*2)
		return 0;
	
	uint8_t *preset = (uint8_t*)&bku->preset;
	memset(preset, 0, sizeof(bku->preset));
	
	int ct = 0;
	for (int i = 0; i < elength*2; i+=2){
		uint32_t value = 0;
		ct += sscanf(&bku->buffer[i], "%2X", &value);
		
		*preset = value;
		preset++;
	}

	if (ct != elength)
		return 0;
	else
		return read_line(bku, "END_DATA");
}

/*
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

void printHex16 (const uint8_t *data, size_t length)
{
	for (int i = 0; i < length; i += 16){
		for (int j = i; j < i+16 && j < length; j++){
			printf("%2.2X ", data[j]);
		}
		printf("\n");
	}
	printf("\n");
}
*/

static inline void printerror (const int anerror)
{
	switch (anerror) {
	case 1: printf("just as with our politicans', your file is corrupt\n"); break;
	case 2: printf("import failed, your file is on a bender\n"); break;
	case 3: printf("eeprom went awol, your file is dishonest\n"); break;
	case 4: printf("emotional damage - i'm all empty\n"); break;
	};
}

static int osbf_reset (as_backup_t *bku)
{
	const fpos_t pos = 0;
	return (!fsetpos(bku->fd, &pos));
}

void osbf_init (as_backup_t *bku, void *ctx, void *fd, backupcb_t backupcb)
{
	memset(bku, 0, sizeof(*bku));
	
	bku->as_ctx = ctx;
	bku->fd = fd;
	bku->blen = OSBF_BUFFERSIZE;
	bku->cb = backupcb;
}

int read_linePreList (as_backup_t *bku, int *value, char *name)
{
	*bku->buffer = 0;
	if (fgets(bku->buffer, bku->blen, bku->fd) == NULL)
		return -1;
	
	char *str = strchr(bku->buffer, ' ');
	if (str++){
		*value = atoi(str);
		
		str = strchr(bku->buffer, '-');
		if (str++){	
			str++;
			strcpy(name, str);
			
			str = strchr(name, '\x0D');
			if (str) *str = 0;
			
			str = strchr(name, '\x0A');
			if (str) *str = 0;
			
			return (strlen(name) > 0);
		}
	}
	return -2;
}

void osbf_read (as_backup_t *bku)
{

	if (!osbf_reset(bku)){
		printerror(4);
		return;
	}
	
	if (read_header(bku) != 1){
		printerror(1);
		return;
	}

	as_product_t pid = as_productLookup(bku->deviceId);
	if (!pid.id){
		printerror(2);
		return;
	}

	if (read_eeprom(bku) != 1){
		printerror(3);
		return;
	}

	printf("\n Device: %s\n", pid.name);
	
	int presetIdx;
	
	uint8_t preRead[AS_PRESET_TOTAL];
	memset(preRead, 0, sizeof(preRead));
	
	if (bku->cb){
		while (read_preset(bku, &presetIdx)){
			bku->cb(bku, presetIdx, OSBF_CB_WRITE);
			preRead[presetIdx] = 1;
		}
	}
	
	char str[128];		// name buffer. max name length is 32, 128 should be enough
	int preNum = 0;
	
	while (read_linePreList(bku, &preNum, str) != -1){
		if ((preNum >= 1 && preNum <= AS_PRESET_TOTAL) && !preRead[preNum-1]){
			if (!strcmp(str, "empty")){
				//printf("%i #%s#\n", preNum, str);
				bku->cb(bku, preNum-1, OSBF_CB_ERASE);
			}
		}
		preNum = 0;
	}
}

