#ifndef STORAGE_TYPES_HXX
#define STORAGE_TYPES_HXX


#include "package.hxx"
#include <list>
#include <iostream>

enum class PackageQueueType {
    FIFO,
    LIFO
};

class IPackageStockpile {
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual void push(Package&& package) = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator begin() const = 0;
    virtual const_iterator cend() const = 0;
    virtual const_iterator end() const = 0;
    virtual bool empty() const = 0;
    virtual size_t size() const = 0;

    virtual ~IPackageStockpile() = default;
};


class IPackageQueue : public IPackageStockpile {
public:
    virtual Package pop() = 0;
    virtual PackageQueueType get_queue_type() const = 0;

    ~IPackageQueue() = default;
};


class PackageQueue : public IPackageQueue {
public:
    PackageQueue() = delete;
    PackageQueue(PackageQueueType type_of_package) : list_of_packages_(), type_of_package_queue_(type_of_package) {}

    void push(Package&& package) { this->list_of_packages_.emplace_back(std::move(package)); }
    const_iterator cbegin() const { return this->list_of_packages_.cbegin(); }
    const_iterator begin() const { return this->list_of_packages_.begin(); }
    const_iterator cend() const { return this->list_of_packages_.cend(); }
    const_iterator end() const { return this->list_of_packages_.end(); }
    bool empty() const { return this->list_of_packages_.empty(); }
    size_t size() const { return this->list_of_packages_.size(); }

    Package pop();
    PackageQueueType get_queue_type() const { return this->type_of_package_queue_; }

    ~PackageQueue() = default;

private:
    std::list<Package> list_of_packages_;
    PackageQueueType type_of_package_queue_;
};

#endif //STORAGE_TYPES_HXX