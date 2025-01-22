/*****************************************************************************
 *                  IMPINJ CONFIDENTIAL AND PROPRIETARY                      *
 *                                                                           *
 * This source code is the property of Impinj, Inc. Your use of this source  *
 * code in whole or in part is subject to your applicable license terms      *
 * from Impinj.                                                              *
 * Contact support@impinj.com for a copy of the applicable Impinj license    *
 * terms.                                                                    *
 *                                                                           *
 * (c) Copyright 2022 - 2024 Impinj, Inc. All rights reserved.               *
 *                                                                           *
 *****************************************************************************/

/**
 * @file gen2x_continuous_inventory_example.c
 * @details  The continuous inventory example below is optimized for read rates.
 * This example sets up the Ex1ContinuousInventoryUseCase and calls
 * the continuous_inventory() function to execute the inventory round.
 * This means the SDK is responsible for starting each inventory round,
 * allowing faster read rates. For better performance, this example is
 * currently not configured to print each inventoried EPC.
 * This can be changed by using the 'verbose_gen2x' inventory configuration
 * parameter. The inventory example below is optimized for approximately 256
 * tags in FOV. To adjust dynamic Q algorithm for other tag populations, the
 * following parameters should be updated:
 *
 *  - initial_q
 */

#include <stdlib.h>
#include "calibration.h"

#include "src/ex10_utils/ex10_inventory_command_line_helper.h"
#include "src/ex10_utils/ex10_use_case_example_errors.h"

#include "ex10_api/board_init_core.h"
//#include "ex10_api/event_fifo_printer.h"
#include "ex10_api/ex10_active_region.h"
#include "ex10_api/ex10_utils.h"
#include "ex10_regulatory/ex10_default_region_names.h"
#include "ex10_api/event_packet_parser.h"
#include "ex10_modules/ex10_ramp_module_manager.h"

//#include "ex10_use_cases/ex10_continuous_inventory_use_case.h"

#include "calibration_lut_gen2x.h"
#include "include_gen2x/ex10_api/event_fifo_printer_gen2x.h"
#include "include_gen2x/ex10_use_cases/ex10_continuous_inventory_use_case_gen2x.h"

#include <zephyr/kernel.h>

//#define OUTPUT_EVERYTHING
enum Verbosity const verbose_gen2x = SILENCE;
#define TOI_LOGS_ENABLED 0

// The number of microseconds per second.
#define us_per_s 1000000u

struct InventoryOptions inventory_options = {
    .region_name   = "FCC",
    .read_rate     = 0u,
    .antenna       = 1u,
    .frequency_khz = 0u,
    .remain_on     = false,
    .tx_power_cdbm = 3000,
    .mode          = {.rf_mode_id = mode_124},
    .target_spec   = 'D',
    .initial_q     = 8,
    .session       = SessionS2,
    .cr            = CrID16,
    .protection    = ProtectionCRC5,
    .id            = IdFull,
};

static const struct StopConditions stop_conditions = {
    .max_number_of_tags   = 0u,
    .max_duration_us      = 2u * us_per_s,
    .max_number_of_rounds = 0u,
};

static struct ContinuousInventorySummary continuous_inventory_summary = {
    .duration_us                = 0,
    .number_of_inventory_rounds = 0,
    .number_of_tags             = 0,
    .reason                     = SRNone,
    .last_op_id                 = 0,
    .last_op_error              = 0,
    .packet_rfu_1               = 0};


