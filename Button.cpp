#include "Button.h"

Button::Button(uint32_t pin_, uint32_t debounce_period_, void (*onClick_)())
{
    pin = pin_;
    debounce_period = debounce_period_;
    onClick = onClick_;
    pinMode(pin, INPUT);
}

void Button::run()
{

    bool btn_state = digitalRead(pin);
    if (!last_btn_state && btn_state && !change_occured)
    {
        change_occured = true;
        state_change_timestamp = debounce_period;
    }

    if (change_occured)
    {
        if (state_change_timestamp > 0)
            state_change_timestamp--;
        if (state_change_timestamp == 0)
        {
            change_occured = false;
            if (btn_state == true)
            {
                if (onClick != NULL)
                {
                    onClick();
                }
            }
        }
    }
    last_btn_state = btn_state;
}
