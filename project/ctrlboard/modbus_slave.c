#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "SDL/SDL.h"
#include "crc16.h"

/* #define DEBUG_LOG */
/* #define DEBUG_MODBUS_SLAVE */

#if defined(_WIN32) || defined(DEBUG_LOG)
#define PRINT_LOG(x) do {printf("LOG: "); printf x;} while(0)
#else
#define PRINT_LOG(x) do {} while(0)
#endif

#define COM_UART_MODBUS     0xaa
#define UART_BUFFER_SIZE    50
#define COM_PORT ITP_DEVICE_UART0
#define COM_DEVICE itpDeviceUart0

static unsigned char send_buf[UART_BUFFER_SIZE];
static unsigned char rec_buf[UART_BUFFER_SIZE];
static int rec_index = 0;

#ifdef DEBUG_MODBUS_SLAVE
    #define PRINT_MODBUS(x) do {printf x;} while(0)
    #define PRINT_RECEIPT() do {PRINT_MODBUS(("slave recv:")); for (i = 0; i < rec_index; i++) PRINT_MODBUS((" 0x%02x", rec_buf[i]));  PRINT_MODBUS(("\n"));} while(0)
    #define PRINT_SENT()    do {PRINT_MODBUS(("slave send:")); for (i = 0; i < send_base->len; i++) PRINT_MODBUS((" 0x%02x", send_buf[i])); PRINT_MODBUS(("\n"));} while(0)
#else
    #define PRINT_MODBUS(x) do {} while(0)
    #define PRINT_RECEIPT() do {} while(0)
    #define PRINT_SENT()    do {} while(0)
#endif

#pragma pack(1)

typedef enum {
    REG_START = 0x00,

    REG_BARCODE = REG_START,  // 0x00 ~ 0x0b
    REG_BARCODE_END = REG_BARCODE + 0x0c, /* 13 */
    REG_SUPPLIER_INFO,        // 0x0c ~ 0x0f
    REG_SUPPLIER_INFO_END = REG_SUPPLIER_INFO + 0x03, /* 4 */

    REG_FIRMWARE_M_VER,         // 0x11 ~ 0x12
    REG_FIRMWARE_S_VER,
    REG_HARDWARE_M_VER,         // 0x13 ~ 0x14
    REG_HARDWARE_S_VER,
} reg_item_t;

typedef enum {
    SLAVE_DEVICE_ID_NLL = 0,    /* 0 */
    SLAVE_DEVICE_ID_POWERBOARD, /* 1 */
        /* 2 */
        /* 3 */
        /* 4 */
        /* 5 */
        /* 6 */
        /* 7 */
        /* 8 */
        /* 9 */
        /* a */
        /* b */
        /* c */
        /* d */
        /* e */
    SLAVE_DEVICE_ID_END             /* f */
} slave_device_id_t;

/* modbus_protocol_stack */

typedef enum {
    OP_SKIP_READ = 0,   /* 0 */
    OP_SKIP_WRITE,      /* 1 */
    OP_FURTHER_READ,    /* 2 */
    OP_FURTHER_WRITE,   /* 3 */
    OP_SKIP_READ_WRITE,    /* 4 */
    OP_FURTHER_READ_WRITE, /* 5 */
    OP_BITS_OR,         /* 6 */
    OP_BITS_AND,        /* 7 */
    /* OP_BITS_XOR,     null */
} op_code_t;

typedef enum {
    FB_NULL         = 0x00,  /* 寄存器操作无错误 */
    FB_OPCODE_FAULT = 0x02,  /* 功能码错误 */
    FB_OVERLENGTH   = 0x04,  /* 数据包长度过长 */
    FB_ID_REPEAT,            /* 数据包ID与上一帧数据重复 */
    FB_WRITE_FAULT,          /* 寄存器写入出错 */
    FB_READ_FAULT,           /* 寄存器读取出错 */
    FB_ADDR_FAULT,           /* 寄存器地址错误 */
    FB_WRITE_READ_FAULTLESS, /* 写读无错回答 */
    FB_MAC_FAULT,            /* 设备故障 */
    FB_BUSY_FAULT,           /* 设备忙碌 */
    FB_CRC_FAULT,            /* CRC校验错误 */
} fb_code_t;

