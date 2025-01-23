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

ParsedLineData parse_line(std::string& line) {
    // rozłóż linię na ID określający typ elementu oraz tokeny "klucz=wartość"
    // każdy token rozłóż na parę (klucz, wartość)
    // na podstawie ID oraz par (klucz, wartość) zwróć odpowiedni obiekt typu ParsedLineData
    std::vector<std::string> tokens;
    std::string token;

    std::istringstream token_stream(line);
    char delimiter = ' ';

    //  Schemat wstępnego przetwarzania pojedynczej linii (rodzaju analizy składniowej – sprawdzenia zgodności z gramatyką języka) jest identyczny dla każdego typu elementu sieci (dla każdego typu węzła i dla połączenia)
    //    sprawdź, czy linia zaczyna się od jednego z predefiniowanych znaczników (np. LINK); jeśli nie – rzuć wyjątek
    //    wyodrębnij występujące po znaczniku pary klucz-wartość (w postaci “klucz=wartość”, rozdzielone spacjami)
    //    i zachowaj je w kontenerze umożliwiającym ich wygodną analizę (np. w postaci mapy);
    //    zwróć uwagę, że na tym etapie wszystkie dane występujące pomiędzy znakiem “=” a spacją powinny być potraktowane jako wartość
    //    – zatem wartość skojarzona z kluczem będzie łańcuchem znaków (np. typu std::string)


    while (std::getline(token_stream, token, delimiter))
        tokens.push_back(token);

    ParsedLineData parsed_data;

    std::map<std::string, ElementType> element_types{
            {"LOADING_RAMP", ElementType::RAMP},
            {"WORKER", ElementType::WORKER},
            {"STOREHOUSE", ElementType::STOREHOUSE},
            {"LINK", ElementType::LINK},
    };

    try {
        parsed_data.element_type = element_types.at(tokens[0]);

        std::for_each(std::next(tokens.cbegin()), tokens.cend(), [&](const std::string& parameter_str) {
            auto key_value = character_split(parameter_str, '=');
            parsed_data.parameters[key_value[0]] = key_value[1];
        });
    } catch (std::out_of_range& ex) {
        throw std::exception();
    }

    return parsed_data;
}

PackageQueueType get_package_queue_type(std::string& package_queue_type_str) {
    std::map<std::string, PackageQueueType> str_type_map{
            {"LIFO", PackageQueueType::LIFO},
            {"FIFO", PackageQueueType::FIFO}
    };

    return str_type_map.at(package_queue_type_str);
}

void link(Factory& factory, const std::map<std::string, std::string>& parameters) {
    enum class NodeType {
        RAMP, WORKER, STORE
    };

    std::map<std::string, NodeType> str_node_type{
            {"ramp", NodeType::RAMP},
            {"worker", NodeType::WORKER},
            {"store", NodeType::STORE}
    };

    std::string src_str = parameters.at("src");
    std::string dest_str = parameters.at("dest");

    auto src_param = character_split(src_str, '-');
    NodeType src_node_t = str_node_type.at(src_param[0]);
    ElementID src_node_id = std::stoi(src_param[1]);

    auto dest_param = character_split(dest_str, '-');
    NodeType dest_node_t = str_node_type.at(dest_param[0]);
    ElementID dest_node_id = std::stoi(dest_param[1]);

    IPackageReceiver* package_receiver = nullptr;

    switch(dest_node_t) {
        case NodeType::RAMP:
            break;
        case NodeType::WORKER:
            package_receiver = &*factory.find_worker_by_id(dest_node_id);
            break;
        case NodeType::STORE: {
            package_receiver = &*factory.find_storehouse_by_id(dest_node_id);
            break;
        }
    }

    switch(src_node_t) {
        case NodeType::RAMP: {
            factory.find_ramp_by_id(src_node_id)->receiver_preferences_.add_receiver(package_receiver);
            break;
        }
        case NodeType::WORKER: {
            factory.find_worker_by_id(src_node_id)->receiver_preferences_.add_receiver(package_receiver);
            break;
        }
        case NodeType::STORE:
            break;
    }
}

