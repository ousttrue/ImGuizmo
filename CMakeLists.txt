cmake_minimum_required(VERSION 3.0.0)
project(ImGuizmoExample)
set(TARGET_NAME ${PROJECT_NAME})
add_executable(${PROJECT_NAME}
    example/main.cpp
    example/imgui.cpp
    example/imgui_draw.cpp
    example/imgui_widgets.cpp
    ImGuizmo.cpp
    ImSequencer.cpp
    ImCurveEdit.cpp
    )
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/example
    )