typedef struct{
    uint8_t synchfield;       /* synchfield */
    uint8_t slave; // : 4;
    uint8_t sn; // : 4;
    uint8_t len;
    uint8_t product; //
    uint8_t cmd;
    uint8_t page;
} base_frame_t;

typedef struct {
    uint8_t reg;
    uint8_t val;
} op_skip_t;

typedef struct {
    uint8_t start;
    uint8_t num;
} op_further_read_t;

typedef struct {
    uint8_t start;
    uint8_t start_var;
} op_further_write_t;

typedef struct{
    uint16_t crc16;
} error_check_t;

static inline uint16_t bswap_16(uint16_t v)
{
    return ((v & 0xff) << 8) | (v >> 8);
}

static bool read_rec(int target)
{
    int cnt = 200;
    if (target < sizeof(rec_buf)) {
        while (rec_index < target) {
            usleep(1000);
            rec_index += read(COM_PORT, rec_buf + rec_index, UART_BUFFER_SIZE - rec_index);
            if (--cnt <= 0) break;
        }
        PRINT_MODBUS(("%dms - ", 200 - cnt));
    }
    if (cnt > 0) return true;
    return false;
}

static bool read_synchfield(void)
{
    int cnt = 200;
    do {
        usleep(1000);
        read(COM_PORT, rec_buf, 1);
        if (--cnt <= 0) break;
    } while (rec_buf[0] != COM_UART_MODBUS);
    PRINT_MODBUS(("%dms - ", 200 - cnt));
    if (cnt > 0) return true;
    return false;
}

/*---------------------------- com_check --------------------------------*/

struct {
    uint16_t cnt;
    uint8_t furwr_flag;
    uint8_t buf[6];
} com_check_para = {0,0,{0,0,0,0,0,0}};

#define COM_TIMEOUT 6000 //10s

static void com_check_and(uint8_t bits, uint8_t val)
{
    if (com_check_para.furwr_flag != bits) return;
    com_check_para.furwr_flag++;
    com_check_para.buf[bits] = val;
}

static uint8_t com_check_furwr_flag(uint8_t bits)
{
    return com_check_para.furwr_flag == bits;
}

void com_check_origin(void)
{
    com_check_para.cnt = 0;
    com_check_para.furwr_flag = 0;
}

/*------------------------------ fun ------------------------------------*/

typedef bool (*pfun_reg)(uint8_t *content, reg_item_t reg);

typedef struct {
    uint8_t addr;
    pfun_reg read;
    pfun_reg write;
} reg_map_t;

/* --- */
static bool reg_void_fail(uint8_t *content, reg_item_t reg)
{
    if (reg);
    *content = *content;
    return false;
}

#define reg_void_read  reg_void_fail
#define reg_void_write reg_void_fail

#include "mach_reg_map.h"

static void com_check_report(void)
{
    if (com_check_para.cnt > COM_TIMEOUT) {
    } else {
        com_check_para.cnt++;
    }
}

static fb_code_t reg_addr_match(uint8_t reg_addr, uint8_t *map_index)
{
    uint8_t i = 0;
    do {
        if (reg_addr == mach_reg_map[i].addr) {
            *map_index = i;
            return true;
        }
    } while (++i < sizeof(mach_reg_map)/sizeof(reg_map_t));
    return false;
}

/* op */

static fb_code_t skip_read_con(uint8_t *addr, uint8_t *result, int length, int *index)
{
    uint8_t map_index = 0;
    *index = 0;
    do {
        if (reg_addr_match(*addr++, &map_index) == true) {
            if ((*mach_reg_map[map_index].read)(result, mach_reg_map[map_index].addr) == false)
                return FB_READ_FAULT;
            result++;
        } else {
            return FB_ADDR_FAULT;
        }
    } while (++(*index) < length);
    return FB_NULL;
}

