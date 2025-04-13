#ifndef FDCAN_LIB_HPP_
#define FDCAN_LIB_HPP_

#include "main.h"
#include "soft_fifo.hpp"

namespace stmlib_v1{

struct CanFdPacket{
    uint32_t packet_id;
    bool is_classic = false;
    bool is_ext_id = false;
    bool is_remote_frame = false;
    size_t dlc;
    uint8_t main_data[32];
};

class canfd_comm_it{

private:
    FDCAN_HandleTypeDef *fdcan;
    SoftFifo<CanFdPacket, 8> tx_soft_fifo;
    SoftFifo<CanFdPacket, 8> rx_soft_fifo;

public:
    canfd_comm_it(FDCAN_HandleTypeDef* _fdcan) : fdcan(_fdcan){
        // nothing to do
    }

    FDCAN_HandleTypeDef* get_handle(){
        return fdcan;
    }

    void start();
    void filter_set_free(uint32_t bank_number, bool is_ext_id);

    size_t get_tx_busy_level();
    bool add_tx_fifo(CanFdPacket &packet);
    void tx_trigger();

    size_t get_rx_busy_level();
    void rx_interruption();
    bool rx(CanFdPacket &packet);

};

void canfd_comm_it::start(){

    HAL_FDCAN_Start(fdcan);
    HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U);
    HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0);
    HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER1);
    HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER2);

}

void canfd_comm_it::filter_set_free(uint32_t bank_number, bool is_ext_id){
    FDCAN_FilterTypeDef filter;

    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = 0x000;
    filter.FilterID2 = 0x000;
    filter.FilterIndex = bank_number;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.IdType = is_ext_id ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;

    HAL_FDCAN_ConfigFilter(fdcan, &filter);
}

size_t canfd_comm_it::get_tx_busy_level(){
    return tx_soft_fifo.get_busy_level();
}

bool canfd_comm_it::add_tx_fifo(CanFdPacket &packet){
    bool is_ok;
    is_ok = tx_soft_fifo.input(packet);
    tx_trigger();
    return is_ok;
}

void canfd_comm_it::tx_trigger(){
    if(get_tx_busy_level() == 0){
        // nothing to do
    }
    else{
        CanFdPacket packet;
        FDCAN_TxHeaderTypeDef tx_header;

        tx_soft_fifo.output(packet);

        tx_header.BitRateSwitch = packet.is_classic ? FDCAN_BRS_OFF : FDCAN_BRS_ON;
        tx_header.DataLength = packet.dlc;
        tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
        tx_header.FDFormat = packet.is_classic ? FDCAN_CLASSIC_CAN : FDCAN_FD_CAN;
        tx_header.Identifier = packet.packet_id;
        tx_header.IdType = packet.is_ext_id ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
        tx_header.MessageMarker = 0U;
        tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        tx_header.TxFrameType = packet.is_remote_frame ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;

        HAL_FDCAN_AddMessageToTxFifoQ(fdcan, &tx_header, packet.main_data);
    }
}

size_t canfd_comm_it::get_rx_busy_level(){
    return rx_soft_fifo.get_busy_level();
}

void canfd_comm_it::rx_interruption(){
    FDCAN_RxHeaderTypeDef rx_header;
    CanFdPacket packet;

    HAL_FDCAN_GetRxMessage(fdcan, FDCAN_RX_FIFO0, &rx_header, packet.main_data);

    packet.dlc = rx_header.DataLength;
    packet.is_classic = rx_header.FDFormat == FDCAN_CLASSIC_CAN ? true : false;
    packet.is_ext_id = rx_header.IdType == FDCAN_EXTENDED_ID ? true : false;
    packet.is_remote_frame = rx_header.RxFrameType == FDCAN_REMOTE_FRAME ? true : false;
    packet.packet_id = rx_header.Identifier;

    rx_soft_fifo.input(packet);
}

bool canfd_comm_it::rx(CanFdPacket &packet){
    return rx_soft_fifo.output(packet);
}


}

#endif /* FDCAN_LIB_HPP_ */