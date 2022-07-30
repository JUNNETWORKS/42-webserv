#include "config/virtual_server_conf.hpp"

#include <iostream>
#include <sstream>

#include "utils/log.hpp"

namespace config {

VirtualServerConf::VirtualServerConf()
    : listen_ip_(), listen_port_(), server_names_(), locations_() {}

VirtualServerConf::VirtualServerConf(const VirtualServerConf &rhs) {
  *this = rhs;
}

VirtualServerConf &VirtualServerConf::operator=(const VirtualServerConf &rhs) {
  if (this != &rhs) {
    listen_ip_ = rhs.listen_ip_;
    listen_port_ = rhs.listen_port_;
    server_names_ = rhs.server_names_;
    locations_ = rhs.locations_;
  }
  return *this;
}

VirtualServerConf::~VirtualServerConf() {}

bool VirtualServerConf::IsValid() const {
  // port番号がセットされている
  // 1つ以上のlocation
  if (listen_port_.empty() || locations_.empty()) {
    return false;
  }
  // locationがすべてvalid
  for (LocationConfsVector::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    if (!(it->IsValid())) {
      return false;
    }
  }
  return true;
}

void VirtualServerConf::Print() const {
  std::stringstream ss;

  ss << "server {\n";

  ss << "\tlisten: " << listen_ip_ << ":" << listen_port_ << ";"
     << "\n";

  ss << "\tserver_name:";
  for (std::set<std::string>::const_iterator it = server_names_.begin();
       it != server_names_.end(); ++it) {
    ss << " " << *it;
  }
  ss << ";\n";

  for (LocationConfsVector::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    it->Print();
  }

  ss << "}\n";

  utils::PrintLog(ss.str());
}

std::string VirtualServerConf::GetListenIp() const {
  return listen_ip_;
}

PortType VirtualServerConf::GetListenPort() const {
  return listen_port_;
}

void VirtualServerConf::SetListenIp(std::string listen_ip) {
  listen_ip_ = listen_ip;
}

void VirtualServerConf::SetListenPort(PortType listen_port) {
  listen_port_ = listen_port;
}

bool VirtualServerConf::IsServerNameIncluded(std::string server_name) const {
  return server_names_.find(server_name) != server_names_.end();
}

void VirtualServerConf::AppendServerName(std::string server_name) {
  server_names_.insert(server_name);
}

const LocationConf *VirtualServerConf::GetLocation(std::string path) const {
  const LocationConf *location_conf = NULL;
  // 最大文字数マッチが採用される｡
  std::string::size_type max_len = 0;

  for (LocationConfsVector::const_iterator it = locations_.begin();
       it != locations_.end(); ++it) {
    if (it->GetPathPattern().size() > max_len && it->IsMatchPattern(path)) {
      location_conf = &(*it);
      max_len = it->GetPathPattern().size();
    }
  }
  return location_conf;
}

void VirtualServerConf::AppendLocation(LocationConf location) {
  locations_.push_back(location);
}

}  // namespace config
