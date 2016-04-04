/*
    Microchip ZigBee2006 Residential Stack

    Demo Coordinator

    This demonstration shows how a ZigBee coordinator can be set up.  
    This demo allows the PICDEM Z/Explorer 16 Demostration Board to act as ZigBee
    protocol Coordinator. It is designed to interact with other ZigBee protocol 
    devices - Routers and End Devices.
    
    Switch and LED functionality are as follows:

    RB4/RD6:  Adds a node to, or removes a node from Group 4.  
    RA0/D10:  Used to indicate if a node is a member of Group 4 (Lit � yes; unlit � no) 

    At startup the devices do not belong to a group, and the lit LEDs just indicate
    they are on the network.
    
    RB5/RD7:   Is used to send a messages to nodes in Group 4.    
               The actual message is a request for all the Group 4 nodes to end 
               back to the requester 10 bytes.
    RA1/D09:   Toggles to indicate that the receiving node is in Group 4 
               and received requests for data.  


    NOTE: To speed network formation, ALLOWED_CHANNELS has been set to
    channel 21 only.  
    Please consult the PICDEM Z Zigbee2006 Residential StackQTGuide.pdf for how to 
    run this sample application.

 *********************************************************************
 * FileName:        Coordinator.c
 * Dependencies:
 * Processor:       PIC18F/PIC24F
 * Complier:        MCC18 v3.20 or higher
 * Complier:        MCC30 v3.10 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (c) 2004-2008 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, copy and distribute Software 
 * only when embedded on a Microchip microcontroller or digital signal 
 * controller and used with a Microchip radio frequency transceiver, which 
 * are integrated into your product or third party product (pursuant to the 
 * sublicense terms in the accompanying license agreement).  You may NOT 
 * modify or create derivative works of the Software.  
 *
 * If you intend to use this Software in the development of a product for 
 * sale, you must be a member of the ZigBee Alliance.  For more information, 
 * go to www.zigbee.org.
 *
 * You should refer to the license agreement accompanying this Software for 
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY 
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR 
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED 
 * UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF 
 * WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR 
 * EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, 
 * PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY 
 * THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER 
 * SIMILAR COSTS.
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO/yy             01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DPL                  08/01/08 Microchip ZigBee-2006 Stack v2.0-2.6
 ********************************************************************/

#include "zAPL.h"
#include "zNVM.h"
#ifdef I_SUPPORT_SECURITY
	#include "zSecurity.h"
	extern NETWORK_KEY_INFO    currentNetworkKeyInfo;
	#ifdef USE_EXTERNAL_NVM
		extern NETWORK_KEY_INFO plainSecurityKey[2];
		
		extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
	#endif
#endif

// If you are going to send data to a terminal, include this file.
#include "console.h"

//******************************************************************************
// Configuration Bits
//******************************************************************************
#if defined(MCHP_C18) 
    #if defined(__18F4620)
    	#pragma romdata CONFIG1H = 0x300001
	    const rom unsigned char config1H = 0b00000110;      // HSPLL oscillator

    	#pragma romdata CONFIG2L = 0x300002
	    const rom unsigned char config2L = 0b00011111;      // Brown-out Reset Enabled in hardware @ 2.0V, PWRTEN disabled

    	#pragma romdata CONFIG2H = 0x300003
	    const rom unsigned char config2H = 0b00010010;      // HW WD disabled, 1:512 prescaler

    	#pragma romdata CONFIG3H = 0x300005
	    const rom unsigned char config3H = 0b10000000;      // PORTB digital on RESET

    	#pragma romdata CONFIG4L = 0x300006
	    const rom unsigned char config4L = 0b10000001;      // DEBUG disabled,
    	                                                    // XINST disabled
        	                                                // LVP disabled
            	                                            // STVREN enabled
    #elif defined(__18F87J10)
        #pragma romdata CONFIG1L = 0x1FFF8
        const rom unsigned char config1L = 0b10111110;      
    
        #pragma romdata CONFIG1H = 0x1FFF9
        const rom unsigned char config1H = 0b11111111;      
    
        #pragma romdata CONFIG2L = 0x1FFFA
        const rom unsigned char config2L = 0b11111101;      
    
        #pragma romdata CONFIG2H = 0x1FFFB
        const rom unsigned char config2H = 0b11111010;      
    
        #pragma romdata CONFIG3L = 0x1FFFC
        const rom unsigned char config3L = 0b11111111;

        #pragma romdata CONFIG3H = 0x1FFFD
        const rom unsigned char config3H = 0b11111111;
        
    #endif

    #pragma romdata
#elif defined(__PIC24F__)
	// Explorer 16 board
	_CONFIG2(FNOSC_PRI & POSCMOD_XT)	// Primary XT OSC with 4X PLL
	_CONFIG1(JTAGEN_OFF & FWDTEN_OFF)	// JTAG off, watchdog timer off
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
	// Explorer 16 board
	_FOSCSEL(FNOSC_PRI)				// primary osc
	_FOSC(OSCIOFNC_OFF & POSCMD_XT)	// XT Osc
	_FWDT(FWDTEN_OFF)				// Disable Watchdog timer
	// JTAG should be disabled as well
#elif defined(__dsPIC30F__)
	// dsPICDEM 1.1 board
	_FOSC(XT_PLL16)		// XT Osc + 16X PLL
	_FWDT(WDT_OFF)		// Disable Watchdog timer
	_FBORPOR(MCLR_EN & PBOR_OFF & PWRT_OFF)
#else
    #error Other compilers are not yet supported.
#endif		


//******************************************************************************
// Compilation Configuration
//******************************************************************************


//******************************************************************************
// Constants
//******************************************************************************

#if defined(__C30__)
	#define PB_LEFT_SWITCH				PORTDbits.RD6
	#define PB_RIGHT_SWITCH			    PORTDbits.RD7
	#define GROUP_INDICATION			LATAbits.LATA6
	#define MESSAGE_INDICATION			LATAbits.LATA7
#else
	#define PB_LEFT_SWITCH            	PORTBbits.RB5
	#define PB_RIGHT_SWITCH             PORTBbits.RB4
	#if defined(__18F4620)
		#define GROUP_INDICATION        LATAbits.LATA0
		#define MESSAGE_INDICATION      LATAbits.LATA1
	#else
		#define GROUP_INDICATION	    LATDbits.LATD0
		#define MESSAGE_INDICATION		LATDbits.LATD1
	#endif
#endif

#define DEBOUNCE_TIME 0x00008FF


/*****************************************************************************/
/* Defines:                                                                  */
/*****************************************************************************/
// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4
// The number of 32 bit words in a key.
#define Nk 4
// Key length in bytes [128 bit]
#define KEYLEN 16
// The number of rounds in AES Cipher.
#define Nr 10

