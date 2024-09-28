# LBE-142x GPS Locked Clock Source Configuration Tool

This project provides a cross-platform configuration tool for Leo Bodnar LBE-1420 (single output) and LBE-1421 (dual output) GPS locked clock source devices.

It allows users to configure device settings, set frequencies, and monitor device status on both Windows and GNU/Linux systems (tested on Ubuntu24.04LTS x64).

## Features

- Support for both LBE-1420 (single output) and LBE-1421 (dual output) models
- Cross-platform compatibility (Windows and GNU/Linux)
- Set output frequencies
- Enable/disable outputs
- Blink device LEDs
- Retrieve and display device status

## Prerequisites

### Windows
- Microsoft Visual Studio 2022 / MINGW64
- CMake (version 3.10 or higher)
- libusb (1.0 or higher) see https://github.com/libusb/libusb/releases

### GNU/Linux
- GCC or Clang
- CMake (version 3.10 or higher)
- libudev-dev

## Building the Project

1. Clone the repository:
   ```
   git clone https://github.com/bvernoux/lbe-142x.git
   cd lbe-142x
   ```

2. Download and extract the LibUSB library for Visual Studio 2022 and MinGW64:
   - Go to the [LibUSB releases page](https://github.com/libusb/libusb/releases)
   - Download the latest release (e.g., `libusb-1.0.27.7z`)
   - Extract the contents to a `libusb` directory in the root of the project
     (The directory structure should look like: `lbe-142x/libusb/include`, `lbe-142x/libusb/MinGW64`, etc.)

3. Create a build directory and run CMake:

   For Windows (Visual Studio 2022):
      ```
      rm -rf build_VS2022
      mkdir build_VS2022 && cd build_VS2022
      cmake -G "Visual Studio 17 2022" -A x64 ..
      ```
   For Windows MinGW64:
      ```
      rm -rf build_MinGW64
      mkdir build_MinGW64 && cd build_MinGW64
      cmake -G "MinGW Makefiles" ..
      ```
   For GNU/Linux:
      ```
      rm -rf build
      mkdir build && cd build
      cmake ..
   ```

4. Build the project:

   For Windows (Visual Studio 2022):
   Open the sln
   
   For MinGW64 and GNU/Linux:
      ```
      cmake --build .
      ```
   
      For a specific build type (e.g., Debug as it is Release by default):
      ```
      cmake --build . --config Debug
      ```

## Usage

After building the project, you can run the `lbe-142x` executable with various command-line options:

```
Usage: lbe-142x [OPTIONS]
Options:
  --f1 <freq>       Set frequency for output 1 (1-1400000000 Hz) and save to flash
  --f2 <freq>       Set frequency for output 2 (1-1400000000 Hz) and save to flash (LBE-1421 only)
  --out <0|1>       Enable or disable outputs
  --blink           Blink output LED(s) for 3 seconds
  --status          Display current device status
```

Examples:

1. Set frequency for output 1 to 10 MHz:
   ```
   ./lbe-142x --f1 10000000
   ```

2. Enable outputs:
   ```
   ./lbe-142x --out 1
   ```

3. Display device status:
   ```
   ./lbe-142x --status
   ```

## Troubleshooting

### GNU/Linux
If you encounter permission issues when accessing the device, you may need to add a udev rule.

Create a file named `/etc/udev/rules.d/99-lbe.rules` with the following content:

```
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0660", GROUP="plugdev"
```
Then add current user to group plugdev
```
sudo usermod -aG plugdev $(whoami)
```

Then, reload the udev rules:

```
sudo udevadm control --reload-rules && sudo udevadm trigger
```

## Contributing

Contributions to this project are welcome. Please fork the repository and submit a pull request with your changes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Leo Bodnar Electronics for the LBE-142x devices and https://github.com/simontheu/lbe-1420
