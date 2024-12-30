#include <stdio.h>
#include <stdlib.h>
#include "EPD_Test.h"
#include "EPD_2in13_V3.h"

// Segment mapping for numbers 0-9
const int segments[10][7] = {
    {1, 1, 1, 0, 1, 1, 1}, // 0
    {0, 0, 1, 0, 0, 1, 0}, // 1
    {1, 0, 1, 1, 1, 0, 1}, // 2
    {1, 0, 1, 1, 0, 1, 1}, // 3
    {0, 1, 1, 1, 0, 1, 0}, // 4
    {1, 1, 0, 1, 0, 1, 1}, // 5
    {1, 1, 0, 1, 1, 1, 1}, // 6
    {1, 0, 1, 0, 0, 1, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

// Draw a single segment
void DrawSegment(int x, int y, int length, int thickness, int segment, int state) {
    int offsetX = 0, offsetY = 0;

    // Set segment offsets
    switch (segment) {
        case 0: offsetX = length / 2; offsetY = 0; break;  // Top
        case 1: offsetX = 0; offsetY = length / 2; break;  // Top-Left
        case 2: offsetX = length; offsetY = length / 2; break;  // Top-Right
        case 3: offsetX = length / 2; offsetY = length; break;  // Middle
        case 4: offsetX = 0; offsetY = length + length / 2; break;  // Bottom-Left
        case 5: offsetX = length; offsetY = length + length / 2; break;  // Bottom-Right
        case 6: offsetX = length / 2; offsetY = length * 2; break;  // Bottom
    }

    if (state) {
        if (segment % 3 == 0) {  // Horizontal segments
            Paint_DrawRectangle(x + offsetX - length / 2, y + offsetY - thickness / 2,
                                x + offsetX + length / 2, y + offsetY + thickness / 2,
                                BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        } else {  // Vertical segments
            Paint_DrawRectangle(x + offsetX - thickness / 2, y + offsetY - length / 2,
                                x + offsetX + thickness / 2, y + offsetY + length / 2,
                                BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        }
    }
}

// Draw a digit using 7-segment style
void DrawDigit(int x, int y, int digit, int length, int thickness) {
    for (int i = 0; i < 7; i++) {
        DrawSegment(x, y, length, thickness, i, segments[digit][i]);
    }
}

// Draw the 7-segment timer
void DrawTimer(int minutes, int seconds) {
    int digitLength = 20;  // Length of each segment
    int digitThickness = 5;  // Thickness of each segment
    int spacing = 10;  // Space between digits

    int x = 50;
    int y = 50;

    // Draw minutes (MM)
    DrawDigit(x, y, minutes / 10, digitLength, digitThickness);
    DrawDigit(x + digitLength + spacing, y, minutes % 10, digitLength, digitThickness);

    // Draw colon
    Paint_DrawRectangle(x + 2 * (digitLength + spacing) - spacing / 2, y + digitLength / 2 - digitThickness,
                        x + 2 * (digitLength + spacing) + spacing / 2, y + digitLength / 2 + digitThickness,
                        BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(x + 2 * (digitLength + spacing) - spacing / 2, y + digitLength + digitThickness,
                        x + 2 * (digitLength + spacing) + spacing / 2, y + digitLength + digitThickness * 2,
                        BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    // Draw seconds (SS)
    DrawDigit(x + 2 * (digitLength + spacing), y, seconds / 10, digitLength, digitThickness);
    DrawDigit(x + 3 * (digitLength + spacing), y, seconds % 10, digitLength, digitThickness);
}

int EPD_2in13_V3_test(void)
{
    printf("EPD_2in13_V3_test Demo\r\n");
    if (DEV_Module_Init() != 0) {
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_2in13_V3_Init();
    EPD_2in13_V3_Clear();

    // Create a new image cache
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_2in13_V3_WIDTH % 8 == 0) ? 
                       (EPD_2in13_V3_WIDTH / 8) : 
                       (EPD_2in13_V3_WIDTH / 8 + 1)) * EPD_2in13_V3_HEIGHT;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to allocate memory...\r\n");
        return -1;
    }
    Paint_NewImage(BlackImage, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, 90, WHITE);
    Paint_Clear(WHITE);

    Paint_SelectImage(BlackImage);

    int minutes = 30;
    int seconds = 0;

    for (;;) {
        Paint_Clear(WHITE);  // Clear the display
        DrawTimer(minutes, seconds);  // Draw the 7-segment timer
        EPD_2in13_V3_Display_Partial(BlackImage);

        DEV_Delay_ms(1000);

        // Timer countdown logic
        if (seconds == 0) {
            if (minutes == 0) {
                break;  // Timer finished
            }
            minutes--;
            seconds = 59;
        } else {
            seconds--;
        }
    }

    // Timer over message
    Paint_Clear(WHITE);
    const char *timeOverMessage = "Time Over";
    int textX = 50;
    int textY = 50;
    Paint_DrawString_EN(textX, textY, timeOverMessage, &Font24, WHITE, BLACK);
    EPD_2in13_V3_Display_Base(BlackImage);

    DEV_Delay_ms(5000);  // Display "Time Over" for 5 seconds

    // Clear the screen and go to sleep
    printf("Clear...\r\n");
    EPD_2in13_V3_Init();
    EPD_2in13_V3_Clear();

    printf("Goto Sleep...\r\n");
    EPD_2in13_V3_Sleep();
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000);
    printf("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();
    return 0;
}
