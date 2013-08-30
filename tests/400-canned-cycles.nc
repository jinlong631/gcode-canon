(testing canned cycles)
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

(following code taken from the official RS274NGC spec)
G17 M3 S1234 F600 (START IN XY-PLANE)
G81 X3 Y4 R0.2 Z-1.1
X1 Y0 R0 G91 L3 (THREE MORE G81’S AN INCH APART)
Y-2 R0.1 (ONE MORE G81)
G82 G90 X4 Y5 R0.2 Z-1.1 P0.6
X2 Z-3.0 (ONE MORE G82)
G91 X-2 Y2 R0 L4 (FOUR MORE G82’S)
G83 G90 X5 Y6 R0.2 Z-1.1 Q0.21
G84 X6 Y7 R0.2 Z-1.1
G85 X7 Y8 R0.2 Z-1.1
G86 X8 Y9 R0.2 Z-1.1 P902.61
G87 X9 Y10 R0.2 Z-1.1 I0.231 J-0 K-3
G91 X1 R0.2 Z-1.1 I0.231 J-0 K-3
G88 X10 Y11 R0.2 Z-1.1 P0.3333
G89 X11 Y12 R0.2 Z-1.1 P1.272
M4 (RUN SPINDLE COUNTERCLOCKWISE)
G86 X8 Y9 R0.2 Z-1.1 P902.61
G87 X9 Y10 R0.2 Z-1.1 I0.231 J-0 K-3
G88 X10 Y11 R0.2 Z-1.1 P0.3333

M2 (THE END)
