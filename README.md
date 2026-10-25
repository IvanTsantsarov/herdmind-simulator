# HerdMind Simulator

HerdMind Simulator is a Qt/C++ desktop application for developing and testing HerdMind's livestock-monitoring and virtual-fencing system. It combines a visual herd/meadow simulation with device-management and LoRaWAN integration components, including MQTT and ChirpStack REST API support.

> **Project status:** Development / engineering simulator. Hardware and LoRaWAN features depend on the configured devices, credentials, network-server configuration, and firmware. This README documents the repository as provided; it does not imply that every feature is production-ready.

## Main components

- **Herd and animal simulation** — herd, animal, shepherd, meadow and scene classes used to represent animals and their environment.
- **Virtual fencing and movement** — virtual-fence logic and simulation timing.
- **Device management** — device manager and dialogs for registering animals, configuring devices, and inspecting messages.
- **LoRaWAN protocol support** — binary protocol helpers in `hardware/protocol.h` and LoRa device abstractions for simulated and hardware-backed devices.
- **Collar firmware / hardware code** — `hardware/collar/` contains collar components such as GPS, screen, battery, signaling, lights, sound, shock, memory and LoRaWAN handling.
- **Bolus code** — `hardware/bolus/` contains bolus-related sensor and LoRaWAN implementation.
- **Gateway simulation** — `hardware/gateway/`.
- **Backend connectivity** — MQTT client and application-level MQTT handling (`mqtt.*`, `apimqtt.*`) plus ChirpStack REST API support (`apirest.*`).
- **ChirpStack development stack** — `chirpstack-docker/` contains Docker Compose and configuration for ChirpStack, Gateway Bridge, REST API, Mosquitto, PostgreSQL and Redis.

## Requirements

### Desktop application

- Qt with the modules used by the project: **Core, GUI, Widgets, Positioning, Network, MQTT and SerialPort**
- C++17-compatible compiler and Qt build tools (`qmake`, `make`)
- OpenSSL crypto library (`-lcrypto`)
- A supported desktop environment for the Qt GUI

The `.pro` file is `HerdmindSimulator.pro`. The project uses `ONPC` as a qmake define for the desktop build.

### ChirpStack stack

- Docker Engine
- Docker Compose v2 (`docker compose`)
- Ports available on the host as listed below

## Build and run the desktop application

From the repository root:

```bash
qmake HerdmindSimulator.pro
make -j"$(nproc)"
```

Run the executable produced by the build (usually `./HerdmindSimulator`, depending on the qmake target and build configuration).

The application restores `environment.ini`, `settings.ini` and `settings_external.ini` from its Qt resources when they are missing, and creates its save directory in the current working directory. Run it from a writable working directory so its settings and saved data can be written there.

If qmake cannot find a required Qt module, install the corresponding Qt development package or use a Qt kit that includes that module. Ensure Qt MQTT and Qt SerialPort are installed.

## Start the local ChirpStack stack

```bash
cd chirpstack-docker
docker compose up -d
docker compose ps
```

Stop the stack without deleting persistent data:

```bash
docker compose down
```

To stop it and remove Compose-managed volumes (this **deletes the persistent PostgreSQL and Redis data**):

```bash
docker compose down -v
```

### Services and host ports

Based on `chirpstack-docker/docker-compose.yml`:

| Service | Host port | Purpose |
|---|---:|---|
| ChirpStack | `8080/tcp` | Web UI and HTTP API |
| ChirpStack REST API | `8090/tcp` | REST/gRPC-gateway API endpoint |
| Gateway Bridge (UDP packet forwarder) | `1700/udp` | Semtech UDP packet-forwarder traffic |
| Gateway Bridge Basic Station | `3001/tcp` | Basic Station connection |
| Mosquitto | `1883/tcp` | MQTT broker |
| PostgreSQL | not published | Internal database |
| Redis | not published | Internal state/cache |

The application and host-side MQTT clients normally connect to `localhost:1883`. Containers connect to the broker using the Compose service hostname `mosquitto` on port `1883`. The ChirpStack UI is normally at `http://localhost:8080`; the REST API is bound at `http://localhost:8090`.

