/*
 * Copyright (C) 2016~2016 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the  file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#include "fcitx-utils/dbus/bus.h"
#include "fcitx-utils/event.h"
#include "fcitx-utils/log.h"
#include "fcitx-utils/metastring.h"

using namespace fcitx::dbus;
using namespace fcitx;

int main() {
    Bus bus(BusType::Session);

    static_assert(std::is_same<DBusSignatureToType<'i', 'u'>::type,
                               std::tuple<int32_t, uint32_t>>::value,
                  "Type is not same");
    static_assert(std::is_same<DBusSignatureToType<'i'>::type, int32_t>::value,
                  "Type is not same");
    static_assert(std::is_same<DBusSignatureToType<'a', 'u'>::type,
                               std::vector<uint32_t>>::value,
                  "Type is not same");
    static_assert(
        std::is_same<DBusSignatureToType<'a', '(', 'i', 'u', ')'>::type,
                     std::vector<DBusStruct<int32_t, uint32_t>>>::value,
        "Type is not same");
    static_assert(
        std::is_same<
            DBusSignatureToType<'a', 'i', 'a', '(', 'i', 'u', ')'>::type,
            std::tuple<std::vector<int>,
                       std::vector<DBusStruct<int32_t, uint32_t>>>>::value,
        "Type is not same");
    static_assert(
        std::is_same<
            DBusSignatureToType<'a', 'i', '(', 'i', 'u', ')'>::type,
            std::tuple<std::vector<int>, DBusStruct<int32_t, uint32_t>>>::value,
        "Type is not same");

    // interface name must has dot
    {
        auto msg = bus.createSignal("/test", "test.a.b.c", "test");
        msg << 1;
        FCITX_ASSERT(msg.signature() == "i");
    }
    {
        auto msg = bus.createSignal("/test", "test.a.b.c", "test");
        msg << DBusSignatureToType<'i', 'u'>::type(1, 2);
        FCITX_ASSERT(msg.signature() == "iu");
    }
    {
        auto msg = bus.createSignal("/test", "test.a.b.c", "test");
        msg << FCITX_STRING_TO_DBUS_TUPLE("siud")("a", 1, 2, 3);
        FCITX_ASSERT(msg.signature() == "siud");
    }
    {
        auto msg = bus.createSignal("/test", "test.a.b.c", "test");
        msg << FCITX_STRING_TO_DBUS_TUPLE("as")(
            std::vector<std::string>{"a", "b"});
        FCITX_ASSERT(msg.signature() == "as");
    }
    return 0;
}
