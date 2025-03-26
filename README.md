# 6502
An NMOS 6502 emulator written in C99.

## Features
- Customizable memory map (for use in various systems)
- CPU and memory debugging
- S to step clock
- D to execute full instructon
- Q/E to scroll through memory viewer
- Supports [Single Step Tests](https://github.com/SingleStepTests/65x02/tree/main/6502) logging

## Single Step Tests
You must provide a folder named `SingleStepTests` in the root project directory with all of the single stepped tests. They are not included in this repo because they are ~1.8 GB in total.

`logs/log.txt` will be updated with all appropriate information.

## To-Do
- Add decimal mode support to ADC/SBC
- Increase cycles when a page boundary is crossed
- ~~Add proper debugging logs for nestest and SingleStepTests~~
