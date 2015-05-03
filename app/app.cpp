#include "multilink.h"
#include <iostream>
#include <memory>
#include "rpc.h"
#include "ioutil.h"
#include "packet_stream_util.h"

class Server {
    Reactor& reactor;
    std::unordered_map<int, Stream*> unused_stream_fds;
    std::unordered_map<int, std::shared_ptr<Multilink::Multilink> > multilinks;

    std::vector<Piper> pipers;

    Stream* take_fd(int num) {
        auto fd = unused_stream_fds[num];
        unused_stream_fds.erase(unused_stream_fds.find(num));
        return fd;
    }

public:
    Server(Reactor& reactor):
        reactor(reactor) {}

    void callback(std::shared_ptr<RPCStream> stream,
                  Json message) {
        std::string type = message["type"].string_value();
        std::cerr << "recv " << message.dump() << std::endl;

        if(type == "provide-stream") {
            int num = message["num"].int_value();
            FD* fd = stream->abandon();
            unused_stream_fds[num] = fd;
            ioutil::write(fd, ByteString::copy_from("ok\n"));
        } else if(type == "multilink") {
            int num = message["num"].int_value();
            int stream_fd = message["stream_fd"].int_value();
            multilinks[num] = std::make_shared<Multilink::Multilink>(reactor);
            auto target = LengthPacketStream::create(reactor, take_fd(stream_fd));
            pipe(reactor, multilinks[num], target);
            pipe(reactor, target, multilinks[num]);
        } else {
            std::cerr << "bad type " << type << std::endl;
        }
    }
};

int main() {
    Reactor reactor;

    Server server {reactor};
    auto callback = [&](std::shared_ptr<RPCStream> stream,
                        Json message) {
        server.callback(stream, message);
    };
    auto rpcserver = RPCServer::create(reactor, "app.sock",
                                       callback);

    reactor.run();
}