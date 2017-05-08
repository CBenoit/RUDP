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

#ifndef RELIABLEUDP_SOCKET_HPP
#define RELIABLEUDP_SOCKET_HPP

#include <string>
#include <type_traits>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/functional/hash.hpp>

#include "protocols.hpp"
#include "Packet.hpp"
#include "Peer.hpp"

namespace rudp {

const unsigned int DEFAULT_CONNECTION_TIMEOUT = 15;
const unsigned int DEFAULT_KEEP_ALIVE_WAIT = 3;

template <typename Header>
class Socket {

	using receive_handler_type = std::function<void(const rudp::Packet<Header>&, size_t, const rudp::Peer&)>;
	using connection_handler_type = std::function<void(const rudp::Peer&)>;
	using disconnection_handler_type = std::function<void(const rudp::Peer&)>;
	using disconnection_timeout_handler_type = std::function<void(const rudp::Peer&)>;

public:
	Socket(boost::asio::io_service& io_service, const size_t buffer_size,
	       const boost::asio::ip::udp::endpoint& endpoint);

	Socket(boost::asio::io_service& io_service, const size_t buffer_size);

	~Socket() {
		if (m_listening)
			close();
	}

	void set_receive_handler(const receive_handler_type& handler) noexcept {
		m_receive_handler = handler;
	}

	void set_connection_handler(const connection_handler_type& handler) noexcept {
		m_connection_handler = handler;
	}

	void set_disconnection_handler(const disconnection_handler_type& handler) noexcept {
		m_disconnection_handler = handler;
	}

	void set_disconnection_timeout_handler(const disconnection_timeout_handler_type& handler) noexcept {
		m_disconnection_timeout_handler = handler;
	}

	void open(const boost::asio::ip::udp& ip_protocol) { m_socket.open(ip_protocol); }

	void close() noexcept;

	void connect(boost::asio::ip::udp::endpoint& endpoint);

	void bind(boost::asio::ip::udp::endpoint endpoint) noexcept { m_socket.bind(endpoint); }

	void async_send_to(void* buffer, size_t buffer_size, boost::asio::ip::udp::endpoint& endpoint) {
		m_socket.async_send_to(boost::asio::buffer(buffer, buffer_size),
		                       endpoint,
		                       [](boost::system::error_code ec, size_t br) { }
		);
	}

	void async_send_to_all(void* buffer, size_t buffer_size) {
		for (auto& peer : m_peers) {
			async_send_to(buffer, buffer_size, peer.endpoint);
		}
	}

	/*template <typename SizedContainer>
	void async_send_to(const SizedContainer& buffer, boost::asio::ip::udp::endpoint& endpoint) {
		m_socket.async_send_to(boost::asio::buffer(buffer),
		                       endpoint,
		                       [](boost::system::error_code ec, size_t br) { }
		);
	}

	template <typename SizedContainer>
	void async_send_to_all(const SizedContainer& buffer, size_t buffer_size) {
		for (auto& peer : m_peers) {
			async_send_to(buffer, peer.endpoint);
		}
	}*/

	const rudp::Peer& self() { return m_self; }

private:
	void handle_async_receive_from(const boost::system::error_code &error_code, size_t bytes_transferred);

	void handle_keep_alive();

	void start_receive();

	void start_keep_alive();

	unsigned int m_connection_timeout;
	bool m_listening;

	boost::asio::io_service& m_io_service;
	boost::asio::ip::udp::socket m_socket;
	boost::asio::ip::udp::endpoint m_remote_endpoint;
	boost::asio::deadline_timer m_timer;

	rudp::Peer m_self;
	std::vector<uint8_t> m_recv_buf;
	std::vector<rudp::Peer> m_peers;

	receive_handler_type m_receive_handler;
	connection_handler_type m_connection_handler;
	disconnection_handler_type m_disconnection_handler;
	disconnection_timeout_handler_type m_disconnection_timeout_handler;
};

}

#include "impl/Socket.tcc"

#endif //RELIABLEUDP_SOCKET_HPP
