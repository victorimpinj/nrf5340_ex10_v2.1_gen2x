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

#include "include_gen2x/ex10_api/ex10_autoset_modes_gen2x.h"
#include "ex10_api/ex10_macros.h"
#include "ex10_api/ex10_regulatory.h"


// clang-format off
// FCC:
// All Skus
static enum RfModes const autoset_5123_rf_modes_gen2x[] = {(enum RfModes)mode_4123, mode_123};
static enum RfModes const autoset_5141_rf_modes_gen2x[] = {(enum RfModes)mode_4141, mode_141};
static enum RfModes const autoset_5146_rf_modes_gen2x[] = {(enum RfModes)mode_4146, mode_146};
static enum RfModes const autoset_5185_rf_modes_gen2x[] = {(enum RfModes)mode_4185, mode_185};

// E510,E710, E910 Only
static enum RfModes const autoset_5124_rf_modes_gen2x[] = {(enum RfModes)mode_4124, mode_124};
static enum RfModes const autoset_5148_rf_modes_gen2x[] = {(enum RfModes)mode_4148, mode_148};

// ETSI_LOWER (EU1):
// All Skus
static enum RfModes const autoset_5222_rf_modes_gen2x[] = {(enum RfModes)mode_4222, mode_222};
static enum RfModes const autoset_5241_rf_modes_gen2x[] = {(enum RfModes)mode_4241, mode_241};
static enum RfModes const autoset_5244_rf_modes_gen2x[] = {(enum RfModes)mode_4244, mode_244};
static enum RfModes const autoset_5285_rf_modes_gen2x[] = {(enum RfModes)mode_4285, mode_285};

// ETSI_UPPER (EU2):
// All Skus
static enum RfModes const autoset_5324_rf_modes_gen2x[] = {(enum RfModes)mode_4324, mode_324};
static enum RfModes const autoset_5342_rf_modes_gen2x[] = {(enum RfModes)mode_4342, mode_342};
static enum RfModes const autoset_5343_rf_modes_gen2x[] = {(enum RfModes)mode_4343, mode_343};
static enum RfModes const autoset_5382_rf_modes_gen2x[] = {(enum RfModes)mode_4382, mode_382};

// E510,E710, E910 Only
static enum RfModes const autoset_5323_rf_modes_gen2x[] = {(enum RfModes)mode_4323, mode_323};
static enum RfModes const autoset_5345_rf_modes_gen2x[] = {(enum RfModes)mode_4345, mode_345};


static struct AutoSetRfModesGen2X const autoset_modes_table[] = {
    {
        .autoset_mode_id = AutoSetModeGen2X_5123,
        .rf_mode_list    = autoset_5123_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5123_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5124,
        .rf_mode_list    = autoset_5124_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5124_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5141,
        .rf_mode_list    = autoset_5141_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5141_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5146,
        .rf_mode_list    = autoset_5146_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5146_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5148,
        .rf_mode_list    = autoset_5148_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5148_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5185,
        .rf_mode_list    = autoset_5185_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5185_rf_modes_gen2x),
    },


    {
        .autoset_mode_id = AutoSetModeGen2X_5222,
        .rf_mode_list    = autoset_5222_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5222_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5241,
        .rf_mode_list    = autoset_5241_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5241_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5244,
        .rf_mode_list    = autoset_5244_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5244_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5285,
        .rf_mode_list    = autoset_5285_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5285_rf_modes_gen2x),
    },


    {
        .autoset_mode_id = AutoSetModeGen2X_5324,
        .rf_mode_list    = autoset_5324_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5324_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5342,
        .rf_mode_list    = autoset_5342_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5342_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5343,
        .rf_mode_list    = autoset_5343_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5343_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5382,
        .rf_mode_list    = autoset_5382_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5382_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5323,
        .rf_mode_list    = autoset_5323_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5323_rf_modes_gen2x),
    },
    {
        .autoset_mode_id = AutoSetModeGen2X_5345,
        .rf_mode_list    = autoset_5345_rf_modes_gen2x,
        .rf_modes_length = ARRAY_SIZE(autoset_5345_rf_modes_gen2x),
    },
};

static struct AutoSetRfModesGen2X const* get_autoset_rf_modes_gen2x(
    enum AutoSetModeIdGen2X autoset_mode_id)
{
    for (size_t iter = 0u; iter < ARRAY_SIZE(autoset_modes_table); ++iter)
    {
        if (autoset_modes_table[iter].autoset_mode_id == autoset_mode_id)
        {
            return &autoset_modes_table[iter];
        }
    }

    return NULL;  // AutoSetModeId match not found.
}

// clang-format on

static struct Ex10Result init_autoset_basic_inventory_sequence(
    struct InventoryRoundConfigBasic* inventory_round_config,
    size_t                            inventory_round_config_size,
    struct AutoSetRfModesGen2X const* autoset_rf_modes,
    uint8_t                           antenna,
    int16_t                           tx_power_cdbm,
    uint8_t                           target,
    enum InventoryRoundControlSession session)
{
    if (inventory_round_config == NULL)
    {
        return make_ex10_sdk_error(Ex10ModuleAutoSetModes,
                                   Ex10SdkErrorNullPointer);
    }

    if (inventory_round_config_size < autoset_rf_modes->rf_modes_length)
    {
        return make_ex10_sdk_error(Ex10ModuleAutoSetModes,
                                   Ex10SdkErrorBadParamValue);
    }

    struct InventoryRoundControlFields const inventory_config_template = {
        .initial_q             = 8u,
        .max_q                 = 15u,
        .min_q                 = 0u,
        .num_min_q_cycles      = 2u,
        .fixed_q_mode          = false,
        .q_increase_use_query  = false,
        .q_decrease_use_query  = false,
        .session               = session,
        .select                = SelectAll,  // Used in Query command Sel
        .target                = target,
        .halt_on_all_tags      = false,
        .fast_id_enable        = false,
        .tag_focus_enable      = false,
        .auto_access           = false,
        .abort_on_fail         = false,
        .halt_on_fail          = false,
        .always_ack            = false,
        .use_tag_read_extended = false,
    };

    struct InventoryRoundControl_2Fields const inventory_config_2_template = {
        .max_queries_since_valid_epc                = 16u,
        .Reserved0                                  = 0u,
        .starting_min_q_count                       = 0u,
        .starting_max_queries_since_valid_epc_count = 0u,
    };

    for (size_t iter = 0u; iter < autoset_rf_modes->rf_modes_length; ++iter)
    {
        struct InventoryRoundConfigBasic* basic_config =
            &inventory_round_config[iter];
        enum RfModes const rf_mode = autoset_rf_modes->rf_mode_list[iter];

        basic_config->antenna       = antenna;
        basic_config->rf_mode       = rf_mode;
        basic_config->tx_power_cdbm = tx_power_cdbm;

        basic_config->inventory_config   = inventory_config_template;
        basic_config->inventory_config_2 = inventory_config_2_template;

        basic_config->send_selects = false;
    }

    return make_ex10_success();
}

struct Ex10AutoSetModesGen2X const* get_ex10_autoset_modes_gen2x(void)
{
    static struct Ex10AutoSetModesGen2X autoset_modes_instance = {
        .get_autoset_rf_modes_gen2x = get_autoset_rf_modes_gen2x,
        .init_autoset_basic_inventory_sequence =
            init_autoset_basic_inventory_sequence,
    };

    return &autoset_modes_instance;
}
