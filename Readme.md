# HerdMind Simulator - Development Environment Setup

This document describes how to set up a development environment for **HerdMind Simulator** on **Ubuntu 22.04 LTS** using **Qt Creator**, **Qt 6.11**, and **Qt MQTT**.

---

# 1. Clone the Repository

Clone the project from GitHub:

```bash
git clone git@github.com:IvanTsantsarov/herdmind-simulator.git
cd herdmind-simulator
```

If you do not have SSH access configured for GitHub, use HTTPS:

```bash
git clone https://github.com/IvanTsantsarov/herdmind-simulator.git
cd herdmind-simulator
```

---

# 2. Install Required Development Tools

Update package lists:

```bash
sudo apt update
```

Install essential build tools:

```bash
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    gdb \
    clang \
    pkg-config \
    libgl1-mesa-dev \
    libxkbcommon-dev \
    libssl-dev
```

Verify installation:

```bash
gcc --version
cmake --version
git --version
```

---

# 3. Install Qt Creator and Qt 6

## Option A (Recommended): Qt Online Installer

Download the Qt Online Installer from:

https://www.qt.io/download

Make the installer executable:

```bash
chmod +x qt-online-installer-linux-*.run
```

Run the installer:

```bash
./qt-online-installer-linux-*.run
```

Install:

* Qt Creator
* Qt 6.11.x
* Desktop gcc_64 kit

Recommended installation path:

```text
~/Qt/6.11.0/gcc_64
```

---

## Verify Qt Installation

Check that qmake is available:

```bash
~/Qt/6.11.0/gcc_64/bin/qmake -query
```

Expected output should show the Qt installation paths.

---

# 4. Install Qt MQTT Module

The HerdMind Simulator project requires the **Qt MQTT** module.

## Important

There are two different MQTT libraries commonly confused:

### Qt MQTT (Official)

Official Qt module.

Used in `.pro` files as:

```pro
QT += mqtt
```

### QMQTT (Third Party)

External library that is **not** an official Qt module.

This library cannot be used directly through:

```pro
QT += mqtt
```

and should not be used for this project.

---

## Build and Install Qt MQTT

Clone the official Qt MQTT module matching the installed Qt version:

```bash
cd ~/Projects

git clone --branch 6.11.0 https://github.com/qt/qtmqtt.git

cd qtmqtt
```

Configure:

```bash
cmake -S . -B build \
    -DCMAKE_PREFIX_PATH=$HOME/Qt/6.11.0/gcc_64 \
    -DCMAKE_INSTALL_PREFIX=$HOME/Qt/6.11.0/gcc_64
```

Build:

```bash
cmake --build build -j$(nproc)
```

Install:

```bash
cmake --install build
```

---

## Verify MQTT Installation

Check that MQTT files are installed inside Qt:

```bash
find $HOME/Qt/6.11.0/gcc_64 -iname "*mqtt*"
```

The module should be discoverable by qmake.

---

## Using MQTT in the Project

In a `.pro` file:

```pro
QT += core gui widgets positioning network mqtt
```

Example include:

```cpp
#include <QMqttClient>
```

---

# 5. Open Project in Qt Creator

Launch Qt Creator:

```bash
qtcreator
```

or

```bash
$HOME/Qt/Tools/QtCreator/bin/qtcreator
```

Open:

```text
herdmind-simulator.pro
```

Select:

* Desktop Qt 6.11.0 gcc_64 Kit

Configure the project and build.

---

# 6. Build from Command Line

Generate Makefiles:

```bash
$HOME/Qt/6.11.0/gcc_64/bin/qmake
```

Build:

```bash
make -j$(nproc)
```

Run:

```bash
./herdmind-simulator
```

---

# Troubleshooting

## MQTT Module Not Found

Error:

```text
Unknown module(s) in QT: mqtt
```

Verify that:

```bash
find $HOME/Qt/6.11.0/gcc_64 -iname "*mqtt*"
```

returns installed MQTT files.

If not, repeat the Qt MQTT installation steps.

---

## Qt Version Mismatch

Error:

```text
Could not find a configuration file for package "Qt6"
```

Ensure that:

```bash
-DCMAKE_PREFIX_PATH=$HOME/Qt/6.11.0/gcc_64
```

matches the installed Qt version exactly.

Qt module source version and installed Qt version should match (e.g. 6.11.0 ↔ 6.11.0).

---

## Clean Build

Remove build artifacts:

```bash
rm -rf build
```

Reconfigure and rebuild the project.