#define Multiply(x, y)                          \
(  ((y & 1) * x) ^                              \
((y>>1 & 1) * xtime(x)) ^                       \
((y>>2 & 1) * xtime(xtime(x))) ^                \
((y>>3 & 1) * xtime(xtime(xtime(x)))) ^         \
((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))   \


/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/

// state - array holding the intermediate results during decryption.
typedef unsigned char state_t[4][4];
static state_t* state;

// The array that stores the round keys.
static unsigned char RoundKey[176];

// The Key input to the AES Program
static const unsigned char* Key;

// Initial Vector used only for CBC mode
static unsigned char* Iv;


// The lookup-tables are marked const so they can be placed in read-only storage instead of RAM
// The numbers below can be computed dynamically trading ROM for RAM -
// This can be useful in (embedded) bootloader applications, where ROM is often limited.
static const unsigned char sbox[256] =   {
    //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const unsigned char rsbox[256] =
{ 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };


// The round constant word array, Rcon[i], contains the values given by
// x to th e power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
// Note that i starts at 1, not 0).
static const unsigned char Rcon[255] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
    0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
    0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
    0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
    0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
    0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
    0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
    0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
    0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
    0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
    0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
    0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
    0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
    0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
    0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb  };

/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/

void *memsettest(void *src, int c, unsigned int count)
{
    //assert(src!=NULL);
    char *tmpsrc=(char*)src;
    while(count--)
        *tmpsrc++ =(char)c;
    return src;
}


static unsigned char getSBoxValue(unsigned char num)
{
    return sbox[num];
}

static unsigned char getSBoxInvert(unsigned char num)
{
    return rsbox[num];
}

// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states.
static void KeyExpansion(void)
{
    unsigned int i, j, k;
    unsigned char tempa[4]; // Used for the column/row operations
    
    // The first round key is the key itself.
    for(i = 0; i < Nk; ++i)
    {
        RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
        RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
        RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
        RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
    }
    
    // All other round keys are found from the previous round keys.
    for(; (i < (Nb * (Nr + 1))); ++i)
    {
        for(j = 0; j < 4; ++j)
        {
            tempa[j]=RoundKey[(i-1) * 4 + j];
        }
        if (i % Nk == 0)
        {
            // This function rotates the 4 bytes in a word to the left once.
            // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]
            
            // Function RotWord()
            {
                k = tempa[0];
                tempa[0] = tempa[1];
                tempa[1] = tempa[2];
                tempa[2] = tempa[3];
                tempa[3] = k;
            }
            
            // SubWord() is a function that takes a four-byte input word and
            // applies the S-box to each of the four bytes to produce an output word.
            
            // Function Subword()
            {
                tempa[0] = getSBoxValue(tempa[0]);
                tempa[1] = getSBoxValue(tempa[1]);
                tempa[2] = getSBoxValue(tempa[2]);
                tempa[3] = getSBoxValue(tempa[3]);
            }
            
            tempa[0] =  tempa[0] ^ Rcon[i/Nk];
        }
        else if (Nk > 6 && i % Nk == 4)
        {
            // Function Subword()
            {
                tempa[0] = getSBoxValue(tempa[0]);
                tempa[1] = getSBoxValue(tempa[1]);
                tempa[2] = getSBoxValue(tempa[2]);
                tempa[3] = getSBoxValue(tempa[3]);
            }
        }
        RoundKey[i * 4 + 0] = RoundKey[(i - Nk) * 4 + 0] ^ tempa[0];
        RoundKey[i * 4 + 1] = RoundKey[(i - Nk) * 4 + 1] ^ tempa[1];
        RoundKey[i * 4 + 2] = RoundKey[(i - Nk) * 4 + 2] ^ tempa[2];
        RoundKey[i * 4 + 3] = RoundKey[(i - Nk) * 4 + 3] ^ tempa[3];
    }
}

// This function adds the round key to state.
// The round key is added to the state by an XOR function.
static void AddRoundKey(unsigned char round)
{
    unsigned char i,j;
    for(i=0;i<4;++i)
    {
        for(j = 0; j < 4; ++j)
        {
            (*state)[i][j] ^= RoundKey[round * Nb * 4 + i * Nb + j];
        }
    }
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void SubBytes(void)
{
    unsigned char i, j;
    for(i = 0; i < 4; ++i)
    {
        for(j = 0; j < 4; ++j)
        {
            (*state)[j][i] = getSBoxValue((*state)[j][i]);
        }
    }
}

// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
static void ShiftRows(void)
{
    unsigned char temp;
    
    // Rotate first row 1 columns to left
    temp           = (*state)[0][1];
    (*state)[0][1] = (*state)[1][1];
    (*state)[1][1] = (*state)[2][1];
    (*state)[2][1] = (*state)[3][1];
    (*state)[3][1] = temp;
    
    // Rotate second row 2 columns to left
    temp           = (*state)[0][2];
    (*state)[0][2] = (*state)[2][2];
    (*state)[2][2] = temp;
    
    temp       = (*state)[1][2];
    (*state)[1][2] = (*state)[3][2];
    (*state)[3][2] = temp;
    
    // Rotate third row 3 columns to left
    temp       = (*state)[0][3];
    (*state)[0][3] = (*state)[3][3];
    (*state)[3][3] = (*state)[2][3];
    (*state)[2][3] = (*state)[1][3];
    (*state)[1][3] = temp;
}

static unsigned char xtime(unsigned char x)
{
    return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

// MixColumns function mixes the columns of the state matrix
static void MixColumns(void)
{
    unsigned char i;
    unsigned char Tmp,Tm,t;
    for(i = 0; i < 4; ++i)
    {
        t   = (*state)[i][0];
        Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
        Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
        Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
        Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
        Tm  = (*state)[i][3] ^ t ;        Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
    }
}



// MixColumns function mixes the columns of the state matrix.
// The method used to multiply may be difficult to understand for the inexperienced.
// Please use the references to gain more information.
static void InvMixColumns(void)
{
    int i;
    unsigned char a,b,c,d;
    for(i=0;i<4;++i)
    {
        a = (*state)[i][0];
        b = (*state)[i][1];
        c = (*state)[i][2];
        d = (*state)[i][3];
        
        (*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
        (*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
        (*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
        (*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
    }
}


// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
static void InvSubBytes(void)
{
    unsigned char i,j;
    for(i=0;i<4;++i)
    {
        for(j=0;j<4;++j)
        {
            (*state)[j][i] = getSBoxInvert((*state)[j][i]);
        }
    }
}

static void InvShiftRows(void)
{
    unsigned char temp;
    
    // Rotate first row 1 columns to right
    temp=(*state)[3][1];
    (*state)[3][1]=(*state)[2][1];
    (*state)[2][1]=(*state)[1][1];
    (*state)[1][1]=(*state)[0][1];
    (*state)[0][1]=temp;
    
    // Rotate second row 2 columns to right
    temp=(*state)[0][2];
    (*state)[0][2]=(*state)[2][2];
    (*state)[2][2]=temp;
    
    temp=(*state)[1][2];
    (*state)[1][2]=(*state)[3][2];
    (*state)[3][2]=temp;
    
    // Rotate third row 3 columns to right
    temp=(*state)[0][3];
    (*state)[0][3]=(*state)[1][3];
    (*state)[1][3]=(*state)[2][3];
    (*state)[2][3]=(*state)[3][3];
    (*state)[3][3]=temp;
}


// Cipher is the main function that encrypts the PlainText.
static void Cipher(void)
{
    unsigned char round = 0;
    
    // Add the First round key to the state before starting the rounds.
    AddRoundKey(0);
    
    // There will be Nr rounds.
    // The first Nr-1 rounds are identical.
    // These Nr-1 rounds are executed in the loop below.
    for(round = 1; round < Nr; ++round)
    {
        SubBytes();
        ShiftRows();
        MixColumns();
        AddRoundKey(round);
    }
    
    // The last round is given below.
    // The MixColumns function is not here in the last round.
    SubBytes();
    ShiftRows();
    AddRoundKey(Nr);
}

static void InvCipher(void)
{
    unsigned char round=0;
    
    // Add the First round key to the state before starting the rounds.
    AddRoundKey(Nr);
    
    // There will be Nr rounds.
    // The first Nr-1 rounds are identical.
    // These Nr-1 rounds are executed in the loop below.
    for(round=Nr-1;round>0;round--)
    {
        InvShiftRows();
        InvSubBytes();
        AddRoundKey(round);
        InvMixColumns();
    }
    
    // The last round is given below.
    // The MixColumns function is not here in the last round.
    InvShiftRows();
    InvSubBytes();
    AddRoundKey(0);
}

static void BlockCopy(unsigned char* output, unsigned char* input)
{
    unsigned char i;
    for (i=0;i<KEYLEN;++i)
    {
        output[i] = input[i];
    }
}


static void XorWithIv(unsigned char* buf)
{
    unsigned char i;
    for(i = 0; i < KEYLEN; ++i)
    {
        buf[i] ^= Iv[i];
    }
}


/*****************************************************************************/
/* Cryptography:                                                             */
/*****************************************************************************/
void AES128_CBC_encrypt_buffer(unsigned char* output, unsigned char* input, unsigned int length, const unsigned char* key, const unsigned char* iv)
{
    unsigned int i;
    unsigned char remainders = length % KEYLEN; /* Remaining bytes in the last non-full block */
    
    BlockCopy(output, input);
    state = (state_t*)output;
    
    // Skip the key expansion if key is passed as 0
    if(0 != key)
    {
        Key = key;
        KeyExpansion();
    }
    
    if(iv != 0)
    {
        Iv = (unsigned char*)iv;
    }
    
    for(i = 0; i < length; i += KEYLEN)
    {
        XorWithIv(input);
        BlockCopy(output, input);
        state = (state_t*)output;
        Cipher();
        Iv = output;
        input += KEYLEN;
        output += KEYLEN;
    }
    
    if(remainders)
    {
        BlockCopy(output, input);
        memsettest(output + remainders, 0, KEYLEN - remainders); /* add 0-padding */
        state = (state_t*)output;
        Cipher();
    }
}

void AES128_CBC_decrypt_buffer(unsigned char* output, unsigned char* input, unsigned int length, const unsigned char* key, const unsigned char* iv)
{
    unsigned int i;
    unsigned char remainders = length % KEYLEN; /* Remaining bytes in the last non-full block */
    
    BlockCopy(output, input);
    state = (state_t*)output;
    
    // Skip the key expansion if key is passed as 0
    if(0 != key)
    {
        Key = key;
        KeyExpansion();
    }
    
    // If iv is passed as 0, we continue to encrypt without re-setting the Iv
    if(iv != 0)
    {
        Iv = (unsigned char*)iv;
    }
    
    for(i = 0; i < length; i += KEYLEN)
    {
        BlockCopy(output, input);
        state = (state_t*)output;
        InvCipher();
        XorWithIv(output);
        Iv = input;
        input += KEYLEN;
        output += KEYLEN;
    }
    
    if(remainders)
    {
        BlockCopy(output, input);
        memsettest(output+remainders, 0, KEYLEN - remainders); /* add 0-padding */
        state = (state_t*)output;
        InvCipher();
    }
}


/*****************************************************************************/
/* Testing:                                                                  */
/*****************************************************************************/
// print string to hex
static void printStringToHex(unsigned char* str)
{
    unsigned char i;
    for(i = 0; i < 16; i++){
        //printf("%.2x", str[i]);
		ConsolePut(str[i]);
    }
    printf("\n");
}

unsigned char bufferEncrypt[16];
unsigned char encryptArrayInput[16] = {};

static void test_encrypt_cbc(unsigned char* in)
{
    unsigned char key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    unsigned char iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    //unsigned char inside[]  = {0x68,0x65,0x6c,0x6c,0x6f,0x77,0x6f,0x72,0x6c,0x64,0x61,0x62,0x63,0x64,0x65,0x66};
    unsigned char out[] = { 0x84,0x2a,0x27,0x04,0xa3,0x74,0xc2,0xf9,0xff,0xc1,0xba,0xde,0x78,0x5a,0x68,0xb7 };
    
    //unsigned char bufferEncrypt[16];
    
    AES128_CBC_encrypt_buffer(bufferEncrypt, encryptArrayInput, 16, key, iv);
    
    
    printf("\r\nCBC encrypt: ");
    /*
    if(0 == strncmp((char*) out, (char*) bufferEncrypt, 16))
    {
        printf("SUCCESS!\r\n");
    }
    else
    {
        printf("FAILURE!\r\n");
    }
	*/    

    unsigned char i;
    printf("\r\nIn function input data: ");
    for(i = 0; i < 1; ++i)
    {
        printStringToHex(in + (i*16));
    }
    printf("\r\n");
    
    printf("\r\nIn function encrypted data: ");
    for(i = 0; i < 1; ++i)
    {
        printStringToHex(bufferEncrypt + (i*16));
    }
    printf("\r\n");
    
    
//    return bufferEncrypt;
    
}

unsigned char bufferDecrypt[16];

static void test_decrypt_cbc(unsigned char* in)
{
    unsigned char key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    unsigned char iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    //unsigned char inside[]  = { 0x84,0x2a,0x27,0x04,0xa3,0x74,0xc2,0xf9,0xff,0xc1,0xba,0xde,0x78,0x5a,0x68,0xb7 };
    unsigned char out[] = { 0x68,0x65,0x6c,0x6c,0x6f,0x77,0x6f,0x72,0x6c,0x64,0x61,0x62,0x63,0x64,0x65,0x66 };

//    unsigned char bufferDecrypt[16];
    
    AES128_CBC_decrypt_buffer(bufferDecrypt, bufferEncrypt,  16, key, iv);
    
    printf("\r\nCBC decrypt: ");
    /*
    if(0 == strncmp((char*) out, (char*) bufferDecrypt, 16))
    {
        printf("SUCCESS!\r\n");
    }
    else
    {
        printf("FAILURE!\r\n");
    }
    */
    
    unsigned char i;
    printf("\r\nIn function input data: ");
    for(i = 0; i < 1; ++i)
    {
        printStringToHex(in + (i*16));
    }
    printf("\r\n");
    
    printf("\r\nIn function decrypted data: ");
    for(i = 0; i < 1; ++i)
    {
        printStringToHex(bufferDecrypt + (i*16));
    }
    printf("\r\n");


//    return bufferDecrypt;
    
}






//******************************************************************************
// Function Prototypes
//******************************************************************************

void HardwareInit( void );
void ProcessNONZigBeeTasks(void);
void ProcessZigBeePrimitives(void);
extern void    RemoveAllGroups(void);
extern void RemoveAllBindings(SHORT_ADDR);
extern BYTE LookupAPSAddress( LONG_ADDR *);
//******************************************************************************
// Application Variables
//******************************************************************************

ZIGBEE_PRIMITIVE    currentPrimitive;

SHORT_ADDR          destinationAddress;
#ifdef I_SUPPORT_SECURITY
	extern KEY_VAL	KeyVal;
	#ifdef USE_EXTERNAL_NVM
		extern WORD trustCenterLongAddr;
		extern NETWORK_KEY_INFO plainSecurityKey[2];
	#else
		extern ROM LONG_ADDR trustCenterLongAddr;
	#endif
#endif

static union
{
    struct
    {
        BYTE    bBroadcastSwitchToggled    : 1;
        BYTE    bLightSwitchToggled        : 1;
        BYTE    bTryingToBind              : 1;
        BYTE    bIsBound                   : 1;
        BYTE    bDestinationAddressKnown   : 1;
        BYTE    bBindRequestDone           : 1;
    } bits;
    BYTE Val;
} myStatusFlags;
#define STATUS_FLAGS_INIT       0x00
#define TOGGLE_BOUND_FLAG       0x08
#define bBindSwitchToggled      bBroadcastSwitchToggled

BYTE FIRST_RUN = 1;

typedef enum _MENU_CMD
{
    MENU_CMD_APS_ACK = '0',
    MENU_CMD_RD_TYPE,
    MENU_CMD_NWK_ADDR_REQ,
    MENU_CMD_ON_DISCOVERED,
    MENU_CMD_OFF_DISCOVERED,
    MENU_CMD_ON_SHORT,
    MENU_CMD_OFF_SHORT,
    MENU_CMD_RREQ_ON,
    MENU_CMD_RREQ_OFF,
    MENU_CMD_LOOP,
    MENU_CMD_REQUEST_KEY = 'A'
} MENU_CMD;

BYTE                apsACK = 0;
SHORT_ADDR          discoveredAddress;
BYTE                routeDiscovery;
extern BYTE				MACDataTransmission;
extern BYTE             APSCounter;

/* Menu System */
ROM char * const menu =
    "\r\n     1: Enable/Disable Joining by Other Devices"
    "\r\n     2: Request Data From Another Device"
    "\r\n     3: Request Data From a Group of Devices"
    "\r\n     4: Send Data To Another Device"
    "\r\n     5: Send Data To a Group of Devices"
    "\r\n     6: Add/Remove Device to/from a Group"
    "\r\n     7: Dump Neighborhood Information"
	"\r\n     8: Transfer a packet"
	"\r\n     0: Cryptography test"
    ;
  
    
void ProcessMenu( void );
void PrintMenu( void );
BYTE GetHexDigit( void );
BYTE GetMACByte( void );

WORD_VAL MSGPacketCount;

BYTE RX_On = 1;

BYTE        i, temp2;
BOOL        gettingAcks = FALSE;

extern BOOL bDisableShortAddress;

extern BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap);
BOOL    group_state_ID5 = TRUE;
BOOL    group_state_ID4 = TRUE;
BYTE    GROUP_ID4       = 0x04;
BYTE    GROUP_ID5       = 0x05;

BOOL PB_LEFT_pressed    = FALSE;        /* RB5/RD6 */
BOOL PB_RIGHT_pressed   = FALSE;        /* RB4/RD7 */

TICK PB_LEFT_press_time;
TICK PB_RIGHT_press_time;
TICK tickDifference;
TICK tick2Difference;

//******************************************************************************
//******************************************************************************
//******************************************************************************
#if defined(__C30__)
    int main(void)
#else
    void main(void)
#endif
{
    #if defined(__18F87J10)
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        OSCTUNEbits.PLLEN = 1;
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
    #endif
    	
    CLRWDT();
    ENABLE_WDT();

    currentPrimitive = NO_PRIMITIVE;

    /* Initialize the UART such that data can be sent and recieved on terminal */
    ConsoleInit();
    
     /* Initialize the hardware before initializing the ZigBee Stack */
    HardwareInit();
    
/* if using security force the use of key slot 0 in stack */
#ifdef I_SUPPORT_SECURITY
    {
	    BYTE i;
    	i = 0xff;
        PutNwkActiveKeyNumber(&i);
    }
#endif
    
    /* Initialize the ZigBee Stack */
    ZigBeeInit();
    //PHYSetShortRAMAddr(0x00, 0x0d);
	MSGPacketCount.Val = 0;

    NWKClearNeighborTable();
    /* Refreash the tables anew each time the Node is booted up */
    #if defined(I_SUPPORT_BINDING)
        
        ClearBindingTable();
    #endif

    /* Clear the Group Table */
    RemoveAllGroups();
    #if defined(I_SUPPORT_BINDING)
        RemoveAllBindings(macPIB.macShortAddress);
    #endif
    // *************************************************************************
    // Perform any other initialization here
    // *************************************************************************

    ConsolePutROMString( (ROM char *)"\r\n\r\n\r\n********************************" );
    ConsolePutROMString( (ROM char *)"\r\nMicroChip ZigBee2006(TM) Stack  v2.0-2.6.0a Coordinator\r\n\r\n" );
    #if (RF_CHIP == MRF24J40)
        ConsolePutROMString( (ROM char *)"Transceiver-MRF24J40\r\n\r\n" );
    #else
        ConsolePutROMString( (ROM char *)"Transceiver-Unknown\r\n\r\n" );
    #endif


    myStatusFlags.Val = STATUS_FLAGS_INIT;

    /* Enable interrupts to get the stack operational */
    #if defined(__C30__)
        RFIE = 1;
    #endif
    #if !defined(__C30__)
        INTCONbits.RBIE = 1;
        RCONbits.IPEN = 1;
        INTCONbits.GIEH = 1;
    #endif
    
    /* Set LEDs to a known state */
    GROUP_INDICATION	    = 0;
	MESSAGE_INDICATION		= 0;
	
	while (1)
    {
        /* Clear the watch dog timer */
        CLRWDT();
        
        /* Process the current ZigBee Primitive */
        ZigBeeTasks( &currentPrimitive );
        
        /* Determine the next ZigBee Primitive */
        ProcessZigBeePrimitives();
        
        /* do any non ZigBee related tasks  */
        ProcessNONZigBeeTasks();
    }
} 
	   
void ProcessZigBeePrimitives(void)
{
        switch (currentPrimitive)
        {
            case NLME_DIRECT_JOIN_confirm:
                if (params.NLME_DIRECT_JOIN_confirm.Status == NWK_TABLE_FULL)
                {
                    ConsolePutROMString( (ROM char *)"  Neighbor table is full.\r\n" );
                }
                else
                {
                    ConsolePutROMString( (ROM char *)"  Direct join successful.\r\n" );
                }
                PrintMenu();
                currentPrimitive = NO_PRIMITIVE;
                break;	        
	        
	        
	        
            case NLME_NETWORK_FORMATION_confirm:
                if (!params.NLME_NETWORK_FORMATION_confirm.Status)
                {
                    ConsolePutROMString( (ROM char *)"PAN " );
                    PrintChar( macPIB.macPANId.byte.MSB );
                    PrintChar( macPIB.macPANId.byte.LSB );
                    ConsolePutROMString( (ROM char *)" started successfully.\r\n" );
                    params.NLME_PERMIT_JOINING_request.PermitDuration = 0xFF;
                    params.NLME_PERMIT_JOINING_request._updatePayload = TRUE;
                    
                    /* Set LEDs to the upp and running ON state*/
                    GROUP_INDICATION   = 1;
                    MESSAGE_INDICATION = 1;
                    
                    currentPrimitive = NLME_PERMIT_JOINING_request;
                }
                else
                {
                    PrintChar( params.NLME_NETWORK_FORMATION_confirm.Status );
                    ConsolePutROMString( (ROM char *)" Error forming network.  Trying again...\r\n" );
                    currentPrimitive = NO_PRIMITIVE;
                }
                break;

            case NLME_PERMIT_JOINING_confirm:
                if (!params.NLME_PERMIT_JOINING_confirm.Status)
                {
                    ConsolePutROMString( (ROM char *)"Joining permitted.\r\n" );
                    currentPrimitive = NO_PRIMITIVE;
                }
                else
                {
                    PrintChar( params.NLME_PERMIT_JOINING_confirm.Status );
                    ConsolePutROMString( (ROM char *)" Join permission unsuccessful. We cannot allow joins.\r\n" );
                    currentPrimitive = NO_PRIMITIVE;
                }
                PrintMenu();
                break;

            case NLME_ROUTE_DISCOVERY_confirm:
                if (!params.NLME_ROUTE_DISCOVERY_confirm.Status)
                {
                    ConsolePutROMString( (ROM char *)"..Route Reply Successful...\r\n" );
                }
                else
                {
                    PrintChar( params.NLME_PERMIT_JOINING_confirm.Status );
                    ConsolePutROMString( (ROM char *)"..Route Reply not Successful..\r\n" );
                }
                currentPrimitive = NO_PRIMITIVE;
                PrintMenu();
                break;


            case NLME_JOIN_indication:
                ConsolePutROMString( (ROM char *)"\r\nNode " );
                PrintChar( params.NLME_JOIN_indication.ShortAddress.byte.MSB );
                PrintChar( params.NLME_JOIN_indication.ShortAddress.byte.LSB );
                ConsolePutROMString( (ROM char *)" With MAC Address " );
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[7]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[6]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[5]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[4]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[3]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[2]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[1]);
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[0]);
                ConsolePutROMString( (ROM char *)" just joined.\r\n" );

                /* For Zigbee 2006: If a new device with the same old longAddress address
                * joins the PAN, then make sure the old short address is no longer used and is 
                * overwritten with the new shortAddress & longAddress combination 
                */
                {  /* same long address check block */
                    APS_ADDRESS_MAP currentAPSAddress1; 
                    currentAPSAddress1.shortAddr   =   params.NLME_JOIN_indication.ShortAddress;
                    currentAPSAddress1.longAddr    =   params.NLME_JOIN_indication.ExtendedAddress;
                   
                 if(LookupAPSAddress(&params.NLME_JOIN_indication.ExtendedAddress) )
                 {
                     
                    for( i = 0; i < apscMaxAddrMapEntries; i++)
                    {
                        #ifdef USE_EXTERNAL_NVM
                            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                        #else
                            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                        #endif
                            if (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&currentAPSAddress1.longAddr, 8 ))
                            {
                                /* overwrite the old with the new short/long address combo  */
                                #ifdef USE_EXTERNAL_NVM
                                    PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress1);
                                #else
                                    PutAPSAddress( &apsAddressMap[i], &currentAPSAddress1 );
                                #endif
                            }
                      }   /* end for loop */
                                    
                  }       /* end if */
            }             /* end address check block */
                            
                #ifdef I_SUPPORT_SECURITY

					#ifdef I_AM_TRUST_CENTER
  		 			{
			  		  	BOOL AllowJoin = TRUE;
						// decide if you allow this device to join
			  		  	if( !AllowJoin )
  		  				{
			  		  		// no need to set deviceAddress, since it is overlap with NLME_JOIN_indication
		  					//params.NLME_LEAVE_request.DeviceAddress = params.NLME_JOIN_indication.ExtendedAddress;
					  		params.NLME_LEAVE_request.RemoveChildren = TRUE;
  		  					currentPrimitive = NLME_LEAVE_request;
							break;
  		  				}
  		  	
						#ifdef I_SUPPORT_SECURITY_SPEC
							if( params.NLME_JOIN_indication.secureJoin )
							{
								BYTE i;
								for(i = 0; i < 16; i++)
								{
									KeyVal.v[i] = 0;
								}
								params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
								params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
				
							} else {
								BYTE i;
								GetNwkActiveKeyNumber(&i);
				#ifdef USE_EXTERNAL_NVM
					currentNetworkKeyInfo = plainSecurityKey[i-1];
				#else
								GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(NetworkKeyInfo[i-1]));
				#endif
								params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
								params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];

							}
						#else
							#ifdef PRECONFIGURE_KEY
							{
								BYTE i;
								for(i = 0; i < 16; i++)
								{
									KeyVal.v[i] = 0;
								}
								params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
								params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
				params.APSME_TRANSPORT_KEY_request._UseSecurity = TRUE;
							}
							#else
			if( params.NLME_JOIN_indication.secureJoin )
							{
								BYTE i;
				for(i = 0; i < 16; i++)
				{
					KeyVal.v[i] = 0;
				}
				params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
				params.APSME_TRANSPORT_KEY_request._UseSecurity = TRUE;	
    	    }
    	    else 
			{
				BYTE i;
								GetNwkActiveKeyNumber(&i);
				#ifdef USE_EXTERNAL_NVM
					currentNetworkKeyInfo = plainSecurityKey[i-1];
				#else
								GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(networkKeyInfo[i-1]));
				#endif
								params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
								params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];		
				params.APSME_TRANSPORT_KEY_request._UseSecurity = FALSE;
							}	
							#endif
			
						#endif
						params.APSME_TRANSPORT_KEY_request.KeyType = ID_NetworkKey;
						params.APSME_TRANSPORT_KEY_request.DestinationAddress = params.NLME_JOIN_indication.ExtendedAddress;
						params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;
						currentPrimitive = APSME_TRANSPORT_KEY_request;
					}
  					#else
						#ifdef I_SUPPORT_SECURITY_SPEC
							params.APSME_UPDATE_DEVICE_request.Status = (params.NLME_JOIN_indication.secureJoin ) ? 0x00 : 0x01;
						#else
							#ifdef PRECONFIGURE_KEY
								params.APSME_UPDATE_DEVICE_request.Status = 0x00;
							#else
								params.APSME_UPDATE_DEVICE_request.Status = 0x01;
							#endif
						#endif
						params.APSME_UPDATE_DEVICE_request.DeviceShortAddress = params.NLME_JOIN_indication.ShortAddress;
						params.APSME_UPDATE_DEVICE_request.DeviceAddress = params.NLME_JOIN_indication.ExtendedAddress;
						GetTrustCenterAddress(&params.APSME_UPDATE_DEVICE_request.DestAddress);
						for(i=0; i < 8; i++)
						    params.APSME_UPDATE_DEVICE_request.DestAddress.v[i] = 0xaa;
						        
						currentPrimitive = APSME_UPDATE_DEVICE_request;
					#endif
				#else
                	currentPrimitive = NO_PRIMITIVE;
				#endif				
                break;

            case NLME_LEAVE_indication:
                {
                    #if defined(__C30__)
                        LONG_ADDR myLongAddr;
                    
                        GetMACAddress(&myLongAddr);
                        if(!memcmppgm2ram( &params.NLME_LEAVE_indication.DeviceAddress, &myLongAddr, 8 ))   
                    #else
                        if (!memcmppgm2ram( &params.NLME_LEAVE_indication.DeviceAddress, (ROM void *)&macLongAddr, 8 ))
                    #endif
                    {
                        ConsolePutROMString( (ROM char *)"Node has left the network.\r\n" );
                    }
                    else
                    {
                        ConsolePutROMString( (ROM char *)" Another node has left the network.\r\n" );
                    }
                }
				#ifdef I_SUPPORT_SECURITY
		 			{
						SHORT_ADDR		LeavingChildShortAddress;
						if( !APSFromLongToShort(&params.NLME_LEAVE_indication.DeviceAddress) ) 
						{
							currentPrimitive = NO_PRIMITIVE;
							break;
						}
						LeavingChildShortAddress = currentAPSAddress.shortAddr;

					#ifdef I_AM_TRUST_CENTER
						params.APSME_UPDATE_DEVICE_indication.Status = 0x02;
						params.APSME_UPDATE_DEVICE_indication.DeviceAddress = params.NLME_LEAVE_indication.DeviceAddress;
						GetMACAddress(&params.APSME_UPDATE_DEVICE_indication.SrcAddress);
						params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress = LeavingChildShortAddress;
						currentPrimitive = APSME_UPDATE_DEVICE_indication;
						break;
					#else
						params.APSME_UPDATE_DEVICE_request.Status = 0x02;
						GetTrustCenterAddress(&params.APSME_UPDATE_DEVICE_request.DestAddress);
						params.APSME_UPDATE_DEVICE_request.DeviceShortAddress = LeavingChildShortAddress;
						currentPrimitive = APSME_UPDATE_DEVICE_request;
						break;
					#endif
					}

				#else			
                	currentPrimitive = NO_PRIMITIVE;
				#endif
                break;
            case NLME_RESET_confirm:
                ConsolePutROMString( (ROM char *)"ZigBee Stack has been reset.\r\n" );
                currentPrimitive = NO_PRIMITIVE;
                break;
                
            case NLME_LEAVE_confirm:
                ConsolePutROMString( (ROM char *)"Device has left the network\r\n" );
                PrintChar(params.NLME_LEAVE_confirm.Status);
                currentPrimitive = NO_PRIMITIVE;
                break;
                
            case APSDE_DATA_confirm:
                if (params.APSDE_DATA_confirm.Status)
                {
                    ConsolePutROMString( (ROM char *)"Error " );
                    PrintChar( params.APSDE_DATA_confirm.Status );
                    ConsolePutROMString( (ROM char *)" sending message.\r\n" );
                }
                else
                {
                    ConsolePutROMString( (ROM char *)" Message sent successfully.\r\n" );
                }
                currentPrimitive = NO_PRIMITIVE;
                break;

            case APSDE_DATA_indication:
                {
                    BYTE        command = 0;
                    BYTE        data;
                    BYTE        frameHeader;
                    BYTE        sequenceNumber = 0;
                    BYTE        transaction;
                    BYTE        transByte = 0;

                    currentPrimitive = NO_PRIMITIVE;

                    switch (params.APSDE_DATA_indication.DstEndpoint)
                    {
                        /* Process anything sent to ZDO */  
                        case EP_ZDO:
                            #define dataLength command
                            if (1)
                            {
                                for (transaction=0; transaction < 1; transaction++)
                                {
                                    sequenceNumber          = APLGet();
                                    switch( params.APSDE_DATA_indication.ClusterId.Val )
                                    {

                                        // ********************************************************
                                        // Put a case here to handle each ZDO response that we requested.
                                        // ********************************************************

                                        case BIND_rsp:
                                        case UNBIND_rsp:
                                            data = APLGet();
                                            if (data == SUCCESS)
                                            {
                                                ConsolePutROMString( (ROM char *)"Binding/Unbinding done successfully.\r\n" );
                                                myStatusFlags.bits.bBindRequestDone = 1;
                                            }
                                            else
                                            {
                                                PrintChar( data );
                                                ConsolePutROMString( (ROM char *)" Error doing binding/unbinding...\r\n" );
                                            }
                                            break;
                                        
                                        case NWK_ADDR_rsp:
                                            if (APLGet() == SUCCESS)
                                            {
                                                /* update our table when device recieves the address it requested */
                                                ConsolePutROMString( (ROM char *)"Receiving NWK_ADDR_rsp.\r\n" );

                                                // Skip over the IEEE address of the responder.
                                                for (data=0; data<8; data++)
                                                {
                                                    currentAPSAddress.longAddr.v[data] = APLGet();
                                                    transByte++;
                                                }
                                                currentAPSAddress.shortAddr.v[0] = destinationAddress.byte.LSB = APLGet();
                                                currentAPSAddress.shortAddr.v[1] = destinationAddress.byte.MSB = APLGet();
                                                transByte += 2;
                                                myStatusFlags.bits.bDestinationAddressKnown = 1;
                                                APSSaveAPSAddress(&currentAPSAddress);
                                            }
                                            break;

                                        /* Process any further ZDO responses here */
                                        default:
                                            break;
                                    }

                                    // Read out the rest of the MSG in case there is another transaction.
                                    for (; transByte<dataLength; transByte++)
                                    {
                                        APLGet();
                                    }
                                }
                            }
                            #undef dataLength
                            break;

                        // ************************************************************************
                        // Place a case for each user defined endpoint.
                        // ************************************************************************

						case 171:
								{
									unsigned char receivedFromRouter[16];
									WORD_VAL clusterID = params.APSDE_DATA_indication.ClusterId;
									switch(clusterID.Val)
									{	
										
										
										case 0x0002:






											for(i = 0; i < 16; i++)
					{
						
						receivedFromRouter[i] = APLGet();
					}
   					printf("\r\nReceived From Router: ");
 				    printStringToHex(receivedFromRouter);
  				  	printf("\r\n");
										
											
										
											break;
									}
									break;
								}






                        default:
                       /* In this example every endpoint except Endpoint EP_ZDO is processed here */
                            {
	                            BYTE i;
	                            BYTE frameHeaderIndex = TxData;
	                            WORD_VAL clusterID = params.APSDE_DATA_indication.ClusterId;
	                                
	                        	frameHeader = 1;
	                        	for(transaction=0; transaction<frameHeader; transaction++)
	                        	{
		                        	BYTE PacketLen;
		                        	BYTE transactionNumber;
		                        	
		                        	switch( clusterID.Val )
		                        	{
			                        	case TRANSMIT_COUNTED_PACKETS_CLUSTER:
			                        	{
				                        	WORD_VAL Seq;
				                        	PacketLen = APLGet();
				                        	
				                        	Seq.v[0] = APLGet();
				                        	Seq.v[1] = APLGet();
				                        	if( Seq.Val > MSGPacketCount.Val )
				                        	{
					                        	MSGPacketCount.Val = Seq.Val;
											}
											for(i = 0; i < PacketLen-2; i++)
												APLGet();
										}
			                        	break;
			                        	
			                        	case RESET_PACKET_COUNT_CLUSTER:
			                        		MSGPacketCount.Val = 0;
			                        		break;
			                        		
			                        	case RETRIEVE_PACKET_COUNT_CLUSTER:			                        		
			                        		TxBuffer[TxData++] = sequenceNumber;
			                        		TxBuffer[TxData++] = 2;
			                        		TxBuffer[TxData++] = MSGPacketCount.v[0];
			                        		TxBuffer[TxData++] = MSGPacketCount.v[1];
			                        		MSGPacketCount.Val++;
			                        		transactionNumber = TxBuffer[frameHeaderIndex] & APL_FRAME_COUNT_MASK;
			                        		TxBuffer[frameHeaderIndex] &= APL_FRAME_TYPE_MASK;
			                        		TxBuffer[frameHeaderIndex] |= (transactionNumber+1);
			                        		
											if( transactionNumber == 0 )
											{
			                        			ZigBeeBlockTx();

                                        		params.APSDE_DATA_request.DstAddrMode = params.APSDE_DATA_indication.SrcAddrMode;
    	                                    	params.APSDE_DATA_request.DstAddress.ShortAddr = params.APSDE_DATA_indication.SrcAddress.ShortAddr;
        	                                	params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
            	                            	params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;
#ifdef I_SUPPORT_SECURITY
												params.APSDE_DATA_request.TxOptions.Val = 1;
#else
                    	                    	params.APSDE_DATA_request.TxOptions.Val = 0;
#endif
												i = params.APSDE_DATA_indication.SrcEndpoint;
												params.APSDE_DATA_request.SrcEndpoint = params.APSDE_DATA_indication.DstEndpoint;
                        	                	params.APSDE_DATA_request.DstEndpoint =  i;
                        	                	params.APSDE_DATA_request.ClusterId.Val = PACKET_COUNT_RESPONSE_CLUSTER;
                            	            	currentPrimitive = APSDE_DATA_request;
                            	            }
			                        		
			                    			break;  
			                    			
			                    		case PACKET_COUNT_RESPONSE_CLUSTER:
			                    		{
				                    		BYTE PC_LSB = APLGet();
				                    		BYTE PC_MSB = APLGet();
				                    		
				                    		ConsolePutROMString((ROM char *)"Packet Count Response: ");
				                    		PrintChar(PC_MSB);
				                    		PrintChar(PC_LSB);
				                    		ConsolePutROMString((ROM char *)"\r\n");
				                    	}  	
				                    	break;
				                    	
				                    	case BUFFER_TEST_REQUEST_CLUSTER:
				                    	{
					                    	BYTE SeqLen = APLGet();
					                    	
					                    #ifdef I_SUPPORT_SECURITY
					                    	if( SeqLen < 66 )
					                    #else
					                    	if( SeqLen < 84 )
					                    #endif
					                    	{
						                    	TxBuffer[TxData++] = SeqLen;
						                    	TxBuffer[TxData++] = SUCCESS;
						                    	for(i = 0; i < SeqLen; i++)
						                    	{
							                    	TxBuffer[TxData++] = i;
							                    }
							                } else {
								                TxBuffer[TxData++] = SeqLen;
	                            	            TxBuffer[TxData++] = 0x01;
	                            	        }
		                          			
		                        			/* package and send response here */
			                        		{
    			                        		/* Do not ask data of myself even though I am in the same group */
    			                        		if(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val==(macPIB.macShortAddress.Val))
    			                        		{
        			                        	    printf(" \r\n Sending to myself\r\n");
        			                        	    APSDiscardRx();
        			                        	    currentPrimitive = NO_PRIMITIVE;
        			                        	    break;
        			                            }
					                    		ZigBeeBlockTx();
	
   	                                    		params.APSDE_DATA_request.DstAddrMode = params.APSDE_DATA_indication.SrcAddrMode;
   		                                    	params.APSDE_DATA_request.DstAddress.ShortAddr = params.APSDE_DATA_indication.SrcAddress.ShortAddr;
       		                                	params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
           		                            	params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
#ifdef I_SUPPORT_SECURITY
												params.APSDE_DATA_request.TxOptions.Val = 1;
#else
                   		                    	params.APSDE_DATA_request.TxOptions.Val = 0;
#endif
												i = params.APSDE_DATA_indication.SrcEndpoint;
												params.APSDE_DATA_request.SrcEndpoint = params.APSDE_DATA_indication.DstEndpoint;
                       	        	        	params.APSDE_DATA_request.DstEndpoint =  i;
                       	            	    	params.APSDE_DATA_request.ClusterId.Val = BUFFER_TEST_RESPONSE_CLUSTER;
                           	            		currentPrimitive = APSDE_DATA_request;
                           	            	}	                            	            
					                    }
					                    /* Group 4 uses End Point 4 so here that application
					                     * is Toggling LED1 it received a  request from Group 4 
					                    */
					                    if(params.APSDE_DATA_indication.SrcEndpoint == GROUP_ID4)
					                        MESSAGE_INDICATION = !MESSAGE_INDICATION;
					                    break;
					                    
					                    case BUFFER_TEST_RESPONSE_CLUSTER:
					                    {
						                    BYTE len = APLGet();
						                    printf("\r\nLen: ");
						                    PrintChar(len);
						                    printf("\r\n");
						                    printf("From Address: ");
						                    PrintChar(params.APSDE_DATA_indication.SrcAddress.ShortAddr.byte.MSB);
						                    PrintChar(params.APSDE_DATA_indication.SrcAddress.ShortAddr.byte.LSB);
						                    printf("\r\n");
						                    for(i = 0; i < len+1; i++) {
						                    	PrintChar(APLGet());
						                    }
						                    printf("\r\n");	
						                }
					                    	break;
					                    
				                    	case FREEFORM_MSG_REQUEST_CLUSTER:
				                    	{
					                    	BYTE requestType = APLGet();
					                    	TxBuffer[TxData++] = requestType;  /* return the type */
					                    	switch(requestType)
					                    	{
						                    	case 0x00:
						                    		TxBuffer[TxData++] = 0x42;
						                    		break;
						                    		
						                    	case 0x01:
						                    		TxBuffer[TxData++] = 0x5a;
						                    		TxBuffer[TxData++] = 0x69;
						                    		TxBuffer[TxData++] = 0x67;
						                    		TxBuffer[TxData++] = 0x42;
						                    		TxBuffer[TxData++] = 0x65;
						                    		TxBuffer[TxData++] = 0x65;
						                    		break;
						                    		
						                    	case 0x02:
						                    		TxBuffer[TxData++] = 0x12;
						                    		TxBuffer[TxData++] = 0x34;
						                    		TxBuffer[TxData++] = 0x56;
						                    		TxBuffer[TxData++] = 0x78;
						                    		break;
						                    }
			                        		{					                    
					                    		ZigBeeBlockTx();

	                                        	params.APSDE_DATA_request.DstAddrMode = params.APSDE_DATA_indication.SrcAddrMode;
    		                                    params.APSDE_DATA_request.DstAddress.ShortAddr = params.APSDE_DATA_indication.SrcAddress.ShortAddr;
        		                                params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
            		                            params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;
#ifdef I_SUPPORT_SECURITY
												params.APSDE_DATA_request.TxOptions.Val = 1;
#else
                    		                    params.APSDE_DATA_request.TxOptions.Val = 0;
#endif
												i = params.APSDE_DATA_indication.SrcEndpoint;
												params.APSDE_DATA_request.SrcEndpoint = params.APSDE_DATA_indication.DstEndpoint;
                        	    	            params.APSDE_DATA_request.DstEndpoint =  i;
                        	        	        params.APSDE_DATA_request.ClusterId.Val = FREEFORM_MSG_RESPONSE_CLUSTER;
                            	        	    currentPrimitive = APSDE_DATA_request;					                    	
                            	        	}
					                    }
					                    break;
				                    	
				                    	case FREEFORM_MSG_RESPONSE_CLUSTER:
				                    	{
					                    	BYTE len = APLGet();
					                    	for(i = 0; i < len; i++)
					                    		APLGet();
				                    		break;
				                    	}
				                    	
				                    	default:
				                    	    /* Catch all place for all none ZDO msgs not processed above */
				                    	    printf("Got message.....");
				                    		break;
			                        }
		                        		
		                        }
	                        	if( currentPrimitive != APSDE_DATA_request )
		                        	TxData = TX_DATA_START;
	                        	
	                        }
                        	break;
                    }
                    APLDiscardRx();
                }
                break;
            case APSME_ADD_GROUP_confirm:
            case APSME_REMOVE_GROUP_confirm:
            case APSME_REMOVE_ALL_GROUPS_confirm:
                currentPrimitive = NO_PRIMITIVE;
                break;    

            /* if nothing to process first check to see if we are in startup seqence */
            case NO_PRIMITIVE:
                if (!ZigBeeStatus.flags.bits.bNetworkFormed)
                {
                    if (!ZigBeeStatus.flags.bits.bTryingToFormNetwork)
                    {
                        ConsolePutROMString( (ROM char *)"Trying to start network...\r\n" );
                        params.NLME_NETWORK_FORMATION_request.ScanDuration          = 8;
                        params.NLME_NETWORK_FORMATION_request.ScanChannels.Val      = ALLOWED_CHANNELS;
                        params.NLME_NETWORK_FORMATION_request.PANId.Val             = 0x1aaa;
                        params.NLME_NETWORK_FORMATION_request.BeaconOrder           = MAC_PIB_macBeaconOrder;
                        params.NLME_NETWORK_FORMATION_request.SuperframeOrder       = MAC_PIB_macSuperframeOrder;
                        params.NLME_NETWORK_FORMATION_request.BatteryLifeExtension  = MAC_PIB_macBattLifeExt;
                        currentPrimitive = NLME_NETWORK_FORMATION_request;
                        break;
                    }
                }
                
                /* Process interrupts from push buttons, after ZigBee is up and running */
                else
                {
                    if (  ZigBeeReady() ) 
                    {
                        /* if already on the network then process menu items */
                        if(!ZigBeeStatus.flags.bits.bHasBackgroundTasks)
                        {           
                            if ( ConsoleIsGetReady() )
                            {
                                ProcessMenu();
                            }
                        } 
                        
                        
                        // ************************************************************************
                        // Place all processes that can send messages here.  Be sure to call
                        // ZigBeeBlockTx() when currentPrimitive is set to APSDE_DATA_request.
                        // ************************************************************************
                    
                        /* Add/Remove Device to/from Group 4 */
                        if( PB_RIGHT_SWITCH == 0 )
                        {
                            /* wait debounce time before taking any action again */
                            if(PB_RIGHT_pressed == FALSE)
                            {
                                /* release to capture another button press */
                                PB_RIGHT_pressed = TRUE;
                                
                                /* determine if we should add or remove group */
                                if(group_state_ID4)
                                {
                                    params.APSME_ADD_GROUP_request.Endpoint          = GROUP_ID4; 
	                                params.APSME_ADD_GROUP_request.GroupAddress.v[1] = 0x00;
	                                params.APSME_ADD_GROUP_request.GroupAddress.v[0] = GROUP_ID4;
	                        
                                    currentPrimitive    = APSME_ADD_GROUP_request;
                                    GROUP_INDICATION = 1;
                                    printf("   \r\nAdded node to group 4\r\n");
                                }
                                else
                                {
                                    params.APSME_REMOVE_GROUP_request.Endpoint          = GROUP_ID4; 
	                                params.APSME_REMOVE_GROUP_request.GroupAddress.v[1] = 0x00;
	                                params.APSME_REMOVE_GROUP_request.GroupAddress.v[0] = GROUP_ID4;
	                        
                                    currentPrimitive = APSME_REMOVE_GROUP_request;
                                    GROUP_INDICATION = 0;
                                    printf("   \r\nRemoved node from group 4\r\n");
                                }
                                group_state_ID4     = !group_state_ID4;
                            
                                break;
                             }
                        }
                        else        /* Debounce Timeout period calculation */
                        {
                            TICK t = TickGet();
                            tick2Difference.Val = TickGetDiff(t,PB_RIGHT_press_time);
                
                            if(tick2Difference.Val > DEBOUNCE_TIME)
                            {
                                PB_RIGHT_pressed = FALSE;
                            }    
                      
                        }
                        
                            
                        /* handle sending Group data request when PB is pressed */   
                        if(PB_LEFT_SWITCH == 0)
                        {
                            /* wait debounce time before taking any action again */
                            if(PB_LEFT_pressed == FALSE)
                            {
                                PB_LEFT_pressed = TRUE;
                                
                                /* Send group message to all the devices */
                                ZigBeeBlockTx();
                                TxBuffer[TxData++] = 0x0a;      /* request 10-bytes */

                                /* Use group addressing mode to send request */
                                params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_GROUP;
                    
                                /* GroupID MSB is zero in this example  */
                                params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = 0x00;
                                params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = GROUP_ID4;
                            
                                params.APSDE_DATA_request.SrcEndpoint               = GROUP_ID4;
                    
	                            params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
                                params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
                                #ifdef I_SUPPORT_SECURITY
                                    params.APSDE_DATA_request.TxOptions.Val = 1;
                                #else
	                            params.APSDE_DATA_request.TxOptions.Val = 0;
                                #endif
                    
                                /* no acknowledgement since this is a broadcast request */
	                            params.APSDE_DATA_request.TxOptions.bits.acknowledged = 0;
	                            params.APSDE_DATA_request.ProfileId.Val = 0x7f01;
                                params.APSDE_DATA_request.ClusterId.Val = BUFFER_TEST_REQUEST_CLUSTER;
                                currentPrimitive = APSDE_DATA_request;
                                
                                /* debounce lockout time start */
                                PB_LEFT_press_time = TickGet();
                                	 
                                break;
                            }   
                        }
                        else        /* Debounce Timeout period calculation */
                        {
                            TICK t = TickGet();
                            tickDifference.Val = TickGetDiff(t,PB_LEFT_press_time);
                
                            if(tickDifference.Val > DEBOUNCE_TIME)
                            {
                                PB_LEFT_pressed = FALSE;
                            }    
                      
                        }
                   
                }
            }
            
            break;    
                
            default:
                PrintChar( currentPrimitive );
                ConsolePutROMString( (ROM char *)" Unhandled primitive.\r\n" );
                currentPrimitive = NO_PRIMITIVE;
                break;
        }
        
}

