#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "avr.h"
#include "lcd.h"
#include <time.h>

// keypad
int get_key();
int is_pressed(int, int);
int keyValue;

// Game Functions
void welcomePopup(void);
void playGame(void);
void init_game(void);
void spawnBlock(void);
int gameOver(void);
void lcd_define_char(unsigned char char_num, const unsigned char *pattern);
void sprite_init(void);
int wait_100k_for_one(void);
unsigned int get_adc_noise_seed(void);

// Game Global Variables
short row;              // which row on the LCD 0 or 1
int blockCounter = 16;  // Number of LCD Columns the block must iterate through
int isJumping;          // check if dino is jumping
int jumpCounter;        // number of times the dino jumps
int isDucking;          // check if dino is Ducking
int isGameOver;
int score = 0;          // game score
char scoreBuf[16];
double speed;           // speed the blocks travel
double decayRate;
int walk_animation = 0; 
int highScore = 0;

int main(void)
{
    DDRC = 0; // set keypad as input
    DDRD = 0xFF; // set LCD screen as output
    lcd_init(); // initialize screen
    lcd_clr();
    sprite_init();

    playGame();
}

int get_key()
{
    int i, j;
    for (i = 3; i >= 0; --i)
        for (j = 2; j >= 0; --j)
            if (is_pressed(i, j))
                return (i * 3) + j + 1;
    return 0;
}

int is_pressed(int r, int c)
{
    /*
    row 0 = PC0
    row 1 = PC1
    row 2 = PC2
    row 3 = PC3
    col 0 = PC4
    col 1 = PC5
    col 2 = PC6
    */

    //----all 8 GPIOs to N / C----
    DDRC = 0;
    PORTC = 0;

    //----set r to "0"----
    SET_BIT(DDRC, r);
    CLR_BIT(PORTC, r); // might need PINC idk

    //----set c to "w1"----
    CLR_BIT(DDRC, c + 4);
    SET_BIT(PORTC, c + 4); // might need PINC idk
    mwait(100);

    // check if w1 flipped to a 0
    if (GET_BIT(PINC, c + 4)) // if true button not pressed (still 1), return False (func is boolean)        
        return 0;

    else // if false button was pressed, return True (func is boolean)
        return 1;
}

void playGame(void) 
{        
    welcomePopup();
    init_game();
    do 
    {
        score++;
        walk_animation = !walk_animation;
        keyValue = get_key();
        
        // check if ducking; if not reset var
        if (keyValue != 8)
            isDucking = 0;

        // print normal dino sprite if not jumping ot 
        if (!isJumping && !isDucking)
        {
            lcd_pos(0,4);
            lcd_put(' ');

            lcd_pos(1,4);
            lcd_put(' ');

            lcd_pos(0,3);
            // lcd_put(0xFF);
            lcd_put(0x00); // top half of custom sprite
            
            lcd_pos(1,3);
            // lcd_put(0xFF);
            if (walk_animation)
                lcd_put(0x01); 
            else
                lcd_put(0x02); 
        }

        if (isJumping)
        {
            ++jumpCounter;
            lcd_pos(0,3);
            if (walk_animation)
                lcd_put(0x04); 
            else
                lcd_put(0x05); 
        }

        if (jumpCounter > 7)
        {
            jumpCounter = 0;
            isJumping = 0;
        }

        if (keyValue >= 1 && keyValue <= 12)
        {
            // Press key 5 to Jump
            if ( (keyValue == 5) && !isDucking)
            {				
                // clear old sprite	
                lcd_pos(0,3);
                lcd_put(' ');
                lcd_pos(1,3);
                lcd_put(' ');

                // print jumping sprite
                lcd_pos(0,4);
                lcd_put(0x03);

                lcd_pos(0,3);
                if (walk_animation)
                    lcd_put(0x04); 
                else
                    lcd_put(0x05); 
                isJumping = 1;
            }
            
            // Press key 8 to Duck
            else if ( (keyValue == 8) && !isJumping)
            {			
                // clear old sprite	
                lcd_pos(0,3);
                lcd_put(' ');
                lcd_pos(0,4);
                lcd_put(' ');
                lcd_pos(1,3);
                lcd_put(' ');
                
                // print ducking sprite
                lcd_pos(1,4);
                lcd_put(0x03);

                lcd_pos(1,3);
                if (walk_animation)
                    lcd_put(0x04); 
                else
                    lcd_put(0x05); 

                isDucking = 1;
            }
        }
        spawnBlock();

        // adjust this value to control how quickly it converges
        mwait((int)speed);
        speed *= decayRate;
        if (speed < 1000) { speed = 1000; }
        
        // Press #1 on the keypad to restart
        while (isGameOver)
        {
            if (gameOver())
            {
                init_game();
                mwait(40000);
                mwait(40000);
                break;
            }
        }
    } while (keyValue == 1 || !isGameOver);
}

