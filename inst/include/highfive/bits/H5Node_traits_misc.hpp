/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <string>
#include <vector>

#include <H5Apublic.h>
#include <H5Fpublic.h>
#include <H5Ppublic.h>
#include <H5Tpublic.h>

#include "../H5DataSet.hpp"
#include "../H5Group.hpp"
#include "../H5Selection.hpp"
#include "../H5Utility.hpp"
#include "H5DataSet_misc.hpp"
#include "H5Iterables_misc.hpp"
#include "H5Selection_misc.hpp"
#include "H5Slice_traits_misc.hpp"

#include "h5l_wrapper.hpp"
#include "h5g_wrapper.hpp"
#include "h5o_wrapper.hpp"


namespace HighFive {


template <typename Derivate>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const DataSpace& space,
                                                   const DataType& dtype,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    return DataSet(detail::h5d_create2(static_cast<Derivate*>(this)->getId(),
                                       dataset_name.c_str(),
                                       dtype.getId(),
                                       space.getId(),
                                       lcpl.getId(),
                                       createProps.getId(),
                                       accessProps.getId()));
}

template <typename Derivate>
template <typename T>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const DataSpace& space,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    return createDataSet(
        dataset_name, space, create_and_check_datatype<T>(), createProps, accessProps, parents);
}

template <typename Derivate>
template <typename T>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const T& data,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    DataSet ds =
        createDataSet(dataset_name,
                      DataSpace::From(data),
                      create_and_check_datatype<typename details::inspector<T>::base_type>(),
                      createProps,
                      accessProps,
                      parents);
    ds.write(data);
    return ds;
}

