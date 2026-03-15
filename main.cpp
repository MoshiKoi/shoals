#include <algorithm>
#include <fstream>
#include <map>
#include <mdspan>
#include <memory>
#include <print>
#include <set>
#include <string>
#include <string_view>
#include <vector>

void scs_table_fill(std::string_view a, std::string_view b, auto scs) {
  auto const n = a.size();
  auto const m = b.size();
  for (std::size_t i = 0; i < n + 1; ++i) {
    scs[i, 0] = i;
  }
  for (std::size_t j = 0; j < m + 1; ++j) {
    scs[0, j] = j;
  }

  for (std::size_t i = 1; i < n + 1; ++i) {
    for (std::size_t j = 1; j < m + 1; ++j) {
      if (a[i - 1] == b[j - 1]) {
        scs[i, j] = scs[i - 1, j - 1] + 1;
      } else {
        scs[i, j] = std::min(scs[i - 1, j], scs[i, j - 1]) + 1;
      }
    }
  }
}

std::size_t scs_length(std::string_view a, std::string_view b) {
  auto const n = a.size();
  auto const m = b.size();
  auto dp = std::make_unique<std::size_t[]>((n + 1) * (m + 1));

  std::mdspan scs{dp.get(), n + 1, m + 1};
  scs_table_fill(a, b, scs);
  return scs[n, m];
}

std::string scs_string(std::string_view a, std::string_view b) {
  auto const n = a.size();
  auto const m = b.size();
  auto dp = std::make_unique<std::size_t[]>((n + 1) * (m + 1));

  std::mdspan scs{dp.get(), n + 1, m + 1};
  scs_table_fill(a, b, scs);

  std::string result;

  std::size_t i = n;
  std::size_t j = m;

  while (i > 0 and j > 0) {
    if (a[i - 1] == b[j - 1]) {
      result.push_back(a[i - 1]);
      i--;
      j--;
    } else if (scs[i - 1, j] < scs[i, j - 1]) {
      result.push_back(a[i - 1]);
      i--;
    } else {
      result.push_back(b[j - 1]);
      j--;
    }
  }

  while (i > 0) {
    result.push_back(a[i - 1]);
    i--;
  }
  while (j > 0) {
    result.push_back(b[j - 1]);
    j--;
  }

  std::ranges::reverse(result);

  return result;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::println(stderr, "Usage: {} <file>", argv[0]);
    return 1;
  }

  std::vector<std::string> fishes;
  std::ifstream ifs{argv[1]};

  std::string buffer;
  while (std::getline(ifs, buffer)) {
    fishes.emplace_back(std::move(buffer));
  }

  std::println("Read {} fishes", fishes.size());

  std::map<std::string, std::set<std::string_view>> strings;

  auto const count_fishes = [&](std::string_view string,
                                std::set<std::string_view> &contains) {
    for (std::string const &fish : fishes) {
      if (string.size() >= fish.size() and
          scs_length(string, fish) == string.size()) {
        contains.emplace(fish);
      }
    }
  };

  for (std::string const &fish : fishes) {
    std::set<std::string_view> contains;
    count_fishes(fish, contains);
    strings.emplace(fish, std::move(contains));
  }

  while (true) {
    decltype(strings)::iterator best_a, best_b;
    std::size_t best_delta = std::numeric_limits<std::size_t>::max();

    auto const reset_best = [&] {
      best_delta = std::numeric_limits<std::size_t>::max();
      best_a = {};
      best_b = {};
    };

    auto a = strings.begin();
  outer:
    while (a != strings.end()) {
      auto b = strings.begin();
      while (b != a) {
        auto &[a_str, a_set] = *a;
        auto &[b_str, b_set] = *b;
        auto scs_len = scs_length(a_str, b_str);
        // is this a good heuristic? who knows
        auto delta = scs_len - std::max(a_str.size(), b_str.size());
        if (delta == 0) {
          if (a_str.size() > b_str.size()) {
            if (best_a == b or best_b == b) {
              reset_best();
            }
            std::println("'{}' -> '{}'", b_str, a_str);
            b = strings.erase(b);
          } else {
            if (best_a == a or best_b == a) {
              reset_best();
            }
            std::println("'{}' -> '{}'", a_str, b_str);
            a = strings.erase(a);
            goto outer;
          }
        } else if (delta < best_delta) {
          best_a = a;
          best_b = b;
          best_delta = delta;
          ++b;
        } else {
          ++b;
        }
      }
      ++a;
    }

    if (best_delta == std::numeric_limits<std::size_t>::max()) {
      continue;
    }

    auto &[a_str, a_set] = *best_a;
    auto &[b_str, b_set] = *best_b;

    auto scs = scs_string(a_str, b_str);
    std::set<std::string_view> merged;
    count_fishes(scs, merged);
    if (merged.size() >= 255) {
      std::println("{}", scs);
      for (auto const &fish : merged) {
        std::print("'{}' ", fish);
      }
      break;
    }

    std::println("'{}' '{}' -> '{}' ({})", a_str, b_str, scs, merged.size());
    strings.erase(best_a);
    strings.erase(best_b);
    strings.emplace(std::move(scs), std::move(merged));
  }
}