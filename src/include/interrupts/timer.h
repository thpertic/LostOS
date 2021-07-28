#ifndef TIME_H
#define TIME_H

#include <system.h>

void init_clock(uint32_t frequency);
void tickHandler(regs_t *r);

#endif