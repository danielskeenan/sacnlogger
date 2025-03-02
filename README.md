# sACN Logger

This program logs changes to one or more sACN universes to a file.

## Features

- Configurable sACN universes.
- Per-Address-Priority (0xDD) support.
- Logs winning levels, priority, and owner per-address.

## Usage
`sacnlogger <path to config file>` will begin logging to files in the current working directory.

## Config File

The config file is in YAML format:

```yaml
# (Required) List of universes to monitor.
universes:
  - 1
  - 2

# (Optional) Respect per-address-priority packets. Defaults to false.
usePap: true
```
