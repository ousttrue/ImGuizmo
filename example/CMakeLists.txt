set(TARGET_NAME ImGuizmoExample)
add_library(${TARGET_NAME}
    main.cpp
    )
target_link_libraries(${TARGET_NAME} PRIVATE
    ImGuizmo
    )