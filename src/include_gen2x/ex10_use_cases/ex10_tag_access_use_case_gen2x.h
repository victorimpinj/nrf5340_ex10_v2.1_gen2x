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

#include "ex10_use_cases/ex10_tag_access_use_case.h"

#include "include_gen2x/ex10_api/application_register_field_enums_gen2x.h"
#include "include_gen2x/ex10_api/application_registers_gen2x.h"
#include "include_gen2x/ex10_api/rf_mode_definitions_gen2x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct These are the use case parameters used to run the inventory.
 * They are generally passed on to the inventory module to setup
 * the inventory rounds.  These also inform the wrapper how to setup
 * the ScanCommandControl and ScanIDCommandControl registers for
 * Gen2 extensions modes (they will be ignored by the firmware if
 * the RF Mode is not a Gen2 extension mode)
 */
struct Ex10TagAccessUseCaseParametersGen2X
{
    uint8_t                                antenna;
    enum RfModesGen2X                      rf_mode;
    int16_t                                tx_power_cdbm;
    uint8_t                                initial_q;
    uint8_t                                session;
    uint8_t                                target;
    uint8_t                                select;
    bool                                   send_selects;
    bool                                   crypto;
    enum ScanCommandControlCodeGen2X       code;
    enum ScanCommandControlCrGen2X         cr;
    enum ScanCommandControlProtectionGen2X cr_protection;
    enum ScanCommandControlIdGen2X         id;
    bool                                   scan_id_enable;
    enum ScanIdCommandControlAppSizeGen2X  app_size;
    uint32_t                               app_id;
};


/**
 * @struct Ex10TagAccessUseCaseGen2X
 */
struct Ex10TagAccessUseCaseGen2X
{
    /**
     * Initialize the Ex10TagAccessUseCase object.
     * This must be called after the Ex10 core has been initialized
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*init)(void);

    /**
     * Release any resources used by the
     * Ex10TagAccessUseCase object.
     * @return struct Ex10Result
     *         Indicates whether the function call passed or failed.
     */
    struct Ex10Result (*deinit)(void);

    /**
     * Register a callback that will be called with the tag read reports
     * and will indicate if the LMAC has successfully halted on the tag
     *
     * @param tag_halted_callback
     *        Notify the client when a tag is singulated. The corresponding
     *        TagRead packet also indicates if the LMAC successfully halted
     *        on the tag.  When this function returns, the LMAC will continue
     *        the inventory round to the next tag.
     */
    void (*register_halted_callback)(
        void (*tag_halted_callback)(struct EventFifoPacket const*,
                                    enum HaltedCallbackResult*,
                                    struct Ex10Result*));

    /**
     * Run an inventory round and halt on all singulated tags
     *
     * @param params a pointer to the Ex10TagAccessUseCaseParameters
     *               structure.
     * @return Info about any encountered errors.
     */
    struct Ex10Result (*run_inventory)(
        struct Ex10TagAccessUseCaseParametersGen2X* params);

    /**
     * Execute Access commands that are enabled.  Should only
     * be called from the halted callback that was registered below.
     * @return The result status of the access commands
     */
    enum TagAccessResult (*execute_access_commands)(void);

    /**
     *  A helper function to retrieve the access packet from the event
     *  fifo packet
     *
     * @return  This function returns a copy of the access packet received
     *          (it is a copy so the buffer can be released)
     */
    struct EventFifoPacket const* (*get_fifo_packet)(void);

    /**
     * A helper function to remove packets from the event fifo queue.
     */
    void (*remove_fifo_packet)(void);

    /**
     * This function verifies that the received Event FIFO packet type is
     * 'Halted' and if confirmed, removes the packet data.
     *
     * @return bool Indicates whether removed EventFifo packet is 'Halted' type.
     * @retval true Received and removed an 'Halted' packet.
     * @retval false Expected Gen2 packet was not received or the received
     *         response was not as expected.
     */
    bool (*remove_halted_packet)(void);

    /**
     * Gets the timeout being used for the use case inventory.
     * @return The timeout value in microseconds
     */
    uint32_t (*get_inventory_timeout_us)(void);

    /**
     * Sets the timeout to wait for the use case inventory to finish.
     * If the time is 0, the timeout will be ignored.
     */
    void (*set_inventory_timeout_us)(uint32_t timeout_us);
};

struct Ex10TagAccessUseCaseGen2X const* get_ex10_tag_access_use_case_gen2x(
    void);

#ifdef __cplusplus
}
#endif
