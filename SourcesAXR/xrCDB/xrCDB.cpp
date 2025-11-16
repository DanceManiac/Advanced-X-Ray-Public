// xrCDB.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#pragma hdrstop

#include "xrCDB.h"

namespace Opcode
{
#include "OPC_TreeBuilders.h"
} // namespace Opcode

using namespace CDB;
using namespace Opcode;

bool APIENTRY DllMain(HANDLE hModule, std::uint32_t ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return true;
}

// Model building
MODEL::MODEL()
{
    tree = 0;
    tris = 0;
    tris_count = 0;
    verts = 0;
    verts_count = 0;
    status = S_INIT;
}
MODEL::~MODEL()
{
    syncronize(); // maybe model still in building
    status = S_INIT;
    CDELETE(tree);
    CFREE(tris);
    tris_count = 0;
    CFREE(verts);
    verts_count = 0;
}

void MODEL::build(Fvector* V, int Vcnt, TRI* T, int Tcnt, build_callback* bc, void* bcp, const bool rebuildTrisRequired)
{
    ZoneScoped;

    R_ASSERT(S_INIT == status);
    R_ASSERT((Vcnt >= 4) && (Tcnt >= 2));

    build_internal(V, Vcnt, T, Tcnt, bc, bcp, rebuildTrisRequired);
    status = S_READY;
}

void MODEL::build_internal(Fvector* V, int Vcnt, TRI* T, int Tcnt, build_callback* bc, void* bcp, const bool rebuildTrisRequired)
{
    ZoneScoped;

    // verts
    verts_count = Vcnt;
    verts = CALLOC(Fvector, verts_count);
    std::memcpy(verts, V, verts_count * sizeof(Fvector));

    // tris
    tris_count = Tcnt;
    tris = CALLOC(TRI, tris_count);
#ifdef _M_X64
    if (rebuildTrisRequired)
    {

        TRI_DEPRECATED* realT = reinterpret_cast<TRI_DEPRECATED*>(T);
        for (int triIter = 0; triIter < tris_count; ++triIter)
        {
            TRI_DEPRECATED& oldTri = realT[triIter];
            TRI& newTri = tris[triIter];
            newTri = oldTri;
        }
    }
    else
    {
        std::memcpy(tris, T, tris_count * sizeof(TRI));
    }
#else
    std::memcpy(tris, T, tris_count * sizeof(TRI));
#endif

    // callback
    if (bc)
        bc(verts, Vcnt, tris, Tcnt, bcp);

    // Release data pointers
    status = S_BUILD;

    // Allocate temporary "OPCODE" tris + convert tris to 'pointer' form
    std::uint32_t* temp_tris = CALLOC(std::uint32_t, tris_count * 3);
    if (0 == temp_tris)
    {
        CFREE(verts);
        CFREE(tris);
        return;
    }
    std::uint32_t* temp_ptr = temp_tris;
    for (int i = 0; i < tris_count; i++)
    {
        *temp_ptr++ = tris[i].verts[0];
        *temp_ptr++ = tris[i].verts[1];
        *temp_ptr++ = tris[i].verts[2];
    }

    // Build a non quantized no-leaf tree
    OPCODECREATE OPCC;
    OPCC.NbTris = tris_count;
    OPCC.NbVerts = verts_count;
    OPCC.Tris = (unsigned*)temp_tris;
    OPCC.Verts = (Point*)verts;
    OPCC.Rules = SPLIT_COMPLETE | SPLIT_SPLATTERPOINTS | SPLIT_GEOMCENTER;
    OPCC.NoLeaf = true;
    OPCC.Quantized = false;
    // if (Memory.debug_mode) OPCC.KeepOriginal = true;

    tree = CNEW(OPCODE_Model)();
    if (!tree->Build(OPCC))
    {
        CFREE(verts);
        CFREE(tris);
        CFREE(temp_tris);
        return;
    };

    // Free temporary tris
    CFREE(temp_tris);
    return;
}

std::uint32_t MODEL::memory()
{
    if (S_BUILD == status)
    {
        Msg("! xrCDB: model still isn't ready");
        return 0;
    }
    std::uint32_t V = verts_count * sizeof(Fvector);
    std::uint32_t T = tris_count * sizeof(TRI);
    return tree->GetUsedBytes() + V + T + sizeof(*this) + sizeof(*tree);
}

bool MODEL::serialize(const char* fileName, serialize_callback callback /*= nullptr*/) const
{
    ZoneScoped;

    IWriter* wstream = FS.w_open(fileName);
    if (!wstream)
        return false;

    CMemoryWriter memory;

    // Write to buffer, to be able to calculate crc
    memory.w_u32(version);

    if (callback)
        callback(memory);

    memory.w_u32(verts_count);
    memory.w(verts, sizeof(Fvector) * verts_count);
    memory.w_u32(tris_count);
    memory.w(tris, sizeof(TRI) * tris_count);

    if (tree)
        tree->Save(&memory);

    // Actually write to file
    const std::uint32_t crc = crc32(memory.pointer(), memory.size());
    wstream->w_u32(crc);
    wstream->w(memory.pointer(), memory.size());

    FS.w_close(wstream);

    return true;
}

bool MODEL::deserialize(const char* fileName, bool checkCrc32 /*= true*/, deserialize_callback callback /*= nullptr*/)
{
    ZoneScoped;

    IReader* rstream = FS.r_open(fileName);
    if (!rstream)
        return false;

    const std::uint32_t crc = rstream->r_u32();
    const std::uint32_t actualCrc =
        checkCrc32 ? crc32(rstream->pointer(), rstream->elapsed()) : crc;

    if (crc != actualCrc || version != rstream->r_u32())
    {
        FS.r_close(rstream);
        return false;
    }

    if (callback && !callback(*rstream))
    {
        FS.r_close(rstream);
        return false;
    }

    xr_free(verts);
    xr_free(tris);
    xr_free(tree);

    verts_count = rstream->r_u32();
    verts = xr_alloc<Fvector>(verts_count);
    const std::uint32_t vertsSize = verts_count * sizeof(Fvector);
    CopyMemory(verts, rstream->pointer(), vertsSize);
    rstream->advance(vertsSize);

    tris_count = rstream->r_u32();
    tris = xr_alloc<TRI>(tris_count);
    const std::uint32_t trisSize = tris_count * sizeof(TRI);
    CopyMemory(tris, rstream->pointer(), trisSize);
    rstream->advance(trisSize);

    tree = xr_new<OPCODE_Model>();
    tree->Load(rstream);
    status = S_READY;

    FS.r_close(rstream);

    return true;
}

// This is the constructor of a class that has been exported.
// see xrCDB.h for the class definition
COLLIDER::COLLIDER()
{
    ray_mode = 0;
    box_mode = 0;
    frustum_mode = 0;
}

COLLIDER::~COLLIDER() { r_free(); }

RESULT& COLLIDER::r_add()
{
    rd.push_back(RESULT());
    return rd.back();
}

void COLLIDER::r_free() { rd.clear(); }