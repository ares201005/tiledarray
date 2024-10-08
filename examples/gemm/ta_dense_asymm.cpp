/*
 * This file is a part of TiledArray.
 * Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <TiledArray/external/btas.h>
#include <TiledArray/util/time.h>
#include <tiledarray.h>
#include <iostream>

int main(int argc, char** argv) {
  // Initialize runtime
  TiledArray::World& world = TA_SCOPED_INITIALIZE(argc, argv);

  // Get command line arguments
  if (argc < 6) {
    std::cout << "multiplies A(Nm,Nk) * B(Nk,Nn), with dimensions m, n, and k "
                 "blocked by Bm, Bn, and Bk, respectively"
              << std::endl
              << "Usage: " << argv[0]
              << " Nm Bm Nn Bn Nk Bk [repetitions=5] [scalar=double] "
                 "[do_memtrace=0]\n";
    return 0;
  }
  const long Nm = atol(argv[1]);
  const long Bm = atol(argv[2]);
  const long Nn = atol(argv[3]);
  const long Bn = atol(argv[4]);
  const long Nk = atol(argv[5]);
  const long Bk = atol(argv[6]);
  if (Nm <= 0 || Nn <= 0 || Nk <= 0) {
    std::cerr << "Error: dimensions must be greater than zero.\n";
    return 1;
  }
  if (Bm <= 0 || Bn <= 0 || Bk <= 0) {
    std::cerr << "Error: block sizes must be greater than zero.\n";
    return 1;
  }
  const long repeat = (argc >= 8 ? atol(argv[7]) : 5);
  if (repeat <= 0) {
    std::cerr << "Error: number of repetitions must be greater than zero.\n";
    return 1;
  }

  const std::string scalar_type_str = (argc >= 9 ? argv[8] : "double");
  if (scalar_type_str != "double" && scalar_type_str != "float" &&
      scalar_type_str != "zdouble" && scalar_type_str != "zfloat") {
    std::cerr << "Error: invalid real type " << scalar_type_str << ".\n";
    std::cerr << "       valid real types are \"double\", \"float\", "
                 "\"zdouble\", and \"zfloat\".\n";
    return 1;
  }

  const bool do_memtrace = (argc >= 10 ? std::atol(argv[9]) : false);

  // Construct TiledRange
  std::vector<unsigned int> blocking_m;
  for (long i = 0l; i <= Nm; i += Bm) blocking_m.push_back(i);
  if (blocking_m.back() != Nm) blocking_m.push_back(Nm);

  std::vector<unsigned int> blocking_n;
  for (long i = 0l; i <= Nn; i += Bn) blocking_n.push_back(i);
  if (blocking_n.back() != Nn) blocking_n.push_back(Nn);

  std::vector<unsigned int> blocking_k;
  for (long i = 0l; i <= Nk; i += Bk) blocking_k.push_back(i);
  if (blocking_k.back() != Nk) blocking_k.push_back(Nk);

  const std::size_t Tm = blocking_m.size() - 1;
  const std::size_t Tn = blocking_n.size() - 1;
  const std::size_t Tk = blocking_k.size() - 1;

  // Structure of c
  std::vector<TiledArray::TiledRange1> blocking_C;
  blocking_C.reserve(2);
  blocking_C.push_back(
      TiledArray::TiledRange1(blocking_m.begin(), blocking_m.end()));
  blocking_C.push_back(
      TiledArray::TiledRange1(blocking_n.begin(), blocking_n.end()));

  // Structure of a
  std::vector<TiledArray::TiledRange1> blocking_A;
  blocking_A.reserve(2);
  blocking_A.push_back(
      TiledArray::TiledRange1(blocking_m.begin(), blocking_m.end()));
  blocking_A.push_back(
      TiledArray::TiledRange1(blocking_k.begin(), blocking_k.end()));

  // Structure of b
  std::vector<TiledArray::TiledRange1> blocking_B;
  blocking_B.reserve(2);
  blocking_B.push_back(
      TiledArray::TiledRange1(blocking_k.begin(), blocking_k.end()));
  blocking_B.push_back(
      TiledArray::TiledRange1(blocking_n.begin(), blocking_n.end()));

  TiledArray::TiledRange  // TRange for c
      trange_c(blocking_C.begin(), blocking_C.end());

  TiledArray::TiledRange  // TRange for a
      trange_a(blocking_A.begin(), blocking_A.end());

  TiledArray::TiledRange  // TRange for b
      trange_b(blocking_B.begin(), blocking_B.end());

  auto run = [&](auto* tarray_ptr) {
    using Array = std::decay_t<std::remove_pointer_t<decltype(tarray_ptr)>>;
    using T = TiledArray::detail::numeric_t<Array>;
    const auto complex_T = TiledArray::detail::is_complex_v<T>;
    const double gflops_per_call =
        (complex_T ? 8 : 2)  // 1 multiply takes 6/1 flops for complex/real
                             // 1 add takes 2/1 flops for complex/real
        * static_cast<std::int64_t>(Nn) * static_cast<std::int64_t>(Nm) *
        static_cast<std::int64_t>(Nk) / 1.e9;

    if (world.rank() == 0)
      std::cout << "TiledArray: dense matrix multiply test...\n"
                << "Number of nodes     = " << world.size()
                << "\nScalar type       = " << scalar_type_str
                << "\nSize of A         = " << Nm << "x" << Nk << " ("
                << double(Nm * Nk * sizeof(T)) / 1.0e9 << " GB)"
                << "\nSize of (largest) A block   = " << Bm << "x" << Bk
                << "\nSize of B         = " << Nk << "x" << Nn << " ("
                << double(Nk * Nn * sizeof(T)) / 1.0e9 << " GB)"
                << "\nSize of (largest) B block   = " << Bk << "x" << Bn
                << "\nSize of C         = " << Nm << "x" << Nn << " ("
                << double(Nm * Nn * sizeof(T)) / 1.0e9 << " GB)"
                << "\nSize of (largest) C block   = " << Bm << "x" << Bn
                << "\n# of blocks of C  = " << Tm * Tn
                << "\nAverage # of blocks of C/node = "
                << double(Tm * Tn) / double(world.size()) << "\n";

    auto memtrace = [do_memtrace, &world](const std::string& str) -> void {
      if (do_memtrace) {
        world.gop.fence();
        madness::print_meminfo(world.rank(), str);
      } else {
        world.gop.fence();
      }
#ifdef TA_TENSOR_MEM_PROFILE
      {
        std::cout
            << str << ": TA::Tensor allocated "
            << TA::hostEnv::instance()->host_allocator_getActualHighWatermark()
            << " bytes and used "
            << TA::hostEnv::instance()->host_allocator().getHighWatermark()
            << " bytes" << std::endl;
      }
#endif
    };

    memtrace("start");
    {  // array lifetime scope
      // Construct and initialize arrays
      Array a(world, trange_a);
      Array b(world, trange_b);
      Array c(world, trange_c);
      a.fill(1.0);
      b.fill(1.0);
      memtrace("allocated a and b");

      // Start clock
      world.gop.fence();
      if (world.rank() == 0)
        std::cout << "Starting iterations: "
                  << "\n";

      // Do matrix multiplication
      for (int i = 0; i < repeat; ++i) {
        TA_RECORD_DURATION(c("m,n") = a("m,k") * b("k,n"); memtrace("c=a*b");)
        const double time = TiledArray::durations().back();
        const double gflop_rate = gflops_per_call / time;
        if (world.rank() == 0)
          std::cout << "Iteration " << i + 1 << "   time=" << time
                    << "   GFLOPS=" << gflop_rate << "\n";
      }

      // Stop clock
      const double wall_time_stop = madness::wall_time();

      if (world.rank() == 0) {
        auto durations = TiledArray::duration_statistics();
        std::cout << "Average wall time   = " << durations.mean
                  << " s\nAverage GFLOPS      = "
                  << gflops_per_call * durations.mean_reciprocal
                  << "\nMedian wall time   = " << durations.median
                  << " s\nMedian GFLOPS      = "
                  << gflops_per_call / durations.median << "\n";
      }

    }  // array lifetime scope
    memtrace("stop");
  };

  // by default use TiledArray tensors
  constexpr bool use_btas = false;
  // btas::Tensor instead
  if (scalar_type_str == "double") {
    if constexpr (!use_btas)
      run(static_cast<TiledArray::TArrayD*>(nullptr));
    else
      run(static_cast<TiledArray::DistArray<
              TiledArray::Tile<btas::Tensor<double, TiledArray::Range>>>*>(
          nullptr));
  } else if (scalar_type_str == "float") {
    if constexpr (!use_btas)
      run(static_cast<TiledArray::TArrayF*>(nullptr));
    else
      run(static_cast<TiledArray::DistArray<
              TiledArray::Tile<btas::Tensor<float, TiledArray::Range>>>*>(
          nullptr));
  } else if (scalar_type_str == "zdouble") {
    if constexpr (!use_btas)
      run(static_cast<TiledArray::TArrayZ*>(nullptr));
    else
      run(static_cast<TiledArray::DistArray<TiledArray::Tile<
              btas::Tensor<std::complex<double>, TiledArray::Range>>>*>(
          nullptr));
  } else if (scalar_type_str == "zfloat") {
    if constexpr (!use_btas)
      run(static_cast<TiledArray::TArrayC*>(nullptr));
    else
      run(static_cast<TiledArray::DistArray<TiledArray::Tile<
              btas::Tensor<std::complex<float>, TiledArray::Range>>>*>(
          nullptr));
  } else {
    abort();  // unreachable
  }

  return 0;
}
