#include "Ramp.h"

Ramp::Ramp(uint32_t IR_enterence_pin_, uint32_t IR_exit_pin_, uint32_t Motor_pin_, void (*car_enterd_)(), void (*car_passed_)())
{
    IR_enterence_pin = IR_enterence_pin_;
    IR_exit_pin = IR_exit_pin_;
    Motor_pin = Motor_pin_;

    car_enterd = car_enterd_;
    car_passed = car_passed_;

    pinMode(IR_enterence_pin, INPUT);
    pinMode(IR_exit_pin, INPUT);

    myServo.attach(Motor_pin);
}

Ramp::Ramp(int Motor_pin_)
{
}

bool Ramp::get_car_on_rump()
{
    return digitalRead(IR_enterence_pin);
}

void Ramp::run()
{

    if (!closed_confirmed)
    {
        close_ramp();
        closed_confirmed = true;
    }

    bool IR_enterence_state = !digitalRead(IR_enterence_pin);
    bool IR_exit_state = digitalRead(IR_exit_pin);

    if (!pre_IR_enterence_state && IR_enterence_state && !change_enterence_occured)
    {
        change_enterence_occured = true;
        state_enterence_change_timestamp = debounce_period;
    }

    if (change_enterence_occured)
    {
        if (state_enterence_change_timestamp > 0)
            state_enterence_change_timestamp--;
        if (state_enterence_change_timestamp == 0)
        {
            change_enterence_occured = false;
            if (IR_enterence_state == true)
            {
                if (car_enterd != NULL)
                {
                    car_enterd();
                }
            }
        }
    }

    if (!pre_IR_ext_state && IR_exit_state && !change_exit_occured)
    {
        change_exit_occured = true;
        state_exit_change_timestamp = debounce_period;
    }

    if (change_exit_occured)
    {
        if (state_exit_change_timestamp > 0)
            state_exit_change_timestamp--;
        if (state_exit_change_timestamp == 0)
        {
            change_exit_occured = false;
            if (IR_exit_state == true)
            {
                if (car_passed != NULL)
                {
                    car_passed();
                }
            }
        }
    }
    pre_IR_ext_state = IR_exit_state;

    pre_IR_enterence_state = IR_enterence_state;
}

void Ramp::open_ramp()
{
    rampState = stanjeRampe::open_ramp_t;
    if (ramp_servo != NULL)
    {
        ramp_servo->write(open_ramp_pos);
    }
}

void Ramp::close_ramp()
{
    rampState = stanjeRampe::close_ramp_t;
    if (ramp_servo != NULL)
    {
        ramp_servo->write(close_ramp_pos);
    }
}

stanjeRampe Ramp::get_ramp_state()
{
    return rampState;
}
