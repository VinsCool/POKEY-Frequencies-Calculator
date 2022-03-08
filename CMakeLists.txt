cmake_minimum_required (VERSION 3.8)

project ("POKEY Frequencies Calculator")

option (FORCE_STATIC_LINKING "Force static linking in case it's not the compiler's default" OFF)

set (SRCS
	main.cpp
)

add_executable (POKECALC ${SRCS})

if (FORCE_STATIC_LINKING)
	if (MSVC)
		message ("TODO: Implement forced static linking on MSVC")
	else ()
		target_link_libraries(POKECALC PUBLIC "-static")
	endif ()
endif (FORCE_STATIC_LINKING)

install (TARGETS POKECALC RUNTIME DESTINATION "bin")
