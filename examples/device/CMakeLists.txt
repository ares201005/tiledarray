#
#  This file is a part of TiledArray.
#  Copyright (C) 2018  Virginia Tech
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  Chong Peng
#  Department of Chemistry, Virginia Tech
#
#  CMakeLists.txt
#  Aug 01, 2018
#


if(TILEDARRAY_HAS_CUDA OR TILEDARRAY_HAS_HIP)

    foreach(_exec device_task ta_dense_device ta_cc_abcd_device ta_vector_device ta_reduce_device)

        # Add executable
        add_ta_executable(${_exec} "${_exec}.cpp" "tiledarray")
        add_dependencies(examples-tiledarray ${_exec})

    endforeach()

endif()
