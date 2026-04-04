# softadastra/discovery

> Peer discovery layer for local-first distributed systems.

The `discovery` module is responsible for **finding and tracking peers on the network**.

It enables devices to:

> Discover each other automatically without relying on any central server.

---

## Purpose

The goal of `softadastra/discovery` is simple:

> Detect available peers and maintain an up-to-date registry of reachable devices.

---

## Core Principle

> Find peers. Do not communicate data.

This module:

* Detects peers
* Tracks their presence
* Exposes their availability

It does **not** exchange application data.

---

## Responsibilities

The `discovery` module provides:

* Peer detection (LAN)
* Peer registration
* Peer state tracking
* Liveness monitoring

---

## What this module does NOT do

* No data transfer (transport module)
* No sync logic (sync module)
* No durability (wal module)
* No filesystem access (fs module)

👉 It only discovers and tracks peers.

---

## Design Principles

### 1. Decentralized

No central server is required.

---

### 2. Best-effort

Discovery is not guaranteed.

The system must tolerate missing or delayed peers.

---

### 3. Lightweight

Discovery should be:

* Fast
* Low bandwidth
* Low overhead

---

### 4. Ephemeral

Peer presence is temporary:

* Devices may appear/disappear
* State must expire

---

## Core Components

### Peer

Represents a discovered device.

Contains:

* Device ID
* Address (IP, port)
* Last seen timestamp
* Status (alive, stale)

---

### DiscoveryService

Main entry point.

Responsible for:

* Starting discovery
* Receiving peer announcements
* Updating registry

---

### LanBeacon

Handles:

* Broadcasting presence
* Listening for other peers
* Periodic announcements

---

### PeerRegistry

Maintains:

* List of known peers
* Their current state
* Expiration logic

---

## Example Usage

```cpp id="ex9"
#include <softadastra/discovery/DiscoveryService.hpp>

using namespace softadastra::discovery;

DiscoveryService discovery;

discovery.onPeerFound([](const Peer& peer) {
    // Connect via transport
});

discovery.start();
```

---

## Discovery Flow

### Announce

1. Node broadcasts presence on LAN
2. Includes:

   * Device ID
   * Address
   * Basic metadata

---

### Listen

1. Node listens for incoming beacons
2. Parses peer information
3. Updates registry

---

### Expire

1. Peer not seen for a period
2. Marked as stale
3. Eventually removed

---

## Integration

Used by:

* transport (to initiate connections)
* sync (indirectly via transport)
* app layer

---

## Network Model

* LAN-based discovery
* Broadcast / multicast (MVP)
* No internet dependency

---

## Failure Model

Handles:

* Peers going offline
* Network instability
* Delayed announcements
* Duplicate discoveries

---

## MVP Scope

* Local network only
* Broadcast discovery
* Basic peer registry
* No authentication

---

## Roadmap

* Secure discovery (signed beacons)
* Multi-network discovery (LAN + WAN)
* Relay-assisted discovery
* NAT traversal support
* Peer scoring and prioritization

---

## Rules

* Never transport application data
* Never assume peer availability
* Always expire stale peers
* Always tolerate duplicates

---

## Philosophy

Discovery is not truth.

> It is a hint about who might be reachable.

---

## Summary

* Finds peers on the network
* Tracks their presence
* Enables connection setup
* Fully decentralized

---

## License

See root LICENSE file.
