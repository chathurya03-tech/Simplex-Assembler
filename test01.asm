;Test01.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source

label: ; an unused label 
        ldc 0 
        ldc -5 
        ldc +5 
loop: br loop ; an infinite loop 
br next ;offset should be zero 
next: 
        ldc loop ; load code address 
        ldc var1 ; forward ref 
var1: data 0 ; a variable