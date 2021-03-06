# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 2.8.2)

if (NOT NACL_FOUND)

    find_path(NACL_INCLUDE_DIRS
        NAMES
            crypto_box_curve25519xsalsa20poly1305.h
        PATHS
            ${NACL_PREFIX}/include
            /usr/include
            /usr/include/nacl
            /usr/local/include
            /opt/local/include
            ${CMAKE_BINARY_DIR}/nacl/build/include/default
        NO_DEFAULT_PATH
    )

    find_library(NACL_LIBRARIES
        NAMES
            nacl
        PATHS
            ${NACL_INCLUDE_DIRS}/../lib
            ${NACL_INCLUDE_DIRS}/../../lib
        NO_DEFAULT_PATH
    )

    if(NACL_INCLUDE_DIRS AND NACL_LIBRARIES)
        message("libnacl found: ${NACL_INCLUDE_DIRS}")
        set(NACL_FOUND TRUE)
    endif()

endif()

if(NOT NACL_FOUND)
    include(ExternalProject)

    # Without this, the build doesn't happen until link time.
    include_directories(${NACL_USE_FILES})

    ExternalProject_Add(NACL
        GIT_REPOSITORY git://github.com/cjdelisle/nacl.git
        GIT_TAG c737ed821164f397a1a0639b9361c66edf2d4000
        SOURCE_DIR "${CMAKE_BINARY_DIR}/nacl"
        BINARY_DIR "${CMAKE_BINARY_DIR}/nacl"
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
    )

    set(NACL_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/nacl/build/include/default/")
    set(NACL_LIBRARIES    "${CMAKE_BINARY_DIR}/nacl/build/lib/default/libnacl.a")
    message("libnacl not found, will be downloaded and compiled.")
    set(NACL_FOUND TRUE)

endif()
