cmake_minimum_required(VERSION 3.4)

project(IMGSEL)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_MODULE_PATH ${IMGSEL_SOURCE_DIR}/cmake)

execute_process(COMMAND git submodule update --init ${CMAKE_CURRENT_SOURCE_DIR}/lib/keycode/depp-keycode)
execute_process(COMMAND git submodule update --init ${CMAKE_CURRENT_SOURCE_DIR}/lib/CLI11)

add_executable(${PROJECT_NAME} old_main.cpp gui/WindowManager.cpp gui/ImagePickerDrawer.cpp gui/ImagePickerDrawer.h exceptions/OutOfBounds.h gui/drawer/ShapeDrawer.h gui/Shape.h  gui/drawer/ShapeDrawerFactory.cpp gui/drawer/ShapeDrawerFactory.h input/x11_keycodes.h gui/dimensions.h input/handler/InputHandler.h input/handler/InputMode.h input/handler/instruction/InputInstructionType.h input/handler/filters/FilteringInputHandler.h input/handler/InputHandler.cpp input/handler/filters/FilteringInputHandler.cpp input/handler/filters/TextFilteringInputHandler.cpp input/handler/filters/TextFilteringInputHandler.h input/handler/InputHandlerFactory.cpp input/handler/InputHandlerFactory.h input/handler/SelectionInputHandler.cpp input/handler/SelectionInputHandler.h input/handler/instruction/InputInstruction.h input/handler/instruction/MoveInstruction.h input/handler/instruction/ModeChangeInstruction.h input/handler/instruction/FilterInstruction.h config/ConfigManager.cpp config/ConfigManager.h config/Config.h config/ConfigBuilder.h gui/drawer/ImageDrawer.cpp gui/drawer/ImageDrawer.h input/handler/instruction/CopyInstruction.h util/ThreadSafeQueue.h input/XInputHandler.cpp input/InstructionRouter.cpp input/XEventWrapper.h util/helpers.h input/XInputHandler.h input/InstructionRouter.h util/helpers.cpp input/x11_keycodes.cpp exceptions/ImageNotLoadable.h main.cpp gui/ImagePickerMove.h)
add_subdirectory(lib/keycode)
add_subdirectory(lib/CLI11)

FIND_PACKAGE(X11)
pkg_check_modules(X11 REQUIRED x11 xinerama xtst)
find_library(XTST_LIBRARY Xtst)

find_package(Qt5Widgets REQUIRED)

find_package(Imlib2)
find_package(Threads)

if(!Imlib2_FOUND)
    message( FATAL_ERROR "Imlib2 is required!")
endif()

target_link_libraries(${PROJECT_NAME} CLI11::CLI11)
target_link_libraries(${PROJECT_NAME} ${XTST_LIBRARY} ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${XINERAMA_LIBRARIES} stdc++fs)
target_link_libraries(${PROJECT_NAME} ${Imlib2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} keycode)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

if (NOT X11_Xinerama_FOUND)
  message(FATAL_ERROR "The Xinerama headers were not found")
endif()

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
  )
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*")
endif()

set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  COMPILE_FLAGS "${WARNING_FLAGS}"
)

#if(CLANG_TIDY_EXE)
#  set_target_properties(
#    ${PROJECT_NAME} PROPERTIES
#    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
#  )
#endif()