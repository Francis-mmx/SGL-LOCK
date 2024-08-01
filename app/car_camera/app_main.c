#include "system/includes.h"
#include "server/ui_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "asm/rtc.h"
#include "storage_device.h"
#include "generic/log.h"



u32 spin_lock_cnt[2] = {0};

u32 uart_timer_handle = 0;


int upgrade_detect(const char *sdcard_name);


/*任务列表 */
const struct task_info task_info_table[] = {
#ifdef __CPU_AC521x__
    {"video_fb",            27,     1024,   1024  },
#else
    {"video_fb",            30,     1024,   1024  },
#endif
    {"ui_server",           29,     1024,   1024  },
    {"ui_task",             30,     4096,   512   },
    {"sys_timer",           10,     512,   2048  },//qsize必须大于2048
    {"init",                30,     1024,   256   },
    {"app_core",            15,     2048,   3072  },
    {"sys_event",           30,     1024,   0     },
    {"video_server",        16,     2048,   256   },
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     5120,   64    },
    {"audio_encoder",       17,     1024,   64    },
    {"speex_encoder",       17,     1024,   64    },
    {"amr_encoder",         17,     1024,   64    },
    {"aac_encoder",         17,     8192,   0     },
    {"video_dec_server",    27,     1024,   1024  },
    {"video0_rec0",         22,     2048,   512   },
    {"video0_rec1",         21,     2048,   512   },
    {"audio_rec0",          24,     2048,   256   },
    {"audio_rec1",          24,     2048,   256   },
    {"audio_rec2",          24,     2048,   256   },
    {"video1_rec0",         19,     2048,   512   },
    {"video2_rec0",         19,     2048,   512   },
    {"isp_update",          27,     1024,   0     },
    {"vpkg_server",         26,     2048,   512   },
    {"vunpkg_server",       23,     1024,   128   },
    {"avi0",                29,     2048,   64    },
    {"avi1",                29,     2048,   64    },
    {"avi2",                29,     2048,   64    },
    {"mov0",                28,     2048,   64    },
    {"mov1",                28,     2048,   64    },
    {"mov2",                28,     2048,   64    },
    {"mov3",                28,     2048,   64    },
    {"mp40",                28,     2048,   64    },
    {"mp41",                28,     2048,   64    },
    {"video_engine_server", 14,     1024,   1024  },
    {"video_engine_task",   15,     2048,   0     },
    {"usb_server",          20,     2048,   128   },
    {"khubd",               25,     1024,   32    },

    {"uvc_transceiver",     26,     2048,   32    },
    {"uvc_transceiver1",    26,     2048,   32    },
    {"uvc_transceiver2",    26,     2048,   32    },
    {"vimc_scale",          26,     2048,   32    },
    {"upgrade_server",      21,     1024,   32    },
    {"upgrade_core",        20,     1024,   32    },
    {"dynamic_huffman0",    15,		1024,	32    },
    {"dynamic_huffman1",    15,		1024,	32    },
    {"video0_devbuf0",      27,     1024,   32    },
    {"video1_devbuf0",      27,     1024,   32    },
    {"video2_devbuf0",      27,     1024,   32    },
    {"video_dec_devbuf",    27,     1024,   32    },
    {"audio_dec_devbuf",    27,     1024,   32    },
    {"jpg_dec",             27,     1024,   32    },

    {"yuv_enc0",            30,     3072,   256   },
    {"imr_rt0",             28,     3072,   256   },
    {"imr_rt1",             27,     3072,   256   },
    {"imr_enc_task",        28,     2048,   32    },
    /*  {"yuv_enc0",            12,     3072,   256   }, */
    /* {"imr_rt0",             30,     3072,   256   }, */
    /* {"imr_rt1",             30,     3072,   256   }, */



    {"auto_test",			15,		1024,	1024  },
    {"fs_test",			    15,		1024,	0     },

    {"powerdet_task",       15,     1024,   1024  },

    {"sys_timer",            15,     512,   256   },
#ifdef CONFIG_CT_AUDIO_TEST
    {"audio_test",            15,     512,   256   },
