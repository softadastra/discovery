/**
 *
 *  @file Discovery.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra Discovery
 *
 */

#ifndef SOFTADASTRA_DISCOVERY_HPP
#define SOFTADASTRA_DISCOVERY_HPP

/*
 * Public legacy API
 */
#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/discovery/DiscoveryService.hpp>
#include <softadastra/discovery/Peer.hpp>

/*
 * Types
 */
#include <softadastra/discovery/types/DiscoveryMessageType.hpp>
#include <softadastra/discovery/types/DiscoveryPeerState.hpp>
#include <softadastra/discovery/types/DiscoveryStatus.hpp>

/*
 * Core
 */
#include <softadastra/discovery/core/DiscoveryAnnouncement.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/core/DiscoveryEnvelope.hpp>
#include <softadastra/discovery/core/DiscoveryMessage.hpp>

/*
 * Encoding
 */
#include <softadastra/discovery/encoding/DiscoveryDecoder.hpp>
#include <softadastra/discovery/encoding/DiscoveryEncoder.hpp>

/*
 * Peer registry
 */
#include <softadastra/discovery/peer/DiscoveredPeer.hpp>
#include <softadastra/discovery/peer/DiscoveryRegistry.hpp>

/*
 * Backend
 */
#include <softadastra/discovery/backend/IDiscoveryBackend.hpp>
#include <softadastra/discovery/backend/UdpDiscoveryBackend.hpp>

/*
 * Client / Server
 */
#include <softadastra/discovery/client/DiscoveryClient.hpp>
#include <softadastra/discovery/server/DiscoveryServer.hpp>

/*
 * Engine
 */
#include <softadastra/discovery/engine/DiscoveryEngine.hpp>

/*
 * Utils
 */
#include <softadastra/discovery/utils/Datagram.hpp>

/*
 * Platform
 */
#ifdef __linux__
#include <softadastra/discovery/platform/linux/UdpSocket.hpp>
#endif

#endif // SOFTADASTRA_DISCOVERY_HPP
