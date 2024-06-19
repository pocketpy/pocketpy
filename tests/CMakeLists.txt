cmake_minimum_required(VERSION 3.10)

function(pkpy_add_test pyfile)
    get_filename_component(test_name ${pyfile} NAME_WE)
    add_test(
        NAME ${test_name} 
        COMMAND $<TARGET_FILE:main> ${pyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    )
    message("Added test " ${test_name})
endfunction()

file(GLOB PK_PYTHON_TESTCASES_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR}/.. "*.py")

foreach(pyfile ${PK_PYTHON_TESTCASES_FILES})
    pkpy_add_test(${pyfile})
endforeach()
