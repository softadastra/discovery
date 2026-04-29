/**
 *
 *  @file DiscoveryRegistry.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_REGISTRY_HPP
#define SOFTADASTRA_DISCOVERY_REGISTRY_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/core/DiscoveryAnnouncement.hpp>
#include <softadastra/discovery/peer/DiscoveredPeer.hpp>
#include <softadastra/discovery/types/DiscoveryPeerState.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>

namespace softadastra::discovery::peer
{
  namespace core = softadastra::discovery::core;
  namespace types = softadastra::discovery::types;
  namespace transport_core = softadastra::transport::core;
  namespace core_time = softadastra::core::time;

  /**
   * @brief In-memory registry of discovered peers.
   *
   * DiscoveryRegistry stores discovered peers indexed by node id.
   *
   * This first version is intentionally simple:
   * - in-memory only
   * - unordered_map-backed
   * - node_id as stable key
   *
   * It does not perform network I/O.
   * It only tracks discovery state.
   */
  class DiscoveryRegistry
  {
  public:
    /**
     * @brief Internal peer map type.
     */
    using Map = std::unordered_map<std::string, DiscoveredPeer>;

    /**
     * @brief Creates an empty discovery registry.
     */
    DiscoveryRegistry() = default;

    /**
     * @brief Inserts or replaces a discovered peer.
     *
     * Invalid peers are ignored.
     *
     * @param peer Discovered peer.
     */
    void upsert(const DiscoveredPeer &peer)
    {
      if (!peer.is_valid())
      {
        return;
      }

      peers_[peer.node_id()] = peer;
    }

    /**
     * @brief Inserts or replaces a discovered peer by move.
     *
     * Invalid peers are ignored.
     *
     * @param peer Discovered peer.
     */
    void upsert(DiscoveredPeer &&peer)
    {
      if (!peer.is_valid())
      {
        return;
      }

      const auto key = peer.node_id();

      peers_[key] = std::move(peer);
    }

    /**
     * @brief Inserts or updates a peer from an announcement.
     *
     * Existing peers are refreshed and marked alive.
     *
     * @param announcement Discovery announcement.
     */
    void upsert_announcement(core::DiscoveryAnnouncement announcement)
    {
      if (!announcement.is_valid())
      {
        return;
      }

      auto *existing = find(announcement.node_id);

      if (existing != nullptr)
      {
        existing->update(std::move(announcement));
        return;
      }

      DiscoveredPeer peer{std::move(announcement)};
      peer.mark_alive();

      upsert(std::move(peer));
    }

    /**
     * @brief Returns true if the peer exists.
     *
     * @param node_id Peer node id.
     * @return true if found.
     */
    [[nodiscard]] bool contains(const std::string &node_id) const
    {
      return peers_.find(node_id) != peers_.end();
    }

    /**
     * @brief Gets a discovered peer by copy.
     *
     * @param node_id Peer node id.
     * @return DiscoveredPeer if found, std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<DiscoveredPeer>
    get(const std::string &node_id) const
    {
      const auto it = peers_.find(node_id);

      if (it == peers_.end())
      {
        return std::nullopt;
      }

      return it->second;
    }

    /**
     * @brief Finds a discovered peer without copying.
     *
     * @param node_id Peer node id.
     * @return DiscoveredPeer pointer, or nullptr.
     */
    [[nodiscard]] DiscoveredPeer *
    find(const std::string &node_id) noexcept
    {
      const auto it = peers_.find(node_id);

      if (it == peers_.end())
      {
        return nullptr;
      }

      return &it->second;
    }

    /**
     * @brief Finds a discovered peer without copying.
     *
     * @param node_id Peer node id.
     * @return DiscoveredPeer pointer, or nullptr.
     */
    [[nodiscard]] const DiscoveredPeer *
    find(const std::string &node_id) const noexcept
    {
      const auto it = peers_.find(node_id);

      if (it == peers_.end())
      {
        return nullptr;
      }

      return &it->second;
    }

    /**
     * @brief Removes a peer by node id.
     *
     * @param node_id Peer node id.
     * @return true if a peer was removed.
     */
    bool erase(const std::string &node_id)
    {
      return peers_.erase(node_id) > 0;
    }

    /**
     * @brief Updates peer state.
     *
     * @param node_id Peer node id.
     * @param state New discovery state.
     * @return true if the peer was found and updated.
     */
    bool set_state(
        const std::string &node_id,
        types::DiscoveryPeerState state)
    {
      if (!types::is_valid(state))
      {
        return false;
      }

      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->state = state;
      peer->touch();

      return true;
    }

    /**
     * @brief Marks a peer as discovered.
     *
     * @param node_id Peer node id.
     * @return true if updated.
     */
    bool mark_discovered(const std::string &node_id)
    {
      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->mark_discovered();

      return true;
    }

    /**
     * @brief Marks a peer as alive.
     *
     * @param node_id Peer node id.
     * @return true if updated.
     */
    bool mark_alive(const std::string &node_id)
    {
      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->mark_alive();

      return true;
    }

    /**
     * @brief Marks a peer as stale.
     *
     * @param node_id Peer node id.
     * @return true if updated.
     */
    bool mark_stale(const std::string &node_id)
    {
      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->mark_stale();

      return true;
    }

    /**
     * @brief Marks a peer as expired.
     *
     * @param node_id Peer node id.
     * @return true if updated.
     */
    bool mark_expired(const std::string &node_id)
    {
      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->mark_expired();

      return true;
    }

    /**
     * @brief Updates the last seen timestamp using the current time.
     *
     * @param node_id Peer node id.
     * @return true if updated.
     */
    bool touch(const std::string &node_id)
    {
      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->touch();

      return true;
    }

    /**
     * @brief Updates the last seen timestamp explicitly.
     *
     * @param node_id Peer node id.
     * @param timestamp Last seen timestamp.
     * @return true if updated.
     */
    bool touch(
        const std::string &node_id,
        core_time::Timestamp timestamp)
    {
      if (!timestamp.is_valid())
      {
        return false;
      }

      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->last_seen_at = timestamp;

      return true;
    }

    /**
     * @brief Increments error count for a peer.
     *
     * @param node_id Peer node id.
     * @return true if updated.
     */
    bool mark_error(const std::string &node_id)
    {
      auto *peer = find(node_id);

      if (peer == nullptr)
      {
        return false;
      }

      peer->mark_error();

      return true;
    }

    /**
     * @brief Updates stale and expired peer states.
     *
     * Peers older than stale_after become Stale.
     * Peers older than expire_after become Expired.
     *
     * @param now Current timestamp.
     * @param stale_after Duration after which peers become stale.
     * @param expire_after Duration after which peers become expired.
     * @return Number of peers whose state changed.
     */
    std::size_t refresh_states(
        core_time::Timestamp now,
        core_time::Duration stale_after,
        core_time::Duration expire_after)
    {
      if (!now.is_valid())
      {
        return 0;
      }

      std::size_t changed = 0;

      for (auto &[_, peer] : peers_)
      {
        if (!peer.last_seen_at.is_valid())
        {
          continue;
        }

        const auto age_ms =
            now.millis() - peer.last_seen_at.millis();

        if (age_ms >= expire_after.millis() &&
            peer.state != types::DiscoveryPeerState::Expired)
        {
          peer.state = types::DiscoveryPeerState::Expired;
          ++changed;
          continue;
        }

        if (age_ms >= stale_after.millis() &&
            peer.state != types::DiscoveryPeerState::Stale)
        {
          peer.state = types::DiscoveryPeerState::Stale;
          ++changed;
        }
      }

      return changed;
    }

    /**
     * @brief Removes expired peers.
     *
     * @return Number of removed peers.
     */
    std::size_t prune_expired()
    {
      std::size_t removed = 0;

      for (auto it = peers_.begin(); it != peers_.end();)
      {
        if (it->second.expired())
        {
          it = peers_.erase(it);
          ++removed;
        }
        else
        {
          ++it;
        }
      }

      return removed;
    }

    /**
     * @brief Returns the number of registered peers.
     *
     * @return Peer count.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
      return peers_.size();
    }

    /**
     * @brief Returns true if registry is empty.
     *
     * @return true when empty.
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return peers_.empty();
    }

    /**
     * @brief Removes all peers.
     */
    void clear() noexcept
    {
      peers_.clear();
    }

    /**
     * @brief Returns read-only access to the internal map.
     *
     * @return Peer map.
     */
    [[nodiscard]] const Map &entries() const noexcept
    {
      return peers_;
    }

    /**
     * @brief Returns all discovered peers.
     *
     * @return Discovered peers.
     */
    [[nodiscard]] std::vector<DiscoveredPeer> all() const
    {
      std::vector<DiscoveredPeer> result;
      result.reserve(peers_.size());

      for (const auto &[_, peer] : peers_)
      {
        result.push_back(peer);
      }

      return result;
    }

    /**
     * @brief Returns all transport peer infos.
     *
     * Expired peers are skipped.
     *
     * @return Transport peer info list.
     */
    [[nodiscard]] std::vector<transport_core::PeerInfo>
    transport_peers() const
    {
      std::vector<transport_core::PeerInfo> result;

      for (const auto &[_, peer] : peers_)
      {
        if (!peer.expired() && peer.to_peer_info().is_valid())
        {
          result.push_back(peer.to_peer_info());
        }
      }

      return result;
    }

    /**
     * @brief Returns all available peers.
     *
     * @return Available discovered peers.
     */
    [[nodiscard]] std::vector<DiscoveredPeer> available_peers() const
    {
      std::vector<DiscoveredPeer> result;

      for (const auto &[_, peer] : peers_)
      {
        if (peer.available())
        {
          result.push_back(peer);
        }
      }

      return result;
    }

    /**
     * @brief Returns all alive peers.
     *
     * @return Alive discovered peers.
     */
    [[nodiscard]] std::vector<DiscoveredPeer> alive_peers() const
    {
      return by_state(types::DiscoveryPeerState::Alive);
    }

    /**
     * @brief Returns all stale peers.
     *
     * @return Stale discovered peers.
     */
    [[nodiscard]] std::vector<DiscoveredPeer> stale_peers() const
    {
      return by_state(types::DiscoveryPeerState::Stale);
    }

    /**
     * @brief Returns all expired peers.
     *
     * @return Expired discovered peers.
     */
    [[nodiscard]] std::vector<DiscoveredPeer> expired_peers() const
    {
      return by_state(types::DiscoveryPeerState::Expired);
    }

  private:
    /**
     * @brief Returns peers matching a state.
     *
     * @param state Discovery peer state.
     * @return Matching peers.
     */
    [[nodiscard]] std::vector<DiscoveredPeer>
    by_state(types::DiscoveryPeerState state) const
    {
      std::vector<DiscoveredPeer> result;

      for (const auto &[_, peer] : peers_)
      {
        if (peer.state == state)
        {
          result.push_back(peer);
        }
      }

      return result;
    }

  private:
    Map peers_{};
  };

} // namespace softadastra::discovery::peer

#endif // SOFTADASTRA_DISCOVERY_REGISTRY_HPP
