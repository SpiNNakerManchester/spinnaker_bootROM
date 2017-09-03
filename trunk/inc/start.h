#ifndef START_H
#define START_H

void reset(void);
void boot_fail(unsigned int reasoncode);
void boot_introm(void);
void ivb_checker(void);
void dputc(unsigned char toprint);

#endif
