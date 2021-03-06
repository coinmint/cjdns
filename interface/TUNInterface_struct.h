/* vim: set expandtab ts=4 sw=4: */
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TUNInterface_struct_H
#define TUNInterface_struct_H

#include "interface/Interface.h"
#include "benc/String.h"

#include <event2/event.h>

struct TUNInterface
{
    struct event* incomingEvent;
    int fileDescriptor;
    String* name;
    struct Interface interface;
};

#endif
