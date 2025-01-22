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

#include "calibration.h"
#include "ex10_api/ex10_print.h"

#include "include_gen2x/ex10_api/event_fifo_printer_gen2x.h"


static void print_event_tag_read_extended(struct EventFifoPacket const* packet)
{
    get_ex10_event_fifo_printer()->print_event_tag_read(packet);

    struct TagReadExtendedGen2X const* tag_read_extended =
        ((struct TagReadExtendedGen2X const*)&packet->static_data
             ->tag_read_extended);

    switch (tag_read_extended->cr)
    {
        case TagReadCrId32:
            ex10_printf(", ID32: 0x%08X", tag_read_extended->cr_value);
            break;
        case TagReadCrId16:
            ex10_printf(", ID16: 0x%04X", tag_read_extended->cr_value);
            break;
        case TagReadCrStoredCrc:
            ex10_printf(", StoredCRC: 0x%04X", tag_read_extended->cr_value);
            break;
        case TagReadCrRn16:
            ex10_printf(", RN16: 0x%04X", tag_read_extended->cr_value);
            break;
        default:
            ex10_printf("Unexpected tag read CR type: %d",
                        tag_read_extended->cr);
            break;
    }
    ex10_printf(", Protection: %d", tag_read_extended->protection);
    ex10_printf(", ID: %d", tag_read_extended->id);
}

static void print_event_tag_read_raw_rssi(struct EventFifoPacket const* packet)
{
    if (packet->packet_type == TagReadExtended)
    {
        print_event_tag_read_extended(packet);
    }
    else
    {
        get_ex10_event_fifo_printer()->print_event_tag_read(packet);
    }

    ex10_printf(", Raw RSSI: %d\n", packet->static_data->tag_read.rssi);
}

static void print_packets(struct EventFifoPacket const* packet)
{
    if (packet->packet_type == TagReadExtended)
    {
        print_event_tag_read_raw_rssi(packet);
    }
    else
    {
        get_ex10_event_fifo_printer()->print_packets(packet);
    }
}

static void print_event_tag_read_compensated_rssi(
    struct EventFifoPacket const* packet,
    enum RfModes                  rf_mode,
    uint8_t                       antenna,
    enum RfFilter                 rf_filter,
    uint16_t                      adc_temperature)
{
    if (packet->packet_type == TagReadExtended)
    {
        print_event_tag_read_extended(packet);
    }
    else
    {
        get_ex10_event_fifo_printer()->print_event_tag_read(packet);
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

    ex10_printf(", RSSI (cdbm): %d\n", compensated_rssi_cdbm);
}

static void dummy_print_event_tag_read(struct EventFifoPacket const* packet)
{
    (void)packet;
}

static void dummy_print_tag_read_data(struct TagReadData const* tag_read_data)
{
    (void)tag_read_data;
}

static struct Ex10EventFifoPrinter ex10_event_fifo_printer = {
    .print_packets        = print_packets,
    .print_event_tag_read = dummy_print_event_tag_read,
    .print_event_tag_read_compensated_rssi =
        print_event_tag_read_compensated_rssi,
    .print_tag_read_data = dummy_print_tag_read_data,
};

struct Ex10EventFifoPrinter const* get_ex10_event_fifo_printer_gen2x(void)
{
    struct Ex10EventFifoPrinter const* printer = get_ex10_event_fifo_printer();
    struct Ex10EventFifoPrinter*       printer_gen2x = &ex10_event_fifo_printer;

    printer_gen2x->print_event_tag_read = printer->print_event_tag_read;
    printer_gen2x->print_tag_read_data  = printer->print_tag_read_data;

    return &ex10_event_fifo_printer;
}