static fb_code_t skip_write_con(op_skip_t *pair, int length)
{
    uint8_t i = 0, map_index = 0;
    do {
        if (reg_addr_match(pair->reg, &map_index) == true) {
            if ((*mach_reg_map[map_index].write)(&pair->val, mach_reg_map[map_index].addr) == false)
                return FB_WRITE_FAULT;
        } else {
            return FB_ADDR_FAULT;
        }
        pair++;
    } while (++i < length/sizeof(op_skip_t));
    return FB_NULL;
}

static fb_code_t bits_or_con(op_skip_t *pair)
{
    uint8_t map_index = 0, value = 0;
    if (reg_addr_match(pair->reg, &map_index) == true) {
        if ((*mach_reg_map[map_index].read)(&value, mach_reg_map[map_index].addr) == false)
            return FB_READ_FAULT;
        value |= pair->val;
        if ((*mach_reg_map[map_index].write)(&value, mach_reg_map[map_index].addr) == false)
            return FB_WRITE_FAULT;
    } else {
        return FB_ADDR_FAULT;
    }
    return FB_NULL;
}

static fb_code_t bits_and_con(op_skip_t *pair)
{
    uint8_t map_index = 0, value = 0;
    if (reg_addr_match(pair->reg, &map_index) == true) {
        if ((*mach_reg_map[map_index].read)(&value, mach_reg_map[map_index].addr) == false)
            return FB_READ_FAULT;
        value &= pair->val;
        if ((*mach_reg_map[map_index].write)(&value, mach_reg_map[map_index].addr) == false)
            return FB_WRITE_FAULT;
    } else {
        return FB_ADDR_FAULT;
    }
    return FB_NULL;
}

static fb_code_t further_read_con(op_further_read_t *pair, uint8_t *result, int *index)
{
    uint8_t map_index = 0;
    uint8_t addr = pair->start;
    *index = 0;
    do {
        if (*index >= sizeof(rec_buf) - sizeof(base_frame_t) - sizeof(error_check_t))
            return FB_OVERLENGTH;
        if (reg_addr_match(addr++, &map_index) == true) {
            if ((*mach_reg_map[map_index].read)(result, mach_reg_map[map_index].addr) == false)
                return FB_READ_FAULT;
            result++;
        } else {
            return FB_ADDR_FAULT;
        }
    } while (++(*index) < pair->num);
    return FB_NULL;
}

static fb_code_t further_write_con(op_further_write_t *pair, int length)
{
    uint8_t i = 0, map_index = 0;
    uint8_t addr = pair->start;
    uint8_t *pvar = &pair->start_var;
    do {
        if (reg_addr_match(addr++, &map_index) == true) {
            if ((*mach_reg_map[map_index].write)(pvar, mach_reg_map[map_index].addr) == false)
                return FB_WRITE_FAULT;
            pvar++;
        } else {
            return FB_ADDR_FAULT;
        }
    } while (++i < length);
    return FB_NULL;
}

static uint8_t parse_cmd(op_code_t cmd, int rec_length, int *send_length, bool flag)
{
    fb_code_t fb;
    uint8_t *source = rec_buf + sizeof(base_frame_t);
    uint8_t *result = send_buf + sizeof(base_frame_t);
    op_further_read_t *further_read = (op_further_read_t*)source;

    switch (cmd) {
    case OP_SKIP_READ:
        fb = skip_read_con(source, result, rec_length, send_length);
        break;
    case OP_SKIP_WRITE:
        fb = skip_write_con((op_skip_t*)source, rec_length);
        *send_length = 0;
        break;
    case OP_BITS_OR:
        fb = bits_or_con((op_skip_t*)source);
        *send_length = 0;
        break;
    case OP_BITS_AND:
        fb = bits_and_con((op_skip_t*)source);
        *send_length = 0;
        break;
    case OP_FURTHER_READ:
        fb = further_read_con(further_read, result, send_length);
        break;
    case OP_FURTHER_WRITE:
        fb = further_write_con((op_further_write_t*)source, rec_length - 1);
        *send_length = 0;
        break;
    default:
        fb = FB_OPCODE_FAULT;
        *send_length = 0;
        break;
    }
    if (flag == false) fb = FB_ID_REPEAT;
    return (uint8_t)fb;
}