#endif
    {0, 0},
};


#ifdef CONFIG_UI_ENABLE

#ifdef PHOTO_STICKER_ENABLE
void ani_play_end_notifice_to_ph_app(u8 end);
#endif
/*
 * 开机动画播放完毕
 */
static void animation_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

#ifdef PHOTO_STICKER_ENABLE
    ani_play_end_notifice_to_ph_app(1);
#endif
    server_close(ui);

    /*
     * 显示完开机画面后更新配置文件,避免效果调节过度导致开机图片偏色
     */
    void *imd = dev_open("imd", NULL);
    if (imd) {
        dev_ioctl(imd, IMD_SET_COLOR_CFG, (u32)"scr_auto.bin"); /* 更新配置文件  */
        dev_close(imd);
    }

    /*
     *按键消息使能
     */
#ifdef CONFIG_PARK_ENABLE
    if (!get_parking_status())
#endif
    {
        sys_key_event_enable();
        sys_touch_event_enable();//使能触摸事件
    }
}

/*
 * 关机动画播放完毕, 关闭电源
 */
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;
    u32 park_en;

    if (ui) {
        server_close(ui);
    }

#ifdef CONFIG_GSENSOR_ENABLE
    park_en = db_select("par");
    set_parking_guard_wkpu(park_en);
    sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
    sys_power_set_port_wakeup("wkup_usb", 1);
    sys_power_set_port_wakeup("wkup_tp", 1);
    sys_power_poweroff(0);
}
#endif
#ifdef MULTI_LCD_EN
extern int ui_platform_init();
extern void *lcd_get_cur_hdl();
extern void lcd_set_cur_hdl(void *dev);
static u8 sw = 0;
u8 get_current_disp_device()
{
    return sw;
}
static void switch_lcd()
{
    struct intent it;
    struct application *app;

    void *lcd_dev = lcd_get_cur_hdl();
    app = get_current_app();
    init_intent(&it);
    if (app) {
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }
    if (lcd_dev) {
        dev_close(lcd_dev);
    }

    if (sw == 0) {
        lcd_dev = dev_open("lcd", "lcd_avout");
    } else {
        gpio_direction_output(IO_PORTF_07, 1);
        lcd_dev = dev_open("lcd", "bst40");
    }
    lcd_set_cur_hdl(lcd_dev);
    sw = !sw;

    ui_platform_init();
    if (app) {
        it.name = app->name;
        if (!strcmp(app->name, "video_rec")) {
            it.action = ACTION_VIDEO_REC_MAIN;
        } else if (!strcmp(app->name, "video_dec")) {
            it.action = ACTION_VIDEO_DEC_MAIN;
        } else if (!strcmp(app->name, "video_photo")) {
            it.action = ACTION_PHOTO_TAKE_MAIN;
        }
        start_app(&it);
    }
}
#endif

static int main_key_event_handler(struct key_event *key)
{
    struct intent it;
    struct application *app;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
#ifdef MULTI_LCD_EN
        case KEY_F1:
            switch_lcd();/*双屏切换*/
            break;
#endif
        case KEY_MODE:
            init_intent(&it);
            app = get_current_app();

            if (app) {
                if (!strcmp(app->name, "usb_app")) {
                    break;
                }
                it.action = ACTION_BACK;
                start_app(&it);

                if (!strcmp(app->name, "video_rec")) {
                    it.name = "video_photo";
                    it.action = ACTION_PHOTO_TAKE_MAIN;
                } else if (!strcmp(app->name, "video_photo")) {
                    it.name = "video_dec";
                    it.action = ACTION_VIDEO_DEC_MAIN;
                } else if (!strcmp(app->name, "video_dec")) {
                    it.name = "video_rec";
                    it.action = ACTION_VIDEO_REC_MAIN;
                }
                start_app(&it);
            }
            break;
        default:
            return false;
        }
        break;
    case KEY_EVENT_LONG:
        if (key->value == KEY_POWER) {
            puts("---key_power\n");
            static u8 power_fi = 0;
            if (power_fi) {
                puts("re enter power off\n");
                break;
            }
            power_fi = 1;
            sys_key_event_disable();

            struct sys_event e;
            e.type = SYS_DEVICE_EVENT;
            e.arg = "sys_power";
            e.u.dev.event = DEVICE_EVENT_POWER_SHUTDOWN;
            sys_event_notify(&e);
        }
        break;
    default:
        return false;
    }

    return true;
}

