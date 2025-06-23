#ifndef UART_LIB_HPP_
#define UART_LIB_HPP_

#include "main.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include <cmath>
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
        if (tx_soft_fifo.get_busy_level() != 0 && uart -> gState == HAL_UART_STATE_READY){
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


class r3008sb{
private:
    UART_HandleTypeDef* uart;
    uint8_t dma_main_buffer[25] = {};
    bool is_fail_safe = true;
    bool is_alive = false;

public:
    int channel_data[16] = {};

    r3008sb(UART_HandleTypeDef* _uart) : uart(_uart){
        // nothing to do
    }

    UART_HandleTypeDef* get_handle(){
        return uart;
    }
    
    
    void init(){

        // __HAL_DMA_DISABLE_IT(uart -> hdmarx, DMA_IT_HT);
        // __HAL_DMA_DISABLE_IT(uart -> hdmarx, DMA_IT_TC);
        
        HAL_UARTEx_ReceiveToIdle_DMA(uart, dma_main_buffer, 25);

    }

    void update(){

        // if(__HAL_UART_GET_FLAG(uart, UART_FLAG_IDLE)){
            
        // }
        
        uint8_t dma_main_buffer_copy [25];
        bool dma_main_buffer_bit[25 * 8];

        size_t start_byte_number = (24 - __HAL_DMA_GET_COUNTER(uart -> hdmarx) + 1 + 25) % 25;

        for (int i = 0; i < 25; i++){
            dma_main_buffer_copy[i] = dma_main_buffer[(start_byte_number + i) % 25];
        }

        for (int i = 0; i < 25 - 1; i++)
        {
            dma_main_buffer_bit [i * 8 + 0] = dma_main_buffer_copy [i + 1] & 0b00000001;
            dma_main_buffer_bit [i * 8 + 1] = dma_main_buffer_copy [i + 1] & 0b00000010;
            dma_main_buffer_bit [i * 8 + 2] = dma_main_buffer_copy [i + 1] & 0b00000100;
            dma_main_buffer_bit [i * 8 + 3] = dma_main_buffer_copy [i + 1] & 0b00001000;
            dma_main_buffer_bit [i * 8 + 4] = dma_main_buffer_copy [i + 1] & 0b00010000;
            dma_main_buffer_bit [i * 8 + 5] = dma_main_buffer_copy [i + 1] & 0b00100000;
            dma_main_buffer_bit [i * 8 + 6] = dma_main_buffer_copy [i + 1] & 0b01000000;
            dma_main_buffer_bit [i * 8 + 7] = dma_main_buffer_copy [i + 1] & 0b10000000;
        }

        for (int i = 0; i < 16; i++)
        {
            channel_data [i] = 0;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 0] << 0;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 1] << 1;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 2] << 2;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 3] << 3;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 4] << 4;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 5] << 5;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 6] << 6;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 7] << 7;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 8] << 8;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 9] << 9;
            channel_data [i] = channel_data [i] | dma_main_buffer_bit [i * 11 + 10] << 10;
        }

        if(dma_main_buffer_copy[0] == 0x0f){
            is_alive = true;
        }
        else{
            is_alive = false;
        }

        if(dma_main_buffer_copy[23] & (1 << 3)){
            is_fail_safe = true;
        }
        else{
            is_fail_safe = false;
        }

    }

    float get_stick_data(size_t axis){

        /*
        channel 4 1022 1680
        channel 3 1038 1680
        channel 1 1019 1680
        channel 2 1009 368
        */
        float stick_data = 0.0f;
        
        if(axis == 0){
            stick_data = (static_cast<float>(channel_data[3]) - 1022.0) / (1680.0 - 1022.0);
        }
        else if(axis == 1){
            stick_data = (static_cast<float>(channel_data[2]) - 1038.0) / (1680.0 - 1038.0);
        }
        else if(axis == 2){
            stick_data = (static_cast<float>(channel_data[0]) - 1019.0) / (1680.0 - 1019.0);
        }
        else if(axis == 3){
            stick_data = (static_cast<float>(channel_data[1]) - 1009.0) / (368.0 - 1009.0);
        }

        if(stick_data < - 0.95){
            stick_data = - 1.0;
        }
        else if(0.95 < stick_data){
            stick_data = 1.0;
        }
        else if(fabsf(stick_data) < 0.09){
            stick_data = 0.0;
        }

        return stick_data;
    }

    size_t get_switch_data(size_t channel){

        if (channel_data[channel] < 500){
            return 2;
        }
        else if (channel_data[channel] > 1500){
            return 0;
        }
        else{
            return 1;
        }

    }

    bool get_is_fail_safe(){
        return is_fail_safe;
    }

    bool get_is_alive(){
        return is_alive;
    }

    

};

}

#endif /* UART_LIB_HPP_ */