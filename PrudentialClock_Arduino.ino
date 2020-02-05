/***********************************************************************************
*This program requires the the LCDKIWI library.

* File                : display_string.ino
* Hardware Environment: Arduino UNO&Mega2560
* Build Environment   : Arduino

* @attention

* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************/

//Load libraries
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library
#include <SPI.h> //Serial-peripheral Interface library

//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV mylcd(ILI9486,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset

//if the IC model is not known and the modules is readable,you can use this constructed function
//LCDWIKI_KBV mylcd(320,480,A3,A2,A1,A0,A4);//width,height,cs,cd,wr,rd,reset

//define colour values for clock display
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0xC618

//Define variables for clock
int h = 24;
int m = 0;
int s = 0;
int displayH;
int now;
boolean flash = false;
signed long msCounter;

//Define variables for receiving data from WiFi board
char weatherBuff[50];
char clockBuff[50];
volatile byte indx;
volatile boolean weatherProcess;
volatile boolean clockProcess;
volatile boolean weatherIn;
volatile boolean clockIn;


//LCD Initialize
void setup() 
{
  //Initialize serial communication for debugging.
  Serial.begin(115200);
  
  //Initialize Settings
  mylcd.Init_LCD();
  Serial.println(mylcd.Read_ID(), HEX);
  mylcd.Set_Rotation(3);
  mylcd.Fill_Screen(BLACK);
  mylcd.Set_Text_Mode(0);
  mylcd.Fill_Screen(0x0000);
  mylcd.Set_Text_colour(CYAN);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(14);
  mylcd.Set_Draw_color(WHITE);

  //Draw Prudential Building
  mylcd.Draw_Line(53, 320, 53, 146);
  mylcd.Draw_Line(53, 146, 69, 140);
  mylcd.Draw_Line(69, 137, 58, 137);
  mylcd.Draw_Line(58, 134, 58, 137);
  mylcd.Draw_Line(58, 137, 60, 133);
  mylcd.Draw_Line(60, 133, 60,120);
  mylcd.Draw_Line(60, 120, 58, 120);
  mylcd.Draw_Line(58, 120, 58, 118);
  mylcd.Draw_Line(58, 118, 93, 106);
  mylcd.Draw_Line(93, 106, 93, 121);
  mylcd.Draw_Line(93, 121, 170,124);
  mylcd.Draw_Line(93, 121, 58,133);
  mylcd.Draw_Line(93, 106, 170, 110);
  mylcd.Draw_Line(170, 110, 170, 113);
  mylcd.Draw_Line(170, 113, 167, 111);
  mylcd.Draw_Line(167, 111, 166, 125);
  mylcd.Draw_Line(166, 125, 170, 125);
  mylcd.Draw_Line(170, 125, 170, 129);
  mylcd.Draw_Line(170, 129, 160, 131);
  mylcd.Draw_Line(160, 131, 160, 132);
  mylcd.Draw_Line(160, 132, 176, 134);
  mylcd.Draw_Line(176, 134, 176, 320);
  mylcd.Draw_Line(73, 112, 73, 106);
  mylcd.Draw_Line(73, 106, 88, 100);
  mylcd.Draw_Line(88, 100, 143, 100);
  mylcd.Draw_Line(143, 100, 143, 106);
  mylcd.Draw_Line(113, 11, 113, 95);
  mylcd.Draw_Line(112, 11, 112, 95);
  mylcd.Draw_Line(114, 11, 114, 95);
  mylcd.Draw_Line(93, 320, 93, 130);
  mylcd.Draw_Line(69, 140, 93, 130);
  mylcd.Draw_Line(93, 130, 160,132);
  mylcd.Set_Text_Size(4);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Print_String("GO",115,163);
  mylcd.Print_String("SOX",102,203);
  
  //Set font size and color for time display
  mylcd.Set_Text_colour(CYAN);
  mylcd.Set_Text_Size(14);
  
  //SPI Communication initialize
  pinMode(12,OUTPUT);
  pinMode(10,INPUT);
  SPCR |= _BV(SPE);
  indx = 0;
  weatherProcess = false;
  weatherIn = false;
  clockProcess = false;
  clockIn = false;
  SPI.attachInterrupt();
}

