# sACN Logger

This program logs changes to one or more sACN universes to a file.

## Features

- Configurable sACN universes.
- Per-Address-Priority (0xDD) support.
- Logs winning levels, priority, and owner per-address.

## Usage

`sacnlogger <path to config file>` will begin logging to files in the current working directory.

## Raspberry Pi

This program can be used on a [Raspberry Pi](https://www.raspberrypi.com/products/raspberry-pi-5/)

### Requirements

- Raspberry Pi 5 8GB or more RAM.
- [RTC Battery](https://www.raspberrypi.com/products/rtc-battery/). The battery is required for accurate log times.
- 1GB or larger microSD Card
- 16GB or larger USB Drive. The actual size required depends on how much data you are monitoring and the length of time
  the monitor will run. One change in sACN data will result in a little over 6kB of data.

### Setup

1. Download the [Raspberry Pi Imager](https://www.raspberrypi.com/software/) program and the compressed disk image with
   this program.
2. Launch imager and click Choose OS > Use Custom. Select the downloaded disk image.
3. Click Choose Storage and select your microSD card.
4. Click Next. *Do not apply OS customization settings*.
5. Create the config file named `sacnlogger.json` and place it in the root of a FAT32-formatted USB drive. Plug this USB
   drive into your Raspberry Pi.
6. Power up your Raspberry Pi. After about 30 seconds, the USB drive should show activity.

## Config File

The config file is in json format:

```json
{
  "universes": [
    1,
    2
  ],
  "usePap": true
}
```
