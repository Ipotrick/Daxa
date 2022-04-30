
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")
add_executable(DaxaTriangle ${SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if(DAXA_DEBUG_BUILD_ASAN)
		target_compile_options(DaxaTriangle PUBLIC /fsanitize=address)
	elseif(DAXA_DEBUG_BUILD_HOT_RELOAD)
		target_compile_options(DaxaTriangle PUBLIC /ZI)
		target_link_options(DaxaTriangle PUBLIC /INCREMENTAL)
	endif()
endif()

target_link_libraries(DaxaTriangle PUBLIC daxa::daxa)
