#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <vector>
#include <algorithm>
#include <stdexcept>
#include "nodes.hpp"
#include "storage_types.hpp"

bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& colors_of_nodes);

class Node {
public:
    virtual ~Node() = default;
};

enum class ElementType {
    LINK, STOREHOUSE, RAMP, WORKER 
}

struct ParsedLineData {
    ElementType elem_type;
    std::map<std::string, std::string> params;
}

ParsedLineData parse_line(std::string& line);
Factory load_factory_structure(std::istream& is);;
void save_factory_structure(Factory& factory, std::ostream& os);

template<typename Node>
class NodeCollection {
public:
    using container_t = typename std_container_t<Node>;
    using const_iterator = typename container_t::const_iterator;
    using iterator = typename container_t::iterator;


    iterator begin() { return c_.begin(); }
    const_iterator begin() const { return c_.cbegin(); }
    const_iterator cbegin() const { return c_.cbegin(); }

    iterator end()) { return c_.end(); }
    const_iterator end() const { return c_.cend(); }
    const_iterator end() const { return c_.cend(); }


    void add(Node&& node) { c_.push_back(std::move(node)); }
    void remove_by_id(ElementID id);

    NodeCollection<Node>::const_iterator find_by_id(ElementID id) const;
    NodeCollection<Node>::iterator find_by_id(ElementID id);

private:
    container_t c_;
} 


class Factory {
public:
    void add_storehouse(Storehouse&& storehouse) { storehouses_.add(std::move(storehouse)); }
    void remove_storehouse(ElementID id);
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const { return storehouses_.find_by_id(id); }
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) { return storehouses_.find_by_id(id); }
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const { return storehouses_.cbegin(); }
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const { return storehouses_.cend(); }

    void add_ramp(Ramp&& ramp) { ramps_.add(std::move(ramp)); }
    void remove_ramp(ElementID id) { ramps_.remove_by_id(id); }
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const { return ramps_.find_by_id(id); }
    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) { return ramps_.find_by_id(id); }
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const { return ramps_.cbegin(); }
    NodeCollection<Ramp>::const_iterator ramp_cend() const { return ramps_.cend(); }

    void add_worker(Worker&& worker) { workers_.add(std::move(worker)); }
    void remove_worker(ElementID id);
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const { return workers_.find_by_id(id); }
    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) { return workers_.find_by_id(id); }
    NodeCollection<Worker>::const_iterator worker_cbegin() const { return workers_.cbegin(); }
    NodeCollection<Worker>::const_iterator worker_cend() const { return workers_.cend(); }

    bool is_consistent() const;
    void do_deliveries(Time time);
    void do_work(Time time);
    void do_package_passing();

private:
    NodeCollection<Storehouse> storehouses_;
    NodeCollection<Ramp> ramps_;
    NodeCollection<Worker> workers_;

    template<class Node>
    void remove_receiver(NodeCollection<Node&> collection, ElementID id);
};



#endif //FACTORY_HPP