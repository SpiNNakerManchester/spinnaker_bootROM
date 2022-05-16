#ifndef PHY_H
#define PHY_H

unsigned short int phy_shift_in(void);
void phy_shift_out(unsigned int command, unsigned int length);
unsigned short int phy_read(unsigned char phy_addr);
void phy_write(unsigned char phy_addr, unsigned short int value);
int test_phy(void);
void print_phy_register_table(void);

#endif /*PHY_H*/
