#include "nodes.hpp"
#include "factory.hpp"

bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& colors_of_nodes) {
    if (colors_of_nodes[sender] == NodeColor::VERIFIED) { return true; }
    colors_of_nodes[sender] = NodeColor::VISITED;

    const auto& pref = sender->receiver_preferences_.get_preferences();
    if (pref.empty()) {
        throw std::logic_error("Brak zdefiniowanych odbiorców");
    }

    for (const auto& [receiver, probability] : pref) {
        switch (receiver->get_receiver_type()) {
            case ReceiverType::STOREHOUSE:
                return true;

            case ReceiverType::WORKER: {
                auto* worker_ptr = dynamic_cast<Worker*>(receiver);
                auto* sendrecv_ptr = dynamic_cast<PackageSender*>(worker_ptr);

                if (!sendrecv_ptr || sender == sendrecv_ptr) {
                    continue;
                }

                if (colors_of_nodes[sendrecv_ptr] == NodeColor::UNVISITED && has_reachable_storehouse(sendrecv_ptr, colors_of_nodes)) {
                    return true;
                }

                break;
            }
        }
    }

    colors_of_nodes[sender] = NodeColor::VERIFIED;
    throw std::logic_error("Magazyn nieosiągalny");
}


template<class Node>
NodeCollection<Node>::remove_by_id(ElementID id){
    iter = find_by_id(id);
    if (iter != c_.cend()){
        c_.erase(iter);
    }
}


template<class Node>
NodeCollection<Node>::const_iterator find_by_id(ElementID id) const {
    for(auto iter = c_.cbegin(); iter != c_.cend(); ++iter){
        if (iter->get_id() == id)
            return iter;
    }

    return c_.cend();
}


template<class Node>
NodeCollection<Node>::iterator find_by_id(ElementID id) const {
    for(auto iter = c_.begin(); iter != c_.end(); ++iter){
        if (iter->get_id() == id)
            return iter;
    }

    return c_.end();
}


void Factory::remove_storehouse(ElementID id) {
    Storehouse* storehouse = storehouses_.find_by_id(id) != storehouses_.cend() ? &(*storehouses_.find_by_id(id)) : nullptr;
    
    for(auto& el : ramps_) {
        el.receiver_preferences_.remove_receiver(storehouse);
    }

    for(auto& el : workers_) {
        el.receiver_preferences_.remove_receiver(storehouse);
    }

    storehouses_.remove_by_id(id);
}


void Factory::remove_worker(ElementID id) {
    Worker* worker = workers_.find_by_id(id) != workers_.cend() ? &(*workers_.find_by_id(id)) : nullptr;
    
    for(auto& el : ramps_) {
        el.receiver_preferences_.remove_receiver(worker);
    }

    for(auto& el : workers_) {
        el.receiver_preferences_.remove_receiver(worker);
    }

    workers_.remove_by_id(id);
}


bool Factory::is_consistent() const {
    std::map<PackageSender*, NodeColor> colour;

    set_unvisited_colors(colour, workers_);
    set_unvisited_colors(colour, ramps_);

    try {
        for (auto& el : ramps_) {
            has_reachable_storehouse(dynamic_cast<PackageSender*>(&el), colour);
        }
    } catch (std::logic_error&) {
        return false;
    }

    return true;
}


void Factory::do_deliveries(Time time) {
    for(auto& el : ramps_){
        el.deliver_goods(time);
    }
}


void Factory::do_work(Time time) {
    for (auto& el : workers_){
        el.do_work(time);
    }
}


void Factory::do_package_passing() {
    for(auto& el : workers_) {
        el.send_package();
    }
    for(auto& el : ramps_) {
        el.send_package();
    }
}


void set_unvisited_colors(std::map<PackageSender*, NodeColor>& colour, auto& container) {
    for (const auto& el : container) {
        PackageSender* sender = dynamic_cast<PackageSender*>(&el);
        colour[sender] = NodeColor::UNVISITED;
    }
}


template<class Node>
void Factory::remove_receiver(NodeCollection<Node>& collection, ElementID id) {

    auto iterator = collection.find_by_id(id);
    auto rec_ptr = dynamic_cast<IPackageReceiver*>(iterator);

    auto remove_receiver_from_prefs = [&](auto& container) {
        for (auto& element : container) {
            auto& prefs = element.receiver_preferences_.get_preferences();
            for (auto& pref : prefs) {
                if (rec_ptr == pref.first) {
                    element.receiver_preferences_.remove_receiver(rec_ptr);
                    break;
                }
            }
        }
    };

    remove_receiver_from_prefs(ramps_);
    remove_receiver_from_prefs(workers_);
}


std::vector<std::string> character_split(const std::string& splittable_str, char delimiter) {
    std::vector<std::string> characters_splitted;
    std::string element;
    
    for (char c : splittable_str) {
        if (c == delimiter) {
            if (!element.empty()) {
                characters_splitted.emplace_back(element);
                element.clear();
            }
        } 
        else {
            element += c;
        }
    }

    if (!element.empty()) {
        characters_splitted.emplace_back(element);
    }

    return result;
}

