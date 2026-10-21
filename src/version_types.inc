; Shared fixed-capacity version catalogue, loader and installation queue records.
VC_ID = 0
VC_TYPE = 128                     ; 0 release, 1 snapshot
VC_DATE = 132                     ; UTF-8 ISO date, capacity 32
VC_SIZE = 164
VC_CAPACITY = 4096
VL_VERSION = 0                    ; UTF-8 loader version, capacity 128
VL_STABLE = 128
VL_SIZE = 132
VL_CAPACITY = 1024
VQ_BASE = 0                       ; UTF-8 vanilla version ID, capacity 128
VQ_LOADER = 128                   ; 0 vanilla, 1 Fabric, 2 Forge, 3 NeoForge, 4 Quilt
VQ_VERSION = 132                  ; UTF-8 loader version, capacity 128
VQ_TARGET = 260                   ; installed profile ID, capacity 128
VQ_STATE = 388                    ; 0 pending, 1 running, 2 complete, 3 failed, 4 cancelled
VQ_SIZE = 392
VQ_CAPACITY = 64
