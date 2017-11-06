;-------------------------------------------------------------------------------
;
; (AMULET3) RAM test program
;
; SBF 22 Aug 1999
; ST  20 Nov 1999
; ST  18 Feb 2010 - adapted for use in SpiNNaker
;
; see "Static RAM Testing Application Note" v1.0
; (May 1997, VLSI Technology) for background to algorithm
;
;;; RAM test program runs in RAM, so only part of RAM can
;;; be tested, then program moved and the rest tested.
;
; Address pattern is 'March C+' (section 3.2.6):
; +/-(w0); +(r0,w1,r1); +(r1,w0,r0);
;          -(r0,w1,r1); -(r1,w0,r0).
; 
; 0,1 patterns are 32-bit true and inverse sets:
;
; 0000000000...0000 1111111111...1111
; 0101010101...0101 1010101010...1010
; 0011001100...0011 1100110011...1100
; 0000111100...1111 1111000011...0000
; 0000000011...1111 1111111100...0000
; 0000000000...1111 1111111111...0000
;
;;; AMULET3H RAM is quad-word oriented, so test uses
;;;   quad-word read and writes (using LDM, STM)
;
;
;;; Note this program cannot exit via "test_fail" & "test_pass" as the
;;; code at these points is overwritten by the test. Exit at "ram_fail"
;;; and "ram_pass" instead;
;
;
;-------------------------------------------------------------------------------
;
		preserve8
;
 		area 	ramtest, readonly, code
;
		export	ram_test_c
		export	ram_test
		
;
;
; C Language interface
;
;	uint ram_test_c (uint *start, uint *end, uint mask);
;
; Returns 0 if OK, non-zero for error
;
; If called from C use the interface below to preserve registers.
; Only do this if you know that your stack memory is working!!
;
;
ram_test_c	push	{r4-r10, lr}
		bl	ram_test
		pop	{r4-r10, pc}
;
;
;-------------------------------------------------------------------------------
;
;
; Assembly language interface
;
; r0 - scratch/return value
; r1 - end address
; r2 - mask
; r3 - pattern pointer
; r4 - start address
; r5 - memory pointer
; r6 - TRUE data pattern
; r7 - INVERSE data pattern
; r8 - TRUE copy
; r9 - INVERSE copy
; r10 - scratch
;
; Returns 0 in r0 if OK, non-zero for error
;
;
ram_test	adr	r3, first_pat		; Point to first pattern
		mov	r4, r0			; Move start address
;
pat_loop	adr	r0, pat_end		; Point past last pattern
		subs	r0, r0, r3		; and compare with pointer
		bxeq	lr  			; Return (0) if same
;
		ldr	r6, [r3], #4		; Get next pattern
		mvn	r7, r6			; Inverse copy
		and	r6, r6, r2		; True data masked
		and	r7, r7, r2		; Inverse data masked
;
		mov	r8, r6			; Make copies of true
		mov	r9, r7			; and inverse
;
; +(w0)
;
		mov	r5, r4
;
11		stmia	r5!, {r6, r8}		; background fill
		cmp	r5, r1
		blt	%b11
;
; +(r0,w1,r1)
;
		mov	r5, r4
;
22		ldmia	r5, {r0, r10}		; read back
		subs	r0, r6, r0
		subseq	r0, r6, r10
		bxne	lr
;
		stmia	r5, {r7, r9}		; store inverse pattern
		ldmia	r5!, {r0, r10}		; read back
		subs	r0, r7, r0
		subseq	r0, r7, r10
		bxne	lr
;
		cmp	r5, r1
		blt	%b22

; +(r1,w0,r0)

		mov	r5, r4
;
33		ldmia	r5, {r0, r10}		; read back
		subs	r0, r7, r0
		subseq	r0, r7, r10
		bxne	lr
;
		stmia	r5, {r6, r8}		; store background pattern
		ldmia	r5!, {r0, r10}		; read back
		subs	r0, r6, r0
		subseq	r0, r6, r10
		bxne	lr
;
		cmp	r5, r1
		blt	%b33
;
; -(r0,w1,r1)
;
		mov	r5, r1
;
44		ldmdb	r5, {r0, r10}		; read back
		subs	r0, r6, r0
		subseq	r0, r6, r10
		bxne	lr
;
		stmdb	r5, {r7, r9}		; store inverse pattern
		ldmdb	r5!, {r0, r10}		; read back
		subs	r0, r7, r0
		subseq	r0, r7, r10
		bxne	lr
;
		cmp	r5, r4
		bgt	%b44
;
; -(r1,w0,r0).
;	
		mov	r5, r1
;
55		ldmdb	r5, {r0, r10}		; read back
		subs	r0, r7, r0
		subseq	r0, r7, r10
		bxne	lr
;
		stmdb	r5, {r6, r8}		; store background pattern
		ldmdb	r5!, {r0, r10}		; read back
		subs	r0, r6, r0
		subseq	r0, r6, r10
		bxne	lr
;
		cmp	r5, r4
		bgt	%b55
;
		
		b	pat_loop
;
;
first_pat	dcd	0x00000000
		dcd	0x55555555
		dcd	0x33333333
		dcd	0x0f0f0f0f
		dcd	0x00ff00ff
		dcd	0x0000ffff
pat_end		equ	.
;
;-------------------------------------------------------------------------------
;
		end
