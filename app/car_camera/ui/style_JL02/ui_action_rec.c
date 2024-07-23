#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "res.h"
#include "system/includes.h"
/* #include "menu_parm_api.h" */
#include "app_database.h"
#include "system/device/uart.h"


#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

#define REC_RUNNING_TO_HOME     1  //录像时返回主界面
#define ENC_MENU_HIDE_ENABLE    1  //选定菜单后子菜单收起
#define TOUCH_R90 				1  //触摸屏XY反转

struct rec_menu_info {
    char resolution;
    char double_route;
    char mic;
    char gravity;
    char motdet;
    char park_guard;
    char wdr;
    char cycle_rec;
    char car_num;
    char dat_label;
    char exposure;
    char gap_rec;

    u8 lock_file_flag; /* 是否当前文件被锁 */

    u8 page_exit;  /*页面退出方式  1切模式退出  2返回HOME退出 */
    u8 menu_status;/*0 menu off, 1 menu on*/
    s8 enc_menu_status;
    u8 battery_val;
    u8 battery_char;

    u8 onkey_mod;
    s8 onkey_sel;
    u8 key_disable;
    u8 hlight_show_status;  /* 前照灯显示状态 */

    u8 language;
    u8 volume_lv;

    int remain_time;
};


int rec_cnt = 0;
volatile char if_in_rec; /* 是否正在录像 */
static struct rec_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct rec_menu_info))

//static u32 test_tx = 0xa533;

extern int spec_uart_send(char *buf, u32 len);
extern int spec_uart_recv(char *buf, u32 len);





extern int storage_device_ready();
int sys_cur_mod;
u8 av_in_statu = 0;

u8 paper_lay_flag = 0;      //壁纸界面显示标志位
u8 page_pic_flag = 0;           //设置主界面页面标志位 0--第一页  1--第二页
static const u8 num_input[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9', ' ', '.'
};

u8 user_name[26]={0};       //储存输入的用户名
int user_name_num = 0;      //当前输入字符的个数

u8 user_name_arrsy[10][26] = {0};       //保存用户名的数组
u8 name_array_num = 0;      //存储用户名的个数
u8 list_page_num = 0;           //用户名列表页数
u8 user_page_flag = 0;          //用户名称设置界面标志位 0--用户管理  1--用户名称  2--用户详情
u8 now_btn_user = 0;        //当前选择进入的用户

bool user_function_array[10][2]={0};

u8 goto_facial_page_flag = 0;       //进入人脸界面标志位
u8 lock_array[8]={0};               //门锁配置项储存
static char move_num_str[10] ={0};
u8 enc_back_flag = 0;       //主界面显示标志位



/************************************************************
				    	录像模式设置
************************************************************/
/*
 * rec分辨率设置
 */
static const u8 table_video_resolution[] = {
    VIDEO_RES_1080P,
    VIDEO_RES_720P,
    VIDEO_RES_VGA,
};


/*
 * rec循环录像设置
 */
static const u8 table_video_cycle[] = {
    0,
    3,
    5,
    10,
};


/*
 * rec曝光补偿设置
 */
static const u8 table_video_exposure[] = {
    3,
    2,
    1,
    0,
    (u8) - 1,
    (u8) - 2,
    (u8) - 3,
};


/*
 * rec重力感应设置
 */
static const u8 table_video_gravity[] = {
    GRA_SEN_OFF,
    GRA_SEN_LO,
    GRA_SEN_MD,
    GRA_SEN_HI,
};



/*
 * rec间隔录影设置, ms
 */
static const u16 table_video_gap[] = {
    0,
    100,
    200,
    500,
};

static const u16 province_gb2312[] = {
    0xA9BE, 0xFEC4, 0xA8B4, 0xA6BB, 0xF2BD, //京，宁，川，沪，津
    0xE3D5, 0xE5D3, 0xE6CF, 0xC1D4, 0xA5D4, //浙，渝，湘，粤，豫
    0xF3B9, 0xD3B8, 0xC9C1, 0xB3C2, 0xDABA, //贵，赣，辽，鲁，黑
    0xC2D0, 0xD5CB, 0xD8B2, 0xF6C3, 0xFABD, //新，苏，藏，闽，晋
    0xEDC7, 0xBDBC, 0xAABC, 0xF0B9, 0xCAB8, //琼，冀，吉，桂，甘，
    0xEECD, 0xC9C3, 0xF5B6, 0xC2C9, 0xE0C7, //皖，蒙，鄂，陕，青，
    0xC6D4                                  //云
};

static const u8 num_table[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9'
};


struct car_num {
    const char *mark;
    u32 text_id;
    u32 text_index;
};

struct car_num_str {
    u8 province;
    u8 town;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
};

/*
 * system语言设置
 */
static const u8 table_rec_language[] = {
    Chinese_Simplified,  /* 简体中文 */
    English,             /* 英文 */
};

/*
 * 音量设置
 */
static const u8 table_key_voice[] = {
    VOICE_LOW,
    VOICE_MIDDLE,
    VOICE_HIGHT,
};

enum {
    PAGE_SHOW = 0,
    MODE_SW_EXIT,
    HOME_SW_EXIT,
};

/*****************************校验位 ************************************/
u16 calculate_checksum(u8 *array, u8 length) {
    u16 sum = 0;
    u8 temp_h,temp_l;
    for (int i = 0; i < length; i++) {
        sum += array[i];
        //printf("checksum %d",sum);
    }
    //temp_h = sum >> 8;
    //temp_l = sum & 0xFF;
    //printf("checksumH %x temp_l %x ",temp_h,temp_l);
    return sum;
}


/*****************************创建数据包 ************************************/
u8 *create_packet(Mode mode, Command command)
{
    Data uart_msg;
    Packet packet;
    uart_msg.header = 0xCD;       
    uart_msg.mode = mode;
    uart_msg.command = command;

    u8 *data_packet = malloc(sizeof(Packet));
    if (data_packet == NULL) {
        return NULL;
    }
    memcpy(packet.data, &uart_msg, sizeof(Data));

    packet.check = calculate_checksum(packet.data,sizeof(Data)/sizeof(uart_msg.header));
    printf("packet.check %x",packet.check);
    memcpy(data_packet, &packet, sizeof(Packet)/sizeof(packet.check));
    data_packet[PACKET_LEN-2] = packet.check >> 8;
    data_packet[PACKET_LEN-1] = packet.check & 0xFF;

    return data_packet;
}


void reset_up_ui_func()
{
    ui_show(ENC_UP_LAY);
}

/*
 * (begin)提示框显示接口 ********************************************
 */

///*
// * (begin)UI状态变更主动请求APP函数 ***********************************
// */
//static void rec_tell_app_exit_menu(void)
//{
////    int err;
////    struct intent it;
////    init_intent(&it);
////    it.name	= "video_rec";
////    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
////    it.data = "exitMENU";
////    err = start_app(&it);
////    if (err) {
////        ASSERT(err == 0, ":rec exitMENU\n");
////    }
//}