void welcomePopup(void)
{
    lcd_clr();
    lcd_puts2(" Max and Oscar");
    
    lcd_pos(1,0);
    lcd_puts2("    PRESENTS");
    mwait(40000);
    mwait(40000);
    mwait(40000);
    mwait(40000);

    lcd_clr();
    lcd_pos(1,0);
    lcd_puts2("  LCDinoBytes");
    mwait(40000);
    mwait(40000);
    mwait(40000);
    mwait(40000);
}

void init_game(void)
{
    srand(get_adc_noise_seed());
    lcd_clr();
    isGameOver = 0;
    blockCounter = 16;
    isJumping = 0;
    isDucking = 0;
    score = 0;
    row = rand() % 2;
    jumpCounter = 0;
    speed = 5000;
    decayRate = 0.997;
}

void spawnBlock(void)
{
    --blockCounter;
    if (blockCounter != 15)
    {
        lcd_pos(row, blockCounter+1);
        lcd_put(' ');
    }

    lcd_pos(row, blockCounter);
    if(row)
        lcd_put(0x06);
    
    else
    lcd_put(0x07); 
    // {
    //     if (walk_animation)
    //         lcd_put(0x07); 
    //     else
    //         lcd_put(0x08); 
    // }
    

    // COLLISION NEEDS WORK
    // standing and at block 3, always a game over
    if (blockCounter == 3 && !isJumping && !isDucking)
        gameOver();

    // if jumping and at block 4, and row is 0, game over
    if (blockCounter == 4 && row == 0 && isJumping)
        gameOver();

    // if ducking and at block 4, and row is 1, game over
    if (blockCounter == 4 && row == 1 && isDucking)
        gameOver();
    
    if(blockCounter == 0)
    {
        lcd_pos(row, blockCounter);
        lcd_put(' ');
        blockCounter = 16;
        row = rand() % 2; // Generates 0 or 1
    }    
}

void lcd_define_char(unsigned char char_num, const unsigned char *pattern) 
{
    unsigned char cgram_addr;
    int i;

    // Custom characters are 0-7. Each takes 8 bytes in CGRAM.
    // CGRAM address for char_num = char_num * 8
    cgram_addr = char_num * 8;

    // Send "Set CGRAM Address" command
    // DB7=0, DB6=1, DB5-DB0 = cgram_addr
    // So, command is 0x40 | cgram_addr
    write(0x40 | cgram_addr, 0); // rs=0 for command

    // Write the 8 bytes of pattern data
    for (i = 0; i < 8; ++i)
        write(pattern[i], 1); // rs=1 for data
    
    // Optional: Set DDRAM address back to home after CGRAM programming.
    // This ensures subsequent lcd_put commands don't accidentally write to CGRAM
    // if the address counter was left pointing there.
    // write(0x02, 0); // Command: Return Home (sets DDRAM address to 0x00)
}

int gameOver(void)
{
    isGameOver = 1;
    if(score > highScore) { highScore = score; }

    lcd_clr();
    lcd_puts2("Game Over");
    
    lcd_pos(1,0);
    sprintf(scoreBuf, "Score: %i", score);
	lcd_puts2(scoreBuf);

    if (wait_100k_for_one()) { return 1; }
    lcd_pos(0,0);
    lcd_puts2("Restart: Press 1");

    lcd_pos(1,0);
    sprintf(scoreBuf, "Record: %i", highScore);
    lcd_puts2(scoreBuf);
    
    if (wait_100k_for_one()) { return 1; }
    return 0;
}

