;Test02.asm
;Name : E V Sai Chathurya
;Roll No. : 2401CS84
;Declaration of Authorship : This program is my own work and has not been copied from any source

; Test error handling 
label: 
label: ; duplicate label definition 
br nonesuch ; no such label 
ldc 08ge ; not a number 
ldc ; missing operand 
add 5 ; unexpected operand 
ldc 5, 6; extra on end of line 
0def: ; bogus label name 
fibble; bogus mnemonic 
0def ; bogus mnemonic