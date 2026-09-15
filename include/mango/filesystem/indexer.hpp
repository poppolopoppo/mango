/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2026 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <string>
#include <map>
#include <mango/filesystem/path.hpp>

namespace mango::filesystem
{

    template <typename Header>
    class Indexer
    {
    public:
        struct Folder
        {
            std::map<std::string, Header *> headers;
        };

    protected:
        std::map<std::string, Folder> folders;
        std::map<std::string, Header> headers;

    public:
        void insert(const std::string& foldername, const std::string& filename, const Header& header)
        {
            Header* ptr = &headers[filename];
            *ptr = header;
            folders[foldername].headers[filename] = ptr;
        }

        const Folder* getFolder(const std::string& pathname) const
        {
            const Folder* result = nullptr;

            auto i = folders.find(pathname);
            if (i != folders.end())
            {
                result = &i->second;
            }

            return result;
        }

        const Header* getHeader(const std::string& filename) const
        {
            const Header* result = nullptr;

            auto i = headers.find(filename);
            if (i != headers.end())
            {
                result = &i->second;
            }

            return result;
        }
    };

    // Insert a file path and intermediate folders into the indexer.
    // mark_folder is called before each parent folder entry is stored (ZIP-style tree).
    template <typename Header, typename MarkFolder>
    void indexFilePath(Indexer<Header>& indexer, std::string filepath, Header header, MarkFolder mark_folder)
    {
        while (!filepath.empty())
        {
            std::string folder = getPath(filepath.substr(0, filepath.length() - 1));

            header.filename = filepath.substr(folder.length());
            indexer.insert(folder, filepath, header);
            mark_folder(header);

            filepath = folder;
        }
    }

} // namespace mango::filesystem
