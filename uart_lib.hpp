#ifndef UART_LIB_HPP_
#define UART_LIB_HPP_

#include "main.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include "soft_fifo.hpp"

namespace stmlib_v1{


class stlink_debag{
private:
    UART_HandleTypeDef *huart;
    uint32_t timeout;

public:
    stlink_debag(UART_HandleTypeDef *_huart, uint32_t _timeout) : huart(_huart), timeout(_timeout){
        // nothing to do
    }

    void transmit_debug(const char *format, ...){
        char buffer[100];

        va_list args;

        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        HAL_UART_Transmit(huart, (uint8_t *)buffer, strlen(buffer), timeout);
    }

};


class uart_comm_it{
private:
    UART_HandleTypeDef *uart;
    uint8_t rx_buf;
    SoftFifo<uint8_t, 8> rx_soft_fifo;
    SoftFifo<uint8_t, 8> tx_soft_fifo;

public:
    uart_comm_it(UART_HandleTypeDef *_uart) : uart(_uart){
        //nothing to do
    }

    UART_HandleTypeDef* get_handle(){
        return uart;
    }

    void comm_start(){
        HAL_UART_Receive_IT(uart, &rx_buf, 1);
    }

    void rx_interruption(){
        rx_soft_fifo.input(rx_buf);
        HAL_UART_Receive_IT(uart, &rx_buf, 1);
    }

    size_t get_rx_busy_level(){
        return rx_soft_fifo.get_busy_level();
    }

    void get_rx_data(uint8_t &user_buf){
        rx_soft_fifo.output(user_buf);
    }

    void tx_trigger(){
        if (tx_soft_fifo.get_busy_level() != 0 && HAL_UART_GetState(uart) == HAL_UART_STATE_READY) {
            uint8_t tx_byte;
            tx_soft_fifo.output(tx_byte);
            HAL_UART_Transmit_IT(uart, &tx_byte, 1);
        }
    }

    void add_tx_data(uint8_t add_data[], size_t data_size){
        for(size_t i = 0; i < data_size; i ++){
            tx_soft_fifo.input(add_data[i]);
        }
        tx_trigger();
    }

};

}

#endif /* UART_LIB_HPP_ */