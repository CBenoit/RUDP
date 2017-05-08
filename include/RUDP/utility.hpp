///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
// Copyright 2017 Benoît CORTIER                                                                 //
//                                                                                               //
// This file is part of RUDP project which is released under the                                 //
// European Union Public License v1.1. If a copy of the EUPL was                                 //
// not distributed with this software, you can obtain one at :                                   //
// https://joinup.ec.europa.eu/community/eupl/og_page/european-union-public-licence-eupl-v11     //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RELIABLEUDP_UTILITY_HPP
#define RELIABLEUDP_UTILITY_HPP

#ifndef ENABLE_RUDP_LOGGING
#define ENABLE_RUDP_LOGGING 1
#endif

#include <string>
#include <cstdint> // uint8_t, uint16_t, uint32_t
#include <limits> // std::numeric_limits

#if ENABLE_RUDP_LOGGING
	#define BOOST_LOG_DYN_LINK 1
	#include <boost/log/trivial.hpp>

	#define RUDP_LOG(level, message) BOOST_LOG_TRIVIAL(level) << message
#else
	#define RUDP_LOG(level, message)
#endif

namespace rudp {

template <typename T>
bool sequence_more_recent(T s1, T s2) {
	return (s1 > s2) && (s1 - s2 <= std::numeric_limits<T>::max() / 2)
	       || (s2 > s1) && (s2 - s1 > std::numeric_limits<T>::max() / 2);
}

template <typename Header>
void build_buffer(uint8_t* buffer, Header header, std::string message) {
	std::copy((uint8_t *) &header, (uint8_t *) &header + sizeof(header), buffer);
	std::copy(message.data(), message.data() + message.size(), buffer + sizeof(header));
}

}

#endif //RELIABLEUDP_UTILITY_HPP
