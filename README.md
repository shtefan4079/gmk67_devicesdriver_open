# GMK67 Ultimate Control

A native Linux application for managing the **Zouya GMK67** mechanical keyboard. 
Built with **Qt6** and **C++**


## Features


- **🎨 Lighting Control**: 
  - Per-key RGB customization.
  - Full support for hardware effects (Breathing, Wave, Ripple, etc.).
  - Real-time visualizer.
- **⌨️ Key Mapping**: 
  - Graphical key remapping interface.
  - Support for standard keys and standard media functions

## Prerequisites

To build the project, you need a C++17 compiler, CMake, Qt6, and HIDAPI.

### Debian / Ubuntu / Linux Mint
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev qt6-quick-controls2-dev libhidapi-dev
```

### Arch Linux / Manjaro
```bash
sudo pacman -S base-devel cmake qt6-base qt6-declarative hidapi
```

### Fedora
```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel qt6-qtdeclarative-devel hidapi-devel
```

## Build Instructions

1. **Create a build directory:**
   ```bash
   mkdir build
   cd build
   ```

2. **Compile:**
   ```bash
   cmake ..
   make -j$(nproc)
   ```

3. **Run:**
   ```bash
    cd build
   ./gmk67_control
   ```

## Post-Installation (Udev Rules)

To access the keyboard device without `sudo`, you need to add a udev rule.

1. Create a file at `/etc/udev/rules.d/99-gmk67.rules`:
   ```bash
   sudo nano /etc/udev/rules.d/99-gmk67.rules
   ```

2. Add the following line (replace `VID` and `PID` if yours differs, generally `05ac:024f` or similar device-specific IDs for the GMK67 interface):
   ```
   # GMK67 Keyboard
   SUBSYSTEM=="hidraw", ATTRS{idVendor}=="05ac", ATTRS{idProduct}=="024f", MODE="0666"
   # Alternative (Generic HID)
   KERNEL=="hidraw*", MODE="0666"
   ```
   *(Note: It is safer to use specific Vendor/Product IDs. Use `lsusb` to check your keyboard's identifiers.)*

3. Reload rules:
   ```bash
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```

