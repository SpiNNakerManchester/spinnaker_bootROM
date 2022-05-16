;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	phy.s
; 
;	TODO Describe file purpose
;
;	Created by Mukaram Khan and Xin Jin
;	Modified by Thomas Sharp
;
;	SpiNNaker Project, The University of Manchester
;	Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		GET	../inc/globals.s
		
		EXPORT	init_phy
		EXPORT  test_phy
 		EXPORT	phy_write
 		EXPORT	phy_read
 		
 		AREA 	phy, READONLY, CODE

; Ethernet MII registers
ETHER			EQU		ETH_BASE
ETH_REGS		EQU		ETH_GEN_COMMAND
ETH_PHY_CTRL	EQU		ETH_PHY_CONTROL - ETH_REGS	; 0x1c

; Phy chip registers
PHY_CTRL_REG	EQU		0
PHY_MASK_REG	EQU		30
PHY_NEG_CAP_REG EQU		4


init_phy	; //TODO bring into init_peripherals
		STMFD	sp!, {lr}
		
		MOV		r0, #PHY_MASK_REG		; Interrupt mask reg addr
		MOV		r1, #0x50    			; Bits 4 (auto-neg), 6 (link down)
		BL		phy_write

;		MOV		r0, #PHY_CTRL_REG		; Control reg addr
;		MOV		r1, #0x1000				; Bit 12 (auto-neg)
;		BL		phy_write

		MOV		r0, #PHY_NEG_CAP_REG	; Manually set caps to 10MegFD - cp 25/Feb/2010
		MOV		r1, #0x61				; 100meg not working, so auto-neg above commented out
		BL		phy_write

		LDMFD	sp!, {pc}



test_phy		STMFD	sp!, {r4-r5, lr}
;
;	MOV	r4, #TUBE
;	MOV	r5, #"T"
;	STR	r5, [r4]
;
		MOV	r0, #PHY_CTRL_REG	; Read control reg
		BL	phy_read
		MOV	r4, r0			; Save in r4
;
		MOV	r1, #0x4000		; Write bits 14, 7
		ORR	r1, r1, #0x0080
		MOV	r5, r1			; and copy to r5
;
		MOV	r0, #PHY_CTRL_REG	; Write control reg
		BL	phy_write
;
		MOV	r0, #PHY_CTRL_REG	; and read back
		BL	phy_read
;
		AND	r0, r0, r5		; Check bits read back as ones
		CMP	r0, r5
		BNE	test_fail
;
		MOV	r1, #0			; Write zeros to control reg
		MOV	r0, #PHY_CTRL_REG
		BL	phy_write
;
		MOV	r0, #PHY_CTRL_REG	; Read back
		BL	phy_read
;
		TST	r0, r5			; Ensure both zero
		BNE	test_fail
;
		MOV	r1, r4			; Restore control reg
		MOV	r0, #PHY_CTRL_REG
		BL	phy_write
;

		MOV	r0, #1
		LDMFD	sp!, {r4-r5, pc}
;
;
test_fail	MOV	r1, r4			; Restore control reg
		MOV	r0, #PHY_CTRL_REG
		BL	phy_write
	
		MOV	r0, #0
		LDMFD	sp!, {r4-r5, pc}
;
;
;-------------------------------------------------------------------------------
;
; Bits in ETH_PHY_CTRL reg
;   0 PHY_RSTn
;   1 MDIO (read only)
;   2 MDIO (write)
;   3 MDIO_OE
;   4 MDC
;   5 IRQ polarity control
;
;
; phy_write - r0 = phy/reg addr, r1 = data
;
;	void phy_write (int addr, int data);
;


