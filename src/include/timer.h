#ifndef TIMER_H
#define TIMER_H

void timer_init(int hz);
void timer_handler();
void draw_clock();
unsigned char read_cmos(unsigned char reg);

#endif