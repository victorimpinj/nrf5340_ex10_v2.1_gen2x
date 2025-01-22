/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2023 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ex10_api/application_register_definitions.h"
#include "ex10_api/ex10_inventory.h"
#include "ex10_api/ex10_inventory_sequence.h"
#include "ex10_api/rf_mode_definitions.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum SequenceSelectConfig
 * Used to change which Selects are currently enabled and run the SendSelectOp.
 * Commands are still added, removed, maintained, etc through the
 * Ex10Gen2TxCommandManager, but the selects can be enabled and disabled
 * through this activity. This allows the user to set up selects beforehand,
 * and reconfigure which ones are sent during the sequence.
 *
 * Note: This command does not allow updating select data on the fly to avoid
 * race conditions. The user can, however, manipulate Ex10Gen2TxCommandManager
 * through the user event FIFO callback while the sequence is running if
 * dynamic buffer updates are required.
 */
struct SequenceSelectConfig
{
    /// An array of bools, each of which can enable a select in the
    /// Ex10Gen2TxCommandManager. For more information on the max size of
    /// this array as well as the selects that are enabled.
    /// @see Ex10Gen2TxCommandManager
    bool const* enables;

    /// The size of the enabled bool array. For more information on the max
    /// size of the array, see documentation for Ex10Gen2TxCommandManager.
    uint8_t enable_array_size;

    /// A flag to determine if the activity will run the SendSelectOp.
    /// If false, the enables will be updated, but the op will not be started.
    /// If true, the enables will be updated and then the op will be started.
    bool run_op_flag;
};

/**
 * @enum SequenceCwConfig
 * Used to turn on CW or change certain aspects of CW at given activity points.
 */
struct SequenceCwConfig
{
    /// The antenna to switch to if CW is off. If CW is currently on and the
    /// antenna setting differs from the current antenna, CW is turned off
    /// and back on with the new setting.
    uint8_t antenna;

    /// The RF mode to switch to. If CW is off, this will be the RF mode used
    /// during the ramp-up which occurs on this activity. CW does not need to
    /// be turned off and on for this change.
    enum RfModes rf_mode;

    /// The transmitter power, in 0.01 dBm, units to be used. If CW is off, a
    /// ramp up sequence will be performed. If CW is on, a dynamic power ramp
    /// will occur to change the power.
    int16_t tx_power_cdbm;
};

/**
 * @enum SequenceActivityConfigType
 * Used to identify that the type of activity at the current index of the
 * sequence.
 */
enum SequenceActivityConfigType
{
    /// The type of activity at the current index of
    /// Ex10ActivitySequence.sequence_activities.type_id is unknown.
    SEQUENCE_CONFIG_UNKNOWN = 0,

    /// Identify the Ex10ActivitySequence.sequence_activities as containing an
    /// array of type struct InventoryRoundConfigBasic.
    SEQUENCE_INVENTORY_ROUND_CONFIG = 1,

    /// Used to change which selects are currently enabled and run the send
    /// select op.
    SEQUENCE_SELECT_CONFIG = 2,

    /// Identify the Ex10ActivitySequence.sequence_activities as containing an
    /// array of type struct SequenceCwConfig. Allows a simple CW configuration
    /// update in the middle of the sequence
    SEQUENCE_POWER_RAMP_CONFIG = 3,

    /// Identify the Ex10ActivitySequence.sequence_activities as containing a CW
    /// off activity. This ramps down CW.
    SEQUENCE_CW_OFF_CONFIG = 4,
};

/**
 * @struct SequenceActivity
 * A struct which is used as a single element in the
 * Ex10ActivitySequence.sequence_activities list. A single instance of this
 * struct denotes the type of activity as well as any additional information
 * needed to perform that activity.
 */
struct SequenceActivity
{
    /// An identifier that indicates the type for the current activity in the
    /// sequence. Note that the index of type_id matches that of configs.
    enum SequenceActivityConfigType type_id;

    /// A pointer to the first node of the InventoryRoundConfigBasic objects.
    /// This type is void* to allow for extensibility in types of activities.
    /// The type of this pointer is determined with the type_id member.
    void const* config;
};

/**
 * @struct Ex10ActivitySequence
 * A struct which contains an array of SequenceActivity types to allow execution
 * of a sequence of activities.
 */
struct Ex10ActivitySequence
{
    /// The number of activities in the sequence_activities array.
    size_t count;

    /// The sequence of activities to execute. This is an array of
    /// SequenceActivity types, each of which denote the type of activity and
    /// any configs which may go along with it.
    struct SequenceActivity const* sequence_activities;
};

#ifdef __cplusplus
}
#endif