The provided configuration enables anonymous MQTT access (`allow_anonymous true`) and uses example/default development credentials and a placeholder ChirpStack API secret. **Do not expose this stack to an untrusted network as configured.** Configure authentication, restrict published ports/firewall access, and replace development secrets before any shared or production deployment.

## Logging and troubleshooting

### Follow logs for all services

Run these commands from `chirpstack-docker/`:

```bash
docker compose logs --follow --tail=200
```

Follow one service:

```bash
docker compose logs --follow --tail=200 chirpstack
docker compose logs --follow --tail=200 chirpstack-gateway-bridge
docker compose logs --follow --tail=200 chirpstack-gateway-bridge-basicstation
docker compose logs --follow --tail=200 chirpstack-rest-api
docker compose logs --follow --tail=200 mosquitto
docker compose logs --follow --tail=200 postgres
docker compose logs --follow --tail=200 redis
```

Use `Ctrl+C` to stop following logs; this does not stop the containers.

### Using the container names shown by `docker ps`

The currently observed Compose container names include `chirpstack-docker-chirpstack-1`, `chirpstack-docker-chirpstack-gateway-bridge-1`, `chirpstack-docker-chirpstack-gateway-bridge-basicstation-1`, `chirpstack-docker-chirpstack-rest-api-1` and `chirpstack-docker-mosquitto-1`. You can also read logs directly:

```bash
sudo docker logs --follow --tail=200 chirpstack-docker-chirpstack-1
sudo docker logs --follow --tail=200 chirpstack-docker-chirpstack-gateway-bridge-1
sudo docker logs --follow --tail=200 chirpstack-docker-chirpstack-gateway-bridge-basicstation-1
sudo docker logs --follow --tail=200 chirpstack-docker-chirpstack-rest-api-1
sudo docker logs --follow --tail=200 chirpstack-docker-mosquitto-1
```

Compose service names are preferable when running commands from the Compose directory because they remain stable if Compose changes the generated container names.

### ChirpStack log level

Edit:

`chirpstack-docker/configuration/chirpstack/chirpstack.toml`

The provided configuration currently sets:

```toml
[logging]
level = "debug"
```

Supported levels in the configuration comments are `trace`, `debug`, `info`, `warn` and `error`. Use `debug` while diagnosing device activation, uplinks, downlinks, integrations and frame-counter issues. Use `info` for quieter normal operation. After changing the setting, recreate/restart the service:

```bash
docker compose restart chirpstack
docker compose logs --follow --tail=200 chirpstack
```

### Gateway Bridge log level

Edit:

`chirpstack-docker/configuration/chirpstack-gateway-bridge/chirpstack-gateway-bridge.toml`

The supplied file sets:

```toml
[general]
log_level = 5
```

For Gateway Bridge versions/configuration that use the documented numeric levels, `5` is the debug level. Check the configuration comments/documentation for the exact version in use if you change this value. Restart and follow logs:

```bash
docker compose restart chirpstack-gateway-bridge
docker compose logs --follow --tail=200 chirpstack-gateway-bridge
```

The Basic Station bridge uses a separate configuration file selected by its Compose command:

`configuration/chirpstack-gateway-bridge/chirpstack-gateway-bridge-basicstation-eu868.toml`

Check that file's `[general]` logging configuration too if you are troubleshooting Basic Station connections. Restart it after changes:

```bash
docker compose restart chirpstack-gateway-bridge-basicstation
docker compose logs --follow --tail=200 chirpstack-gateway-bridge-basicstation
```

### Mosquitto logging

The current Mosquitto configuration is:

`chirpstack-docker/configuration/mosquitto/config/mosquitto.conf`

It currently contains:

```conf
listener 1883
allow_anonymous true
```

To enable detailed broker logs, add or adjust these directives:

```conf
log_dest stdout
log_type all
connection_messages true
```

`log_dest stdout` is important for seeing broker logs with `docker compose logs`. Restart Mosquitto after editing:

```bash
docker compose restart mosquitto
docker compose logs --follow --tail=200 mosquitto
```