void ProcessNONZigBeeTasks(void)
{

        // *********************************************************************
        // Place any non-ZigBee related processing here.  Be sure that the code
        // will loop back and execute ZigBeeTasks() in a timely manner.
        // *********************************************************************
        {
            
          
        }
}
/*******************************************************************************
HardwareInit

All port directioning and SPI must be initialized before calling ZigBeeInit().
*******************************************************************************/
#if defined(__C30__)
void HardwareInit(void)
{
    #if(CLOCK_FREQ < 1000000)
        SPI1CON1 = 0b0000000100111111;      // CLOCK_FREQ as SPI CLOCK
        SPI1STAT = 0x8000;
       
        SPI2CON1 = 0b0000000100111111;      // CLOCK_FREQ as SPI CLOCK
        SPI2STAT = 0x8000;
    #else
        SPI1CON1 = 0b0000000100111110;      // CLOCK_FREQ/4 as SPI CLOCK
        SPI1STAT = 0x8000;
        
        SPI2CON1 = 0b0000000100111110;      // CLOCK_FREQ/4 as SPI CLOCK
        SPI2STAT = 0x8000;
    #endif
    
    #ifdef USE_EXTERNAL_NVM
    	EEPROM_nCS		= 1;
    	EEPROM_nCS_TRIS	= 0;
    	IFS2bits.SPI2IF = 1;
    #endif
    
    PHY_RESETn = 0;
    PHY_RESETn_TRIS = 0;
    PHY_CS = 1;
    PHY_CS_TRIS = 0;
    
    TRISAbits.TRISA6 = 0;
    TRISAbits.TRISA7 = 0;
    
    RFIF = 0;
    RFIE = 1;
    
    if(RF_INT_PIN == 0)
    {
        RFIF = 1;
    }
    
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD7 = 1;
  
    CNEN1bits.CN15IE = 1;
    CNEN2bits.CN16IE = 1;
    CNPU1bits.CN15PUE = 1;
    CNPU2bits.CN16PUE = 1;
    
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
    	   
}
#else
void HardwareInit(void)
{

    #ifdef USE_EXTERNAL_NVM
        EEPROM_nCS          = 1;
        EEPROM_nCS_TRIS     = 0;
    #endif

    #if defined(USE_EXTERNAL_NVM) && !defined(EE_AND_RF_SHARE_SPI)
        RF_SPIInit();
        EE_SPIInit();
    #else
        SPIInit();
    #endif

    #if (RF_CHIP == MRF24J40)
        // Start with MRF24J40 disabled and not selected
        PHY_CS              = 1;
        PHY_RESETn          = 1;

        // Set the directioning for the MRF24J40 pin connections.
        PHY_CS_TRIS         = 0;
        PHY_RESETn_TRIS     = 0;

        // Initialize the interrupt.
        INTCON2bits.INTEDG0 = 0;
    #else
        #error Unknown transceiver selected
    #endif

    #if defined(USE_EXTERNAL_NVM) && !defined(EE_AND_RF_SHARE_SPI)
        // Initialize the SPI1 pins and directions
        LATCbits.LATC3               = 0;    // SCK
        LATCbits.LATC5               = 1;    // SDO
        TRISCbits.TRISC3             = 0;    // SCK
        TRISCbits.TRISC4             = 1;    // SDI
        TRISCbits.TRISC5             = 0;    // SDO
    
        // Initialize the SPI2 pins and directions
        LATDbits.LATD6               = 0;    // SCK
        LATDbits.LATD4               = 1;    // SDO
        TRISDbits.TRISD6             = 0;    // SCK
        TRISDbits.TRISD5             = 1;    // SDI
        TRISDbits.TRISD4             = 0;    // SDO
    
        RF_SSPSTAT_REG = 0x40;
        RF_SSPCON1_REG = 0x21;
        EE_SSPSTAT_REG = 0x40;
        EE_SSPCON1_REG = 0x21;
    #else
        // Initialize the SPI pins and directions
        LATCbits.LATC3               = 0;    // SCK
        LATCbits.LATC5               = 1;    // SDO
        TRISCbits.TRISC3             = 0;    // SCK
        TRISCbits.TRISC4             = 1;    // SDI
        TRISCbits.TRISC5             = 0;    // SDO
    
        SSPSTAT_REG = 0x40;
        SSPCON1_REG = 0x20;
    #endif

    //-------------------------------------------------------------------------
    // This section is required for application-specific hardware
    // initialization.
    //-------------------------------------------------------------------------

    #if defined (__18F4620)
        // D1 and D2 are on RA0 and RA1 respectively, and CS of the TC77 is on RA2.
        // Make PORTA digital I/O.
        ADCON1 = 0x0F;
    
        // Deselect the TC77 temperature sensor (RA2)
        LATA = 0x04;
    
        // Make RA0, RA1, RA2 and RA4 outputs.
        TRISA = 0xE0;
    #endif

    // Clear the RBIF flag (INTCONbits.RBIF)
    INTCONbits.RBIF = 0;

    // Enable PORTB pull-ups (INTCON2bits.RBPU)
    INTCON2bits.RBPU = 0;

    // Make the PORTB switch connections inputs.
    #if !defined(__18F4620)
        TRISDbits.TRISD7 = 0;
        TRISBbits.TRISB3 = 1;
    #endif
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
}
#endif

