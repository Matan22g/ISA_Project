	
	add $s0, $zero, $imm, 6		# set $s0 = 6
	out $imm, $s0, $zero, LEDS	# set irqhandler as LEDS
	add $s1, $zero, $imm, 256	# $s1 = 256
	out $s1, $zero, $imm, 13	# write max timer value
	add $s2, $zero, $imm, 1		# set $s2 = 1
	out $s2, $zero, $imm, 0		# enable irq0
	out $s2, $zero, $imm, 11	# enable timer

LOOP:
	beq $imm, $zero, $zero, LOOP	# unconditional LOOP, untill the final led would turn off.

LEDS:
	in $t1, $zero, $imm, 9		# read leds register into $t1
	beq $imm, $zero, $t1, FIRSTLED	# if leds=0 jump to FIRSTLED
	sll $t1, $t1, $imm, 1		# left shift led pattern to the left
	out $t1, $zero, $imm, 9		# write the new led pattern
	beq $imm, $zero, $t1, END	# if leds=0 jump to END
	out $zero, $zero, $imm, 3	# clear irq0 status
	reti $zero, $zero, $zero, 0	# return from interrupt

FIRSTLED:
	add $t2, $zero, $imm, 1		# set $t2 = 1
	out $t2, $zero, $imm, 9		# turn on the first led
	out $zero, $zero, $imm, 3	# clear irq0 status
	reti $zero, $zero, $zero, 0	# return from interrupt

END:
	halt $zero, $zero, $zero, 0	# halt

