find_package(Python3 REQUIRED)

file(GLOB_RECURSE POCKETPY_PYLIB_SRC ${CMAKE_CURRENT_LIST_DIR}/*.py)

add_custom_target(EmbedPyLibs ALL
	COMMENT "Embedding Python library sources..."
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/../prebuild.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
	DEPENDS
		${CMAKE_CURRENT_LIST_DIR}/../prebuild.py
		${POCKETPY_PYLIB_SRC}
	BYPRODUCTS
		${CMAKE_CURRENT_LIST_DIR}/../include/pocketpy/common/_generated.hpp 
		${CMAKE_CURRENT_LIST_DIR}/../src/common/_generated.cpp
	VERBATIM
)
