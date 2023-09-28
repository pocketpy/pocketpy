#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //define something for Windows (32-bit and 64-bit, this part is common)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif

/* 
  @raysan5: To avoid conflicting windows.h symbols with raylib, some flags are defined
  WARNING: Those flags avoid inclusion of some Win32 headers that could be required 
  by user at some point and won't be included...
*/

/* If defined, the following flags inhibit definition of the indicated items.*/
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
/*#define NONLS             // All NLS defines and routines*/
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

/* Type required before windows.h inclusion  */
typedef struct tagMSG *LPMSG;

#include <windows.h>

/* Type required by some unused function...  */
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#include <objbase.h>
#include <mmreg.h>
#include <mmsystem.h>

/* @raysan5: Some required types defined for MSVC/TinyC compiler */
#if defined(_MSC_VER) || defined(__TINYC__)
    #include <propidl.h>
#endif

    #define PK_EXPORT __declspec(dllexport)
    #define PK_SUPPORT_DYLIB    1
    #define PK_SYS_PLATFORM     "win32"
#elif __EMSCRIPTEN__
    #include <emscripten.h>
    #define PK_EXPORT EMSCRIPTEN_KEEPALIVE
    #define PK_SUPPORT_DYLIB    0
    #define PK_SYS_PLATFORM     "emscripten"
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        // iOS, tvOS, or watchOS Simulator
        #define PK_SYS_PLATFORM     "ios"
        #define PK_SUPPORT_DYLIB    4
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
        #define PK_SYS_PLATFORM     "ios"
        #define PK_SUPPORT_DYLIB    4
    #elif TARGET_OS_MAC
        #define PK_SYS_PLATFORM     "darwin"
        #ifdef PK_USE_DYLIB
            #include <dlfcn.h>
            #define PK_SUPPORT_DYLIB    2
        #else
            #define PK_SUPPORT_DYLIB    0
        #endif
    #else
    #   error "Unknown Apple platform"
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
#elif __ANDROID__
    #ifdef PK_USE_DYLIB
        #include <dlfcn.h>
        #define PK_SUPPORT_DYLIB    3
    #else
        #define PK_SUPPORT_DYLIB    0
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     "android"
#elif __linux__
    #ifdef PK_USE_DYLIB
        #include <dlfcn.h>
        #define PK_SUPPORT_DYLIB    2
    #else
        #define PK_SUPPORT_DYLIB    0
    #endif
    #define PK_EXPORT __attribute__((visibility("default")))
    #define PK_SYS_PLATFORM     "linux"
#else
    #define PK_EXPORT
    #define PK_SUPPORT_DYLIB    0
    #define PK_SYS_PLATFORM     "unknown"
#endif