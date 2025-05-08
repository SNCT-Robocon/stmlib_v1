#ifndef AMT102V_HPP_
#define AMT102V_HPP_

#include "main.h"

namespace stmlib_v1{

class amt102
{
private:
    TIM_HandleTypeDef* tim;
    bool direction;

    int pulse_count = 0;
    int16_t counter_value[2] = {0, 0};

public:
    amt102(TIM_HandleTypeDef* _tim, bool _dirction) : tim(_tim), direction(_dirction){};

    void start(void){
        HAL_TIM_Encoder_Start(tim, TIM_CHANNEL_ALL);
    }

    int get_pulse_count(void){

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




#endif /* AMT102V_HPP_ */