/*******************************************************************************
User Interrupt Handler

The stack uses some interrupts for its internal processing.  Once it is done
checking for its interrupts, the stack calls this function to allow for any
additional interrupt processing.
*******************************************************************************/
#if defined(__C30__)
void UserInterruptHandler(void)
{
}

void _ISRFAST __attribute__((interrupt, auto_psv)) _CNInterrupt(void)
{
    // *************************************************************************
    // Place any application-specific interrupt processing here
    // *************************************************************************
    // Is this an interrupt-on-change interrupt?
    if ( IFS1bits.CNIF == 1 )
    {
        // Disable further RBIF until we process it
        IEC1bits.CNIE = 0;

        // Clear mis-match condition and reset the interrupt flag
        LATD = PORTD;

        IFS1bits.CNIF = 0;
    }	
}

#else
void UserInterruptHandler(void)
{

    // *************************************************************************
    // Place any application-specific interrupt processing here
    // *************************************************************************

    // Is this an interrupt-on-change interrupt?
    if ( INTCONbits.RBIF == 1 )
    {

        // Disable further RBIF until we process it
        INTCONbits.RBIE = 0;

        // Clear mis-match condition and reset the interrupt flag
        LATB = PORTB;

        INTCONbits.RBIF = 0;
    }
}
#endif


