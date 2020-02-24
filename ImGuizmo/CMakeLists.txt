set(TARGET_NAME ImGuizmo)
add_library(${TARGET_NAME}
    ImGuizmo.cpp
    ImSequencer.cpp
    ImCurveEdit.cpp
    )
target_include_directories(${TARGET_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )
target_link_libraries(${TARGET_NAME}
    PUBLIC
    imgui
    )
