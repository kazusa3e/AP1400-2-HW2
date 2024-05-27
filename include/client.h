#ifndef CLIENT_H
#define CLIENT_H

#include <cstddef>
#include <string>
#include "server.h"

class Server;

class Client {
public:
    Client(const std::string &id, const Server &server);
    std::string get_id() const {
        return id_;
    }
    std::string get_publickey() const {
        return public_key_;
    }
    double get_wallet() const;
    std::string sign(std::string txt) const;
    bool transfer_money(const std::string &receiver, double value);
    std::size_t generate_nonce() const;

private:
    Server const *const server_;
    const std::string id_;
    std::string public_key_;
    std::string private_key_;
};

#endif  // CLIENT_H