#ifndef NODES_HPP
#define NODES_HPP

#include "helpers.hxx"
#include "package.hxx"
#include "storage_types.hxx"
#include "types.hxx"
#include <map>
#include <memory>
#include <utility>
#include <optional>


class IPackageReceiver {
public:
    virtual IPackageStockpile::const_iterator begin() const = 0;
    virtual IPackageStockpile::const_iterator cbegin() const = 0;
    virtual IPackageStockpile::const_iterator end() const = 0;
    virtual IPackageStockpile::const_iterator cend() const = 0;

    virtual void receive_package(Package&& p) = 0;
    virtual ElementID get_id() const = 0;

    virtual ~IPackageReceiver() = default;
};


class ReceiverPreferences {
public:
    using preferences_t = std::map<IPackageReceiver*, double>;
    using const_iterator = preferences_t::const_iterator;

    const_iterator begin() const { return prefs_.begin(); }
    const_iterator cbegin() const { return prefs_.cbegin(); }
    const_iterator end() const { return prefs_.end(); }
    const_iterator cend() const { return prefs_.cend(); }

    ReceiverPreferences(ProbabilityGenerator pg = probability_generator) { probability_generated_ = std::move(pg); }

    void add_receiver(IPackageReceiver* r);
    void remove_receiver(IPackageReceiver* r);

    IPackageReceiver* choose_receiver();

    const preferences_t& get_preferences() const { return this->prefs_; }

private:
    preferences_t prefs_;
    ProbabilityGenerator probability_generated_;
};


class PackageSender {
public:
    ReceiverPreferences receiver_preferences_;

    PackageSender() = default;
    PackageSender(PackageSender&& moved_element) = default;

    void send_package();
    const std::optional<Package>& get_sending_buffer() const { return buff_; }

protected:
    std::optional<Package> buff_ = std::nullopt;

    void push_package(Package&& package);
};


class Storehouse : public IPackageReceiver {
public:
    Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> s = std::make_unique<PackageQueue>(PackageQueueType::LIFO)) { id_ = id; s_ = std::move(s); }

    IPackageStockpile::const_iterator begin() const { return s_->begin(); }
    IPackageStockpile::const_iterator cbegin() const { return s_->cbegin(); }
    IPackageStockpile::const_iterator end() const { return s_->end(); }
    IPackageStockpile::const_iterator cend() const { return s_->cend(); }

    void receive_package(Package&& p) { s_->push(std::move(p)); }
    ElementID get_id() const { return id_; };

private:
    ElementID id_;
    std::unique_ptr<IPackageStockpile> s_;
};


class Worker : public PackageSender, public IPackageReceiver {
public:
    Worker(ElementID id, TimeOffset processing_duration, std::unique_ptr<IPackageQueue> queue) { PackageSender(); id_ = id; processing_duration_ = processing_duration; queue_ = std::move(queue); }

    void do_work(Time t);

    TimeOffset get_processing_duration() { return processing_duration_; }
    Time get_package_processing_start_time() { return t_; }

    IPackageStockpile::const_iterator begin() const { return queue_->begin(); }
    IPackageStockpile::const_iterator cbegin() const { return queue_->cbegin(); }
    IPackageStockpile::const_iterator end() const { return queue_->end(); }
    IPackageStockpile::const_iterator cend() const { return queue_->cend(); }

    void receive_package(Package&& p) { queue_->push(std::move(p)); };
    ElementID get_id() const { return id_; };

private:
    ElementID id_;
    TimeOffset processing_duration_;
    std::unique_ptr<IPackageQueue> queue_;
    Time t_;

protected:
    std::optional<Package> buff_ = std::nullopt;
};


class Ramp : public PackageSender {
public:
    Ramp(ElementID id, TimeOffset delivery_interval) { PackageSender(); id_ = id; delivery_interval_ = delivery_interval; }

    void deliver_goods(Time t);

    TimeOffset get_delivery_interval() const { return delivery_interval_; }
    ElementID get_id() const { return id_; }

private:
    ElementID id_;
    TimeOffset delivery_interval_;
    Time t_;

protected:
    std::optional<Package> buff_ = std::nullopt;
};

#endif //NODES_HPP