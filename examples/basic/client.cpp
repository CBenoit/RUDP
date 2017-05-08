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

#define ENABLE_RUDP_LOGGING 0

#include <string>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <RUDP/Packet.hpp>
#include <RUDP/protocols.hpp>
#include <RUDP/utility.hpp>
#include <RUDP/Peer.hpp>
#include <RUDP/Socket.hpp>

using boost::asio::ip::udp;

using Header = rudp::BasicHeader;

boost::asio::io_service io_service;
rudp::Socket<Header> sock(io_service, 512);

void receive_handler(const rudp::Packet<Header>& packet,
                     size_t bytes_transferred,
                     const rudp::Peer /*peer*/) {
	std::string message(packet.get_message().begin(), packet.get_message().begin() + bytes_transferred - sizeof(packet.get_header()));
	std::cout << message << std::endl;
}

void disconnection_timeout_handler(const rudp::Peer& peer) {
	std::cout << "Server connection has timed out!" << std::endl;
}

void disconnection_handler(const rudp::Peer& /*peer*/) {
	std::cout << "Disconnected from server!" << std::endl;
}

void connection_handler(const rudp::Peer& /*peer*/) {
	std::cout << "Connected to server!" << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: client <host>" << std::endl;
		return EXIT_FAILURE;
	}

	sock.set_receive_handler(receive_handler);
	sock.set_disconnection_timeout_handler(disconnection_timeout_handler);
	sock.set_disconnection_handler(disconnection_handler);
	sock.set_connection_handler(connection_handler);

	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), argv[1], "2000");
	udp::endpoint remote_endpoint = *resolver.resolve(query);
	sock.connect(remote_endpoint);

	std::thread thread([]() { io_service.run(); });

	for (;;) {
		std::string message;
		std::getline(std::cin, message, '\n');

		if (message == "/quit") {
			break;
		}

		Header header;
		header.uuid = sock.self().uuid;
		size_t packet_size = sizeof(header) + message.size();
		uint8_t buffer[packet_size];
		build_buffer(buffer, header, message);

		sock.async_send_to(buffer, packet_size, remote_endpoint);
	}

	sock.close();
	io_service.stop();
	thread.join();

	return EXIT_SUCCESS;
}
