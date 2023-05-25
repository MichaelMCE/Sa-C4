

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"






typedef const wchar_t * (HID_API_CALL *hid_error_t)(hid_device *dev);
typedef int (HID_API_CALL *hid_get_indexed_string_t)(hid_device *dev, int string_index, wchar_t *string, size_t maxlen);
typedef int (HID_API_CALL *hid_get_serial_number_string_t)(hid_device *dev, wchar_t *string, size_t maxlen);
typedef int (HID_API_CALL *hid_get_product_string_t)(hid_device *dev, wchar_t *string, size_t maxlen);
typedef int (HID_API_CALL *hid_get_manufacturer_string_t)(hid_device *dev, wchar_t *string, size_t maxlen);
typedef void (HID_API_CALL *hid_close_t)(hid_device *dev);
typedef int (HID_API_CALL *hid_get_feature_report_t)(hid_device *dev, unsigned char *data, size_t length);
typedef int (HID_API_CALL *hid_send_feature_report_t)(hid_device *dev, const unsigned char *data, size_t length);
typedef int (HID_API_CALL *hid_set_nonblocking_t)(hid_device *dev, int nonblock);
typedef int (HID_API_CALL *hid_read_t)(hid_device *dev, unsigned char *data, size_t length);
typedef int (HID_API_CALL *hid_read_timeout_t)(hid_device *dev, unsigned char *data, size_t length, int milliseconds);
typedef int (HID_API_CALL *hid_write_t)(hid_device *dev, const unsigned char *data, size_t length);
typedef hid_device *(HID_API_CALL *hid_open_path_t)(const char *path);
typedef hid_device *(HID_API_CALL *hid_open_t)(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number);
typedef void (HID_API_CALL *hid_free_enumeration_t)(struct hid_device_info *devs);
typedef struct hid_device_info *(HID_API_CALL *hid_enumerate_t)(unsigned short vendor_id, unsigned short product_id);
typedef int (HID_API_CALL *hid_exit_t)(void);
typedef int (HID_API_CALL *hid_init_t)(void);


static hid_error_t in_hid_error;
static hid_get_indexed_string_t in_hid_get_indexed_string;
static hid_get_serial_number_string_t in_hid_get_serial_number_string;
static hid_get_product_string_t in_hid_get_product_string;      
static hid_get_manufacturer_string_t in_hid_get_manufacturer_string; 
static hid_close_t in_hid_close;
static hid_get_feature_report_t in_hid_get_feature_report;
static hid_send_feature_report_t in_hid_send_feature_report;
static hid_set_nonblocking_t in_hid_set_nonblocking;
static hid_read_t in_hid_read;                    
static hid_read_timeout_t in_hid_read_timeout;
static hid_write_t in_hid_write;                   
static hid_open_path_t in_hid_open_path;               
static hid_open_t in_hid_open;
static hid_free_enumeration_t in_hid_free_enumeration;        
static hid_enumerate_t in_hid_enumerate;
static hid_exit_t in_hid_exit;
static hid_init_t in_hid_init;

static int initialized = 0;



static int importFunctions ()
{
	HMODULE lib_handle = LoadLibraryA("nero_hidapi.dll");
	if (lib_handle == NULL)
		lib_handle = LoadLibraryA("hidapi.dll_");
	
	#define RESOLVE(x) in_##x = (x##_t)GetProcAddress(lib_handle, #x); if (!in_##x) return 0;
	RESOLVE(hid_error);
	RESOLVE(hid_get_indexed_string);
	RESOLVE(hid_get_serial_number_string);
	RESOLVE(hid_get_product_string);
	RESOLVE(hid_get_manufacturer_string);
	RESOLVE(hid_close);
	RESOLVE(hid_get_feature_report);
	RESOLVE(hid_send_feature_report);
	RESOLVE(hid_set_nonblocking);
	RESOLVE(hid_read);
	RESOLVE(hid_read_timeout);
	RESOLVE(hid_write);
	RESOLVE(hid_open_path);
	RESOLVE(hid_open);
	RESOLVE(hid_free_enumeration); 
	RESOLVE(hid_enumerate);
	RESOLVE(hid_exit);
	RESOLVE(hid_init);
	
	return 1;
}

static inline void dumbHex (const unsigned char *data, size_t length)
{
	for (int i = 0; i < length; i++)
		printf(" %i", data[i]);
	printf("\n");

#if 1
	for (int i = 0; i < length; i++){
		if (data[i] > 31)
			printf("%c", data[i]);
		else
			printf(" ");
	}
	printf("\n");
#endif
	printf("\n");

}

