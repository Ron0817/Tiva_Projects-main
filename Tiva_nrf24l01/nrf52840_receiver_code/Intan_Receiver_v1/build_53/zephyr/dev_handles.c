#include <device.h>
#include <toolchain.h>

/* 1 : /soc/clock@41005000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/soc/interrupt-controller@e000e100)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_clock_41005000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 2 : /soc/gpio@418c0500:
 * Direct Dependencies:
 *   - (/soc)
 * Supported:
 *   - (/buttons/button_0)
 *   - (/buttons/button_1)
 *   - (/buttons/button_2)
 *   - (/buttons/button_3)
 *   - (/leds/led_0)
 *   - (/leds/led_1)
 *   - (/leds/led_2)
 *   - (/leds/led_3)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_gpio_418c0500[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 3 : /soc/gpio@418c0800:
 * Direct Dependencies:
 *   - (/soc)
 * Supported:
 *   - (/soc/spi@41013000)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_gpio_418c0800[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };

/* 4 : /soc/uart@41013000:
 * Direct Dependencies:
 *   - (/soc)
 *   - (/pin-controller/uart0_default)
 *   - (/pin-controller/uart0_sleep)
 *   - (/soc/interrupt-controller@e000e100)
 */
const device_handle_t __aligned(2) __attribute__((__section__(".__device_handles_pass2")))
__devicehdl_DT_N_S_soc_S_uart_41013000[] = { DEVICE_HANDLE_SEP, DEVICE_HANDLE_SEP, DEVICE_HANDLE_ENDS };
