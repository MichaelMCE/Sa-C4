
/*
  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
  SPDX-License-Identifier: MIT
  Copyright (c)

  Permission is hereby  granted, free of charge, to any  person obtaining a copy
  of this software and associated  documentation files (the "Software"), to deal
  in the Software  without restriction, including without  limitation the rights
  to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
  copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
  IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
  FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
  AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
  LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
    
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>

#include "tiny-json/tiny-json.h"
#include "preset.h"




typedef struct {
    json_t **mem;
    unsigned int nextFree;
    unsigned int totalMemPools;
	unsigned int totalAlloc;
	unsigned int currentPool;
    jsonPool_t pool;
}jsonStaticPool_t;



typedef void (*jsonObjCB_t) (json_t const *json, preset_t *preset);

typedef struct preset {
	int id;
	int type;
	char *name;
	jsonObjCB_t func;
}jsonpreset_t;
 
static void jas_processPresetFile (json_t const *json, preset_t *preset);
static void jas_processUserObject (json_t const *json, preset_t *preset);
static void jas_processUserInfoObject (json_t const *json, preset_t *preset);


static const jsonpreset_t jsonpreset[] = {
 {PRESET_OBJECTID,       JSON_TEXT,    "objectId"},
 {PRESET_PREVIEWCODE,    JSON_TEXT,    "previewCode"},
 {PRESET_EFFECTID,       JSON_INTEGER, "effectID"},
 {PRESET_PRESETFILE,     JSON_OBJ,     "presetFile", jas_processPresetFile},
 {PRESET_USEROBJECT,     JSON_OBJ,     "userObject", jas_processUserObject},
 {PRESET_FEATURED,       JSON_BOOLEAN, "featured"},
 {PRESET_USERNAME,       JSON_TEXT,    "username"},
 {PRESET_PRESETTYPE,     JSON_TEXT,    "presetType"},
 {PRESET_ARTIST,         JSON_BOOLEAN, "artist"},
 {PRESET_EFFECTNAME,     JSON_TEXT,    "effectName"},
 {PRESET_NAME,           JSON_TEXT,    "name"},
 {PRESET_PRODUCTID,      JSON_INTEGER, "productID"},
 {PRESET_NEUROVERSION,   JSON_TEXT,    "neuroVersion"},
 {PRESET_DESCRIPTION,    JSON_TEXT,    "description"},
 {PRESET_USERINFOOBJECT, JSON_OBJ,     "userInfoObject", jas_processUserInfoObject},
 {PRESET_PRESET,         JSON_TEXT,    "preset"},
 {PRESET_CREATORNAME,    JSON_TEXT,    "creatorName"},
 {PRESET_FACTORY,        JSON_BOOLEAN, "factory"},
 {PRESET_CREATORID,      JSON_TEXT,    "creatorID"},
 {PRESET_LIKES_COUNT,    JSON_INTEGER, "likes_count"},
 {PRESET_INSTRUMENTID,   JSON_INTEGER, "instrumentId"},
 {PRESET_CREATEDAT,      JSON_TEXT,    "createdAt"},
 {PRESET_UPDATEDAT,      JSON_TEXT,    "updatedAt"},
 {PRESET_ADDEDCOUNT,     JSON_INTEGER, "addedCount"},
 
 {0, 0, ""}
};


// presetFile
static const jsonpreset_t presetFile[] = {
 {PRESET_FILE_TYPE,     JSON_TEXT, "__type"},
 {PRESET_FILE_NAME,     JSON_TEXT, "name"},
 {PRESET_FILE_URL,      JSON_TEXT, "url"},

 {0, 0, ""}
};

// userObject
static const jsonpreset_t presetUserObj[] = {
 {PRESET_USEROBJ_TYPE,        JSON_TEXT, "__type"},
 {PRESET_USEROBJ_CLASSNAME,   JSON_TEXT, "className"},
 {PRESET_USEROBJ_OBJECTID,    JSON_TEXT, "objectId"},
 
 {0, 0, ""}
};


// userInfoObject Photo
static const jsonpreset_t presetInfoUserObjPhoto[] = {
 {PRESET_USERINFOOBJ_PHOTO_TYPE, JSON_TEXT, "__type"},
 {PRESET_USERINFOOBJ_PHOTO_NAME, JSON_TEXT, "name"}, 
 {PRESET_USERINFOOBJ_PHOTO_URL,  JSON_TEXT, "url"}, 
 
 {0, 0, ""}
};


// userInfoObject Joindate
static const jsonpreset_t presetUserInfoObjJoindate[] = {
 {PRESET_USERINFOOBJ_JOINDATE_TYPE,  JSON_TEXT, "__type"}, 
 {PRESET_USERINFOOBJ_JOINDATE_ISO,   JSON_TEXT, "iso"}, 
 
 {0, 0, ""}
};

// userInfoObject SocialLinks
static const jsonpreset_t presetUserInfoObjSocialLinks[] = {
 {PRESET_USERINFOOBJ_SOCIALLINKS_YOUTUBE,     JSON_TEXT, "youtube"}, 
 {PRESET_USERINFOOBJ_SOCIALLINKS_TWITTER,     JSON_TEXT, "twitter"}, 
 {PRESET_USERINFOOBJ_SOCIALLINKS_FACEBOOK,    JSON_TEXT, "facebook"}, 
 {PRESET_USERINFOOBJ_SOCIALLINKS_SOUNDCLOUD,  JSON_TEXT, "soundcloud"}, 
 {PRESET_USERINFOOBJ_SOCIALLINKS_INSTAGRAM,   JSON_TEXT, "instagram"}, 

 {0, 0, ""}
};


static void jas_processUserInfoObjSocialLinks (json_t const *json, preset_t *preset);
static void jas_processUserInfoObjJoindate (json_t const *json, preset_t *preset);
static void jas_processUserInfoObjPhoto (json_t const *json, preset_t *preset);
static void jas_processUserInfoObjAddedPresets (json_t const *json, preset_t *preset);
static void jas_processUserInfoObjLikedPresets (json_t const *json, preset_t *preset);
static void jas_processUserInfoObjFollowingUsers (json_t const *json, preset_t *preset);
static void jas_processUserInfoObjNotiToken (json_t const *json, preset_t *preset);



// userInfoObject
static const jsonpreset_t presetInfoUserObj[] = {
 {PRESET_USERINFOOBJ_OBJECTID,          JSON_TEXT,    "objectId"},
 {PRESET_USERINFOOBJ_DESCRIPTION,       JSON_TEXT,    "description"},
 {PRESET_USERINFOOBJ_NEUROVERSION,      JSON_TEXT,    "neuroVersion"},
 {PRESET_USERINFOOBJ_LASTNAME,          JSON_TEXT,    "lastName"},
 {PRESET_USERINFOOBJ_FIRSTNAME,         JSON_TEXT,    "firstName"},
 {PRESET_USERINFOOBJ_BANDNAME,          JSON_TEXT,    "bandName"},
 {PRESET_USERINFOOBJ_SOCIALLINKS,       JSON_OBJ,     "socialLinks", jas_processUserInfoObjSocialLinks},
 {PRESET_USERINFOOBJ_JOINDATE,          JSON_OBJ,     "joinDate",    jas_processUserInfoObjJoindate},
 {PRESET_USERINFOOBJ_PUBLISHCOUNT,      JSON_INTEGER, "publishCount"},   
 {PRESET_USERINFOOBJ_ARTISTLINK,        JSON_TEXT,    "artistLink"},        
 {PRESET_USERINFOOBJ_USERNAME,          JSON_TEXT,    "username"},          
 {PRESET_USERINFOOBJ_NINJA,             JSON_BOOLEAN, "ninja"},          
 {PRESET_USERINFOOBJ_ARTIST,            JSON_BOOLEAN, "artist"},         
 {PRESET_USERINFOOBJ_EMAIL,             JSON_TEXT,    "email"},             
 {PRESET_USERINFOOBJ_PHOTO,             JSON_OBJ,     "photo",       jas_processUserInfoObjPhoto},              
 {PRESET_USERINFOOBJ_CREATEDAT,         JSON_TEXT,    "createdAt"},         
 {PRESET_USERINFOOBJ_UPDATEDAT,         JSON_TEXT,    "updatedAt"},         
 {PRESET_USERINFOOBJ_NOTIFICATIONTOKEN, JSON_ARRAY,   "notificationToken", jas_processUserInfoObjNotiToken},
 {PRESET_USERINFOOBJ_ADDED_PRESETS,     JSON_ARRAY,   "addedPresets",      jas_processUserInfoObjAddedPresets},    
 {PRESET_USERINFOOBJ_LIKED_PRESETS,     JSON_ARRAY,   "liked_presets",     jas_processUserInfoObjLikedPresets},    
 {PRESET_USERINFOOBJ_DOWNLOADCOUNT,     JSON_INTEGER, "downloadCount"},  
 {PRESET_USERINFOOBJ_LIKECOUNT,         JSON_INTEGER, "likeCount"},      
 {PRESET_USERINFOOBJ_ABOUTME,           JSON_TEXT,    "aboutMe"},           
 {PRESET_USERINFOOBJ_MUSICGENRES,       JSON_TEXT,    "musicGenres"},       
 {PRESET_USERINFOOBJ_MYGEAR,            JSON_TEXT,    "myGear"},            
 {PRESET_USERINFOOBJ_FOLLOWERSCOUNT,    JSON_INTEGER, "followersCount"},
 {PRESET_USERINFOOBJ_FOLLOWINGUSERS,    JSON_ARRAY,   "followingUsers",    jas_processUserInfoObjFollowingUsers},
 {PRESET_USERINFOOBJ_TYPE,              JSON_TEXT,    "__type"},            
 {PRESET_USERINFOOBJ_CLASSNAME,         JSON_TEXT,    "className"},         
 
 {0, 0, ""}
};







static preset_t *presetsCreatePreset ()
{
	return calloc(1, sizeof(preset_t));
}

static void presetsFree (presets_t *presets)
{
	while (presets->total-- > 0)
		free(presets->list[presets->total]);
}

static void presetsResize (presets_t *presets, const int newSize)
{
	presets->list = realloc(presets->list, newSize * sizeof(preset_t*));
	presets->total = newSize;
}

static void presetsAdd (presets_t *presets, preset_t *preset)
{
	presets->list[presets->total-1] = preset;
}

static inline bool presetAddItem (preset_t *preset, const int presetItemId, const jsonType_t itemType, json_t const *child)
{
	//printf("presetAddItem %i %i\n", presetItemId, (int)itemType);
	
	if (itemType == JSON_TEXT)
		preset->item[presetItemId].u.str = json_getValue(child);
	else if (itemType == JSON_BOOLEAN)
		preset->item[presetItemId].u.boolean = json_getBoolean(child);
	else if (itemType == JSON_INTEGER)
		preset->item[presetItemId].u.i32 = (int32_t)json_getInteger(child);
	else if (itemType == JSON_REAL)
		preset->item[presetItemId].u.dbl = (double)json_getReal(child);
	else
		return 0;	// not handled
	
	return 1;	// handled
}


static inline void jas_processType (json_t const *json, const jsonType_t propertyType)
{
	char const *name = json_getName(json);
	if (name){
		if (propertyType == JSON_OBJ) printf(" {JSON_OBJ, \"%s\"},\n", name);
		if (propertyType == JSON_ARRAY) printf(" {JSON_ARRAY, \"%s\"},\n", name);
		if (propertyType == JSON_TEXT) printf(" {JSON_TEXT, \"%s\"},\n", name);
		if (propertyType == JSON_BOOLEAN) printf(" {JSON_BOOLEAN, \"%s\"},\n", name);
		if (propertyType == JSON_INTEGER) printf(" {JSON_INTEGER, \"%s\"},\n", name);
		if (propertyType == JSON_REAL) printf(" {JSON_REAL, \"%s\"},\n", name);
		if (propertyType == JSON_NULL) printf(" {JSON_NULL, \"%s\"},\n", name);
	}
}

static void jas_processUserInfoObjNotiToken (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		//jsonType_t propertyType = json_getType(child);

	
		/*if (propertyType == JSON_TEXT){
			char const *value = json_getValue(child);
			if (value) printf("noti token: %s\n", value);
		}*/
    }
}

