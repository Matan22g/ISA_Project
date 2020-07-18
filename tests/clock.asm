
	add $t2, $zero, $imm, 1		# set $t2 = 1
	sll $t2, $t2, $imm, 4		# left shift by 4 bits: 1---->10 (hex)
	add $t2, $t2, $imm, 9		# set $t2 = 10 + 9 = 19 (hex)
	sll $t2, $t2, $imm, 4		# left shift by 4 bits: 19---->190 (hex)
	add $t2, $t2, $imm, 5		# set $t2 = 190 + 5 = 195 (hex)
	sll $t2, $t2, $imm, 4		# left shift by 4 bits: 195---->1950 (hex)
	add $t2, $t2, $imm, 9		# set $t2 = 1950 + 9 = 1959 (hex)
	sll $t2, $t2, $imm, 4		# left shift by 4 bits: 1---->19590 (hex)
	add $t2, $t2, $imm, 5		# set $t2 = 19590 + 5 = 19595 (hex)
	sll $t2, $t2, $imm, 4		# left shift by 4 bits: 1---->195950 (hex)
	add $t2, $t2, $imm, 5		# set $t2 = 195950 + 5 = 195955 (hex)

	add $s0, $zero, $imm, 6		# set $s0 = 6
	out $imm, $s0, $zero, CLOCK	# set irqhandler as CLOCK
	add $s1, $zero, $imm, 256	# $s1 = 256
	out $s1, $zero, $imm, 13	# write max timer value
	add $s2, $zero, $imm, 1		# set $s2 = 1
	out $s2, $zero, $imm, 0		# enable irq0
	out $s2, $zero, $imm, 11	# enable timer

	add $s0, $zero, $imm, 2		# set $s0 = 2, for clock matching.
	add $s0, $zero, $imm, 2		# set $s0 = 2, for clock matching.
	add $s0, $zero, $imm, 2		# set $s0 = 2, for clock matching.
	add $s0, $zero, $imm, 2		# set $s0 = 2, for clock matching.

	out $t2, $zero, $imm, 10	# intial display to 195955 (hex)

LOOP:
	beq $imm, $zero, $zero, LOOP	# unconditional LOOP, untill the final led would turn off.

CLOCK:
	in $t1, $zero, $imm, 10		# read display register into $t1
	and $t2, $t1, $imm, 15		# checks if 195959 and its a new hour, if true: sets $t2 to be 9
	sub $t2, $t2, $imm, 9		# checks if $t2 is 9: $t2 = 9 - 9 = 0
	beq $imm, $zero, $t2, NEWHOUR	# if $t2=0 jump to NEWHOUR
	add $t1, $t1, $imm, 1		# incriments seconds by 1
	out $t1, $zero, $imm, 10	# write to display +1 one sec
	and $t2, $t1, $imm, 7		# checks if 200005, if true: sets $t2 to be 5, and with 111
	sub $t2, $t2, $imm, 5		# checks if $t2 is 5: $t2 = 5 - 5 = 0
	beq $imm, $zero, $t2, END	# if $t2 = 0 ---> clock = 200005 jump to END

	out $zero, $zero, $imm, 3	# clear irq0 status
	reti $zero, $zero, $zero, 0	# return from interrupt

NEWHOUR:
	sll $t3, $s0, $imm, 20		# left shift by 4 * 5 = 20 bits: 2 ----> 200000 (hex)
	out $t3, $zero, $imm, 10	# set new hour 200000 (hex)

	out $zero, $zero, $imm, 3	# clear irq0 status
	reti $zero, $zero, $zero, 0	# return from interrupt

END:
	halt $zero, $zero, $zero, 0	# halt