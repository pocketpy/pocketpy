# @szdytom favored testing, set BUILD_TESTING to enable it
# You can use scripts/run_tests.py as an alternative
# Note: the CI uses scripts/run_tests.py to run the tests

cmake_minimum_required(VERSION 3.10)

function(pkpy_add_test pyfile)
    get_filename_component(test_name ${pyfile} NAME_WE)
    add_test(
        NAME ${test_name} 
        COMMAND $<TARGET_FILE:main> ${pyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    )
endfunction()

message("Testing enabled")

file(GLOB PK_PYTHON_TESTCASES_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR}/.. "*.py")

foreach(pyfile ${PK_PYTHON_TESTCASES_FILES})
    pkpy_add_test(${pyfile})
endforeach()
