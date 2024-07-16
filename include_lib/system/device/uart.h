#ifndef DEVICE_UART_H
#define DEVICE_UART_H

#include "typedef.h"
#include "device/device.h"
#include "generic/ioctl.h"
#include "system/task.h"

#define UART_DMA_SUPPORT 	0x00000001
#define UART_TX_USE_DMA 	0x00000003
#define UART_RX_USE_DMA 	0x00000005
#define UART_DEBUG 			0x00000008

struct uart_outport {
    u8  tx_pin;
    u8  rx_pin;
    u16 value;
};

extern void putbyte(char a);




enum uart_clk_src {
    LSB_CLK,
    OSC_CLK,
    PLL_48M,
};

/**************勿改***************/
#define PORT_BASE_VALUE	0x10000 //该值需要大于16 * 8

//	    PORT_TX_RX
#define PORTA_10_11		(PORT_BASE_VALUE + 0)
#define PORTA_12_13		(PORT_BASE_VALUE + 0)
#define PORTH_4_3		(PORT_BASE_VALUE + 0)

#define PORTG_6_7		(PORT_BASE_VALUE + 1)
#define PORTH_2_5		(PORT_BASE_VALUE + 1)
#define PORTF_0_1		(PORT_BASE_VALUE + 1)

#define PORTH_12_11		(PORT_BASE_VALUE + 2)
#define PORTH_9_10		(PORT_BASE_VALUE + 2)
#define PORTD_0_1		(PORT_BASE_VALUE + 2)
#define PORTH_12_13		(PORT_BASE_VALUE + 2)

#define PORTD_4_5		(PORT_BASE_VALUE + 3)
#define PORTF_2_3		(PORT_BASE_VALUE + 3)
#define PORTH_0_1		(PORT_BASE_VALUE + 3)

#define PORT_MIN_NUM	(PORT_BASE_VALUE + 0)
#define PORT_MAX_NUM	(PORT_BASE_VALUE + 4)
#define PORT_REMAP		(PORT_BASE_VALUE + 5)

#define OUTPUT_CHANNEL0	(PORT_BASE_VALUE + 0)
#define OUTPUT_CHANNEL1	(PORT_BASE_VALUE + 1)
#define OUTPUT_CHANNEL2	(PORT_BASE_VALUE + 2)
#define OUTPUT_CHANNEL3	(PORT_BASE_VALUE + 3)
#define INPUT_CHANNEL0	(PORT_BASE_VALUE + 4)
#define INPUT_CHANNEL1	(PORT_BASE_VALUE + 5)
#define INPUT_CHANNEL2	(PORT_BASE_VALUE + 6)
#define INPUT_CHANNEL3	(PORT_BASE_VALUE + 7)

#define PORT_GET_VALUE(port)	(port - PORT_BASE_VALUE)
#define CHANNEL_GET_VALUE(port)	(port - PORT_BASE_VALUE)

/*io_map_port:
  IO_PORTA_00 - IO_PORTA_13,
  IO_PORTD_00 - IO_PORTD_15,
  IO_PORTF_00 - IO_PORTF_07,
  IO_PORTG_00 - IO_PORTG_15,
  IO_PORTH_00 - IO_PORTH_13
  */
/*********************************/

/*
enum _uart_port0{
    //uart0
    PORTA_10_11 = 0,
    PORTG_6_7,
    PORTH_12_11,
    PORTD_4_5,
};
enum _uart_port1{
    //uart1
    PORTA_11_12 = 0,
    PORTH_2_5,
    PORTH_9_10,
    PORTF_2_3,
};
enum _uart_port2{
    //uart2
    PORTH_4_3 = 0,
    PORTF_0_1,
    PORTD_0_1,
    PORTH_0_1,
};

enum _uart_port3{
    //uart3
    PORTA_10_11 = 0,
    PORTG_6_7,
    PORTH_12_13,
    PORTD_4_5,
};
*/

struct uart_platform_data {
    u8 *name;

    u8  irq;
    int tx_pin;//不配置需设置-1
    int rx_pin;//不配置需设置-1
    int flags;
    u32 baudrate;

    int port;//enum _uart_port0-3的值
    int output_channel;
    int input_channel;
    u32 max_continue_recv_cnt;
    u32 idle_sys_clk_cnt;
    enum uart_clk_src clk_src;
};

enum {
    UART_CIRCULAR_BUFFER_WRITE_OVERLAY = -1,
    UART_RECV_TIMEOUT = -2,
    UART_RECV_EXIT = -3,
};

#define UART_MAGIC                          'U'
#define UART_FLUSH                          _IO(UART_MAGIC,1)
#define UART_SET_RECV_ALL                   _IOW(UART_MAGIC,2,bool)
#define UART_SET_RECV_BLOCK                 _IOW(UART_MAGIC,3,bool)
#define UART_SET_RECV_TIMEOUT               _IOW(UART_MAGIC,4,u32)
#define UART_SET_RECV_TIMEOUT_CB            _IOW(UART_MAGIC,5,int (*)(void))
#define UART_GET_RECV_CNT                   _IOR(UART_MAGIC,6,u32)
#define UART_START                          _IO(UART_MAGIC,7)
#define UART_SET_CIRCULAR_BUFF_ADDR         _IOW(UART_MAGIC,8,void *)
#define UART_SET_CIRCULAR_BUFF_LENTH        _IOW(UART_MAGIC,9,u32)


#define UART_PLATFORM_DATA_BEGIN(data) \
    static const struct uart_platform_data data = {


#define UART_PLATFORM_DATA_END() \
    };


struct uart_device {
    char *name;
    const struct uart_operations *ops;
    struct device dev;
    const struct uart_platform_data *priv;
    OS_MUTEX mutex;
};




struct uart_operations {
    int (*init)(struct uart_device *);
    int (*read)(struct uart_device *, void *buf, u32 len);
    int (*write)(struct uart_device *, void *buf, u16 len);
    int (*ioctl)(struct uart_device *, u32 cmd, u32 arg);
    int (*close)(struct uart_device *);
};



#define REGISTER_UART_DEVICE(dev) \
    static const struct uart_device dev sec(.uart)

extern struct uart_device uart_device_begin[], uart_device_end[];

#define list_for_each_uart_device(p) \
    for (p=uart_device_begin; p<uart_device_end; p++)



extern const struct device_operations uart_dev_ops;



#endif