static void jas_processUserInfoObjFollowingUsers (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		//jsonType_t propertyType = json_getType(child);

		/*if (propertyType == JSON_TEXT){
			char const *value = json_getValue(child);
			if (value) printf("like: %s\n", value);
		}*/
    }
}

static void jas_processUserInfoObjAddedPresets (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		//jsonType_t propertyType = json_getType(child);

		/*if (propertyType == JSON_TEXT){
			char const *value = json_getValue(child);
			if (value) printf("like: %s\n", value);
		}*/
    }
}

static void jas_processUserInfoObjLikedPresets (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		//jsonType_t propertyType = json_getType(child);

		/*if (propertyType == JSON_TEXT){
			char const *value = json_getValue(child);
			if (value) printf("like: %s\n", value);
		}*/
    }
}

static void jas_processUserInfoObjSocialLinks (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; presetUserInfoObjSocialLinks[i].id; i++){
			if (propertyType  == presetUserInfoObjSocialLinks[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, presetUserInfoObjSocialLinks[i].name)){
					if (!presetAddItem(preset, presetUserInfoObjSocialLinks[i].id, propertyType, child)){
						if (presetUserInfoObjSocialLinks[i].func)
							presetUserInfoObjSocialLinks[i].func(child, preset);
					}
				}
			}
		}
    }
}

