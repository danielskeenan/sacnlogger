# sACN Logger

This program logs changes to one or more sACN universes to a file.

## Features

- Configurable sACN universes.
- Per-Address-Priority (0xDD) support.
- Logs winning levels, priority, and owner per-address.

## Usage

`sacnlogger <path to config file>` will begin logging to files in the current working directory.

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