/* Print the header for the logs output */
static void print_logs_header(void)
{
    ex10_ex_printf("SKU,Tx power (cdBm),Reader mode,Target spec,Initial Q,Region Name,Duration (ms),Session,Scan cr,Scan protection,Scan id,Local time\r\n");
    ex10_ex_printf("%x,%i,%u,%c,%u,", 
            0x710, 
            inventory_options.tx_power_cdbm, 
            inventory_options.mode.raw, 
            inventory_options.target_spec, 
            inventory_options.initial_q);
    ex10_ex_printf("%s", inventory_options.region_name);
    ex10_ex_printf(",%u", continuous_inventory_summary.duration_us / us_per_s);
    ex10_ex_printf(",%u", inventory_options.session);
    ex10_ex_printf(",%u,%u,%u,%u", 
            inventory_options.session, 
            inventory_options.cr, 
            inventory_options.protection, 
            inventory_options.id);
    ex10_ex_printf("\n");

    ex10_ex_printf("Timestamp(us),Target,ChannelFreq(kHz),Antenna,ReaderMode,PacketType,"
                "EpcLength(bytes),EPC,PhaseStart,PhaseEnd,RSSI(cdBm),MultiReadTimes\n");

}
static void printf_tag_read(
    struct EventFifoPacket const* packet,
    enum RfModes                  rf_mode,
    uint8_t                       antenna,
    uint8_t                       target,
    uint32_t                      channel_freq_kHz,
    enum RfFilter                 rf_filter,
    uint16_t                      adc_temperature)
{
    struct TagReadFields tag_read =
    get_ex10_event_parser()->get_tag_read_fields(
        packet->dynamic_data,
        packet->dynamic_data_length,
        packet->static_data->tag_read.type,
        packet->static_data->tag_read.tid_offset);
    
    uint8_t epc_byte_length = 0;
    if (inventory_options.id == 0)
    {
        epc_byte_length = 3;    // +0x <- 1-byte
    }
    else if (inventory_options.id == 1)
    {
        epc_byte_length = 8;
    }
    else if (inventory_options.id == 2)
    {
        epc_byte_length = 10;
    }
    else
    {
        epc_byte_length = (packet->dynamic_data_length - 2 - 2); // Exclude PC & CRC
    }

    // FW timestamp, Target, Antenna, reader mode, EPC or other ID, Phase start, Phase end, RSSI
    ex10_ex_printf("%u,%c,%u,%u,%u,T%u,%u,", 
            packet->us_counter,
            target,
            channel_freq_kHz,
            antenna,
            rf_mode,
            packet->packet_type,
            epc_byte_length);

    if(packet->static_data->tag_read.type == 4)
    {
        // For Nickname reads, the nickname will show up in the PC word
        ex10_ex_printf("0x%04X,", (int) ex10_swap_bytes(*(tag_read.pc)));
    }
    else
    {
        uint8_t const* data = (uint8_t const*)tag_read.epc;
        for (size_t iter = 0u; iter < tag_read.epc_length; ++iter, ++data)
        {
            // if ((iter % 4u == 0u) && (iter > 0u))
            // {
            //     fprintf(file_pointer, " ");
            // }
            ex10_ex_printf("%02X", *data);
        }
        ex10_ex_printf(",");
    }

    int16_t const compensated_rssi_cdbm =
        get_ex10_calibration()->get_compensated_rssi(
            packet->static_data->tag_read.rssi,
            (uint16_t)rf_mode,
            (const struct RxGainControlFields*)&packet->static_data->tag_read
                .rx_gain_settings,
            antenna,
            rf_filter,
            adc_temperature);

    ex10_ex_printf("%u,%u,%i", 
            packet->static_data->tag_read.rf_phase_begin,
            packet->static_data->tag_read.rf_phase_end,
            compensated_rssi_cdbm);

    ex10_ex_printf("\n");
}

static void packet_subscriber_callback(struct EventFifoPacket const* packet,
                                       struct Ex10Result*            result_ptr)
{
    *result_ptr                  = make_ex10_success();

    if (verbose_gen2x >= PRINT_EVERYTHING)
    {
        get_ex10_event_fifo_printer()->print_packets(packet);
    }
    else if (verbose_gen2x == PRINT_SCOPED_EVENTS)
    {
        if (packet->packet_type == TagRead ||
            packet->packet_type == ContinuousInventorySummary)
        {
            get_ex10_event_fifo_printer()->print_packets(packet);
        }
    }

    if (packet->packet_type == ContinuousInventorySummary)
    {
        continuous_inventory_summary =
            packet->static_data->continuous_inventory_summary;
    }
    // Generate logs for ToI process
    if ((TOI_LOGS_ENABLED) && (packet->packet_type == TagRead))
    {
        // TODO: get these somehow
        uint16_t reader_mode = inventory_options.mode.rf_mode_id;
        uint8_t antenna = inventory_options.antenna;
        char target = inventory_options.target_spec;

        // BUG: If the channel has changed since the tag read occurred, this will be out of date
        uint32_t channel_freq_kHz = get_ex10_active_region()->get_active_channel_khz();

        // BUG: Prints the FW timer packet time, not the time since start of inventory
        printf_tag_read(packet,
                         reader_mode,
                         antenna,
                         target,
                         channel_freq_kHz,
                         get_ex10_active_region()->get_rf_filter(),
                         get_ex10_ramp_module_manager()->retrieve_adc_temperature());
    }

}

static struct Ex10Result gen2x_continuous_inventory_example(void)
{
    ex10_ex_printf("Starting Gen2X continuous inventory example\n");
    print_logs_header();

    struct Ex10ContinuousInventoryUseCaseGen2X const* ciucg =
        get_ex10_continuous_inventory_use_case_gen2x();

    ciucg->init();
    // Clear out any left over packets
    // ex10_discard_packets(false, true, false);
    ciucg->register_packet_subscriber_callback(packet_subscriber_callback);
    ciucg->enable_packet_filter(verbose_gen2x < PRINT_EVERYTHING);