extern u8 get_usb_in_status();
static int main_dev_event_handler(struct sys_event *event)
{
    struct intent it;
    struct application *app;

    init_intent(&it);
    app = get_current_app();
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
#ifdef CONFIG_UI_ENABLE
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && strcmp(app->name, "usb_app") && strcmp(app->name, "sdcard_upgrade")) {
                it.action = ACTION_BACK;
                start_app(&it);

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            else if (!app) { //主界面进入usb界面
                union uireq req;
                struct server *ui;
                /* ui = server_open("ui_server", NULL); */
#ifdef MULTI_LCD_EN
                struct ui_style style = {0};
                if (get_current_disp_device()) {
                    style.file = "mnt/spiflash/res/avo_LY.sty\0";
                } else {
                    style.file = "mnt/spiflash/res/lcd_LY.sty\0";
                }
                ui = server_open("ui_server", &style);
#else
                ui = server_open("ui_server", NULL);
#endif

                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }
#endif
#ifdef CONFIG_PARK_ENABLE
            if (get_parking_status()) {
                sys_key_event_enable();
                sys_touch_event_enable();
            }
#endif
        }
#endif

#ifdef CONFIG_PARK_ENABLE
        if (!ASCII_StrCmp(event->arg, "parking", 7)) {
            if (app) {
                if (!strcmp(app->name, "video_rec")) {
                    break;
                }
                if ((!strcmp(app->name, "video_photo"))
                    || (!strcmp(app->name, "video_dec"))
                    || (!strcmp(app->name, "video_system"))
                    || (!strcmp(app->name, "usb_app"))) {

                    if (!strcmp(app->name, "usb_app")) {
                        if (get_usb_in_status()) {
                            puts("usb in status\n");
                            break;
                        }
                    }

                    it.action = ACTION_BACK;
                    start_app(&it);
                }
                puts("\n =============parking on eee video rec=========\n");
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            else if (!app) { //主界面进入倒车界面
                union uireq req;
                struct server *ui;
                ui = server_open("ui_server", NULL);
                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }
#endif

        }
#endif
        break;
    case DEVICE_EVENT_OUT:
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && !strcmp(app->name, "usb_app")) {
                it.action = ACTION_BACK;
                start_app(&it);

#ifdef PHOTO_STICKER_ENABLE
                it.name	= "video_photo";
                it.action = ACTION_PHOTO_TAKE_MAIN;
#else
                it.name	= "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
#endif

                start_app(&it);
#ifdef CONFIG_PARK_ENABLE
                if (get_parking_status()) {
                    sys_key_event_disable();
                    sys_touch_event_disable();
                }
#endif
            }
        }
        break;
    case DEVICE_EVENT_CHANGE:
        break;
    case DEVICE_EVENT_POWER_SHUTDOWN:
        if (!strcmp(event->arg, "sys_power")) {
            u32 park_en;

            init_intent(&it);
            app = get_current_app();
            if (app) {
                it.action = ACTION_BACK;
                start_app(&it);
            }

#ifdef CONFIG_UI_ENABLE
            struct ui_style style;
            style.file = "mnt/spiflash/audlogo/ani.sty";
            style.version = ANI_UI_VERSION;
            struct server *ui = server_open("ui_server", &style);

            if (ui) {
                union uireq req;

                if (get_parking_status()) {
                    puts("hide_park_ui\n");
                    req.hide.id = ID_WINDOW_PARKING;
                    server_request(ui, UI_REQ_HIDE, &req);
                }

                req.show.id = ANI_ID_PAGE_POWER_OFF;
                server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                                     power_off_play_end, ui);
            }
