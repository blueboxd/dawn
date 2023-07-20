// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_TRANSFORM_TRANSFORM_H_
#define SRC_TINT_TRANSFORM_TRANSFORM_H_

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/tint/core/castable.h"

namespace tint::transform {

/// Data is the base class for transforms that accept extra input or emit extra output information.
class Data : public utils::Castable<Data> {
  public:
    /// Constructor
    Data();

    /// Copy constructor
    Data(const Data&);

    /// Destructor
    ~Data() override;

    /// Assignment operator
    /// @returns this Data
    Data& operator=(const Data&);
};

/// DataMap is a map of Data unique pointers keyed by the Data's ClassID.
class DataMap {
  public:
    /// Constructor
    DataMap();

    /// Move constructor
    DataMap(DataMap&&);

    /// Constructor
    /// @param data_unique_ptrs a variadic list of additional data unique_ptrs produced by the
    /// transform
    template <typename... DATA>
    explicit DataMap(DATA... data_unique_ptrs) {
        PutAll(std::forward<DATA>(data_unique_ptrs)...);
    }

    /// Destructor
    ~DataMap();

    /// Move assignment operator
    /// @param rhs the DataMap to move into this DataMap
    /// @return this DataMap
    DataMap& operator=(DataMap&& rhs);

    /// Adds the data into DataMap keyed by the ClassID of type T.
    /// @param data the data to add to the DataMap
    template <typename T>
    void Put(std::unique_ptr<T>&& data) {
        static_assert(std::is_base_of<Data, T>::value, "T does not derive from Data");
        map_[&utils::TypeInfo::Of<T>()] = std::move(data);
    }

    /// Creates the data of type `T` with the provided arguments and adds it into DataMap keyed by
    /// the ClassID of type T.
    /// @param args the arguments forwarded to the initializer for type T
    template <typename T, typename... ARGS>
    void Add(ARGS&&... args) {
        Put(std::make_unique<T>(std::forward<ARGS>(args)...));
    }

    /// @returns a pointer to the Data placed into the DataMap with a call to Put()
    template <typename T>
    T const* Get() const {
        return const_cast<DataMap*>(this)->Get<T>();
    }

    /// @returns a pointer to the Data placed into the DataMap with a call to Put()
    template <typename T>
    T* Get() {
        auto it = map_.find(&utils::TypeInfo::Of<T>());
        if (it == map_.end()) {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    /// Add moves all the data from other into this DataMap
    /// @param other the DataMap to move into this DataMap
    void Add(DataMap&& other) {
        for (auto& it : other.map_) {
            map_.emplace(it.first, std::move(it.second));
        }
        other.map_.clear();
    }

  private:
    template <typename T0>
    void PutAll(T0&& first) {
        Put(std::forward<T0>(first));
    }

    template <typename T0, typename... Tn>
    void PutAll(T0&& first, Tn&&... remainder) {
        Put(std::forward<T0>(first));
        PutAll(std::forward<Tn>(remainder)...);
    }

    std::unordered_map<const utils::TypeInfo*, std::unique_ptr<Data>> map_;
};

/// Interface for transforms.
class Transform : public utils::Castable<Transform> {
  public:
    /// @copydoc tint::transform::Data
    using Data = tint::transform::Data;
    /// @copydoc tint::transform::DataMap
    using DataMap = tint::transform::DataMap;

    /// Constructor
    Transform();
    /// Destructor
    ~Transform() override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_TRANSFORM_H_
