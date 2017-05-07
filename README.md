# RUDP

RUDP is a header only library aiming at providing various asynchronous UDP based sockets such as reliable UDP socket.

Currently it only has a "connection" UDP socket (an UDP socket that handles connections).

RUDP uses some C++14 features and boost libraries (mainly asio and uuid).

[More explanations to come]

## Basic usage

Minimal server code:
```C++
using boost::asio::ip::udp;

boost::asio::io_service io_service;
rudp::Socket<rudp::BasicHeader> socket(io_service, 512, { udp::v4(), 2000 });

io_service.run();
```

Minimal client code:
```C++
using boost::asio::ip::udp;

boost::asio::io_service io_service;
rudp::Socket<rudp::BasicHeader> socket(io_service, 512);

udp::resolver resolver(io_service);
udp::resolver::query query(udp::v4(), "localhost", "2000");
udp::endpoint remote_endpoint = *resolver.resolve(query);
socket.connect(remote_endpoint);

io_service.run();
```

To perform any useful task handlers shall be set.

`rudp::BasicHeader` is the simplest header of RUDP. A `rudp::Socket<rudp::BasicHeader>` only provides a "connection" UDP socket.

For more complete examples, check the [example folder](examples).

## License

This work is under the [European Union Public License v1.1](LICENSE.md).

You may get a copy of this license in your language from the European Commission [here](https://joinup.ec.europa.eu/community/eupl/og_page/european-union-public-licence-eupl-v11).

Extract of article 13 :

    All linguistic versions of this Licence, approved by the European Commission, have identical value.
    Parties can take advantage of the linguistic version of their choice.

## Planned

- Add makefiles
- Add doxygen documentation.
- Add time critical protocol.
- Add reliable order protocol.
