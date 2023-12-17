#ifndef XRCDB_H
#define XRCDB_H

// #pragma once
//  The following ifdef block is the standard way of creating macros which make exporting
//  from a DLL simpler. All files within this DLL are compiled with the XRCDB_EXPORTS
//  symbol defined on the command line. this symbol should not be defined on any project
//  that uses this DLL. This way any other project whose source files include this file see
//  XRCDB_API functions as being imported from a DLL, wheras this DLL sees symbols
//  defined with this macro as being exported.
#ifdef XRCDB_EXPORTS
#define XRCDB_API __declspec(dllexport)
#else
#define XRCDB_API __declspec(dllimport)
#endif
#ifdef M_VISUAL
#define ALIGN(a) __declspec(align(a))
#else
#define ALIGN(a)
#endif

// forward declarations
class CFrustum;
namespace Opcode
{
    class OPCODE_Model;
    class AABBNoLeafNode;
}; // namespace Opcode

#pragma pack(push, 8)
namespace CDB
{
#ifdef _M_X64
    struct TRI_DEPRECATED
    {
    public:
        std::uint32_t verts[3]; // 3*4 = 12b
        union
        {
            std::uint32_t dummy; // 4b
            struct
            {
                std::uint32_t material : 14;        //
                std::uint32_t suppress_shadows : 1; //
                std::uint32_t suppress_wm : 1;      //
                std::uint32_t sector : 16;          //
            };
        };

    public:
        IC std::uint32_t IDvert(std::uint32_t ID) { return verts[ID]; }
    };
#endif

    // Triangle
    class XRCDB_API TRI //*** 16 bytes total (was 32 :)
    {
    public:
        std::uint32_t verts[3]; // 3*4 = 12b
        union
        {
            size_t dummy; // 4b
            struct
            {
                size_t material : 14;
                size_t suppress_shadows : 1;
                size_t suppress_wm : 1;
                size_t sector : 16;

#if defined(_M_X64)
                size_t dumb : 32;
#endif
            };
        };

    public:
        IC std::uint32_t IDvert(std::uint32_t ID) { return verts[ID]; }

#if defined(_M_X64)
        TRI(TRI_DEPRECATED& oldTri)
        {
            verts[0] = oldTri.verts[0];
            verts[1] = oldTri.verts[1];
            verts[2] = oldTri.verts[2];
            dummy = oldTri.dummy;
            dumb = 0;
        }

        TRI()
        {
            verts[0] = 0;
            verts[1] = 0;
            verts[2] = 0;
            dummy = 0;
        }

        TRI& operator=(const TRI_DEPRECATED& oldTri)
        {
            verts[0] = oldTri.verts[0];
            verts[1] = oldTri.verts[1];
            verts[2] = oldTri.verts[2];
            dummy = oldTri.dummy;
            dumb = 0;
            return *this;
        }
#endif
    };

    // Build callback
    using build_callback = void(Fvector* V, int Vcnt, TRI* T, int Tcnt, void* params);
    using serialize_callback = void(IWriter& writer);
    using deserialize_callback = bool(IReader& reader);

    // Model definition
    class XRCDB_API MODEL
    {
        friend class COLLIDER;
        enum { S_READY = 0, S_INIT = 1, S_BUILD = 2, S_forcedword = std::uint32_t(-1) };

    private:
        mutable std::recursive_mutex cs;
        Opcode::OPCODE_Model* tree;
        std::uint32_t status; // 0=ready, 1=init, 2=building
        std::uint32_t version;

        // tris
        TRI* tris;
        int tris_count;
        Fvector* verts;
        int verts_count;

    public:
        MODEL();
        ~MODEL();

        IC Fvector* get_verts() { return verts; }
        IC const Fvector* get_verts() const { return verts; }
        IC int get_verts_count() const { return verts_count; }
        IC const TRI* get_tris() const { return tris; }
        IC TRI* get_tris() { return tris; }
        IC int get_tris_count() const { return tris_count; }
        IC void syncronize() const
        {
            if (S_READY != status)
            {
                Log("! WARNING: syncronized CDB::query");
                cs.lock();
                cs.unlock();
            }
        }

        void build_internal(Fvector* V, int Vcnt, TRI* T, int Tcnt, build_callback* bc = nullptr,
            void* bcp = nullptr, const bool rebuildTrisRequired = true);
        void build(Fvector* V, int Vcnt, TRI* T, int Tcnt, build_callback* bc = nullptr,
            void* bcp = nullptr, const bool rebuildTrisRequired = true);
        std::uint32_t memory();

        void set_version(std::uint32_t value) { version = value; }
        bool serialize(const char* fileName, serialize_callback callback = nullptr) const;
        bool deserialize(const char* fileName, bool checkCrc32 = true,
            deserialize_callback callback = nullptr);
    };

