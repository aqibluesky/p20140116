.include    ..\..\..\..\..\BSP\include\GPCE063.inc
.DEFINE     _ASM_PORT

.IFDEF      _ASM_PORT
.PUBLIC     _vPortStartFirstTask
.ENDIF

.IFDEF      _ASM_PORT
.PUBLIC     _vPortYield
.ENDIF

.PUBLIC     _xPortReadFlagRegister

.IFDEF      _ASM_PORT
.PUBLIC     _IRQ7
.ENDIF

.EXTERNAL   _pxCurrentTCB
.EXTERNAL   _vTaskSwitchContext
.EXTERNAL   _vTaskIncrementTick


// Macro definition
PUSHALL: .MACRO

    PUSH R1,R5 TO [SP]

    .ENDM

POPALL: .MACRO

    POP R1,R5 FROM [SP]

    .ENDM

PUSHFR: .MACRO

    SECBANK ON
    R1 = FR
    R2 = 0x1FFF
    R1 = R1 & R2
    PUSH R1 TO [SP]
    SECBANK OFF

    .ENDM

POPFR: .MACRO

    SECBANK ON
    POP R1 FROM [SP]
    R2 = 0x1FFF
    R1 = R1 & R2
    FR = R1
    SECBANK OFF

    .ENDM

portSAVE_CONTEXT: .MACRO

    PUSHFR

    PUSHALL

    R1 = [_pxCurrentTCB]
    [R1] = SP

.ENDM

portRESTORE_CONTEXT: .MACRO

    R1 = [_pxCurrentTCB]
    SP = [R1]

    POPALL

    RETI

.ENDM

// Code section
.CODE

.IFDEF      _ASM_PORT
_vPortStartFirstTask: .PROC
    
    .IF 0
    R1 = [_pxCurrentTCB]
    SP = [R1]
    POPALL
    RETI
    .ELSE
    portRESTORE_CONTEXT
    .ENDIF

.ENDP
.ENDIF

.IFDEF      _ASM_PORT
_vPortYield: .PROC

    PUSHFR

    PUSHALL

    R1 = [_pxCurrentTCB]
    [R1] = SP

    CALL _vTaskSwitchContext

    R1 = [_pxCurrentTCB]
    SP = [R1]

    POPALL

    RETI

.ENDP
.ENDIF

_xPortReadFlagRegister: .PROC

    R1 = FR
    R2 = 0x1FFF
    R1 = R1 & R2

    RETF

.ENDP

// Text section
.TEXT

.IFDEF      _ASM_PORT
_IRQ7:

    PUSHALL

    R1 = C_IRQ7_64Hz
    [P_INT_Status] = R1

    R1 = C_Watchdog_Clear
    [P_Watchdog_Clear] = R1

    CALL _vTaskIncrementTick
    CALL _vPortYield

    POPALL

    RETI

.ENDIF