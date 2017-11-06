		GET ../inc/peripherals.s      

				GBLL   	DebugOutPut         
DebugOutPut     SETL    {FALSE}
				GBLL   	DebugOber         
DebugOber       SETL    {FALSE}   
				GBLL   	RandomI         
RandomI         SETL    {FALSE}   


DTCMLimits      EQU     0x410000        

;Neurons relatives
;configurable
NeuronNum       EQU     10     ; I of excitatory = 3*I of inhibitator
BlkSize         EQU     40      ;bytes
                             
TimeLimits      EQU     300  
NeuOutNum       EQU     9       ; which neuron's data to output (0 - NeuronNum-1)
InhibitI        EQU     2       ; Max I of inhibitatory: 2^n

DMA_REQ_BUF_SIZE    EQU     8*16    ; bytes, buffer up to 16 DMA request     
COMMS_BUF_SIZE      EQU     4000    ; bytes, buffer for comm                

;Constant
;NeuronSize      EQU     0x2c    ; 44
;NDelayOffset    EQU     0x0c    ; 12




;************************************************
	END