///*
// * (begin)APP状态变更，UI响应回调 ***********************************
// */
//static int rec_on_handler(const char *type, u32 arg)
//{
//    puts("\n***rec_on_handler.***\n");
//    if_in_rec = TRUE;
//    ui_hide(ENC_TIM_REMAIN);
//    ui_show(ENC_TIM_REC);
//    ui_show(ENC_ANI_REC_HL);
//
//    return 0;
//}

/*
 * 录像模式的APP状态响应回调
 */
static const struct uimsg_handl rec_msg_handler[] = {
//    { "lockREC",        rec_lock_handler     }, /* 锁文件 */
//    { "onREC",          rec_on_handler       }, /* 开始录像 */
//    { "offREC",         rec_off_handler      }, /* 停止录像 */
//    { "saveREC",        rec_save_handler     }, /* 保存录像 */
//    { "noCard",         rec_no_card_handler  }, /* 没有SD卡 */
//    { "fsErr",          rec_fs_err_handler   },
//    { "avin",           rec_av_in_handler    },
//    { "avoff",          rec_av_off_handler   },
//    { "HlightOn",    rec_headlight_on_handler},
//    { "HlightOff",   rec_headlight_off_handler},
//    { "Remain",         rec_remain_handler  },
    // { "onMIC",          rec_on_mic_handler   },
    // { "offMIC",         rec_off_mic_handler  },
     { NULL, NULL},      /* 必须以此结尾！ */
};
/*
 * (end)
 */
/*****************************录像模式页面回调 ************************************/
static int video_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***rec mode onchange init***\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_REC, rec_msg_handler); /* 注册APP消息响应 */
        sys_cur_mod = 1;  /* 1:rec, 2:tph, 3:dec */
        memset(__this, 0, sizeof_this);
        break;
    case ON_CHANGE_RELEASE:
//        if (__this->menu_status) {
//#if DOUBLE_720
//            ui_hide(ENC_SET_LAY);
//#else
//            ui_hide(ENC_SET_WIN);
//#endif
//        }
        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE);
        }
//        __rec_msg_hide(0);//强制隐藏消息框

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onchange = video_mode_onchange,
 .ontouch = NULL,
};
static int parking_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_PARKING, rec_msg_handler); /* 注册APP消息响应 */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_PARKING)
.onchange = parking_page_onchange,
 .ontouch = NULL,
};


/*****************************图标布局回调 ************************************/


static int rec_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    int index;
    int err;
    static int lock_event_flag = 0;

    switch (e) {
    case ON_CHANGE_INIT:
//        lock_event_id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, lock_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        //      unregister_sys_event_handler(lock_event_id);
        break;
    case ON_CHANGE_FIRST_SHOW: /* 在此获取默认隐藏的图标的状态并显示 */


        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY)
.onchange = rec_layout_up_onchange,
};



static int enc_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("enc_onchange\n");
        sys_key_event_takeover(true, false);
        __this->onkey_mod = 0;
        __this->onkey_sel = 0;

        if(goto_facial_page_flag){
            ui_show(ENC_LAY_USER_PAGE);
            ui_show(ENC_LAY_USER_DETAILS);
            break;
        }
        ui_show(ENC_LAY_BACK);
        reset_up_ui_func();
        break;
    default:
        return false;
    }

    return false;
}
static int enc_onkey(void *ctr, struct element_key_event *e)
{
    return true;
}
REGISTER_UI_EVENT_HANDLER(ENC_WIN)
.onchange = enc_onchange,
 .onkey = enc_onkey,
};

/***************************** 星期文字动作 ************************************/
static void get_system_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    /* printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec); */
    dev_close(fd);
}
static int ReturnWeekDay(unsigned int iYear, unsigned int iMonth, unsigned int iDay)
{
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    if (iMonth == 1 || iMonth == 2) {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    } else {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }

    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);        //iWeek为负时取模
    if (iWeek == 0) {     //星期日不作为一周的第一天
        iWeek = 7;
    }

    return iWeek;
}
static int text_week_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        printf("\nit is week %d\n", ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day));
        ui_text_set_index(text, ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(REC_TXT_WEEKDAY)
.onchange = text_week_onchange,
 .ontouch = NULL,
};

/****************************主界面时间控件动作 ************************************/
static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(REC_TIM_TIME)
.onchange = timer_sys_rec_onchange,
 .ontouch = NULL,
};
/*****************************主界面系统日期控件动作 ************************************/
static int timer_sys_date_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
//        puts("******************text_week_onchange");
        get_system_time(&sys_time);
        temp = ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1;
        if(temp!=last_temp){
            last_temp = temp;
            ui_text_show_index_by_id(REC_TXT_WEEKDAY, last_temp);
        }
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(REC_TIM_DATE)
.onchange = timer_sys_date_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 进入密码界面按钮 ************************************/
static int rec_goto_password_page_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 rx_buf[8] = {0};
    UI_ONTOUCH_DEBUG("**rec_goto_password_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(REC_POWER_UP_BTN);
        ui_hide(REC_POWER_UP_PIC);
        enc_back_flag = 0;
        ui_hide(ENC_LAY_BACK);
        ui_show(ENC_PASSWORD_LAY);
        reset_up_ui_func();
        spec_uart_send(create_packet(voice,input_admin_infor),PACKET_LEN);
        //uart_receive_package(rx_buf, 6);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(REC_PASSWORD_BTN)
.ontouch = rec_goto_password_page_ontouch,
};

/***************************** 退出密码界面返回壁纸界面按钮 ************************************/
static int rec_goto_back_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_back_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        ui_hide(ENC_PASSWORD_LAY);
        ui_show(ENC_LAY_BACK);
        reset_up_ui_func();
        spec_uart_send(create_packet(voice,operate_success),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_RETURN_BTN)
.ontouch = rec_goto_back_page_ontouch,
};

/***************************** 密码界面进入设置界面按钮 ************************************/
static int rec_goto_set_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        ui_hide(ENC_PASSWORD_LAY);
        ui_show(ENC_LAY_PAGE);
        spec_uart_send(create_packet(voice,enter_admin_mode),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN_SET)
.ontouch = rec_goto_set_page_ontouch,
};

/***************************** 设置界面返回密码界面 ************************************/
static int rec_set_list_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->language = index_of_table8(db_select("lag"), TABLE(table_rec_language));
        ui_text_show_index_by_id(SET_LANG_TXT,__this->language);

        __this->volume_lv = index_of_table8(db_select("kvo"), TABLE(table_key_voice));
        ui_text_show_index_by_id(SET_SOUND_TXT,__this->volume_lv);

        printf("=================== back :%d\n",db_select("back"));
        ui_text_show_index_by_id(SET_PAPER_TXT,db_select("back"));

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_LIST_LAY)
.onchange = rec_set_list_lay_onchange,
};

