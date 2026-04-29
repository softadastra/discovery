/**
 *
 *  @file DiscoveryContext.hpp
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

#ifndef SOFTADASTRA_DISCOVERY_CONTEXT_HPP
#define SOFTADASTRA_DISCOVERY_CONTEXT_HPP

#include <softadastra/core/Core.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

namespace softadastra::discovery::core
{
  namespace transport_engine = softadastra::transport::engine;
  namespace core_errors = softadastra::core::errors;
  namespace core_types = softadastra::core::types;

  /**
   * @brief Shared runtime dependencies for the discovery module.
   *
   * DiscoveryContext groups the objects required by the discovery layer.
   *
   * It provides access to:
   * - the active DiscoveryConfig
   * - the TransportEngine used as the peer integration point
   *
   * The context does not own these objects.
   * The caller must ensure they outlive the discovery components using them.
   */
  struct DiscoveryContext
  {
    /**
     * @brief Discovery configuration.
     */
    const DiscoveryConfig *config{nullptr};

    /**
     * @brief Transport engine used as the peer integration point.
     */
    transport_engine::TransportEngine *transport{nullptr};

    /**
     * @brief Result type returned by checked accessors.
     */
    template <typename T>
    using Result = core_types::Result<T, core_errors::Error>;

    /**
     * @brief Creates an empty invalid context.
     */
    DiscoveryContext() = default;

    /**
     * @brief Creates a discovery context from dependencies.
     *
     * @param discovery_config Discovery configuration reference.
     * @param transport_engine Transport engine reference.
     */
    DiscoveryContext(
        const DiscoveryConfig &discovery_config,
        transport_engine::TransportEngine &transport_engine) noexcept
        : config(&discovery_config),
          transport(&transport_engine)
    {
    }

    /**
     * @brief Returns true if the context is usable.
     *
     * @return true when config and transport engine are present and valid.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
      return config != nullptr &&
             transport != nullptr &&
             config->is_valid() &&
             transport->is_running();
    }

    /**
     * @brief Returns the discovery configuration pointer.
     *
     * @return DiscoveryConfig pointer, or nullptr.
     */
    [[nodiscard]] const DiscoveryConfig *
    config_ptr() const noexcept
    {
      return config;
    }

    /**
     * @brief Returns the transport engine pointer.
     *
     * @return TransportEngine pointer, or nullptr.
     */
    [[nodiscard]] transport_engine::TransportEngine *
    transport_ptr() const noexcept
    {
      return transport;
    }

    /**
     * @brief Returns the discovery configuration as a Result.
     *
     * @return DiscoveryConfig pointer on success, Error on failure.
     */
    [[nodiscard]] Result<const DiscoveryConfig *>
    config_checked() const
    {
      if (config == nullptr)
      {
        return Result<const DiscoveryConfig *>::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidState,
                "discovery context config is null"));
      }

      if (!config->is_valid())
      {
        return Result<const DiscoveryConfig *>::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidArgument,
                "discovery config is invalid"));
      }

      return Result<const DiscoveryConfig *>::ok(config);
    }

    /**
     * @brief Returns the transport engine as a Result.
     *
     * @return TransportEngine pointer on success, Error on failure.
     */
    [[nodiscard]] Result<transport_engine::TransportEngine *>
    transport_checked() const
    {
      if (transport == nullptr)
      {
        return Result<transport_engine::TransportEngine *>::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidState,
                "discovery context transport engine is null"));
      }

      if (!transport->is_running())
      {
        return Result<transport_engine::TransportEngine *>::err(
            core_errors::Error::make(
                core_errors::ErrorCode::InvalidState,
                "transport engine is not running"));
      }

      return Result<transport_engine::TransportEngine *>::ok(transport);
    }

    /**
     * @brief Returns true if a configuration is present.
     *
     * @return true when config is not null.
     */
    [[nodiscard]] bool has_config() const noexcept
    {
      return config != nullptr;
    }

    /**
     * @brief Returns true if a transport engine is present.
     *
     * @return true when transport is not null.
     */
    [[nodiscard]] bool has_transport() const noexcept
    {
      return transport != nullptr;
    }
  };

} // namespace softadastra::discovery::core

#endif // SOFTADASTRA_DISCOVERY_CONTEXT_HPP
