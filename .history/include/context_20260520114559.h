#ifndef CONTEXT_H
#define CONTEXT_H

#include <I2C_LCD.h>


/**
 * Alarm Clock context.
 *
 * This struct is used to store the state of the alarm clock, such as the hardware components, which should
 * Feel free to extend the context with additional members, if you decide to implement additional features.
 */
struct context {
    I2C_LCD *lcd;
};

#endif // CONTEXT_H