/***************************** 设置界面返回密码界面按钮 ************************************/
static int rec_set_goto_paw_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_goto_paw_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(paper_lay_flag == 0){                        //系统设置界面返回密码界面
            ui_hide(ENC_SET_LAY);
            ui_show(ENC_LAY_PAGE);
        }else{                                         //壁纸设置界面返回系统设置界面
            ui_hide(ENC_PAPER_LIST_LAY);
            ui_show(ENC_SET_LIST_LAY);
            ui_text_show_index_by_id(ENC_SET_TXT,0);
            ui_hide(ENC_PAPER_SET_PIC);
        }
        spec_uart_send(create_packet(voice,command_0),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ENC_SET_RETURN)
.ontouch = rec_set_goto_paw_page_ontouch,
};

/***************************** 设置界面 时间设置按钮 ************************************/
static int rec_goto_set_time_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_time_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_show(SET_DATE_LAY);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_TIME_BTN)
.ontouch = rec_goto_set_time_ontouch,
};

/***************************** 时间设置界面 ************************************/
static int rec_set_date_time_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_highlight_element_by_id(SET_TIME_DATE_HIGH_LIGHT);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_LAY)
.onchange = rec_set_date_time_onchange,
};


//struct ui_time *sys_set_date;
/*****************************设置日期控件动作 ************************************/
static int sys_set_date_year_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        break;
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        //sys_set_date->year  = sys_time.year;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_YEAR_CUR)
.onchange = sys_set_date_year_onchange,
};


static int sys_set_date_month_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->month = sys_time.month;
        break;
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->month = sys_time.month;
        //sys_set_date->month  = sys_time.month;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_MONTH_CUR)
.onchange = sys_set_date_month_onchange,
};

static int sys_set_date_day_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->day = sys_time.day;
        break;
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->day = sys_time.day;
        //sys_set_date->day  = sys_time.day;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_DAY_CUR)
.onchange = sys_set_date_day_onchange,
};

/*****************************显示设置日期控件 ************************************/
/*
static int show_set_date_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        break;
    case ON_CHANGE_INIT:
        time->year = sys_set_date->year;
        time->month = sys_set_date->month;
        time->day = sys_set_date->day;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_CURRENT)
.onchange = sys_set_date_year_onchange,
};
*/


/***************************** 设置界面 语言设置按钮 ************************************/
static int rec_goto_set_lang_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_lang_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_show(SET_LANG_LAY);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_LANG_BTN)
.ontouch = rec_goto_set_lang_ontouch,
};

/***************************** 语言设置界面 ************************************/
const static int lang_btn_id[] = {
        SET_BTN_LANG_1,
        SET_BTN_LANG_2,
};
const static int lang_pic_id[] = {
        SET_PIC_LANG_1,
        SET_PIC_LANG_2,
};
static int rec_lang_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->language = index_of_table8(db_select("lag"), TABLE(table_rec_language));
        ui_highlight_element_by_id(lang_pic_id[__this->language]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_LANG_LAY)
.onchange = rec_lang_lay_onchange,
};

static void menu_rec_language_set(int sel_item)
{
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "lag";
    it.exdata = table_rec_language[sel_item];
    start_app(&it);
    __this->language = sel_item;
}

static int rec_language_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_language_ontouch**");
    struct intent it;
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;

    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        j=sizeof(lang_pic_id)/sizeof(lang_pic_id[0]);
        for (i = 0; i < j; i++) {
            ui_no_highlight_element_by_id(lang_pic_id[i]);
        }
        j=sizeof(lang_btn_id)/sizeof(lang_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == lang_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        ui_highlight_element_by_id(lang_pic_id[sel_item]);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        j=sizeof(lang_btn_id)/sizeof(lang_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == lang_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        menu_rec_language_set(sel_item);
        ui_hide(ENC_SET_TXT);
        ui_show(ENC_SET_TXT);
        ui_hide(SET_TEXT_LANG_LAY);
        ui_show(SET_TEXT_LANG_LAY);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_BTN_LANG_1)
.ontouch = rec_language_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_BTN_LANG_2)
.ontouch = rec_language_ontouch,
};

/***************************** 设置界面 音量设置按钮 ************************************/
static int rec_goto_set_vol_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_vol_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_show(SET_VOLUME_LAY);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_SOUND_BTN)
.ontouch = rec_goto_set_vol_ontouch,
};

/***************************** 音量设置界面 ************************************/
const static int volume_btn_id[] = {
        SET_BTN_VOL_1,
        SET_BTN_VOL_2,
        SET_BTN_VOL_3,
};
const static int volume_pic_id[] = {
        SET_PIC_VOL_1,
        SET_PIC_VOL_2,
        SET_PIC_VOL_3,
};
static int rec_volume_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->volume_lv = index_of_table8(db_select("kvo"), TABLE(table_key_voice));
        ui_highlight_element_by_id(volume_pic_id[__this->volume_lv]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_VOLUME_LAY)
.onchange = rec_volume_lay_onchange,
};

static void menu_rec_volume_set(int sel_item)
{
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "kvo";
    it.exdata = table_key_voice[sel_item];
    start_app(&it);
    __this->volume_lv = sel_item;
}

static int rec_volume_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_volume_ontouch**");
    struct intent it;
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;

    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        j=sizeof(volume_pic_id)/sizeof(volume_pic_id[0]);
        for (i = 0; i < j; i++) {
            ui_no_highlight_element_by_id(volume_pic_id[i]);
        }
        j=sizeof(volume_btn_id)/sizeof(volume_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == volume_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        ui_highlight_element_by_id(volume_pic_id[sel_item]);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        j=sizeof(volume_btn_id)/sizeof(volume_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == volume_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        menu_rec_volume_set(sel_item);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_BTN_VOL_1)
.ontouch = rec_volume_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_BTN_VOL_2)
.ontouch = rec_volume_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_BTN_VOL_3)
.ontouch = rec_volume_ontouch,
};


/***************************** 设置界面 二级设置界面-关闭按钮 ************************************/
const static int menu_off_btn_id[] = {
        SET_LANG_OFF_BTN,
        SET_VOL_OFF_BTN,
        SET_TIME_OFF_BTN,
};
static int rec_set_two_menu_off_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    u8 i,j;
    int sel_item = 0;
    UI_ONTOUCH_DEBUG("**rec_set_two_menu_off_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        j=sizeof(menu_off_btn_id)/sizeof(menu_off_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == menu_off_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        printf("============== menu off id :%d\n",sel_item);
        switch(sel_item){
        case 0:
            __this->language = index_of_table8(db_select("lag"), TABLE(table_rec_language));
            ui_hide(SET_LANG_LAY);
            ui_text_show_index_by_id(SET_LANG_TXT,__this->language);
            break;
        case 1:
            __this->volume_lv = index_of_table8(db_select("kvo"), TABLE(table_key_voice));
            ui_hide(SET_VOLUME_LAY);
            ui_text_show_index_by_id(SET_SOUND_TXT,__this->volume_lv);
            break;
        case 2:
            ui_hide(SET_DATE_LAY);
            break;
        default:
            break;
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_LANG_OFF_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_VOL_OFF_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_OFF_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};


/***************************** 设置界面 壁纸设置按钮 ************************************/
static int rec_goto_set_paper_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_paper_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_SET_LIST_LAY);
        ui_show(ENC_PAPER_LIST_LAY);
        ui_text_show_index_by_id(ENC_SET_TXT,1);
        ui_show(ENC_PAPER_SET_PIC);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_PAPER_BTN)
