cmake_minimum_required(VERSION 3.4)

project(IMGSEL-QT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_MODULE_PATH ${IMGSEL_SOURCE_DIR}/cmake)

add_executable(${PROJECT_NAME} main.cpp gui/picker/ImagePickerDrawer.cpp gui/picker/ImagePickerDrawer.h util/exceptions/OutOfBounds.h gui/drawer/ShapeDrawer.h gui/util/Shape.h gui/util/Dimensions.h
        input/handler/instruction/InputInstructionType.h input/handler/filters/FilteringInputHandler.h input/handler/InputHandler.cpp input/handler/filters/FilteringInputHandler.cpp
        input/handler/filters/TextFilteringInputHandler.cpp input/handler/filters/TextFilteringInputHandler.h input/handler/InputHandlerFactory.cpp input/handler/InputHandlerFactory.h
        input/handler/SelectionInputHandler.cpp input/handler/SelectionInputHandler.h input/handler/instruction/InputInstruction.h input/handler/instruction/MoveInstruction.h
        input/handler/instruction/ModeChangeInstruction.h input/handler/instruction/FilterInstruction.h util/config/ConfigManager.cpp util/config/ConfigManager.h util/config/Config.h util/config/ConfigBuilder.h
        gui/drawer/ImageDrawer.cpp gui/drawer/ImageDrawer.h input/handler/instruction/CopyInstruction.h util/exceptions/ImageNotLoadable.h gui/picker/ImagePickerMove.h util/lib/CLI11.hpp)
find_package(Qt5Widgets REQUIRED)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
)
if (NOT CLANG_TIDY_EXE)
    message(STATUS "clang-tidy not found.")
else ()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*")
endif ()

set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        COMPILE_FLAGS "${WARNING_FLAGS}"
)
