from typing import overload
from stdc import intptr

class gpio_config:
    direction: int #  (gpio_direction_t)
    edge: int #  (gpio_edge_t)
    event_clock: int #  (gpio_event_clock_t)
    debounce_us: int #  (uint32_t)
    bias: int #  (gpio_bias_t)
    drive: int #  (gpio_drive_t)
    inverted: bool #  (bool)
    label: str #  (const char*)
    
    @overload
    def __init__(self): ...
    @overload
    def __init__(self, direction: int, edge: int, event_clock: int, debounce_us: int, bias: int, drive: int, inverted: bool, label: str): ...
    
class spi_msg:
    txbuf: intptr #  (const uint8_t*)
    rxbuf: intptr #  (uint8_t*)
    len: int #  (size_t)
    deselect: bool #  (bool)
    deselect_delay_us: int #  (uint16_t)
    word_delay_us: int #  (uint8_t)
    
    @overload
    def __init__(self): ...
    @overload
    def __init__(self, txbuf: intptr, rxbuf: intptr, len: int, deselect: bool, deselect_delay_us: int, word_delay_us: int): ...
    
class periphery_version:
    major: int #  (unsigned)
    minor: int #  (unsigned)
    patch: int #  (unsigned)
    commit_id: str #  (const char*)
    
    @overload
    def __init__(self): ...
    @overload
    def __init__(self, major: int, minor: int, patch: int, commit_id: str): ...
    
def gpio_new() -> intptr:
    """Wraps `gpio_t* gpio_new()`"""

def gpio_open(gpio: intptr, path: str, line: int, direction: int, /) -> int:
    """Wraps `int gpio_open(gpio_t* gpio, const char* path, unsigned line, gpio_direction_t direction)`"""

def gpio_open_name(gpio: intptr, path: str, name: str, direction: int, /) -> int:
    """Wraps `int gpio_open_name(gpio_t* gpio, const char* path, const char* name, gpio_direction_t direction)`"""

def gpio_open_advanced(gpio: intptr, path: str, line: int, config: intptr, /) -> int:
    """Wraps `int gpio_open_advanced(gpio_t* gpio, const char* path, unsigned line, const gpio_config_t* config)`"""

def gpio_open_name_advanced(gpio: intptr, path: str, name: str, config: intptr, /) -> int:
    """Wraps `int gpio_open_name_advanced(gpio_t* gpio, const char* path, const char* name, const gpio_config_t* config)`"""

def gpio_open_sysfs(gpio: intptr, line: int, direction: int, /) -> int:
    """Wraps `int gpio_open_sysfs(gpio_t* gpio, unsigned line, gpio_direction_t direction)`"""

def gpio_read(gpio: intptr, value: intptr, /) -> int:
    """Wraps `int gpio_read(gpio_t* gpio, bool* value)`"""

def gpio_write(gpio: intptr, value: bool, /) -> int:
    """Wraps `int gpio_write(gpio_t* gpio, bool value)`"""

def gpio_poll(gpio: intptr, timeout_ms: int, /) -> int:
    """Wraps `int gpio_poll(gpio_t* gpio, int timeout_ms)`"""

def gpio_close(gpio: intptr, /) -> int:
    """Wraps `int gpio_close(gpio_t* gpio)`"""

def gpio_free(gpio: intptr, /) -> None:
    """Wraps `void gpio_free(gpio_t* gpio)`"""

def gpio_read_event(gpio: intptr, edge: intptr, timestamp: intptr, /) -> int:
    """Wraps `int gpio_read_event(gpio_t* gpio, gpio_edge_t* edge, uint64_t* timestamp)`"""

def gpio_poll_multiple(gpios: intptr, count: int, timeout_ms: int, gpios_ready: intptr, /) -> int:
    """Wraps `int gpio_poll_multiple(gpio_t** gpios, size_t count, int timeout_ms, bool* gpios_ready)`"""

def gpio_get_direction(gpio: intptr, direction: intptr, /) -> int:
    """Wraps `int gpio_get_direction(gpio_t* gpio, gpio_direction_t* direction)`"""

