	
	add $sp, $zero, $imm, 1		# set $sp = 1
	sll $sp, $sp, $imm, 11		# set $sp = 1 << 11 = 2048

	add $a0, $zero, $imm, 0		# Set argument 1 to low index =0.
	add $a1, $zero, $imm, 15	# Set argument 2 to last index.
	jal $imm, $zero, $zero, quicksort
	halt $zero, $zero, $zero, 0		# halt

swap:				#swap method

	add $sp, $sp, $imm, -3	# Make stack room for one
	sw $a1, $sp, $imm, 2		# save $s1
	sw $a0, $sp, $imm, 1		# save $s2
	sw $ra, $sp, $imm, 0		# save return address

	add $t0, $a0, $imm, 0	#t0 = a
	add $t0, $t0, $imm, 1024	#t0 = arr + a
	lw $t1, $t0, $imm, 0	#t1 = arr[a]

	add $t2, $a1, $imm, 0	#t2 = b
	add $t2, $t2, $imm, 1024	#t2 = arr + b
	lw $t3, $t2, $imm, 0	#t3 = arr[b]

	sw $t3, $t0, $imm, 0	#arr[a] = arr[b]
	sw $t1, $t2, $imm, 0	#arr[b] = arr[a]

	lw $ra, $sp, $imm, 0	# restore $ra
	lw $a0, $sp, $imm, 1	# restore $a0
	lw $a1, $sp, $imm, 2	# restore $a1
	add $sp, $sp, $imm, 3	# Restoring the stack size

	beq $ra, $zero, $zero, 0	# return to caller

partition: 			#partition method

	add $sp, $sp, $imm, -6		# adjust stack for 3 items
	sw $a0, $sp, $imm, 5		# save $a0
	sw $a1, $sp, $imm, 4		# save $a1
	sw $s0, $sp, $imm, 3		# save $s0
	sw $s1, $sp, $imm, 2		# save $s1
	sw $s2, $sp, $imm, 1		# save $s2
	sw $ra, $sp, $imm, 0		# save return address

	lw $s0, $a0, $imm, 1024		#s0 = arr[low] //pivot
	add $t0, $a0, $imm, -1		#t0, i=low -1
	add $t1, $a1, $imm, 1		#t1, j=high+1


whileloop:

L1:
	add $t1, $t1, $imm, -1		#t1, j=j-1
	lw $s1, $t1, $imm, 1024		#s1 = arr[j]
	bgt $imm, $s1, $s0, L1		#jump to L1 if arr[j]>pivot

L2:
	add $t0, $t0, $imm, 1			#t0, i=i+1
	lw $s2, $t0, $imm, 1024			#s2 = arr[i]
	blt $imm, $s2, $s0, L2			#jump to L2 if arr[i]<pivot
	bge $imm, $t0, $t1, endwhile	#jump to endwhile if i>=j
	add $a0, $t0, $imm, 0			#a0, i
	add $a1, $t1, $imm, 0			#a1, j
	jal $imm, $zero, $zero, swap	# swap A[i],A[j]
	add $t0, $a0, $imm, 0			#RESTORE VALUES t0, i
	add $t1, $a1, $imm, 0			#RESTORE VALUES t0, j
	jal $imm, $zero, $zero, whileloop 

endwhile:

	add $v0, $t1, $zero, 0		# return j
	lw $ra, $sp, $imm, 0		# restore $ra
	lw $s2, $sp, $imm, 1		# restore $s2
	lw $s1, $sp, $imm, 2		# restore $s1
	lw $s0, $sp, $imm, 3		# restore $s0
	lw $a1, $sp, $imm, 4		# restore $a1
	lw $a0, $sp, $imm, 5		# restore $a0
	add $sp, $sp, $imm, 6		# Restoring the stack size
	beq $ra, $zero, $zero, 0	# return to caller


quicksort:				#quicksort method

	add $sp, $sp, $imm, -4		# adjust stack for 4 items
	sw $s0, $sp, $imm, 3		# save $s0
	sw $ra, $sp, $imm, 2		# save return address
	sw $a1, $sp, $imm, 1		# save argument r
	sw $a0, $sp, $imm, 0		# save argument p

	bge $imm, $a0, $a1, endsort		#endsort 

	jal $imm, $zero, $zero, partition	#v0
	add $s0, $v0, $imm, 0				#s0 = v0 

	lw $a0, $sp, $imm, 0		# restore $a0
	add $a1, $s0, $imm, 0		# a1 = q
	jal $imm, $zero, $zero,	quicksort # quicksort(p,q)

	add $a0, $s0, $imm, 1		# a0 = q+1
	lw $a1, $sp, $imm, 1		# restore $a0
	jal $imm, $zero, $zero, quicksort # quicksort(q+1,r)

 endsort:

	lw $a0, $sp, $imm, 0		# restore $a0
	lw $a1, $sp, $imm, 1		# restore $a1
	lw $ra, $sp, $imm, 2		# restore $ra
	lw $s0, $sp, $imm, 3		# restore $s0
	add $sp, $sp, $imm, 4		# Restoring the stack size
	beq $ra, $zero, $zero, 0	# return to caller


	.word 1024 7
	.word 1025 6
	.word 1026 5
	.word 1027 4
	.word 1028 3
	.word 1029 2
	.word 1030 1
	.word 1031 9
	.word 1032 8
	.word 1033 7
	.word 1034 6
	.word 1035 5
	.word 1036 4
	.word 1037 3
	.word 1038 2
	.word 1039 1

