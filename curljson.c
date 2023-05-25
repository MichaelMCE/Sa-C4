
#include <inttypes.h> 
#include "curl/curl.h"
#include "as.h"


#define JAS_USERAGENT		"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:95.0) Gecko/20100101 Firefox/95.0"
#define JAS_URL				"https://neuro-production.herokuapp.com/parse/classes/presetPublished"
#define JAS_CERTIFICATE		"ca-bundle.crt"



static const char requestCount[] = 
  "{"
	"\"where\":"
	"{"
	" 	\"productID\":%i,"
	" 	\"factory\":false"
	"},"
	"\"limit\":0,"
	"\"count\":1,"
	"\"_method\":\"GET\","
	"\"_ApplicationId\":\"shDuwenrsAqpJmVTU2Y23dxAbUYZ5K6VzOeRAQTX\","
	"\"_ClientVersion\":\"js1.11.1\","
	"\"_InstallationId\":\"bcb9419c-5eac-f8ba-bb87-a0f9f35dbae7\""
  "} ";



static const char requestPresets[] = 
  "{"
	"\"where\":"
	"{"
	" 	\"productID\":%i,"
	" 	\"factory\":false"
	"},"
	"\"include\":\"userInfoObject\","
	"\"limit\":%i,"
	"\"skip\":%i,"
	"\"order\":\"-createdAt\","
	"\"_method\":\"GET\","
	"\"_ApplicationId\":\"shDuwenrsAqpJmVTU2Y23dxAbUYZ5K6VzOeRAQTX\","
	"\"_ClientVersion\":\"js1.11.1\","
	"\"_InstallationId\":\"1.11.1\""
  "} ";


typedef struct _callAlloc_cb {
	char *buffer;
	uint32_t blen;
	uint32_t pos;
}callAlloc_cb_t;


static size_t write_data (void *buffer, size_t size, size_t nmemb, void *userp)
{
	//printf("%s", (char*)buffer);

	const size_t len = (size*nmemb);
	callAlloc_cb_t *cb = (callAlloc_cb_t*)userp;
	
	cb->buffer = realloc(cb->buffer, cb->blen + len + 2);
	memcpy(&cb->buffer[cb->pos], buffer, len);
	cb->buffer[cb->pos+len] = 0;
	cb->blen += len;
	cb->pos += len;

	return size * nmemb;
}

int jas_getAvailable (const int productId)
{
	int total = -1;

	struct curl_slist *slist1 = NULL;
	slist1 = curl_slist_append(slist1, JAS_USERAGENT);
	slist1 = curl_slist_append(slist1, "Accept: */*");
	slist1 = curl_slist_append(slist1, "Accept-Language: en-GB,en;q=0.5");
	slist1 = curl_slist_append(slist1, "Accept-Encoding: gzip, deflate, br");
	slist1 = curl_slist_append(slist1, "Content-Type: text/plain");
	slist1 = curl_slist_append(slist1, "Origin: https://neuro.sourceaudio.net");
	slist1 = curl_slist_append(slist1, "DNT: 1");
	slist1 = curl_slist_append(slist1, "Connection: keep-alive");
	slist1 = curl_slist_append(slist1, "Referer: https://neuro.sourceaudio.net/");
	slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: empty");
	slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: cors");
	slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
	
	CURL *hnd = curl_easy_init();
	if (!hnd) return 0;
	
	callAlloc_cb_t cb = {0};
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &cb);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data); 



	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 1024000L);
	curl_easy_setopt(hnd, CURLOPT_URL, JAS_URL);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);

	int rlen = strlen(requestCount) + 4096;
	char request[rlen];
	snprintf(request, rlen, requestCount, productId);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request);

	curl_easy_setopt(hnd, CURLOPT_USERAGENT, JAS_USERAGENT);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 500L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_CAINFO, JAS_CERTIFICATE);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	
	CURLcode ret = curl_easy_perform(hnd);

	curl_easy_cleanup(hnd);
	curl_slist_free_all(slist1);

	if (ret != CURLE_OK){
		free(cb.buffer);
		return total;
	}

	char *found = strstr(cb.buffer, "\"count\":");
	if (found) total = atoi(found+8);

	free(cb.buffer);
	return total;
}
 
char *jas_getList (const int productId, const int getTotal, const int getFrom)
{
	if (getTotal < 1 || getTotal > 128 || getFrom < 0)
		return NULL;
	
	struct curl_slist *slist1 = NULL;
	slist1 = curl_slist_append(slist1, JAS_USERAGENT);
	slist1 = curl_slist_append(slist1, "Accept: */*");
	slist1 = curl_slist_append(slist1, "Accept-Language: en-GB,en;q=0.5");
	slist1 = curl_slist_append(slist1, "Accept-Encoding: gzip, deflate, br");
	slist1 = curl_slist_append(slist1, "Content-Type: text/plain");
	slist1 = curl_slist_append(slist1, "Origin: https://neuro.sourceaudio.net");
	slist1 = curl_slist_append(slist1, "DNT: 1");
	slist1 = curl_slist_append(slist1, "Connection: keep-alive");
	slist1 = curl_slist_append(slist1, "Referer: https://neuro.sourceaudio.net/");
	slist1 = curl_slist_append(slist1, "Sec-Fetch-Dest: empty");
	slist1 = curl_slist_append(slist1, "Sec-Fetch-Mode: cors");
	slist1 = curl_slist_append(slist1, "Sec-Fetch-Site: cross-site");
	
	CURL *hnd = curl_easy_init();
	if (!hnd) return NULL;
	
	
	callAlloc_cb_t cb = {0};
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &cb);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data); 

	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 1024000L);
	curl_easy_setopt(hnd, CURLOPT_URL, JAS_URL);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	//curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, requestCount);

	int rlen = strlen(requestPresets) + 4096;	// few bytes extra for formatting
	char request[rlen];
	snprintf(request, rlen, requestPresets, productId, getTotal, getFrom);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request);
	
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, JAS_USERAGENT);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 500L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_CAINFO, JAS_CERTIFICATE);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	
	CURLcode ret = curl_easy_perform(hnd);

	curl_easy_cleanup(hnd);
	curl_slist_free_all(slist1);

	if (ret != CURLE_OK){
		free(cb.buffer);
		cb.buffer = NULL;
	}
	return cb.buffer;
}


#if 0
int main (int argc, char *argv[])
{
	int total = jas_getAvailable(AS_PRODUCT_C4_SYNTH);
	printf("total available: %i\n", total);

	char *buffer = jas_getList(AS_PRODUCT_C4_SYNTH, 5, 0);
	if (buffer){
		printf("\n%s\n", buffer);
		free(buffer);
	}
	
	return 1;
}
#endif