`log_type all` can be noisy. Once troubleshooting is complete, reduce logging to the categories needed. Do not enable anonymous access on a broker exposed to untrusted networks.

### Check MQTT traffic

If `mosquitto_sub` is installed on the host, subscribe to all topics while debugging:

```bash
mosquitto_sub -h localhost -p 1883 -t '#' -v
```

This can expose device telemetry and application messages; avoid leaving it running or sharing its output if the data is sensitive. The Compose configuration uses the `eu868` topic prefix for Gateway Bridge events, states and commands, while ChirpStack's application MQTT integration uses its own ChirpStack topic structure.

### Common diagnostics

```bash
# Service state and port mappings
docker compose ps

# Last 300 lines from the relevant services
docker compose logs --tail=300 chirpstack chirpstack-gateway-bridge mosquitto

# Check whether host ports are listening
ss -lntup | grep -E ':(8080|8090|1883|1700|3001)\b'
```

For an uplink that appears at the gateway but not on the device page, inspect the path in order: gateway packet reception, Gateway Bridge logs, MQTT broker connectivity/topics, ChirpStack logs, device DevEUI/DevAddr and activation keys, frame-counter validation, and application/device assignment. A radio transmission result alone does not prove that ChirpStack accepted the frame or that a downlink was scheduled and received.

## MQTT and ChirpStack configuration

- The desktop application has an MQTT client abstraction in `mqtt.*`; `ApiMqtt` in `apimqtt.*` provides application-level device messaging.
- MQTT settings are loaded through `QSettings`; the application uses `settings.ini` and `settings_external.ini`. Inspect the settings dialog and bundled resource versions for the actual keys and values expected by your build.
- ChirpStack's server configuration is in `chirpstack-docker/configuration/chirpstack/chirpstack.toml`.
- Gateway Bridge configuration files are in `chirpstack-docker/configuration/chirpstack-gateway-bridge/`.
- Mosquitto configuration is in `chirpstack-docker/configuration/mosquitto/config/mosquitto.conf`.
- PostgreSQL initialization scripts are in `chirpstack-docker/configuration/postgresql/initdb/`.

Keep API tokens, network session keys, application keys and other credentials out of source control and logs. The offline JWT check in `chirpstack.h` validates a token's structure and expiry claim; it does not cryptographically verify a token signature and must not be treated as authentication.

## Repository map

```text
.
├── HerdmindSimulator.pro
├── main.cpp
├── mainwindow.*                 # Main GUI
├── herd.* / animal.*            # Herd and animal model
├── meadow.* / scene.*           # Environment and visualization
├── virtualfence.*               # Virtual-fencing logic
├── devmanager.*                 # Device management
├── mqtt.* / apimqtt.*           # MQTT client and application messaging
├── apirest.*                    # REST API integration
├── hardware/
│   ├── bolus/                   # Bolus code
│   ├── collar/                  # Collar components and LoRaWAN device
│   ├── gateway/                 # Gateway simulation
│   └── protocol.h               # Binary protocol helpers
└── chirpstack-docker/
    ├── docker-compose.yml
    └── configuration/           # ChirpStack, Gateway Bridge, MQTT, PostgreSQL
```

## Development notes

- The project targets C++17 and uses Qt's signal/slot architecture.
- `hardware/protocol.h` defines protocol helpers, including timestamps relative to 2000-01-01 and binary integer/coordinate encoding.
- The project contains both simulated LoRa devices and hardware-oriented collar/bolus implementations. Keep simulator behavior, wire protocol and embedded firmware behavior aligned when changing packet formats.
- When debugging LoRaWAN, distinguish radio-level transmission success from network-server acceptance and end-device downlink reception.
- Clear Gateway HT-ML7603: 
* ivants@ivants-home:~$ ssh -o KexAlgorithms=+diffie-hellman-group14-sha1 -o HostKeyAlgorithms=+ssh-rsa root@192.168.1.29
* The default root password for Heltec devices is heltec.org
* Then execute: root@HT-M7603:~#> /var/lora_pkt_fwd.log
* with ">"
