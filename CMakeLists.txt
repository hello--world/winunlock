cmake_minimum_required(VERSION 3.15)
project(WinUnlock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加源文件
add_library(WinUnlock SHARED
    CredentialProvider.cpp
    CredentialProvider.h
    dllmain.cpp
    WinUnlock.def
)

# 包含目录
target_include_directories(WinUnlock PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
target_link_libraries(WinUnlock PRIVATE
    netapi32
    wtsapi32
    userenv
    crypt32
    advapi32
    shlwapi
    ole32
    oleaut32
    secur32
    ntdll
)

# 设置 DLL 输出名称
set_target_properties(WinUnlock PROPERTIES
    OUTPUT_NAME "WinUnlock"
    PREFIX ""
)

# 编译选项
if(MSVC)
    # 设置 Windows SDK 版本（确保包含 credentialprovider.h）
    set(CMAKE_SYSTEM_VERSION "10.0" CACHE STRING "Windows SDK Version")
    
    target_compile_options(WinUnlock PRIVATE
        /W4
        /WX-
        /EHsc
        /permissive-
    )
    # 设置子系统为 Windows
    set_target_properties(WinUnlock PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
    # 确保使用 Windows 10 SDK
    set_target_properties(WinUnlock PROPERTIES
        VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0"
    )
else()
    target_compile_options(WinUnlock PRIVATE
        -Wall
        -Wextra
    )
endif()