int HID_API_EXPORT hid_init (void)
{
	printf("hid_init(): \n");
	
	if (!initialized){
		initialized = importFunctions();
		
		if (!initialized){
			printf("import failed\n");
			return 0;
		}
	}
	
	
	int ret = in_hid_init();
	printf("in_hid_init %i\n", ret);
	
	return ret;
}

int HID_API_EXPORT hid_exit (void)
{
	printf("hid_exit(): \n");
	return in_hid_exit();
}

struct hid_device_info HID_API_EXPORT * HID_API_CALL hid_enumerate (unsigned short vendor_id, unsigned short product_id)
{
	printf("hid_enumerate(): V:%X P:%X\n", vendor_id, product_id);
	
	return in_hid_enumerate(vendor_id, product_id);
}

void HID_API_EXPORT HID_API_CALL hid_free_enumeration (struct hid_device_info *devs)
{
	printf("hid_free_enumeration(): \n");
	
	in_hid_free_enumeration(devs);
}

hid_device * HID_API_EXPORT HID_API_CALL hid_open (unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	printf("hid_open(): V:%X P:%X Ser:'%s'\n", vendor_id, product_id, serial_number);
	
	return in_hid_open(vendor_id, product_id, serial_number);
}

hid_device * HID_API_EXPORT HID_API_CALL hid_open_path (const char *path)
{
	printf("hid_open_path():\n '%s'\n", path);
	
	return in_hid_open_path(path);
}

int HID_API_EXPORT HID_API_CALL hid_write (hid_device *dev, const unsigned char *data, size_t length)
{
	printf("hid_write(): len:%i\n", length);
	dumbHex(data, length);
	
	return in_hid_write(dev, data, length);
}

int HID_API_EXPORT HID_API_CALL hid_read_timeout (hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
	printf("hid_read_timeout(): len:%i, time:%i\n", length, milliseconds);
	
	memset(data, 0, length);
	int ret = in_hid_read_timeout(dev, data, length, milliseconds);
	if (ret > 0) dumbHex(data, length);
	return ret;
}

int HID_API_EXPORT HID_API_CALL hid_read (hid_device *dev, unsigned char *data, size_t length)
{
	printf("hid_read(): len:%i\n");
	
	memset(data, 0, length);
	int ret = in_hid_read(dev, data, length);
	if (ret > 0) dumbHex(data, length);
	return ret;
}

int HID_API_EXPORT HID_API_CALL hid_set_nonblocking (hid_device *dev, int nonblock)
{
	printf("hid_set_nonblocking(): nb:%i\n", nonblock);
	
	return in_hid_set_nonblocking(dev, nonblock);
}

int HID_API_EXPORT HID_API_CALL hid_send_feature_report (hid_device *dev, const unsigned char *data, size_t length)
{
	printf("hid_send_feature_report(): len:%i\n");
	dumbHex(data, length);
	
	return in_hid_send_feature_report(dev, data, length);
}

int HID_API_EXPORT HID_API_CALL hid_get_feature_report (hid_device *dev, unsigned char *data, size_t length)
{
	printf("in_hid_get_feature_report(): len:%i\n");

	int ret = in_hid_get_feature_report(dev, data, length);
	if (ret > 0) dumbHex(data, ret);
	
	return ret;
}

void HID_API_EXPORT HID_API_CALL hid_close (hid_device *dev)
{
	printf("hid_close(): \n");
	
	return in_hid_close(dev);
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_manufacturer_string (hid_device *dev, wchar_t *string, size_t maxlen)
{
	printf("hid_get_manufacturer_string(): len:%i\n", maxlen);
	
	int ret = in_hid_get_manufacturer_string(dev, string, maxlen);
	if (ret > 0) printf(" '%s'\n", string);
	return ret;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_product_string (hid_device *dev, wchar_t *string, size_t maxlen)
{
	printf("hid_get_product_string(): len:%i\n", maxlen);
	
	int ret = in_hid_get_product_string(dev, string, maxlen);
	if (ret > 0) printf(" '%s'\n", string);
	return ret;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_serial_number_string (hid_device *dev, wchar_t *string, size_t maxlen)
{
	printf("hid_get_serial_number_string(): len:%i\n", maxlen);
	
	int ret = in_hid_get_serial_number_string(dev, string, maxlen);
	if (ret > 0) printf(" '%s'\n", string);
	return ret;
}

int HID_API_EXPORT_CALL HID_API_CALL hid_get_indexed_string (hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	printf("hid_get_indexed_string(): len:%i\n", maxlen);
	
	int ret = in_hid_get_indexed_string(dev, string_index, string, maxlen);
	if (ret > 0) printf(" '%s'\n", string);
	return ret;
}

const wchar_t *HID_API_EXPORT HID_API_CALL hid_error (hid_device *dev)
{
	printf("hid_error(): \n");
	
	return in_hid_error(dev);
}