void sprite_init(void) 
{

    const unsigned char run_top[] = {
        0b00010, // Row 0
        0b00111, // Row 1
        0b00111, // Row 2
        0b00111, // Row 3
        0b01110, // Row 4
        0b01110, // Row 5
        0b11111, // Row 6
        0b11101  // Row 7
    };


    const unsigned char run_bl[] = {
        0b11100, // Row 0
        0b11110, // Row 1
        0b11110, // Row 2
        0b11110, // Row 3
        0b10010, // Row 4
        0b10011, // Row 5
        0b10000, // Row 6
        0b11000  // Row 7
    };

    const unsigned char run_br[] = {
        0b11100, // Row 0
        0b11110, // Row 1
        0b11110, // Row 2
        0b11110, // Row 3
        0b10010, // Row 4
        0b11010, // Row 5
        0b00010, // Row 6
        0b00011  // Row 7
    };

    const unsigned char duck_top[] = {
        0b00010, // Row 0
        0b00111, // Row 1
        0b11111, // Row 2
        0b11111, // Row 3
        0b11110, // Row 4
        0b00000, // Row 5
        0b00000, // Row 6
        0b00000  // Row 7 (Cursor line)
    };

    const unsigned char duck_bl[] = {
        0b00000, // Row 0
        0b00000, // Row 1
        0b01111, // Row 2
        0b11111, // Row 3
        0b11111, // Row 4
        0b10010, // Row 5
        0b10011, // Row 6
        0b11000  // Row 7
    };

    const unsigned char duck_br[] = {
        0b00000, // Row 0
        0b00000, // Row 1
        0b01111, // Row 2
        0b11111, // Row 3
        0b11111, // Row 4
        0b10010, // Row 5
        0b11010, // Row 6
        0b00011  // Row 7
    };

    const unsigned char cactus[] = {
        0b00100,
        0b00101,
        0b10101,
        0b10101,
        0b10111,
        0b11110,
        0b01110,
        0b01110
    };

    const unsigned char bird1[] = {
        0b00000,
        0b01000,
        0b11000,
        0b00110,
        0b00111,
        0b00110,
        0b00100,
        0b00100
    };

    // const unsigned char bird2[] = {
    //     0b00100,
    //     0b00100,
    //     0b01100,
    //     0b11110,
    //     0b00111,
    //     0b00110,
    //     0b00000,
    //     0b00000
    // };


    // Your DDR initializations here (DDRC, DDRD etc.)
    DDRC = 0;    // Assuming keypad input
    // DDRD for LCD data is set in lcd_init (or in set_data)
    // Control lines DDRB for RS, RW, EN are set in lcd_init

    lcd_init();
    lcd_clr();

    // --- Define the custom characters ---
    lcd_define_char(0, run_top); // Define smiley character at slot 0
    lcd_define_char(1, run_bl);
    lcd_define_char(2, run_br);    // Define bell character at slot 1

    lcd_define_char(3, duck_top);
    lcd_define_char(4, duck_bl); 
    lcd_define_char(5, duck_br); 

    lcd_define_char(6, cactus);
    lcd_define_char(7, bird1); 
    // lcd_define_char(8, bird2); 

    // You can define up to 8 custom characters (slots 0 through 7)

    // --- Display the custom characters ---
    // lcd_pos(0, 1);
    // lcd_put(0x03); // Display custom character from slot 0

    // while(1){
    //     lcd_pos(0, 0);
    //     lcd_put(0x04); // Display custom character from slot 1
    //     mwait(5000);
    //     lcd_pos(0, 0);
    //     lcd_put(0x05); // Display custom character from slot 2
    //     mwait(5000);
    // }
}


int wait_100k_for_one(void)
{
    for (int i = 0; i < 100; ++i)
    {
        mwait(10);
        keyValue = get_key();
        if (keyValue == 1)
            return 1;
    }
    return 0;
}

unsigned int get_adc_noise_seed(void) 
{
    ADMUX |= (1<<REFS0);
	SET_BIT(ADCSRA, 7); //Set ADEN
	SET_BIT(ADCSRA, 6); //Set ADSC start conversion
	while(GET_BIT(ADCSRA, 6)); //wait to finish measure
    return ADC; // Use the raw ADC value as seed
}