BYTE GetHexDigit( void )
{
    BYTE    c;

    while (!ConsoleIsGetReady());
    c = ConsoleGet();
    ConsolePut(c);

    if (('0' <= c) && (c <= '9'))
        c -= '0';
    else if (('a' <= c) && (c <= 'f'))
        c = c - 'a' + 10;
    else if (('A' <= c) && (c <= 'F'))
        c = c - 'A' + 10;
    else
        c = 0;

    return c;
}

BYTE GetMACByte( void )
{
    BYTE    oneByte;

    //ConsolePutROMString( (ROM char * const) "\r\n\r\nEnter last MAC byte in hex: " );
    oneByte = GetHexDigit() << 4;
    oneByte += GetHexDigit();
    //ConsolePutROMString( (ROM char * const) "\r\n\r\n" );

    return oneByte;
}



void ProcessMenu( void )
{

    BYTE        c;

    DISABLE_WDT();

    /* Get the user's input from the keyboard. */
    c = ConsoleGet();
    ConsolePut( c );
    switch (c)
    { 
        /* Enable or Disable Joining by other devices */       
		case '1':  
        	ConsolePutROMString((ROM char * const)"\r\n0=Disable Join, 1=Enable Join : ");
        	while( !ConsoleIsGetReady());
        	c = ConsoleGet();
        	ConsolePut(c);
        	switch(c)
        	{
        		case '0':
        			params.NLME_PERMIT_JOINING_request.PermitDuration   = 0x00;
        			params.NLME_PERMIT_JOINING_request._updatePayload          = FALSE;
        			currentPrimitive = NLME_PERMIT_JOINING_request;
        			break;
        		case '1':
        			params.NLME_PERMIT_JOINING_request.PermitDuration   = 0xFF;
        			params.NLME_PERMIT_JOINING_request._updatePayload          = FALSE;
        			currentPrimitive = NLME_PERMIT_JOINING_request;
        			break;
        		
        	}   
            break;
            
        /* Request 16-bytes of data from Another Device */    			
		case '2':
	 		printf("\r\nHow many bytes are you requesting(hex): ");
	 		TxBuffer[TxData++] = GetMACByte();
	 		ZigBeeBlockTx();
	 		
            params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
            printf("\r\nWhat is the short address of device you want data from: ");
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = GetMACByte();
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] =  GetMACByte();
        	params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
        	
            params.APSDE_DATA_request.DiscoverRoute =  ROUTE_DISCOVERY_SUPPRESS;
            
            
        #ifdef I_SUPPORT_SECURITY
            params.APSDE_DATA_request.TxOptions.Val = 1;
        #else
        	params.APSDE_DATA_request.TxOptions.Val = 0;
        #endif
			params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
			params.APSDE_DATA_request.SrcEndpoint = 1;
            params.APSDE_DATA_request.DstEndpoint =  240;
            params.APSDE_DATA_request.ProfileId.Val = 0x7f01;
            params.APSDE_DATA_request.ClusterId.Val = BUFFER_TEST_REQUEST_CLUSTER;
            currentPrimitive = APSDE_DATA_request;	 
            break;		 	

		/* Request Data From a group of devices */
        case '3':
        		 		
	 		TxBuffer[TxData++] = 0x0a;      /* request 10-bytes */
	 		
			ZigBeeBlockTx();

            params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_GROUP;
            printf("\r\nPlease enter the Group ID of the Data Request: ");
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = GetMACByte();
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] =  GetMACByte();
        	params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
            params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
        #ifdef I_SUPPORT_SECURITY
            params.APSDE_DATA_request.TxOptions.Val = 1;
        #else
        	params.APSDE_DATA_request.TxOptions.Val = 0;
        #endif
			params.APSDE_DATA_request.TxOptions.bits.acknowledged = 0;
			params.APSDE_DATA_request.ProfileId.Val = 0x7f01;
            params.APSDE_DATA_request.ClusterId.Val = BUFFER_TEST_REQUEST_CLUSTER;
            currentPrimitive = APSDE_DATA_request;	 
            break;		 			
			
    
	    /* Send data directly to another device */		
		case '4':
		    printf("\r\nPlease enter the number of bytes to send (hex): ");
		    temp2 = GetMACByte();
	 		if(temp2 > 0x52)
	 		    temp2 = 0x52;  

			/* send buffer test to device with short address 0002 */
			TxBuffer[TxData++]   = temp2 + 2; // Length
			TxBuffer[TxData++]   = 0x00;                    // octet sequence 
			TxBuffer[TxData++]   = 0x00;
   			
   			/* Load the transmit buffer with the data to send */		
            for(i = 0; i < temp2; i++)
		    {
  		        TxBuffer[TxData++] = i;
			}

    	    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
       		printf("\r\nPlease enter the short address of the destination device: ");
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = GetMACByte();
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = GetMACByte();
    	    
    	    params.APSDE_DATA_request.SrcEndpoint    = 1;
            params.APSDE_DATA_request.DstEndpoint    = 240;
            params.APSDE_DATA_request.ProfileId.Val  = MY_PROFILE_ID;
    	    
       		//params.APSDE_DATA_request.asduLength; TxData
			params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
	   	    params.APSDE_DATA_request.DiscoverRoute = TRUE;
	   	    params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
	   	    
	   	    
            params.APSDE_DATA_request.DiscoverRoute =  ROUTE_DISCOVERY_SUPPRESS;
            
	   	    #ifdef I_SUPPORT_SECURITY
			    params.APSDE_DATA_request.TxOptions.Val = 1;
            #else
                params.APSDE_DATA_request.TxOptions.Val = 0;
            #endif
			params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;			        
    		params.APSDE_DATA_request.ClusterId.Val = TRANSMIT_COUNTED_PACKETS_CLUSTER;	            
    				
    		ZigBeeBlockTx();
		    currentPrimitive = APSDE_DATA_request;
		    break;
		    

		case '5':
		    printf("\r\nPlease enter the number of bytes to send (hex): ");
		    temp2 = GetMACByte();
	 		if(temp2 > 0x52)
	 		    temp2 = 0x52;  

			/* send buffer test to device with short address 0002 */
			TxBuffer[TxData++]   = temp2 + 2; // Length
			TxBuffer[TxData++]   = 0x00;                    // octet sequence 
			TxBuffer[TxData++]   = 0x00;
   			
   			/* Load the transmit buffer with the data to send */		
            for(i = 0; i < temp2; i++)
		    {
  		        TxBuffer[TxData++] = i;
			}

    	    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_GROUP;
       		printf("\r\nEnter the GroupID of devices to send data: ");
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = GetMACByte();
    	    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = GetMACByte();
    	    
    	    params.APSDE_DATA_request.SrcEndpoint    = 1;
            params.APSDE_DATA_request.DstEndpoint    = 240;
            params.APSDE_DATA_request.ProfileId.Val  = MY_PROFILE_ID;
    	    
			params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
	   	    params.APSDE_DATA_request.DiscoverRoute = TRUE;
	   	    params.APSDE_DATA_request.TxOptions.bits.acknowledged = 0;
	   	    
	   	    
            params.APSDE_DATA_request.DiscoverRoute =  ROUTE_DISCOVERY_SUPPRESS;
            
	   	    #ifdef I_SUPPORT_SECURITY
			    params.APSDE_DATA_request.TxOptions.Val = 1;
            #else
                params.APSDE_DATA_request.TxOptions.Val = 0;
            #endif
	        
    		params.APSDE_DATA_request.ClusterId.Val = TRANSMIT_COUNTED_PACKETS_CLUSTER;	            
    				
    		ZigBeeBlockTx();
		    currentPrimitive = APSDE_DATA_request;
		    break;

        
        /* Add/Remove device to/from a Group */
		case '6':
		    
		    printf("\r\n\n What Group Activity do you want to do ? \r\n"); 
		    printf("0=Add Device to a Group \r\n1=Remove Device from a Group\r\n2=Remove Device from All Groups: ");
		    
			while( !ConsoleIsGetReady());
 			c = ConsoleGet();
 			ConsolePut(c);
 			
 			if( c < '0' || c > '2' )
		        break;
		        
		        
		    switch(c)
		    {
    		    case '0':
    		    case '1':
		            printf("\r\nEnter 16-bit Group ID (Hex): ");    
		            params.APSME_ADD_GROUP_request.GroupAddress.v[1] = GetMACByte();
		            params.APSME_ADD_GROUP_request.GroupAddress.v[0] = GetMACByte();
		            
		            if(c == '0')
		            {
    		            /* Using a Fixed endpoint here to simplify things in this application */
		                params.APSME_ADD_GROUP_request.Endpoint = GROUP_ID4; 
                        currentPrimitive = APSME_ADD_GROUP_request;
                        GROUP_INDICATION    = 1;
                    }
                    else
                    {
                        params.APSME_REMOVE_GROUP_request.Endpoint = GROUP_ID4;
                        currentPrimitive = APSME_REMOVE_GROUP_request;
                        GROUP_INDICATION    = 0;
                    }    
		            break;
		            
		        case '2':
		            params.APSME_REMOVE_ALL_GROUPS_request.Endpoint = GROUP_ID4;
		            currentPrimitive = APSME_REMOVE_ALL_GROUPS_request;
		            GROUP_INDICATION    = 0;
		            break;
		            
		     }
		     break;

		
		case '7':
		    #ifdef USE_EXTERNAL_NVM
                    pCurrentNeighborRecord = neighborTable;   //+ (WORD)neighborIndex * (WORD)sizeof(NEIGHBOR_RECORD);
            #else
                    pCurrentNeighborRecord = &(neighborTable[0]);
            #endif
            printf("\r\nShort              MAC                 Type        Rntlship ");
		    for ( i=0; i < MAX_NEIGHBORS; i++ )
            {
                
                GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                if ((currentNeighborRecord.deviceInfo.bits.bInUse))
                {
                    BYTE z;
                    printf("\r\n");
                    PrintChar(currentNeighborRecord.shortAddr.byte.MSB);
                    PrintChar(currentNeighborRecord.shortAddr.byte.LSB);
                    printf("    |    ");
                    
                    for(z=0; z < 8; z++)
                       PrintChar(currentNeighborRecord.longAddr.v[7-z]);
                    
                    printf("    |     ");
                    
                    if((currentNeighborRecord.deviceInfo.bits.deviceType == 0x01))
                        printf("RTR");

                    else if((currentNeighborRecord.deviceInfo.bits.deviceType == 0x02))
                        printf("RFD");
                        
                    else if((currentNeighborRecord.deviceInfo.bits.deviceType == 0x00))
                        printf("CRD");
                       
                    else
                        printf("UKN");
                        
                    printf("    |     ");
                    if(currentNeighborRecord.deviceInfo.bits.Relationship == 0x01)
                        printf("CHILD ");
                    else if(currentNeighborRecord.deviceInfo.bits.Relationship == 0x00)
                        printf("PARENT");
                    else
                        printf("UNKWN ");      
     
                }
                #ifdef USE_EXTERNAL_NVM
                    pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD);
                #else
                    pCurrentNeighborRecord++;
                #endif
            }
            printf("\r\n");	
		    break;
		
		case '8':
					printf("\r\nPlease input two numbers with blanks:");
					/* Load the payload with the data to send */
					char sending_data = 0;
					for(i = 0; i < 3; i++)
					{
						sending_data = ConsoleGet();
						ConsolePut(sending_data);
						TxBuffer[TxData++] =sending_data; // write the data into transmit buffer
					}
					params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT; // use network address
					params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = 0x00; //MSB of Dest address
					params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = 0x01; //LSB of Dest address
					params.APSDE_DATA_request.SrcEndpoint = 171; // set the endpoints
					params.APSDE_DATA_request.DstEndpoint = 172;
					params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID; // ignore this
					//params.APSDE_DATA_request.asduLength; TxData
					params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS; // ignore this
					params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1; // usd ACK or not
					params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS; // select therouting mode
					
					/* Test Secure Send */
					params.APSDE_DATA_request.TxOptions.bits.securityEnabled = 1;

					// ignore the security
					#ifdef I_SUPPORT_SECURITY
					params.APSDE_DATA_request.TxOptions.Val = 1;
					#else
					params.APSDE_DATA_request.TxOptions.Val = 0;
					#endif
					params.APSDE_DATA_request.ClusterId.Val= 0x1111; //set the cluster ID
					ZigBeeBlockTx(); // block other transmission until finish
					currentPrimitive = APSDE_DATA_request; // give the command of transmission
					break;

		/* Request the route to a device */            
		case '9':
                    params.NLME_ROUTE_DISCOVERY_request.DstAddrMode = APS_ADDRESS_16_BIT;
                    printf("\r\nPlease enter the short address of the destination device(hex): ");
    	            params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = GetMACByte();
    	            params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = GetMACByte();
                    
                    params.NLME_ROUTE_DISCOVERY_request.Radius      = DEFAULT_RADIUS;
                   
					ZigBeeBlockTx();
					currentPrimitive = NLME_ROUTE_DISCOVERY_request;
	      		    break;	

		case '0':
					printf("\r\nPlease input string: ");
					/* Load the payload with the data to send */
					unsigned char encryptStringInput[16];

					for(i = 0; i < 16; i++)
					{
						encryptStringInput[i] = ConsoleGet();
						ConsolePut(encryptStringInput[i]);
					}

					/* String?Hex */
					//    unsigned char encryptStringInput[] = "helloworldabc";
    				printf("\r\nInput string:");
    				printf(encryptStringInput);
    				printf("\r\n");
    
    
    				//unsigned char encryptArrayInput[] = {};
    				//unsigned long len = strlen(encryptStringInput);
    
    
    				int i;
    				for(i = 0; i < 16; i++)
    				{
    				    //if(i<len){
    				    	encryptArrayInput[i] = encryptStringInput[i];
    				    //}else{
 				      		//encryptArrayInput[i] = 0;
   				    	//}
   					 }
   					 printf("\r\nstring to hex (Input array): ");
   					 printStringToHex(encryptArrayInput);
    				 printf("\r\n");
    
    
    				/* AES CBC?? */
  				    //??????
				    //    unsigned char encryptOutputArray[16];
				    //????
				    test_encrypt_cbc(encryptArrayInput);
				    //    memcpy(encryptOutputArray, test_encrypt_cbc(encryptArrayInput), 16);
 				    //???????
 				    printf("\r\nreturned encrypted data: ");
 				    printStringToHex(bufferEncrypt);
  				  	printf("\r\n");
    
    
    				
  				  	/* AES CBC?? */
