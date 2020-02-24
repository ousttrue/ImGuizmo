set(TARGET_NAME ImGuizmoExample)
add_executable(${TARGET_NAME}
    main.cpp
    ImApp.cpp
    glutil.cpp
    )
target_link_libraries(${TARGET_NAME} PRIVATE
    ImGuizmo
    )
target_compile_definitions(${TARGET_NAME} PRIVATE
    )
