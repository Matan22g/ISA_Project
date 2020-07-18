
		add $sp, $zero, $imm, 1			# set $sp = 1
		sll $sp, $sp, $imm, 11		# set $sp = 1 << 11 = 2048

		add $sp, $sp, $imm, -3			# adjust stack for 3 items
		sw $s0, $sp, $imm, 2			# save $s0
		sw $s1, $sp, $imm, 1			# save $s1
		sw $s2, $sp, $imm, 0			# save $s2

		add $s0, $zero, $imm, 256		# $s0 = &M1[0][0], 0x100 =256
		add $s1, $zero, $imm, 272		# $s1 = &M2[0][0], 0x110 =272
		add $s2, $zero, $imm, 16		# $s2 = 16
		add $t0, $zero, $imm, 0			# i = 0

		
whileloop:

		add $t1, $s0, $t0, 0		# $t1 = & M1 + i
		add $t2, $s1, $t0, 0		# $t2 = & M2 + i

		lw $t1, $t1, $imm, 0		# $t1 = M1[row][i]
		lw $t2, $t2, $imm, 0		# $t2 = M2[row][i]

		add $t2, $t1, $t2, 0		# $t2= M1[row][i] + M2[row][i]
		sw $t2, $t0, $imm, 288		# store M3[row][i], 0x120 =288
		 
		add $t0, $t0, $imm, 1		# i++
		blt $imm, $t0, $s2, whileloop	# jump to whileloop if i <16 - while  i<16

		lw $s2, $sp, $imm, 0			# restore $s2
		lw $s1, $sp, $imm, 1			# restore $s1
		lw $s0, $sp, $imm, 2			# restore $s0
		add $sp, $sp, $imm, 3			# Restoring the stack size
		halt $zero, $zero, $zero, 0		# halt		

		.word 0x100 1
		.word 0x101 2
		.word 0x102 3
		.word 0x103 4
		.word 0x104 5
		.word 0x105 6
		.word 0x106 7
		.word 0x107 8
		.word 0x108 9
		.word 0x109 10
		.word 0x10A 11
		.word 0x10B 12
		.word 0x10C 13
		.word 0x10D 14
		.word 0x10E 15
		.word 0x10F 16
	
		.word 0x110 1
		.word 0x111 1
		.word 0x112 1
		.word 0x113 1
		.word 0x114 1
		.word 0x115 1
		.word 0x116 1
		.word 0x117 1
		.word 0x118 1
		.word 0x119 1
		.word 0x11A 1
		.word 0x11B 1
		.word 0x11C 1
		.word 0x11D 1
		.word 0x11E 1
		.word 0x11F 1