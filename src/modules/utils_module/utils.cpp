#include "./utils.h"

namespace utils {
    std::expected<std::string, std::string> read_file(const std::string& file_name) {
        std::ifstream file(file_name, std::ios::binary);
        if (!file) {
            return std::unexpected("Cannot read file");
        }

        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
}
