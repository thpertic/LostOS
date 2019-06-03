#ifndef TIME_H
#define TIME_H

#include <system.h>

void clock_init(uint32_t frequency);
void tickHandler(regs_t *r);

#endif