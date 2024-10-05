;
; scanner example
;
                
                .ORG $200
                
start:          ld v0, 28   ; x pixel
                ld v1, 16   ; y pixel
                ld v2, 0    ; initialize counter to 0
                ld i, data
                
loop:           cls
                drw v0, v1, 1
                
                add v2, 1   ; increment the counter
                sne v2, 16  ; if counter is not equal to 16, skip next instruction
                jp start
                
                ld v3, 5    ; set delay timer to one half second
                ld dt, v3
wait:           ld v3, dt   ; read delay timer
                sne v3, 0   ; if delay timer is not equal to 0, skip next instruction
                jp update
                jp wait
                
update:         ld i, data  ; set i to the data address
                add i, v2   ; add the counter to i
                jp loop
                
data:           .byte %10000000
                .byte %01000000
                .byte %00100000
                .byte %00010000
                .byte %00001000
                .byte %00000100
                .byte %00000010
                .byte %00000001
                .byte %00000001
                .byte %00000010
                .byte %00000100
                .byte %00001000
                .byte %00010000
                .byte %00100000
                .byte %01000000
                .byte %10000000
