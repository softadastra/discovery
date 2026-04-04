/*
 * DiscoveryContext.hpp
 */

#ifndef SOFTADASTRA_DISCOVERY_CONTEXT_HPP
#define SOFTADASTRA_DISCOVERY_CONTEXT_HPP

#include <stdexcept>

#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

namespace softadastra::discovery::core
{
  namespace transport_engine = softadastra::transport::engine;

  /**
   * @brief Shared runtime dependencies for the discovery module
   *
   * The discovery layer uses TransportEngine as its integration point
   * for handing discovered peers to the transport layer.
   */
  struct DiscoveryContext
  {
    /**
     * Discovery configuration
     */
    const DiscoveryConfig *config{nullptr};

    /**
     * Transport engine used as the peer integration point
     */
    transport_engine::TransportEngine *transport{nullptr};

    /**
     * @brief Check whether the context is usable
     */
    bool valid() const noexcept
    {
      return config != nullptr &&
             transport != nullptr &&
             config->valid();
    }

    /**
     * @brief Return discovery configuration
     */
    const DiscoveryConfig &config_ref() const
    {
      if (config == nullptr)
      {
        throw std::runtime_error(
            "DiscoveryContext: config is null");
      }

      return *config;
    }

    /**
     * @brief Return transport engine reference
     */
    transport_engine::TransportEngine &transport_ref() const
    {
      if (transport == nullptr)
      {
        throw std::runtime_error(
            "DiscoveryContext: transport is null");
      }

      return *transport;
    }
  };

} // namespace softadastra::discovery::core

#endif
