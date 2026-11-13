; Golden fixture groups were captured in independent oracle processes.
; Preserve the caller's full x87 state and start each group with the same
; empty stack/TOP=0. FSINCOS outside its range can change TOP even when all
; registers are empty. Do not reset between rows: their order is evidence.
.code
BeginFixtureX87 PROC
    fnsave [rcx]
    ret
BeginFixtureX87 ENDP
EndFixtureX87 PROC
    frstor [rcx]
    ret
EndFixtureX87 ENDP
SetFixtureX87ControlWord PROC
    sub rsp, 8
    mov word ptr [rsp], cx
    fldcw word ptr [rsp]
    add rsp, 8
    ret
SetFixtureX87ControlWord ENDP
END