phy_write
		STMFD	sp!, {r4-r5, lr}
		MOV		r4, r0					; Save address
		MOV		r5, r1					; Save data

		MOV		r2, #ETHER				; r2 -> ethernet
		ADD		r2, r2, #ETH_REGS		; r2 -> ctrl regs

		MOV		r3, #0x0d				; Enable MDIO pin, set MDIO=1, MDC=0, CP-set0 bit to enable phy
		STR		r3, [r2, #ETH_PHY_CTRL]
;
		MOV		r0, #0xffffffff			; Shift out 32 ones
		MOV		r1, #32
		BL		phy_shift_out
;
		MOV		r0, r4, LSL #18			; Shift arg to top-4 of r0
		ORR		r0, r0, #0x50000000		; Add in 0101 at top
		MOV		r1, #14
		BL		phy_shift_out
;
		MOV		r1, #2					; Send 2 clocks
		MOV		r0, #0x80000000			; Send 1 then 0
		BL		phy_shift_out
;
		MOV		r0, r5, LSL #16			; Move data (shifted up) to r0	
		MOV		r1, #16					; Shift out 16 bits
		BL		phy_shift_out
;
		BIC		r3, r3, #0x08			; Turn off MDIO pin,
		STR		r3, [r2, #ETH_PHY_CTRL]
;
		LDMFD	sp!, {r4-r5, pc}       
;
;
;-------------------------------------------------------------------------------
;
; phy_read - r0 = phy/reg addr
;
;	int phy_read (int addr);
;
;
phy_read	STMFD	sp!, {r4, lr}
;
;	MOV	r4, #TUBE
;	MOV	r2, #"R"
;	STR	r2, [r4]
;
		MOV	r4, r0			; Save PHY addr, reg addr from r0
;
		MOV	r2, #ETHER		; r2 -> ethernet
		ADD	r2, r2, #ETH_REGS	; r2 -> ctrl regs
;
		MOV	r3, #0x0d		; Enable MDIO pin, set MDIO=1, MDC=0, , CP-set0 bit to enable phy
		STR	r3, [r2, #ETH_PHY_CTRL]
;
		MOV	r0, #0xffffffff		; Shift out 32 ones
		MOV	r1, #32
		BL	phy_shift_out		; 
;
		MOV	r0, r4, LSL #18		; Shift arg to top-4 of r0
		ORR	r0, r0, #0x60000000	; Add in 0110 at top
		MOV	r1, #14
		BL	phy_shift_out
;
		BIC	r3, r3, #0x08		; Turn off MDIO pin
		STR	r3, [r2, #ETH_PHY_CTRL]
;
		MOV	r1, #2			; Send 2 clocks
		BL	phy_shift_out
;
		MOV	r1, #16			; Shift in data
		MOV	r0, #0
		BL	phy_shift_in		; !! optimise?
;
		LDMFD	sp!, {r4, pc}       
;
;-------------------------------------------------------------------------------
;
phy_shift_in	LDR	r3, [r2, #ETH_PHY_CTRL]	; Read PHY_CTRL
		TST	r3, #0x02		; Get state of MDIO input
		ORRNE	r0, r0, #1		; and copy to bit 0 of r0
;
		ORR	r3, r3, #0x10		; Set clock bit
		STR	r3, [r2, #ETH_PHY_CTRL]
		
		STMFD	sp!, {r8}
                MOV     r8,#0x8000                 
delayin         SUBS    r8,r8,#1		;TODO Decide on usage of delay (T.S.)
                CMP     r8,#0
                BNE     delayin		
		LDMFD	sp!, {r8} 
		
		BIC	r3, r3, #0x10		; Clear clock bit
		STR	r3, [r2, #ETH_PHY_CTRL]
;
		SUBS	r1, r1, #1		; Loop count
		LSLNE	r0, r0, #1		; Shift r0 left unless last
		BNE	phy_shift_in
;
		BX		LR
;
;
;
phy_shift_out	TST	r0, #0x80000000		; Test data bit
		BICEQ	r3, r3, #0x04
		ORRNE	r3, r3, #0x04
		STR	r3, [r2, #ETH_PHY_CTRL]	; Set MDIO pin
;
		LSL	r0, r0, #1		; Shift data up
;
		ORR	r3, r3, #0x10		; Set clock bit
		STR	r3, [r2, #ETH_PHY_CTRL]
		
		STMFD	sp!, {r8}
                MOV     r8,#0x8000                 
delayout        SUBS    r8,r8,#1
                CMP     r8,#0
                BNE     delayout		
		LDMFD	sp!, {r8} 
		
		
		BIC	r3, r3, #0x10		; Clear clock bit
		STR	r3, [r2, #ETH_PHY_CTRL]
;
		SUBS	r1, r1, #1		; Loop count
		BNE	phy_shift_out
;
		BX		LR
;
		END
