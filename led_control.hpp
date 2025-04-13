#ifndef LED_CONTROL_HPP_
#define LED_CONTROL_HPP_

#include "main.h"

namespace stmlib_v1{

class led_control{
private:
    GPIO_TypeDef *gpio_port;
    const uint16_t gpio_pin;

public:
    led_control(GPIO_TypeDef *_gpio_port, const uint16_t _gpio_pin)
        : gpio_port(_gpio_port), gpio_pin(_gpio_pin){
    }

    void set_state(bool state);
    void change_state();

};

void led_control::set_state(bool state){
    if (state){
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
    }
    else{
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);
    }
}

void led_control::change_state(){
    HAL_GPIO_TogglePin(gpio_port, gpio_pin);
}


}

#endif /* LED_CONTROL_HPP_ */