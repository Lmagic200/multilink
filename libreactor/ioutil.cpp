#include "ioutil.h"
#include "misc.h"
#include <cassert>

namespace ioutil {

    Future<ByteString> read(Stream* fd, int size) {
        std::shared_ptr<int> pointer = std::make_shared<int>(0);
        ImmediateCompleter<ByteString> completer {ByteString(size)};
        *pointer = 0;

        auto on_read = ([=]() {
            while(true) {
                Buffer r = fd->read(completer.value().as_buffer().slice(*pointer));
                if(r.size == 0) break;
                *pointer += r.size;
                if(*pointer == size) {
                    auto completerPtr = completer;
                    fd->set_on_read_ready(nothing);
                    completerPtr.result();
                    break;
                }
            }
        });

        fd->set_on_read_ready(on_read);
        on_read();

        return completer.future();
    }

    Future<unit> write(Stream* fd, ByteString data) {
        std::shared_ptr<int> pointer = std::make_shared<int>(0);
        Completer<unit> completer;

        auto on_write = ([=]() {
            while(true) {
                int wrote = fd->write(data.as_buffer().slice(*pointer));
                if(wrote == 0) break;
                *pointer += wrote;
                if(*pointer == data.size()) {
                    auto completerPtr = completer;
                    fd->set_on_write_ready(nothing);
                    completerPtr.result({});
                    break;
                }
            }
        });

        fd->set_on_write_ready(on_write);
        on_write();

        return completer.future();
    }

}