static void jas_processUserInfoObjJoindate (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; presetUserInfoObjJoindate[i].id; i++){
			if (propertyType  == presetUserInfoObjJoindate[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, presetUserInfoObjJoindate[i].name)){
					if (!presetAddItem(preset, presetUserInfoObjJoindate[i].id, propertyType, child)){
						if (presetUserInfoObjJoindate[i].func)
							presetUserInfoObjJoindate[i].func(child, preset);
					}
				}
			}
		}
    }
}

static void jas_processUserInfoObjPhoto (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; presetInfoUserObjPhoto[i].id; i++){
			if (propertyType  == presetInfoUserObjPhoto[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, presetInfoUserObjPhoto[i].name)){
					if (!presetAddItem(preset, presetInfoUserObjPhoto[i].id, propertyType, child)){
						if (presetInfoUserObjPhoto[i].func)
							presetInfoUserObjPhoto[i].func(child, preset);
					}
				}
			}
		}
    }
}

static void jas_processUserObject (json_t const *json, preset_t *preset)
{

    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; presetUserObj[i].id; i++){
			if (propertyType  == presetUserObj[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, presetUserObj[i].name)){
					if (!presetAddItem(preset, presetUserObj[i].id, propertyType, child)){
						if (presetUserObj[i].func)
							presetUserObj[i].func(child, preset);
					}
				}
			}
		}
    }
}

