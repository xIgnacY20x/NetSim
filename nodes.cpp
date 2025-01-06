#include "nodes.hxx"

void PackageSender::push_package(Package&& package) {
    ElementID package_id = package.get_id();
    buff_ = package_id;
}


void PackageSender::send_package() {
    if (buff_) {
        IPackageReceiver* receiver = receiver_preferences_.choose_receiver();
        if (receiver) {
            receiver->receive_package(std::move(*buff_));
            buff_.reset();
        }
    }
}

void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
    size_t num_of_receivers = prefs_.size();
    double new_probability = 1.0 / (num_of_receivers + 1);

    for (auto& rec : prefs_) {
        rec.second = new_probability;
    }
    prefs_[r] = new_probability;
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
    size_t num_of_receivers = prefs_.size();
    prefs_.erase(r);

    if (num_of_receivers > 1) {
        double new_probability = 1.0 / (num_of_receivers - 1);
        for (auto& rec : prefs_) {
            rec.second = new_probability;
        }
    }
}

IPackageReceiver* ReceiverPreferences::choose_receiver() {
    double prob = probability_generated_();
    double cumulative_probability = 0.0;
    for (const auto& rec : prefs_) {
        cumulative_probability += rec.second;
        if (prob <= cumulative_probability) {
            return rec.first;
        }
    }
    return nullptr;
}

void Worker::do_work(Time t) {
    if (!buff_ && !queue_->empty()) {
        buff_ = queue_->pop();
        t_ = t;
    }
    else if (buff_ && (t - t_ + 1 == processing_duration_)) {
        push_package(Package(buff_->get_id()));
        buff_.reset();

        if (!queue_->empty()) {
            buff_ = queue_->pop();
        }
    }
}

void Ramp::deliver_goods(Time t) {
    if (!buff_) {
        buff_ = Package(id_);
        push_package(Package());
        t_ = t;
    }
    else if (t - t_ == delivery_interval_) {
        push_package(Package());
    }
}
