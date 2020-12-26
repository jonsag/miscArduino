EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L My_Arduino:Arduino_Mini_Pro U2
U 1 1 5FE12205
P 4950 3500
F 0 "U2" H 5700 3981 50  0000 C CNN
F 1 "Arduino_Mini_Pro" H 5700 3890 50  0000 C CNN
F 2 "My_Arduino:Arduino_Pro_Mini_vC_larger_pads" H 5750 1600 50  0001 C CNN
F 3 "" H 5200 3650 50  0001 C CNN
	1    4950 3500
	1    0    0    -1  
$EndComp
$Comp
L My_Headers:3-pin_double_NO_switch_header J2
U 1 1 5FE1E30B
P 7850 4700
F 0 "J2" H 8013 4581 50  0000 L CNN
F 1 "3-pin_double_NO_switch_header" H 7850 4325 50  0001 C CNN
F 2 "My_Headers:3-pin_double_NO_switch_header_large" H 7900 4225 50  0001 C CNN
F 3 "~" H 7850 4700 50  0001 C CNN
	1    7850 4700
	1    0    0    -1  
$EndComp
$Comp
L My_Headers:2-pin_power_input_header J1
U 1 1 5FE1ED1F
P 6750 3000
F 0 "J1" V 6713 2842 50  0000 R CNN
F 1 "2-pin_power_input_header" H 6750 2800 50  0001 C CNN
F 2 "My_Headers:2-pin_power_input_header_larger_pads" H 6800 2700 50  0001 C CNN
F 3 "~" H 6750 3000 50  0001 C CNN
	1    6750 3000
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6450 3600 6850 3600
Wire Wire Line
	6850 3600 6850 4700
Wire Wire Line
	6850 4700 7650 4700
Wire Wire Line
	6850 3600 6850 3200
Connection ~ 6850 3600
Wire Wire Line
	6750 3200 6750 3500
Wire Wire Line
	6750 3500 6450 3500
Wire Wire Line
	7650 4800 6750 4800
Wire Wire Line
	6750 4800 6750 4100
Wire Wire Line
	6750 4100 6450 4100
Wire Wire Line
	7650 4900 6650 4900
Wire Wire Line
	6650 4900 6650 4200
Wire Wire Line
	6650 4200 6450 4200
Text Notes 8250 4850 0    50   ~ 0
Reset
Text Notes 8250 4950 0    50   ~ 0
Dice button
Wire Wire Line
	4250 4500 4950 4500
Wire Wire Line
	4250 4400 4950 4400
Wire Wire Line
	4250 4300 4950 4300
Wire Wire Line
	4250 4200 4950 4200
Wire Wire Line
	4250 4100 4950 4100
Wire Wire Line
	4250 4000 4950 4000
Wire Wire Line
	4250 3900 4950 3900
$Comp
L Device:R R1
U 1 1 5FE1BAA3
P 3450 3150
F 0 "R1" H 3520 3196 50  0000 L CNN
F 1 "R" H 3520 3105 50  0000 L CNN
F 2 "My_Misc:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal_larger_pads" V 3380 3150 50  0001 C CNN
F 3 "~" H 3450 3150 50  0001 C CNN
	1    3450 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 3900 3450 3900
Wire Wire Line
	3450 3900 3450 3800
Wire Wire Line
	3650 3800 3450 3800
Connection ~ 3450 3800
Wire Wire Line
	3450 3800 3450 3300
Wire Wire Line
	3450 3000 3450 2450
Wire Wire Line
	3450 2450 7250 2450
Wire Wire Line
	7250 2450 7250 3800
Wire Wire Line
	7250 3800 6450 3800
Text Notes 6650 2800 0    50   ~ 0
Power IN
$Comp
L My_Misc:JSD-5613Bx U1
U 1 1 5FE220DD
P 3950 4200
F 0 "U1" H 3950 3533 50  0000 C CNN
F 1 "JSD-5613Bx" H 3950 3624 50  0000 C CNN
F 2 "My_Misc:7SegmentLED_JSD-5613xx_large" H 3950 3600 50  0001 C CNN
F 3 "http://www.kingbright.com/attachments/file/psearch/000/00/00/KCSA02-107(Ver.10A).pdf" H 3450 4675 50  0001 L CNN
	1    3950 4200
	-1   0    0    1   
$EndComp
Text Label 6850 3600 0    50   ~ 0
GND
$EndSCHEMATC
