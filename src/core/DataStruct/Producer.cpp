#include "DataStruct.h"
#include "SharedMemoryManager.h"
#include <iostream>
#include <string>
#include <cwchar>

#ifdef PLATFORM_WINDOWS
#include "../Utils/WinUtils.h"
#endif

// Remove the global variables and function definitions
// that are now in SharedMemoryManager

// If any Producer-specific functionality is needed, it can be added here
// For now, this file can remain mostly empty since the functionality has been moved