.ontouch = rec_goto_set_paper_ontouch,
};

/***************************** 设置界面 壁纸设置界面 ************************************/
const static int REC_SET_PAPER_PIC[4] = {
    SET_PAPER_PIC_1,
    SET_PAPER_PIC_2,
    SET_PAPER_PIC_3,
    SET_PAPER_PIC_4,
};
static int rec_paper_list_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;

    switch (e) {
    case ON_CHANGE_INIT:
        paper_lay_flag = 1;
        break;
    case ON_CHANGE_RELEASE:
        paper_lay_flag = 0;
        break;
    case ON_CHANGE_FIRST_SHOW:

        ui_pic_show_image_by_id(REC_SET_PAPER_PIC[db_select("back")],1);        //显示选中选项

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAPER_LIST_LAY)
.onchange = rec_paper_list_lay_onchange,
};

/***************************** 设置界面 壁纸选择按钮 ************************************/
static int rec_set_paper_ui_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_paper_ui_ontouch**");
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;
    const int btn_id[] = {
        SET_PAPER_1,
        SET_PAPER_2,
        SET_PAPER_3,
        SET_PAPER_4
    };
    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        j=sizeof(btn_id)/sizeof(btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == btn_id[i]) {
                sel_item = i;
                break;
            }
        }

        ui_pic_show_image_by_id(REC_SET_PAPER_PIC[db_select("back")],0);        //隐藏选中选项

        db_update("back",sel_item);
        db_flush();
        printf("================= paper num:%d\n",db_select("back"));
        ui_pic_show_image_by_id(ENC_PAPER_SET_PIC,sel_item);
        ui_pic_show_image_by_id(REC_SET_PAPER_PIC[db_select("back")],1);        //显示选中选项
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_PAPER_1)
.ontouch = rec_set_paper_ui_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_PAPER_2)
.ontouch = rec_set_paper_ui_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_PAPER_3)
.ontouch = rec_set_paper_ui_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_PAPER_4)
.ontouch = rec_set_paper_ui_ontouch,
};

/***************************** 开机主界面 和 设置界面 壁纸图片UI ************************************/
static int rec_paper_set_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic,db_select("back"));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAPER_SET_PIC)
.onchange = rec_paper_set_pic_onchange,
};
REGISTER_UI_EVENT_HANDLER(PIC_BACK_REC)
.onchange = rec_paper_set_pic_onchange,
};

/***************************** 密码界面 密码输入按钮 ************************************/
#define PAW_NUM 8           //密码最大个数
u8 password_num = 0;        //输入的密码个数
u8 password_code[PAW_NUM+1] = {0};       //保存输入的密码
u8 asterisk_number[PAW_NUM] = {'*','*','*','*','*','*','*','*'};      //显示*号
u8 get_password_code[PAW_NUM+1] = {1,2,3,4,5,6,7,8};
static int rec_password_in_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_password_in_ontouch**");
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;
    const int btn_id[] = {
        PWD_NUM_BTN0,
        PWD_NUM_BTN1,
        PWD_NUM_BTN2,
        PWD_NUM_BTN3,
        PWD_NUM_BTN4,
        PWD_NUM_BTN5,
        PWD_NUM_BTN6,
        PWD_NUM_BTN7,
        PWD_NUM_BTN8,
        PWD_NUM_BTN9
    };
    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(password_num == PAW_NUM){
            printf("======================= pwd num max\n");
            break;
        }
        j=sizeof(btn_id)/sizeof(btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        password_code[password_num] = sel_item;
        printf("============ in pwd:");
        put_buf(password_code,PAW_NUM);             //输出当前输入的密码
        password_num++;
        ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", &asterisk_number[PAW_NUM-password_num]);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN0)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN1)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN2)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN3)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN4)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN5)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN6)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN7)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN8)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN9)
.ontouch = rec_password_in_ontouch,
};

/***************************** 密码界面 密码删除按钮 ************************************/
static int rec_password_del_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_password_del_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(password_num == 0){
            ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", " ");
            break;
        }
        password_num--;
        password_code[password_num] = 'a';
        printf("============== del pwd:");
        put_buf(password_code,PAW_NUM);             //输出当前输入的密码
        ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", &asterisk_number[PAW_NUM-password_num]);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_DEL_KEY)
.ontouch = rec_password_del_ontouch,
};

/***************************** 密码界面 密码确认按钮 ************************************/
static int rec_password_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 pw[PAW_NUM+1] = {0};
    u8 i,j;
    UI_ONTOUCH_DEBUG("**rec_password_ok_ontouch**");
    int tmp = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        put_buf(get_password_code,PAW_NUM);             //输出密码
        put_buf(password_code,PAW_NUM);             //输出输入密码
#if 0
        pw[0] = (db_select("pwd1") >> 24) & 0xFF - 48; 
        printf("password[0]  %d\n",pw[0]);
        pw[1] = (db_select("pwd1") >> 16) & 0xFF - 48; 
        printf("password[1]  %d\n",pw[1]);
        pw[2] = (db_select("pwd1") >> 8) & 0xFF - 48; 
        printf("password[2]  %d\n",pw[2]);
        pw[3] = (db_select("pwd1") >> 0) & 0xFF - 48; 
        printf("password[3]  %d\n",pw[3]);

        pw[4] = (db_select("pwd2") >> 24) & 0xFF - 48; 
        printf("password[4]  %d\n",pw[4]);
        pw[5] = (db_select("pwd2") >> 16) & 0xFF - 48; 
        printf("password[5]  %d\n",pw[5]);
        pw[6] = (db_select("pwd2") >> 8) & 0xFF - 48; 
        printf("password[6]  %d\n",pw[6]);
        pw[7] = (db_select("pwd2") >> 0) & 0xFF - 48; 
        printf("password[7]  %d\n",pw[7]);
#else        
        for (i = 0; i < 4; i++) {
            pw[i] = (db_select("pwd1") >> (24 - i * 8)) & 0xFF - 48;
            printf("password[%d]  %d\n", i, pw[i]);
        }
        
        for (i = 0; i < 4; i++) {
            pw[i+4] = (db_select("pwd2") >> (24 - i * 8)) & 0xFF - 48;
            printf("password[%d]  %d\n", i+4, pw[i+4]);
        }
