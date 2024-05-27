#ifndef SERVER_H
#define SERVER_H

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "client.h"

class Client;

class Server {
public:
    // Server();
    std::shared_ptr<Client> add_client(const std::string &id);
    std::shared_ptr<Client> get_client(const std::string &id) const;
    double get_wallet(const std::string &id) const;
    bool add_pending_trx(const std::string &trx, const std::string &signature) const;
    std::size_t mine();

    static bool parse_trx(const std::string &trx, std::string &sender, std::string &receiver, double &value);
    friend void show_wallets(const Server &server);

private:
    const std::size_t kinitial_coins = 5;

    std::map<std::shared_ptr<Client>, double> clients_;
    std::map<std::shared_ptr<Client>, double>::const_iterator find(const std::string &id) const;
    std::map<std::shared_ptr<Client>, double>::iterator find(const std::string &id);
};

extern std::vector<std::string> pending_trxs;

#endif  // SERVER_H