


#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <inttypes.h>
#include <conio.h>
#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>


// gcc hid.c -lsetupapi -lhid

//typedef struct _HIDP_PREPARSED_DATA *PHIDP_PREPARSED_DATA;
//BOOLEAN NTAPI HidD_GetPreparsedData (HANDLE HidDeviceObject, PHIDP_PREPARSED_DATA *PreparsedData);




void getDeviceCapabilities (HANDLE deviceHandle)
{
	PHIDP_PREPARSED_DATA preparsedData;
	
	HidD_GetPreparsedData(deviceHandle, &preparsedData);

	HIDP_CAPS capabilities;
	HidP_GetCaps(preparsedData, &capabilities);


	printf("%s%X\n", "Usage Page: ", capabilities.UsagePage);
	printf("%s%d\n", "Input Report Byte length: ", capabilities.InputReportByteLength);
	printf("%s%d\n", "Output Report Byte length: ", capabilities.OutputReportByteLength);
	printf("%s%d\n", "Feature Report Byte length: ", capabilities.FeatureReportByteLength);
	printf("%s%d\n", "Number of Link Collection Nodes: ", capabilities.NumberLinkCollectionNodes);
	printf("%s%d\n", "Number of Input Button Caps: ", capabilities.NumberInputButtonCaps);
	printf("%s%d\n", "Number of InputValue Caps: ", capabilities.NumberInputValueCaps);
	printf("%s%d\n", "Number of InputData Indices: ", capabilities.NumberInputDataIndices);
	printf("%s%d\n", "Number of Output Button Caps: ", capabilities.NumberOutputButtonCaps);
	printf("%s%d\n", "Number of Output Value Caps: ", capabilities.NumberOutputValueCaps);
	printf("%s%d\n", "Number of Output Data Indices: ", capabilities.NumberOutputDataIndices);
	printf("%s%d\n", "Number of Feature Button Caps: ", capabilities.NumberFeatureButtonCaps);
	printf("%s%d\n", "Number of Feature Value Caps: ", capabilities.NumberFeatureValueCaps);
	printf("%s%d\n", "Number of Feature Data Indices: ", capabilities.NumberFeatureDataIndices);

	HidD_FreePreparsedData(preparsedData);
}

static char devicePath[256];


void deviceFound (HANDLE deviceHandle, PSP_DEVICE_INTERFACE_DETAIL_DATA detailData)
{	
	strncpy(devicePath, detailData->DevicePath, sizeof(devicePath)-1);
	
	getDeviceCapabilities(deviceHandle);
}

int hid_findDevice (const int16_t VID, const int16_t PID)
{
	int found = 0;
	SP_DEVICE_INTERFACE_DATA devInfoData = {.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA)};
	
	GUID hidGuid;
	HidD_GetHidGuid(&hidGuid);	
	HANDLE hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	
	int memberIndex = 0;
	int result = 1;
	

	while ((result=(int)SetupDiEnumDeviceInterfaces(hDevInfo, 0, &hidGuid, memberIndex, &devInfoData))){
		printf("\n\nDevice %i\n", memberIndex);

		DWORD length = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &length, NULL);

		PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, length);
		if (!detailData) break;
		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		ULONG required;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, length, &required, NULL);

		HANDLE deviceHandle = CreateFile(detailData->DevicePath, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle){
			HIDD_ATTRIBUTES attributes = {.Size = sizeof(attributes)};
			HidD_GetAttributes(deviceHandle, &attributes);

			if (attributes.VendorID == VID && attributes.ProductID == PID){
				deviceFound(deviceHandle, detailData);

				printf("VID:%X, PID:%X  Version:%X\n", attributes.VendorID, attributes.ProductID, attributes.VersionNumber);


				wchar_t buffer[132] = {0};
				HidD_GetSerialNumberString(deviceHandle, buffer, sizeof(buffer)/sizeof(buffer[0]));
				wprintf(L"Serial: '%s'\n", buffer);

				buffer[0] = 0;
				HidD_GetProductString(deviceHandle, buffer, sizeof(buffer)/sizeof(buffer[0]));
				wprintf(L"Prod. String: '%s'\n", buffer);

				buffer[0] = 0;
				HidD_GetManufacturerString(deviceHandle, buffer, sizeof(buffer)/sizeof(buffer[0]));
				wprintf(L"Manu. String: '%s'\n", buffer);
				
				found = 1;
			}

			CloseHandle(deviceHandle);
		}
		
		free(detailData);
		memberIndex++;
	};
	
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return found;
}

