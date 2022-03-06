# CMakeList.txt : CMake project for POKEY Frequencies Calculator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("POKEY Frequencies Calculator")

# Add source to this project's executable.
add_executable (POKECALC "main.cpp" "main.h"  )
target_link_libraries(POKECALC PUBLIC "-static")

# TODO: Add tests and install targets if needed.




