(testing rapid movements)
(machine setup)
M05 S600
M09
M23
M49
M69

(turn servos ON)
M17

(control setup)
G15
G17
G23
G40
G49
G50
G69

(in mm)
G21

(absolute coordinates)
G90
G00 X0.000 Y0.000 Z0.000
G00 Y10.000
G00 X10.000
X10.000
G00 Z10.000
Z10.000
Z10.000
G00 X0.000 Y0.000 Z0.000

(incremental coordinates)
G91
G00 X0.000 Y0.000 Z0.000
G00 Y10.000
G00 X10.000
X10.000
G00 Z10.000
Z10.000
Z10.000
G00 X0.000 Y0.000 Z0.000


(and in inch)
G20

(absolute coordinates)
G90
G00 X0.000 Y0.000 Z0.000
G00 Y10.000
G00 X10.000
X10.000
G00 Z10.000
Z10.000
Z10.000
G00 X0.000 Y0.000 Z0.000

(incremental coordinates)
G91
G00 X0.000 Y0.000 Z0.000
G00 Y10.000
G00 X10.000
X10.000
G00 Z10.000
Z10.000
Z10.000
G90 G00 X0.000 Y0.000 Z0.000


(and finally, for something more exotic)
G16 G21
G00 X10.000 Y45
Y90

M02