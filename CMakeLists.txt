cmake_minimum_required(VERSION 3.15)
project(WinUnlock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录（Visual Studio 生成器会自动添加配置目录）
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

# 对于 Visual Studio 生成器，确保 Release 配置输出到正确位置
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_target_properties(WinUnlock PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin/MinSizeRel
    )
endif()

# 包含目录
target_include_directories(WinUnlock PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 确保 Windows SDK 包含路径被正确设置
if(MSVC)
    # 获取 Windows SDK 版本（CMake 自动检测的）
    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(WINDOWS_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
    else()
        set(WINDOWS_SDK_VERSION "10.0")
    endif()
    
    # 获取 Windows SDK 根目录
    get_filename_component(WINDOWS_SDK_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
    
    # 如果注册表方法失败，尝试从环境变量获取
    if(NOT WINDOWS_SDK_DIR)
        set(WINDOWS_SDK_DIR "$ENV{ProgramFiles(x86)}/Windows Kits/10" CACHE PATH "Windows SDK Directory")
    endif()
    
    # 尝试多个可能的 SDK 版本路径
    set(POSSIBLE_SDK_PATHS
        "${WINDOWS_SDK_DIR}/Include/${WINDOWS_SDK_VERSION}/um"
        "${WINDOWS_SDK_DIR}/Include/10.0.26100.0/um"
        "${WINDOWS_SDK_DIR}/Include/10.0.22621.0/um"
        "${WINDOWS_SDK_DIR}/Include/10.0.22000.0/um"
        "${WINDOWS_SDK_DIR}/Include/10.0.19041.0/um"
    )
    
    # 查找存在的 SDK 路径
    foreach(SDK_PATH ${POSSIBLE_SDK_PATHS})
        if(EXISTS "${SDK_PATH}/credentialprovider.h")
            message(STATUS "Found credentialprovider.h at: ${SDK_PATH}")
            get_filename_component(SDK_UM_DIR ${SDK_PATH} ABSOLUTE)
            get_filename_component(SDK_SHARED_DIR "${SDK_UM_DIR}/../shared" ABSOLUTE)
            target_include_directories(WinUnlock PRIVATE
                ${SDK_UM_DIR}
                ${SDK_SHARED_DIR}
            )
            message(STATUS "Added Windows SDK include paths: ${SDK_UM_DIR}, ${SDK_SHARED_DIR}")
            break()
        endif()
    endforeach()
    
    # 如果还是没找到，尝试查找所有版本
    if(EXISTS "${WINDOWS_SDK_DIR}/Include")
        file(GLOB SDK_VERSIONS "${WINDOWS_SDK_DIR}/Include/*")
        if(SDK_VERSIONS)
            list(SORT SDK_VERSIONS)
            list(REVERSE SDK_VERSIONS)
            foreach(SDK_VERSION_DIR ${SDK_VERSIONS})
                set(TEST_PATH "${SDK_VERSION_DIR}/um/credentialprovider.h")
                if(EXISTS "${TEST_PATH}")
                    get_filename_component(SDK_VERSION ${SDK_VERSION_DIR} NAME)
                    message(STATUS "Found credentialprovider.h in SDK version: ${SDK_VERSION}")
                    target_include_directories(WinUnlock PRIVATE
                        "${SDK_VERSION_DIR}/um"
                        "${SDK_VERSION_DIR}/shared"
                    )
                    break()
                endif()
            endforeach()
        endif()
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
        /wd4996  # 禁用已弃用函数警告
        /wd4267  # 禁用 size_t 转换警告
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

