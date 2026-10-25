section '.idata' import data readable writeable
library kernel32,'KERNEL32.DLL',gdiplus,'GDIPLUS.DLL',msimg32,'MSIMG32.DLL',user32,'USER32.DLL',gdi32,'GDI32.DLL',shell32,'SHELL32.DLL',advapi32,'ADVAPI32.DLL',winhttp,'WINHTTP.DLL',comctl32,'COMCTL32.DLL',dwmapi,'DWMAPI.DLL',crypt32,'CRYPT32.DLL',comdlg32,'COMDLG32.DLL',ws2_32,'WS2_32.DLL',dnsapi,'DNSAPI.DLL',ole32,'OLE32.DLL',uxtheme,'UXTHEME.DLL'
import ole32,CoInitializeEx,'CoInitializeEx',CoUninitialize,'CoUninitialize',CoCreateInstance,'CoCreateInstance',CoTaskMemFree,'CoTaskMemFree'
import ws2_32,WSAStartup,'WSAStartup',WSACleanup,'WSACleanup',socket,'socket',closesocket,'closesocket',ioctlsocket,'ioctlsocket',connect,'connect',send,'send',recv,'recv',select,'select',getsockopt,'getsockopt',WSAGetLastError,'WSAGetLastError',GetAddrInfoExW,'GetAddrInfoExW',FreeAddrInfoExW,'FreeAddrInfoExW'
import dnsapi,DnsQuery_W,'DnsQuery_W',DnsRecordListFree,'DnsRecordListFree'
import comdlg32,GetOpenFileNameW,'GetOpenFileNameW',GetSaveFileNameW,'GetSaveFileNameW'
import kernel32,\
 GetModuleHandleW,'GetModuleHandleW',GetProcAddress,'GetProcAddress',ExitProcess,'ExitProcess',GetModuleFileNameW,'GetModuleFileNameW',GetStdHandle,'GetStdHandle',\
 GetFullPathNameW,'GetFullPathNameW',LocalFree,'LocalFree',lstrcmpiW,'lstrcmpiW',GlobalMemoryStatusEx,'GlobalMemoryStatusEx',\
 GetTempPathW,'GetTempPathW',GetCurrentProcessId,'GetCurrentProcessId',GlobalAlloc,'GlobalAlloc',GlobalLock,'GlobalLock',GlobalUnlock,'GlobalUnlock',GlobalFree,'GlobalFree',\
 GetEnvironmentVariableW,'GetEnvironmentVariableW',SearchPathW,'SearchPathW',GetFileAttributesW,'GetFileAttributesW',\
 FindFirstFileW,'FindFirstFileW',FindNextFileW,'FindNextFileW',FindClose,'FindClose',\
 GetProcessHeap,'GetProcessHeap',HeapAlloc,'HeapAlloc',HeapReAlloc,'HeapReAlloc',HeapFree,'HeapFree',\
 CreateFileW,'CreateFileW',ReadFile,'ReadFile',WriteFile,'WriteFile',GetFileSize,'GetFileSize',GetFileSizeEx,'GetFileSizeEx',\
 GetFileInformationByHandle,'GetFileInformationByHandle',\
 CloseHandle,'CloseHandle',CreateProcessW,'CreateProcessW',WaitForSingleObject,'WaitForSingleObject',\
 CreatePipe,'CreatePipe',SetHandleInformation,'SetHandleInformation',PeekNamedPipe,'PeekNamedPipe',\
 CreateFileMappingW,'CreateFileMappingW',MapViewOfFile,'MapViewOfFile',UnmapViewOfFile,'UnmapViewOfFile',\
 GetSystemTimeAsFileTime,'GetSystemTimeAsFileTime',GetLocalTime,'GetLocalTime',CompareFileTime,'CompareFileTime',GetFileTime,'GetFileTime',SetFilePointer,'SetFilePointer',\
 GetExitCodeProcess,'GetExitCodeProcess',GetSystemDirectoryW,'GetSystemDirectoryW',GetWindowsDirectoryW,'GetWindowsDirectoryW',\
 MultiByteToWideChar,'MultiByteToWideChar',WideCharToMultiByte,'WideCharToMultiByte',LCMapStringW,'LCMapStringW',\
 lstrcpyW,'lstrcpyW',lstrcatW,'lstrcatW',lstrlenW,'lstrlenW',lstrlenA,'lstrlenA',lstrcpyA,'lstrcpyA',\
 lstrcatA,'lstrcatA',lstrcmpA,'lstrcmpA',lstrcmpiA,'lstrcmpiA',GetLastError,'GetLastError',\
 CreateDirectoryW,'CreateDirectoryW',DeleteFileW,'DeleteFileW',MoveFileExW,'MoveFileExW',CreateHardLinkW,'CreateHardLinkW',CopyFileW,'CopyFileW',TerminateProcess,'TerminateProcess',\
 CreateThread,'CreateThread',GetTickCount,'GetTickCount',Sleep,'Sleep',MulDiv,'MulDiv',\
 GetPrivateProfileStringW,'GetPrivateProfileStringW',WritePrivateProfileStringW,'WritePrivateProfileStringW',GetPrivateProfileIntW,'GetPrivateProfileIntW',\
 QueryPerformanceCounter,'QueryPerformanceCounter',QueryPerformanceFrequency,'QueryPerformanceFrequency',GetUserDefaultUILanguage,'GetUserDefaultUILanguage'
