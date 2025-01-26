#include "nodes.hxx"
#include "factory.hxx"

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
                if (!worker_ptr) continue;
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


template<typename Node>
typename NodeCollection<Node>::const_iterator NodeCollection<Node>::find_by_id(ElementID id) const {
    for(auto iter = c_.cbegin(); iter != c_.cend(); ++iter){
        if (iter->get_id() == id)
            return iter;
    }

    return c_.cend();
}


template<typename Node>
typename NodeCollection<Node>::iterator NodeCollection<Node>::find_by_id(ElementID id){
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
    std::map<const PackageSender*, NodeColor> colour;

    auto set_unvisited_colours = [&colour](const auto& cont) {
        for (const auto& item : cont) {
            if (const PackageSender* sender = dynamic_cast<const PackageSender*>(&item)) {
                colour[sender] = NodeColor::UNVISITED;
            }
        }
    };

    set_unvisited_colours(ramps_);
    set_unvisited_colours(workers_);

    try {
        for (auto& el : ramps_) {
            has_reachable_storehouse(dynamic_cast<const PackageSender*>(&el), colour);
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


void Factory::do_package_passing() {
    for(auto& el : workers_) {
        el.send_package();
    }
    for(auto& el : ramps_) {
        el.send_package();
    }
}


void Factory::do_work(Time time) {
    for (auto& el : workers_){
        el.do_work(time);
    }
}



template<typename Node>
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

    remove_receiver_from_prefs(workers_);
    remove_receiver_from_prefs(ramps_);
}


std::vector<std::string> char_split(const std::string& splittable_str, char delimiter) {
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

    return characters_splitted;
}


ParsedLineData parse_line(std::string& line) {
    std::vector<std::string> parts;
    std::string current_part;

    std::istringstream input_stream(line);
    char split_char = ' ';

    while (std::getline(input_stream, current_part, split_char))
        parts.push_back(current_part);

    const std::map<std::string, ElementType> element_type_map = {
            {"LINK", ElementType::LINK},
            {"LOADING_RAMP", ElementType::RAMP},
            {"WORKER", ElementType::WORKER},
            {"STOREHOUSE", ElementType::STOREHOUSE},
    };

    if (parts.empty()) {
        throw std::invalid_argument("Wejście jest puste");
    }

    ParsedLineData parsed_result;

    try {
        parsed_result.elem_type = element_type_map.at(parts.front());

        for (size_t i = 1; i < parts.size(); ++i) {
            const auto key_value_pair = char_split(parts[i], '=');
            
            if (key_value_pair.size() != 2) {
                throw std::logic_error("Nieprawidłowy format klucz=wartość: " + parts[i]);
            }
            parsed_result.params[key_value_pair[0]] = key_value_pair[1];
        }
    } catch (const std::out_of_range&) {
        throw std::logic_error("Niewłaściwy typ danych wejścia");
    }

    return parsed_result;
}

PackageQueueType get_queue_type_of_package(std::string& string_package_queue) {
    std::map<std::string, PackageQueueType> str_type{
            {"FIFO", PackageQueueType::FIFO},
            {"LIFO", PackageQueueType::LIFO},
    };

    return str_type.at(string_package_queue);
}


void link(Factory& factory, const std::map<std::string, std::string>& parameters) {
    enum class NodeType {
        RAMP, WORKER, STORE
    };

    const std::map<std::string, NodeType> node_type_map{
            {"ramp", NodeType::RAMP},
            {"worker", NodeType::WORKER},
            {"store", NodeType::STORE}
    };

    std::string src_str = parameters.at("src");
    auto src_parts = char_split(src_str, '-');
    NodeType src_node_type = node_type_map.at(src_parts[0]);
    ElementID src_node_id = std::stoi(src_parts[1]);

    std::string dest_str = parameters.at("dest");
    auto dest_parts = char_split(dest_str, '-');
    NodeType dest_node_type = node_type_map.at(dest_parts[0]);
    ElementID dest_node_id = std::stoi(dest_parts[1]);

    IPackageReceiver* receiver = nullptr;
    if (dest_node_type == NodeType::WORKER) {
        receiver = &*factory.find_worker_by_id(dest_node_id);
    } else if (dest_node_type == NodeType::STORE) {
        receiver = &*factory.find_storehouse_by_id(dest_node_id);
    }

    if (src_node_type == NodeType::RAMP) {
        factory.find_ramp_by_id(src_node_id)->receiver_preferences_.add_receiver(receiver);
    } else if (src_node_type == NodeType::WORKER) {
        factory.find_worker_by_id(src_node_id)->receiver_preferences_.add_receiver(receiver);
    }
}


std::string queue_type(PackageQueueType package_queue_type) {
    if (package_queue_type == PackageQueueType::FIFO) {
        return "FIFO";
    } else if (package_queue_type == PackageQueueType::LIFO) {
        return "LIFO";
    }
    return {};
}


void link_fill(std::stringstream& output_stream, const PackageSender& sender, ElementID sender_id, std::string&& sender_name) {
    const auto& preferences = sender.receiver_preferences_.get_preferences();

    for (const auto& [receiver, preference] : preferences) {
        output_stream << "LINK src=" << sender_name << "-" << sender_id << " ";

        ReceiverType receiver_type = receiver->get_receiver_type();
        std::string receiver_type_str = (receiver_type == ReceiverType::WORKER) ? "worker" : "store";

        output_stream << "dest=" << receiver_type_str << "-" << receiver->get_id() << '\n';
        std::cout << output_stream.str();
    }
}


Factory load_factory_structure(std::istream& input_stream) {
    Factory factory;

    std::string current_line;
    std::vector<std::string> relevant_lines;

    for (std::string line_buffer; std::getline(input_stream, line_buffer);) {
        if (!line_buffer.empty() && line_buffer[0] != ';') {
            relevant_lines.push_back(line_buffer);
        }
    }

    for (auto& single_line : relevant_lines) {
        ParsedLineData parsed_data = parse_line(single_line);

        if (parsed_data.elem_type == ElementType::LINK) {
            link(factory, parsed_data.params);

        } else if (parsed_data.elem_type == ElementType::WORKER) {
            ElementID worker_id = std::stoi(parsed_data.params.at("id"));
            TimeOffset worker_processing_time = std::stoi(parsed_data.params.at("processing-time"));
            PackageQueueType worker_queue_type = get_queue_type_of_package(parsed_data.params.at("queue-type"));
            Worker new_worker(worker_id, worker_processing_time, std::make_unique<PackageQueue>(worker_queue_type));
            factory.add_worker(std::move(new_worker));

        } else if (parsed_data.elem_type == ElementType::STOREHOUSE) {
            ElementID storehouse_id = std::stoi(parsed_data.params.at("id"));
            Storehouse new_storehouse(storehouse_id);
            factory.add_storehouse(std::move(new_storehouse));

        } else if (parsed_data.elem_type == ElementType::RAMP) {
            ElementID ramp_id = std::stoi(parsed_data.params.at("id"));
            TimeOffset ramp_delivery_interval = std::stoi(parsed_data.params.at("delivery-interval"));
            Ramp new_ramp(ramp_id, ramp_delivery_interval);
            factory.add_ramp(std::move(new_ramp));
        }
    }

    return factory;
}


void save_factory_structure(Factory& factory, std::ostream& output_stream) {
    std::stringstream link_stream;

    for (auto it = factory.ramp_cbegin(); it != factory.ramp_cend(); ++it) {
        const auto& ramp = *it;
        ElementID ramp_id = ramp.get_id();
        output_stream << "LOADING_RAMP id=" << ramp_id << ' '
                      << "delivery-interval=" << ramp.get_delivery_interval() << '\n';

        link_fill(link_stream, ramp, ramp_id, "ramp");
    }

    for (auto it = factory.worker_cbegin(); it != factory.worker_cend(); ++it) {
        const auto& worker = *it;
        PackageQueueType queue_t = worker.get_queue()->get_queue_type();
        ElementID worker_id = worker.get_id();
        output_stream << "WORKER id=" << worker_id << ' '
                << "processing-time=" << worker.get_processing_duration() << ' '
                      << "queue-type=" << queue_type(queue_t) << '\n';

        link_fill(link_stream, worker, worker_id, "worker");
    }

    for (auto it = factory.storehouse_cbegin(); it != factory.storehouse_cend(); ++it) {
        const auto& storehouse = *it;
        output_stream << "STOREHOUSE id=" << storehouse.get_id() << '\n';
    }
    
    output_stream << link_stream.str();
    output_stream.flush();
}