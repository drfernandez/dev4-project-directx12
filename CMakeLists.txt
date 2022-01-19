cmake_minimum_required(VERSION 3.16)

project(Project_DX12)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if(WIN32)
	# by default CMake selects "ALL_BUILD" as the startup project
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		PROPERTY VS_STARTUP_PROJECT Project_DX12)
		
	set(SOURCE
		source/gateware.h
		source/main.cpp
		source/renderer.h
		source/d3dx12.h
		source/xtime.h
		source/xtime.cpp
		source/chronotimer.h
		source/structures.h
		source/level.h
		source/level.cpp
		source/materialmanager.h
		source/materialmanager.cpp
	)

	# CMake FXC shader compilation, add any shaders you want compiled here
	set(VERTEX_SHADERS 
		# add vertex shader (.hlsl) files here
		shaders/vertexshader.hlsl
	)
	set(PIXEL_SHADERS 
		# add pixel shader (.hlsl) files here
		shaders/pixelshader.hlsl
	)

	# Add any new C/C++ source code here
	add_executable (Project_DX12 
		# Header & CPP files go here
		${SOURCE}
		${VERTEX_SHADERS}
		${PIXEL_SHADERS}
	)

	# This code will tell visual studio to convert above shader .hlsl to .h files
	# The produced .h files can be included and contain the compiled shader byte code
	set_source_files_properties( ${VERTEX_SHADERS} PROPERTIES 
		VS_SHADER_TYPE Vertex 
		VS_SHADER_MODEL 6.0 
		VS_SHADER_ENTRYPOINT main 
		#VS_TOOL_OVERRIDE "None"
		#VS_SHADER_OUTPUT_HEADER_FILE "%(RelativeDir)%(Filename).h"
		#VS_SHADER_VARIABLE_NAME "%(Filename)"
		#VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
	)
	set_source_files_properties( ${PIXEL_SHADERS} PROPERTIES 
		VS_SHADER_TYPE Pixel 
		VS_SHADER_MODEL 6.0 
		VS_SHADER_ENTRYPOINT main
		#VS_TOOL_OVERRIDE "None"
		#VS_SHADER_OUTPUT_HEADER_FILE "%(RelativeDir)%(Filename).h"
		#VS_SHADER_VARIABLE_NAME "%(Filename)"
		#VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
	)
	
	#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})
endif(WIN32)