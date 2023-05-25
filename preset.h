


#ifndef _PRESET_H
#define _PRESET_H


#include <stdbool.h>





typedef enum {
 PRESET_OBJECTID = 1,       
 PRESET_PREVIEWCODE,    
 PRESET_EFFECTID,       
 PRESET_PRESETFILE,     
 PRESET_USEROBJECT,     
 PRESET_FEATURED,       
 PRESET_USERNAME,       
 PRESET_PRESETTYPE,     
 PRESET_ARTIST,         
 PRESET_EFFECTNAME,     
 PRESET_NAME,           
 PRESET_PRODUCTID,      
 PRESET_NEUROVERSION,   
 PRESET_DESCRIPTION,    
 PRESET_USERINFOOBJECT, 
 PRESET_PRESET,         
 PRESET_CREATORNAME,    
 PRESET_FACTORY,        
 PRESET_CREATORID,      
 PRESET_LIKES_COUNT,    
 PRESET_INSTRUMENTID,   
 PRESET_CREATEDAT,      
 PRESET_UPDATEDAT,      
 PRESET_ADDEDCOUNT,
 
 //presetFile
 PRESET_FILE_TYPE,
 PRESET_FILE_NAME,
 PRESET_FILE_URL,
 
 PRESET_USEROBJ_TYPE,
 PRESET_USEROBJ_CLASSNAME,
 PRESET_USEROBJ_OBJECTID,
 
 PRESET_USERINFOOBJ_OBJECTID,         
 PRESET_USERINFOOBJ_DESCRIPTION,      
 PRESET_USERINFOOBJ_NEUROVERSION,     
 PRESET_USERINFOOBJ_LASTNAME,         
 PRESET_USERINFOOBJ_FIRSTNAME,        
 PRESET_USERINFOOBJ_BANDNAME,         
 PRESET_USERINFOOBJ_SOCIALLINKS,      
 PRESET_USERINFOOBJ_JOINDATE,         
 PRESET_USERINFOOBJ_PUBLISHCOUNT,     
 PRESET_USERINFOOBJ_ARTISTLINK,       
 PRESET_USERINFOOBJ_USERNAME,         
 PRESET_USERINFOOBJ_NINJA,            
 PRESET_USERINFOOBJ_ARTIST,           
 PRESET_USERINFOOBJ_EMAIL,            
 PRESET_USERINFOOBJ_PHOTO,            
 PRESET_USERINFOOBJ_CREATEDAT,        
 PRESET_USERINFOOBJ_UPDATEDAT,        
 PRESET_USERINFOOBJ_NOTIFICATIONTOKEN,
 PRESET_USERINFOOBJ_ADDED_PRESETS,
 PRESET_USERINFOOBJ_LIKED_PRESETS,
 PRESET_USERINFOOBJ_DOWNLOADCOUNT,    
 PRESET_USERINFOOBJ_LIKECOUNT,        
 PRESET_USERINFOOBJ_ABOUTME,          
 PRESET_USERINFOOBJ_MUSICGENRES,      
 PRESET_USERINFOOBJ_MYGEAR,           
 PRESET_USERINFOOBJ_FOLLOWERSCOUNT,
 PRESET_USERINFOOBJ_FOLLOWINGUSERS,
 PRESET_USERINFOOBJ_TYPE,           
 PRESET_USERINFOOBJ_CLASSNAME,

 PRESET_USERINFOOBJ_PHOTO_TYPE,
 PRESET_USERINFOOBJ_PHOTO_NAME,
 PRESET_USERINFOOBJ_PHOTO_URL,
 
 PRESET_USERINFOOBJ_JOINDATE_TYPE,
 PRESET_USERINFOOBJ_JOINDATE_ISO,

 PRESET_USERINFOOBJ_SOCIALLINKS_YOUTUBE,
 PRESET_USERINFOOBJ_SOCIALLINKS_TWITTER,
 PRESET_USERINFOOBJ_SOCIALLINKS_FACEBOOK,  
 PRESET_USERINFOOBJ_SOCIALLINKS_SOUNDCLOUD,
 PRESET_USERINFOOBJ_SOCIALLINKS_INSTAGRAM,

 PRESET_MAXITEMS
}presetTypes_t;



typedef struct {
	union {
	  char const *str;
	  int32_t i32;
	  int64_t i64;
	  bool boolean;
	  double dbl;
	}u;
}presetItem_t;


typedef struct {
	presetItem_t item[PRESET_MAXITEMS];
}preset_t;

typedef struct {
	preset_t **list;
	int total;
	void *source;	// input json buffer
	void *spool;
}presets_t;




#endif

