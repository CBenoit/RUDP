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

#ifndef RELIABLEUDP_PROTOCOLS_HPP
#define RELIABLEUDP_PROTOCOLS_HPP

#include <cstdint> // uint8_t, uint16_t, uint32_t

#include <boost/uuid/uuid.hpp>

namespace rudp {

using protocol_type = uint32_t;

struct BasicHeader {
	static protocol_type constexpr PROTOCOL_ID = 57986;

	BasicHeader() : protocol(PROTOCOL_ID) {}

	BasicHeader(boost::uuids::uuid uuid)
		: protocol(PROTOCOL_ID)
		, uuid(uuid) {}

	protocol_type protocol;
	boost::uuids::uuid uuid;
};

struct TimeCriticalHeader {
	static protocol_type constexpr PROTOCOL_ID = 57987;

	TimeCriticalHeader() : protocol(PROTOCOL_ID) {}

	TimeCriticalHeader(boost::uuids::uuid uuid)
		: protocol(PROTOCOL_ID)
		, uuid(uuid) {}

	protocol_type protocol;
	uint16_t sequence;
	uint16_t ack;
	boost::uuids::uuid uuid;
};

struct ReliableOrderHeader {
	static protocol_type constexpr PROTOCOL_ID = 57988;

	ReliableOrderHeader() : protocol(PROTOCOL_ID) {}

	ReliableOrderHeader(boost::uuids::uuid uuid)
		: protocol(PROTOCOL_ID)
		, uuid(uuid) {}

	protocol_type protocol;
	uint16_t sequence;
	uint16_t ack;
	uint32_t ack_bits;
	boost::uuids::uuid uuid;
};

}

#endif //RELIABLEUDP_PROTOCOLS_HPP