def gpio_get_edge(gpio: intptr, edge: intptr, /) -> int:
    """Wraps `int gpio_get_edge(gpio_t* gpio, gpio_edge_t* edge)`"""

def gpio_get_event_clock(gpio: intptr, event_clock: intptr, /) -> int:
    """Wraps `int gpio_get_event_clock(gpio_t* gpio, gpio_event_clock_t* event_clock)`"""

def gpio_get_debounce_us(gpio: intptr, debounce_us: intptr, /) -> int:
    """Wraps `int gpio_get_debounce_us(gpio_t* gpio, uint32_t* debounce_us)`"""

def gpio_get_bias(gpio: intptr, bias: intptr, /) -> int:
    """Wraps `int gpio_get_bias(gpio_t* gpio, gpio_bias_t* bias)`"""

def gpio_get_drive(gpio: intptr, drive: intptr, /) -> int:
    """Wraps `int gpio_get_drive(gpio_t* gpio, gpio_drive_t* drive)`"""

def gpio_get_inverted(gpio: intptr, inverted: intptr, /) -> int:
    """Wraps `int gpio_get_inverted(gpio_t* gpio, bool* inverted)`"""

def gpio_set_direction(gpio: intptr, direction: int, /) -> int:
    """Wraps `int gpio_set_direction(gpio_t* gpio, gpio_direction_t direction)`"""

def gpio_set_edge(gpio: intptr, edge: int, /) -> int:
    """Wraps `int gpio_set_edge(gpio_t* gpio, gpio_edge_t edge)`"""

def gpio_set_event_clock(gpio: intptr, event_clock: int, /) -> int:
    """Wraps `int gpio_set_event_clock(gpio_t* gpio, gpio_event_clock_t event_clock)`"""

def gpio_set_debounce_us(gpio: intptr, debounce_us: int, /) -> int:
    """Wraps `int gpio_set_debounce_us(gpio_t* gpio, uint32_t debounce_us)`"""

def gpio_set_bias(gpio: intptr, bias: int, /) -> int:
    """Wraps `int gpio_set_bias(gpio_t* gpio, gpio_bias_t bias)`"""

def gpio_set_drive(gpio: intptr, drive: int, /) -> int:
    """Wraps `int gpio_set_drive(gpio_t* gpio, gpio_drive_t drive)`"""

def gpio_set_inverted(gpio: intptr, inverted: bool, /) -> int:
    """Wraps `int gpio_set_inverted(gpio_t* gpio, bool inverted)`"""

def gpio_line(gpio: intptr, /) -> int:
    """Wraps `unsigned gpio_line(gpio_t* gpio)`"""

def gpio_fd(gpio: intptr, /) -> int:
    """Wraps `int gpio_fd(gpio_t* gpio)`"""