    // Collider result
    struct XRCDB_API RESULT
    {
        Fvector verts[3];
        union {
            size_t dummy; // 4b
            struct {
                size_t material : 14;        //
                size_t suppress_shadows : 1; //
                size_t suppress_wm : 1;      //
                size_t sector : 16;          //
#if defined(_M_X64)
                std::uint64_t dumb : 32;
#endif
            };
        };
        int id;
        float range;
        float u, v;
    };

    // Collider Options
    enum {
        OPT_CULL = (1 << 0),
        OPT_ONLYFIRST = (1 << 1),
        OPT_ONLYNEAREST = (1 << 2),
        OPT_FULL_TEST = (1 << 3) // for box & frustum queries - enable class III test(s)
    };

    // Collider itself
    class XRCDB_API COLLIDER
    {
        // Ray data and methods
        std::uint32_t ray_mode;
        std::uint32_t box_mode;
        std::uint32_t frustum_mode;

        // Result management
        xr_vector<RESULT> rd;

    public:
        COLLIDER();
        ~COLLIDER();

        void ray_options(std::uint32_t f) { ray_mode = f; }
        void ray_query(const MODEL* m_def, const Fvector& r_start, const Fvector& r_dir,
            float r_range = 10000.f);

        void box_options(std::uint32_t f) { box_mode = f; }
        void box_query(const MODEL* m_def, const Fvector& b_center, const Fvector& b_dim);

        void frustum_options(std::uint32_t f) { frustum_mode = f; }
        void frustum_query(const MODEL* m_def, const CFrustum& F);

        RESULT* r_begin() { return std::data(rd); }
        RESULT* r_end() { return std::data(rd) + std::size(rd); }
        RESULT& r_add();
        void r_free();
        int r_count() { return rd.size(); }
        void r_clear() { rd.clear(); }
        void r_clear_compact() { rd.clear(); }
    };

    //
    class XRCDB_API Collector
    {
        xr_vector<Fvector> verts;
        xr_vector<TRI> faces;

        std::uint32_t VPack(const Fvector& V, float eps);

    public:
        void add_face(const Fvector& v0, const Fvector& v1, const Fvector& v2, std::uint16_t material,
            std::uint16_t sector);
        void add_face_D(const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy);
        void add_face_packed(const Fvector& v0, const Fvector& v1, const Fvector& v2,
            std::uint16_t material, std::uint16_t sector, float eps = EPS);
        void add_face_packed_D(const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy,
            float eps = EPS);
        void remove_duplicate_T();
        void calc_adjacency(xr_vector<std::uint32_t>& dest);

        Fvector* getV() { return &*verts.begin(); }
        size_t getVS() { return verts.size(); }
        TRI* getT() { return &*faces.begin(); }
        size_t getTS() { return faces.size(); }
        void clear()
        {
            verts.clear();
            faces.clear();
        }
    };

    struct non_copyable
    {
        non_copyable() {}

    private:
        non_copyable(const non_copyable&) {}
        non_copyable& operator=(const non_copyable&) {}
    };

#pragma warning(push)
#pragma warning(disable : 4275)
    const std::uint32_t clpMX = 24, clpMY = 16, clpMZ = 24;
    class XRCDB_API CollectorPacked : public non_copyable
    {
        typedef xr_vector<std::uint32_t> DWORDList;
        typedef DWORDList::iterator DWORDIt;

    private:
        xr_vector<Fvector> verts;
        xr_vector<TRI> faces;
        xr_vector<std::uint32_t> flags;
        Fvector VMmin, VMscale;
        DWORDList VM[clpMX + 1][clpMY + 1][clpMZ + 1];
        Fvector VMeps;

        std::uint32_t VPack(const Fvector& V);

    public:
        CollectorPacked(const Fbox& bb, int apx_vertices = 5000, int apx_faces = 5000);

        //		__declspec(noinline) CollectorPacked &operator=	(const CollectorPacked &object)
        //		{
        //			verts
        //		}

        void add_face(const Fvector& v0, const Fvector& v1, const Fvector& v2, std::uint16_t material,
            std::uint16_t sector, std::uint32_t flags);
        void add_face_D(const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy,
            std::uint32_t flags);

        xr_vector<Fvector>& getV_Vec() { return verts; }
        Fvector* getV() { return &*verts.begin(); }
        size_t getVS() { return verts.size(); }
        TRI* getT() { return &*faces.begin(); }
        std::uint32_t getfFlags(std::uint32_t index) { return flags[index]; }
        IC TRI& getT(std::uint32_t index) { return faces[index]; }
        size_t getTS() { return faces.size(); }
        void clear();
    };
#pragma warning(pop)
}; // namespace CDB

#pragma pack(pop)
#endif
