/*
 * File:   main.c
 * Author: Phil Glazzard
 * MPLABX IDE v6.00
 * XC8 v2.36
 * Optimisations 0
 *
 * Created on 11 December 2022, 18:23
 * 
 * * *                  16f1459
 *                  ---------
 *   +5 Volts    1 |Vdd      | 20 0 Volts
                 2 |RA5   RA0| 19   
 *               3 |RA4   RA1| 18   
      MCLR       4 |RA3      | 17  
                 5 |RC5   RC0| 16 
 DIAGNOSTIC LED  6 |RC4   RC1| 15  
 *               7 |RC3   RC2| 14  ADC AN13 input = 0 - 1.024V
 *               8 |RC6   RB4| 13  
                 9 |RC7   RB5| 12  
 *    TX        10 |RB7   RB6| 11  
 *                  ---------
 * 
 * 
 */


#include <xc.h>
#include "config.h"
#include "constants.h"

#include "configOsc.h"
#include "configPins.h"
#include "configSPI.h"
#include "configEUSART.h"
#include "configBaud.h"
#include "configPCD8544.h"

#include "writeCommand.h"
#include "writeData.h"
#include "select0To47.h"
#include "configADC.h"
//#include "pid.h"
#include <stdio.h>
uint8_t bgRow, bgCol;

 
 uint8_t backGrnd[6][84] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x30,0xc8,0x88,0xff,0x00,0xff,0x99,0xa9,0x66},
{0,0,0,0,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0,0,0,0},
{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};


 
 
void main(void) 
{
   
  
    int16_t  opposite, adjacent, radius, ycoord, xcoord;
 
    const uint8_t Kp = 1;
   /*****struct variable of type pageToRow*****/
    pageToRow ypixel;
      
    configOsc();
    configPins();
    configSPI();
    configADC();
    configEUSART();
    configBaud();
    configPCD8544();
 
   while(1)
   {
      
        ADCON0bits.GO_nDONE = HI;       //A/D conversion cycle in progress. Setting this bit starts an A/D conversion cycle.
                                    //This bit is automatically cleared by hardware when the A/D conversion has completed.
        while(ADCON0bits.GO_nDONE == HI)
        {
            ;   // wait for GO_nDONE to clear
        }
        adcVal = (int16_t)(ADRESH<<8 | ADRESL);// + 0x2d;  // 0x2d = 45 deg, which is the offset needed to display 45 degrees with 0V input
        //adcVal = (int16_t)(ADRESH<<8 | ADRESL);
        /****printing of adcVal ONLY when adcVal has changed from previous value*****/
         // adcVal ranges from 45 (105mV) to 135(290mV), voltage reference is 2.048V
                            // adcVal is the angle of the needle from the horizontal axis
       
        adcVal = Kp * adcVal;
        /******needle clamping at lower and upper ranges*/
        if(adcVal <=45)
        {
            adcVal = 45;
        }
        
         if(adcVal >=135)
        {
            adcVal = 135;
        }
       
       // printf("%d\n", adcVal);
       
        
       
     
         adcVal = (adcVal)+1;
         for(radius = 0; radius <=58; radius++)
        {
            opposite = radius * sineLookUp[adcVal];     // x = opposite, adcVal is the angle of the needle from the horizontal axis
            adjacent = radius * cosineLookUp[adcVal];   // y = adjacent
         
            xcoord = (opposite>>8) + 42;
            ycoord = (adjacent>>8)  -9;
         
            select0To47(ycoord, &ypixel);// pass address of pageToRow ypixel to pagaToRow *ptr
            writeCommand(0x22); //horizontal addressing
             if(ycoord >=0)
             {
                backGrnd[ypixel.page][xcoord] = (ypixel.byte)|backGrnd[ypixel.page][xcoord];    //write needle position into backGrnd[][]]
             }
        }
       // __delay_ms(50);
         
         /****display needle & graticule on LCD display*****/
        for(bgRow = 0; bgRow <6; bgRow++)
    {
        for(bgCol = 0; bgCol <84; bgCol++)
        {
           writeCommand(0x20); //horizontal addressing
           writeData(backGrnd[bgRow][bgCol]);   // draw background and needle
        }           
     }     
    for(bgRow = 0; bgRow <6; bgRow++)
    {          
        if(bgRow == 1)
        {
            for(bgCol = 0; bgCol <84; bgCol++)
            {         
                backGrnd[bgRow][bgCol] = 0x80;   // draw graticule
            }  
        }
        else if(bgRow == 2)
        {
            for(bgCol = 0; bgCol <84; bgCol++)
            {
                if(bgCol%8 == 1)
                {
                    backGrnd[bgRow][bgCol] = 0x03;   // draw graticule
                }
                else
                {
                    backGrnd[bgRow][bgCol] = 0x00;
                }
            }    
        } 
        else
        {
            for(bgCol = 0; bgCol <84; bgCol++) 
            {
                backGrnd[bgRow][bgCol] = 0;   // clear pages 0 | 3 | 4 | 5
            }  
        } 
     } 
        RC4 = ~RC4;    
         
}
   
}   