    uint8_t const target =
        (inventory_options.target_spec == 'A') ? target_A : target_B;
    bool const dual_target = (inventory_options.target_spec == 'D');
    //bool const dual_target = false;
    struct Ex10ContinuousInventoryUseCaseParametersGen2X params = {
        .antenna         = inventory_options.antenna,
        .rf_mode         = inventory_options.mode.rf_mode_id,
        .tx_power_cdbm   = inventory_options.tx_power_cdbm,
        .initial_q       = inventory_options.initial_q,
        .session         = (uint8_t)inventory_options.session,
        .target          = target,
        .select          = (uint8_t)SelectAll,
        .send_selects    = false,
        .stop_conditions = &stop_conditions,
        .dual_target     = dual_target,
        .crypto          = false,
        .code            = CodeAntipodal,
        .cr              = inventory_options.cr,
        .cr_protection   = inventory_options.protection,
        .id              = inventory_options.id,
        .scan_id_enable  = false,
        .app_size        = AppSize8Bits,
        .app_id          = 0u};

    if (inventory_options.frequency_khz != 0)
    {
        get_ex10_active_region()->set_single_frequency(
            inventory_options.frequency_khz);
    }

    if (inventory_options.remain_on)
    {
        get_ex10_active_region()->disable_regulatory_timers();
    }

    struct Ex10Result ex10_result = ciucg->continuous_inventory(&params);
    if (ex10_result.error)
    {
        // Something bad happened so we exit with an error
        // (we assume that the user was notified by whatever set the error)
        ex10_discard_packets(true, true, true);
        return ex10_result;
    }

    // if (continuous_inventory_summary.reason != SRMaxDuration)
    // {
    //     // Unexpected stop reason so we exit with an error
    //     ex10_ex_eprintf("Unexpected stop reason: %u\n",
    //                     continuous_inventory_summary.reason);
    //     return make_ex10_app_error(Ex10StopReasonUnexpected);
    // }

    uint32_t const read_rate =
        ex10_calculate_read_rate(continuous_inventory_summary.number_of_tags,
                                 continuous_inventory_summary.duration_us);

    ex10_ex_printf(
        "\nRead rate = %u - tags: %u / seconds %u.%03u (Mode %u)\n",
        read_rate,
        continuous_inventory_summary.number_of_tags,
        continuous_inventory_summary.duration_us / us_per_s,
        (continuous_inventory_summary.duration_us % us_per_s) / 1000u,
        params.rf_mode);

    if (continuous_inventory_summary.number_of_tags == 0)
    {
        ex10_ex_printf("No tags found in inventory\n");
        return make_ex10_app_error(Ex10ApplicationTagCount);
    }

    if ((inventory_options.read_rate) &&
        (read_rate < inventory_options.read_rate))
    {
        ex10_ex_printf("Read rate of %u below minimal threshold of %u\n",
                       read_rate,
                       inventory_options.read_rate);
        return make_ex10_app_error(Ex10ApplicationReadRate);
    }

    return make_ex10_success();
}

int gen2x_continuous_inventory(int min_read_rate)
{
    struct Ex10Result ex10_result;
   /*
    struct Ex10Result ex10_result =
       // ex10_parse_inventory_command_line(&inventory_options, argc, argv);
       ex10_parse_inventory_command_line(&inventory_options, 0, "v");
    if (ex10_result.error || ex10_inventory_command_line_help_requested())
    {
         printk("error\n");
        //return -1;
    }
    */
    //k_msleep(1000);
    enum Ex10RegionId const region_id =
        get_ex10_default_region_names()->get_region_id(
            inventory_options.region_name);

    ex10_result = ex10_core_board_setup(region_id, DEFAULT_SPI_CLOCK_HZ);
    if (ex10_result.error)
    {
        ex10_ex_eprintf("ex10_core_board_setup() failed:\n");
        print_ex10_result(ex10_result);
        ex10_core_board_teardown();
        return -1;
    }

    // if (inventory_options.frequency_khz != 0)
    // {
    //     get_ex10_active_region()->set_single_frequency(
    //         inventory_options.frequency_khz);
    // }

    // if (inventory_options.remain_on)
    // {
    //     get_ex10_active_region()->disable_regulatory_timers();
    // }

    set_calibration_lut_gen2x();
    get_ex10_calibration()->init(get_ex10_protocol());
    ex10_result = ex10_set_default_gpio_setup();
    if (ex10_result.error == false)
    {
        ex10_result = gen2x_continuous_inventory_example();
        if (ex10_result.error == true)
        {
            print_ex10_app_result(ex10_result);
        }
    }
    else
    {
        ex10_ex_eprintf("ex10_set_default_gpio_setup() failed:\n");
        print_ex10_result(ex10_result);
    }

    ex10_ex_printf(
        "Ending continuous inventory example: "
        "%s\n",
        ex10_result.error ? "failed" : "success");

    ex10_core_board_teardown();
    return ex10_result.error ? -1 : 0;
}
