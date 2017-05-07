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

#ifndef RELIABLEUDP_PACKET_HPP
#define RELIABLEUDP_PACKET_HPP

#include <cstdint> // uint8_t, uint16_t, uint32_t
#include <vector>
#include <exception>

namespace rudp {

struct bad_header_exception : std::runtime_error {
	bad_header_exception() : std::runtime_error("Bad header") {}
};

struct bad_protocol_exception : std::runtime_error {
	bad_protocol_exception() : std::runtime_error("Bad protocol") {}
};

template<typename Header>
class Packet {
public:
	Packet(std::vector<uint8_t> packet_buffer) noexcept(false) {
		if (packet_buffer.size() < sizeof(Header)) {
			throw bad_header_exception();
		}

		m_header = *reinterpret_cast<Header*>(packet_buffer.data());
		if (m_header.protocol != Header::PROTOCOL_ID) {
			throw bad_protocol_exception();
		}

		m_message = std::vector<uint8_t>(packet_buffer.begin() + sizeof(m_header), packet_buffer.end());
	}

	const Header& get_header() const noexcept {
		return m_header;
	}

	const std::vector<uint8_t>& get_message() const noexcept {
		return m_message;
	}

private:
	Header m_header;
	std::vector<uint8_t> m_message;
};

}

#endif //RELIABLEUDP_PACKET_HPP
