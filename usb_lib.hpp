#ifndef USB_LIB_HPP_
#define USB_LIB_HPP_

#include "main.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "soft_fifo.hpp"
//#include <deque>


namespace stmlib_v1{


class usb_comm_it{
private:
    SoftFifo<uint8_t, 8> rx_soft_fifo;

public:
    void tx(uint8_t* tx_data, uint16_t data_len){
        CDC_Transmit_FS(tx_data, data_len);
    }
    
    size_t get_rx_busy_level(){
        return rx_soft_fifo.get_busy_level();
    }

    void add_rx_data(const uint8_t* rx_data, uint32_t len){
        for(uint32_t i = 0; i < len; i ++){
            rx_soft_fifo.input(rx_data[i]);
        }
    }

    void get_rx_data(uint8_t &rx_buf){
        rx_soft_fifo.output(rx_buf);
    }

};

}


#endif /* USB_LIB_HPP_ */