Factory load_factory_structure(std::istream& is) {
    // utwórz (pusty) obiekt typu Factory
    //
    // dla każdej linii w pliku
    //      jeśli linia pusta lub rozpoczyna się od znaku komentarza - przejdź do kolejnej linii
    //      dokonaj parsowania linii
    //      w zależności od typu elementu - wykorzystaj pary (klucz, wartość) do poprawnego:
    //       * zainicjalizowania obiektu właściwego typu węzła i dodania go do obiektu fabryki, albo
    //       * utworzenia połączenia między zadanymi węzłami sieci

    Factory factory;

    std::string line;

    while (std::getline(is, line)) {
        if (line.empty() || line[0] == ';')
            continue;

        ParsedLineData parsed = parse_line(line);

        switch(parsed.element_type) {
            case ElementType::RAMP: {
                ElementID element_id = std::stoi(parsed.parameters.at("id"));
                TimeOffset delivery_interval = std::stoi(parsed.parameters.at("delivery-interval"));
                Ramp ramp(element_id, delivery_interval);

                factory.add_ramp(std::move(ramp));
                break;
            }
            case ElementType::WORKER: {
                ElementID element_id = std::stoi(parsed.parameters.at("id"));
                TimeOffset processing_time = std::stoi(parsed.parameters.at("processing-time"));
                PackageQueueType package_queue_t = get_package_queue_type(parsed.parameters.at("queue-type"));
                Worker worker(element_id, processing_time, std::make_unique<PackageQueue>(package_queue_t));

                factory.add_worker(std::move(worker));
                break;
            }
            case ElementType::STOREHOUSE: {
                ElementID element_id = std::stoi(parsed.parameters.at("id"));
                Storehouse storehouse(element_id);

                factory.add_storehouse(std::move(storehouse));
                break;
            }
            case ElementType::LINK: {
                link(factory, parsed.parameters);
                break;
            }
        }
    }

    return factory;
}

std::string queue_type_str(PackageQueueType package_queue_type) {
    switch(package_queue_type) {
        case PackageQueueType::FIFO:
            return "FIFO";
        case PackageQueueType::LIFO:
            return "LIFO";
    }
    return {};
}

void link_stream_fill(std::stringstream& link_stream, const PackageSender& package_sender, ElementID package_sender_id, std::string&& package_sender_name) {
    auto prefs = package_sender.receiver_preferences_.get_preferences();

    std::for_each(prefs.cbegin(), prefs.cend(), [&](const std::pair<IPackageReceiver*, double>& key_value) {
        link_stream << "LINK src=" << package_sender_name << "-" << package_sender_id << " ";
        const IPackageReceiver* package_receiver = key_value.first;
        ReceiverType receiver_type = package_receiver->get_receiver_type();

        std::string receiver_type_str = receiver_type == ReceiverType::WORKER ? "worker" : "store";

        link_stream << "dest=" << receiver_type_str << "-" << package_receiver->get_id() << '\n';
        std::cout << link_stream.str();
    });
}

void save_factory_structure(Factory& factory, std::ostream& os) {
    // Elementy w pliku występującą w następującej kolejności:
    // LOADING_RAMP, WORKER, STOREHOUSE, LINK.

    std::stringstream link_stream;

    std::for_each(factory.ramp_cbegin(), factory.ramp_cend(), [&](const Ramp& ramp) {
        ElementID ramp_id = ramp.get_id();
        os << "LOADING_RAMP id=" << ramp_id << ' '
           << "delivery-interval=" << ramp.get_delivery_interval() << '\n';

        link_stream_fill(link_stream, ramp, ramp_id, "ramp");
    });

    std::for_each(factory.worker_cbegin(), factory.worker_cend(), [&](const Worker& worker) {
        PackageQueueType queue_type = worker.get_queue()->get_queue_type();
        ElementID worker_id = worker.get_id();
        os << "WORKER id=" << worker_id << ' '
           << "processing-time=" << worker.get_processing_duration() << ' '
           << "queue-type=" << queue_type_str(queue_type) << '\n';

        link_stream_fill(link_stream, worker, worker_id, "worker");
    });

    std::for_each(factory.storehouse_cbegin(), factory.storehouse_cend(), [&](const Storehouse& storehouse) {
        os << "STOREHOUSE id=" << storehouse.get_id() << '\n';
    });

    os << link_stream.str();

    os.flush();
}
