# TCMT 主可执行文件配置 (类似 .vcxproj)

if(WIN32)
    # Windows 主程序源文件
    set(PROJECT1_SOURCES "src/main.cpp")
    
    file(GLOB_RECURSE PLATFORM_SOURCES
        "src/core/*.cpp"
        "src/core/*.h"
    )
    list(FILTER PLATFORM_SOURCES EXCLUDE REGEX ".*Test.*")
    list(FILTER PLATFORM_SOURCES EXCLUDE REGEX ".*Demo.*")
    list(APPEND PROJECT1_SOURCES ${PLATFORM_SOURCES})
    
    # Windows 主可执行文件
    add_executable(TCMT-Main ${PROJECT1_SOURCES})
    
    set_target_properties(TCMT-Main PROPERTIES
        WIN32_EXECUTABLE FALSE
        VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
        VS_DOTNET_REFERENCES "System;mscorlib;LibreHardwareMonitorLib"
        COMMON_LANGUAGE_RUNTIME "/clr"
        VS_GLOBAL_COMPILE_OPTIONS "/ZW /EHsc"
    )
    
    # CUDA 支持 (仅 Windows)
    if(ENABLE_CUDA)
        find_package(CUDAToolkit 12.6 QUIET)
        if(CUDAToolkit_FOUND)
            target_link_libraries(TCMT-Main PRIVATE CUDA::cudart)
            target_compile_definitions(TCMT-Main PRIVATE CUDA_SUPPORTED=1)
        endif()
    endif()
    
    # 包含目录
    target_include_directories(TCMT-Main
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/core
            ${CMAKE_SOURCE_DIR}/src/core/common
            ${CMAKE_SOURCE_DIR}/src/core/Utils
            ${CMAKE_SOURCE_DIR}/src/third_party
    )
    
    # 编译定义
    target_compile_definitions(TCMT-Main
        PRIVATE
            UNICODE
            _UNICODE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )
    
    # 链接库
    target_link_libraries(TCMT-Main
        PRIVATE
            CoreLib
            CPP-parsers
            wbemuuid
            ole32
            oleaut32
            advapi32
            shell32
            user32
            gdi32
            kernel32
            ws2_32
            dxgi
            tbs
            crypt32
            setupapi
            cfgmgr32
            pdh
    )
    
    # 预编译头文件 (MSVC)
    if(MSVC AND EXISTS ${CMAKE_SOURCE_DIR}/src/core/Utils/stdafx.h)
        target_precompile_headers(TCMT-Main PRIVATE src/core/Utils/stdafx.h)
    endif()
    
else()
    # 跨平台主程序源文件 (macOS/Linux)
    set(CROSSPLATFORM_SOURCES
        "src/platform/CrossPlatformMain.cpp"
        "src/platform/CrossPlatformHardwareInfo.cpp"
    )
    
    # 跨平台主可执行文件
    add_executable(TCMT-Main ${CROSSPLATFORM_SOURCES})
    
    # 包含目录
    target_include_directories(TCMT-Main
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/core
            ${CMAKE_SOURCE_DIR}/src/core/common
            ${CMAKE_SOURCE_DIR}/src/core/Utils
            ${CMAKE_SOURCE_DIR}/src/platform
            ${CMAKE_SOURCE_DIR}/src/third_party
            ${CMAKE_SOURCE_DIR}/src/CPP-parsers/include
            ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern
            ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/json/single_include
    )
    
    # 编译定义
    target_compile_definitions(TCMT-Main
        PRIVATE
            UNICODE
            _UNICODE
    )
    
    # 链接库
    target_link_libraries(TCMT-Main
        PRIVATE
            CoreLib
            CPP-parsers
    )
    
    # 设置属性
    set_target_properties(TCMT-Main PROPERTIES
        CXX_STANDARD 20
        C_STANDARD 17
        POSITION_INDEPENDENT_CODE ON
        FOLDER "Executables"
    )
    
    # macOS 特定库
    if(APPLE)
        find_library(CORE_FOUNDATION_LIB CoreFoundation REQUIRED)
        find_library(IOKIT_LIB IOKit REQUIRED)
        find_library(SYSTEM_CONFIGURATION_LIB SystemConfiguration REQUIRED)
        find_library(FOUNDATION_LIB Foundation REQUIRED)
        
        target_link_libraries(TCMT-Main
            PRIVATE
                ${CORE_FOUNDATION_LIB}
                ${IOKIT_LIB}
                ${SYSTEM_CONFIGURATION_LIB}
                ${FOUNDATION_LIB}
        )
        
        # macOS 特定编译选项
        target_compile_options(TCMT-Main PRIVATE
            -Wall -Wextra -Wpedantic
            -Wno-unused-parameter
        )
    endif()
    
    # Linux 特定库
    if(UNIX AND NOT APPLE)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(UDEV QUIET libudev)
            pkg_check_modules(PCI QUIET libpci)
        endif()
        
        target_link_libraries(TCMT-Main
            PRIVATE
                ${UDEV_LIBRARIES}
                ${PCI_LIBRARIES}
                pthread
                dl
                rt
        )
        
        # Linux 特定编译选项
        target_compile_options(TCMT-Main PRIVATE
            -Wall -Wextra -Wpedantic
            -Wno-unused-parameter
        )
    endif()
endif()

# 输出目录设置
set_target_properties(TCMT-Main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
)

# 安装规则
install(TARGETS TCMT-Main
    RUNTIME DESTINATION bin
)