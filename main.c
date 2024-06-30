#include <string.h>  // Include standard string manipulation library
#include <stdio.h>   // Include standard I/O library

#include "wdt/wdt.h"    // Include Watchdog Timer library
#include "pmc/pmc.h"    // Include Power Management Controller library
#include "pio/pio.h"    // Include Parallel Input/Output controller library

#include "board.h"      // Include board-specific definitions
#include "lcd-ge8.h"    // Include LCD screen handling library
#include "delay_us.h"   // Include delay functions in microseconds

// Define constants for screen and character properties
#define SCR_BCOLOR BLUE       // Screen background color
#define CHAR_CHAR 'X'         // Character to display
#define CHAR_BCOLOR RED       // Character background color
#define CHAR_FCOLOR YELLOW    // Character foreground color

#define CHAR_DELTA_X 7        // Movement step size for X-axis
#define CHAR_DELTA_Y 5        // Movement step size for Y-axis
#define CHAR_REPEAT_DELAY 10  // Delay for auto-repeat (in loop iterations)
#define CHAR_REPEAT_PERIOD 3  // Interval for auto-repeat (every N loops)

// Function to place a character on the LCD screen at the specified coordinates
void placeCharacter(int *x, int *y, char c, int charFColor, int charBColor) {
    char txt[2] = {c, '\0'};  // Create a string with the character
    LCDGotoXY(*x, *y);        // Move cursor to specified position
    LCDCharColor(charFColor, charBColor);  // Set character color
    LCDPutStr(txt);           // Display the character
}

// Function to move a character on the LCD screen
void moveCharacter(int *x, int *y, int dx, int dy, char c, int scrColor, int charFColor, int charBColor) {
    // Remove old character by overwriting with background color
    char txt[1];
    LCDGotoXY(*x, *y);        // Move cursor to old position
    sprintf(txt, " ");        // Prepare to clear the character
    LCDCharColor(charFColor, scrColor);  // Set clear color
    LCDPutStr(txt);           // Clear the character

    // Update character's position
    *x += dx;
    *y += dy;

    // Constrain character within screen boundaries
    if (*x < 0)
        *x = 0;
    else if (*x > LCD_MAX_X - LCDGetCharWidth())
        *x = LCD_MAX_X - LCDGetCharWidth();
    if (*y < 0)
        *y = 0;
    else if (*y > LCD_MAX_Y - LCDGetCharHeight())
        *y = LCD_MAX_Y - LCDGetCharHeight();

    // Place the character at the new position
    placeCharacter(x, y, c, charFColor, charBColor);
}