#else
#ifdef CONFIG_GSENSOR_ENABLE
            park_en = db_select("par");
            set_parking_guard_wkpu(park_en);  //gsensor parking guard */
            sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
            sys_power_set_port_wakeup("wkup_usb", 1);
            sys_power_set_port_wakeup("wkup_tp", 1);
            sys_power_poweroff(0);
#endif
        }
        break;
    }
    return 0;
}


/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
        break;
    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}
#if 0
void malloc_debug_start(void);
void malloc_debug_show(void);
void malloc_stats(void);
static void rtos_stack_check_func(void *p)
{
    char pWriteBuffer[2048];
    extern void vTaskList(char *pcWriteBuffer);
    vTaskList((char *)&pWriteBuffer);
    printf(" \n\ntask_name          task_state priority stack task_num\n%s\n", pWriteBuffer);
    malloc_stats();
    malloc_debug_show();

    /* extern unsigned char ucGetCpuUsage(void); */
    /* printf("cpu use:%d%%\n",ucGetCpuUsage()); */
}

void malloc_st(void *p)
{
    rtos_stack_check_func(NULL);
}

#endif

/*************************************Changed by liumenghui*************************************/
/*************************************发送数据包和重发机制*************************************/
extern u8 tx_flag;

int spec_uart_send(char *buf, u32 len) ;//串口发送
int uart_receive_package(u8 *buf, int len)  //串口接收   最大接收长度512字节  接收时间580ms左右
{
    u8 i;
    if(len > 0)
    {
        if(buf[0] == 0xAA && buf[1] == 0xBB && buf[2] == 0x6F && buf[3] == 0x6B)//确认信号
        {
            /*取消重发*/
            for(i=0;i<(tx_flag+1);i++)//在重发第三次的时候同样可以取消，否则进入超时处理
            {
                sys_timeout_del(uart_timer_handle);//删除添加的超时回调

            }
            tx_flag = 0;
            put_buf(buf,len);//打印接收值
        }
    }
}

static struct intent uart_buf;
int uart_send_package(u8 mode,u16 *command,u8 com_len)
{
    u8 total_length = com_len * (sizeof(u16)) + PACKET_OTHER_LEN;
    const char *packet_buf = create_packet_uncertain_len(mode,command,com_len);
    spec_uart_send(packet_buf,total_length);//首次发送

    //启用新的任务调度
    init_intent(&uart_buf);
    uart_buf.name	= "video_rec";
    uart_buf.action = ACTION_VIDEO_REC_UART_RETRANSMIT;
    uart_buf.data = packet_buf;
    uart_buf.exdata = total_length;
    start_app(&uart_buf);

    return 0;
}

void transmit_callback(struct intent *it)
{
    spec_uart_send(it->data,it->exdata);//重发数据包

    //再次调度任务
    uart_buf.name	= "video_rec";
    uart_buf.action = ACTION_VIDEO_REC_UART_RETRANSMIT;
    start_app(&uart_buf);
}

void transmit_overtime(void)//超时处理函数，后续可新增功能
{
    printf("uart transmit overtime\n");
}


int uart_recv_retransmit()
{
    if(tx_flag < MAX_TRANSMIT)
    {
        tx_flag++;
        uart_timer_handle = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
    }
    else
    {
        tx_flag = 0;
        uart_timer_handle = sys_timeout_add(0,transmit_overtime,100);//超时    超时处理      100ms后删除
    }
}
/*************************************Changed by liumenghui*************************************/

#define SECTOR_SIZE 0x1000              //4K

u32 flash_offset = 0x7EF000;            //8*1024*1024-68*1024
void write_data_to_flash(u8 *buf,u32 size)
{
    u16 len = 0;
    u8 sec = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;            //传入的size有几个sector
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return ;
    }
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, (flash_offset - SECTOR_SIZE) * sec);     //擦除基地址前sec个扇区
    len = dev_bulk_write(dev, buf, flash_offset-size, size);                   //从基地址往前面写数据
    if(len != size)
    {
        printf("write error!\n");
    }       
    dev_close(dev);
    dev = NULL;
}

