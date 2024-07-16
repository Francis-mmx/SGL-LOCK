#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H



#ifdef __CPU_AC521x__

#define FAT_CACHE_NUM   32

#define VIDEO0_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO1_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#ifdef CONFIG_PSRAM_ENABLE
#define VREC0_FBUF_SIZE     (512*1024)
#define VREC1_FBUF_SIZE     (256*1024)
#define VREC2_FBUF_SIZE     (256 * 1024)
#define AUDIO_BUF_SIZE      (256*1024)
#else
#if __SDRAM_SIZE__ <= (2 * 1024 * 1024)
#define VREC0_FBUF_SIZE     (600*1024)
#define VREC1_FBUF_SIZE     (300*1024)
#define VREC2_FBUF_SIZE     (300*1024)
#define AUDIO_BUF_SIZE      (64*1024)
#else
#if ((DOUBLE_720 == 0) && DOUBLE_720_SMALL_SCR == 0)
#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
#define VREC0_FBUF_SIZE     (1*1024*1024 )
#else
#define VREC0_FBUF_SIZE     (2*1024*1024 )
#endif
#define VREC1_FBUF_SIZE     (1*1024*1024 + 128*1024)
#define VREC2_FBUF_SIZE     (1*1024*1024 + 128*1024)
#elif (DOUBLE_720_SMALL_SCR == 1)
#define VREC0_FBUF_SIZE     (4*1024*1024 )
#define VREC1_FBUF_SIZE     (4*1024*1024 )
#define VREC2_FBUF_SIZE     (2*1024*1024 )
#else
#define VREC0_FBUF_SIZE     (2*1024*1024 + 384*1024 + 128*1024)
#define VREC1_FBUF_SIZE     (2*1024*1024 + 384*1024 + 128*1024)
#define VREC2_FBUF_SIZE     (2*1024*1024 )
#endif
#define AUDIO_BUF_SIZE      (300*1024)
#endif
#endif

#define USB_CAMERA_BUF_SIZE (512 * 1024) // + 512 * 1024)
#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
#define CAMERA_CAP_BUF_SIZE (200 * 1024)
#else
#define CAMERA_CAP_BUF_SIZE (2 * 1024 * 1024)
#endif




#elif defined __CPU_AC5401__

#define FAT_CACHE_NUM   64

#ifdef MP4_PACKAGE_ENABLE
#define CONFIG_AAC_DEC_ENABLE
#define CONFIG_AAC_ENC_ENABLE
#define VIDEO0_REC_FORMAT   VIDEO_FMT_MP4
#else
#define VIDEO0_REC_FORMAT   VIDEO_FMT_MOV
#endif
#define VIDEO1_REC_FORMAT   VIDEO_FMT_MOV
#if THREE_WAY_ENABLE
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#define VREC0_FBUF_SIZE     (12*1024*1024)
#define VREC1_FBUF_SIZE     (8*1024*1024)
#define VREC2_FBUF_SIZE     (8*1024*1024)
#else
#define VIDEO2_REC_FORMAT   VIDEO_FMT_MOV
#define VREC0_FBUF_SIZE     (6*1024*1024)
#define VREC1_FBUF_SIZE     (2*1024*1024)
#define VREC2_FBUF_SIZE     (4*1024*1024)
#endif
#define AUDIO_BUF_SIZE      (300*1024)
#define USB_CAMERA_BUF_SIZE (3 * 1024 * 1024) // + 512 * 1024)
#define CAMERA_CAP_BUF_SIZE (2 * 1024 * 1024)






#elif defined __CPU_AC5601__

#define FAT_CACHE_NUM   64

#define VIDEO0_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO1_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#define VREC0_FBUF_SIZE     (6*1024*1024)
#define VREC1_FBUF_SIZE     (2*1024*1024)
#define VREC2_FBUF_SIZE     (4*1024*1024)
#define AUDIO_BUF_SIZE      (300*1024)
#define CAMERA_CAP_BUF_SIZE (2 * 1024 * 1024)




#else
#error "undefined CPU"
#endif

#ifndef CONFIG_VIDEO1_ENABLE
#undef VREC1_FBUF_SIZE
#define VREC1_FBUF_SIZE (0)
#endif

#ifndef CONFIG_VIDEO2_ENABLE
#undef VREC2_FBUF_SIZE
#define VREC2_FBUF_SIZE (0)
#endif






























#endif
