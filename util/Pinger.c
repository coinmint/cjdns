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
#include "util/Bits.h"
#include "util/Pinger.h"
#include "util/Time.h"
#include "util/Timeout.h"

struct Ping
{
    struct Pinger_Ping public;
    struct Pinger* pinger;
    struct Ping** selfPtr;
    struct Timeout* timeout;
    String* data;
    uint32_t timeSent;
    Pinger_SEND_PING(sendPing);
    Pinger_ON_RESPONSE(onResponse);
};

struct Pinger
{
    struct Ping* pings[Pinger_MAX_CONCURRENT_PINGS];
    struct event_base* eventBase;
    struct Log* logger;
    struct Allocator* allocator;
};

static void callback(String* data, struct Ping* ping)
{
    uint32_t now = Time_currentTimeMilliseconds(ping->pinger->eventBase);
    ping->onResponse(data, now - ping->timeSent, ping->public.context);
    *(ping->selfPtr) = NULL;
    ping->public.pingAlloc->free(ping->public.pingAlloc);
}

static void timeoutCallback(void* vping)
{
    callback(NULL, (struct Ping*) vping);
}

static void sendPingCallback(void* vping)
{
    struct Ping* ping = vping;
    ping->sendPing(ping->data, ping->public.context);
}

struct Pinger_Ping* Pinger_ping(String* data,
                                Pinger_ON_RESPONSE(onResponse),
                                Pinger_SEND_PING(sendPing),
                                uint32_t timeoutMilliseconds,
                                struct Pinger* pinger)
{
    struct Ping** location = NULL;
    uint32_t index;
    for (index = 0; index < Pinger_MAX_CONCURRENT_PINGS; index++) {
        if (pinger->pings[index] == NULL) {
            location = &pinger->pings[index];
            break;
        }
    }

    if (!location) {
        return NULL;
    }

    struct Allocator* alloc = pinger->allocator->child(pinger->allocator);

    // Prefix the data with the slot number.
    String* toSend = String_newBinary(NULL, ((data) ? data->len : 0) + 4, alloc);
    Bits_memcpyConst(toSend->bytes, &index, 4);
    if (data) {
        Bits_memcpy(toSend->bytes + 4, data->bytes, data->len);
    }

    struct Ping* ping = alloc->clone(sizeof(struct Ping), alloc, &(struct Ping) {
        .public = {
            .pingAlloc = alloc,
        },
        .sendPing = sendPing,
        .pinger = pinger,
        .selfPtr = location,
        .data = toSend,
        .timeSent = Time_currentTimeMilliseconds(pinger->eventBase),
        .onResponse = onResponse
    });
    ping->timeout =
        Timeout_setTimeout(timeoutCallback, ping, timeoutMilliseconds, pinger->eventBase, alloc);
    *location = ping;

    Timeout_setTimeout(sendPingCallback, ping, 0, pinger->eventBase, alloc);

    return &ping->public;
}

void Pinger_pongReceived(String* data, struct Pinger* pinger)
{
    uint32_t slot = *((uint32_t*) data->bytes);
    if (slot >= Pinger_MAX_CONCURRENT_PINGS) {
        Log_debug1(pinger->logger, "Ping out of range [%d].", slot);
    } else if (pinger->pings[slot] == NULL) {
        Log_debug(pinger->logger, "Got ping and slot value is null.");
    } else {
        data->len -= 4;
        data->bytes += 4;
        callback(data, pinger->pings[slot]);
    }
}

struct Pinger* Pinger_new(struct event_base* eventBase, struct Log* logger, struct Allocator* alloc)
{
    return alloc->clone(sizeof(struct Pinger), alloc, &(struct Pinger) {
        .eventBase = eventBase,
        .logger = logger,
        .allocator = alloc
    });
}
