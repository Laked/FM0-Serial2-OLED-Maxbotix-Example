#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
//  #define BUTTON_A  9  //using for BatVolts
#define BUTTON_B  6
#define BUTTON_C  5
#define BATTERY_PIN A7
#define DISPLAY_LINE_BUFFER_LENGTH 22

uint16_t battery_mV;
char display_line_buffer[DISPLAY_LINE_BUFFER_LENGTH];
char display_scroll_buffer_0[DISPLAY_LINE_BUFFER_LENGTH];
char display_scroll_buffer_1[DISPLAY_LINE_BUFFER_LENGTH];
char display_scroll_buffer_2[DISPLAY_LINE_BUFFER_LENGTH];
char display_scroll_buffer_3[DISPLAY_LINE_BUFFER_LENGTH];
uint8_t display_line_buffer_length;


//hardware UART for receiving Maxbotix Serial  Rx Pin D11, Tx Pin D10
// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3

// Instantiate the Serial2 class
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);

//Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}


void setup() {
  Serial2.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  // pinMode(BUTTON_A, INPUT_PULLUP); // Using for BatVolts
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  //Red LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  appendAndScroll("Starting...");
  blip(3);
}

char c;
char maxbotix_buffer[4];
uint16_t maxbotix_int;
uint16_t cycle_count = 0;

void loop() {

  if (Serial2.available()) {
    blip(2);
    c = Serial2.read();
    display_line_buffer[0] = c;
    if (c == 'R') {
      for (int i = 0; i < 4; i++) {
        maxbotix_buffer[i] = Serial2.read();
      }
      c = Serial2.read();
      if (c == 13) {  // looks for a carriage return and sends a line end
        maxbotix_int = atoi(maxbotix_buffer);
        display_line_buffer_length = sprintf(display_line_buffer, "%4d  %4dmm  %4dmV", cycle_count++, maxbotix_int, battery_mV);
        appendAndScroll(display_line_buffer);
      }

    }
  }else{
//    blip(1);
  }


  battery_mV = get_mV(BATTERY_PIN);
  //  display_line_buffer_length = sprintf(display_line_buffer, "      Battery: %4dmV", battery_mV);
  //  appendAndScroll(display_line_buffer);
  delay(100);
  yield();
}

void appendAndScroll(char s[DISPLAY_LINE_BUFFER_LENGTH]) {
  display.clearDisplay();
  display.setCursor(0, 0);
  memcpy(display_scroll_buffer_3, display_scroll_buffer_2, DISPLAY_LINE_BUFFER_LENGTH); //Length 32B
  memcpy(display_scroll_buffer_2, display_scroll_buffer_1, DISPLAY_LINE_BUFFER_LENGTH); //Length 32B
  memcpy(display_scroll_buffer_1, display_scroll_buffer_0, DISPLAY_LINE_BUFFER_LENGTH); //Length 32B
  memcpy(display_scroll_buffer_0, s, DISPLAY_LINE_BUFFER_LENGTH); //Length 22B
  display.println(display_scroll_buffer_3);
  display.println(display_scroll_buffer_2);
  display.println(display_scroll_buffer_1);
  display.println(display_scroll_buffer_0);
  display.display();
}

uint16_t get_mV(int pin) {
  return (66 * analogRead(pin)) / 10; //For a 0.5*Voltage Divider and a 3.3V RefV
}

void blip(uint8_t count) {
  for (; 0 < count; count--) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
}