//Interrupt function
ISR (SPI_STC_vect) 
{
  byte c = SPDR;
  //The '@' symbol from the WiFi board communicates that Weather data is incoming.
  //The '%' symbol from the WiFi board communicates that Clock data is incoming.
  if (c == '@') {
    weatherIn = true;
  } else if (c == '%') {
    clockIn = true;
  }
  //The '!' symbol from the WiFi board communicates that transmission of the weather data is complete.
  //The '$' symbol from the WiFi board communicates that transmission of the clock data is complete.
  if (indx < sizeof weatherBuff and weatherIn == true) {
    weatherBuff [indx++] = c;
    if (c == '!') {
      weatherIn = false;
      weatherProcess = true;
    } 
  }
  else if (indx < sizeof clockBuff and clockIn == true) {
    clockBuff [indx++] = c;
    if (c == '$') {
      clockIn = false;
      clockProcess = true;
    }
  }
}


void loop() 
{
  now = millis();
  
  //If new clock data is available, update time variables from buffer.
  if (clockProcess) {
    Serial.println("clockProcess occured");
    clockProcess = false;
    memmove (clockBuff, clockBuff + 1, strlen (clockBuff+2));
    h = clockBuff[0] - '0';
    h*=10;
    memmove (clockBuff, clockBuff + 1, strlen (clockBuff+2));
    h+=(clockBuff[0] - '0');
    memmove (clockBuff, clockBuff + 2, strlen (clockBuff+2));
    m = clockBuff[0] - '0';
    m*=10;
    memmove (clockBuff, clockBuff + 1, strlen (clockBuff+2));
    m+=(clockBuff[0] - '0');
    indx = 0;
  }

  //If new weather data is available, update weather variables from buffer.
  if (weatherProcess) {
    Serial.println("weatherProcess occured");
    weatherProcess = false;
    
    //Clear buffer of any previous weather condition overflow
    for (int i = indx; i > 0; i--) {
      if (weatherBuff[i] == '!') {
        weatherBuff[i] = '\0';
      }
    }
    indx = 0;  
    
    //For weather conditions with static images, draw new image.
    //For weather conditions with animations, set draw color for animation.
    if ( strcmp(weatherBuff,"@Clouds") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(GREY);
      drawClouds();
    } else if (strcmp(weatherBuff,"@Rain") == 0) {
      mylcd.Set_Draw_color(BLUE); 
    } else if (strcmp(weatherBuff,"@Snow") == 0) {
      mylcd.Set_Draw_color(RED); 
    } else if (strcmp(weatherBuff,"@Drizzle") == 0) {
      mylcd.Set_Draw_color(CYAN);
    } else if (strcmp(weatherBuff,"@Thunderstorm") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(GREY);
      drawThunderstorm();
      mylcd.Set_Draw_color(MAGENTA);
    } else if (strcmp(weatherBuff,"@Clear") == 0) {
      clearWeatherArtwork();
      drawClearSkies();
    } else {
      mylcd.Set_Draw_color(GREEN); 
    }
  }

  //If flash is toggled on, flash prudential lights and first image of weather conditions with animations.
  //If flash is not toggled on, erase prudential lights and use second image of weather condition animations.
  if (flash == true) {
  if ( strcmp(weatherBuff,"@Clouds") == 0) {
      mylcd.Set_Draw_color(GREY);
    } else if (strcmp(weatherBuff,"@Rain") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(BLUE);
      drawRainOne();
    } else if (strcmp(weatherBuff,"@Snow") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(WHITE); 
      drawSnowOne();
      mylcd.Set_Draw_color(RED); 
    } else if (strcmp(weatherBuff,"@Drizzle") == 0) {
      drawDrizzleOne();
      mylcd.Set_Draw_color(CYAN);
    } else if (strcmp(weatherBuff,"@Thunderstorm") == 0) {
      mylcd.Set_Draw_color(MAGENTA);
    } else if (strcmp(weatherBuff,"@Clear") == 0) {
      mylcd.Set_Draw_color(YELLOW);  
    } else {
      mylcd.Set_Draw_color(GREEN); 
    }
  drawPrudentialLights();
  } else {
  mylcd.Set_Draw_color(BLACK);
  drawPrudentialLights();
  if (strcmp(weatherBuff,"@Drizzle") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(CYAN); 
      drawDrizzleTwo();
  } else if (strcmp(weatherBuff,"@Rain") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(BLUE); 
      drawRainTwo();
  } else if (strcmp(weatherBuff,"@Snow") == 0) {
      clearWeatherArtwork();
      mylcd.Set_Draw_color(WHITE); 
      drawSnowTwo();
   }
  };

  //If 60,000 milliseconds have passed, increment time by 1 minute and reset counter.
  if (msCounter <= 0) 
  {
  m = m+1;
  if (m > 59) {
    m = 0;
    h = h+1;
  };
  if (h > 24) {
    h = 1;
  };
  if (h > 12) 
  {
    displayH = h - 12;
  }
  else {
    displayH = h;
  };
  if (displayH < 10) {
    mylcd.Print_Number_Int(0, 280, 50, 2, 0, 10);
    mylcd.Print_Number_Int(displayH, 364, 50, 2, 0, 10);
  } else {
  mylcd.Print_Number_Int(displayH, 280, 50, 2, 0, 10);
  };
  if (m < 10) {
    mylcd.Print_Number_Int(0, 280, 180, 2, 0, 10);
    mylcd.Print_Number_Int(m, 364, 180, 2, 0, 10);
  }
  else {
    mylcd.Print_Number_Int(m, 280, 180, 2, 0, 10);
  };
  msCounter = 60000 + msCounter;
  };

  //Toggle flash condition for Prudential lights and animations.
  if (flash == true) {
    flash = false;
  } else {
    flash = true;
  };
  
  //Calculate time spent in this loop iteration.
  int difference = millis()-now;
  if (difference < 0 || difference >= 32767) {
    difference = 220;
  };

  //Update millisecond counter and then delay one second.
  msCounter = msCounter - difference - 1001;
  delay(1000);
}

