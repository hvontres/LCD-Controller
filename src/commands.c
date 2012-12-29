#include "common.h"
#include "globals.h"
#include "spi_xfer.h"
#include "commands.h"



// Command Table Entries
    tCmdLineEntry g_sCmdTable[] =
    {
    { "s", CMD_Switch, "Switch Buffers" },
    { "b", CMD_Blank, "Blank the display." },
    { "u", CMD_UnBlank, "Unblank the display." },
    { "x", CMD_Exit, "Exit MainLoop." },
    { "h", CMD_Help, "Application help." },
    {0,0,0}
    };
    
const int NUM_CMD = sizeof(g_sCmdTable)/sizeof(tCmdLineEntry);

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
CMD_Switch (int argc, char **argv)
{
  UARTprintf("\nSwitch Buffers here :)\n");
  //switch from A to B
  SwitchBuffers();
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

