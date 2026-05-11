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
L kendryte:K210 U3
U 1 1 63572052
P 2150 3500
F 0 "U3" H 1875 4215 50  0000 C CNN
F 1 "K210" H 1875 4124 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 1900 4100 50  0001 C CNN
F 3 "" H 1900 4100 50  0001 C CNN
	1    2150 3500
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Motion:BMI160 U2
U 1 1 6357A218
P 6800 1700
F 0 "U2" H 6750 2281 50  0000 C CNN
F 1 "BMI160" H 6750 2190 50  0000 C CNN
F 2 "Package_LGA:Bosch_LGA-14_3x2.5mm_P0.5mm" H 6800 1700 50  0001 C CNN
F 3 "https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMI160-DS000.pdf" H 6100 2550 50  0001 C CNN
	1    6800 1700
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 2 1 6357DB44
P 3900 2200
F 0 "U3" H 3900 2815 50  0000 C CNN
F 1 "K210" H 3900 2724 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 3650 2800 50  0001 C CNN
F 3 "" H 3650 2800 50  0001 C CNN
	2    3900 2200
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 3 1 63582589
P 3500 3300
F 0 "U3" H 3808 3865 50  0000 C CNN
F 1 "K210" H 3808 3774 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 3250 3900 50  0001 C CNN
F 3 "" H 3250 3900 50  0001 C CNN
	3    3500 3300
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 4 1 63587FD4
P 3550 4400
F 0 "U3" H 3933 5065 50  0000 C CNN
F 1 "K210" H 3933 4974 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 3300 5000 50  0001 C CNN
F 3 "" H 3300 5000 50  0001 C CNN
	4    3550 4400
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 5 1 6358E412
P 3900 5200
F 0 "U3" H 3875 5715 50  0000 C CNN
F 1 "K210" H 3875 5624 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 3650 5800 50  0001 C CNN
F 3 "" H 3650 5800 50  0001 C CNN
	5    3900 5200
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 6 1 63593BD5
P 3900 6200
F 0 "U3" H 3875 6715 50  0000 C CNN
F 1 "K210" H 3875 6624 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 3650 6800 50  0001 C CNN
F 3 "" H 3650 6800 50  0001 C CNN
	6    3900 6200
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 7 1 6359A148
P 5350 5100
F 0 "U3" H 5325 5515 50  0000 C CNN
F 1 "K210" H 5325 5424 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 5100 5700 50  0001 C CNN
F 3 "" H 5100 5700 50  0001 C CNN
	7    5350 5100
	1    0    0    -1  
$EndComp
$Comp
L kendryte:K210 U3
U 8 1 6359FCBE
P 5350 6200
F 0 "U3" H 5325 6665 50  0000 C CNN
F 1 "K210" H 5325 6574 50  0000 C CNN
F 2 "footprints:Kendryte_K210" H 5100 6800 50  0001 C CNN
F 3 "" H 5100 6800 50  0001 C CNN
	8    5350 6200
	1    0    0    -1  
$EndComp
$Comp
L ISP1807:ISP1807-LR-RS U1
U 1 1 635AAA2A
P 8100 1250
F 0 "U1" H 9375 1507 60  0000 C CNN
F 1 "ISP1807-LR-RS" H 9375 1401 60  0000 C CNN
F 2 "ISP1807-LR-RS" H 9350 1340 60  0001 C CNN
F 3 "" H 8100 1250 60  0000 C CNN
	1    8100 1250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x24 J1
U 1 1 635D8B26
P 6850 4700
F 0 "J1" H 6930 4692 50  0000 L CNN
F 1 "Conn_01x24" H 6930 4601 50  0000 L CNN
F 2 "Connector_FFC-FPC:Hirose_FH12-24S-0.5SH_1x24-1MP_P0.50mm_Horizontal" H 6850 4700 50  0001 C CNN
F 3 "~" H 6850 4700 50  0001 C CNN
	1    6850 4700
	1    0    0    -1  
$EndComp
$Comp
L ch552t:CH552T U4
U 1 1 635F81A0
P 6600 2850
F 0 "U4" H 6600 3165 50  0000 C CNN
F 1 "CH552T" H 6600 3074 50  0000 C CNN
F 2 "Package_SO:TSSOP-20_4.4x6.5mm_P0.65mm" H 6600 2850 50  0001 C CNN
F 3 "https://cdn.hackaday.io/files/1696717259204064/CH552%20Datasheet_C111367.zh-CN.en.pdf" H 6600 2850 50  0001 C CNN
	1    6600 2850
	1    0    0    -1  
$EndComp
$Comp
L Connector:USB_C_Receptacle_USB2.0 J2
U 1 1 63606FA1
P 1300 1500
F 0 "J2" H 1407 2367 50  0000 C CNN
F 1 "USB_C_Receptacle_USB2.0" H 1407 2276 50  0000 C CNN
F 2 "Connector_USB:USB_C_Receptacle_XKB_U262-16XN-4BVC11" H 1450 1500 50  0001 C CNN
F 3 "https://www.usb.org/sites/default/files/documents/usb_type-c.zip" H 1450 1500 50  0001 C CNN
	1    1300 1500
	1    0    0    -1  
$EndComp
$Comp
L GD25LQ64:GD25LQ U5
U 1 1 635A09DA
P 3850 7250
F 0 "U5" H 3825 7715 50  0000 C CNN
F 1 "GD25LQ" H 3825 7624 50  0000 C CNN
F 2 "footprints:SOIC127P790X216-8N" H 3850 7250 50  0001 C CNN
F 3 "https://www.digikey.com/en/products/detail/gigadevice-semiconductor-hk-limited/GD25LQ64CSIGR/9484705" H 3850 7250 50  0001 C CNN
	1    3850 7250
	1    0    0    -1  
$EndComp
$EndSCHEMATC