/********************
Define images or animations for all weather conditions
********************/

void clearWeatherArtwork()
{
  mylcd.Fill_Rect(125,20,95,76,BLACK);
  mylcd.Fill_Rect(20,20,89,54,BLACK);
}

void drawClouds()
{
  mylcd.Fill_Rect(140,60,63,20,GREY);
  mylcd.Fill_Circle(203, 68, 12);
  mylcd.Fill_Circle(140, 66, 14);
  mylcd.Fill_Circle(165, 50, 15);
  mylcd.Fill_Circle(186, 53, 10);
  mylcd.Fill_Rect(35,50,63,20,GREY);
  mylcd.Fill_Circle(98, 58, 12);
  mylcd.Fill_Circle(35, 56, 14);
  mylcd.Fill_Circle(60, 40, 15);
  mylcd.Fill_Circle(81, 43, 10);
}

void drawThunderstorm()
{
  mylcd.Fill_Rect(140,60,31,10,GREY);
  mylcd.Fill_Circle(171, 64, 6);
  mylcd.Fill_Circle(140, 63, 7);
  mylcd.Fill_Circle(152, 55, 8);
  mylcd.Fill_Circle(163, 56, 5);
  mylcd.Set_Draw_color(YELLOW);
  mylcd.Fill_Triangle(145,70,156,74,153,78);
  mylcd.Fill_Triangle(145,76,159,84,153,76);
  mylcd.Fill_Triangle(145,70,156,74,152,70);
  mylcd.Fill_Triangle(160,70,171,74,168,78);
  mylcd.Fill_Triangle(160,76,174,84,168,76);
  mylcd.Fill_Triangle(160,70,171,74,167,70);
  mylcd.Fill_Rect(35,50,31,10,GREY);
  mylcd.Set_Draw_color(GREY);
  mylcd.Fill_Circle(66, 54, 6);
  mylcd.Fill_Circle(35, 53, 7);
  mylcd.Fill_Circle(47, 45, 8);
  mylcd.Fill_Circle(58, 47, 5);
  mylcd.Set_Draw_color(YELLOW);
  mylcd.Fill_Triangle(40,60,51,64,48,68);
  mylcd.Fill_Triangle(40,66,54,74,48,66);
  mylcd.Fill_Triangle(40,60,51,64,47,60);
  mylcd.Fill_Triangle(55,60,66,64,63,68);
  mylcd.Fill_Triangle(55,66,69,74,63,66);
  mylcd.Fill_Triangle(55,60,66,64,62,60);
}

void drawClearSkies()
{
  if (h > 7 && h < 18) {
    mylcd.Set_Draw_color(YELLOW);
    mylcd.Fill_Circle(182, 58, 18); 
    mylcd.Draw_Line(182,40,182,20);
    mylcd.Draw_Line(200,58,220,58);
    mylcd.Draw_Line(164,58,144,58);
    mylcd.Draw_Line(182,76,182,96);
    mylcd.Draw_Line(170,46,158,34);
    mylcd.Draw_Line(170,70,158,82);
    mylcd.Draw_Line(194,46,206,34);
    mylcd.Draw_Line(194,70,206,82);
  } else {
    mylcd.Set_Draw_color(WHITE);
    mylcd.Fill_Circle(182, 58, 18);
    mylcd.Set_Draw_color(BLACK);
    mylcd.Fill_Circle(195, 58, 15);
  }
}

