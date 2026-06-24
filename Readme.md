# HerdMind Simulator - Development Environment Setup

This document describes how to set up a development environment for **HerdMind Simulator** on **Ubuntu 22.04 LTS** using **Qt Creator**, **Qt 6.11**, **Qt MQTT**, Docker, and ChirpStack.

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
sudo apt install -y build-essential cmake ninja-build git gdb clang pkg-config libgl1-mesa-dev libxkbcommon-dev libssl-dev
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
* Desktop gcc_64 Kit

Recommended installation path:

```text
~/Qt/6.11.0/gcc_64
```

Verify installation:

```bash
~/Qt/6.11.0/gcc_64/bin/qmake -query
```

---

# 4. Install Qt MQTT Module

The HerdMind Simulator project requires the official Qt MQTT module.

## Important

There are two MQTT libraries commonly confused:

### Qt MQTT (Official)

Official Qt module.

Used in `.pro` files as:

```pro
QT += mqtt
```

### QMQTT (Third Party)

External library that is **not** an official Qt module.

Do not use QMQTT for this project.

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
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/Qt/6.11.0/gcc_64 -DCMAKE_INSTALL_PREFIX=$HOME/Qt/6.11.0/gcc_64
```

Build:

```bash
cmake --build build -j$(nproc)
```

Install:

```bash
cmake --install build
```

Verify installation:

```bash
find $HOME/Qt/6.11.0/gcc_64 -iname "*mqtt*"
```

Example usage in the project:

```pro
QT += core gui widgets positioning network mqtt
```

Example include:

```cpp
#include <QMqttClient>
```

---

# 5. Install Docker

The project uses Docker containers for local services and development tooling.

## Install Docker Engine

Remove old Docker packages if present:

```bash
sudo apt remove docker docker-engine docker.io containerd runc
```

Install prerequisites:

```bash
sudo apt update

sudo apt install -y ca-certificates curl gnupg lsb-release
```

Add Docker's official GPG key:

```bash
sudo mkdir -p /etc/apt/keyrings

curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
```

Add Docker repository:

```bash
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
```

Install Docker:

```bash
sudo apt update

sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

Verify installation:

```bash
docker --version
docker compose version
```

Allow running Docker without sudo:

```bash
sudo usermod -aG docker $USER
```

Log out and log back in for the group membership change to take effect.

Verify:

```bash
docker run hello-world
```

---

# 6. Install ChirpStack Docker Images

Pull the required ChirpStack images:

```bash
git clone https://github.com/chirpstack/chirpstack-docker.git
cd chirpstack-docker
```

Expected images:

```text
chirpstack/chirpstack
chirpstack/chirpstack-gateway-bridge
postgres
redis
eclipse-mosquitto
```

---

# 7. Verify Development Environment

Verify the following components are available:

```bash
git --version
cmake --version
docker --version
docker compose version
```

Verify Qt:

```bash
$HOME/Qt/6.11.0/gcc_64/bin/qmake -query
```

Verify Qt MQTT:

```bash
find $HOME/Qt/6.11.0/gcc_64 -iname "*mqtt*"
```

At this point the development environment should be ready for building and running HerdMind Simulator.
