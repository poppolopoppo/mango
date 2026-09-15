/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2026 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;
using namespace mango::filesystem;

namespace
{

    // -----------------------------------------------------------------
    // In-memory test container ".tbox"
    //
    // [magic: "TBOX"][u32 count][entries...][file bytes...]
    //
    // entry: u32 offset, u32 size, u8 name_length, name[name_length]
    // -----------------------------------------------------------------

    static constexpr u32 TBOX_MAGIC = 0x584f4254; // "TBOX" little-endian

    struct TboxEntry
    {
        u64 offset = 0;
        u64 size = 0;
        std::string filename;
        bool is_folder = false;

        bool isFolder() const
        {
            return is_folder;
        }
    };

    class MapperTBOX : public AbstractMapper
    {
    public:
        ConstMemory m_parent;
        Indexer<TboxEntry> m_folders;

        MapperTBOX(ConstMemory parent, const std::string& password)
            : m_parent(parent)
        {
            MANGO_UNREFERENCED(password);

            if (!parent.address || parent.size < 8)
                return;

            LittleEndianConstPointer p = parent.address;
            if (p.read32() != TBOX_MAGIC)
                return;

            const u32 count = p.read32();
            std::vector<TboxEntry> entries;
            entries.reserve(count);

            for (u32 i = 0; i < count; ++i)
            {
                if (p + 9 > parent.end())
                    return;

                TboxEntry entry;
                entry.offset = p.read32();
                entry.size = p.read32();
                const u8 name_length = p.read8();

                if (p + name_length > parent.end())
                    return;

                entry.filename.assign(p.cast<const char>(), name_length);
                p += name_length;

                entries.push_back(entry);
            }

            for (const TboxEntry& entry : entries)
            {
                TboxEntry header = entry;
                indexFilePath(m_folders, entry.filename, header, [](TboxEntry& h)
                {
                    h.is_folder = true;
                    h.size = 0;
                });
            }
        }

        u64 getSize(const std::string& filename) const override
        {
            const TboxEntry* entry = m_folders.getHeader(filename);
            return entry ? entry->size : 0;
        }

        bool isFile(const std::string& filename) const override
        {
            const TboxEntry* entry = m_folders.getHeader(filename);
            return entry && !entry->isFolder();
        }

        void getIndex(FileIndex& index, const std::string& pathname) override
        {
            const Indexer<TboxEntry>::Folder* folder = m_folders.getFolder(pathname);
            if (!folder)
                return;

            for (auto i : folder->headers)
            {
                const TboxEntry& entry = *i.second;

                u32 flags = 0;
                u64 size = entry.size;

                if (entry.isFolder())
                {
                    flags |= FileInfo::Directory;
                    size = 0;
                }

                index.emplace(entry.filename, size, flags);
            }
        }

        std::unique_ptr<VirtualMemory> map(const std::string& filename) override
        {
            const TboxEntry* entry = m_folders.getHeader(filename);
            if (!entry || entry->isFolder())
            {
                MANGO_EXCEPTION("[mapper.tbox] File \"{}\" not found.", filename);
            }

            if (entry->offset + entry->size > m_parent.size)
            {
                MANGO_EXCEPTION("[mapper.tbox] File \"{}\" is out of bounds.", filename);
            }

            ConstMemory slice = m_parent.slice(entry->offset, entry->size);
            return std::make_unique<VirtualMemoryView>(slice);
        }
    };

    AbstractMapper* createMapperTBOX(ConstMemory parent, const std::string& password)
    {
        return new MapperTBOX(parent, password);
    }

    void buildTestArchive(Buffer& buffer)
    {
        struct Source
        {
            const char* path;
            const char* data;
            size_t size;
        };

        const Source sources[] =
        {
            { "readme.txt", "hello", 5 },
            { "maps/start.bsp", "nested", 6 },
        };

        struct EntryDesc
        {
            u32 offset;
            u32 size;
            std::string name;
        };

        std::vector<EntryDesc> directory;
        directory.reserve(std::size(sources));

        size_t directory_bytes = 8;
        for (const Source& source : sources)
        {
            directory_bytes += 4 + 4 + 1 + std::strlen(source.path);
        }

        u32 offset = u32(directory_bytes);
        for (const Source& source : sources)
        {
            EntryDesc desc;
            desc.offset = offset;
            desc.size = u32(source.size);
            desc.name = source.path;
            directory.push_back(desc);
            offset += desc.size;
        }

        buffer.reserve(offset);

        LittleEndianPointer p = buffer.append(8);
        p.write32(TBOX_MAGIC);
        p.write32(u32(directory.size()));

        for (const EntryDesc& desc : directory)
        {
            p = buffer.append(4 + 4 + 1 + desc.name.length());
            p.write32(desc.offset);
            p.write32(desc.size);
            p.write8(u8(desc.name.length()));
            for (char c : desc.name)
                p.write8(u8(c));
        }

        for (const Source& source : sources)
        {
            buffer.append(source.data, source.size);
        }
    }

    int g_failed = 0;

    void check(bool condition, const char* message)
    {
        if (!condition)
        {
            printLine(Print::Error, "FAILED: {}", message);
            ++g_failed;
        }
    }

} // namespace

int main()
{
    registerMapper(createMapperTBOX, ".tbox");

    check(isMapperRegistered(".tbox"), "custom mapper is registered");
    check(Mapper::isCustomMapper("archive.tbox"), "isCustomMapper recognizes .tbox");

    Buffer archive;
    buildTestArchive(archive);
    Mapper mapper(archive, ".tbox", "");

    check(mapper.isFile("readme.txt"), "readme.txt is a file");
    check(mapper.isFile("maps/start.bsp"), "maps/start.bsp is a file");
    check(!mapper.isFile("maps/"), "maps/ is not a file");
    check(mapper.getSize("readme.txt") == 5, "readme.txt size");
    check(mapper.getSize("maps/start.bsp") == 6, "maps/start.bsp size");

    const FileIndex& index = mapper.index();
    check(index.size() >= 2, "index lists files");

    bool found_readme = false;
    bool found_maps_dir = false;

    for (const FileInfo& info : index)
    {
        if (info.name == "readme.txt" && info.isFile())
            found_readme = true;
        if (info.name == "maps/" && info.isDirectory())
            found_maps_dir = true;
    }

    check(found_readme, "index contains readme.txt");
    check(found_maps_dir, "index contains maps/");

    FileIndex maps_index;
    mapper.getIndex(maps_index, "maps/");

    bool found_bsp = false;
    for (const FileInfo& info : maps_index)
    {
        if (info.name == "start.bsp" && info.isFile())
            found_bsp = true;
    }

    check(found_bsp, "index contains maps/start.bsp");

    {
        std::unique_ptr<VirtualMemory> memory = mapper.map("readme.txt");
        ConstMemory view = memory ? ConstMemory(*memory) : ConstMemory();
        check(memory && view.size == 5, "map readme.txt");
        if (memory)
        {
            check(std::memcmp(view.address, "hello", 5) == 0, "readme.txt contents");
        }
    }

    {
        std::unique_ptr<VirtualMemory> memory = mapper.map("maps/start.bsp");
        ConstMemory view = memory ? ConstMemory(*memory) : ConstMemory();
        check(memory && view.size == 6, "map maps/start.bsp");
        if (memory)
        {
            check(std::memcmp(view.address, "nested", 6) == 0, "maps/start.bsp contents");
        }
    }

    if (g_failed)
    {
        printLine(Print::Error, "{} test(s) failed.", g_failed);
        return 1;
    }

    printLine("filesystem_mapper: PASSED");
    return 0;
}
