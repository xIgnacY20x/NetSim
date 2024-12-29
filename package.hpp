#include "types.hpp"
#include <set>

class Package {
public:
    Package();
    Package(ElementID id) { active_IDs.insert(id); id_ = id; }
    Package(Package&& package) { id_ = package.id_; }

    ElementID get_id() const { return id_; }

    Package& operator=(Package &&otherPackage) noexcept;
    ~Package();

private:
    static std::set<ElementID> active_IDs;
    static std::set<ElementID> available_IDs;
    ElementID id_;
};