static void jas_processUserInfoObject (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; presetInfoUserObj[i].id; i++){
			if (propertyType  == presetInfoUserObj[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, presetInfoUserObj[i].name)){
					if (presetInfoUserObj[i].func)
						presetInfoUserObj[i].func(child, preset);
					else
						presetAddItem(preset, presetInfoUserObj[i].id, propertyType, child);
				}
			}
		}
    }
}

static void jas_processPresetFile (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY){
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; presetFile[i].id; i++){
			if (propertyType  == presetFile[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, presetFile[i].name)){
					if (presetFile[i].func)
						presetFile[i].func(child, preset);
					else
						presetAddItem(preset, presetFile[i].id, propertyType, child);
				}
			}
		}
    }
}

static void jas_processSingle (json_t const *json, preset_t *preset)
{
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY) {
        puts("error");
        return;
    }
    
    json_t const *child;

	for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
		jsonType_t propertyType = json_getType(child);

		for (int i = 0; jsonpreset[i].id; i++){
			if (propertyType == jsonpreset[i].type){
				char const *name = json_getName(child);
				if (name && !strcmp(name, jsonpreset[i].name)){
					if (!presetAddItem(preset, jsonpreset[i].id, propertyType, child)){
						if (jsonpreset[i].func)
							jsonpreset[i].func(child, preset);
					}
				}
			}
		}
    }
}

