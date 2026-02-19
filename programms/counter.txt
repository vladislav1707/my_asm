IMM 1           # ввод 1
MOV R0 R1       # из 0 в 1
IMM 0           # ввод 0 (необязательно, на всякий случай просто)
MOV R0 R2       # из 0 в 2

label cycleStart
ADD             # добавить
MOV R3 R2       # из 3 в 2
MOV R3 OUT1     # из 3 в OUT1
IMM cycleStart
JMP             # конец цикла
