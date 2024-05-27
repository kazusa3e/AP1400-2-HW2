#include "client.h"
#include <string>
#include "crypto.h"
#include <random>
#include <limits>

Client::Client(const std::string &id, const Server &server)
    : server_(&server), id_(id) {
    std::string public_key{}, private_key{};
    crypto::generate_key(public_key, private_key);
    public_key_ = public_key;
    private_key_ = private_key;
}

double Client::get_wallet() const {
    return server_->get_wallet(id_);
}

std::string Client::sign(std::string txt) const {
    return crypto::signMessage(private_key_, txt);
}

bool Client::transfer_money(const std::string &receiver, double value) {
    std::string trx = id_ + "-" + receiver + "-" + std::to_string(value);
    return server_->add_pending_trx(
        trx,
        crypto::signMessage(private_key_, trx));
}

std::size_t Client::generate_nonce() const {
    std::random_device rd;
    std::mt19937 gen {rd()};
    std::uniform_int_distribution<std::size_t> dist(0, std::numeric_limits<std::size_t>::max());
    return dist(gen);
}