/* 				  	//??????
					//    unsigned char decryptOutputArray[16];
   				 	//????
    				test_decrypt_cbc(bufferEncrypt);
					//    memcpy(decryptOutputArray, test_decrypt_cbc(bufferEncrypt), 16);
 				    //???????
  				 	printf("\r\nreturned decrypted data: ");
   				 	//printf("%s",bufferDecrypt);
					for(i = 0; i < 16; i++)
					{
						
						ConsolePut(bufferDecrypt[i]);
					}
   					printf("\r\n");
*/


					printf("\r\nSending bufferEncrypt to router...\r\n");
					for(i = 0; i < 16; i++)
					{
						
						TxBuffer[TxData++] = bufferEncrypt[i];
					}

					
					params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT; // use network address
					params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = 0x00; //MSB of Dest address
					params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = 0x01; //LSB of Dest address
					params.APSDE_DATA_request.SrcEndpoint = 171; // set the endpoints
					params.APSDE_DATA_request.DstEndpoint = 172;
					params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID; // ignore this
					//params.APSDE_DATA_request.asduLength; TxData
					params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS; // ignore this
					params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1; // usd ACK or not
					params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS; // select therouting mode
					
					/* Test Secure Send */
					params.APSDE_DATA_request.TxOptions.bits.securityEnabled = 1;

					// ignore the security
					#ifdef I_SUPPORT_SECURITY
					params.APSDE_DATA_request.TxOptions.Val = 1;
					#else
					params.APSDE_DATA_request.TxOptions.Val = 0;
					#endif
					params.APSDE_DATA_request.ClusterId.Val= 0x1111; //set the cluster ID
					ZigBeeBlockTx(); // block other transmission until finish
					currentPrimitive = APSDE_DATA_request; // give the command of transmission
					break;




        default:
            break;
    }

    PrintMenu();

    ENABLE_WDT();

}


void PrintMenu( void )
{
    ConsolePutROMString(menu);
    ConsolePutROMString( (ROM char * const) "\r\nEnter a menu choice: " );
}
