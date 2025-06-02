#ifndef _LCD_H
#define _LCD_H

void lcd_init(void);

void lcd_clr(void);

void lcd_pos(unsigned char r, unsigned char c);

void lcd_put(char c);

void lcd_puts1(const char *s);

void lcd_puts2(const char *s);

void write(unsigned char, unsigned char);

static void output(unsigned char, unsigned char);

static unsigned char input(unsigned char);

static unsigned char get_data(void);

void set_data(unsigned char);

#endif /* _LCD_H */