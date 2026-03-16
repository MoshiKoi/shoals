#include <algorithm>
#include <fstream>
#include <mdspan>
#include <print>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

void scs_table_fill(std::string_view a, std::string_view b,
                    std::vector<std::size_t> &buffer) {
  auto const n = a.size();
  auto const m = b.size();
  buffer.reserve((n + 1) * (m + 1));
  std::mdspan scs{buffer.data(), n + 1, m + 1};

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

std::size_t scs_length(std::string_view a, std::string_view b,
                       std::size_t *buffer) {
  auto const n = a.size();
  auto const m = b.size();
  std::mdspan scs{buffer, n + 1, m + 1};
  return scs[n, m];
}

std::string scs_string(std::string_view a, std::string_view b,
                       std::size_t *buffer) {
  auto const n = a.size();
  auto const m = b.size();

  std::mdspan scs{buffer, n + 1, m + 1};
  std::string result;
  result.reserve(scs[n, m]);

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

struct scs_pair {
  double ratio;
  std::string string;

  auto operator<=>(scs_pair const &other) const = default;
};

bool has_subsequence(std::string_view a, std::string_view b) {
  char const *p = a.data();
  char const *end = a.data() + a.size();
  for (char c : b) {
    while (p != end and *p != c) {
      ++p;
    }
    if (p == end) {
      return false;
    }
    ++p;
  }
  return true;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::println(stderr, "Usage: {} <file>", argv[0]);
    return 1;
  }

  std::vector<std::string> fishes;
  std::ifstream ifs{argv[1]};

  std::string line;
  while (std::getline(ifs, line)) {
    fishes.emplace_back(std::move(line));
  }

  std::println("Read {} fishes", fishes.size());

  std::vector<std::size_t> buffer;

  auto const count_fishes = [&](std::string_view string) {
    return std::ranges::count_if(fishes, [&](auto const &fish) {
      return has_subsequence(string, fish);
    });
  };

  std::set<std::string> strings{std::from_range, fishes};
  std::set<scs_pair> pairs;

  for (auto a = strings.begin(); a != strings.end(); ++a) {
    for (auto b = strings.begin(); b != a; ++b) {
      auto &a_str = *a;
      auto &b_str = *b;
      scs_table_fill(a_str, b_str, buffer);
      auto scs = scs_string(a_str, b_str, buffer.data());

      if (strings.contains(scs)) {
        continue;
      }

      auto count = count_fishes(scs);
      double ratio = static_cast<double>(scs.size()) / count;
      pairs.emplace(ratio, scs);
    }
  }

  std::string current_solution;

  while (true) {
    auto iter = pairs.begin();
    auto count = count_fishes(iter->string);

    if (count >= 255) {
      if (iter->string.size() < current_solution.size() ||
          current_solution.empty()) {
        std::println("Possible solution: {} ({}, {}, {})", iter->string, iter->string.size(), count, iter->ratio);
        current_solution = iter->string;
      }
    } else {
      for (auto &a_str : strings) {
        scs_table_fill(a_str, iter->string, buffer);
        auto scs = scs_string(a_str, iter->string, buffer.data());

        if (strings.contains(scs)) {
          continue;
        }

        auto count = count_fishes(scs);
        double ratio = static_cast<double>(scs.size()) / count;
        pairs.emplace(ratio, scs);
      }
      strings.emplace(iter->string);
    }
    pairs.erase(iter);
  }
}
