.SUBCKT LF353A  13  15  20  5  7
*Pinorder -In +In Out V+ V-
* Project LF353A_SCH
* Powerview Wirelist Created with Version 5.2
* Inifile   : wspice.ini
* Options   : -p -f -# -w 
* Levels    : 
* 

D3       1   2   DLIMIT
E1I111   3   4   5   4   1
E1I112   4   6   4   7   1
VLIM2    8   6   1.8
VLIM1    3   9   1.7
DLIM2    8   10  DLIMIT
DLIM1    11  9   DLIMIT
RLIM1    11  12  2.5
RLIM2    10  12  2.5
I1I2     7   13  DC 105P
G1I20    14  4   15  13  1.9E-4
E1I21    16  4   12  4   1
CI2      13  7   2.7P
CDOMPOL  12  4   2.1E-9
R2       17  18  10K
C1       17  4   13P
C2       18  4   1.3P
E1I38    19  4   18  4   1
ROUT     19  20  50
RDOMPOL  12  4   1E7
R1       16  17  1K
E1I55    4   7   21  7   1
RSIG1    5   21  1E6
RSIG2    21  7   1E6
RRPS     5   1   100K
G1I60    5   7   1   7   2.4E-3
CI1      7   15  3P
I1I8     7   15  DC 100P
RG1      14  4   1E4
G1I85    12  4   14  4   .021
D1       4   14  DLIMIT
D2       14  4   DLIMIT
D4       2   7   DLIMIT

* DICTIONARY 22
* $1N72 = 21
* OUT = 20
* $1N41 = 19
* $1N46 = 18
* $1N49 = 17
* $1N52 = 16
* INNON = 15
* $1N13 = 14
* ININV = 13
* $1N25 = 12
* $1N145 = 11
* $1N153 = 10
* $1N143 = 9
* $1N155 = 8
* V- = 7
* $1N152 = 6
* V+ = 5
* $1N81 = 4
* $1N121 = 3
* $1N102 = 2
* $1N104 = 1
* GND = 0

.model dlimit D(IS=5E-15 N=1.0 CJO=0.0 )

.ENDS
