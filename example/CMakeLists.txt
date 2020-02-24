set(TARGET_NAME ImGuizmoExample)
add_executable(${TARGET_NAME}
    main.cpp
    glutil.cpp
    Win32Window.cpp
    ImGuiImplScreenState.cpp
    Renderer.cpp
    camera.cpp
    MySeqence.cpp
    )
set_property(TARGET ${TARGET_NAME} 
    PROPERTY CXX_STANDARD 20
    )    
target_link_libraries(${TARGET_NAME} PRIVATE
    ImGuizmo
    winmm
    )
target_compile_definitions(${TARGET_NAME} PRIVATE
    )
