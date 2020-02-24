set(TARGET_NAME imgui)
add_library(${TARGET_NAME}
    imgui.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    )
target_include_directories(${TARGET_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )
