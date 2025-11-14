# TCMT 项目结构配置
# 类似 Visual Studio 解决方案 (.sln) 的多项目配置

# ============================================================================
# 0. 静态库定义
# ============================================================================

# 1. 核心静态库 (CoreLib) - 已在 src/core/CMakeLists.txt 中定义

# 2. CPP解析器静态库 (CPP-parsers)
set(CPPPARSERS_SOURCES
    src/CPP-parsers/src/IniConfigParser.cpp
    src/CPP-parsers/src/JsonConfigParser.cpp
    src/CPP-parsers/src/YamlConfigParser.cpp
    src/CPP-parsers/src/XmlConfigParser.cpp
    src/CPP-parsers/src/TomlConfigParser.cpp
    src/CPP-parsers/extern/inih/ini.c
    src/CPP-parsers/extern/inih/cpp/INIReader.cpp
)

add_library(CPP-parsers STATIC ${CPPPARSERS_SOURCES})

target_include_directories(CPP-parsers
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/include
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/json/single_include
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/yaml-cpp/include
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/tinyxml2
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/tomlplusplus/include
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/inih
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/inih/cpp
)

target_link_libraries(CPP-parsers
    PUBLIC
        nlohmann_json::nlohmann_json
        yaml-cpp
        tinyxml2
        inih
)

set_target_properties(CPP-parsers PROPERTIES
    CXX_STANDARD 20
    POSITION_INDEPENDENT_CODE ON
    FOLDER "Static Libraries"
)

# 1. 核心静态库 (CoreLib)
# 2. CPP解析器静态库 (CPP-parsers)
# 3. 跨平台主程序 (TCMT-Main)
# 4. CLI客户端 (TCMT-CLI)
# 5. 动态库 (TCMT-Core-Dynamic)

# ============================================================================
# 1. CLI 客户端 (跨平台命令行工具)
# ============================================================================

