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

#ifndef RELIABLEUDP_PEER_HPP
#define RELIABLEUDP_PEER_HPP

#include <chrono>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/asio/ip/udp.hpp>

namespace rudp {

struct Peer {
	Peer() {}

	Peer(boost::asio::ip::udp::endpoint endpoint)
		: uuid(boost::uuids::random_generator{}())
		, endpoint(endpoint)
	{}

	Peer(boost::asio::ip::udp::endpoint endpoint, boost::uuids::uuid uuid)
		: uuid(uuid)
		, endpoint(endpoint)
	{}

	Peer(boost::asio::ip::udp::endpoint endpoint, boost::uuids::uuid uuid, std::chrono::seconds last_packet_timestamp)
		: uuid(uuid)
		, endpoint(endpoint)
		, last_packet_timestamp(last_packet_timestamp)
	{}

	boost::uuids::uuid uuid;
	boost::asio::ip::udp::endpoint endpoint;
	std::chrono::seconds last_packet_timestamp;
};

}

#endif //RELIABLEUDP_PEER_HPP
