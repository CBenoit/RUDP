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

#include <string>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "RUDP/Packet.hpp"
#include "RUDP/protocols.hpp"
#include "RUDP/utility.hpp"
#include "RUDP/Peer.hpp"
#include "RUDP/Socket.hpp"

using boost::asio::ip::udp;

using Header = rudp::BasicHeader;

boost::asio::io_service io_service;
rudp::Socket<Header> sock(io_service, 512, { udp::v4(), 2000 });

void receive_handler(const rudp::Packet<Header>& packet,
					 size_t bytes_transferred,
                     const rudp::Peer& peer) {
	std::string message(packet.get_message().begin(), packet.get_message().begin() + bytes_transferred - sizeof(packet.get_header()));
	std::cout << boost::uuids::to_string(peer.uuid) << ": " << message << std::endl;

	Header answer_header;
	answer_header.uuid = sock.self().uuid;
	std::string answer_message;
	bool stop_server = false;
	if (message == "/stop_server") {
		answer_message = "Server is stopping...";
		stop_server = true;
	} else {
		answer_message = boost::uuids::to_string(peer.uuid) + ": " + message;
	}
	size_t packet_size = sizeof(answer_header) + answer_message.size();
	uint8_t buffer[packet_size];
	build_buffer(buffer, answer_header, answer_message);
	sock.async_send_to_all(buffer, packet_size);

	if (stop_server) {
		sock.close();
		io_service.stop();
	}
}

void disconnection_timeout_handler(const rudp::Peer& peer) {
	Header answer_header;
	answer_header.uuid = sock.self().uuid;
	std::string answer_message = boost::uuids::to_string(peer.uuid) + "'s connection has timed out!";
	size_t packet_size = sizeof(answer_header) + answer_message.size();
	uint8_t buffer[packet_size];
	build_buffer(buffer, answer_header, answer_message);

	sock.async_send_to_all(buffer, packet_size);
	std::cout << answer_message << std::endl;
}

void disconnection_handler(const rudp::Peer& peer) {
	Header answer_header;
	answer_header.uuid = sock.self().uuid;
	std::string answer_message = boost::uuids::to_string(peer.uuid) + " disconnected!";
	size_t packet_size = sizeof(answer_header) + answer_message.size();
	uint8_t buffer[packet_size];
	build_buffer(buffer, answer_header, answer_message);

	sock.async_send_to_all(buffer, packet_size);
	std::cout << answer_message << std::endl;
}

void connection_handler(const rudp::Peer& peer) {
	Header answer_header;
	answer_header.uuid = sock.self().uuid;
	std::string answer_message = boost::uuids::to_string(peer.uuid) + " connected!";
	size_t packet_size = sizeof(answer_header) + answer_message.size();
	uint8_t buffer[packet_size];
	build_buffer(buffer, answer_header, answer_message);

	sock.async_send_to_all(buffer, packet_size);
	std::cout << answer_message << std::endl;
}

int main() {
	sock.set_receive_handler(receive_handler);
	sock.set_disconnection_timeout_handler(disconnection_timeout_handler);
	sock.set_disconnection_handler(disconnection_handler);
	sock.set_connection_handler(connection_handler);

	std::cout << "Starting server." << std::endl;
	io_service.run();
	std::cout << "Closing server." << std::endl;

	return EXIT_SUCCESS;
}
