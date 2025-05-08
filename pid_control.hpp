#ifndef PID_CONTROL_HPP_
#define PID_CONTROL_HPP_

#include "main.h"
#include <algorithm>
#include <cmath>

namespace stmlib_v1{

class pid_control{

private:
    float loop_hz;
    float gain_p, gain_i, gain_d;
    float lim_min, lim_max;
    float old_diff;
    float diff_integ;
    float anti_windup_min;
    float anti_windup_max;
    float i_reset_triger;

public:

    pid_control(
        float _loop_hz,
        float _gain_p,
        float _gain_i,
        float _gain_d,
        float _lim_min,
        float _lim_max,
        float _anti_windup_min,
        float _anti_windup_max,
        float _i_reset_triger
    ) : loop_hz(_loop_hz),
        gain_p(_gain_p),
        gain_i(_gain_i),
        gain_d(_gain_d),
        lim_min(_lim_min),
        lim_max(_lim_max),
        old_diff(0.0f),
        diff_integ(0.0f),
        anti_windup_min(_anti_windup_min),
        anti_windup_max(_anti_windup_max),
        i_reset_triger(_i_reset_triger)
    {
        // nothing to do
    }

    float calculate_output(float diff){
        float output = 0.0;

        diff_integ += (diff + old_diff) / 2.0000 / this -> loop_hz;
        diff_integ = std::clamp(diff_integ, anti_windup_min, anti_windup_max);

        if(fabsf(diff) < i_reset_triger){
            diff_integ = 0;
        }

        output += diff * gain_p;
        output += (diff - old_diff) * this -> loop_hz * gain_d;
        output += diff_integ * gain_i;

        old_diff = diff;

        output = std::clamp(output, lim_min, lim_max);

        return output;
    }

    void set_gain(float _gain_p, float _gain_i, float _gain_d){
        gain_p = _gain_p;
        gain_i = _gain_i;
        gain_d = _gain_d;
    }

    void set_lim(float _lim_min, float _lim_max){
        lim_min = _lim_min;
        lim_max = _lim_max;
    }

    void set_anti_windup(float _anti_windup_min, float _anti_windup_max){
        anti_windup_min = _anti_windup_min;
        anti_windup_max = _anti_windup_max;
    }

    void reset(void){
        old_diff = 0.0;
        diff_integ = 0.0;
    }

};

}

#endif /* PID_CONTROL_HPP_ */