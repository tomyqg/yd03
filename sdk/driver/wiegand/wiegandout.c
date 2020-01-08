#include "wiegandout.h"
#include <sys/time.h>
#include "ite/ith.h"
#include "wiegand/wiegand.h"
#include "wiegand/wiegandout.h"



typedef enum WIEGAND_OUT_REGISTER_TAG
{
	WOMR = 0x0,
	WOSR = 0x4,
	WISR = 0x8,
	WOER = 0xC,
}WIEGAND_OUT_REGISTER;

static unsigned int _wiegand_out_get_address(WIEGAND_OUT_ID id, WIEGAND_OUT_REGISTER reg)
{
#define WIEGAND_OUT_BIT 0x30
#define WIEGAND_OUT_DEVICE_REG_INTERVAL 0x10
	return WIEGAND_BASE + WIEGAND_OUT_BIT + (id*WIEGAND_OUT_DEVICE_REG_INTERVAL) + reg;
}

static void _wiegand_out_reset(WIEGAND_OUT_ID id)
{
#define WIEGAND_OUT_RST_ENABLE	(1 << 31)
#define WIEGAND_OUT_RST_DISABLE (0 << 31)
#define WIEGAND_OUT_RST_MASK  	(1 << 31)

	ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_RST_ENABLE, WIEGAND_OUT_RST_MASK);
	usleep(1000);
	ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_RST_DISABLE, WIEGAND_OUT_RST_MASK);
}

static void _wiegand_out_controller_enable(WIEGAND_OUT_ID id, int enable)
{
#define WIEGAND_OUT_CTRL_ENABLE	 (1 << 30)
#define WIEGAND_OUT_CTRL_DISABLE (0 << 30)
#define WIEGAND_OUT_CTRL_MASK	 (1 << 30)

	if (enable)
		ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_CTRL_ENABLE, WIEGAND_OUT_CTRL_MASK);
	else
		ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_CTRL_DISABLE, WIEGAND_OUT_CTRL_MASK);
}

static void _wiegand_out_loopback(WIEGAND_OUT_ID id, WIEGAND_LOOPBACK_DIRECTION direction)
{
#define WIEGAND_OUT_LOOPBACK_I2O	(1 << 28)
#define WIEGAND_OUT_LOOPBACK_O2I	(2 << 28)
#define WIEGAND_OUT_LOOPBACK_BYPASS (0 << 28)
#define WIEGAND_OUT_LOOPBACK_MASK	(3 << 28)

	switch (direction)
	{
	case I2O:
		ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_LOOPBACK_I2O, WIEGAND_OUT_LOOPBACK_MASK);
		break;
	case O2I:
		ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_LOOPBACK_O2I, WIEGAND_OUT_LOOPBACK_MASK);
		break;
	case BYPASS:
		ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), WIEGAND_OUT_LOOPBACK_BYPASS, WIEGAND_OUT_LOOPBACK_MASK);
		break;
	default:
		break;
	}
}

static void _wiegand_out_pulse(WIEGAND_OUT_ID id, int value)
{

#define WIEGAND_OUT_PULSE_BIT	0
#define WIEGAND_OUT_PULSE_MASK	0xfffff << WIEGAND_OUT_PULSE_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WOMR), value << WIEGAND_OUT_PULSE_BIT, WIEGAND_OUT_PULSE_MASK);
}

static void _wiegand_out_set_bit_count(WIEGAND_OUT_ID id, int bitcnt)
{
#define WIEGAND_OUT_BIT_COUNT_BIT	24
#define WIEGAND_OUT_BIT_COUNT_MASK	0xff << WIEGAND_OUT_PULSE_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WOSR), bitcnt << WIEGAND_OUT_BIT_COUNT_BIT, WIEGAND_OUT_BIT_COUNT_MASK);
}

static void _wiegand_out_set_pulseprescale(WIEGAND_OUT_ID id, int value)
{
#define WIEGAND_OUT_PULSE_PRESCALE_BIT	0
#define WIEGAND_OUT_PULSE_PRESCALE_MASK	0xffff << WIEGAND_OUT_PULSE_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WOSR), value << WIEGAND_OUT_PULSE_PRESCALE_BIT, WIEGAND_OUT_PULSE_PRESCALE_MASK);
}

