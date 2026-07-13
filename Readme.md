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

## Set up Docker's apt repository

```bash
# Add Docker's official GPG key:
sudo apt update
sudo apt install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
sudo tee /etc/apt/sources.list.d/docker.sources <<EOF
Types: deb
URIs: https://download.docker.com/linux/ubuntu
Suites: $(. /etc/os-release && echo "${UBUNTU_CODENAME:-$VERSION_CODENAME}")
Components: stable
Architectures: $(dpkg --print-architecture)
Signed-By: /etc/apt/keyrings/docker.asc
EOF
sudo apt update
```

## Install the Docker packages.

```bash
sudo apt install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

## Create docker group to prevent alway using sudo

```bash
sudo usermod -aG docker $USER
newgrp docker
```


# 6. Install ChirpStack Docker Images

```bash
cd chirpstack-docker
docker compose up -d
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

# 7. Verify Development Environment (if needed)

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

___
# 8. Configure chirpstack from Web UI

Login to localhost:8080 with name "admin" and password "admin".
There are two settings.ini and settings_external.ini (for external connection). 
All of next steps requres creating new intries into these settings accordingly
Web UI Chirpstack configuration.

## From Network side bar menu
### In API keys
1. Create new api key and before closing it key copy it
2. Add copied api key to [Chirpstack] tenantKey="..."

### In Tenants
1. Create new Tenant (Hermind tenant) alonside chirpstack tentant
2. Open created tetant and copy the id in [Chirpstack] tenantId="..."
3. Check "Tenant can have gateways" on in selected tenant / configuration
4. Press Submit

## From Tenant side bar menu
### Into Applications: 
1. Create new application
2. Copy its id into [Chirpstack] appId="..."

### Into Api keys: 
1. Create new API key
2. Add it to config file into [Chirpstack] apiKey="..."

### In Device profiles:
 1. Add Bolus device profile and add it to [Chirpstack] bolusProfileId="..."
 2. Add Collar device profile and add it to [Chirpstack] collarProfileId="..."  
 3. Enable Class C for Collar

### In Gateways 
1. Add gateway
2. Copy gateway id into [MQTT] gatewayId="..."


## Debug

docker logs -f chirpstack-docker-chirpstack-1


sudo apt install mosquitto-clients

mosquitto_sub -t "#" -v


mosquitto_sub -h localhost -p 1883 -t "#" -v