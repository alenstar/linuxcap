# Build steps
#file(GLOB_RECURSE APP_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
set(APP_SRC_FILES 
	main.cpp
	encoder.c
	cap.c
	nano_bmp.c
	utils.c
	)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/ext/libyuv/include)
include_directories(${PROJECT_SOURCE_DIR}/ext/uWebSockets/src)
include_directories(/usr/include/cairo)
#include_directories(/usr/include)
#add_dependencies(${APP_NAME} ${APP_LIB_NAME})
#add_dependencies(${APP_NAME} ${ly_lib_name})
add_executable(${APP_NAME} ${APP_SRC_FILES})
add_dependencies(${APP_NAME} ${ly_lib_name})
add_dependencies(${APP_NAME} uWS)
#target_include_directories(${APP_NAME} PUBLIC ../src)
target_link_libraries(${APP_NAME} LINK_PUBLIC uWS ${ly_lib_name}  m c pthread z cairo X11 Xext x264 x265)
#install(TARGETS ${APP_NAME} RUNTIME DESTINATION bin)
