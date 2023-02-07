//
// Created by Paul Walker on 2/5/23.
//

#ifndef SCXT_SRC_MESSAGING_CLIENT_DETAIL_CLIENT_JSON_DETAILS_H
#define SCXT_SRC_MESSAGING_CLIENT_DETAIL_CLIENT_JSON_DETAILS_H

#include "json/scxt_traits.h"

namespace scxt::messaging::client::detail
{
template <typename T> struct client_message_traits : public scxt::json::scxt_traits<T>
{
};
using client_message_value = tao::json::basic_value<client_message_traits>;
} // namespace scxt::messaging::client::detail
#endif // SHORTCIRCUIT_CLIENT_JSON_DETAILS_H