void drawRainOne()
{ 
  mylcd.Draw_Line(144,40,141,46);
  mylcd.Draw_Line(145,40,142,46);
  mylcd.Draw_Line(170,47,167,53);
  mylcd.Draw_Line(171,47,168,53);
  mylcd.Draw_Line(152,47,149,53);
  mylcd.Draw_Line(153,47,150,53);
  mylcd.Draw_Line(160,56,157,62);
  mylcd.Draw_Line(161,56,158,62);
  mylcd.Draw_Line(167,83,164,89);
  mylcd.Draw_Line(168,83,165,89);
  mylcd.Draw_Line(147,63,144,69);
  mylcd.Draw_Line(148,63,145,69);
  mylcd.Draw_Line(174,70,171,76);
  mylcd.Draw_Line(175,70,172,76);
  mylcd.Draw_Line(182,78,179,84);
  mylcd.Draw_Line(183,78,180,84);
  mylcd.Draw_Line(152,78,149,84);
  mylcd.Draw_Line(153,78,150,84);
  mylcd.Draw_Line(188,86,185,93);
  mylcd.Draw_Line(189,86,186,93);
  mylcd.Draw_Line(194,45,191,51);
  mylcd.Draw_Line(195,45,192,51);
  mylcd.Draw_Line(198,58,195,64);
  mylcd.Draw_Line(199,58,196,64);
  mylcd.Draw_Line(207,65,204,71);
  mylcd.Draw_Line(208,65,205,71);
  mylcd.Draw_Line(215,82,212,88);
  mylcd.Draw_Line(216,82,213,88);
  mylcd.Draw_Line(39,46,36,55);
  mylcd.Draw_Line(40,46,37,55);
  mylcd.Draw_Line(51,48,48,54);
  mylcd.Draw_Line(52,48,49,54);
  mylcd.Draw_Line(61,46,58,52);
  mylcd.Draw_Line(62,46,59,52);
  mylcd.Draw_Line(71,47,68,53);
  mylcd.Draw_Line(72,47,69,53);
  mylcd.Draw_Line(39,57,36,63);
  mylcd.Draw_Line(40,57,37,63);
  mylcd.Draw_Line(42,66,39,72);
  mylcd.Draw_Line(43,66,40,72);
  mylcd.Draw_Line(54,59,51,65);
  mylcd.Draw_Line(55,59,52,65);
  mylcd.Draw_Line(58,63,55,69);
  mylcd.Draw_Line(59,63,56,69);
  mylcd.Draw_Line(72,60,69,67);
  mylcd.Draw_Line(73,60,70,67);
  mylcd.Draw_Line(80,58,77,64);
  mylcd.Draw_Line(81,58,78,64);
  mylcd.Draw_Line(90,67,87,73);
  mylcd.Draw_Line(91,67,88,73); 
}

void drawRainTwo()
{
  mylcd.Draw_Line(150,43,147,49);
  mylcd.Draw_Line(151,43,148,49);
  mylcd.Draw_Line(176,50,173,56);
  mylcd.Draw_Line(177,50,174,56);
  mylcd.Draw_Line(158,50,155,56);
  mylcd.Draw_Line(159,50,156,56);
  mylcd.Draw_Line(166,59,163,65);
  mylcd.Draw_Line(167,59,164,65);
  mylcd.Draw_Line(173,86,170,92);
  mylcd.Draw_Line(174,86,171,92);
  mylcd.Draw_Line(153,66,150,72);
  mylcd.Draw_Line(154,66,151,72);
  mylcd.Draw_Line(180,73,177,79);
  mylcd.Draw_Line(181,73,178,79);
  mylcd.Draw_Line(188,81,185,87);
  mylcd.Draw_Line(189,81,186,87);
  mylcd.Draw_Line(158,81,155,87);
  mylcd.Draw_Line(159,81,156,87);
  mylcd.Draw_Line(194,89,191,95);
  mylcd.Draw_Line(195,89,192,95);
  mylcd.Draw_Line(200,48,197,54);
  mylcd.Draw_Line(201,48,198,54);
  mylcd.Draw_Line(205,61,202,67);
  mylcd.Draw_Line(206,61,203,67);
  mylcd.Draw_Line(213,68,210,74);
  mylcd.Draw_Line(214,68,211,74);
  mylcd.Draw_Line(218,85,215,91);
  mylcd.Draw_Line(219,85,216,91);
  mylcd.Draw_Line(39,43,36,49);
  mylcd.Draw_Line(40,43,37,49);
  mylcd.Draw_Line(51,45,48,51);
  mylcd.Draw_Line(52,45,49,51);
  mylcd.Draw_Line(61,43,58,49);
  mylcd.Draw_Line(62,43,59,49);
  mylcd.Draw_Line(71,44,68,50);
  mylcd.Draw_Line(72,44,69,50);
  mylcd.Draw_Line(39,54,36,60);
  mylcd.Draw_Line(40,54,37,60);
  mylcd.Draw_Line(42,63,39,69);
  mylcd.Draw_Line(43,63,40,69);
  mylcd.Draw_Line(54,56,51,62);
  mylcd.Draw_Line(55,56,52,62);
  mylcd.Draw_Line(58,60,55,66);
  mylcd.Draw_Line(59,60,56,66);
  mylcd.Draw_Line(72,57,69,63);
  mylcd.Draw_Line(73,57,70,63);
  mylcd.Draw_Line(80,55,77,61);
  mylcd.Draw_Line(81,55,78,61);
  mylcd.Draw_Line(90,65,87,71);
  mylcd.Draw_Line(91,65,88,71);
}

