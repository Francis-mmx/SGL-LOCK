#ifndef APP_ACTION_H
#define APP_ACTION_H
#include "app_config.h"


#define ACTION_HOME_MAIN 				0x00001001
#define ACTION_DEVICE_OUT 				0x00001002



#define ACTION_VIDEO_REC_MAIN 			0x00002001
#define ACTION_VIDEO_REC_SET_CONFIG 	0x00002002
#define ACTION_VIDEO_REC_GET_CONFIG 	0x00002003
#define ACTION_VIDEO_REC_CHANGE_STATUS 	0x00002004
#define ACTION_VIDEO_REC_CONTROL        0x00002005
#define ACTION_VIDEO_REC_SWITCH_WIN     0x00002006
#define ACTION_VIDEO_REC_LOCK_FILE      0x00002007
#define ACTION_VIDEO_REC_SWITCH_WIN_OFF 0x00002008

#define ACTION_VIDEO_DEC_MAIN 			0x00004001
#define ACTION_VIDEO_DEC_SET_CONFIG 	0x00004002
#define ACTION_VIDEO_DEC_GET_CONFIG 	0x00004003
#define ACTION_VIDEO_DEC_CHANGE_STATUS 	0x00004004
#define ACTION_VIDEO_DEC_OPEN_FILE    	0x00004005
#define ACTION_VIDEO_DEC_CONTROL        0x00004006
#define ACTION_VIDEO_DEC_CUR_PAGE       0x00004007
#define ACTION_VIDEO_DEC_SWITCH         0x00004008

#define ACTION_PHOTO_TAKE_MAIN 			0x00008001
#define ACTION_PHOTO_TAKE_SET_CONFIG 	0x00008002
#define ACTION_PHOTO_TAKE_GET_CONFIG 	0x00008003
#define ACTION_PHOTO_TAKE_CHANGE_STATUS	0x00008004
#define ACTION_PHOTO_TAKE_GET_CAMERAID 	0x00008005
#define ACTION_PHOTO_TAKE_CONTROL       0x00008006
#define ACTION_PHOTO_TAKE_SWITCH_WIN    0x00008007

#define ACTION_MUSIC_PLAY_MAIN 			0x00009001
#define ACTION_MUSIC_PLAY_SET_CONFIG 	0x00009002
#define ACTION_MUSIC_PLAY_GET_CONFIG 	0x00009003
#define ACTION_MUSIC_PLAY_CHANGE_STATUS 0x00009004


#define ACTION_AUDIO_REC_MAIN 			0x00010001
#define ACTION_AUDIO_REC_SET_CONFIG 	0x00010002
#define ACTION_AUDIO_REC_GET_CONFIG 	0x00010003
#define ACTION_AUDIO_REC_CHANGE_STATUS  0x00010004


#define ACTION_USB_SLAVE_MAIN 			0x00020001
#define ACTION_USB_SLAVE_SET_CONFIG 	0x00020002
#define ACTION_USB_SLAVE_GET_CONFIG 	0x00020003

#define ACTION_UPGRADE_MAIN             0x00030001
#define ACTION_UPGRADE_SET_CONFIRM      0x00030002

#define ACTION_SYSTEM_MAIN 	    		0x00FFF001
#define ACTION_SYSTEM_SET_CONFIG 		0x00FFF002
#define ACTION_SYSTEM_GET_CONFIG 		0x00FFF003


#define ACTION_AUDIO_MAIN 	     		0x00011001
#define ACTION_AUDIO_SET_CONFIG     	0x00011002
#define ACTION_AUDIO_GET_CONFIG     	0x00011003
#define ACTION_AUDIO_CHANGE_STATUS      0x00011004




#endif