def gpio_name(gpio: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int gpio_name(gpio_t* gpio, char* str, size_t len)`"""

def gpio_label(gpio: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int gpio_label(gpio_t* gpio, char* str, size_t len)`"""

def gpio_chip_fd(gpio: intptr, /) -> int:
    """Wraps `int gpio_chip_fd(gpio_t* gpio)`"""

def gpio_chip_name(gpio: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int gpio_chip_name(gpio_t* gpio, char* str, size_t len)`"""

def gpio_chip_label(gpio: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int gpio_chip_label(gpio_t* gpio, char* str, size_t len)`"""

def gpio_tostring(gpio: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int gpio_tostring(gpio_t* gpio, char* str, size_t len)`"""

def gpio_errno(gpio: intptr, /) -> int:
    """Wraps `int gpio_errno(gpio_t* gpio)`"""

def gpio_errmsg(gpio: intptr, /) -> str:
    """Wraps `const char* gpio_errmsg(gpio_t* gpio)`"""

def mmio_new() -> intptr:
    """Wraps `mmio_t* mmio_new()`"""

def mmio_open(mmio: intptr, base: int, size: int, /) -> int:
    """Wraps `int mmio_open(mmio_t* mmio, uintptr_t base, size_t size)`"""

def mmio_open_advanced(mmio: intptr, base: int, size: int, path: str, /) -> int:
    """Wraps `int mmio_open_advanced(mmio_t* mmio, uintptr_t base, size_t size, const char* path)`"""

def mmio_ptr(mmio: intptr, /) -> intptr:
    """Wraps `void* mmio_ptr(mmio_t* mmio)`"""

def mmio_read64(mmio: intptr, offset: int, value: intptr, /) -> int:
    """Wraps `int mmio_read64(mmio_t* mmio, uintptr_t offset, uint64_t* value)`"""

def mmio_read32(mmio: intptr, offset: int, value: intptr, /) -> int:
    """Wraps `int mmio_read32(mmio_t* mmio, uintptr_t offset, uint32_t* value)`"""

def mmio_read16(mmio: intptr, offset: int, value: intptr, /) -> int:
    """Wraps `int mmio_read16(mmio_t* mmio, uintptr_t offset, uint16_t* value)`"""

def mmio_read8(mmio: intptr, offset: int, value: intptr, /) -> int:
    """Wraps `int mmio_read8(mmio_t* mmio, uintptr_t offset, uint8_t* value)`"""

def mmio_read(mmio: intptr, offset: int, buf: intptr, len: int, /) -> int:
    """Wraps `int mmio_read(mmio_t* mmio, uintptr_t offset, uint8_t* buf, size_t len)`"""

def mmio_write64(mmio: intptr, offset: int, value: int, /) -> int:
    """Wraps `int mmio_write64(mmio_t* mmio, uintptr_t offset, uint64_t value)`"""

def mmio_write32(mmio: intptr, offset: int, value: int, /) -> int:
    """Wraps `int mmio_write32(mmio_t* mmio, uintptr_t offset, uint32_t value)`"""

def mmio_write16(mmio: intptr, offset: int, value: int, /) -> int:
    """Wraps `int mmio_write16(mmio_t* mmio, uintptr_t offset, uint16_t value)`"""

def mmio_write8(mmio: intptr, offset: int, value: int, /) -> int:
    """Wraps `int mmio_write8(mmio_t* mmio, uintptr_t offset, uint8_t value)`"""

def mmio_write(mmio: intptr, offset: int, buf: intptr, len: int, /) -> int:
    """Wraps `int mmio_write(mmio_t* mmio, uintptr_t offset, const uint8_t* buf, size_t len)`"""

def mmio_close(mmio: intptr, /) -> int:
    """Wraps `int mmio_close(mmio_t* mmio)`"""

def mmio_free(mmio: intptr, /) -> None:
    """Wraps `void mmio_free(mmio_t* mmio)`"""

def mmio_base(mmio: intptr, /) -> int:
    """Wraps `uintptr_t mmio_base(mmio_t* mmio)`"""

def mmio_size(mmio: intptr, /) -> int:
    """Wraps `size_t mmio_size(mmio_t* mmio)`"""

def mmio_tostring(mmio: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int mmio_tostring(mmio_t* mmio, char* str, size_t len)`"""

def mmio_errno(mmio: intptr, /) -> int:
    """Wraps `int mmio_errno(mmio_t* mmio)`"""

def mmio_errmsg(mmio: intptr, /) -> str:
    """Wraps `const char* mmio_errmsg(mmio_t* mmio)`"""

def pwm_new() -> intptr:
    """Wraps `pwm_t* pwm_new()`"""

def pwm_open(pwm: intptr, chip: int, channel: int, /) -> int:
    """Wraps `int pwm_open(pwm_t* pwm, unsigned chip, unsigned channel)`"""

def pwm_enable(pwm: intptr, /) -> int:
    """Wraps `int pwm_enable(pwm_t* pwm)`"""

def pwm_disable(pwm: intptr, /) -> int:
    """Wraps `int pwm_disable(pwm_t* pwm)`"""

def pwm_close(pwm: intptr, /) -> int:
    """Wraps `int pwm_close(pwm_t* pwm)`"""

def pwm_free(pwm: intptr, /) -> None:
    """Wraps `void pwm_free(pwm_t* pwm)`"""

def pwm_get_enabled(pwm: intptr, enabled: intptr, /) -> int:
    """Wraps `int pwm_get_enabled(pwm_t* pwm, bool* enabled)`"""

def pwm_get_period_ns(pwm: intptr, period_ns: intptr, /) -> int:
    """Wraps `int pwm_get_period_ns(pwm_t* pwm, uint64_t* period_ns)`"""

def pwm_get_duty_cycle_ns(pwm: intptr, duty_cycle_ns: intptr, /) -> int:
    """Wraps `int pwm_get_duty_cycle_ns(pwm_t* pwm, uint64_t* duty_cycle_ns)`"""

def pwm_get_period(pwm: intptr, period: intptr, /) -> int:
    """Wraps `int pwm_get_period(pwm_t* pwm, double* period)`"""

def pwm_get_duty_cycle(pwm: intptr, duty_cycle: intptr, /) -> int:
    """Wraps `int pwm_get_duty_cycle(pwm_t* pwm, double* duty_cycle)`"""

def pwm_get_frequency(pwm: intptr, frequency: intptr, /) -> int:
    """Wraps `int pwm_get_frequency(pwm_t* pwm, double* frequency)`"""

def pwm_get_polarity(pwm: intptr, polarity: intptr, /) -> int:
    """Wraps `int pwm_get_polarity(pwm_t* pwm, pwm_polarity_t* polarity)`"""

def pwm_set_enabled(pwm: intptr, enabled: bool, /) -> int:
    """Wraps `int pwm_set_enabled(pwm_t* pwm, bool enabled)`"""

def pwm_set_period_ns(pwm: intptr, period_ns: int, /) -> int:
    """Wraps `int pwm_set_period_ns(pwm_t* pwm, uint64_t period_ns)`"""

def pwm_set_duty_cycle_ns(pwm: intptr, duty_cycle_ns: int, /) -> int:
    """Wraps `int pwm_set_duty_cycle_ns(pwm_t* pwm, uint64_t duty_cycle_ns)`"""

def pwm_set_period(pwm: intptr, period: float, /) -> int:
    """Wraps `int pwm_set_period(pwm_t* pwm, double period)`"""

def pwm_set_duty_cycle(pwm: intptr, duty_cycle: float, /) -> int:
    """Wraps `int pwm_set_duty_cycle(pwm_t* pwm, double duty_cycle)`"""

def pwm_set_frequency(pwm: intptr, frequency: float, /) -> int:
    """Wraps `int pwm_set_frequency(pwm_t* pwm, double frequency)`"""

def pwm_set_polarity(pwm: intptr, polarity: int, /) -> int:
    """Wraps `int pwm_set_polarity(pwm_t* pwm, pwm_polarity_t polarity)`"""

def pwm_chip(pwm: intptr, /) -> int:
    """Wraps `unsigned pwm_chip(pwm_t* pwm)`"""

def pwm_channel(pwm: intptr, /) -> int:
    """Wraps `unsigned pwm_channel(pwm_t* pwm)`"""

def pwm_tostring(pwm: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int pwm_tostring(pwm_t* pwm, char* str, size_t len)`"""

def pwm_errno(pwm: intptr, /) -> int:
    """Wraps `int pwm_errno(pwm_t* pwm)`"""

def pwm_errmsg(pwm: intptr, /) -> str:
    """Wraps `const char* pwm_errmsg(pwm_t* pwm)`"""

def serial_new() -> intptr:
    """Wraps `serial_t* serial_new()`"""

def serial_open(serial: intptr, path: str, baudrate: int, /) -> int:
    """Wraps `int serial_open(serial_t* serial, const char* path, uint32_t baudrate)`"""

def serial_open_advanced(serial: intptr, path: str, baudrate: int, databits: int, parity: int, stopbits: int, xonxoff: bool, rtscts: bool, /) -> int:
    """Wraps `int serial_open_advanced(serial_t* serial, const char* path, uint32_t baudrate, unsigned databits, serial_parity_t parity, unsigned stopbits, bool xonxoff, bool rtscts)`"""

def serial_read(serial: intptr, buf: intptr, len: int, timeout_ms: int, /) -> int:
    """Wraps `int serial_read(serial_t* serial, uint8_t* buf, size_t len, int timeout_ms)`"""

def serial_write(serial: intptr, buf: intptr, len: int, /) -> int:
    """Wraps `int serial_write(serial_t* serial, const uint8_t* buf, size_t len)`"""

def serial_flush(serial: intptr, /) -> int:
    """Wraps `int serial_flush(serial_t* serial)`"""

def serial_input_waiting(serial: intptr, count: intptr, /) -> int:
    """Wraps `int serial_input_waiting(serial_t* serial, unsigned* count)`"""

def serial_output_waiting(serial: intptr, count: intptr, /) -> int:
    """Wraps `int serial_output_waiting(serial_t* serial, unsigned* count)`"""

def serial_poll(serial: intptr, timeout_ms: int, /) -> int:
    """Wraps `int serial_poll(serial_t* serial, int timeout_ms)`"""

def serial_close(serial: intptr, /) -> int:
    """Wraps `int serial_close(serial_t* serial)`"""

def serial_free(serial: intptr, /) -> None:
    """Wraps `void serial_free(serial_t* serial)`"""

def serial_get_baudrate(serial: intptr, baudrate: intptr, /) -> int:
    """Wraps `int serial_get_baudrate(serial_t* serial, uint32_t* baudrate)`"""

def serial_get_databits(serial: intptr, databits: intptr, /) -> int:
    """Wraps `int serial_get_databits(serial_t* serial, unsigned* databits)`"""

def serial_get_parity(serial: intptr, parity: intptr, /) -> int:
    """Wraps `int serial_get_parity(serial_t* serial, serial_parity_t* parity)`"""

def serial_get_stopbits(serial: intptr, stopbits: intptr, /) -> int:
    """Wraps `int serial_get_stopbits(serial_t* serial, unsigned* stopbits)`"""

def serial_get_xonxoff(serial: intptr, xonxoff: intptr, /) -> int:
    """Wraps `int serial_get_xonxoff(serial_t* serial, bool* xonxoff)`"""

def serial_get_rtscts(serial: intptr, rtscts: intptr, /) -> int:
    """Wraps `int serial_get_rtscts(serial_t* serial, bool* rtscts)`"""

def serial_get_vmin(serial: intptr, vmin: intptr, /) -> int:
    """Wraps `int serial_get_vmin(serial_t* serial, unsigned* vmin)`"""

def serial_get_vtime(serial: intptr, vtime: intptr, /) -> int:
    """Wraps `int serial_get_vtime(serial_t* serial, float* vtime)`"""

def serial_set_baudrate(serial: intptr, baudrate: int, /) -> int:
    """Wraps `int serial_set_baudrate(serial_t* serial, uint32_t baudrate)`"""

def serial_set_databits(serial: intptr, databits: int, /) -> int:
    """Wraps `int serial_set_databits(serial_t* serial, unsigned databits)`"""

def serial_set_parity(serial: intptr, parity: enum serial_parity, /) -> int:
    """Wraps `int serial_set_parity(serial_t* serial, enum serial_parity parity)`"""

def serial_set_stopbits(serial: intptr, stopbits: int, /) -> int:
    """Wraps `int serial_set_stopbits(serial_t* serial, unsigned stopbits)`"""

def serial_set_xonxoff(serial: intptr, enabled: bool, /) -> int:
    """Wraps `int serial_set_xonxoff(serial_t* serial, bool enabled)`"""

def serial_set_rtscts(serial: intptr, enabled: bool, /) -> int:
    """Wraps `int serial_set_rtscts(serial_t* serial, bool enabled)`"""

def serial_set_vmin(serial: intptr, vmin: int, /) -> int:
    """Wraps `int serial_set_vmin(serial_t* serial, unsigned vmin)`"""

def serial_set_vtime(serial: intptr, vtime: float, /) -> int:
    """Wraps `int serial_set_vtime(serial_t* serial, float vtime)`"""

def serial_fd(serial: intptr, /) -> int:
    """Wraps `int serial_fd(serial_t* serial)`"""

def serial_tostring(serial: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int serial_tostring(serial_t* serial, char* str, size_t len)`"""

def serial_errno(serial: intptr, /) -> int:
    """Wraps `int serial_errno(serial_t* serial)`"""

def serial_errmsg(serial: intptr, /) -> str:
    """Wraps `const char* serial_errmsg(serial_t* serial)`"""

def spi_new() -> intptr:
    """Wraps `spi_t* spi_new()`"""

def spi_open(spi: intptr, path: str, mode: int, max_speed: int, /) -> int:
    """Wraps `int spi_open(spi_t* spi, const char* path, unsigned mode, uint32_t max_speed)`"""

def spi_open_advanced(spi: intptr, path: str, mode: int, max_speed: int, bit_order: int, bits_per_word: int, extra_flags: int, /) -> int:
    """Wraps `int spi_open_advanced(spi_t* spi, const char* path, unsigned mode, uint32_t max_speed, spi_bit_order_t bit_order, uint8_t bits_per_word, uint8_t extra_flags)`"""

def spi_open_advanced2(spi: intptr, path: str, mode: int, max_speed: int, bit_order: int, bits_per_word: int, extra_flags: int, /) -> int:
    """Wraps `int spi_open_advanced2(spi_t* spi, const char* path, unsigned mode, uint32_t max_speed, spi_bit_order_t bit_order, uint8_t bits_per_word, uint32_t extra_flags)`"""

def spi_transfer(spi: intptr, txbuf: intptr, rxbuf: intptr, len: int, /) -> int:
    """Wraps `int spi_transfer(spi_t* spi, const uint8_t* txbuf, uint8_t* rxbuf, size_t len)`"""

def spi_transfer_advanced(spi: intptr, msgs: intptr, count: int, /) -> int:
    """Wraps `int spi_transfer_advanced(spi_t* spi, const spi_msg_t* msgs, size_t count)`"""

def spi_close(spi: intptr, /) -> int:
    """Wraps `int spi_close(spi_t* spi)`"""

def spi_free(spi: intptr, /) -> None:
    """Wraps `void spi_free(spi_t* spi)`"""

def spi_get_mode(spi: intptr, mode: intptr, /) -> int:
    """Wraps `int spi_get_mode(spi_t* spi, unsigned* mode)`"""

def spi_get_max_speed(spi: intptr, max_speed: intptr, /) -> int:
    """Wraps `int spi_get_max_speed(spi_t* spi, uint32_t* max_speed)`"""

def spi_get_bit_order(spi: intptr, bit_order: intptr, /) -> int:
    """Wraps `int spi_get_bit_order(spi_t* spi, spi_bit_order_t* bit_order)`"""

def spi_get_bits_per_word(spi: intptr, bits_per_word: intptr, /) -> int:
    """Wraps `int spi_get_bits_per_word(spi_t* spi, uint8_t* bits_per_word)`"""

def spi_get_extra_flags(spi: intptr, extra_flags: intptr, /) -> int:
    """Wraps `int spi_get_extra_flags(spi_t* spi, uint8_t* extra_flags)`"""

def spi_get_extra_flags32(spi: intptr, extra_flags: intptr, /) -> int:
    """Wraps `int spi_get_extra_flags32(spi_t* spi, uint32_t* extra_flags)`"""

def spi_set_mode(spi: intptr, mode: int, /) -> int:
    """Wraps `int spi_set_mode(spi_t* spi, unsigned mode)`"""

def spi_set_max_speed(spi: intptr, max_speed: int, /) -> int:
    """Wraps `int spi_set_max_speed(spi_t* spi, uint32_t max_speed)`"""

def spi_set_bit_order(spi: intptr, bit_order: int, /) -> int:
    """Wraps `int spi_set_bit_order(spi_t* spi, spi_bit_order_t bit_order)`"""

def spi_set_bits_per_word(spi: intptr, bits_per_word: int, /) -> int:
    """Wraps `int spi_set_bits_per_word(spi_t* spi, uint8_t bits_per_word)`"""

def spi_set_extra_flags(spi: intptr, extra_flags: int, /) -> int:
    """Wraps `int spi_set_extra_flags(spi_t* spi, uint8_t extra_flags)`"""

def spi_set_extra_flags32(spi: intptr, extra_flags: int, /) -> int:
    """Wraps `int spi_set_extra_flags32(spi_t* spi, uint32_t extra_flags)`"""

def spi_fd(spi: intptr, /) -> int:
    """Wraps `int spi_fd(spi_t* spi)`"""

def spi_tostring(spi: intptr, str: intptr, len: int, /) -> int:
    """Wraps `int spi_tostring(spi_t* spi, char* str, size_t len)`"""

def spi_errno(spi: intptr, /) -> int:
    """Wraps `int spi_errno(spi_t* spi)`"""

def spi_errmsg(spi: intptr, /) -> str:
    """Wraps `const char* spi_errmsg(spi_t* spi)`"""

def periphery_version() -> str:
    """Wraps `const char* periphery_version()`"""

def periphery_version_info() -> intptr:
    """Wraps `const periphery_version_t* periphery_version_info()`"""

# aliases
gpio_direction_t = int
gpio_edge_t = int
gpio_event_clock_t = int
gpio_bias_t = int
gpio_drive_t = int
pwm_polarity_t = int
serial_parity_t = int
spi_bit_order_t = int
# enums
GPIO_ERROR_ARG: int
GPIO_ERROR_OPEN: int
GPIO_ERROR_NOT_FOUND: int
GPIO_ERROR_QUERY: int
GPIO_ERROR_CONFIGURE: int
GPIO_ERROR_UNSUPPORTED: int
GPIO_ERROR_INVALID_OPERATION: int
GPIO_ERROR_IO: int
GPIO_ERROR_CLOSE: int
GPIO_DIR_IN: int
GPIO_DIR_OUT: int
GPIO_DIR_OUT_LOW: int
GPIO_DIR_OUT_HIGH: int
GPIO_EDGE_NONE: int
GPIO_EDGE_RISING: int
GPIO_EDGE_FALLING: int
GPIO_EDGE_BOTH: int
GPIO_EVENT_CLOCK_REALTIME: int
GPIO_EVENT_CLOCK_MONOTONIC: int
GPIO_EVENT_CLOCK_HTE: int
GPIO_BIAS_DEFAULT: int
GPIO_BIAS_PULL_UP: int
GPIO_BIAS_PULL_DOWN: int
GPIO_BIAS_DISABLE: int
GPIO_DRIVE_DEFAULT: int
GPIO_DRIVE_OPEN_DRAIN: int
GPIO_DRIVE_OPEN_SOURCE: int
MMIO_ERROR_ARG: int
MMIO_ERROR_OPEN: int
MMIO_ERROR_CLOSE: int
PWM_ERROR_ARG: int
PWM_ERROR_OPEN: int
PWM_ERROR_QUERY: int
PWM_ERROR_CONFIGURE: int
PWM_ERROR_CLOSE: int
PWM_POLARITY_NORMAL: int
PWM_POLARITY_INVERSED: int
SERIAL_ERROR_ARG: int
SERIAL_ERROR_OPEN: int
SERIAL_ERROR_QUERY: int
SERIAL_ERROR_CONFIGURE: int
SERIAL_ERROR_IO: int
SERIAL_ERROR_CLOSE: int
PARITY_NONE: int
PARITY_ODD: int
PARITY_EVEN: int
SPI_ERROR_ARG: int
SPI_ERROR_OPEN: int
SPI_ERROR_QUERY: int
SPI_ERROR_CONFIGURE: int
SPI_ERROR_TRANSFER: int
SPI_ERROR_CLOSE: int
SPI_ERROR_UNSUPPORTED: int
MSB_FIRST: int
LSB_FIRST: int