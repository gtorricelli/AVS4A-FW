# InnoSenT RadarSensing - IMD-2000 / IMD-2002 prototype

The same USART1 acquisition task supports IMD-2000 and IMD-2002 and writes one
aggregate report per second to the dedicated USB service port (`CDC_SERIAL2`).
The interactive shell remains on `CDC_SERIAL1`, so unsolicited radar and PIR
telemetry cannot interfere with commands or command responses.

The three USB CDC instances are assigned as follows:

| CDC instance | Purpose |
|---|---|
| `CDC_SERIAL0` | CPU/Linux binary protocol |
| `CDC_SERIAL1` | Interactive debug shell |
| `CDC_SERIAL2` | Radar reports, raw radar diagnostics and PIR events |

## Hardware connection

| Radar | BD0-0002-000 / STM32H562 | Notes |
|---|---|---|
| UART TX | J6 pin 6 / PB15 (USART1_RX) | 3.3 V TTL |
| UART RX | J6 pin 5 / PB14 (USART1_TX) | 3.3 V TTL |
| GND | GND | Common ground is mandatory |
| Supply | Suitable regulated rail | Follow the sensor datasheet limits |

USART1 is configured as 256000 bit/s, 8 data bits, no parity, 1 stop bit,
without hardware flow control.

## Acquisition flow

After a 200 ms boot delay, the task sends the InnoSenT start command. After a
valid ACK it requests a target list and immediately requests the next list after
a valid response. IMD-2000 responses contain 20 slots of 16 bytes (334-byte
frame). IMD-2002 responses contain 15 slots of 20 bytes (314-byte frame); the
fifth float is the incident azimuth angle.

Every response is validated for delimiters, addresses, function code, target
count and checksum. The sequence number counts lost lists. A response timeout
causes a retry; after three timeouts the start handshake is repeated.

## Sensor and diagnostic selection

Defaults are selected at compile time in `task/radar_task.h`:

```c
#define RADAR_DEFAULT_SENSOR INNOSENT_SENSOR_IMD2000
#define RADAR_DIAGNOSTIC_RAW_DEFAULT 0
```

The debug shell permits runtime selection:

```text
radar                 # current configuration
radar imd2000         # select IMD-2000 and restart acquisition
radar imd2002         # select IMD-2002 and restart acquisition
radar raw on          # send every decoded target tuple to service CDC2
radar raw off         # aggregate output only
```

Raw records contain list ID, target index, range, velocity, signal and ETA.
IMD-2002 records also contain `angle_deg`. Raw mode can produce a high service
port data rate and should be enabled only while collecting diagnostic logs.

The `pir` shell command performs one immediate reading and returns to the
prompt. Automatic `PIR event=n` notifications are sent only to the service
port.

## One-second report

Targets from all valid lists are grouped in range/velocity space. Provisional
parameters in `task/radar_task.h` are:

- `MIN_CLUSTER_POINTS` (default 4 total points/window)
- `RADAR_CLUSTER_RANGE_EPSILON_M` (default 0.50 m)
- `RADAR_CLUSTER_VELOCITY_EPSILON_MPS` (default 0.75 m/s)
- `RADAR_CLUSTER_ANGLE_EPSILON_DEG` (default 15 degrees for IMD-2002)

Example:

```text
RADAR sensor=IMD2000 raw=0 window_ms=1000 frames=10 lost=0 invalid=0 uart_errors=0 rx_overruns=0 clusters=1 min_points=4
CLUSTER id=0 points_mean=2.40 range_mean_m=3.812 range_sd_m=0.064 velocity_mean_mps=-0.310 frames_seen=10 total_points=24
```

For IMD-2002, a `CLUSTER_ANGLE` line follows each cluster and reports the mean
and sample standard deviation of the per-observation angular centroids.

`points_mean` is the cluster point count divided by every valid observation in
the window. Range, velocity and angle statistics are calculated from the
per-observation centroids where that cluster is present.

Thresholds are initial bench values. Preserve raw field logs before defining
production thresholds.

## Host-side module tests

```sh
cd MCU_NDIR_H562_R0/tests
make test
```

Regenerate and compile the final target build with STM32CubeIDE.
