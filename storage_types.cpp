#include "storage_types.hxx"
#include <stdexcept>

Package PackageQueue::pop() {
    auto get_package = [this]() -> Package {
        if (this->type_of_package_queue_ == PackageQueueType::FIFO) {
            Package package = std::move(this->list_of_packages_.front());
            this->list_of_packages_.pop_front();
            return package; }
        else if (this->type_of_package_queue_ == PackageQueueType::LIFO) {
            Package package = std::move(this->list_of_packages_.back());
            this->list_of_packages_.pop_back();
            return package; }
        else {
            throw std::invalid_argument("Incorrect package's queue type name");
        }
    };

    return get_package();
}