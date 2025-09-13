# tests/CMakeLists.txt

# test source auto collection : use CONFIGURE_DEPENDS for CMake to find out new file append.
file(GLOB TEST_SOURCES CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# common function : make test file to independent target/ctest 
function(add_unit_test_from_file TEST_SRC)
    get_filename_component(_name ${TEST_SRC} NAME_WE)
    set(_tgt "ut_${_name}")

    add_executable(${_tgt} ${TEST_SRC})
    target_link_libraries(${_tgt} PRIVATE
        core
        GTest::gtest_main
    )
    target_compile_features(${_tgt} PRIVATE cxx_std_20)

    # San/Coverage flag
    if(ENABLE_SANITIZERS)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${_tgt} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${_tgt} PRIVATE    -fsanitize=address,undefined -fno-omit-frame-pointer)
        elseif(MSVC)
        # MSVC(Clang-cl가 아니면 ASan 지원 제한). 가능한 경우에만 활성화.
        # clang-cl 사용 시: /fsanitize=address
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${_tgt} PRIVATE /fsanitize=address /Zi)
            target_link_options(${_tgt}    PRIVATE /fsanitize=address)
        endif()
        endif()
    endif()

    if(ENABLE_CODE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${_tgt} PRIVATE --coverage)
            target_link_options(${_tgt} PRIVATE --coverage)
        endif()
    endif()

    # ctest register 
    add_test(NAME ${_tgt} COMMAND ${_tgt})
    set_tests_properties(${_tgt} PROPERTIES
        LABELS "unit;daily"
        TIMEOUT 15
    )
endfunction()

# 수집된 모든 테스트 파일을 타겟으로 변환
foreach(T IN LISTS TEST_SOURCES)
  add_unit_test_from_file(${T})
endforeach()

# "매일 돌리는" 레이블만 실행하는 타겟
add_custom_target(test-daily
  COMMAND ${CMAKE_CTEST_COMMAND} -L daily --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run tests labeled 'daily'"
)