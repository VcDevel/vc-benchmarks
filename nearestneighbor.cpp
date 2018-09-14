#include "benchmark.h"
#include <limits>
#include <random>

template <class Float = float> struct PositionTemplate {
  Float x, y, z;
  Vc_SIMDIZE_INTERFACE((x, y, z));
};

using Position = PositionTemplate<>;
using PositionV = Vc::simdize<Position>;
using FloatV = Vc::simdize<float>;
using IntV = Vc::simdize<int, FloatV::size()>;
using IntM = IntV::mask_type;

template <class T> int index_of_min(T x) { return (x.min() == x).firstOne(); }

struct std_for_each {
  int operator()(const Position to, const std::vector<Position> &particles) {
    float best = std::numeric_limits<float>::max();
    int best_index = 0, i = 0;
    std::for_each(particles.begin(), particles.end(), [&](auto p) {
      auto dx = p.x - to.x;
      auto dy = p.y - to.y;
      auto dz = p.z - to.z;
      auto distance2 = dx * dx + dy * dy + dz * dz;
      const auto min_distance = distance2;
      if (min_distance < best) {
        best = min_distance;
        best_index = i;
      }
      ++i;
    });
    return best_index;
  }
};

struct simd_for_each {
  int operator()(const Position to, const std::vector<Position> &particles) {
    FloatV best = std::numeric_limits<float>::max();
    IntV best_index = 0;
    int i = 0;
    Vc::simd_for_each(particles.begin(), particles.end(), [&](const auto &p) {
      auto dx = p.x - to.x;
      auto dy = p.y - to.y;
      auto dz = p.z - to.z;
      auto distance2 = simd_cast<FloatV>(sqrt(dx * dx + dy * dy + dz * dz));
      if (any_of(distance2 < best)) {
        best_index(simd_cast<IntM>(distance2 < best)) =
            i + IntV([&](int n) { return n % p.size(); });
        best = min(distance2, best);
      }
      i += p.size();
    });
    return best_index[index_of_min(best)];
  }
};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> rnd0_10(0.f, 10.f);

std::vector<Position> create_particles(int size) {
  std::vector<Position> particles;
  particles.reserve(size);
  for (; size; --size) {
    particles.push_back({rnd0_10(gen), rnd0_10(gen), rnd0_10(gen)});
  }
  return particles;
}

template <class Method, class Verify>
void find_nearest(benchmark::State &state) {
  const auto particles = create_particles(state.range(0));
  Position to{rnd0_10(gen), rnd0_10(gen), rnd0_10(gen)};
  Method findNearest;
  int index = 0;
  for (auto _ : state) {
    to.x *= 0.9f;
    to.y *= 0.9f;
    to.z *= 0.9f;
    benchmark::DoNotOptimize(index = findNearest(to, particles));
  }
  Verify verify;
  if (verify(to, particles) != index) {
    std::cerr << "the find implementations don't agree\n";
  }
  state.counters["Bytes"] = state.iterations() * particles.size() * sizeof(Position);
}

void aovs(benchmark::State &state) {
  int size = state.range(0);
  std::vector<PositionV> particles;
  particles.reserve(size / PositionV::size());
  for (; size > 0; size -= PositionV::size()) {
    particles.push_back(PositionV([](int) -> Position {
      return {rnd0_10(gen), rnd0_10(gen), rnd0_10(gen)};
    }));
  }
  Position to{rnd0_10(gen), rnd0_10(gen), rnd0_10(gen)};
  for (auto _ : state) {
    to.x *= 0.9f;
    to.y *= 0.9f;
    to.z *= 0.9f;
    FloatV best = std::numeric_limits<float>::max();
    IntV best_index = 0, i([](int n) { return n; });
    std::for_each(particles.begin(), particles.end(), [&](PositionV p) {
      auto dx = p.x - to.x;
      auto dy = p.y - to.y;
      auto dz = p.z - to.z;
      auto distance2 = dx * dx + dy * dy + dz * dz;
      if (any_of(distance2 < best)) {
        best_index(simd_cast<IntM>(distance2 < best)) = i;
        best = min(distance2, best);
      }
      i += int(p.size());
    });
    benchmark::DoNotOptimize(best_index[index_of_min(best)]);
  }
  state.counters["Bytes"] = state.iterations() * particles.size() * sizeof(PositionV);
}

BENCHMARK_TEMPLATE(find_nearest, std_for_each, simd_for_each)->Range(8, 8 << 20);
BENCHMARK_TEMPLATE(find_nearest, simd_for_each, std_for_each)->Range(8, 8 << 20);
BENCHMARK(aovs)->Range(8, 8 << 20);