import user32,\
 RegisterClassExW,'RegisterClassExW',CreateWindowExW,'CreateWindowExW',DefWindowProcW,'DefWindowProcW',\
 ShowWindow,'ShowWindow',UpdateWindow,'UpdateWindow',GetMessageW,'GetMessageW',PeekMessageW,'PeekMessageW',SystemParametersInfoW,'SystemParametersInfoW',TranslateMessage,'TranslateMessage',DispatchMessageW,'DispatchMessageW',\
 PostQuitMessage,'PostQuitMessage',LoadCursorW,'LoadCursorW',LoadIconW,'LoadIconW',\
 BeginPaint,'BeginPaint',EndPaint,'EndPaint',GetClientRect,'GetClientRect',FillRect,'FillRect',\
 DrawTextW,'DrawTextW',InvalidateRect,'InvalidateRect',SendMessageW,'SendMessageW',SendMessageA,'SendMessageA',\
 PostMessageW,'PostMessageW',SetWindowTextW,'SetWindowTextW',SetWindowTextA,'SetWindowTextA',\
 GetWindowTextW,'GetWindowTextW',GetWindowTextA,'GetWindowTextA',EnableWindow,'EnableWindow',\
 GetDpiForSystem,'GetDpiForSystem',SetProcessDPIAware,'SetProcessDPIAware',AdjustWindowRectEx,'AdjustWindowRectEx',\
 SetWindowPos,'SetWindowPos',MapWindowPoints,'MapWindowPoints',GetWindowRect,'GetWindowRect',SetFocus,'SetFocus',SetTimer,'SetTimer',KillTimer,'KillTimer',\
 wsprintfA,'wsprintfA',wsprintfW,'wsprintfW',SetCursor,'SetCursor',GetSysColorBrush,'GetSysColorBrush',\
 MessageBoxW,'MessageBoxW',DestroyWindow,'DestroyWindow',SetWindowLongW,'SetWindowLongW',\
 GetKeyState,'GetKeyState',GetFocus,'GetFocus',\
 OpenClipboard,'OpenClipboard',EmptyClipboard,'EmptyClipboard',SetClipboardData,'SetClipboardData',CloseClipboard,'CloseClipboard',\
 TrackMouseEvent,'TrackMouseEvent',GetSystemMetrics,'GetSystemMetrics',IsDialogMessageW,'IsDialogMessageW',\
 EnumChildWindows,'EnumChildWindows',GetClassNameW,'GetClassNameW',GetParent,'GetParent',IsIconic,'IsIconic',GetForegroundWindow,'GetForegroundWindow',\
 CreatePopupMenu,'CreatePopupMenu',AppendMenuW,'AppendMenuW',TrackPopupMenu,'TrackPopupMenu',DestroyMenu,'DestroyMenu',GetCursorPos,'GetCursorPos',ClientToScreen,'ClientToScreen'
import gdi32,\
 CreateSolidBrush,'CreateSolidBrush',SetBkColor,'SetBkColor',CreatePen,'CreatePen',SelectObject,'SelectObject',DeleteObject,'DeleteObject',\
 CreateCompatibleDC,'CreateCompatibleDC',CreateCompatibleBitmap,'CreateCompatibleBitmap',DeleteDC,'DeleteDC',\
 BitBlt,'BitBlt',SetBkMode,'SetBkMode',SetTextColor,'SetTextColor',RoundRect,'RoundRect',\
 MoveToEx,'MoveToEx',LineTo,'LineTo',Rectangle,'Rectangle',Ellipse,'Ellipse',Polygon,'Polygon',\
 CreateFontW,'CreateFontW',GetStockObject,'GetStockObject',SetMapMode,'SetMapMode',SetWindowExtEx,'SetWindowExtEx',SetViewportExtEx,'SetViewportExtEx',SaveDC,'SaveDC',CreateDIBSection,'CreateDIBSection',CreateRoundRectRgn,'CreateRoundRectRgn',SelectClipRgn,'SelectClipRgn',StretchBlt,'StretchBlt',SetStretchBltMode,'SetStretchBltMode',IntersectClipRect,'IntersectClipRect',RestoreDC,'RestoreDC',SetTextCharacterExtra,'SetTextCharacterExtra'
