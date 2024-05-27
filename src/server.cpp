#include "server.h"
#include <memory.h>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include "client.h"
#include "crypto.h"

std::string random_digits(std::size_t digits) {
    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_int_distribution<> dist(0, 10);
    std::string ret;
    std::generate_n(std::back_inserter(ret), digits, [&dist, &gen]() {
        return dist(gen) + '0';
    });
    return ret;
}

std::map<std::shared_ptr<Client>, double>::const_iterator Server::find(const std::string &id) const {
    // OPTIM:
    for (auto iter = clients_.cbegin(); iter != clients_.cend(); ++iter) {
        if (iter->first->get_id() == id) return iter;
    }
    return clients_.cend();
}

std::map<std::shared_ptr<Client>, double>::iterator Server::find(const std::string &id) {
    // OPTIM:
    for (auto iter = clients_.begin(); iter != clients_.end(); ++iter) {
        if (iter->first->get_id() == id) return iter;
    }
    return clients_.end();
}

std::shared_ptr<Client> Server::add_client(const std::string &id) {
    std::string _id{id};
    if (auto pos = find(id); pos != clients_.end()) {
        _id += random_digits(4);
    }
    std::shared_ptr<Client> client = std::make_shared<Client>(_id, *this);
    clients_.insert(std::make_pair(client, kinitial_coins));
    return client;
}

std::shared_ptr<Client> Server::get_client(const std::string &id) const {
    if (auto pos = find(id); pos != clients_.end()) {
        return pos->first;
    }
    // throw std::runtime_error{"client with id \"" + id + "\" not found"};
    return nullptr;
}

double Server::get_wallet(const std::string &id) const {
    if (auto pos = find(id); pos != clients_.end()) {
        return pos->second;
    }
    throw std::runtime_error{"client with id \"" + id + "\" not found"};
}

bool Server::add_pending_trx(const std::string &trx, const std::string &signature) const {
    std::string sender, receiver;
    double value;
    if (auto res = parse_trx(trx, sender, receiver, value); !res) {
        return false;
    }
    if (auto pos = find(sender); pos == clients_.end()) {
        return false;
    }
    if (auto pos = find(receiver); pos == clients_.end()) {
        return false;
    }
    auto sender_ptr = get_client(sender);
    if (auto res = crypto::verifySignature(sender_ptr->get_publickey(), trx, signature); !res) {
        return false;
    }
    if (sender_ptr->get_wallet() < value) {
        return false;
    }
    pending_trxs.push_back(trx);
    return true;
}

std::size_t Server::mine() {
    std::ostringstream oss;
    for (auto trx : pending_trxs) {
        oss << trx;
    }
    const std::string mempool{oss.str()};
    auto order = clients_.begin();
    std::size_t nonce;
    while (true) {
        nonce = order->first->generate_nonce();
        std::string hash = crypto::sha256(mempool + std::to_string(nonce));
        if (hash.substr(0, 10).find("000") != std::string::npos) {
            break;
        }
        ++order;
        if (order == clients_.end()) order = clients_.begin();
    }
    std::for_each(pending_trxs.begin(), pending_trxs.end(), [this](const std::string &trx) {
        std::string sender, receiver;
        double value;
        parse_trx(trx, sender, receiver, value);
        find(sender)->second -= value;
        find(receiver)->second += value;
    });
    order->second += 6.25;
    pending_trxs.clear();
    return nonce;
}

bool Server::parse_trx(const std::string &trx, std::string &sender, std::string &receiver, double &value) {
    std::regex pattern{R"(([a-zA-Z0-9_]+)-([a-zA-Z0-9_]+)-([\d\.]+))"};
    std::smatch match;
    if (std::regex_search(trx, match, pattern)) {
        sender = match[1];
        receiver = match[2];
        value = std::stod(match[3]);
        return true;
    }
    throw std::runtime_error{"trx \"" + trx + "\" not a valid transactions"};
}

void show_wallets(const Server &server) {
    std::cout << std::string(20, '*') << std::endl;
    for (const auto &client : server.clients_)
        std::cout << client.first->get_id() << " : " << client.second << std::endl;
    std::cout << std::string(20, '*') << std::endl;
}