#endif
        pw[PAW_NUM] = '\0';

        for(i=0;i<PAW_NUM;i++){
            printf("input_paw[%d] %d\n",i,password_code[i]);
        }
        tmp = strcmp(password_code,pw);
        if(tmp == 0){
            printf("================== pwd check succ\n");
        }else{
            printf("================== pwd check err\n");
        }
        spec_uart_send(create_packet(voice,unlocked),PACKET_LEN);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN_OK)
.ontouch = rec_password_ok_ontouch,
};

/***************************** 密码输入界面 ************************************/
static int rec_password_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        memset(password_code,'a',PAW_NUM);          //显示时赋值为a，表示为空
        password_num = 0;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PASSWORD_LAY)
.onchange = rec_password_lay_onchange,
};


/***************************** 设置主界面 ************************************/
static int rec_lay_set_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic,page_pic_flag);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_pic_show_image_by_id(ENC_PAGE_RIGHT_PIC,page_pic_flag);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_SET_PIC)
.onchange = rec_lay_set_pic_onchange,
};

/***************************** 设置主界面文字 ************************************/
static int rec_lay_set_txt1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT1,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT2,page_pic_flag);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_TXT1_PAGE)
.onchange = rec_lay_set_txt1_onchange,
};

static int rec_lay_set_txt2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT3,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT4,page_pic_flag);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_TXT2_PAGE)
.onchange = rec_lay_set_txt2_onchange,
};

/***************************** 主界面切换按钮 ************************************/
static int rec_page_left_right_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_page_left_right_ontouch**");
    struct button *btn = (struct button *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        page_pic_flag = !page_pic_flag;
        if(page_pic_flag == 0){
            ui_show(ENC_LAY_TXT2_PAGE);
            ui_text_show_index_by_id(ENC_LAY_PAGE_TXT3,page_pic_flag);
            ui_text_show_index_by_id(ENC_LAY_PAGE_TXT4,page_pic_flag);
        }else{
            ui_hide(ENC_LAY_TXT2_PAGE);
        }
        ui_pic_show_image_by_id(ENC_PAGE_RIGHT_PIC,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT1,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT2,page_pic_flag);
        ui_pic_show_image_by_id(ENC_LAY_SET_PIC,page_pic_flag);
        reset_up_ui_func();
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAGE_RIGHT_BTN)
.ontouch = rec_page_left_right_ontouch,
};

/***************************** 主界面选项按钮 1 ************************************/
static int rec_LAY_BTN_1_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_1_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(page_pic_flag == 0){
            ui_hide(ENC_LAY_PAGE);
            ui_show(ENC_LAY_USER_PAGE);
        }else{
            ui_hide(ENC_LAY_PAGE);
            ui_show(ENC_LAY_DOOR_LOCK_PAGE);
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_1)
.ontouch = rec_LAY_BTN_1_ontouch,
};

/***************************** 主界面选项按钮 2 ************************************/
static int rec_LAY_BTN_2_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_2_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(page_pic_flag == 0){
            ui_hide(ENC_LAY_PAGE);
            ui_show(ENC_LAY_RECORD_PAGE);
        }else{
            ui_hide(ENC_LAY_PAGE);
            ui_show(ENC_LAY_SYS_INFO_PAGE);
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_2)
.ontouch = rec_LAY_BTN_2_ontouch,
};

/***************************** 主界面选项按钮 3 ************************************/
static int rec_LAY_BTN_3_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_3_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_SET_LAY);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_3)
.ontouch = rec_LAY_BTN_3_ontouch,
};

/***************************** 主界面选项按钮 4 ************************************/
static int rec_LAY_BTN_4_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_4_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_4)
.ontouch = rec_LAY_BTN_4_ontouch,
};

/***************************** 设置界面返回密码界面按钮 ************************************/
static int rec_lay_page_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    
    UI_ONTOUCH_DEBUG("**rec_lay_page_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_PASSWORD_LAY);
        reset_up_ui_func();
        page_pic_flag = 0;
        spec_uart_send(create_packet(voice,exit_admin_mode),PACKET_LEN);
        
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAGE_RETURN)
.ontouch = rec_lay_page_btn_ontouch,
};



/***************************** 用户管理界面 ************************************/
static int rec_lay_user_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        if(goto_facial_page_flag){
            ui_show(ENC_LAY_USER_DETAILS);
        }else{
            ui_show(ENC_LAY_USER_LIST);
        }
        break;
    case ON_CHANGE_SHOW:
        sys_timeout_add(NULL,reset_up_ui_func,100);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_PAGE)
.onchange = rec_lay_user_page_onchange,
};



/***************************** 用户设置界面 ************************************/

const int lay_user_list[]={
    ENC_LAY_USER_NAME_1,
    ENC_LAY_USER_NAME_2,
    ENC_LAY_USER_NAME_3,
    ENC_LAY_USER_NAME_4,
    ENC_LAY_USER_NAME_5,
};
static int rec_lay_user_list_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 0;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);

        for(int i=0;i<name_array_num;i++){
            ui_show(lay_user_list[i]);
            if(i>4){
                break;
            }
        }

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_LIST)
.onchange = rec_lay_user_list_onchange,
};
static int rec_lay_user_input_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 1;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        memset(user_name,0,sizeof(user_name));
        user_name_num = 0;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_INPUT)
.onchange = rec_lay_user_input_onchange,
};


static int rec_lay_user_details_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 2;
        goto_facial_page_flag = 0;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        memset(user_name,0,sizeof(user_name));
        user_name_num = 0;
        ui_text_set_str_by_id(ENC_NOW_USER_NAME, "ascii", &user_name_arrsy[now_btn_user+(list_page_num*5)]);

        ui_pic_show_image_by_id(ENC_USER_PUSH_PIC,user_function_array[(list_page_num*5)+now_btn_user][0]);
        ui_text_show_index_by_id(ENC_USER_PUSH_TXT,user_function_array[(list_page_num*5)+now_btn_user][0]);

        ui_pic_show_image_by_id(ENC_USER_POWER_PIC,user_function_array[(list_page_num*5)+now_btn_user][1]);
        ui_text_show_index_by_id(ENC_USER_POWER_TXT,user_function_array[(list_page_num*5)+now_btn_user][1]);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_DETAILS)
.onchange = rec_lay_user_details_onchange,
};

/***************************** 用户管理界面返回按钮 ************************************/
static int rec_set_return_user_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_return_user_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(user_page_flag == 0){
            ui_hide(ENC_LAY_USER_PAGE);
            ui_show(ENC_LAY_PAGE);
        }else if(user_page_flag == 1){
            ui_hide(ENC_LAY_USER_INPUT);
            ui_show(ENC_LAY_USER_LIST);
        }else if(user_page_flag == 2){
            ui_hide(ENC_LAY_USER_DETAILS);
            ui_show(ENC_LAY_USER_LIST);
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_RETURN_USER)
.ontouch = rec_set_return_user_ontouch,
};


/***************************** 新建用户按钮 ************************************/
static int rec_set_new_user_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_new_user_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_LAY_USER_LIST);
        ui_show(ENC_LAY_USER_INPUT);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_NEW_USER_BTN)
