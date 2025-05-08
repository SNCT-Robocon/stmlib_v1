#ifndef AS5600_HPP_
#define AS5600_HPP_

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


}

#endif /* AS5600_HPP_ */