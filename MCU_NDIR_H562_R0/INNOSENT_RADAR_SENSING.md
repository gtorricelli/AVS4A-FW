# InnoSenT RadarSensing — IMD-2000 prototype

This increment connects an IMD-2000 to USART1 and writes one aggregate report
per second to the existing USB debug console.

## Hardware connection

| IMD-2000 | BD0-0002-000 / STM32H562 | Notes |
|---|---|---|
| UART TX | J6 pin 6 / PB15 (USART1_RX) | 3.3 V TTL |
| UART RX | J6 pin 5 / PB14 (USART1_TX) | 3.3 V TTL |
| GND | GND | Common ground is mandatory |
| Supply | Suitable regulated rail | Follow the sensor datasheet limits |

USART1 is configured as 256000 bit/s, 8 data bits, no parity, 1 stop bit,
without hardware flow control.

## Acquisition flow

After a 200 ms boot delay, the task sends the InnoSenT start command. After a
valid ACK it requests a target list, validates the 334-byte response and
immediately requests the next list. A response timeout causes a retry; after
three timeouts the start handshake is repeated.

Every accepted response is validated for delimiters, source/destination,
function code, target count and checksum. The target-list sequence number is
used to count lost lists.

## One-second report

Targets from all valid lists in the window are grouped in range/velocity space.
The provisional compile-time parameters are in `task/radar_task.h`:

- `MIN_CLUSTER_POINTS` (default 4 total points/window)
- `RADAR_CLUSTER_RANGE_EPSILON_M` (default 0.50 m)
- `RADAR_CLUSTER_VELOCITY_EPSILON_MPS` (default 0.75 m/s)

Clusters are printed in descending order by total point count. Example:

```text
RADAR sensor=IMD2000 window_ms=1000 frames=10 lost=0 invalid=0 uart_errors=0 rx_overruns=0 clusters=1 min_points=4
CLUSTER id=0 points_mean=2.40 range_mean_m=3.812 range_sd_m=0.064 velocity_mean_mps=-0.310 frames_seen=10 total_points=24
```

`points_mean` is the cluster point count divided by every valid observation in
the window. `range_mean_m` is the mean of the per-observation range centroids;
`range_sd_m` is their sample standard deviation. `velocity_mean_mps` is the
mean of the per-observation velocity centroids. Observations where a cluster is
absent affect `points_mean`, but not its centroid statistics.

The thresholds are intentionally initial bench values. Preserve the raw report
logs during the field test so they can be calibrated before adding IMD-2002
angle processing.

## Host-side module tests

The pure protocol and clustering modules can be checked without STM32 tools:

```sh
cd MCU_NDIR_H562_R0/tests
make test
```

The final target build must be regenerated and compiled with STM32CubeIDE so
the new files under `task/` are added to the managed build.
