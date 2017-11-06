;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; start.s file 
; 
; used in the device drivers' coding
; created by mukaram khan and xin jin
; 
; SpiNNaker Project, The University of Manchester
; Copyright (C) SpiNNaker Project, 2008. All rights reserved.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       
        GET ../inc/peripherals.s        
 		
 		EXPORT	loadApplicationAndStart
   		EXPORT  loadApplicationData
 			
 		AREA 	ApplicationLoader, CODE, READONLY
        ENTRY
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;routine to be used by fascicle processors to load the application into ITCM and 
;run it
;parameters: r0=source, r1=destination, r2=size, r3=address of main routine in the application
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
loadApplicationAndStart
	    SUBS     R2, R2, #0x10
        LDMCS    R0!, {R4-R7}
        STMCS    R1!, {R4-R7}
        BHI      loadApplicationAndStart 
        ;LSLS     r2,r2,#29
        ;LDMCS    r0!,{r4,r5}
        ;STMCS    r1!,{r4,r5}
        ;LDRMI    r4,[r0,#0]
        ;STRMI    r4,[r1,#0]
        ;BX       lr
        MOV		 PC, R3
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;routine to be used by fascicle processors to load the data in local memory eg. DTCM
;
;parameters: r0=source, r1=destination, r2=size, 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;        
loadApplicationData
	    SUBS     R2, R2, #0x10
        LDMCS    R0!, {R3-R6}
        STMCS    R1!, {R3-R6}
        BHI      loadApplicationData 
        ;LSLS     r2,r2,#29
        ;LDMCS    r0!,{r4,r5}
        ;STMCS    r1!,{r4,r5}
        ;LDRMI    r4,[r0,#0]
        ;STRMI    r4,[r1,#0]
        MOV		 PC, lr       
        
        END