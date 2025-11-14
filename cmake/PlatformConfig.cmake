# 平台和编译器配置文件

# 平台检测
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    add_definitions(-DPLATFORM_WINDOWS=1)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    # 不在这里定义PLATFORM_MACOS，避免重复定义
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    add_definitions(-DPLATFORM_LINUX=1)
endif()

# 架构检测
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_X64 TRUE)
    add_definitions(-DARCH_X64)
else()
    set(ARCH_X86 TRUE)
    add_definitions(-DARCH_X86)
endif()

# 编译器特定设置
if(MSVC)
    # Windows MSVC 配置
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /wd4005 /wd4244")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    
    # C++/CLI 支持（仅Windows）
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clr")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /clr")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /clr")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # macOS/Linux Clang 配置
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Linux GCC 配置
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# 平台特定库查找
if(WIN32)
    # Windows 特定库
    find_library(WBEMUUID_LIB wbemuuid)
    find_library(OLE32_LIB ole32)
    find_library(OLEAUT32_LIB oleaut32)
    find_library(ADVAPI32_LIB advapi32)
    find_library(SHELL32_LIB shell32)
    find_library(USER32_LIB user32)
    find_library(GDI32_LIB gdi32)
    find_library(KERNEL32_LIB kernel32)
    find_library(WS2_32_LIB ws2_32)
    find_library(DXGI_LIB dxgi)
    find_library(TBS_LIB tbs)
    find_library(CRYPT32_LIB crypt32)
    
elseif(APPLE)
    # macOS 特定库
    find_library(CORE_FOUNDATION_LIB CoreFoundation REQUIRED)
    find_library(IOKIT_LIB IOKit REQUIRED)
    find_library(SYSTEM_CONFIGURATION_LIB SystemConfiguration REQUIRED)
    find_library(FOUNDATION_LIB Foundation REQUIRED)
    
elseif(UNIX)
    # Linux 特定库
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(UDEV QUIET libudev)
        pkg_check_modules(PCI QUIET libpci)
    endif()
endif()