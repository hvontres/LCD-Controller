

#define CMDLINE_MAX_ARGS 3

//*****************************************************************************
//
// Declaration for the callback functions 
//
//*****************************************************************************
extern int CMD_Help (int argc, char **argv);
extern int CMD_Switch (int argc, char **argv);
extern int CMD_Splash (int argc, char **argv);
extern int CMD_Write (int argc, char **argv);
extern int CMD_Line (int argc, char **argv);
extern int CMD_Clear (int argc, char **argv);
extern int CMD_Blank (int argc, char **argv);
extern int CMD_Exit (int argc, char **argv);
extern int CMD_UnBlank (int argc, char **argv);
extern void WriteDisplay(void);