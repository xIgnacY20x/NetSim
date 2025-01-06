#include "nodes.hxx"

void PackageSender::push_package(Package&& package) {
    ElementID id = package.get_id();
    buff_.emplace(id);
}