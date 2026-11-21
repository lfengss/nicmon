# NICMon - Network Interface Card Monitor

This project is maintained by Lin Zhiyong.

A lightweight Windows utility for monitoring network adapter performance in real-time.

## Features

- **Real-time throughput tracking** - Monitor RX/TX bandwidth per adapter
- **Latency measurement** - Track packet round-trip times
- **Per-adapter statistics** - Detailed counters including drops, errors, and multicast
- **Low overhead** - Uses OID queries with adaptive polling intervals
- **Multi-adapter support** - Monitor all NICs simultaneously
- **Alert thresholds** - Configurable notifications for latency/drop anomalies

## Build

### Prerequisites
- Visual Studio 2022 (or Build Tools) with C++ workload
- Windows SDK 10.0.22621.0 or later
- CMake 3.20+

`
git clone https://github.com/lfengss/nicmon.git
cd nicmon
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
`

## Usage

`
# Show all adapters with live stats
nicmon.exe --list

# Monitor specific adapter
nicmon.exe --monitor "Ethernet"

# Export stats to CSV
nicmon.exe --monitor "Ethernet" --export stats.csv
`

## How It Works

NICMon queries adapter statistics through the Windows NDIS API using GetIfEntry2 and OID requests. It calculates deltas between polling intervals to derive real-time throughput, and tracks min/max/average latency through ICMP probes.

## License

[MIT](LICENSE)