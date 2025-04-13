#ifndef TIMER_INTERRUPTION_HPP_
#define TIMER_INTERRUPTION_HPP_

#include "main.h"
#include <functional>


namespace stmlib_v1{

class HardTimer{

private:
    TIM_HandleTypeDef* tim;
    std::function<void(void)> callback;
    

public:
    HardTimer(TIM_HandleTypeDef* _tim) : tim(_tim) {};

    TIM_HandleTypeDef* get_handle(void){
        return tim;
    }

    void start(void){
        HAL_TIM_Base_Start_IT(tim);
    }

    void stop(void){
        HAL_TIM_Base_Stop_IT(tim);
    }

    void set_callback(std::function<void(void)> _callback){
        callback = _callback;
    }

    void handle_callback(void){
        if(callback){
            callback();
        }
    }
    
};



}

#endif /* TIMER_INTERRUPTION_HPP_ */