int main(void) {
    // Declare and initialize variables for joystick and button states
    unsigned int joyst0, joyst1, joystREdge;
    unsigned int but0, but1;

    // Screen and character color variables
    unsigned int scrColor = BLACK;
    unsigned int charFColor = YELLOW;
    unsigned int charBColor = RED;

    // Variables for character movement steps and repeat delay
    unsigned int charDeltaX = 2;
    unsigned int charDeltaY = 2;
    unsigned int charRepeatDelay = 10;
    unsigned int charRepeatPeriod = 3;

    // Counters for handling auto-repeat of joystick movement
    unsigned int count1 = 0;
    unsigned int count2 = 0;

    // Initial coordinates for the character
    int x, y;

    // Character to be displayed
    char c = 'X';

    // Disable watchdog timer to prevent system resets during operation
    WDTC_Disable(pWDTC);

    // Enable clock for PIOA and PIOB peripherals
    PMC_EnablePeriphClock(pPMC, AT91C_ID_PIOA);
    PMC_EnablePeriphClock(pPMC, AT91C_ID_PIOB);

    // Configure joystick and buttons I/O pins as inputs with deglitching
    PIO_CfgPin(JOYSTICK_PIO_BASE, PIO_INPUT, PIO_DEGLITCH, JOYSTICK_ALL_bm);
    PIO_CfgPin(BUTTONS_PIO_BASE, PIO_INPUT, PIO_DEGLITCH, BUTTON_ALL_bm);

    // Configure SPI pins, SPI and LCD controllers
    CfgLCDCtrlPins();
    LCDInitSpi(LCD_SPI_BASE, LCD_SPI_ID);
    LCDInitCtrl(LCDRstPin);

    // Clear LCD screen and set up for displaying characters
    LCDClrScr(BLACK);
    LCDCharSize(LARGE);    // Set character size
    LCDInitCharIO();
    CfgLCDBacklightPin();  // Configure backlight pin
    LCDBacklight(LCD_BL_ON);  // Turn on LCD backlight

    // Set initial position of the character to the center of the screen
    x = (LCD_MAX_X / 2) - (LCDGetCharWidth() / 2);
    y = LCD_MAX_Y / 2 - LCDGetCharHeight() / 2;
    placeCharacter(&x, &y, c, charFColor, charBColor);  // Place character at initial position

    // Main loop
    while (1) {
        // Read current joystick and button inputs
        joyst0 = ~PIO_GetInput(JOYSTICK_PIO_BASE);
        joystREdge = ~joyst1 & joyst0;  // Calculate rising edge for joystick inputs
        but0 = ~PIO_GetInput(BUTTONS_PIO_BASE);

        // Move character based on joystick input
        if (joystREdge & JOYSTICK_UP_bm)
            moveCharacter(&x, &y, 0, -charDeltaY, c, scrColor, charFColor, charBColor);  // Move up
        else if (joystREdge & JOYSTICK_DOWN_bm)
            moveCharacter(&x, &y, 0, charDeltaY, c, scrColor, charFColor, charBColor);  // Move down
        else if (joystREdge & JOYSTICK_LEFT_bm)
            moveCharacter(&x, &y, -charDeltaX, 0, c, scrColor, charFColor, charBColor);  // Move left
        else if (joystREdge & JOYSTICK_RIGHT_bm)
            moveCharacter(&x, &y, charDeltaX, 0, c, scrColor, charFColor, charBColor);  // Move right

        // Adjust movement step size when SW1 button is pressed
        if ((but0 & BUTTON_SW1_bm) != 0 & (but0 & BUTTON_SW2_bm) == 0) {
            charDeltaX = 4;  // Increase X-axis step size
            charDeltaY = 4;  // Increase Y-axis step size
        } else {
            charDeltaX = 2;  // Default X-axis step size
            charDeltaY = 2;  // Default Y-axis step size
        }

        // Reset character position to the center if SW2 button is pressed
        if ((but0 & BUTTON_SW1_bm) == 0 & (but0 & BUTTON_SW2_bm) != 0) {
            LCDGotoXY(x, y);
            LCDCharColor(charFColor, scrColor);  // Set to clear the character
            LCDPutChar(' ');  // Clear the current character
            x = (LCD_MAX_X / 2) - (LCDGetCharWidth() / 2);
            y = LCD_MAX_Y / 2 - LCDGetCharHeight() / 2;
            placeCharacter(&x, &y, c, charFColor, charBColor);  // Place character at the center
        }

        // Increment count1 if joystick is held
        if (joyst0)
            count1++;
        else
            count1 = 0;

        // Handle continuous movement if joystick is held down
        if (count1 >= charRepeatDelay) {
            count2++;
            if (count2 >= charRepeatPeriod) {
                if (joyst0 & JOYSTICK_UP_bm)
                    moveCharacter(&x, &y, 0, -charDeltaY, c, scrColor, charFColor, charBColor);  // Move up continuously
                else if (joyst0 & JOYSTICK_DOWN_bm)
                    moveCharacter(&x, &y, 0, charDeltaY, c, scrColor, charFColor, charBColor);  // Move down continuously
                else if (joyst0 & JOYSTICK_LEFT_bm)
                    moveCharacter(&x, &y, -charDeltaX, 0, c, scrColor, charFColor, charBColor);  // Move left continuously
                else if (joyst0 & JOYSTICK_RIGHT_bm)
                    moveCharacter(&x, &y, charDeltaX, 0, c, scrColor, charFColor, charBColor);  // Move right continuously
                else
                    count1 = 0;  // Reset count if no joystick direction is active
                count2 = 0;  // Reset count2 for next repeat period
            }
        }

        // Store current joystick and button states for edge detection
        joyst1 = joyst0;
        but1 = but0;

        // Wait for a short period (100000 microseconds --> 100 ms)
        Delay_us(100000U);
    }

    return 0;
}
