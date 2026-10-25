; Live palette. The names page modules have always used now read the current
; light or dark values, which ThemeApply (personalize.inc) fills in.
; A_* are GDI+ ARGB, T_* are GDI COLORREF (0x00BBGGRR).
A_CANVAS      equ [themeCanvasARGB]
A_SIDEBAR     equ [themeSidebarARGB]
A_SIDELINE    equ [themeSidelineARGB]
A_DOT         equ [themeDotARGB]
A_CROSS       equ [themeCrossARGB]
A_CARD        equ [palCard]
A_HAIRLINE    equ [palHairline]
A_DIVIDER     equ [palDivider]
A_FIELD       equ [palField]
A_FIELDLINE   equ [palFieldLine]
A_TRACK       equ [palTrack]
A_SECOND      equ [palSecond]
A_SECOND_HI   equ [palSecondHi]
A_SECOND_LO   equ [palSecondLo]
A_INK         equ [palInk]
A_MUTED       equ [palMuted]
A_HOVER       equ [palHover]        ; RGB for faint hover washes (alpha set by caller)
A_TEXINK      equ [palTexInk]       ; what background textures are mixed toward
T_INK         equ [palTInk]
T_SECOND      equ [palTSecond]
T_MUTED       equ [palTMuted]
T_ACCENT      equ [themeAccentText]
T_CANVAS      equ [themeCanvasCR]
T_TIPTEXT     equ [palTipText]      ; text on an ink-coloured tooltip pill
; Legacy COLORREF names used by page modules.
C_INK         equ [palTInk]
C_MUTED       equ [palTSecond]
C_BLUE        equ [themeAccentText]
C_LINE        equ [palLineCR]
