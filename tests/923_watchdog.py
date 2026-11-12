import pkpy

if pkpy.configmacros['PK_ENABLE_WATCHDOG']:
    import sys

    if pkpy.configmacros['PK_ENABLE_OS'] and sys.platform in ('darwin', 'linux', 'win32'):
        import os

        if sys.platform == 'win32':
            command = 'powershell -NoProfile -Command "Start-Sleep -Milliseconds 400"'
        else:
            command = 'sleep 0.4'
        pkpy.watchdog_begin(100)
        status = os.system(command)
        pkpy.watchdog_end()
        assert status == 0

    timed_out = False
    pkpy.watchdog_begin(20)
    try:
        for _ in range(10000000):
            pass
    except TimeoutError as error:
        assert str(error) == 'watchdog timeout'
        timed_out = True
    pkpy.watchdog_end()
    assert timed_out
