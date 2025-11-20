#ifndef PCH_H
#define PCH_H

#include <windows.h>
#include <objbase.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <comdef.h>
#include <new>
#include <ntsecapi.h>

// Try to include system credentialprovider.h first, fallback to local copy
#ifdef _WIN32
    #if __has_include(<credentialprovider.h>)
        #include <credentialprovider.h>
    #else
        #include "credentialprovider.h"
    #endif
#else
    #include "credentialprovider.h"
#endif

#endif //PCH_H

