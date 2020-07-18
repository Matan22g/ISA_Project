.word 1024 14			

.word 1025 4			

.word 1026 9	
    
.word 1027 11	
    
.word 1028 12	
        
.word 1029 2

add $t0, $zero, $imm, 1024	# $t0 = 1024

	add $t2, $zero, $imm, 1038	# $t2 = 1038

L1:

	add $t1, $t0, $zero, 0		# $t1 = $t0

L2:

	lw $s0, $t1, $zero, 0       #set $s0=mem[$t1]

	lw $s1, $t1, $imm, 1		# set $s1=mem[$t1+1]

	ble $imm, $s0, $s1, Skip	# if ($s0<=$s1) skip swap
    
    
	sw $s0, $t1, $imm, 1		# swap

	sw $s1, $t1, $zero, 0		# swap

Skip:

	add $t1, $t1, $imm, 1       # $t1 = $t1 + 1

	ble $imm, $t1, $t2, L2		# if ($t1<=$t2) go to L2 

	sub $t2, $t2, $imm, 1		# $t2--

	ble $imm, $t0, $t2, L1		# if ($t0<=$t2) go to L1

	halt $zero, $zero, $zero, 0	# halt

