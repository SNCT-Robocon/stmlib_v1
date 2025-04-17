#ifndef ENCODER_LIB_HPP_
#define ENCODER_LIB_HPP_

#include "main.h"

namespace stmlib_v1{

class as5600{
private:
    I2C_HandleTypeDef* i2c;
    bool dir;
    uint8_t raw_angle_u8[2];
    uint16_t raw_angle_u16[2];
    int16_t displacement_1kHz;
    uint32_t rescaled_angle;

public:
    as5600(I2C_HandleTypeDef* _i2c, bool _dir) : i2c(_i2c), dir(_dir){
        raw_angle_u8[0] = 0;
        raw_angle_u8[1] = 0;
        raw_angle_u16[0] = 0;
        raw_angle_u16[1] = 0;
        rescaled_angle = 0;
    }

    I2C_HandleTypeDef* get_handle(){
        return i2c;
    }

    void angle_update_trigger(){
        HAL_I2C_Mem_Read_IT(i2c, 0x36 << 1, 0x0c, I2C_MEMADD_SIZE_8BIT, raw_angle_u8, 2);
    }

    void angle_update(){

        raw_angle_u16[0] = raw_angle_u8[0] << 8 | raw_angle_u8[1];
        int16_t displacement = raw_angle_u16[0] - raw_angle_u16[1];
        raw_angle_u16[1] = raw_angle_u16[0];

        if(displacement < - 2000){
            displacement += 4096;
        }
        else if(2000 < displacement){
            displacement -= 4096;
        }

        displacement *= dir ? 1 : - 1;

        rescaled_angle += displacement;

        displacement_1kHz = displacement;

    }

    void reset_angle(){
        rescaled_angle = 0;
    }

    uint16_t get_raw_angle(){
        return raw_angle_u16[0];
    }

    int16_t get_displacement_1kHz(){
        return displacement_1kHz;
    }

    uint32_t get_rescaled_angle(){
        return rescaled_angle;
    }

};


class amt102
{
private:
    TIM_HandleTypeDef* tim;
    bool direction;

    long long pulse_count = 0;
    int16_t counter_value[2] = {0, 0};

public:
    amt102(TIM_HandleTypeDef* _tim, bool _dirction) : tim(_tim), direction(_dirction){};

    void start(void){
        HAL_TIM_Encoder_Start(tim, TIM_CHANNEL_ALL);
    }

    long long get_pulse_count(void){

        counter_value[0] = __HAL_TIM_GET_COUNTER (tim);
        int counter_displacement = counter_value[0] - counter_value[1];
        counter_value[1] = counter_value[0];

        if (counter_displacement < - 40000)
        {
            counter_displacement += 65536;
        }
        else if (40000 < counter_displacement)
        {
            counter_displacement -= 65536;
        }
        pulse_count += counter_displacement * ((direction) ? 1 : - 1);

        return pulse_count;
    }

    void set_pulse_count(int16_t set_count){
        pulse_count = set_count;
    }

};


}




#endif /* ENCODER_LIB_HPP_ */