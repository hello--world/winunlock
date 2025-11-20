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

# 确保 Windows SDK 包含路径被正确设置
if(MSVC)
    # Visual Studio 和 CMake 应该自动处理 Windows SDK 路径
    # 但我们可以显式添加以确保 credentialprovider.h 能被找到
    # 获取 Windows SDK 根目录
    get_filename_component(WINDOWS_SDK_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
    
    # 如果注册表方法失败，尝试从环境变量获取
    if(NOT WINDOWS_SDK_DIR)
        set(WINDOWS_SDK_DIR "$ENV{ProgramFiles(x86)}/Windows Kits/10" CACHE PATH "Windows SDK Directory")
    endif()
    
    if(EXISTS "${WINDOWS_SDK_DIR}/Include")
        # 查找最新的 SDK 版本
        file(GLOB SDK_VERSIONS "${WINDOWS_SDK_DIR}/Include/*")
        if(SDK_VERSIONS)
            list(SORT SDK_VERSIONS)
            list(REVERSE SDK_VERSIONS)
            list(GET SDK_VERSIONS 0 LATEST_SDK)
            get_filename_component(LATEST_SDK_VERSION ${LATEST_SDK} NAME)
            message(STATUS "Found Windows SDK version: ${LATEST_SDK_VERSION}")
            message(STATUS "Adding Windows SDK include paths")
            target_include_directories(WinUnlock PRIVATE
                "${WINDOWS_SDK_DIR}/Include/${LATEST_SDK_VERSION}/um"
                "${WINDOWS_SDK_DIR}/Include/${LATEST_SDK_VERSION}/shared"
            )
        endif()
    else()
        message(WARNING "Windows SDK directory not found. CMake should auto-detect it.")
    endif()
endif()

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

