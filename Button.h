#pragma once
#include <Arduino.h>

class Button
{
private:
    uint32_t pin;
    uint32_t debounce_period;
    void (*onClick)() = NULL;
    bool last_btn_state = false;
    uint32_t state_change_timestamp = 0;
    bool change_occured = false;

public:
    Button(uint32_t pin_, uint32_t debounce_period_, void (*onClick_)());
    void run();
};
