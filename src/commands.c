#include "common.h"
#include "globals.h"
//*****************************************************************************
// Serial Command Handler
//*****************************************************************************

//*****************************************************************************
// Copyright (c) 2012, Henry von Tresckow
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.

// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY HENRY VON TRESCKOW "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL {{THE COPYRIGHT HOLDER OR CONTRIBUTORS}} BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
//*****************************************************************************

#include "spi_xfer.h"
#include "commands.h"

#include "blank.h"
#include "dp2.h"

static unsigned char Current_Line=0;

// Command Table Entries
    tCmdLineEntry g_sCmdTable[] =
    {
    { "s", CMD_Splash, "Draw Splash Image" },
    { "w", CMD_Write, "Write String to Buffer" },
    { "l", CMD_Line, "Set Current Line Number" },
    { "c", CMD_Clear, "Clear Display and set current line to 0" },
    { "b", CMD_Blank, "Blank the display." },
    { "u", CMD_UnBlank, "Unblank the display." },
    { "x", CMD_Exit, "Exit MainLoop." },
    { "h", CMD_Help, "Application help." },
    {0,0,0}
    };
    
const int NUM_CMD = sizeof(g_sCmdTable)/sizeof(tCmdLineEntry);
const char BlankLine[54]={[0 ... 52] = 32};

//*****************************************************************************
//
// Command: help
//
// Print the help strings for all commands.
//
//*****************************************************************************
int
CMD_Help (int argc, char **argv)
{
    int index;
    
    (void)argc;
    (void)argv;
    
    UARTprintf("\n");
    for (index = 0; index < NUM_CMD; index++)
    {
      UARTprintf("%17s %s\n\n",
        g_sCmdTable[index].pcCmd,
        g_sCmdTable[index].pcHelp);
    }
    UARTprintf("\n"); 
    
    return (0);
}




int
CMD_Clear (int argc, char **argv)
{
  Current_Line=0;
  GrImageDraw(&sDisplayContext, g_pucBlank,0,0);
  for (int line=0;line<30;line++) {
      g_ucScreenText[line][0]=0;
    }
  UARTprintf("1%s1\n",BlankLine);
  return (0);
}

int
CMD_Splash (int argc, char **argv)
{
  Current_Line=0;
  GrImageDraw(&sDisplayContext, g_pucSplash,0,0);
  return (0);
}

int
CMD_Line (int argc, char **argv)
{
  Current_Line=atoi(argv[1]);
  return (0);
}
int
CMD_Exit (int argc, char **argv)
{
  UARTprintf("\nExit here :)\n");
  g_ucExit=1;
  return (0);
}

int
CMD_Blank (int argc, char **argv)
{
  //Turn Off SSI2 Xfers
  DisableSSI2Transfer();
  //Blank Display
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
  return (0);
}

int
CMD_UnBlank (int argc, char **argv)
{
  //Set BLANK High
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
  //Reboot MSP430
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0);
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);
  // Setup SSI2 Xfers
  InitSSI2Transfer();
  // Release Blank Pin after short delay
  SysCtlDelay(SysCtlClockGet() / 20 / 3);
  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0);
  return (0);
}

int 
CMD_Write (int argc, char **argv)
{
  char RedrawAll=0;
  char *Input;
  //UARTprintf("argc: %i\n",argc);
  Input=argv[1];
  while(*Input){
    if (*Input=='\t'){
      *Input=' ';
    }
    Input++;
  }
    
  if (Current_Line >29){
    for (int line=1;line<30;line++) {
      strncpy (g_ucScreenText[line-1] , g_ucScreenText[line], sizeof(g_ucScreenText[line-1]) );
    }
    Current_Line=29;
    RedrawAll=1; 
  }
  //UARTprintf("%i: %s \n",Current_Line,argv[1]);
  strncpy (g_ucScreenText[Current_Line] , BlankLine, sizeof(g_ucScreenText[Current_Line]) );
  strncpy (g_ucScreenText[Current_Line] , argv[1], sizeof(g_ucScreenText[Current_Line]) );
  //UARTprintf("%s \n",g_ucScreenText[Current_Line]);
  if (RedrawAll){
    WriteDisplay(); //Refresh Whole Screen during scrolling
  }
  else { // Redraw Current Line Only
    GrStringDraw(&sDisplayContext,BlankLine,53,0,Current_Line*8,1); //Blank line
    GrStringDraw(&sDisplayContext,g_ucScreenText[Current_Line],53,0,Current_Line*8,1); //Update Line
  }
  Current_Line++;
  return (0);
}


void
WriteDisplay(void)
{
  //Clear Screen
  GrImageDraw(&sDisplayContext, g_pucBlank,0,0);
  for (int line=0;line<30;line++) {
      
    GrStringDraw(&sDisplayContext,g_ucScreenText[line],53,0,line*8,1);
    }
}