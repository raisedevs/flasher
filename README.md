# Raise.dev Firmware Flasher

Raise's ESP32 and ESP8266 firmware flasher cross-platform Qt desktop application for flashing ESP8266s and ESP32s from the Raise.dev Console.

**Note: unless a Raise.dev cofounder has invited you to join the Raise.dev Console, you can only use this for serial port monitoring (for now).**

## Building and Developing (on macOS)

We recommend building this by using QtCreator:

```console
# Install QtCreator from Homebrew Cask
$ brew install --cask qt-creator

# Install the Homebrew dependencies
$ brew bundle

# Output the Caveats to setup Homebrew's Qt in QtCreator
$ brew info --formula qt
```

Alternatively, you can build from the Terminal with:

```console
script/bootstrap
```

## Building (on Linux or Windows)

See the [GitHub Actions `build.yml` workflow using Ubuntu or Windows](https://github.com/raisedevs/flasher/blob/main/.github/workflows/build.yml).

Note that you'll need to manually install Qt and CMake.

## Status

Alpha and in active development.

## Contact

[Mike McQuaid](mailto:mike@raise.dev)

## License

Licensed under the [MIT License](https://en.wikipedia.org/wiki/MIT_License).
The full license text is available in [LICENSE.txt](https://github.com/raisedevs/raise-dev-library/blob/master/LICENSE.txt).