static void jas_processBatch (json_t const *json, presets_t *presets)
{

    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY) {
        puts("error");
        return;
    }
    json_t const *child;
    
    for (child = json_getChild(json); child != 0; child = json_getSibling(child)){
        jsonType_t propertyType = json_getType(child);

		if (propertyType == JSON_OBJ){
			presetsResize(presets, presets->total+1);
			preset_t *preset = presetsCreatePreset();
			presetsAdd(presets, preset);

			jas_processSingle(child, preset);
		}else{
			printf("error processing json. %i is not an ojbect\n", propertyType);
			break;
		}
    }
}

static size_t fileLength (FILE *fp)
{
	fpos_t pos;
	
	fgetpos(fp, &pos);
	fseek(fp, 0, SEEK_END);
	size_t fl = ftell(fp);
	fsetpos(fp, &pos);
	
	return fl;
}

void *jas_loadFile (const char *filename, size_t *flength)
{

	*flength = 0;
	void *data = NULL;
	FILE *fd = fopen(filename, "rb");
	
	if (fd){
		*flength = fileLength(fd);
		if (*flength > 63){
			data = malloc(*flength);
			if (data){
				size_t ret = (size_t)fread(data, 1, *flength, fd);
				if (ret != *flength){
					printf("getfile(): file read error\n");
					free(data);
					data = NULL;
					*flength = 0;
				}
			}
		}
		fclose(fd);
	}
	
	return data;
}

static json_t *poolInit (jsonPool_t *pool)
{
    jsonStaticPool_t *spool = json_containerOf(pool, jsonStaticPool_t, pool);
    spool->nextFree = 1;

	spool->totalMemPools = 1;
	spool->totalAlloc = 8;		// total json_t in each pool
	
	spool->mem = calloc(spool->totalMemPools, sizeof(json_t*));
	spool->mem[0] = calloc(spool->totalAlloc, sizeof(json_t));

    return &spool->mem[spool->currentPool][0];
}

static json_t *poolAlloc (jsonPool_t *pool)
{
    jsonStaticPool_t *spool = json_containerOf(pool, jsonStaticPool_t, pool);
    if (spool->nextFree >= spool->totalAlloc){
    	spool->totalAlloc += 4096;
		spool->totalMemPools++;
		spool->mem = realloc(spool->mem, spool->totalMemPools  *sizeof(json_t*));
		spool->mem[++spool->currentPool] = calloc(spool->totalAlloc, sizeof(json_t));
    }
    return &spool->mem[spool->currentPool][spool->nextFree++];
}

static void poolFree (jsonPool_t *pool)
{
	jsonStaticPool_t *spool = json_containerOf(pool, jsonStaticPool_t, pool);
	
	for (int i = 0; i < spool->totalMemPools; i++)
		free(spool->mem[i]);

	free(spool->mem);
}

char const *presetGetStr (preset_t *preset, const int itemId)
{
	return preset->item[itemId].u.str;
}

int32_t presetGetInt (preset_t *preset, const int itemId)
{
	return preset->item[itemId].u.i32;
}

bool presetHasInt (preset_t *preset, const int itemId)
{
	return (preset->item[itemId].u.i32 != 0);
}

bool presetHasStr (preset_t *preset, const int itemId)
{
	return (preset->item[itemId].u.str != NULL) && *preset->item[itemId].u.str;
}

void dumpPresets (presets_t *presets)
{
	for (int i = 0; i < presets->total; i++){
		preset_t *preset = presets->list[i];
		
		printf("%i:\t\t%s\n", i, presetGetStr(preset, PRESET_OBJECTID));
		
		if (presetHasStr(preset, PRESET_USERINFOOBJ_LASTNAME) && presetHasStr(preset, PRESET_USERINFOOBJ_FIRSTNAME))
			printf("  %s %s\n", presetGetStr(preset, PRESET_USERINFOOBJ_FIRSTNAME), presetGetStr(preset, PRESET_USERINFOOBJ_LASTNAME));
		else
			printf("  %s\n", presetGetStr(preset, PRESET_USERNAME));

		printf("  %s\n", presetGetStr(preset, PRESET_NAME));
		printf("  %s\n", presetGetStr(preset, PRESET_EFFECTNAME));
		if (presetHasStr(preset, PRESET_DESCRIPTION))
			printf("  %s\n", presetGetStr(preset, PRESET_DESCRIPTION));

		printf("\n\n");
	}
}