void drawSnowOne()
{
  mylcd.Fill_Circle(144, 43, 2);
  mylcd.Fill_Circle(156, 45, 2);
  mylcd.Fill_Circle(168, 44, 2);
  mylcd.Fill_Circle(182, 46, 2);
  mylcd.Fill_Circle(199, 43, 2);
  mylcd.Fill_Circle(148, 51, 2);
  mylcd.Fill_Circle(159, 53, 2);
  mylcd.Fill_Circle(173, 52, 2);
  mylcd.Fill_Circle(191, 51, 2);
  mylcd.Fill_Circle(143, 65, 2);
  mylcd.Fill_Circle(155, 67, 2);
  mylcd.Fill_Circle(166, 62, 2);
  mylcd.Fill_Circle(183, 66, 2);
  mylcd.Fill_Circle(197, 69, 2);
  mylcd.Fill_Circle(147, 78, 2);
  mylcd.Fill_Circle(160, 80, 2);
  mylcd.Fill_Circle(172, 79, 2);
  mylcd.Fill_Circle(187, 81, 2);
  mylcd.Fill_Circle(200, 77, 2);
  mylcd.Fill_Circle(40, 43, 2);
  mylcd.Fill_Circle(56, 45, 2);
  mylcd.Fill_Circle(68, 44, 2);
  mylcd.Fill_Circle(82, 46, 2);
  mylcd.Fill_Circle(48, 51, 2);
  mylcd.Fill_Circle(59, 53, 2);
  mylcd.Fill_Circle(73, 52, 2);
  mylcd.Fill_Circle(91, 51, 2);
  mylcd.Fill_Circle(43, 65, 2);
  mylcd.Fill_Circle(55, 67, 2);
  mylcd.Fill_Circle(66, 62, 2);
  mylcd.Fill_Circle(83, 66, 2);
}

void drawSnowTwo()
{
  mylcd.Fill_Circle(143, 50, 2);
  mylcd.Fill_Circle(154, 52, 2);
  mylcd.Fill_Circle(166, 51, 2);
  mylcd.Fill_Circle(180, 53, 2);
  mylcd.Fill_Circle(197, 50, 2);
  mylcd.Fill_Circle(146, 58, 2);
  mylcd.Fill_Circle(157, 60, 2);
  mylcd.Fill_Circle(171, 59, 2);
  mylcd.Fill_Circle(189, 58, 2);
  mylcd.Fill_Circle(143, 72, 2);
  mylcd.Fill_Circle(153, 73, 2);
  mylcd.Fill_Circle(164, 69, 2);
  mylcd.Fill_Circle(181, 73, 2);
  mylcd.Fill_Circle(195, 76, 2);
  mylcd.Fill_Circle(145, 85, 2);
  mylcd.Fill_Circle(158, 87, 2);
  mylcd.Fill_Circle(170, 76, 2);
  mylcd.Fill_Circle(185, 88, 2);
  mylcd.Fill_Circle(198, 84, 2);
  mylcd.Fill_Circle(40, 50, 2);
  mylcd.Fill_Circle(56, 52, 2);
  mylcd.Fill_Circle(68, 51, 2);
  mylcd.Fill_Circle(82, 53, 2);
  mylcd.Fill_Circle(48, 58, 2);
  mylcd.Fill_Circle(59, 60, 2);
  mylcd.Fill_Circle(73, 59, 2);
  mylcd.Fill_Circle(91, 58, 2);
  mylcd.Fill_Circle(43, 69, 2);
  mylcd.Fill_Circle(55, 71, 2);
  mylcd.Fill_Circle(66, 69, 2);
  mylcd.Fill_Circle(83, 70, 2);
}

