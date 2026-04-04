/*
 * DiscoveryRegistry.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_REGISTRY_HPP
#define SOFTADASTRA_DISCOVERY_REGISTRY_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <softadastra/discovery/peer/DiscoveredPeer.hpp>
#include <softadastra/discovery/types/DiscoveryPeerState.hpp>

namespace softadastra::discovery::peer
{
  namespace types = softadastra::discovery::types;

  /**
   * @brief In-memory registry of discovered peers
   */
  class DiscoveryRegistry
  {
  public:
    /**
     * @brief Insert or replace a discovered peer
     */
    void upsert(const DiscoveredPeer &peer)
    {
      peers_[peer.announcement.node_id] = peer;
    }

    /**
     * @brief Insert or replace a discovered peer by move
     */
    void upsert(DiscoveredPeer &&peer)
    {
      peers_[peer.announcement.node_id] = std::move(peer);
    }

    /**
     * @brief Return true if the peer exists
     */
    bool contains(const std::string &node_id) const
    {
      return peers_.find(node_id) != peers_.end();
    }

    /**
     * @brief Get a discovered peer copy
     */
    std::optional<DiscoveredPeer> get(const std::string &node_id) const
    {
      auto it = peers_.find(node_id);
      if (it == peers_.end())
      {
        return std::nullopt;
      }

      return it->second;
    }

    /**
     * @brief Remove a peer by node id
     */
    bool erase(const std::string &node_id)
    {
      return peers_.erase(node_id) > 0;
    }

    /**
     * @brief Update peer state
     */
    bool set_state(const std::string &node_id, types::DiscoveryPeerState state)
    {
      auto it = peers_.find(node_id);
      if (it == peers_.end())
      {
        return false;
      }

      it->second.state = state;
      return true;
    }

    /**
     * @brief Update last seen timestamp
     */
    bool touch(const std::string &node_id, std::uint64_t now_ms)
    {
      auto it = peers_.find(node_id);
      if (it == peers_.end())
      {
        return false;
      }

      it->second.last_seen_at = now_ms;
      return true;
    }

    /**
     * @brief Increment error count for a peer
     */
    bool mark_error(const std::string &node_id)
    {
      auto it = peers_.find(node_id);
      if (it == peers_.end())
      {
        return false;
      }

      ++it->second.error_count;
      return true;
    }

    /**
     * @brief Number of registered peers
     */
    std::size_t size() const noexcept
    {
      return peers_.size();
    }

    /**
     * @brief Return true if registry is empty
     */
    bool empty() const noexcept
    {
      return peers_.empty();
    }

    /**
     * @brief Remove all peers
     */
    void clear()
    {
      peers_.clear();
    }

    /**
     * @brief Return all discovered peers
     */
    std::vector<DiscoveredPeer> all() const
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
     * @brief Return all alive peers
     */
    std::vector<DiscoveredPeer> alive_peers() const
    {
      std::vector<DiscoveredPeer> result;

      for (const auto &[_, peer] : peers_)
      {
        if (peer.state == types::DiscoveryPeerState::Alive)
        {
          result.push_back(peer);
        }
      }

      return result;
    }

    /**
     * @brief Return all stale peers
     */
    std::vector<DiscoveredPeer> stale_peers() const
    {
      std::vector<DiscoveredPeer> result;

      for (const auto &[_, peer] : peers_)
      {
        if (peer.state == types::DiscoveryPeerState::Stale)
        {
          result.push_back(peer);
        }
      }

      return result;
    }

    /**
     * @brief Return all expired peers
     */
    std::vector<DiscoveredPeer> expired_peers() const
    {
      std::vector<DiscoveredPeer> result;

      for (const auto &[_, peer] : peers_)
      {
        if (peer.state == types::DiscoveryPeerState::Expired)
        {
          result.push_back(peer);
        }
      }

      return result;
    }

  private:
    std::unordered_map<std::string, DiscoveredPeer> peers_;
  };

} // namespace softadastra::discovery::peer

#endif
