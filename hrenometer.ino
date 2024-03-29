/*
Хренометр 191116-1
by AlexGyver & Butylkus
Вычислялки и сенсорика в основном Гайвер, свистоперделки и немного оптимизации - Бутылкус.
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


int set;
boolean initial;         //флаг первого пуска
int n=1;                 //номер выстрела, начиная с 1
float velocity;          //переменная скорости
float mass[4] = {0.67, 0.87, 0.91, 1.05};         //массы снарядов в граммах. На калибр и прочее пофиг. На экран лезет только 4 строки, поэтому только 4 массы в памяти.
float dist=0.124;        //расстояние между датчиками в метрах  
volatile unsigned long gap1, gap2;    //отметки времени прохождения пулей датчиков



// Hardware SPI (Потому что ниибаца быстрее софтверного):
// LCD serial clock (SCLK) - D13 на Uno
// LCD DIN - D11 на Uno
// D5 - Data/Command (D/C)
// D7 - chip select (CS)
// D6 - reset (RST)
// уотакуот, потому что на дефолтных пинах либы (3й сто процентов) висят прерывания
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 7, 6);


//оверлей Хренометра - лого
const unsigned char PROGMEM logo [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x81, 0xf8, 
0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x1f, 0xc7, 0x3c, 0xf0, 0xe3, 0xff, 0xe0, 0x00, 
0x00, 0x78, 0x38, 0x71, 0xe7, 0x00, 0x70, 0xe3, 0x80, 0xf0, 0x00, 0x00, 0x38, 0x78, 0x70, 0xe7, 
0x00, 0x70, 0xe3, 0x80, 0x70, 0x00, 0x00, 0x3c, 0x70, 0x70, 0xe7, 0x00, 0xf1, 0xe3, 0x80, 0x70, 
0x00, 0x00, 0x1e, 0xf0, 0x70, 0xe7, 0x00, 0xe1, 0xc3, 0x80, 0x70, 0x00, 0x00, 0x0e, 0xe0, 0x71, 
0xef, 0xc0, 0xff, 0xc3, 0xc0, 0x70, 0x00, 0x00, 0x0f, 0xe0, 0x7f, 0xcf, 0xf8, 0xe1, 0xe1, 0xc0, 
0x70, 0x00, 0x00, 0x07, 0xc0, 0x7f, 0x8e, 0x01, 0xe0, 0xe1, 0xc0, 0xf0, 0x00, 0x00, 0x07, 0x80, 
0xf0, 0x0e, 0x01, 0xc0, 0xe1, 0xe0, 0xe0, 0x00, 0x00, 0x0f, 0x80, 0xe0, 0x0e, 0x01, 0xc0, 0xf0, 
0xe0, 0xe0, 0x00, 0x00, 0x1f, 0xc0, 0xe0, 0x0e, 0x01, 0xc0, 0x70, 0xf1, 0xe0, 0x00, 0x00, 0x1d, 
0xe0, 0xe0, 0x0f, 0x01, 0xc0, 0x70, 0x73, 0xc0, 0x00, 0x00, 0x38, 0xe0, 0xe0, 0x07, 0xf0, 0x00, 
0x70, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1d, 0xf0, 0x00, 0x00, 0x00, 0x00, 
0x71, 0xc7, 0xfe, 0x0f, 0xff, 0x9c, 0x70, 0x00, 0x00, 0x00, 0x00, 0xf9, 0xc7, 0x8e, 0x7d, 0xc3, 
0x9c, 0x70, 0x00, 0x00, 0x00, 0x00, 0xf9, 0xe3, 0x80, 0x01, 0xc0, 0x1c, 0x70, 0x00, 0x00, 0x00, 
0x01, 0xfb, 0xe3, 0x80, 0x01, 0xc0, 0x1c, 0x70, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe3, 0x80, 0x01, 
0xc0, 0x1c, 0x70, 0x00, 0x00, 0x00, 0x03, 0xff, 0xe3, 0x80, 0x01, 0xc0, 0x1c, 0x70, 0x00, 0x00, 
0x00, 0x03, 0xbe, 0xe3, 0xfe, 0x01, 0xc0, 0x39, 0xf0, 0x00, 0x00, 0x00, 0x03, 0x9d, 0xe3, 0x80, 
0x01, 0xc0, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x07, 0x81, 0xc3, 0x80, 0x01, 0xc0, 0x38, 0x00, 0x00, 
0x00, 0x00, 0x07, 0x01, 0xe3, 0x80, 0x01, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0xe3, 
0x80, 0x01, 0xc0, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0xe3, 0xff, 0x03, 0xc0, 0x1c, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0xe3, 0xc7, 0x03, 0x80, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x40, 0x01, 0xc0, 0x02, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x70, 
0x02, 0x00, 0x11, 0x00, 0x01, 0x80, 0x00, 0x00, 0x62, 0x41, 0x12, 0x63, 0x00, 0x11, 0x04, 0xc1, 
0xc0, 0x00, 0x00, 0x62, 0x41, 0x72, 0x23, 0x89, 0x91, 0x24, 0x43, 0x00, 0x00, 0x00, 0x7b, 0x41, 
0xc2, 0x22, 0x0c, 0x91, 0xc4, 0x43, 0x80, 0x00, 0x00, 0x69, 0xc1, 0x62, 0x62, 0x04, 0x91, 0x84, 
0xc0, 0xc0, 0x00, 0x00, 0x58, 0x41, 0x33, 0xf2, 0x43, 0x91, 0x67, 0xf0, 0x40, 0x00, 0x00, 0x78, 
0x41, 0xf1, 0x03, 0xc1, 0x19, 0x22, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x0f, 0x00, 
0x00, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ещё оверлей - подложка под показания
const unsigned char PROGMEM overlay [] = {
0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x01, 0xc8, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x66, 0x00, 0x00, 0x01, 0xc8, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x66, 0x42, 0x72, 0x12, 
0xaf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x5a, 0x42, 0x82, 0x12, 0xa8, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x5a, 0x7a, 0x83, 0xd0, 0x88, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x4a, 0x82, 
0x50, 0x88, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x7a, 0x73, 0xd0, 0x88, 0x00, 0x00, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
0x00, 0xf0, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x08, 
0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x91, 0x17, 0x90, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x91, 0x14, 0x50, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x90, 0xa4, 0x5e, 
0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x10, 0xa4, 0x52, 0x08, 0x00, 0x00, 0x01, 0x00, 
0x00, 0x00, 0x03, 0xf8, 0x47, 0x9e, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x08, 0x44, 
0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x09, 0x84, 0x00, 0x0f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x01, 0x88, 0x00, 0x00, 0x01, 0x1f, 0x66, 0x60, 0x00, 0x00, 
0x00, 0x00, 0x68, 0x79, 0xef, 0x01, 0x32, 0x34, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x49, 0x09, 
0x01, 0x32, 0x1d, 0x80, 0x00, 0x00, 0x00, 0x00, 0x68, 0x7d, 0xe8, 0x01, 0x32, 0x1f, 0x80, 0x00, 
0x00, 0x00, 0x01, 0x88, 0x45, 0x08, 0x01, 0x7e, 0x1b, 0x80, 0x00, 0x00, 0x00, 0x00, 0x08, 0x45, 
0x29, 0x01, 0x42, 0x32, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x08, 0x7d, 0xef, 0x01, 0x43, 0x66, 0x60
};


// Ви таки не повегите - ещё одна картинка! Кажет "фейл" при кривой стрельбе
const unsigned char PROGMEM moloko [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x01, 
0x80, 0x00, 0x00, 0x1c, 0x03, 0xe0, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x02, 
0x30, 0x00, 0x02, 0x21, 0x11, 0x8a, 0x40, 0x00, 0x00, 0x41, 0x03, 0x1c, 0x00, 0x02, 0x21, 0x0a, 
0x8a, 0xc0, 0x00, 0x00, 0x60, 0xc1, 0x86, 0x00, 0x02, 0x3d, 0x04, 0x53, 0xc0, 0x00, 0x00, 0x20, 
0x60, 0x80, 0x00, 0x02, 0x25, 0x0a, 0x53, 0x40, 0x00, 0x00, 0x20, 0x20, 0xc0, 0x00, 0x02, 0x3d, 
0x11, 0x22, 0x40, 0x40, 0x1c, 0x30, 0x30, 0x40, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x40, 0x1e, 
0x10, 0x10, 0x60, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x40, 0x0b, 0x8c, 0x10, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x40, 0x0c, 0xc6, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 
0x04, 0x63, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x30, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x38, 0x40, 0x04, 0x18, 0x00, 0x00, 0x00, 0x00, 0x38, 0x18, 0xf0, 0xe8, 
0x40, 0x06, 0x08, 0x00, 0x00, 0x23, 0x89, 0x24, 0x31, 0x98, 0x80, 0x40, 0x02, 0x00, 0x64, 0x78, 
0x72, 0x8b, 0x32, 0x21, 0x09, 0x80, 0x40, 0x02, 0x00, 0x64, 0x48, 0x52, 0xce, 0x12, 0x63, 0x09, 
0x00, 0x40, 0x02, 0x00, 0xec, 0x4c, 0x52, 0x4c, 0x12, 0x42, 0x19, 0x00, 0x00, 0x02, 0x00, 0xac, 
0x44, 0xd2, 0x4e, 0x12, 0x41, 0xf1, 0x00, 0x00, 0x00, 0x01, 0xb4, 0x44, 0x9a, 0x4a, 0x12, 0x60, 
0x00, 0x84, 0x00, 0x00, 0x03, 0x06, 0x44, 0x8b, 0xcb, 0x3e, 0x3c, 0x00, 0xf8, 0x40, 0x00, 0x00, 
0x02, 0x7c, 0x09, 0x89, 0x18, 0x00, 0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x90, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x07, 0xee, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 
0x8b, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x98, 0x9e, 0x00, 0x02, 0x00, 
0x00, 0x00, 0xff, 0xff, 0xff, 0x0d, 0x90, 0x90, 0x30, 0x03, 0x40, 0x00, 0x00, 0x84, 0x10, 0x5f, 
0x01, 0x10, 0x90, 0x7c, 0x01, 0xc0, 0x00, 0x00, 0xbd, 0xdd, 0xdf, 0x01, 0x1f, 0x30, 0xc4, 0x18, 
0x00, 0x00, 0x00, 0xbd, 0xdd, 0xdf, 0x00, 0x00, 0x30, 0xc2, 0x30, 0x00, 0x00, 0x00, 0x84, 0x1d, 
0xdf, 0x00, 0x00, 0x1c, 0xe2, 0x20, 0x80, 0x00, 0x00, 0xbd, 0xdd, 0xdf, 0x00, 0x00, 0x00, 0x84, 
0x60, 0x80, 0x00, 0x00, 0xbd, 0xdd, 0xdf, 0x00, 0x00, 0x00, 0x74, 0x63, 0x80, 0x00, 0x00, 0xbd, 
0xdd, 0xdf, 0x00, 0x00, 0x00, 0x18, 0x3f, 0x00, 0x00, 0x00, 0xbd, 0xd0, 0x41, 0x00, 0x00, 0x00, 
0x00, 0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00
};



void setup() {
	Serial.begin(9600);               //открываем COM-порт
	attachInterrupt(1,start,RISING);  //аппаратное прерывание при прохождении первого датчика
	attachInterrupt(0,finish,RISING); //аппаратное прерывание при прохождении второго датчика

  display.begin();                  //включаем в работу дисплей
  display.setContrast(60);          //тут надо поиграться, многое зависит от модели и чуть ли не фазы луны
 
}

// Рисует приветствие на экране
void ShowLogo(){
  display.clearDisplay();
  display.drawBitmap(0,0,logo,84,48,1);
  display.display();
}

// Выдаёт результат в виде таблички из скорости и энергий для заданных масс
void ShowResult(){
  display.clearDisplay();
  display.setTextColor(BLACK);
  display.drawBitmap(0,0,overlay,84,48,1);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(round(velocity)); // если там 100+, то можно забить на десятые, это уже неважно
  display.setTextSize(1);
  display.setCursor(67,2);
  display.print(round(mass[0]*sq(velocity)/2000));
  display.setCursor(67,12);
  display.print(round(mass[1]*sq(velocity)/2000));
  display.setCursor(67,22);
  display.print(round(mass[2]*sq(velocity)/2000));
  display.setCursor(67,32);
  display.print(round(mass[3]*sq(velocity)/2000));
  display.setCursor(38,2);
  display.print(mass[0]);
  display.setCursor(38,12);
  display.print(mass[1]);
  display.setCursor(38,22);
  display.print(mass[2]);
  display.setCursor(38,32);
  display.print(mass[3]);
  display.display();
}

// Показывает ошибку при кривом выстреле
void ShowFail(){
  display.clearDisplay();
  display.drawBitmap(0,0,moloko,84,48,1);
  display.display();
  delay(2000);
  ShowLogo();
}


void start(){
	if (gap1==0) {   //если измерение еще не проводилось
		gap1=micros(); //получаем время первой отсечки
	}
}

void finish() {
	if (gap2==0) {    //если измерение еще не проводилось
		gap2=micros();  //получаем время второй отсечки
	}
}

void loop() {
	if (initial==0) {                                         //флажок первого запуска
		Serial.println("Press 1 to service mode");              //режим отладки (резисторы, положение диодов-транзов, уровни сигналов)
		Serial.println("Press 0 speed measure mode (default)"); //выход из режимов
		Serial.println("System is ready");                      //уведомление о том, что хрон готов к работе
		Serial.println(" ");
		initial=1;                                              //первый запуск, больше не показываем сообщения
    ShowLogo();                                             // на экране лого
	}

	if (Serial.available() > 0 && set!=2) {   //если есть какие буквы на входе с порта
		int val=Serial.read();                  //прочитать что было послано в порт
		switch(val) {                           //оператор выбора

		case 48: set=0; initial=0; break;       //если приняли 0 то выбрать 0 режим
		case 49: set=1; break;                  //если приняли 1 то запустить режим 1
		}
	}

// режим калибровки датчиков. За подробностями к Гайверу.
	if (set==1) {                       //если 1 режим
	  	Serial.print("sensor 1: ");
	  	Serial.println(analogRead(2));  //показать значение на первом датчике
      Serial.print("sensor 2: ");
      Serial.println(analogRead(4));  //показать значение на первом датчике
	  	Serial.println();        
    
	}


	if (gap1!=0 && gap2!=0 && gap2>gap1 && set==0) {  //если пуля прошла оба датчика в 0 режиме
		velocity=(1000000*(dist)/(gap2-gap1));          //вычисление скорости
    ShowResult();                                   //показываем результат на экране
		gap1=0;                                         //сброс значений
		gap2=0;
		n++;                                            //номер выстрела +1
	}

	if (micros()-gap1>1000000 && gap1!=0 && set!=1) { // (если пуля прошла первый датчик) И (прошла уже 1 секунда, а второй датчик не тронут)
		Serial.println("FAIL");                         //выдаёт FAIL через 1 секунду, если пуля прошла через первый датчик, а через второй нет
    ShowFail();                                     // показываем ошибку на экране
		gap1=0;
		gap2=0;
	}

	delay(100);  //задержка для стабильности. Если убрать, экран будет обновляться постоянно. Также освобождает систему для более аккуратной работы прерываний.
}
