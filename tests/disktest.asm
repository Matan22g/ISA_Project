	
	add $s0, $zero, $imm, 1000	# set diskbuffer  = 1000 the address we want to write to from the sector
	add $s1, $zero, $imm, 0		# $s1 = 0  - sector to read from
	add $s2, $zero, $imm, 0 	# $s2 = 0 , 0 FOR READ, 1 FOR WRITE.


LOOP:
	in $t0, $zero, $imm, 17		# checking disk status
	add $t1, $zero, $imm, 1 	# set $t1 = 1, for comapring with status

	beq $imm, $t1, $t0, LOOP	# if status = 1 ---> BUSY ---> LOOP
	beq $imm, $zero, $s2, READ	# if $s1 = 0 ---> READ MODE GOTO READ
	beq $imm, $t1, $s2, WRITE	# if $s1 = 1 ---> WRITE MODE GOTO WRITE

READ:
	add $t1, $zero, $imm, 1512 	# set $t1 = 1512, stop condition
	add $t2, $zero, $imm, 1 	# set $t2 = 1, read/write indicator

	out $s1, $zero, $imm, 15	# write to which sector we want to read from
	out $s0, $zero, $imm, 16	# write to which address we want to write it 
	out $t2, $zero, $imm, 14	# write to diskcmd command to read from sector $s1 to address $s0  
	add $s0, $s0, $imm, 128		# diskbuffer  += 128 
	add $s1, $s1, $imm, 1		# $s1 += 1  - sector to read from
	blt $imm, $s0, $t1 , LOOP	# if diskbuffer < 1512 GO TO LOOP

	add $s2, $zero, $imm, 1	 	# else done reading, swich to writing --> $s2=1
	add $s1, $zero, $imm, 4		# set sector target to 4 
	sub $s0, $s0, $imm, 512		# restore diskbuffer to 1000 for writing
	beq $imm, $zero, $zero, LOOP	# unconditional LOOP.

WRITE:
	add $t1, $zero, $imm, 1512 	# set $t1 = 1512, stop condition
	add $t2, $zero, $imm, 2 	# set $t2 = 2, read/write indicator

	out $s1, $zero, $imm, 15	# write to which sector we want to write to
	out $s0, $zero, $imm, 16	# write to which address we want to read it 
	out $t2, $zero, $imm, 14	# write to diskcmd - 2 for writing
	add $s0, $s0, $imm, 128		# diskbuffer  += 128 
	add $s1, $s1, $imm, 1		# $s1 += 1  - sector to write to
	blt $imm, $s0, $t1 , LOOP	# if diskbuffer < 1512 GO TO LOOP
	
	halt $zero, $zero, $zero, 0	# else stop, all content has been copied.
