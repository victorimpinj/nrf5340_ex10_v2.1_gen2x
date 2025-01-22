/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2024 Impinj, Inc. All rights reserved.                      *
 *                                                                           *
 *****************************************************************************/

#pragma once

#include <stddef.h>

#include "ex10_api/application_registers.h"
#include "ex10_api/ex10_inventory_sequence.h"
#include "ex10_api/ex10_regulatory.h"
#include "ex10_api/ex10_result.h"
#include "include_gen2x/ex10_api/rf_mode_definitions_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

/// The number of RF modes contained within an Gen2X AutoSet mode.
#define AUTOSET_RF_MODE_COUNT_GEN2X ((size_t)2u)

/**
 * @enum AutoSetModeIdGen2X
 * Enumeration of Gen2X AutoSet modes supported by the SDK.
 */
enum AutoSetModeIdGen2X
{
    AutoSetModeGen2X_Invalid = 0,  ///< Indicates the AutoSet mode is invalid.

    // FCC
    // All Skus
    AutoSetModeGen2X_5123 = 5123,
    AutoSetModeGen2X_5141 = 5141,
    AutoSetModeGen2X_5146 = 5146,
    AutoSetModeGen2X_5185 = 5185,

    // E510,E710, E910 Only
    AutoSetModeGen2X_5124 = 5124,
    AutoSetModeGen2X_5148 = 5148,

    // ETSI_LOWER (EU1):
    // All Skus
    AutoSetModeGen2X_5222 = 5222,
    AutoSetModeGen2X_5241 = 5241,
    AutoSetModeGen2X_5244 = 5244,
    AutoSetModeGen2X_5285 = 5285,

    // ETSI_UPPER (EU2):
    // All Skus
    AutoSetModeGen2X_5324 = 5324,
    AutoSetModeGen2X_5342 = 5342,
    AutoSetModeGen2X_5343 = 5343,
    AutoSetModeGen2X_5382 = 5382,

    // E510,E710, E910 Only
    AutoSetModeGen2X_5323 = 5323,
    AutoSetModeGen2X_5345 = 5345,
};

/**
 * @struct AutoSetRfModesGen2X
 * A container of RF modes aggregated by an AutoSet mode.
 */
struct AutoSetRfModesGen2X
{
    /// The AutoSet mode ID.
    enum AutoSetModeIdGen2X autoset_mode_id;
    /// The RF modes contained as an array.
    enum RfModes const* rf_mode_list;
    /// The number of RF modes within the array.
    size_t rf_modes_length;
};

/**
 * @struct Ex10AutoSetModesGen2X
 * Each AutoSet mode accessor references an array of RF modes containing
 * AUTOSET_RF_MODE_COUNT elements.
 *
 * These RF modes, from lowest to highest index, within the array, provide
 * better sensitivity with slower tag access.
 *
 * The AutoSet modes are specific to a given region.
 */
struct Ex10AutoSetModesGen2X
{
    /**
     * Get a pointer to a struct of AutoSetRfModesGen2X based on the AutoSet
     * mode passed in and the region name.
     *
     * @param autoset_mode_id The AutoSet mode.
     *
     * @return struct AutoSetRfModesGen2X const* The RF modes associated with an
     *         AutoSet mode. NULL if the function call fails.
     * @note   Invalid region names will return the "Other regions" RF modes.
     */
    struct AutoSetRfModesGen2X const* (*get_autoset_rf_modes_gen2x)(
        enum AutoSetModeIdGen2X autoset_mode_id);

    /**
     * Initialize the array of struct InventoryRoundConfigBasic nodes.
     *
     * @param [out] inventory_round_config
     *   An array of struct InventoryRoundConfigBasic nodes to initialize with
     *   AutoSet RF modes and related inventory parameters.
     * @param inventory_round_config_size
     *   The number of nodes in the inventory_round_config array.
     *   Used to bounds check the inventory_round_config array size against
     *   the autoset_rf_modes array size.
     * @param autoset_rf_modes
     *   A const pointer struct AutoSetRfModesGen2X, which contains an array of
     *   RF modes for running inventory rounds.
     * @param antenna       The antenna port to use during the inventory round.
     * @param tx_power_cdbm The transmit power in cdBm to use during the round.
     * @param target        The inventory flag to target during the inventory
     *                      round; i.e. target_A or target_B.
     * @param session       The inventory session for tags to participate in.
     *
     * @return struct Ex10Result
     *         Indicates whether the function was successful or not.
     */
    struct Ex10Result (*init_autoset_basic_inventory_sequence)(
        struct InventoryRoundConfigBasic* inventory_round_config,
        size_t                            inventory_round_config_size,
        struct AutoSetRfModesGen2X const* autoset_rf_modes,
        uint8_t                           antenna,
        int16_t                           tx_power_cdbm,
        uint8_t                           target,
        enum InventoryRoundControlSession session);
};

struct Ex10AutoSetModesGen2X const* get_ex10_autoset_modes_gen2x(void);

#ifdef __cplusplus
}
#endif
