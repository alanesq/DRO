// See SetupX_Template.h for all options available
// this file: "Setup91_ILI9341_2_CYD2U.h
// comment any other definition and add following (without quotes) to User_Setup_Select.h "#include <User_Setups/Setup91_ILI9341_2_CYD2U.h>   // Setup file for CYD  TFT    240x320"

#define USER_SETUP_ID 91

// User defined information reported by "Read_User_Setup" test & diagnostics example
//#warning "ILI9341_2 CYD2U" selected
#define USER_SETUP_INFO "ILI9341_2 CYD2U"

#define ILI9341_2_DRIVER

#define TFT_WIDTH  	240 	// ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 	320 	// ST7789 240 x 320

#define TFT_INVERSION_ON	// required for 2 USB version of CYD
#define TFT_BACKLIGHT_ON HIGH  	// Level to turn ON back-light (HIGH or LOW)

#define TFT_BL   	21      // LED back-light control pin
#define TFT_MISO 	12
#define TFT_MOSI 	13
#define TFT_SCLK 	14
#define TFT_CS   	15  	// Chip select control pin
#define TFT_DC    	 2  	// Data Command control pin
//#define TFT_RST   	 4  	// Reset pin (could connect to RST pin)
#define TFT_RST  	-1  	// Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define XPT2046_IRQ  	36
#define XPT2046_MOSI 	32
#define XPT2046_MISO 	39
#define XPT2046_CLK  	25
#define XPT2046_CS   	33
// duplicates ...
#define TOUCH_CS     	33	// Chip select pin (T_CS) of touch screen
#define TOUCH_IRQ    	36

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT

#define SPI_FREQUENCY  	    40000000
#define SPI_READ_FREQUENCY   6000000
#define SPI_TOUCH_FREQUENCY  2500000

