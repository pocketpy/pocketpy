---
icon: dot
title: Profiling
order: 79
---

To profile your pocketpy scripts, you can run `main.exe` with `--profile` flag.

For example, to profile `test/test_math.py`, run

```
main.exe --profile test/test_math.py
```

This will output a JSON report file named `profile_report.json` in the current directory,
which records the time spent for each line. To visualize the report, please install our VSCode extension.

https://marketplace.visualstudio.com/items?itemName=pocketpy.pocketpy

With pocketpy VSCode extension, press `F1` and type `pocketpy: Load Line Profiler Report`,
select `profile_report.json` and you will see a nice visualization of the profiling result.

![lp](../static/profiler_demo.png)
