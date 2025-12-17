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
PWM_ERROR_ARG: int
PWM_ERROR_OPEN: int
PWM_ERROR_QUERY: int
PWM_ERROR_CONFIGURE: int
PWM_ERROR_CLOSE: int
PWM_POLARITY_NORMAL: int
PWM_POLARITY_INVERSED: int