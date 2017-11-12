#include <config.h>

#include <primitiv/error.h>
#include <primitiv/model.h>
#include <primitiv/string_utils.h>

namespace primitiv {

void Model::add_parameter(const std::string &name, Parameter &param) {
  if (name_set_.find(name) != name_set_.end()) {
    THROW_ERROR(
        "Name '" << name << "' already exists in the model.");
  }
  if (param_set_.find(&param) != param_set_.end()) {
    THROW_ERROR(
        "Parameter '" << &param << "' already exists in the model.");
  }
  name_set_.emplace(name);
  param_set_.emplace(&param);
  param_kv_.emplace(name, &param);
}

void Model::add_submodel(const std::string &name, Model &model) {
  if (&model == this) {
    THROW_ERROR("Can't add self as a submodel.");
  }
  if (model.has_submodel(*this)) {
    THROW_ERROR("Can't add an ancestor model as a submodel.");
  }
  if (name_set_.find(name) != name_set_.end()) {
    THROW_ERROR(
        "Name '" << name << "' already exists in the model.");
  }
  if (submodel_set_.find(&model) != submodel_set_.end()) {
    THROW_ERROR(
        "Model '" << &model << "' already exists in the model.");
  }
  name_set_.emplace(name);
  submodel_set_.emplace(&model);
  submodel_kv_.emplace(name, &model);
}

const Model &Model::get_semiterminal(
    const std::vector<std::string> &names) const {
  const Model *cur = this;
  for (auto it = names.begin(), end = names.end() - 1; it != end; ++it) {
    const auto next = cur->submodel_kv_.find(*it);
    if (next == cur->submodel_kv_.end()) {
      THROW_ERROR(
          "Parameter or submodel not found: "
          "'" << string_utils::join(names, ".") << "'");
    }
    cur = next->second;
  }
  return *cur;
}

const Parameter &Model::get_parameter(
    const std::vector<std::string> &names) const {
  const Model &st = get_semiterminal(names);
  const auto it = st.param_kv_.find(names.back());
  if (it == st.param_kv_.end()) {
    THROW_ERROR(
        "Parameter not found: '" << string_utils::join(names, ".") << "'");
  }
  return *it->second;
}

const Model &Model::get_submodel(const std::vector<std::string> &names) const {
  const Model &st = get_semiterminal(names);
  const auto it = st.submodel_kv_.find(names.back());
  if (it == st.submodel_kv_.end()) {
    THROW_ERROR(
        "Submodel not found: '" << string_utils::join(names, ".") << "'");
  }
  return *it->second;
}

std::map<std::vector<std::string>, Parameter *> Model::get_all_parameters() const {
  std::map<std::vector<std::string>, Parameter *> params;
  for (const auto &kv : param_kv_) {
    params.emplace(std::vector<std::string> { kv.first }, kv.second);
  }
  for (const auto &sm_kv : submodel_kv_) {
    for (const auto &p_kv : sm_kv.second->get_all_parameters()) {
      std::vector<std::string> key { sm_kv.first };
      key.insert(key.end(), p_kv.first.begin(), p_kv.first.end());
      params.emplace(key, p_kv.second);
    }
  }
  return params;
}

bool Model::has_submodel(const Model &model) const {
  for (const Model *sm : submodel_set_) {
    if (sm == &model) return true;
    if (sm->has_submodel(model)) return true;
  }
  return false;
}

}  // namespace primitiv
