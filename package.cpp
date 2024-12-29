#include "package.hpp"

std::set<ElementID> Package::active_IDs = {};
std::set<ElementID> Package::available_IDs = {};

Package::Package() {
    if (active_IDs.empty() && available_IDs.empty()) {
        id_ = 1;
    } else if (!available_IDs.empty()) {
        id_ = *available_IDs.begin();
        available_IDs.erase(available_IDs.begin());
    } else if (!active_IDs.empty()) {
        id_ = *active_IDs.rbegin() + 1; 
    }
    active_IDs.insert(id_);
}

Package &Package::operator=(Package &&otherPackage) noexcept {
    if (this == &otherPackage)
        return *this;
    active_IDs.erase(this->id_);
    available_IDs.insert(this->id_);
    this->id_ = otherPackage.id_;
    active_IDs.insert(this->id_);
    return *this;
}

Package::~Package() {
    available_IDs.insert(id_);
    active_IDs.erase(id_);
}