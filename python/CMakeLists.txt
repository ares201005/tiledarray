cmake_minimum_required(VERSION 3.12)

project(python-tiledarray)

if (NOT TARGET Python::Module)
  find_package(Python COMPONENTS Interpreter Development REQUIRED)
endif()

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY      https://github.com/ValeevGroup/pybind11.git
  GIT_TAG             ValeevGroup/v2.11
  )
FetchContent_MakeAvailable(pybind11)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)

pybind11_add_module(python-tiledarray MODULE src/tiledarray.cc)

# mpi c libs need come from tiledarray
# Eigen and Boost also transitively come from tiledarray
target_link_libraries(python-tiledarray PRIVATE tiledarray)

# pybind uses sized deallocations when using C++17, clang requires -fsized-deallocation compiler flag to enable them
# see https://reviews.llvm.org/D8467 and https://github.com/pybind/pybind11/issues/1818
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(python-tiledarray PRIVATE -fsized-deallocation)
endif()

set_target_properties(
  python-tiledarray
  PROPERTIES
  #PREFIX ""
  OUTPUT_NAME tiledarray
  )

# tests
if (BUILD_TESTING)
  # build step
  add_test(tiledarray/unit/python/build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target python-tiledarray)
  set_tests_properties(tiledarray/unit/python/build PROPERTIES FIXTURES_SETUP TA_UNIT_TESTS_PYTHON_EXEC)

  # check for presence of prerequisite modules
  foreach(_mod pytest numpy)
    execute_process(COMMAND ${Python_EXECUTABLE} -c "import ${_mod}"
        OUTPUT_QUIET ERROR_QUIET
        RESULTS_VARIABLE check_for_${_mod})
    if (check_for_${_mod})
      message(FATAL_ERROR "Python module \"${_mod}\" is not installed; install via \"${Python_EXECUTABLE} -m pip install ${_mod}\" and rerun cmake")
    endif(check_for_${_mod})
  endforeach(_mod)

  # test step
  add_test(
    NAME tiledarray/unit/python/run
    # need to use pytest to find tiledarray module properly
    COMMAND ${Python_EXECUTABLE} -m pytest ${PROJECT_SOURCE_DIR}/test_tiledarray.py -v
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
  set_tests_properties(tiledarray/unit/python/run
          PROPERTIES FIXTURES_REQUIRED TA_UNIT_TESTS_PYTHON_EXEC
          ENVIRONMENT MAD_NUM_THREADS=2)

endif(BUILD_TESTING)