.ontouch = rec_set_new_user_ontouch,
};

/***************************** 用户名列表显示 ************************************/
static int rec_lay_user_name_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_USER_NAME_TXT_1, "ascii", &user_name_arrsy[(list_page_num*5)+0]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_1)
.onchange = rec_lay_user_name_1_onchange,
};
static int rec_lay_user_name_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_USER_NAME_TXT_2, "ascii", &user_name_arrsy[(list_page_num*5)+1]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_2)
.onchange = rec_lay_user_name_2_onchange,
};
static int rec_lay_user_name_3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_USER_NAME_TXT_3, "ascii", &user_name_arrsy[(list_page_num*5)+2]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_3)
.onchange = rec_lay_user_name_3_onchange,
};
static int rec_lay_user_name_4_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_USER_NAME_TXT_4, "ascii", &user_name_arrsy[(list_page_num*5)+3]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_4)
.onchange = rec_lay_user_name_4_onchange,
};
static int rec_lay_user_name_5_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_USER_NAME_TXT_5, "ascii", &user_name_arrsy[(list_page_num*5)+4]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_5)
.onchange = rec_lay_user_name_5_onchange,
};



/***************************** 用户名列表选择界面 ************************************/

static int rec_user_name_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_name_btn_ontouch**");
    struct button *btn = (struct button *)ctr;
    const int user_name_btn[] = {
        ENC_USER_NAME_BTN_1,
        ENC_USER_NAME_BTN_2,
        ENC_USER_NAME_BTN_3,
        ENC_USER_NAME_BTN_4,
        ENC_USER_NAME_BTN_5
    };
    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        j=sizeof(user_name_btn)/sizeof(user_name_btn[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == user_name_btn[i]) {
                now_btn_user = i;
                break;
            }
        }

        ui_hide(ENC_LAY_USER_LIST);
        ui_show(ENC_LAY_USER_DETAILS);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_1)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_2)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_3)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_4)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_5)
.ontouch = rec_user_name_btn_ontouch,
};


/***************************** 用户名输入界面 ************************************/


static int rec_lay_user_scanf_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_lay_user_scanf_ontouch**");
    struct button *btn = (struct button *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if((user_name_num == 0) && (btn->elm.id != BTN_USER_BACK)){
            ui_hide(ENC_PLASE_INPUT_TXT);
        }
        switch(btn->elm.id){
        case BTN_USER_A:
            user_name[user_name_num] = num_input[0];
            break;
        case BTN_USER_B:
            user_name[user_name_num] = num_input[1];
            break;
        case BTN_USER_C:
            user_name[user_name_num] = num_input[2];
            break;
        case BTN_USER_D:
            user_name[user_name_num] = num_input[3];
            break;
        case BTN_USER_E:
            user_name[user_name_num] = num_input[4];
            break;
        case BTN_USER_F:
            user_name[user_name_num] = num_input[5];
            break;
        case BTN_USER_G:
            user_name[user_name_num] = num_input[6];
            break;
        case BTN_USER_H:
            user_name[user_name_num] = num_input[7];
            break;
        case BTN_USER_I:
            user_name[user_name_num] = num_input[8];
            break;
        case BTN_USER_J:
            user_name[user_name_num] = num_input[9];
            break;
        case BTN_USER_K:
            user_name[user_name_num] = num_input[10];
            break;
        case BTN_USER_L:
            user_name[user_name_num] = num_input[11];
            break;
        case BTN_USER_M:
            user_name[user_name_num] = num_input[12];
            break;
        case BTN_USER_N:
            user_name[user_name_num] = num_input[13];
            break;
        case BTN_USER_O:
            user_name[user_name_num] = num_input[14];
            break;
        case BTN_USER_P:
            user_name[user_name_num] = num_input[15];
            break;
        case BTN_USER_Q:
            user_name[user_name_num] = num_input[16];
            break;
        case BTN_USER_R:
            user_name[user_name_num] = num_input[17];
            break;
        case BTN_USER_S:
            user_name[user_name_num] = num_input[18];
            break;
        case BTN_USER_T:
            user_name[user_name_num] = num_input[19];
            break;
        case BTN_USER_U:
            user_name[user_name_num] = num_input[20];
            break;
        case BTN_USER_V:
            user_name[user_name_num] = num_input[21];
            break;
        case BTN_USER_W:
            user_name[user_name_num] = num_input[22];
            break;
        case BTN_USER_X:
            user_name[user_name_num] = num_input[23];
            break;
        case BTN_USER_Y:
            user_name[user_name_num] = num_input[24];
            break;
        case BTN_USER_Z:
            user_name[user_name_num] = num_input[25];
            break;
        case BTN_USER_0:
            user_name[user_name_num] = num_input[26];
            break;
        case BTN_USER_1:
            user_name[user_name_num] = num_input[27];
            break;
        case BTN_USER_2:
            user_name[user_name_num] = num_input[28];
            break;
        case BTN_USER_3:
            user_name[user_name_num] = num_input[29];
            break;
        case BTN_USER_4:
            user_name[user_name_num] = num_input[30];
            break;
        case BTN_USER_5:
            user_name[user_name_num] = num_input[31];
            break;
        case BTN_USER_6:
            user_name[user_name_num] = num_input[32];
            break;
        case BTN_USER_7:
            user_name[user_name_num] = num_input[33];
            break;
        case BTN_USER_8:
            user_name[user_name_num] = num_input[34];
            break;
        case BTN_USER_9:
            user_name[user_name_num] = num_input[35];
            break;
        case BTN_USER_KONG:
            user_name[user_name_num] = num_input[36];
            break;
        case BTN_USER_PIONT:
            user_name[user_name_num] = num_input[37];
            break;
        case BTN_USER_BACK:
            user_name_num-=2;
            printf("======== user_name_num:%d\n",user_name_num);
            if(user_name_num<0){
                user_name_num = 0;
                user_name[0] = '\0';
                spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
                ui_hide(ENC_USER_NAME_TXT);
                ui_show(ENC_PLASE_INPUT_TXT);
                return false;
            }

            break;
        case BTN_USER_OK:
            printf("======== btn ok : user name:");
            puts(user_name);
            memcpy(user_name_arrsy[name_array_num],user_name,sizeof(user_name));
            now_btn_user = name_array_num;
            name_array_num++;
            if(name_array_num>9){
                name_array_num = 0;
            }
            spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
            ui_hide(ENC_LAY_USER_INPUT);
            ui_show(ENC_LAY_USER_DETAILS);
            return false;
        }
        user_name[user_name_num+1] = '\0';
        printf("============= user name:");
        puts(user_name);
        if(user_name_num == 0){
            ui_show(ENC_USER_NAME_TXT);
        }
        ui_text_set_str_by_id(ENC_USER_NAME_TXT, "ascii", &user_name);
        if(/*(btn->elm.id != BTN_USER_BACK) &&*/ (btn->elm.id != BTN_USER_OK)){
            user_name_num++;
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BTN_USER_1)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_2)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_3)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_4)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_5)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_6)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_7)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_8)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_9)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_0)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_Q)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_W)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_E)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_R)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_T)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_Y)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_U)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_I)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_O)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_P)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_A)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_S)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_D)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_F)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_G)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_H)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_J)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_K)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_L)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_Z)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_X)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_C)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_V)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_B)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_N)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_M)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_BACK)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_KONG)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_PIONT)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_OK)
.ontouch = rec_lay_user_scanf_ontouch,
};


