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
		source/chronotimer.h
		source/structures.h
		source/level.h
		source/level.cpp
		source/materialmanager.h
		source/materialmanager.cpp
		source/texturemanager.h
		source/texturemanager.cpp
	)

	# CMake FXC shader compilation, add any shaders you want compiled here
	set(VERTEX_SHADERS 
		# add vertex shader (.hlsl) files here
		shaders/vertexshader.hlsl
		shaders/vertexshaderskybox.hlsl
	)
	set(PIXEL_SHADERS 
		# add pixel shader (.hlsl) files here
		shaders/pixelshader.hlsl
		shaders/pixelshaderskybox.hlsl
	)
	set(IMGUI
		imgui/imconfig.h
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_internal.h
		imgui/imgui_tables.cpp
		imgui/imgui_widgets.cpp
		imgui/imgui.h
		imgui/imgui.cpp
		imgui/imgui_impl_dx12.h
		imgui/imgui_impl_dx12.cpp
		imgui/imgui_impl_win32.h
		imgui/imgui_impl_win32.cpp
	)

	# Add any new C/C++ source code here
	add_executable (Project_DX12 
		# Header & CPP files go here
		${SOURCE}
		${VERTEX_SHADERS}
		${PIXEL_SHADERS}
		${IMGUI}
	)

	# This code will tell visual studio to convert above shader .hlsl to .h files
	# The produced .h files can be included and contain the compiled shader byte code
	set_source_files_properties( ${VERTEX_SHADERS} PROPERTIES 
		VS_SHADER_TYPE Vertex 
		VS_SHADER_MODEL 5.1 
		VS_SHADER_ENTRYPOINT main
		VS_SHADER_FLAGS /enable_unbounded_descriptor_tables
		#VS_TOOL_OVERRIDE "None"
		#VS_SHADER_OUTPUT_HEADER_FILE "%(RelativeDir)%(Filename).h"
		#VS_SHADER_VARIABLE_NAME "%(Filename)"
		#VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
	)
	set_source_files_properties( ${PIXEL_SHADERS} PROPERTIES 
		VS_SHADER_TYPE Pixel 
		VS_SHADER_MODEL 5.1 
		VS_SHADER_ENTRYPOINT main
		VS_SHADER_FLAGS /enable_unbounded_descriptor_tables
		#VS_TOOL_OVERRIDE "None"
		#VS_SHADER_OUTPUT_HEADER_FILE "%(RelativeDir)%(Filename).h"
		#VS_SHADER_VARIABLE_NAME "%(Filename)"
		#VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
	)
	# add support for dds texture loading
	include_directories(${CMAKE_SOURCE_DIR}/directxtk12/include)
	# Find the libraries
	find_library(DDS_LIB_D NAMES DirectXTK12 PATHS ${CMAKE_SOURCE_DIR}/directxtk12/lib/x64/Debug)
	find_library(DDS_LIB_R NAMES DirectXTK12 PATHS ${CMAKE_SOURCE_DIR}/directxtk12/lib/x64/Release)
	# link the ktx sdk include and lib files
	target_link_libraries(Project_DX12 debug ${DDS_LIB_D} optimized ${DDS_LIB_R})
	
	#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMGUI})
endif(WIN32)