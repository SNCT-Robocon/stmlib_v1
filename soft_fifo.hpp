#ifndef SOFT_FIFO_HPP_
#define SOFT_FIFO_HPP_

#include "main.h"

/*
通信系ペリフェラルの受信割り込みや送信完了割り込み時に使用する
もしスレッドがわかれないような使い方をするなら
 __enbale_irqと、__disable_irqはないほうがいい
*/

namespace stmlib_v1{

template<typename T, size_t buf_size_bit>
class SoftFifo{
private:
    const size_t BUF_SIZE = 1 << buf_size_bit;
    const size_t BUF_MASK = BUF_SIZE - 1;
    T buffer[1 << buf_size_bit];
    size_t data_head = 0;
    size_t data_tail = 0;
    size_t total_data = 0;

public:
    bool input(const T &input_data){
        __disable_irq();
        buffer[data_head] = input_data;
        data_head = (data_head + 1) & BUF_MASK;
        total_data ++;
        if(total_data > BUF_SIZE){
            total_data = BUF_SIZE;
            data_tail = data_head;
            __enable_irq();
            return false;
        }else{
            __enable_irq();
            return true;
        }
    }

    bool output(T &output_data){
        __disable_irq();
        if(total_data > 0){
            output_data = buffer[data_tail];
            data_tail = (data_tail + 1) & BUF_MASK;
            total_data --;
            __enable_irq();
            return true;
        }else{
            __enable_irq();
            return false;
        }
    }

    size_t get_free_level(){
        return BUF_SIZE - total_data;
    }

    size_t get_busy_level(){
        return total_data;
    }

    void reset(){
        data_head = 0;
        data_tail = 0;
        total_data = 0;
    }

};

}


#endif /* SOFT_FIFO_HPP_ */