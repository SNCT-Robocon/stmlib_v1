#ifndef PWM_CONTROL_HPP_
#define PWM_CONTROL_HPP_

#include "main.h"

namespace stmlib_v1{

class pwm_control{
private:
    TIM_HandleTypeDef *tim;
    uint32_t channel;
    uint32_t arr;

public:
    pwm_control(TIM_HandleTypeDef *_tim, uint32_t _channel) : tim(_tim), channel(_channel){
        // nothing to do
    }

    void start(){
        HAL_TIM_PWM_Start(tim, channel);
        __HAL_TIM_SET_COMPARE(tim, channel, 0U);
        arr = __HAL_TIM_GET_AUTORELOAD(tim);
    }

    void set_duty(float duty){
        if(duty < 0.0f) duty = 0.0f;
        if(1.0f < duty) duty = 1.0f;
        uint32_t ccr = static_cast<uint32_t>(arr * duty);
        __HAL_TIM_SET_COMPARE(tim, channel, ccr);
    }

};


}



#endif /* PWM_CONTROL_HPP_ */