void drawDrizzleOne()
{
  mylcd.Fill_Rect(140,40,80,56,BLACK);
  mylcd.Fill_Rect(35,40,63,34,BLACK);
  mylcd.Set_Draw_color(CYAN); 
  mylcd.Draw_Line(170,47,167,53);
  mylcd.Draw_Line(171,47,168,53);
  mylcd.Draw_Line(160,56,157,62);
  mylcd.Draw_Line(161,56,158,62);
  mylcd.Draw_Line(167,83,164,89);
  mylcd.Draw_Line(168,83,165,89);
  mylcd.Draw_Line(182,78,179,84);
  mylcd.Draw_Line(183,78,180,84);
  mylcd.Draw_Line(152,78,149,84);
  mylcd.Draw_Line(153,78,150,84);
  mylcd.Draw_Line(188,86,185,93);
  mylcd.Draw_Line(189,86,186,93);
  mylcd.Draw_Line(198,58,195,64);
  mylcd.Draw_Line(199,58,196,64);
  mylcd.Draw_Line(207,65,204,71);
  mylcd.Draw_Line(208,65,205,71);
  mylcd.Draw_Line(215,82,212,88);
  mylcd.Draw_Line(216,82,213,88);
  mylcd.Draw_Line(61,42,58,48);
  mylcd.Draw_Line(62,42,59,48);
  mylcd.Draw_Line(71,40,68,46);
  mylcd.Draw_Line(72,40,69,46);
  mylcd.Draw_Line(42,60,39,66);
  mylcd.Draw_Line(43,60,40,66);
  mylcd.Draw_Line(58,57,55,63);
  mylcd.Draw_Line(59,57,56,63);
  mylcd.Draw_Line(72,54,69,60);
  mylcd.Draw_Line(73,54,70,60);
  mylcd.Draw_Line(90,62,87,68);
  mylcd.Draw_Line(91,62,88,68);
}

void drawDrizzleTwo()
{
  mylcd.Draw_Line(166,59,163,65);
  mylcd.Draw_Line(167,59,164,65);
  mylcd.Draw_Line(173,86,170,92);
  mylcd.Draw_Line(174,86,171,92);
  mylcd.Draw_Line(153,66,150,72);
  mylcd.Draw_Line(154,66,151,72);
  mylcd.Draw_Line(180,73,177,79);
  mylcd.Draw_Line(181,73,178,79);
  mylcd.Draw_Line(188,81,185,87);
  mylcd.Draw_Line(189,81,186,87);
  mylcd.Draw_Line(158,81,155,87);
  mylcd.Draw_Line(159,81,156,87);
  mylcd.Draw_Line(194,89,191,95);
  mylcd.Draw_Line(195,89,192,95);
  mylcd.Draw_Line(205,61,202,67);
  mylcd.Draw_Line(206,61,203,67);
  mylcd.Draw_Line(218,85,215,91);
  mylcd.Draw_Line(219,85,216,91);
  mylcd.Draw_Line(55,46,52,52);
  mylcd.Draw_Line(56,46,53,52);
  mylcd.Draw_Line(71,47,68,53);
  mylcd.Draw_Line(72,47,69,53);
  mylcd.Draw_Line(42,66,39,72);
  mylcd.Draw_Line(43,66,40,72);
  mylcd.Draw_Line(58,63,55,69);
  mylcd.Draw_Line(59,63,56,69);
  mylcd.Draw_Line(75,58,72,64);
  mylcd.Draw_Line(76,58,73,64);
  mylcd.Draw_Line(90,67,87,73);
  mylcd.Draw_Line(91,67,88,73);
}

void drawPrudentialLights()
{
  mylcd.Fill_Circle(69, 124, 4);
  mylcd.Fill_Circle(83, 119, 4);
  mylcd.Fill_Circle(106, 115, 4);
  mylcd.Fill_Circle(131, 117, 4);
  mylcd.Fill_Circle(156, 118, 4);
  mylcd.Fill_Circle(113, 7, 4);
}
