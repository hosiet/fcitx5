/*
 * Copyright (C) 2016~2016 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef _FCITX_UTILS_STANDARDPATH_H_
#define _FCITX_UTILS_STANDARDPATH_H_

/// \addtogroup FcitxUtils
/// \{
/// \file
/// \brief Utility classes to handle XDG file path.
///
/// Example:
/// \code{.cpp}
/// auto files = path.multiOpenAll(StandardPath::Type::PkgData, "inputmethod",
///                                O_RDONLY, filter::Suffix(".conf"));
/// \endcode
/// Open all files under $XDG_CONFIG_{HOME,DIRS}/fcitx5/inputmethod/*.conf.

#include "fcitxutils_export.h"
#include <fcitx-utils/flags.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/stringutils.h>
#include <fcitx-utils/unixfd.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fcitx {

namespace filter {

/// \brief Filter class to chain sub filters together.
template <typename... Types>
class Chainer;

template <>
class Chainer<> {
public:
    bool operator()(const std::string &, const std::string &, bool) {
        return true;
    }
};

template <typename First, typename... Rest>
class Chainer<First, Rest...> : Chainer<Rest...> {
    typedef Chainer<Rest...> super_class;

public:
    Chainer(First first, Rest... rest) : super_class(rest...), filter(first) {}

    bool operator()(const std::string &path, const std::string &dir,
                    bool user) {
        if (!filter(path, dir, user)) {
            return false;
        }
        return super_class::operator()(path, dir, user);
    }

private:
    First filter;
};

/// \brief Filter class that revert the sub filter result.
template <typename T>
struct NotFilter {
    NotFilter(T filter_) : filter(filter_) {}

    bool operator()(const std::string &path, const std::string &dir,
                    bool isUser) {
        return !filter(path, dir, isUser);
    }

private:
    T filter;
};

template <typename T>
NotFilter<T> Not(T t) {
    return {t};
}

/// \brief Filter class that filters based on user file.
struct FCITXUTILS_EXPORT User {
    bool operator()(const std::string &, const std::string &, bool isUser) {
        return isUser;
    }
};

/// \brief Filter class that filters file based on prefix
struct FCITXUTILS_EXPORT Prefix {
    Prefix(const std::string &prefix_) : prefix(prefix_) {}

    bool operator()(const std::string &path, const std::string &, bool) {
        return stringutils::startsWith(path, prefix);
    }

    std::string prefix;
};

/// \brief Filter class that filters file based on suffix
struct FCITXUTILS_EXPORT Suffix {
    Suffix(const std::string &suffix_) : suffix(suffix_) {}

    bool operator()(const std::string &path, const std::string &, bool) {
        return stringutils::endsWith(path, suffix);
    }

    std::string suffix;
};
}

/// \brief File descriptor wrapper that handles file descriptor and rename
/// automatically.
class FCITXUTILS_EXPORT StandardPathTempFile {
public:
    StandardPathTempFile(int fd = -1, const std::string &realFile = {},
                         const std::string &tempPath = {})
        : fd_(UnixFD::own(fd)), path_(realFile), tempPath_(tempPath) {}
    StandardPathTempFile(StandardPathTempFile &&other) = default;
    virtual ~StandardPathTempFile();

    int fd() const { return fd_.fd(); }

    const std::string &path() const { return path_; }
    const std::string &tempPath() const { return tempPath_; }

    int release();
    void close();
    void removeTemp();

private:
    UnixFD fd_;
    std::string path_;
    std::string tempPath_;
};

/// \brief Utility class that wraps around UnixFD. It also contains the actual
/// file name information.
class FCITXUTILS_EXPORT StandardPathFile {
public:
    StandardPathFile(int fd = -1, const std::string &path = {})
        : fd_(UnixFD::own(fd)), path_(path) {}
    StandardPathFile(StandardPathFile &&other) = default;
    virtual ~StandardPathFile();

    int fd() const { return fd_.fd(); }

    const std::string &path() const { return path_; }

    int release();

private:
    UnixFD fd_;
    std::string path_;
};

class StandardPathPrivate;

typedef std::map<std::string, StandardPathFile> StandardPathFileMap;
typedef std::map<std::string, std::vector<StandardPathFile>>
    StandardPathFilesMap;

/// \brief Utility class to open, locate, list files based on XDG standard.
class FCITXUTILS_EXPORT StandardPath {
public:
    /// \brief Enum for location type.
    enum class Type { Config, PkgConfig, Data, Cache, Runtime, Addon, PkgData };

    StandardPath(bool skipFcitxPath = false);
    virtual ~StandardPath();

    /// \brief Return the global instance of StandardPath.
    ///
    /// return a global default so we can share it, C++11 static initialization
    /// is thread-safe
    static const StandardPath &global();

    /// \brief Return fcitx specific path defined at compile time.
    static std::string fcitxPath(const char *path);

    /// \brief Scan the directories of given type.
    ///
    /// Callback returns true to continue the scan.
    void scanDirectories(
        Type type,
        std::function<bool(const std::string &path, bool user)> scanner) const;

    /// \brief Scan files scan file under [directory]/[path]
    /// \param path sub directory name.
    void scanFiles(Type type, const std::string &path,
                   std::function<bool(const std::string &path,
                                      const std::string &dir, bool user)>
                       scanner) const;

    /// \brief Get user writable directory for given type.
    std::string userDirectory(Type type) const;

    /// \brief Get all directories in the order of priority.
    std::vector<std::string> directories(Type type) const;

    /// \brief Check if a file exists.
    std::string locate(Type type, const std::string &path) const;

    /// \brief list all matched files.
    std::vector<std::string> locateAll(Type type,
                                       const std::string &path) const;

    /// \brief Open the first matched and succeeded file.
    ///
    /// This function is preferred over locale if you just want to open the
    /// file. Then you can avoid the race condition.
    /// \see openUser()
    StandardPathFile open(Type type, const std::string &path, int flags) const;

    /// \brief Open the user file.
    StandardPathFile openUser(Type type, const std::string &path,
                              int flags) const;

    /// \brief Open user file, but create file with mktemp.
    StandardPathTempFile openUserTemp(Type type,
                                      const std::string &pathOrig) const;

    /// \brief Save the file safely with write and rename to make sure the
    /// operation is atomic.
    /// \param callback Callback function that accept a file descriptor and
    /// return whether the save if success or not.
    bool safeSave(Type type, const std::string &pathOrig,
                  std::function<bool(int)> callback) const;

    /// \brief Open all files match the first [directory]/[path].
    std::vector<StandardPathFile> openAll(Type type, const std::string &path,
                                          int flags) const;
    /// \brief Open all files match the filter under first [directory]/[path].
    StandardPathFileMap
    multiOpenFilter(Type type, const std::string &path, int flags,
                    std::function<bool(const std::string &path,
                                       const std::string &dir, bool user)>
                        filter) const;

    /// \brief Open all files match the filter under first [directory]/[path].
    ///
    /// You may pass multiple filter to it.
    template <typename... Args>
    StandardPathFileMap multiOpen(Type type, const std::string &path, int flags,
                                  Args... args) const {
        return multiOpenFilter(type, path, flags,
                               filter::Chainer<Args...>(args...));
    }

    /// \brief Open all files match the filter under all [directory]/[path].
    StandardPathFilesMap
    multiOpenAllFilter(Type type, const std::string &path, int flags,
                       std::function<bool(const std::string &path,
                                          const std::string &dir, bool user)>
                           filter) const;

    /// \brief Open all files match the filter under all [directory]/[path].
    ///
    /// You may pass multiple filter to it.
    template <typename... Args>
    StandardPathFilesMap multiOpenAll(Type type, const std::string &path,
                                      int flags, Args... args) const {
        return multiOpenAllFilter(type, path, flags,
                                  filter::Chainer<Args...>(args...));
    }

private:
    std::unique_ptr<StandardPathPrivate> d_ptr;
    FCITX_DECLARE_PRIVATE(StandardPath);
};
}

#endif // _FCITX_UTILS_STANDARDPATH_H_