void deviceRead (HANDLE deviceHandle, HANDLE hIOPort)
{
	OVERLAPPED overlapped = {0};
	OVERLAPPED *poverlapped = NULL;
	DWORD len = 0;
	DWORD *pkey = NULL;
	DWORD bytesRead = 0;
	
	
	unsigned char report[39*2];
	unsigned char buffer[39] = {0, 0x45, 0x00, 0x00, 0x18, 0x35, 0x04, 0x08, 0x20, 0xf8, 0x9a, 0x07, 0xbe, 0x50, 0xc8, 0x01, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x35, 0x04, 0x08, 0x30, 0xf8, 0x9a, 0x07, 0xa0, 0xcb, 0x2c, 0x01, 0x18, 0x35, 0x04};
												//	 0x18, 0x35, 0x04, 0x08, 0x30, 0xf8, 0x9a, 0x07, 0xa0, 0xcb, 0x2c, 0x01, 
													// 0x18, 0x35, 0x04};

	
	while(!kbhit()){
		bytesRead = 0;
		

		SetLastError(ERROR_SUCCESS);
/*
WriteFile(
  [in]                HANDLE       hFile,
  [in]                LPCVOID      lpBuffer,
  [in]                DWORD        nNumberOfBytesToWrite,
  [out, optional]     LPDWORD      lpNumberOfBytesWritten,
  [in, out, optional] LPOVERLAPPED lpOverlapped
);
*/
		DWORD NumberOfBytesWritten = 0;
		int result = WriteFile(deviceHandle, buffer, 39, &NumberOfBytesWritten, NULL);
		//printf("WriteFile %i %i, %i\n", result, (int)NumberOfBytesWritten, (int)GetLastError());
		
		memset(report, 0, 39);
		result = ReadFile(deviceHandle, report, 39*2, &bytesRead, NULL);
		//printf("read result %i %i %i\n", result, bytesRead, (int)GetLastError());
		
		
		if (result){
			for (int i = 0; i < 39; i++)
				printf("%X ", (int)report[i]);
				
			printf("\n");
		}

		
		
		//if (GetQueuedCompletionStatus(deviceHandle, &len, (void*)&pkey, &poverlapped, INFINITE)){
		//	printf("GetQueuedCompletionStatus\n");
		//}
		Sleep(50);
	}
}

int main ()
{
	if (!hid_findDevice(0x29A4, 0x302)){
		printf("device not found\n");
		return 1;
	}
	
	


	printf("path: '%s'\n\n", devicePath);
	
	HANDLE deviceHandle = CreateFile(devicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//HANDLE deviceHandle = CreateFile(devicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	
	if (deviceHandle != INVALID_HANDLE_VALUE){
		printf("device opened\n");
		
		uint32_t key = 33;
		HANDLE hIOPort = CreateIoCompletionPort(deviceHandle, NULL, (ULONG_PTR)&key, 1);
		if (hIOPort == INVALID_HANDLE_VALUE){
			printf("Invalid hIOPort handle\n");
			return 0;
		}
	
		deviceRead(deviceHandle, hIOPort);
	
		PostQueuedCompletionStatus(hIOPort, 0, (ULONG_PTR)0, 0);
		Sleep(50);
		CloseHandle(hIOPort);
		CloseHandle(deviceHandle);	
	}
	return 0;
}





