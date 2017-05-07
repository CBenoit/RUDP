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

#include <algorithm>
#include <chrono>

#include <boost/uuid/uuid_io.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "RUDP/Socket.hpp"

namespace {
	template <typename ContainerT, typename PredicateT>
	void erase_if(ContainerT& container, const PredicateT& predicate) {
		for (auto it = container.cbegin(); it != container.cend(); ) {
			if (predicate(*it)) {
				it = container.erase(it);
			} else {
				++it;
			}
		}
	}

	template <typename Header>
	void is_valid_specialization() {
		static_assert(std::is_same<decltype(Header::PROTOCOL_ID), decltype(rudp::BasicHeader::PROTOCOL_ID)>::value,
		              "'PROTOCOL_ID' static field should be of the same type as 'rudp::BasicHeader::PROTOCOL_ID'.");
		static_assert(std::is_same<decltype(Header::protocol), decltype(rudp::BasicHeader::protocol)>::value,
		              "'protocol' field should be of the same type as 'rudp::BasicHeader::protocol'.");
		static_assert(std::is_same<decltype(Header::uuid), decltype(rudp::BasicHeader::uuid)>::value,
		              "'uuid' field should be of the same type as 'rudp::BasicHeader::uuid'.");

		return; // Header type should have a field 'uuid', 'protocol' of the same type as 'rudp::BasicHeader::uuid' and 'rudp::BasicHeader::protocol' and a static field 'PROTOCOL_ID' of the same type as 'rudp::BasicHeader::PROTOCOL_ID'.
	}

	using lib_message_type = uint32_t;

	const lib_message_type KEEP_ALIVE_MESSAGE = 424967295;
	const lib_message_type CONNECTION_MESSAGE = 424967296;
	const lib_message_type DISCONNECTION_MESSAGE = 424967297;

	template <typename Header>
	struct LibMessage {
		Header header;
		lib_message_type message;
	};

	template <typename Header>
	inline LibMessage<Header> build_lib_message(boost::uuids::uuid self_uuid, const lib_message_type type) {
		LibMessage<Header> lib_message;
		lib_message.header = Header(self_uuid);
		lib_message.message = type;
		return lib_message;
	}
}

template <typename Header>
rudp::Socket<Header>::Socket(boost::asio::io_service& io_service, size_t buffer_size,
                             const boost::asio::ip::udp::endpoint& endpoint)
			: m_connection_timeout(DEFAULT_CONNECTION_TIMEOUT)
			, m_listening(true)
			, m_io_service(io_service)
			, m_socket(io_service, endpoint)
			, m_timer(io_service)
			, m_self(endpoint)
			, m_recv_buf(buffer_size)
{
	is_valid_specialization<Header>();
	start_keep_alive();
	start_receive();
}

template <typename Header>
rudp::Socket<Header>::Socket(boost::asio::io_service& io_service, size_t buffer_size)
			: m_connection_timeout(DEFAULT_CONNECTION_TIMEOUT)
            , m_listening(false)
			, m_io_service(io_service)
			, m_socket(io_service)
			, m_timer(io_service)
			, m_self(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::loopback(), 0))
			, m_recv_buf(buffer_size)
{ is_valid_specialization<Header>(); }

template <typename Header>
void rudp::Socket<Header>::close() noexcept {
	LibMessage<Header> lib_message = build_lib_message<Header>(m_self.uuid, DISCONNECTION_MESSAGE);
	async_send_to_all(reinterpret_cast<void*>(&lib_message), sizeof(lib_message));

	m_listening = false;
}

template <typename Header>
void rudp::Socket<Header>::connect(boost::asio::ip::udp::endpoint& remote_endpoint) {
	m_socket.open(remote_endpoint.protocol());

	LibMessage<Header> lib_message = build_lib_message<Header>(m_self.uuid, CONNECTION_MESSAGE);
	async_send_to(reinterpret_cast<void*>(&lib_message), sizeof(lib_message), remote_endpoint);

	m_listening = true;

	start_keep_alive();
	start_receive();
}

