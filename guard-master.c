// *********************************************************************
// *                           E01 - TX                                *
// *                        RF transmitter                             *
// *********************************************************************
// Example of simple RF transmission.
//
// Intended for:
//    HW: IQRF TR modules TR-52B, TR-53B and compatibles
//        DK-EVAL-03, DK-EVAL-04 development kit
//    OS: v3.00 or higher
//
// Description:
//    This example shows how to make a simple transmitter. It can be
//    tested with the E02-RX example.
//    - RF message is sent (and red LED flashes once) whenever the
//      pushbutton on the kit is pressed.
//    - Content of the messages = Application data.
//      (It is the 32 B array in EEPROM dedicated to user information
//      about the application - see the IQRF OS manual.
//      The "01234567890123456789012345678901" data array is used here.
//    - To lower power consumption the module utilizes the Sleep mode
//      and Wake-up on pin change for response to the button.
//      Wake-up on pin change is under user's control
//      and has to be enabled by the user when needed.
//    - The watchdog is repeatedly cleared.
//      If omitted:
//      - The module is regularly reset from the watchdog overflow
//        which leads to additional transmission whenever the
//        watchdog overflows while the button is just pressed.
//      - Transmission aborted if watchdog overflows while transmitting.
//
//
// File:    E01-TX.c
// Version: v1.00                                   Revision: 08/12/2010
//
// Revision history:
//     v1.00: 08/12/2010  First release
//
// *********************************************************************
#include "includes/template-basic.h"

// *********************************************************************

#define SPIDEBUG
void take_picture(void);

void APPLICATION()             	// Obligatory assigning - see E00-START
{
	uns8 picture_taken = 0;
	clearBufferRF();
	
	#ifdef SPIDEBUG
	enableSPI();	
	#else //not SPIDEBUG
	disableSPI();
	TRISC.3 = 0;	//port direction of portc.3 (sam as _SCK) for camera trigger
	_SCK = 0;
	#endif //SPIDEBUG

    while (1)                  	// Main cycle (perpetually repeated)
    {
        clrwdt();              	// Clear watchdog
		/*
		SWDTEN = 1;            	// Minimize consumption (switch off WDT)
        iqrfSleep();           	// Put the module into Sleep mode --wakeup C5(SS), WDT,poweron/off
								// Requested sequence for wake-up:
		pulseLEDR();
		pulseLEDG();
		waitDelay(25);
        */
		//do not fall asleep, else RX can't work
		/*
		GIE = 0;               	//   disable all interrupts
        RBIE = 1;              	//   enable wake-up on pin change

        SWDTEN = 0;            	// Minimize consumption (switch off WDT)
        iqrfSleep();           	// Put the module into Sleep mode --wakeup C5(SS), WDT,poweron/off
        RBIF = 0;              	// Requested: clear flag
		*/
		
		if (RFRXpacket())           // If anything was received		
        {
			pulseLEDG();            //   LED indication
			
			#ifdef SPIDEBUG
            copyBufferRF2COM();     //   Copy received RF data  from bufferRF to bufferCOM
            startSPI(DLEN);         //     and send it via SPI
			#endif
			
			//alarm recevied
			if (bufferRF[0] == 'A')
			{
				if (picture_taken) //take picture, then send ACK to slave
				{
					PIN = 0;
					bufferRF[3] = bufferRF[1]; //copy NUM OF UNIT
					bufferRF[0] = 'A';
					bufferRF[1] = 'C';
					bufferRF[2] = 'K';
					//bufferRF[3] = --already filled
					bufferRF[4] = 'A'; //slave will ring alarm
					
					DLEN = 5;         	//      Setup RF packet length
					RFTXpacket();      	//      Transmit the message
					waitDelay(25);     	//      and wait 250ms (25*10ms)
					pulseLEDR();
					picture_taken = 0;
				}else
				{					
					take_picture();					
					picture_taken = 1;
				}
			}
        }		
    }                          	// End of main cycle
}

void take_picture(void)
{
	uns8 i,j;
	
	#ifndef SPIDEBUG
	_SCK = 1;           // turn on camera and take picture
	#endif
	
	for(i=0; i<2; i++) //wait 5 seconds??? need to be tested
	{
		waitDelay(25);     	//      and wait 250ms (25*10ms)
		clrwdt();              	// Clear watchdog
	}
	
	#ifndef SPIDEBUG
	_SCK = 0;           // turn off camera
	#endif	
	pulsingLEDR();      // 4x LED flash (ALARM indication)
	waitDelay(95);
	stopLEDR();
}

// *********************************************************************

#pragma packedCdataStrings 0    // Store the string unpacked
                                //   (one byte in one location)

//                             00000000001111111111222222222233
#pragma cdata[__EEAPPINFO] =  "Commands: 'C'heck status        "
                                // 32 B to be stored to the
                                // Application Info array in EEPROM
// *********************************************************************

