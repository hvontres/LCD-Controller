EESchema Schematic File Version 2  date Sun 04 Nov 2012 10:40:06 AM PST
LIBS:Custom_Parts
LIBS:BoosterPack
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:maxim-7
LIBS:maxim-7a
LIBS:Lcd Booster-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 2
Title ""
Date "3 nov 2012"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	9650 4800 9400 4800
$Comp
L GND #PWR01
U 1 1 5080A57E
P 9400 4600
F 0 "#PWR01" H 9400 4600 30  0001 C CNN
F 1 "GND" H 9400 4530 30  0001 C CNN
	1    9400 4600
	0    1    1    0   
$EndComp
Connection ~ 9500 4600
$Comp
L TI_BOOSTER_J1 J1
U 1 1 5080AA0E
P 9800 2750
F 0 "J1" H 9500 4050 60  0000 C CNN
F 1 "J1-J2" H 9700 1500 60  0000 C CNN
	1    9800 2750
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 5080AA99
P 9050 3800
F 0 "#PWR02" H 9050 3800 30  0001 C CNN
F 1 "GND" H 9050 3730 30  0001 C CNN
	1    9050 3800
	0    1    1    0   
$EndComp
$Sheet
S 6450 2000 1000 900 
U 5080F862
F0 "LcdDriver" 60
F1 "LcdDriver.sch" 60
F2 "Reset" I R 7450 2150 60 
F3 "Blank" I R 7450 2300 60 
F4 "SDI" I R 7450 2450 60 
F5 "SDO" O R 7450 2600 60 
F6 "SCLK" O R 7450 2750 60 
$EndSheet
Wire Wire Line
	7450 2750 8650 2750
Wire Wire Line
	8650 2750 8650 2150
Wire Wire Line
	8650 2150 9050 2150
Wire Wire Line
	7450 2600 8450 2600
Wire Wire Line
	8450 2600 8450 2350
Wire Wire Line
	8450 2350 9050 2350
Wire Wire Line
	7450 2450 8000 2450
Wire Wire Line
	8000 2450 8000 2250
Wire Wire Line
	8000 2250 9050 2250
Wire Wire Line
	9050 1650 7750 1650
Wire Wire Line
	7750 1650 7750 2300
Wire Wire Line
	7750 2300 7450 2300
Wire Wire Line
	7450 2150 8250 2150
Wire Wire Line
	8250 2150 8250 2050
Wire Wire Line
	8250 2050 9050 2050
$Comp
L +3.3V #PWR03
U 1 1 5089E60E
P 9050 3700
F 0 "#PWR03" H 9050 3660 30  0001 C CNN
F 1 "+3.3V" H 9050 3810 30  0000 C CNN
	1    9050 3700
	0    -1   -1   0   
$EndComp
$Comp
L CONN_2 P2
U 1 1 508A340C
P 10000 4700
F 0 "P2" V 9950 4700 40  0000 C CNN
F 1 "CONN_2" V 10050 4700 40  0000 C CNN
	1    10000 4700
	1    0    0    1   
$EndComp
Wire Wire Line
	9650 4600 9400 4600
$Comp
L +5V #PWR04
U 1 1 508A34CF
P 9400 4800
F 0 "#PWR04" H 9400 4890 20  0001 C CNN
F 1 "+5V" H 9400 4890 30  0000 C CNN
	1    9400 4800
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