static void _wiegand_out_set_bit_order(WIEGAND_OUT_ID id, WIEGAND_OUT_BIT_ORDER order)
{
#define WIEGAND_OUT_BIT_ORDER_BIT	31
#define WIEGAND_OUT_BIT_ORDER_MASK	0x1 << WIEGAND_OUT_BIT_ORDER_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WISR), order << WIEGAND_OUT_BIT_ORDER_BIT, WIEGAND_OUT_BIT_ORDER_MASK);
}

static void _wiegand_out_set_fifo_state(WIEGAND_OUT_ID id, int value)
{
#define WIEGAND_OUT_FIFO_STATE_BIT	16
#define WIEGAND_OUT_FIFO_STATE_MASK	0xff << WIEGAND_OUT_FIFO_STATE_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WISR), value << WIEGAND_OUT_FIFO_STATE_BIT, WIEGAND_OUT_FIFO_STATE_MASK);
}

static void _wiegand_out_set_uartdiv(WIEGAND_OUT_ID id, int value)
{
#define WIEGAND_OUT_UARTDIV_BIT	0
#define WIEGAND_OUT_UARTDIV_MASK	0xffff << WIEGAND_OUT_UARTDIV_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WISR), value << WIEGAND_OUT_UARTDIV_BIT, WIEGAND_OUT_UARTDIV_MASK);
}

static void _wiegand_out_set_interval(WIEGAND_OUT_ID id, int value)
{
#define WIEGAND_OUT_INTERVAL_BIT	0
#define WIEGAND_OUT_INTERVAL_MASK	0xfffff << WIEGAND_OUT_INTERVAL_BIT

	ithWriteRegMaskA(_wiegand_out_get_address(id, WOER), value << WIEGAND_OUT_INTERVAL_BIT, WIEGAND_OUT_INTERVAL_MASK);
}

void init_wiegandout_controller(WIEGAND_OUT_ID id, int bitcnt, WIEGAND_LOOPBACK_DIRECTION dir, WIEGAND_OUT_BIT_ORDER order)
{
	unsigned int busclk = ithGetBusClock();
	unsigned int pulse = busclk * 0.0003325;			// Set pulse 0.3325ms
	unsigned int pulseprescale = busclk * 0.00005;		// Set pulse prescale 50us
	unsigned int uartdiv = busclk / WIEGAND_UART_BAUD;
	unsigned int interval = busclk * 0.0001;			// Set interval 100us

	_wiegand_out_pulse(id, pulse);						// 0x67e8 = 26600 = pluse*80M => pluse = 26600 / 80*10^6 = 3.325*10^-4 = 0.3325ms
	_wiegand_out_set_pulseprescale(id, pulseprescale);	// 0xfao = 4000 = prescale*80M => prescale = 4000 / 80*10^6 = 5*10^-5 = 50us
	_wiegand_out_set_uartdiv(id, uartdiv);				// 0x208d = 8333 = 80M / 9600(baudrate)
	_wiegand_out_set_fifo_state(id, 0x1e0);				// 0x1e0 = 480?
	_wiegand_out_set_interval(id, interval);			// 0x1f40 = 8000 = interval*80M => interval = 8000 / 80*10^6 = 10*10^-5 = 100us

	_wiegand_out_set_bit_count(id, bitcnt);
	_wiegand_out_loopback(id, dir);
	_wiegand_out_set_bit_order(id, order);
}

void wiegandout_set_bitcnt(WIEGAND_OUT_ID id, int bitcnt)
{
	_wiegand_out_set_bit_count(id, bitcnt);
}

void wiegandout_set_loopback(WIEGAND_OUT_ID id, WIEGAND_LOOPBACK_DIRECTION dir)
{
	_wiegand_out_loopback(id, dir);
}

void wiegandout_set_bit_order(WIEGAND_OUT_ID id, WIEGAND_OUT_BIT_ORDER order)
{
	_wiegand_out_set_bit_order(id, order);
}

void wiegandout_controller_enable(WIEGAND_OUT_ID id)
{
	_wiegand_out_controller_enable(id, 1);

	ithGpioSetMode(77, ITH_GPIO_MODE2);
	ithGpioSetMode(78, ITH_GPIO_MODE2);
}

// TODO: Check wiegand out probe

// TODO: internal bus clock: wclk & pclk 不同?
// TODO: 參數計算正確?
// TODO: FIFO_STATE設定原因?
// TODO: 參數基本上不需調整?
// TODO: Output gpio可選擇? (script似乎是設成mode2?)

// TODO: LSB SELECTION?