 lw   0 1 start
 lw   0 2 end
 lw   0 3 one
 lw   0 4 zero
loop add  4 1 4
 add  1 3 1
 beq  1 2 last
 beq  0 0 loop
last add  4 2 4    
 sw   0 4 res    
 halt
start .fill 22
end .fill 33
one .fill 1
zero .fill 0
res .fill 0