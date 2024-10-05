;
; test file
;
                
                .ORG $200
                
                cls
                ret
                jp $0210
                jp next
next:           call next2
next2:          se v0, $ff
                sne v1, $ff
                se v2, v3
                ld v4, $ff
                add v5, $ff
                ld v6, v7
                or v8, v9
                and va, vb
                xor vc, vd
                add ve, vf
                sub v0, v1
                shr v2
                subn v3, v4
                shl v5
                sne v6, v7
                ld i, next
                jp v0, next2
                rnd v8, $ff
                drw v9, va, $f
                skp vb
                sknp vc
                ld vd, dt
                ld ve, k
                ld dt, vf
                ld st, v0
                add i, v1
                ld f, v2
                ld b, v3
                ld [i], v4
                ld v5, [i]