void read_data_from_flash(struct record_infor *buf,u32 size)
{
    u8 len = 0;
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return ;
    }
    len = dev_bulk_read(dev, buf, flash_offset-size, size);                   //从基地址往前面写数据
    if(len != size)
    {
        printf("read error!\n");
    }   
    dev_close(dev);
    dev = NULL;
}



/*
 * 应用程序主函数
 */
void app_main()
{
    struct intent it;
    int err;

    puts("app_main\n");
#ifdef MULTI_LCD_EN
    void *lcd_dev;
#if  0
    sw = 1;
    lcd_dev = dev_open("lcd", "lcd_avout");
#else
    sw = 0;
    lcd_dev = dev_open("lcd", "bst40");
#endif
    lcd_set_cur_hdl(lcd_dev);
#endif

    if (!fdir_exist("mnt/spiflash")) {
        mount("spiflash", "mnt/spiflash", "sdfile", 0, NULL);
    }

    mount_sd_to_fs(SDX_DEV);
    err = upgrade_detect(SDX_DEV);
    if (!err) {
        return;
    }

    extern int sys_power_init(void);
    sys_power_init();
    extern int user_isp_cfg_init(void);
    user_isp_cfg_init();
    uart_receive_callback(uart_receive_package); //串口回调函数


#if 0
    // mode 有三种
    // 0 直接缩放
    // 1 直接剪切
    // 2 先剪切，再缩放
    // 其它模式无效，自己根据需要选择使用模式
    // 其它参数根据名称
    /* extern int manual_scale_and_encode_jpeg(u8 *src_yuv, u8 *jpg_path, int src_width, int src_height, int dst_width, int dst_height, u8 mode); */
#define SRC_W	1280
#define SRC_H	720
#define YUV_SIZE (SRC_W * SRC_H * 3 / 2)

    u8 *yuv = (u8 *)malloc(YUV_SIZE);
    FILE *fp = fopen(CONFIG_ROOT_PATH"I420.yuv", "r");
    fread(fp, yuv, YUV_SIZE);
    fclose(fp);

    for (int i = 0; i < 10; i++) {
        // 手动缩放YUV并保存成保存jpg
        int jpg_size = manual_scale_and_encode_jpeg(yuv, CONFIG_ROOT_PATH"mm***.jpg", SRC_W, SRC_H, 2592, 1968, 1);
        /* os_time_dly(10); */
    }
#endif
    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    struct ui_style style;

    style.file = "mnt/spiflash/audlogo/ani.sty";
    style.version = ANI_UI_VERSION;

    struct server *ui = server_open("ui_server", &style);
    if (ui) {
        union uireq req;

        req.show.id = ANI_ID_PAGE_POWER_ON;
        /* #if ((LCD_DEV_WIDTH > 480) && (__SDRAM_SIZE__ <= 8*1024*1024))  */
        server_request(ui, UI_REQ_SHOW_SYNC, &req);
        animation_play_end(ui);

        /* #else */
        /* server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req, */
        /* animation_play_end, ui); */
        /* #endif */
    }
#else
    sys_key_event_enable();
    sys_touch_event_enable();
#endif

/*******************************************上电*******************************************/
    u8 mode_buf = voice;
    u16 command_buf[] = {powered};
    u8 c_len = sizeof(command_buf)/sizeof(command_buf[0]);
    uart_send_package(mode_buf,command_buf,c_len);
/*******************************************上电*******************************************/
    sys_power_auto_shutdown_start(db_select("aff") * 60);
    sys_power_low_voltage_shutdown(320, PWR_DELAY_INFINITE);
//    sys_power_charger_off_shutdown(10, 1);


    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

    /* sys_timer_add(NULL,malloc_st,5000);  */
#ifdef CONFIG_UI_ENABLE
    if (dev_online("usb0")) {
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_MAIN;
        start_app(&it);
        return;
    }
#endif



#ifdef PHOTO_STICKER_ENABLE
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_MAIN;
#else
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
#endif
    start_app(&it);

}







