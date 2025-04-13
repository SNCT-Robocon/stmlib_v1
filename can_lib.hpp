#ifndef CAN_LIB_HPP_
#define CAN_LIB_HPP_

#include "main.h"
#include "soft_fifo.hpp"

namespace stmlib_v1{

struct CanPacket{
    uint32_t packet_id;
    bool is_ext_id = false;
    bool is_remote_frame = false;
    size_t dlc;
    uint8_t main_data[8];
};

class can_comm_it{

private:
    CAN_HandleTypeDef *can;
    SoftFifo<CanPacket, 8> tx_soft_fifo;
    SoftFifo<CanPacket, 8> rx_soft_fifo;

public:
    can_comm_it(CAN_HandleTypeDef* _can) : can(_can){
        // nothing to do
    }

    CAN_HandleTypeDef* get_handle(void){
        return can;
    }

    void start();
    void filter_set_free(uint32_t bank_number, bool is_ext_id);

    size_t get_tx_busy_level();
    bool add_tx_fifo(CanPacket &packet);
    void tx_trigger();

    size_t get_rx_busy_level();
    void rx_interruption();
    bool rx(CanPacket &packet);

};

void can_comm_it::start(){
    HAL_CAN_Start(can);
    HAL_CAN_ActivateNotification(can, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_ActivateNotification(can, CAN_TX_MAILBOX0);
    HAL_CAN_ActivateNotification(can, CAN_TX_MAILBOX1);
    HAL_CAN_ActivateNotification(can, CAN_TX_MAILBOX2);
}

void can_comm_it::filter_set_free(uint32_t bank_number, bool is_ext_id){
    CAN_FilterTypeDef filter;

    filter.FilterBank = bank_number;
    filter.FilterIdHigh = 0;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow = 0;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.SlaveStartFilterBank = 14;
    filter.FilterActivation = ENABLE;
    
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    HAL_CAN_ConfigFilter(can, &filter);
}

size_t can_comm_it::get_tx_busy_level(){
    return tx_soft_fifo.get_busy_level();
}

bool can_comm_it::add_tx_fifo(CanPacket &packet){
    bool is_ok;
    is_ok = tx_soft_fifo.input(packet);
    tx_trigger();
    return is_ok;
}

void can_comm_it::tx_trigger(){
    if(get_tx_busy_level() == 0){
        // nothing to do
    }
    else{
        CanPacket packet;
        CAN_TxHeaderTypeDef tx_header;
        uint32_t mailbox_number;

        tx_soft_fifo.output(packet);

        tx_header.IDE = packet.is_ext_id ? CAN_ID_EXT : CAN_ID_STD;
        tx_header.ExtId = packet.is_ext_id ? packet.packet_id : 0;
        tx_header.StdId = packet.is_ext_id ? 0 : packet.packet_id;
        tx_header.RTR = packet.is_remote_frame ? CAN_RTR_REMOTE : CAN_RTR_DATA;
        tx_header.DLC = packet.dlc;
        tx_header.TransmitGlobalTime = DISABLE;

        HAL_CAN_AddTxMessage(can, &tx_header, packet.main_data, &mailbox_number);
    }
}

size_t can_comm_it::get_rx_busy_level(){
    return rx_soft_fifo.get_busy_level();
}

void can_comm_it::rx_interruption(){
    CAN_RxHeaderTypeDef rx_header;
    CanPacket packet;

    HAL_CAN_GetRxMessage(can, CAN_RX_FIFO0, &rx_header, packet.main_data);

    packet.dlc = rx_header.DLC;
    packet.is_ext_id = rx_header.IDE == CAN_ID_EXT ? true : false;
    packet.is_remote_frame = rx_header.RTR == CAN_RTR_REMOTE ? true : false;
    packet.packet_id = rx_header.IDE == CAN_ID_EXT ? rx_header.ExtId : rx_header.StdId;

    rx_soft_fifo.input(packet);
}

bool can_comm_it::rx(CanPacket &packet){
    return rx_soft_fifo.output(packet);
}

}

#endif /* CAN_LIB_HPP_ */