/***************************** 用户详情界面 消息推送按钮 ************************************/
static int rec_user_push_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_push_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        user_function_array[(list_page_num*5)+now_btn_user][0] = !user_function_array[(list_page_num*5)+now_btn_user][0];
        printf("================== %d\n",user_function_array[(list_page_num*5)+now_btn_user][0]);
        ui_pic_show_image_by_id(ENC_USER_PUSH_PIC,user_function_array[(list_page_num*5)+now_btn_user][0]);
        ui_text_show_index_by_id(ENC_USER_PUSH_TXT,user_function_array[(list_page_num*5)+now_btn_user][0]);
        break;
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_PUSH_BTN)
.ontouch = rec_user_push_btn_ontouch,
};

/***************************** 用户详情界面 管理员权限按钮 ************************************/
static int rec_user_power_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_power_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        user_function_array[(list_page_num*5)+now_btn_user][1] = !user_function_array[(list_page_num*5)+now_btn_user][1];

        ui_pic_show_image_by_id(ENC_USER_POWER_PIC,user_function_array[(list_page_num*5)+now_btn_user][1]);
        ui_text_show_index_by_id(ENC_USER_POWER_TXT,user_function_array[(list_page_num*5)+now_btn_user][1]);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_POWER_BTN)
.ontouch = rec_user_power_btn_ontouch,
};

/***************************** 用户详情界面 新建人脸按钮 ************************************/
static int rec_user_facial_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_facial_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        init_intent(&it);
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_SWITCH_WIN;
        start_app(&it);

        ui_hide(ENC_WIN);
        ui_show(ENC_FACIAL_LAY);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_FACIAL_BTN)
.ontouch = rec_user_facial_btn_ontouch,
};

/***************************** 人脸界面 返回按钮 ************************************/
static int rec_facial_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_facial_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        init_intent(&it);
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_SWITCH_WIN_OFF;
        start_app(&it);
        goto_facial_page_flag = 1;
        ui_hide(ENC_FACIAL_LAY);
        ui_show(ENC_WIN);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FACIAL_RETURN_BTN)
.ontouch = rec_facial_return_btn_ontouch,
};


/***************************** 记录查询界面 ************************************/
const int lay_record_list[]={
    ENC_RECORD_INFOR_LIST_1,
    ENC_RECORD_INFOR_LIST_2,
    ENC_RECORD_INFOR_LIST_3,
    ENC_RECORD_INFOR_LIST_4,
    ENC_RECORD_INFOR_LIST_5,
};

static int rec_lay_record_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_show(ENC_RECORD_INFOR);
        for(int i=0;i<name_array_num;i++){
            ui_show(lay_record_list[i]);
            if(i>4){
                break;
            }
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_RECORD_PAGE)
.onchange = rec_lay_record_page_onchange,
};


/***************************** 记录查询界面 返回按钮 ************************************/
static int rec_record_page_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_record_page_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_LAY_RECORD_PAGE);
        ui_show(ENC_LAY_PAGE);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_RETURN)
.ontouch = rec_record_page_return_btn_ontouch,
};


/*****************************记录查询年月日 ************************************/
static int timer_record_ymd_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        temp = ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1;
        if(temp!=last_temp){
            last_temp = temp;
            ui_text_show_index_by_id(REC_TXT_WEEKDAY, last_temp);
        }
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_TIME_YMD)
.onchange = timer_record_ymd_onchange,
};


/****************************记录时间控件动作 ************************************/
static int timer_1_record_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_1)
.onchange = timer_1_record_infor_onchange,
};

static int timer_2_record_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_2)
.onchange = timer_2_record_infor_onchange,
};

static int timer_3_record_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_3)
.onchange = timer_3_record_infor_onchange,
};

static int timer_4_record_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_4)
.onchange = timer_4_record_infor_onchange,
};

static int timer_5_record_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_5)
.onchange = timer_5_record_infor_onchange,
};


/***************************** 记录列表显示 ************************************/
static int rec_record_infor_list_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_RECORD_INFOR_TXT_1, "ascii", &user_name_arrsy[(list_page_num*5)+0]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_1)
.onchange = rec_record_infor_list_1_onchange,
};
static int rec_record_infor_list_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_RECORD_INFOR_TXT_2, "ascii", &user_name_arrsy[(list_page_num*5)+1]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_2)
.onchange = rec_record_infor_list_2_onchange,
};
static int rec_record_infor_list_3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_RECORD_INFOR_TXT_3, "ascii", &user_name_arrsy[(list_page_num*5)+2]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_3)
.onchange = rec_record_infor_list_3_onchange,
};
static int rec_record_infor_list_4_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_RECORD_INFOR_TXT_4, "ascii", &user_name_arrsy[(list_page_num*5)+3]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_4)
.onchange = rec_record_infor_list_4_onchange,
};
static int rec_record_infor_list_5_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_str_by_id(ENC_RECORD_INFOR_TXT_5, "ascii", &user_name_arrsy[(list_page_num*5)+4]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_5)
.onchange = rec_record_infor_list_5_onchange,
};



/***************************** 系统信息界面 返回按钮 ************************************/
static int rec_sys_info_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_sys_info_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_LAY_SYS_INFO_PAGE);
        ui_show(ENC_LAY_PAGE);
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SYS_INFO_RETURN)
.ontouch = rec_sys_info_return_btn_ontouch,
};

/***************************** 门锁配置界面 开关按钮 ************************************/
static int rec_door_lock_onoff_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_door_lock_onoff_btn_ontouch**");
    struct button *btn = (struct button *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        switch(btn->elm.id){
        case ENC_LOCK_LIST_BTN_2:
            lock_array[1] = !lock_array[1];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_2,lock_array[1]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_2,lock_array[1]);
            break;
        case ENC_LOCK_LIST_BTN_3:
            lock_array[2] = !lock_array[2];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_3,lock_array[2]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_3,lock_array[2]);
            break;
        case ENC_LOCK_LIST_BTN_4:
            lock_array[3] = !lock_array[3];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_4,lock_array[3]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_4,lock_array[3]);
            break;
        case ENC_LOCK_LIST_BTN_5:
            lock_array[4] = !lock_array[4];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_5,lock_array[4]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_5,lock_array[4]);
            break;
        case ENC_LOCK_LIST_BTN_6:
            lock_array[5] = !lock_array[5];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_6,lock_array[5]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_6,lock_array[5]);
            break;
        case ENC_LOCK_LIST_BTN_7:
            lock_array[6] = !lock_array[6];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_7,lock_array[6]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_7,lock_array[6]);
            break;
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_2)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_3)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_4)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_5)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_6)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_7)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};