if(NOT WIN32)
    # CLI 源文件
    set(CLI_SOURCES
        src/TCMT-CLI/main.cpp
        src/TCMT-CLI/SharedMemoryReader.cpp
        src/TCMT-CLI/DiagnosticsPipeClient.cpp
        src/TCMT-CLI/CLISystemInfo.cpp
        src/TCMT-CLI/DataUpdateManager.cpp
        src/TCMT-CLI/DisplayFormatter.cpp
        src/TCMT-CLI/TerminalRenderer.cpp
    )
    
    # CLI 可执行文件
    add_executable(TCMT-CLI ${CLI_SOURCES})
    
    # CLI 包含目录
    target_include_directories(TCMT-CLI
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/core
            ${CMAKE_SOURCE_DIR}/src/core/common
            ${CMAKE_SOURCE_DIR}/src/CPP-parsers/include
            ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern
            ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/json/single_include
    )
    
    # CLI 链接库
    target_link_libraries(TCMT-CLI
        PRIVATE
            CoreLib
            CPP-parsers
    )
    
    # CLI 平台特定库
    if(APPLE)
        target_link_libraries(TCMT-CLI
            PRIVATE
            "-framework CoreFoundation"
            "-framework IOKit"
            "-framework SystemConfiguration"
            "-framework Foundation"
        )
    endif()
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(TCMT-CLI
            PRIVATE
            pthread
            dl
            rt
        )
    endif()
    
    # CLI 属性
    set_target_properties(TCMT-CLI PROPERTIES
        CXX_STANDARD 20
        POSITION_INDEPENDENT_CODE ON
        FOLDER "CLI"
    )
    
    # CLI 编译选项
    target_compile_options(TCMT-CLI PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
endif()

# ============================================================================
# 2. 跨平台主程序 (TCMT-Main)
# ============================================================================

if(WIN32)
    # Windows 主程序源文件
    set(MAIN_SOURCES
        src/main.cpp
    )
    
    file(GLOB_RECURSE MAIN_PLATFORM_SOURCES
        "src/core/*.cpp"
        "src/core/*.h"
    )
    list(FILTER MAIN_PLATFORM_SOURCES EXCLUDE REGEX ".*Test.*")
    list(FILTER MAIN_PLATFORM_SOURCES EXCLUDE REGEX ".*Demo.*")
    list(APPEND MAIN_SOURCES ${MAIN_PLATFORM_SOURCES})
    
    # Windows 主程序可执行文件
    add_executable(TCMT-Main ${MAIN_SOURCES})
    
    # Windows 主程序属性
    set_target_properties(TCMT-Main PROPERTIES
        WIN32_EXECUTABLE FALSE
        VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
        VS_DOTNET_REFERENCES "System;mscorlib;LibreHardwareMonitorLib"
        COMMON_LANGUAGE_RUNTIME "/clr"
        VS_GLOBAL_COMPILE_OPTIONS "/ZW /EHsc"
        FOLDER "Main"
    )
    
    # Windows 主程序包含目录
    target_include_directories(TCMT-Main
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/core
            ${CMAKE_SOURCE_DIR}/src/core/common
            ${CMAKE_SOURCE_DIR}/src/core/Utils
            ${CMAKE_SOURCE_DIR}/src/third_party
    )
    
    # Windows 主程序编译定义
    target_compile_definitions(TCMT-Main
        PRIVATE
            UNICODE
            _UNICODE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )
    
    # Windows 主程序链接库
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
    
    # CUDA 支持 (仅 Windows)
    if(ENABLE_CUDA)
        find_package(CUDAToolkit 12.6 QUIET)
        if(CUDAToolkit_FOUND)
            target_link_libraries(TCMT-Main PRIVATE CUDA::cudart)
            target_compile_definitions(TCMT-Main PRIVATE CUDA_SUPPORTED=1)
        endif()
    endif()
    
else()
    # 跨平台主程序源文件 (macOS/Linux)
    set(CROSSPLATFORM_MAIN_SOURCES
        src/main_crossplatform.cpp
        src/platform/CrossPlatformHardwareInfo.cpp
    )
    
    # 跨平台主程序可执行文件
    add_executable(TCMT-Main ${CROSSPLATFORM_MAIN_SOURCES})
    
    # 跨平台主程序包含目录
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
    
    # 跨平台主程序编译定义
    target_compile_definitions(TCMT-Main
        PRIVATE
            UNICODE
            _UNICODE
    )
    
    # 跨平台主程序链接库
    target_link_libraries(TCMT-Main
        PRIVATE
            CoreLib
            CPP-parsers
    )
    
    # 跨平台主程序属性
    set_target_properties(TCMT-Main PROPERTIES
        CXX_STANDARD 20
        POSITION_INDEPENDENT_CODE ON
        FOLDER "Main"
    )
    
    # 跨平台主程序平台特定库
    if(APPLE)
        target_link_libraries(TCMT-Main
            PRIVATE
            "-framework CoreFoundation"
            "-framework IOKit"
            "-framework SystemConfiguration"
            "-framework Foundation"
        )
        
        target_compile_options(TCMT-Main PRIVATE
            -Wall -Wextra -Wpedantic
            -Wno-unused-parameter
        )
    endif()
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(TCMT-Main
            PRIVATE
            pthread
            dl
            rt
        )
        
        target_compile_options(TCMT-Main PRIVATE
            -Wall -Wextra -Wpedantic
            -Wno-unused-parameter
        )
    endif()
endif()

# ============================================================================
# 3. 动态库 (TCMT-Core-Dynamic)
# ============================================================================

# 动态库源文件
set(DYNAMIC_SOURCES
    src/core/DataStruct/SharedMemoryManager.cpp
    src/core/DataStruct/Producer.cpp
    src/core/usb/USBInfo.cpp
    src/platform/CrossPlatformHardwareInfo.cpp
    src/platform/CrossPlatformTemperatureMonitor.cpp
    src/core/Utils/Logger.cpp
)

# 动态库
add_library(TCMT-Core-Dynamic SHARED ${DYNAMIC_SOURCES})

# 动态库包含目录
target_include_directories(TCMT-Core-Dynamic
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/core/common
        ${CMAKE_SOURCE_DIR}/src/platform
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/include
        ${CMAKE_SOURCE_DIR}/src/CPP-parsers/extern/json/single_include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/third_party
)

# 动态库编译定义
target_compile_definitions(TCMT-Core-Dynamic
    PUBLIC
        TCMT_CORE_DYNAMIC_EXPORTS
    PRIVATE
        UNICODE
        _UNICODE
        $<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
        $<$<PLATFORM_ID:Darwin>:PLATFORM_MACOS>
        $<$<PLATFORM_ID:Linux>:PLATFORM_LINUX>
        $<$<CXX_COMPILER_ID:MSVC>:ARCH_X64>
        $<$<CXX_COMPILER_ID:GNU>:ARCH_X64>
        $<$<CXX_COMPILER_ID:Clang>:ARCH_X64>
)

# 动态库链接库
target_link_libraries(TCMT-Core-Dynamic
    PRIVATE
    CPP-parsers
)

# 动态库属性
set_target_properties(TCMT-Core-Dynamic PROPERTIES
    CXX_STANDARD 20
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    FOLDER "Dynamic Libraries"
)

# 动态库平台特定库
if(APPLE)
    target_link_libraries(TCMT-Core-Dynamic
        PRIVATE
        "-framework CoreFoundation"
        "-framework IOKit"
        "-framework SystemConfiguration"
        "-framework DiskArbitration"
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(TCMT-Core-Dynamic
        PRIVATE
        pthread
        dl
        rt
    )
endif()

# ============================================================================
# 4. 输出目录配置
# ============================================================================

# 设置所有目标的输出目录
foreach(TARGET TCMT-Main TCMT-CLI TCMT-Core-Dynamic)
    set_target_properties(${TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release
    )
endforeach()

# ============================================================================
# 5. 安装规则
# ============================================================================()

# 安装可执行文件
install(TARGETS TCMT-Main TCMT-CLI
    RUNTIME DESTINATION bin
    COMPONENT applications
)

# 安装动态库
install(TARGETS TCMT-Core-Dynamic
    LIBRARY DESTINATION lib
    COMPONENT libraries
)

# 安装静态库
install(TARGETS CoreLib CPP-parsers
    ARCHIVE DESTINATION lib
    COMPONENT libraries
)