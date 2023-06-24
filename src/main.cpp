#include <Arduino.h>

#include <Wire.h>           // Include the I2C library (required)
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509



//config variables
#define NUM_LED_COLUMNS (4)
#define NUM_LED_ROWS (4)
#define NUM_BTN_COLUMNS (4)
#define NUM_BTN_ROWS (4)
#define NUM_COLORS (3)

#define MAX_DEBOUNCE (2)

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout

// Global variables
static uint8_t LED_outputs[NUM_LED_COLUMNS][NUM_LED_ROWS];
static int32_t next_scan;

static const uint8_t button_grounds[NUM_BTN_COLUMNS]         = {0, 2, 4, 6};
static const uint8_t led_grounds[NUM_LED_COLUMNS]         = {1, 3, 5, 7};

static const uint8_t button_rows[NUM_BTN_ROWS]           =  {8, 10, 12, 14};
static const uint8_t led_red_rows[NUM_LED_ROWS]           = {9, 11, 13, 15};



static int8_t debounce_count[NUM_BTN_COLUMNS][NUM_BTN_ROWS]; // debounce counter for each button
// it is used to count the number of times a button is read as pressed

static void pad_pins_setup()
{

    // initialize
    // select lines
    // LED columns
    for (uint8_t i = 0; i < NUM_LED_COLUMNS; i++) {
        io.pinMode(led_grounds[i], OUTPUT);
        // with nothing selected by default
        io.digitalWrite(led_grounds[i], HIGH);
    }

    // LED drive lines
    for (uint8_t i = 0; i < NUM_LED_ROWS; i++) {
        io.pinMode(led_red_rows[i], OUTPUT);
        io.digitalWrite(led_red_rows[i], LOW);
    }

    // button columns
    for (uint8_t i = 0; i < NUM_BTN_COLUMNS; i++) {
        io.pinMode(button_grounds[i], OUTPUT);

        // with nothing selected by default
        io.digitalWrite(button_grounds[i], HIGH);
    }

    // button row input lines
    for (uint8_t i = 0; i < NUM_BTN_ROWS; i++) {
        io.pinMode(button_rows[i], INPUT_PULLUP);
    }

    // Initialize the debounce counter array
    for (uint8_t i = 0; i < NUM_BTN_COLUMNS; i++) {
        for (uint8_t j = 0; j < NUM_BTN_ROWS; j++) {
            debounce_count[i][j] = 0;
        }
    }
}

static void scan()
{
    static uint8_t current = 0;
    uint8_t val;
    uint8_t i, j;

    // Select current columns
    io.digitalWrite(button_grounds[current], LOW);
    io.digitalWrite(led_grounds[current], LOW);

    // output LED row values
    for (i = 0; i < NUM_LED_ROWS; i++) {
        if (LED_outputs[current][i]) {
            io.digitalWrite(led_red_rows[i], HIGH);
        }
    }

    // pause a moment
    delay(1);

    // Read the button inputs
    for ( j = 0; j < NUM_BTN_ROWS; j++) {
        val = io.digitalRead(button_rows[j]);

        if (val == LOW) {
            // active low: val is low when btn is pressed
            if ( debounce_count[current][j] < MAX_DEBOUNCE) {
                debounce_count[current][j]++;
                if ( debounce_count[current][j] == MAX_DEBOUNCE ) {
                    Serial.print("Key Down ");
                    Serial.println((current * NUM_BTN_ROWS) + j);

                    // Do whatever you want to with the button press here:
                    // toggle the current LED state
                    LED_outputs[current][j] = !LED_outputs[current][j];
                }
            }
        }
        else {
            // otherwise, button is released
            if ( debounce_count[current][j] > 0) {
                debounce_count[current][j]--;
                if ( debounce_count[current][j] == 0 ) {
                    Serial.print("Key Up ");
                    Serial.println((current * NUM_BTN_ROWS) + j);

                    // If you want to do something when a key is released, do it here:

                }
            }
        }
    }// for j = 0 to 3;

    delay(1);

    io.digitalWrite(button_grounds[current], HIGH);
    io.digitalWrite(led_grounds[current], HIGH);

    for (i = 0; i < NUM_LED_ROWS; i++) {
        io.digitalWrite(led_red_rows[i], LOW);
    }

    current++;
    if (current >= NUM_LED_COLUMNS) {
        current = 0;
    }

}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("Starting Setup...");

    Wire.begin(); //Initialize I2C bus
    Serial.println("I2C bus initialized.");

    // Call io.begin(<address>) to initialize the SX1509. If it
    // successfully communicates, it'll return 1.
    bool io_success = io.begin(SX1509_ADDRESS);
    if (!io_success) {
        Serial.println("Failed to communicate. Check wiring and address of SX1509.");
        io.digitalWrite(13, HIGH); // If we failed to communicate, turn the pin 13 LED on
        while (1)
            ; // If we fail to communicate, loop forever.
    }
    Serial.println("SX1509 communication OK.");

    // setup 4x4 pad
    pad_pins_setup();

    // init global variables
    for (uint8_t i = 0; i < NUM_LED_COLUMNS; i++) {
        for (uint8_t j = 0; j < NUM_LED_ROWS; j++) {
            LED_outputs[i][j] = 0;
        }
    }

    Serial.println("Setup Complete.\n");

}

void loop() {
    // put your main code here, to run repeatedly:
    scan();
}