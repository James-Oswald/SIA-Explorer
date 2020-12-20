move 0 r4
move 20 r5
move 1 r6
move 0 r0
move 0 r1
push r1
move 1 r2
push r2
branchifequal r0 r5 9
add r1 r2 r3 
add r2 r4 r1
add r3 r4 r2
push r2
add r0 r6 r0
jump 8
halt