presets_t *jas_importJsonBuffer (void *buffer, const size_t length)
{

    jsonStaticPool_t spool = {.pool = {.init = poolInit, .alloc = poolAlloc}};
    json_t const *json = json_createWithPool(buffer, &spool.pool);
    if (!json) {
        printf("jas_importJsonBuffer(): error json create\n");
        return NULL;
    }

	presets_t *presets = calloc(1, sizeof(*presets));
	presets->source = buffer;
	presets->spool = malloc(sizeof(spool));
	memcpy(presets->spool, &spool, sizeof(spool));
	
    jas_processBatch(json_getChild(json), presets);
	return presets;
}

presets_t *jas_importJsonPath (const char *path)
{

	size_t length = 0;
	char *str = jas_loadFile(path, &length);
	if (!str){
		printf("jas_loadJson(): Load failed\n");
		return NULL;
	}

	presets_t *presets = jas_importJsonBuffer(str, length);
	if (!presets) free(str);
	return presets;
}


void jas_free (presets_t *presets)
{	
	presetsFree(presets);
	poolFree(&((jsonStaticPool_t*)presets->spool)->pool);
    free(presets->spool);
//    free(presets->source);
}

preset_t *jas_getPreset (presets_t *presets, const int presetIdx)
{
	if (presetIdx < presets->total)
		return presets->list[presetIdx];
	else
		return NULL;
}


int jas_getTotalPresets (presets_t *presets)
{
	return presets->total;
}

const char *jas_getPresetXml (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset){
		void const *data = presetGetStr(preset, PRESET_PRESET);
		//if (data) return strdup(data);
		if (data) return (void*)data;
	}
	return NULL;
}

const char *jas_getPresetDesc (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset){
		void const *data = presetGetStr(preset, PRESET_DESCRIPTION);
		//if (data) return strdup(data);
		if (data) return (void*)data;
	}
	return NULL;
}

const char *jas_getPresetName (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset){
		void const *data = presetGetStr(preset, PRESET_NAME);
		//if (data) return strdup(data);
		if (data) return (void*)data;
	}
	return NULL;
}

const char *jas_getPresetUserName (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset){
		void const *data = presetGetStr(preset, PRESET_USERNAME);
		//if (data) return strdup(data);
		if (data) return (void*)data;
	}
	return NULL;
}

const char *jas_getPresetEffectName (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset){
		void const *data = presetGetStr(preset, PRESET_EFFECTNAME);
		//if (data) return strdup(data);
		if (data) return (void*)data;
	}
	return NULL;
}

const char *jas_getPresetURL (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset){
		void const *data = presetGetStr(preset, PRESET_FILE_URL);
		//if (data) return strdup(data);
		if (data) return (void*)data;
	}
	return NULL;
}

int16_t jas_getPresetProductId (presets_t *presets, const int presetIdx)
{
	preset_t *preset = jas_getPreset(presets, presetIdx);
	if (preset)
		return presetGetInt(preset, PRESET_PRODUCTID);
	return 0;
}

#if 0
void jas_test (void)
{
	presets_t *presets = jas_importJsonPath("latest.json");
	if (presets){
		const int total = jas_getTotalPresets(presets);
		for (int i = 0; i < total; i++){
			printf("\n%i:\n", i+1);
			//printf("%s\n\n", jas_getPresetXml(presets, i));
			printf("%s\n", jas_getPresetUserName(presets, i));
			printf("%s\n", jas_getPresetName(presets, i));
			printf("%s\n", jas_getPresetDesc(presets, i));
			printf("%s\n", jas_getPresetEffectName(presets, i));
			//printf("%i\n", jas_getPresetProductId(presets, i));
			
		}
		jas_free(presets);
	}
}


int  main ()
{

	jas_test();
	return EXIT_SUCCESS;
}
#endif

