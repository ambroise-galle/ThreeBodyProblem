cmake_minimum_required(VERSION 3.10)
project(ThreeBodyProblem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
find_package(Eigen3 REQUIRED)

add_subdirectory(imgui-sfml)

include_directories(
    ${EIGEN3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-sfml
)


set(IMGUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-sfml/imgui-SFML.cpp  # Fichier SFML backend
)

add_executable(ThreeBodyProblem main.cpp ${IMGUI_SRC})


target_link_libraries(ThreeBodyProblem 
    PRIVATE
    sfml-graphics 
    sfml-window 
    sfml-system 
    ImGui-SFML::ImGui-SFML
)

target_compile_options(ThreeBodyProblem PRIVATE ${IMGUI_SFML_WARNINGS})
