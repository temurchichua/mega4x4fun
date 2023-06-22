#include <Arduino.h>
/******************************************************************************
red-plus-buttons.ino
Byron Jacquot @ SparkFun Electronics
1/6/2015

Example to drive the red LEDs and scan the buttons of the RGB button pad.

Exercise 2 in a series of 3.
https://learn.sparkfun.com/tutorials/button-pad-hookup-guide/exercise-2-monochrome-plus-buttons

Development environment specifics:
Developed in Arduino 1.6.5
For an Arduino Mega 2560

This code is released under the [MIT License](http://opensource.org/licenses/MIT).

Distributed as-is; no warranty is given.
******************************************************************************/
//config variables
#define NUM_LED_COLUMNS (4)
#define NUM_LED_ROWS (4)
#define NUM_BTN_COLUMNS (4)
#define NUM_BTN_ROWS (4)
#define NUM_COLORS (3)

#define MAX_DEBOUNCE (3)

// Global variables
static bool LED_outputs[NUM_LED_COLUMNS][NUM_LED_ROWS];
static int32_t next_scan;

static const uint8_t button_grounds[NUM_BTN_COLUMNS]         = {43, 45, 47, 49};
static const uint8_t button_rows[NUM_BTN_ROWS]           = {22, 26, 30, 34};

static const uint8_t led_grounds[NUM_LED_COLUMNS]         = {42, 44, 46, 48};
static const uint8_t led_rows_rgb[NUM_LED_ROWS][NUM_COLORS] = {{23, 24, 25},
                                                               {27, 28, 29},
                                                               {31, 32, 33},
                                                               {35, 36, 37} };
// first column is red, second is green, third is blue



static int8_t debounce_count[NUM_BTN_COLUMNS][NUM_BTN_ROWS]; // debounce counter for each button
// it is used to count the number of times a button is read as pressed

static void setuppins()
{
    uint8_t i;

    // initialize
    // select lines
    for(i = 0; i < NUM_LED_COLUMNS; i++)
    {
        pinMode(led_grounds[i], OUTPUT);

        // with nothing selected by default
        digitalWrite(led_grounds[i], HIGH);
    }

    for(i = 0; i < NUM_BTN_COLUMNS; i++)
    {
        pinMode(button_grounds[i], OUTPUT);

        // with nothing selected by default
        digitalWrite(button_grounds[i], HIGH);
    }

    // key return lines
    for(i = 0; i < 4; i++)
    {
        pinMode(button_rows[i], INPUT_PULLUP);
    }

    // LED drive lines
    for(i = 0; i < NUM_LED_ROWS; i++)
    {
        for(uint8_t j = 0; j < NUM_COLORS; j++)
        {
            pinMode(led_rows_rgb[i][j], OUTPUT);
            digitalWrite(led_rows_rgb[i][j], LOW);
        }
    }

    for(uint8_t i = 0; i < NUM_BTN_COLUMNS; i++)
    {
        for(uint8_t j = 0; j < NUM_BTN_ROWS; j++)
        {
            debounce_count[i][j] = 0;
        }
    }
}

static void scan()
{
    static uint8_t current = 0;
    uint8_t val;
    uint8_t i, j;

    //run
    digitalWrite(button_grounds[current], LOW);
    digitalWrite(led_grounds[current], LOW);

    for(i = 0; i < NUM_LED_ROWS; i++)
    {
        uint8_t val = (LED_outputs[current][i] & 0x03);

        if(val)
        {
            digitalWrite(led_rows_rgb[i][val - 1], HIGH);
        }
    }


    delay(1);

    for( j = 0; j < NUM_BTN_ROWS; j++)
    {
        val = digitalRead(button_rows[j]);

        if(val == LOW)
        {
            // active low: val is low when btn is pressed
            if( debounce_count[current][j] < MAX_DEBOUNCE)
            {
                debounce_count[current][j]++;
                if( debounce_count[current][j] == MAX_DEBOUNCE )
                {
                    Serial.print("Key Down ");
                    Serial.println((current * NUM_BTN_ROWS) + j);

                    LED_outputs[current][j]++;
                }
            }
        }
        else
        {
            // otherwise, button is released
            if( debounce_count[current][j] > 0)
            {
                debounce_count[current][j]--;
                if( debounce_count[current][j] == 0 )
                {
                    Serial.print("Key Up ");
                    Serial.println((current * NUM_BTN_ROWS) + j);
                }
            }
        }
    }// for j = 0 to 3;

    delay(1);

    digitalWrite(button_grounds[current], HIGH);
    digitalWrite(led_grounds[current], HIGH);

    for(i = 0; i < NUM_LED_ROWS; i++)
    {
        for(j = 0; j < NUM_COLORS; j++)
        {
            digitalWrite(led_rows_rgb[i][j], LOW);
        }
    }

    current++;
    if (current >= NUM_BTN_COLUMNS)
    {
        current = 0;
    }
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    Serial.print("Starting Setup...");

    // setup hardware
    setuppins();

    // init global variables
    next_scan = millis() + 1;

    for(uint8_t i = 0; i < NUM_LED_ROWS; i++)
    {
        for(uint8_t j = 0; j < NUM_LED_COLUMNS; j++)
        {
            LED_outputs[i][j] = 0;
        }
    }

    Serial.println("Setup Complete.");
}

void loop() {
    // put your main code here, to run repeatedly:

    if(millis() >= next_scan)
    {
        next_scan = millis()+1;
        scan();
    }
}