/***************************** 门锁配置界面初始化 ************************************/
static int rec_lay_lock_list_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_2,lock_array[1]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_2,lock_array[1]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_3,lock_array[2]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_3,lock_array[2]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_4,lock_array[3]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_4,lock_array[3]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_5,lock_array[4]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_5,lock_array[4]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_LOCK_LIST_1)
.onchange = rec_lay_lock_list_1_onchange,
};

void move_pic_timeout_func()
{
    ui_opt_ctr_move(ENC_LOCK_LEV_PIC,lock_array[7]*10,0);
}
static int rec_lay_lock_list_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_6,lock_array[5]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_6,lock_array[5]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_7,lock_array[6]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_7,lock_array[6]);

//        ui_show(ENC_LOCK_LEV_PIC);
        printf("========= %d , %s\n",lock_array[7],move_num_str);
        if(lock_array[7] == 0){
            ui_text_set_str_by_id(ENC_LOCK_LEV_NUM_TXT,"ascii","0");
        }else{
            ui_text_set_str_by_id(ENC_LOCK_LEV_NUM_TXT,"ascii",move_num_str);
        }
//        ui_opt_ctr_move(ENC_LOCK_LEV_PIC,lock_array[7]*10,0);
        sys_timeout_add(NULL,move_pic_timeout_func,100);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_LOCK_LIST_2)
.onchange = rec_lay_lock_list_2_onchange,
};

/***************************** 系统信息界面 翻页按钮 ************************************/
u8 door_lock_page_flag = 0;         //记录现在在哪个设置页面
static int rec_lock_list_page_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_lock_list_page_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        door_lock_page_flag = !door_lock_page_flag;
        if(door_lock_page_flag == 0){
            ui_hide(ENC_LAY_LOCK_LIST_1);
            ui_show(ENC_LAY_LOCK_LIST_2);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PAGE_PIC,1);
        }else{
            ui_hide(ENC_LAY_LOCK_LIST_2);
            ui_show(ENC_LAY_LOCK_LIST_1);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PAGE_PIC,0);
        }
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_PAGE_BTN)
.ontouch = rec_lock_list_page_btn_ontouch,
};


/***************************** 系统信息界面 返回按钮 ************************************/
static int rec_door_lock_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_door_lock_return_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(ENC_LAY_DOOR_LOCK_PAGE);
        ui_show(ENC_LAY_PAGE);
        door_lock_page_flag = 0;
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_DOOR_LOCK_RETURN_BTN)
.ontouch = rec_door_lock_return_btn_ontouch,
};


/***************************** 系统信息界面 自动反锁阈值调整 ************************************/
#define SLID_GAP  10  //每一项的间隔(滑块长度/项目数)
#define SLID_ITEM 40  //项目数

static int rec_door_lock_lev_move_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_door_lock_lev_move_btn_ontouch**");
    static s16 x_pos_down = 0;
    static int i = 0;
    static int tmp = 0;
    int move_point = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        x_pos_down = e->pos.x;
        i = lock_array[7];
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        memset(move_num_str,' ',sizeof(move_num_str));
        s16 x_pos_now = e->pos.x;
        s16 x_pos_ch = x_pos_now-x_pos_down;
        if (x_pos_ch < SLID_GAP && x_pos_ch > -SLID_GAP) {
            return false;
        }
//        printf("============ x_pos_now:%d , x_pos_ch:%d",x_pos_now,x_pos_now);
        tmp = i + x_pos_ch / SLID_GAP;
//        printf("============ tmp:%d\n",tmp);
        if (tmp > SLID_ITEM - 1) {
            tmp = SLID_ITEM - 1;
        }else if (tmp < 0) {
            tmp = 0;
        }
        if(tmp==i){
            return false;
        }
        move_point = tmp - lock_array[7];
        lock_array[7] = tmp;
        sprintf(move_num_str,"%d",lock_array[7]);
        printf("============ move: %d,  num: %d \n",move_point,lock_array[7]);
//        puts(move_num_str);
        ui_text_set_str_by_id(ENC_LOCK_LEV_NUM_TXT,"ascii",move_num_str);
        ui_opt_ctr_move(ENC_LOCK_LEV_PIC,move_point*10,0);
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LEV_BTN)
.ontouch = rec_door_lock_lev_move_btn_ontouch,
};

/***************************** 关机按键 ************************************/
static int rec_rec_power_up_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_rec_power_up_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        sys_power_shutdown();
        spec_uart_send(create_packet(voice,key_sound),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(REC_POWER_UP_BTN)
.ontouch = rec_rec_power_up_ontouch,
};

/***************************** 屏保按键 ************************************/
static int rec_rec_lock_up_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_rec_lock_up_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        ui_lcd_light_off();
        spec_uart_send(create_packet(voice,locked),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(RES_LOCK_BTN)
.ontouch = rec_rec_lock_up_ontouch,
};

/***************************** 铃声按键 ************************************/
static int rec_rec_ling_up_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_rec_ling_up_ontouch**");
    static tmp = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        printf("========= tmp:%d\n",tmp);
        ui_pic_show_image_by_id(PIC_BAT_REC,tmp);
        tmp++;
        if(tmp == 5){
            tmp = 0;
        }
        spec_uart_send(create_packet(voice,door_bell),PACKET_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(RES_LING_BTN)
.ontouch = rec_rec_ling_up_ontouch,
};

/***************************** 顶部UI界面 ************************************/

static int rec_enc_back_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        enc_back_flag = 1;
        break;
    case ON_CHANGE_RELEASE:
        enc_back_flag = 0;
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BACK)
.onchange = rec_enc_back_lay_onchange,
};

/***************************** 顶部UI界面 ************************************/
static int rec_enc_up_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        if(enc_back_flag){
            ui_show(REC_POWER_UP_BTN);
            ui_show(REC_POWER_UP_PIC);
        }else{
            ui_hide(REC_POWER_UP_BTN);
            ui_hide(REC_POWER_UP_PIC);
        }

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_UP_LAY)
.onchange = rec_enc_up_lay_onchange,
};

/***************************** 设置主界面 ************************************/
static int rec_enc_set_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        sys_timeout_add(NULL,reset_up_ui_func,100);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_LAY)
.onchange = rec_enc_set_lay_onchange,
};
REGISTER_UI_EVENT_HANDLER(ENC_LAY_PAGE)
.onchange = rec_enc_set_lay_onchange,
};
REGISTER_UI_EVENT_HANDLER(ENC_LAY_SYS_INFO_PAGE)
.onchange = rec_enc_set_lay_onchange,
};
REGISTER_UI_EVENT_HANDLER(ENC_LAY_DOOR_LOCK_PAGE)
.onchange = rec_enc_set_lay_onchange,
};

#endif
