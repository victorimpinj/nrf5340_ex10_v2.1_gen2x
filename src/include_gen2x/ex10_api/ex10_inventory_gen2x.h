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
#include <stdint.h>

#include "ex10_api/ex10_inventory.h"

#include "include_gen2x/ex10_api/application_register_definitions_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct InventoryParamsGen2X
 */
struct InventoryParamsGen2X
{
    uint8_t                              antenna;
    enum RfModes                         rf_mode;
    int16_t                              tx_power_cdbm;
    struct InventoryRoundControlFields   inventory_config;
    struct InventoryRoundControl_2Fields inventory_config_2;
    struct ScanCommandControlFields      scan_control;
    struct ScanIdCommandControlFields    scan_id_control;
    bool                                 send_selects;
};

struct Ex10InventoryGen2X
{
    /**
     * Sends all enabled selects if directed by the user, then starts an
     * inventory round.  If this is used with a non Gen2X RF mode,
     * it will operate as the normal Ex10Inventory function but write
     * the scan and scan_id fields which will be ignored by the firmware.
     *
     * This function runs the SendSelectOp if send_selects is true,
     * followed by the StartInventoryRoundOp.
     *
     * @param inventory_config      @see struct InventoryRoundControlFields
     * @param inventory_config_2    @see struct InventoryRoundControl_2Fields
     * @param scan_control          @see struct ScanCommandControlFields
     * @param scan_id_control       @see struct ScanIdCommandControlFields
     * @param send_selects          When set to true the select op is run.
     *                              The user should have already updated the
     *                              needed info in the Gen2 Tx command buffer
     *                              and appropriate control registers.
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*run_inventory)(
        struct InventoryRoundControlFields const*   inventory_config,
        struct InventoryRoundControl_2Fields const* inventory_config_2,
        struct ScanCommandControlFields const*      scan_control,
        struct ScanIdCommandControlFields const*    scan_id_control,
        bool                                        send_selects);

    /**
     * Starts an Inventory by running the StartInventoryRoundOp.
     * This function calls Ex10Inventory.run_inventory() after performing
     * the preconditions:
     * - Ramp up the transmitter, if necessary.
     * - Update the temperature measurement.
     * - Set the RF mode.
     *
     * @param antenna               The antenna to use.
     * @param rf_mode               The RF mode to use.
     * @param tx_power_cdbm         The transmitter power, in centi-dB.
     * @param inventory_config      @see struct InventoryRoundControlFields
     * @param inventory_config_2    @see struct InventoryRoundControl_2Fields
     * @param scan_control          @see struct ScanCommandControlFields
     * @param scan_id_control       @see struct ScanIdCommandControlFields
     * @param send_selects          When set to true the select op is run.
     *                              The user should have already updated the
     *                              needed info in the Gen2 Tx command buffer
     *                              and appropriate control registers.
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*start_inventory)(
        uint8_t                                     antenna,
        enum RfModes                                rf_mode,
        int16_t                                     tx_power_cdbm,
        struct InventoryRoundControlFields const*   inventory_config,
        struct InventoryRoundControl_2Fields const* inventory_config_2,
        struct ScanCommandControlFields const*      scan_control,
        struct ScanIdCommandControlFields const*    scan_id_control,
        bool                                        send_selects);

    /**
     * Checks to see if the LMAC is currently in the halted state.
     * (the LMAC could have ramped down due to regulatory for example)
     *
     * @return True if the LMAC is halted, False if not
     */
    bool (*inventory_halted)(void);

    /**
     * Checks for errors within ex10_result and attempts to map them to
     * continuous inventory errors. If the error does not correlate to a known
     * continuous inventory error, the continuous inventory error will be
     * SRReasonUnknown.
     *
     * @return The continuous inventory error mapped from the passed Ex10Result.
     */
    enum StopReason (*ex10_result_to_continuous_inventory_error)(
        struct Ex10Result ex10_result);

    /**
     * Enables a feature in the LMAC to accept both CRC5 or CRC5+ CR protection
     * values when either CRC5 or CRC5+ are the CR protection setting for
     * the Scan or ScanID commands.  This defaults to disabled.
     *
     * @param enable  true will enable accepting both; false will only
     *                accept the CR protection selected by the Scan/ScanID
     */
    void (*enable_crc5_and_crc5plus)(bool enable);
};

const struct Ex10InventoryGen2X* get_ex10_inventory_gen2x(void);

#ifdef __cplusplus
}
#endif