template <typename Header>
void rudp::Socket<Header>::handle_async_receive_from(const boost::system::error_code &error_code, size_t bytes_transferred) {
	if (!error_code) {
		try {
			rudp::Packet<Header> packet(m_recv_buf);

			std::vector<Peer>::iterator peer_it = std::find_if(m_peers.begin(), m_peers.end(), [this, &packet](rudp::Peer& peer) -> bool {
				if (peer.uuid == packet.get_header().uuid) {
					peer.last_packet_timestamp =
						std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
					return true;
				}
				return false;
			});

			if (peer_it == m_peers.end()) { // peer doesn't exist
				//BOOST_LOG_TRIVIAL(trace) << "New peer: " << boost::uuids::to_string(packet.get_header().uuid);

				m_peers.emplace_back(m_remote_endpoint,
				                    packet.get_header().uuid,
				                    std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()));
				peer_it = m_peers.end() - 1;

				if (m_connection_handler) {
					m_connection_handler(*peer_it);

					LibMessage<Header> lib_message = build_lib_message<Header>(m_self.uuid, CONNECTION_MESSAGE);
					async_send_to(reinterpret_cast<void*>(&lib_message), sizeof(lib_message), peer_it->endpoint);
				}
			}

			bool is_a_user_message = true;
			if (bytes_transferred - sizeof(Header) == sizeof(lib_message_type)) {
				const lib_message_type lib_message = *reinterpret_cast<const lib_message_type*>(packet.get_message().data());
				if (lib_message == CONNECTION_MESSAGE || lib_message == KEEP_ALIVE_MESSAGE) {
					is_a_user_message = false;
				} else if (lib_message == DISCONNECTION_MESSAGE) {
					is_a_user_message = false;
					if (m_disconnection_handler) {
						m_disconnection_handler(*peer_it);
					}

					m_peers.erase(std::remove_if(m_peers.begin(), m_peers.end(), [this, peer_it](const auto& other_peer) -> bool {
						if (peer_it->uuid == other_peer.uuid)
							return true;
						return false;
					}), m_peers.end());
				}
			}

			if (is_a_user_message && m_receive_handler) {
				m_receive_handler(packet, bytes_transferred, *peer_it);
			}
		} catch (std::exception& e) {
			//BOOST_LOG_TRIVIAL(info) << "Received packet with a bad protocol.";
			m_socket.send_to(boost::asio::buffer("Bad protocol"), m_remote_endpoint);
			// FIXME: possible network loop...
		}

		if (m_listening) {
			start_receive();
		}
	}
}

template <typename Header>
void rudp::Socket<Header>::handle_keep_alive() {
	//BOOST_LOG_TRIVIAL(trace) << "keep alive";

	// check for timeouts
	std::chrono::seconds now =
		std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
	m_peers.erase(std::remove_if(m_peers.begin(), m_peers.end(), [this, &now](const auto& peer) -> bool {
		if (peer.last_packet_timestamp + std::chrono::seconds(m_connection_timeout) < now) {
			if (m_disconnection_timeout_handler) {
				m_disconnection_timeout_handler(peer);
			}
			return true;
		}
		return false;
	}), m_peers.end());

	if (m_listening) {
		// Keep alive.
		LibMessage<Header> lib_message = build_lib_message<Header>(m_self.uuid, KEEP_ALIVE_MESSAGE);
		async_send_to_all(reinterpret_cast<void*>(&lib_message), sizeof(lib_message));

		start_keep_alive();
	}
}

template <typename T>
void rudp::Socket<T>::start_receive() {
	m_socket.async_receive_from(
		boost::asio::buffer(m_recv_buf),
		m_remote_endpoint,
		[this](auto ec, auto bytes_transfered) {
			this->handle_async_receive_from(ec, bytes_transfered);
		}
	);
}

template <typename T>
void rudp::Socket<T>::start_keep_alive() {
	m_timer.expires_from_now(boost::posix_time::seconds(DEFAULT_KEEP_ALIVE_WAIT));
	m_timer.async_wait([this](auto ec) {
		this->handle_keep_alive();
	});
}