import gdiplus,\
 GdiplusStartup,'GdiplusStartup',GdiplusShutdown,'GdiplusShutdown',GdipCreateFromHDC,'GdipCreateFromHDC',GdipDeleteGraphics,'GdipDeleteGraphics',\
 GdipSetSmoothingMode,'GdipSetSmoothingMode',GdipSetPixelOffsetMode,'GdipSetPixelOffsetMode',GdipScaleWorldTransform,'GdipScaleWorldTransform',GdipTranslateWorldTransform,'GdipTranslateWorldTransform',\
 GdipSaveGraphics,'GdipSaveGraphics',GdipRestoreGraphics,'GdipRestoreGraphics',GdipCreateSolidFill,'GdipCreateSolidFill',GdipDeleteBrush,'GdipDeleteBrush',\
 GdipCreateLineBrushI,'GdipCreateLineBrushI',GdipCreatePath,'GdipCreatePath',GdipDeletePath,'GdipDeletePath',GdipAddPathArc,'GdipAddPathArc',\
 GdipAddPathRectangle,'GdipAddPathRectangle',GdipAddPathPie,'GdipAddPathPie',GdipAddPathPolygon,'GdipAddPathPolygon',GdipClosePathFigure,'GdipClosePathFigure',GdipStartPathFigure,'GdipStartPathFigure',GdipCreateBitmapFromScan0,'GdipCreateBitmapFromScan0',GdipGetImageGraphicsContext,'GdipGetImageGraphicsContext',GdipDisposeImage,'GdipDisposeImage',GdipAddPathLine,'GdipAddPathLine',\
 GdipFillPath,'GdipFillPath',GdipDrawPath,'GdipDrawPath',GdipCreatePen1,'GdipCreatePen1',GdipDeletePen,'GdipDeletePen',\
 GdipSetPenLineCap197819,'GdipSetPenLineCap197819',GdipSetPenLineJoin,'GdipSetPenLineJoin',GdipSetPenDashArray,'GdipSetPenDashArray',GdipDrawLine,'GdipDrawLine',\
 GdipFillEllipse,'GdipFillEllipse',GdipDrawArc,'GdipDrawArc',GdipSetClipPath,'GdipSetClipPath',GdipResetClip,'GdipResetClip',\
 GdipGetDC,'GdipGetDC',GdipReleaseDC,'GdipReleaseDC',\
 GdipLoadImageFromFile,'GdipLoadImageFromFile',GdipCreateBitmapFromFile,'GdipCreateBitmapFromFile',GdipGetImageWidth,'GdipGetImageWidth',GdipGetImageHeight,'GdipGetImageHeight',\
 GdipCloneBitmapAreaI,'GdipCloneBitmapAreaI',GdipDrawImagePointsRectI,'GdipDrawImagePointsRectI',GdipSetInterpolationMode,'GdipSetInterpolationMode',GdipImageRotateFlip,'GdipImageRotateFlip'
import msimg32,AlphaBlend,'AlphaBlend'
import shell32,ShellExecuteW,'ShellExecuteW',SHCreateItemFromParsingName,'SHCreateItemFromParsingName'
import advapi32,\
 CryptAcquireContextW,'CryptAcquireContextW',CryptCreateHash,'CryptCreateHash',CryptHashData,'CryptHashData',\
 CryptGetHashParam,'CryptGetHashParam',CryptDestroyHash,'CryptDestroyHash',CryptReleaseContext,'CryptReleaseContext',\
 RegOpenKeyExW,'RegOpenKeyExW',RegQueryValueExW,'RegQueryValueExW',RegCloseKey,'RegCloseKey'
import winhttp,\
 WinHttpOpen,'WinHttpOpen',WinHttpConnect,'WinHttpConnect',WinHttpOpenRequest,'WinHttpOpenRequest',\
 WinHttpSendRequest,'WinHttpSendRequest',WinHttpReceiveResponse,'WinHttpReceiveResponse',\
 WinHttpQueryHeaders,'WinHttpQueryHeaders',WinHttpQueryDataAvailable,'WinHttpQueryDataAvailable',WinHttpReadData,'WinHttpReadData',\
 WinHttpCloseHandle,'WinHttpCloseHandle',WinHttpCrackUrl,'WinHttpCrackUrl',WinHttpSetTimeouts,'WinHttpSetTimeouts',WinHttpSetOption,'WinHttpSetOption'
import comctl32,InitCommonControlsEx,'InitCommonControlsEx'
import uxtheme,SetWindowTheme,'SetWindowTheme'
import dwmapi,DwmSetWindowAttribute,'DwmSetWindowAttribute',DwmFlush,'DwmFlush'
import crypt32,CryptProtectData,'CryptProtectData',CryptUnprotectData,'CryptUnprotectData'
