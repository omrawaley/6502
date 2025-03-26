# 6502
An NMOS 6502 emulator written in C99.

## Features
- Customizable memory map (for use in various systems)
- CPU and memory debugging
- S to step clock
- D to execute full instructon
- Up/Down to scroll through memory viewer
- Supports [Single Step Tests](https://github.com/SingleStepTests/65x02/tree/main/6502) logging (see `logs/log.txt`)

## To-Do
- Add decimal mode support to ADC/SBC
- Increase cycles when a page boundary is crossed
- ~~Add proper debugging logs for nestest and SingleStepTests~~
