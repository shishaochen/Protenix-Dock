#include <atomic>
#include <chrono>
#include <iostream>
#include <string>

#if ENABLE_SIMD_AVX2

#include "bytedock/core/data.h"
#include "bytedock/core/entrance.h"
#include "bytedock/core/interaction.h"
#include "bytedock/ext/pfile.h"

namespace bytedock {

template <int SIMD = 0>
int time_mm_intra_nb(const std::string& pdb) {
    self_nonbonded_interactions sni;

    std::string receptor_file = pdb + "/prepared-receptor.json";
    auto receptor = std::make_shared<torsional_receptor>();
    {
        receptor->parse(open_for_read(receptor_file));
    }
    auto mol_xyz = receptor->get_positions();
    auto mol_ffdata = receptor->get_ffdata();
    molecule_pose xyz_gradient(mol_xyz.size());

    if constexpr (SIMD > 0) {
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto start = std::chrono::high_resolution_clock::now();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto energy = sni.put_gradients(mol_xyz, mol_ffdata, xyz_gradient);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto end = std::chrono::high_resolution_clock::now();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    } else {
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto start = std::chrono::high_resolution_clock::now();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        double energy = sni.put_gradients_nosimd(mol_xyz, mol_ffdata, xyz_gradient);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto end = std::chrono::high_resolution_clock::now();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
}

}

int main(int argc, char** argv) {
    bytedock::setup_global_logging("-", 1);
    auto infile = bytedock::open_for_read("pdb.list");
    std::cout << "pdb, normal_ms, simd_ms" << std::endl;
    std::string line;
    size_t line_no = 0;
    while (std::getline(*infile, line)) {
        line_no += 1;
        if (!line.empty()) {
            int normal_ms = 0, simd_ms = 0;
            for (int i = 0; i < 100; ++i) {
                normal_ms += bytedock::time_mm_intra_nb<0>(line);
                simd_ms += bytedock::time_mm_intra_nb<1>(line);
            }
            std::cout << line << ", " << (normal_ms / 100.) << ", " << (simd_ms / 100.) << std::endl;
        }
    }
    return 0;
}

#else

int main() {
    std::cout << "SIMD is not enabled." << std::endl;
    return 0;
}

#endif
