#pragma once

#include <Arduino.h>
#include "ESP32Servo.h"

typedef enum
{
    close_ramp_t = 0,
    open_ramp_t = 1
} stanjeRampe;

class Ramp
{

private:
    Servo myServo;

    Servo *ramp_servo = &myServo; // create servo object to control a servo
    uint32_t IR_enterence_pin;
    uint32_t IR_exit_pin;
    uint32_t Motor_pin;
    bool pre_IR_enterence_state = false;
    bool change_exit_occured = false;
    bool change_enterence_occured = false;
    bool pre_IR_ent_state = false;
    bool pre_IR_ext_state = true;

    void (*car_enterd)() = NULL;
    void (*car_passed)() = NULL;
    uint32_t car_counter = 0;
    stanjeRampe rampState = stanjeRampe::close_ramp_t;
    uint32_t state_exit_change_timestamp;
    uint32_t state_enterence_change_timestamp;
    uint32_t debounce_period = 5;

    uint8_t open_ramp_pos = 90;
    uint8_t close_ramp_pos = 0;
    bool closed_confirmed = false;

public:
    Ramp(uint32_t IR_enterence_pin_, uint32_t IR_exit_pin_, uint32_t Motor_pin_, void (*car_enterd_)(), void (*car_passed_)());
    Ramp(int Motor_pin_);
    stanjeRampe get_ramp_state();
    bool get_car_on_rump();
    void run();
    void open_ramp();
    void close_ramp();
};
