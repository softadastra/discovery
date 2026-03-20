# Changelog - softadastra/discovery

All notable changes to the **discovery module** are documented in this file.

The `discovery` module is responsible for **detecting and tracking peers on the network**, enabling decentralized communication without relying on a central server.

---

## [0.1.0] - Initial Discovery Layer

### Added

* Core peer representation (`Peer`):

  * Device ID
  * Network address (IP, port)
  * Last seen timestamp
  * Status (alive, unknown)
* `DiscoveryService`:

  * Start/stop discovery
  * Handle incoming peer announcements
* `LanBeacon`:

  * Broadcast presence on LAN
  * Listen for peer announcements
* `PeerRegistry`:

  * Track known peers
  * Store peer state
* Basic event system:

  * `onPeerFound`
  * `onPeerLost` (basic)

### Design

* Fully decentralized (no central server)
* LAN-only discovery
* No dependency on transport or sync modules

---

## [0.1.1] - Stability Improvements

### Improved

* More consistent peer detection under unstable networks
* Reduced duplicate peer events
* Better handling of rapid join/leave cycles

### Fixed

* Edge cases where peers were not detected on first broadcast
* Incorrect updates of last-seen timestamps

---

## [0.2.0] - Peer Lifecycle Management

### Added

* Peer state tracking:

  * alive
  * stale
  * expired
* Expiration logic for inactive peers
* Configurable timeouts for peer validity

### Improved

* Accuracy of peer liveness detection
* Cleaner transitions between peer states

---

## [0.3.0] - Network Behavior Improvements

### Added

* Broadcast retry mechanism
* Configurable beacon intervals
* Support for multiple network interfaces (basic)

### Improved

* Reliability of discovery on noisy LAN environments
* Reduced packet loss impact

---

## [0.4.0] - Registry Enhancements

### Added

* Efficient `PeerRegistry`:

  * Lookup by ID
  * Lookup by address
* Event-driven updates for peer changes

### Improved

* Faster updates when many peers are present
* Reduced redundant registry updates

---

## [0.5.0] - Integration Readiness

### Added

* Structured peer information for transport layer
* Hooks for connection initiation
* Stable peer identity representation

### Improved

* Clear separation between discovery and connection logic
* Better compatibility with sync and transport modules

---

## [0.6.0] - Performance Improvements

### Added

* Lightweight caching for known peers
* Reduced memory overhead in registry

### Improved

* Faster peer detection cycles
* Lower CPU usage during idle discovery

---

## [0.7.0] - Extraction Ready

### Added

* Namespace stabilization (`softadastra::discovery`)
* Public API cleanup
* Documentation for exposed interfaces

### Improved

* Decoupling from application-specific assumptions
* Prepared for reuse in:

  * Softadastra Sync OS
  * SDK
  * Distributed systems frameworks

---

## Roadmap

### Planned

* Secure discovery (signed / authenticated beacons)
* NAT traversal support
* Multi-network discovery (LAN + WAN)
* Relay-assisted discovery
* Peer scoring and prioritization
* Discovery over alternative transports (Bluetooth, etc.)

---

## Philosophy

Discovery is not guaranteed.

> It is a best-effort mechanism to find peers.

---

## Rules

* Never assume peers are reachable
* Never depend on discovery for correctness
* Always expire stale peers
* Always tolerate duplicates

---

## Summary

The `discovery` module ensures:

* Peer detection
* Peer tracking
* Decentralized discovery

It is the **entry point for peer awareness in Softadastra**.