static void* modbus_task(void* arg)
{
    static uint8_t sn_pre = 0xff;
    base_frame_t *send_base = (base_frame_t*)send_buf;
    base_frame_t *rec_base  = (base_frame_t*)rec_buf;
    error_check_t *ec;
    int send_length;
    bool flag = true;

    PRINT_LOG(("%s\n", __func__));

#ifndef _WIN32
    itpRegisterDevice(COM_PORT, &COM_DEVICE);
#endif // _WIN32

    ioctl(COM_PORT, ITP_IOCTL_INIT, NULL);

    for (;;) {
#ifdef DEBUG_MODBUS_SLAVE
        int i;
#endif
        rec_base->synchfield = 0;
        usleep(30000);
#ifndef _WIN32
        PRINT_MODBUS(("\n"));

        if (read_synchfield() == false) goto _out_;
        if (rec_base->synchfield != COM_UART_MODBUS)       {PRINT_LOG(("slave rec false! synchfield = 0x%02x\n", rec_base->synchfield)); goto _out_;}
        rec_index = 1;
        if (read_rec(sizeof(base_frame_t)) == false)       {PRINT_LOG(("slave rec lack of base frame! rec_index = %d\n", rec_index)); goto _out_;}
        if (rec_base->slave != SLAVE_DEVICE_ID_POWERBOARD) {PRINT_LOG(("slave rec DEVICE_ID false!\n")); goto _out_;}
        if (read_rec(rec_base->len) == false)              {PRINT_LOG(("slave rec lack of length! rec_index = %d\n, rec_index")); goto _out_;}

        ec = (error_check_t*)(rec_buf + rec_base->len - sizeof(error_check_t));
        if (ec->crc16 == bswap_16(calc_crc16(rec_buf, rec_base->len - sizeof(error_check_t)))) {
            com_check_origin();

            if (sn_pre == rec_base->sn) flag = false;
            else sn_pre = rec_base->sn;

            send_base->synchfield = COM_UART_MODBUS;
            send_base->sn = sn_pre;
            send_base->slave = rec_base->slave;
            send_base->cmd = parse_cmd((op_code_t)rec_base->cmd,
                rec_base->len - sizeof(base_frame_t) - sizeof(error_check_t),
                &send_length,
                flag);
            send_base->product = rec_base->product;
            send_length += sizeof(base_frame_t);
            send_base->len = (uint8_t)(send_length + sizeof(error_check_t));

            ec = (error_check_t*)(send_buf + send_length);
            ec->crc16 = bswap_16(calc_crc16(send_buf, send_length));

            PRINT_SENT();
            write(COM_PORT, send_buf, send_base->len);

        } else {
            PRINT_LOG(("[CHECKSUM] Wrong, slave checkSum=0x%04x\n", ec->crc16));
        }
_out_:
        PRINT_RECEIPT();
        com_check_report();
#endif // _WIN32
    }
    return NULL;
}

#define STACK_SIZE 5000

static bool inited, quit;
static pthread_t task;
static pthread_attr_t attr;

void modbus_slave_init(void)
{
    inited = quit = false;
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, STACK_SIZE);

    pthread_create(&task, &attr, modbus_task, NULL);

    inited = true;
    PRINT_LOG(("%s\n", __func__));
}

void modbus_slave_exit(void)
{
    if (!inited)
        return;

    quit = true;

    pthread_join(task, NULL);
    PRINT_LOG(("%s\n", __func__));
}
