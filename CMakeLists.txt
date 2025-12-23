cmake_minimum_required(VERSION 3.25)
project(starsector-nano)

set(CMAKE_CXX_STANDARD 17)

# 设置EasyX路径（改为相对路径）
set(EASYX_PATH "${CMAKE_SOURCE_DIR}/easyx")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
# 包含头文件（改为相对路径）
include_directories(${EASYX_PATH}/include)
include_directories("${CMAKE_SOURCE_DIR}/json-3.11.3/include")

# 链接库目录（改为相对路径）
link_directories(${EASYX_PATH}/lib64)

add_executable(starsector-nano WIN32 src/main.cpp
        include/Vector2f.h
        src/tool/Vector2f.cpp
        include/Timer.h
        src/tool/Timer.cpp
        src/tool/Manager.cpp
        include/Entity.h
        include/Manager.h
        src/Entity.cpp
        src/tool/Animation.cpp
        include/Animation.h
        src/effect/Distortion.cpp
        include/Distortion.h
        include/Engine.h
        include/Polygon.h
        src/tool/Polygon.cpp
        src/tool/cJSON.c
        include/ImageProcessing.h
        src/effect/ImageProcessing.cpp
        include/ShipAPI.h
        src/Engine.cpp
        src/ShipAPI.cpp
        src/Background.cpp
        include/Background.h
        src/tool/MathTool.cpp
        include/MathTool.h
        src/Camera.cpp
        include/Camera.h
        include/ShipPool.h
        src/WeaponAPI.cpp
        include/WeaponAPI.h
        src/ShipPool.cpp
        src/Control.cpp
        include/Control.h
        src/BulletAPI.cpp
        include/BulletAPI.h
        src/BulletPool.cpp
        include/BulletPool.h
        src/effect/ExplosionAPI.cpp
        include/ExplosionAPI.h
        src/ExplosionPool.cpp
        include/ExplosionPool.h
        src/tool/ThreadPool.cpp
        include/ThreadPool.h
        src/GlobalCombatData.cpp
        include/GlobalCombatData.h
        include/CustomFSM.h
        src/CustomFSM.cpp
        include/TacticalMap.h
        src/TacticalMap.cpp
        src/effect/SequencedExplosion.cpp
        include/SequencedExplosion.h
)

target_include_directories(starsector-nano PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 链接库（注意顺序）
target_link_libraries(starsector-nano
        easyx
        gdi32
        ole32
        kernel32
        user32
        winspool
        shell32
        oleaut32
        uuid
        comdlg32
        advapi32
        Msimg32
)
