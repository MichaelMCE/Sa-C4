
#ifndef _BACKUP_H
#define _BACKUP_H


#define OSBF_BUFFERSIZE		(4096)
#define OSBF_CB_WRITE		1
#define OSBF_CB_ERASE		2

typedef void (*backupcb_t) (void *object, const int presetIdx, const int action);

typedef struct _backup_t {
	as_t *as_ctx;
	void *fd;
	backupcb_t cb;
	
	char buffer[OSBF_BUFFERSIZE];
	size_t blen;
		
	int deviceId;
	char eeprom[AS_EEPROM_SIZE];
	int presetIdx;
	as_preset_t preset;
	
}as_backup_t;






int osbf_write_eeprom (as_backup_t *bku);
int osbf_write_preset (as_backup_t *bku, const int presetIdx);
void osbf_write_names (as_backup_t *bku);
void osbf_generate (as_backup_t *bku);



void osbf_init (as_backup_t *bku, void *ctx, void *fd, backupcb_t backupcb);
int osbf_read_open (as_backup_t *bku, const char *path);
void osbf_read (as_backup_t *bku);
void osbf_close (as_backup_t *bku);


#endif