template <typename Derivate>
inline DataSet NodeTraits<Derivate>::getDataSet(const std::string& dataset_name,
                                                const DataSetAccessProps& accessProps) const {
    return DataSet(detail::h5d_open2(static_cast<const Derivate*>(this)->getId(),
                                     dataset_name.c_str(),
                                     accessProps.getId()));
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string& group_name, bool parents) {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    return detail::make_group(detail::h5g_create2(static_cast<Derivate*>(this)->getId(),
                                                  group_name.c_str(),
                                                  lcpl.getId(),
                                                  H5P_DEFAULT,
                                                  H5P_DEFAULT));
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string& group_name,
                                               const GroupCreateProps& createProps,
                                               bool parents) {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    return detail::make_group(detail::h5g_create2(static_cast<Derivate*>(this)->getId(),
                                                  group_name.c_str(),
                                                  lcpl.getId(),
                                                  createProps.getId(),
                                                  H5P_DEFAULT));
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::getGroup(const std::string& group_name) const {
    return detail::make_group(detail::h5g_open2(static_cast<const Derivate*>(this)->getId(),
                                                group_name.c_str(),
                                                H5P_DEFAULT));
}

template <typename Derivate>
inline DataType NodeTraits<Derivate>::getDataType(const std::string& type_name,
                                                  const DataTypeAccessProps& accessProps) const {
    return DataType(detail::h5t_open2(static_cast<const Derivate*>(this)->getId(),
                                      type_name.c_str(),
                                      accessProps.getId()));
}

template <typename Derivate>
inline size_t NodeTraits<Derivate>::getNumberObjects() const {
    hsize_t res;
    detail::h5g_get_num_objs(static_cast<const Derivate*>(this)->getId(), &res);
    return static_cast<size_t>(res);
}

template <typename Derivate>
inline std::string NodeTraits<Derivate>::getObjectName(size_t index) const {
    return details::get_name([&](char* buffer, size_t length) {
        return detail::h5l_get_name_by_idx(static_cast<const Derivate*>(this)->getId(),
                                           ".",
                                           H5_INDEX_NAME,
                                           H5_ITER_INC,
                                           index,
                                           buffer,
                                           length,
                                           H5P_DEFAULT);
    });
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::rename(const std::string& src_path,
                                         const std::string& dst_path,
                                         bool parents) const {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    herr_t err = detail::h5l_move(static_cast<const Derivate*>(this)->getId(),
                                  src_path.c_str(),
                                  static_cast<const Derivate*>(this)->getId(),
                                  dst_path.c_str(),
                                  lcpl.getId(),
                                  H5P_DEFAULT);

    return err >= 0;
}

template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listObjectNames(IndexType idx_type) const {
    std::vector<std::string> names;
    details::HighFiveIterateData iterateData(names);

    size_t num_objs = getNumberObjects();
    names.reserve(num_objs);

    detail::h5l_iterate(static_cast<const Derivate*>(this)->getId(),
                        static_cast<H5_index_t>(idx_type),
                        H5_ITER_INC,
                        NULL,
                        &details::internal_high_five_iterate<H5L_info_t>,
                        static_cast<void*>(&iterateData));
    return names;
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::_exist(const std::string& node_path, bool raise_errors) const {
    SilenceHDF5 silencer{};
    const auto val = detail::nothrow::h5l_exists(static_cast<const Derivate*>(this)->getId(),
                                                 node_path.c_str(),
                                                 H5P_DEFAULT);
    if (val < 0) {
        if (raise_errors) {
            HDF5ErrMapper::ToException<GroupException>("Invalid link for exist()");
        } else {
            return false;
        }
    }

    // The root path always exists, but H5Lexists return 0 or 1
    // depending of the version of HDF5, so always return true for it
    // We had to call H5Lexists anyway to check that there are no errors
    return (node_path == "/") ? true : (val > 0);
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::exist(const std::string& node_path) const {
    // When there are slashes, first check everything is fine
    // so that subsequent errors are only due to missing intermediate groups
    if (node_path.find('/') != std::string::npos) {
        _exist("/");  // Shall not throw under normal circumstances
        // Unless "/" (already checked), verify path exists (not throwing errors)
        return (node_path == "/") ? true : _exist(node_path, false);
    }
    return _exist(node_path);
}


template <typename Derivate>
inline void NodeTraits<Derivate>::unlink(const std::string& node_path) const {
    detail::h5l_delete(static_cast<const Derivate*>(this)->getId(), node_path.c_str(), H5P_DEFAULT);
}


// convert internal link types to enum class.
// This function is internal, so H5L_TYPE_ERROR shall be handled in the calling context
static inline LinkType _convert_link_type(const H5L_type_t& ltype) noexcept {
    switch (ltype) {
    case H5L_TYPE_HARD:
        return LinkType::Hard;
    case H5L_TYPE_SOFT:
        return LinkType::Soft;
    case H5L_TYPE_EXTERNAL:
        return LinkType::External;
    default:
        // Other link types are possible but are considered strange to HighFive.
        // see https://support.hdfgroup.org/HDF5/doc/RM/H5L/H5Lregister.htm
        return LinkType::Other;
    }
}

template <typename Derivate>
inline LinkType NodeTraits<Derivate>::getLinkType(const std::string& node_path) const {
    H5L_info_t linkinfo;
    detail::h5l_get_info(static_cast<const Derivate*>(this)->getId(),
                         node_path.c_str(),
                         &linkinfo,
                         H5P_DEFAULT);

    if (linkinfo.type == H5L_TYPE_ERROR) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Link type of \"") + node_path +
                                                   "\" is H5L_TYPE_ERROR");
    }
    return _convert_link_type(linkinfo.type);
}

template <typename Derivate>
inline ObjectType NodeTraits<Derivate>::getObjectType(const std::string& node_path) const {
    const auto id = detail::h5o_open(static_cast<const Derivate*>(this)->getId(),
                                     node_path.c_str(),
                                     H5P_DEFAULT);
    auto object_type = _convert_object_type(detail::h5i_get_type(id));
    detail::h5o_close(id);
    return object_type;
}


template <typename Derivate>
inline void NodeTraits<Derivate>::createSoftLink(const std::string& link_name,
                                                 const std::string& obj_path,
                                                 LinkCreateProps linkCreateProps,
                                                 const LinkAccessProps& linkAccessProps,
                                                 const bool parents) {
    if (parents) {
        linkCreateProps.add(CreateIntermediateGroup{});
    }
    detail::h5l_create_soft(obj_path.c_str(),
                            static_cast<const Derivate*>(this)->getId(),
                            link_name.c_str(),
                            linkCreateProps.getId(),
                            linkAccessProps.getId());
}


template <typename Derivate>
inline void NodeTraits<Derivate>::createExternalLink(const std::string& link_name,
                                                     const std::string& h5_file,
                                                     const std::string& obj_path,
                                                     LinkCreateProps linkCreateProps,
                                                     const LinkAccessProps& linkAccessProps,
                                                     const bool parents) {
    if (parents) {
        linkCreateProps.add(CreateIntermediateGroup{});
    }
    detail::h5l_create_external(h5_file.c_str(),
                                obj_path.c_str(),
                                static_cast<const Derivate*>(this)->getId(),
                                link_name.c_str(),
                                linkCreateProps.getId(),
                                linkAccessProps.getId());
}

template <typename Derivate>
template <typename T, typename>
inline void NodeTraits<Derivate>::createHardLink(const std::string& link_name,
                                                 const T& target_obj,
                                                 LinkCreateProps linkCreateProps,
                                                 const LinkAccessProps& linkAccessProps,
                                                 const bool parents) {
    static_assert(!std::is_same<T, Attribute>::value,
                  "hdf5 doesn't support hard links to Attributes");
    if (parents) {
        linkCreateProps.add(CreateIntermediateGroup{});
    }
    detail::h5l_create_hard(target_obj.getId(),
                            ".",
                            static_cast<const Derivate*>(this)->getId(),
                            link_name.c_str(),
                            linkCreateProps.getId(),
                            linkAccessProps.getId());
}


}  // namespace HighFive
