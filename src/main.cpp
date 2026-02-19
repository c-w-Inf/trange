#include <fstream>
#include <iostream>

#include "json.h"

std::vector<std::string> read_lines (const std::string& path) {
    std::ifstream fin (path);
    std::vector<std::string> lines;

    for (std::string line; std::getline (fin, line);) lines.push_back (line);

    return lines;
}
void save_lines (const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream fout (path);
    for (size_t i = 0; i < lines.size () - 1; ++i) fout << lines[i] << '\n';
    fout << lines[lines.size () - 1];
}

int main () {
    json::json tr_json;
    std::cin >> tr_json;
    try {
        const auto& tr = tr_json.get_object ();

        const auto& files = tr.at ("translation").get_object ();
        const auto& cons = tr.at ("constants").get_object ();

        for (const auto& [file_path, file_json] : files) {
            const auto& file = file_json.get_array ();
            auto lines = read_lines (file_path);

            std::vector<std::map<size_t, std::pair<size_t, std::string>, std::greater<size_t>>> tranges (lines.size ());

            for (const auto& trans_json : file) {
                const auto& trans = trans_json.get_object ();

                auto origin = trans.at ("org").get_string ();
                auto row = trans.at ("row").get_number ().get_int () - 1;

                const auto& trraw = trans.at ("trans").get_string ();
                auto trrlen = trraw.length ();
                std::string translated;
                for (size_t pos = 0;; ++pos) {
                    auto tpos = trraw.find ("#_(", pos);

                    if (tpos == std::string::npos) {
                        translated.append (trraw, pos, trrlen - pos);
                        break;
                    }

                    translated.append (trraw, pos, tpos - pos);
                    tpos += 3;

                    pos = trraw.find (')');
                    if (pos == std::string::npos) {
                        std::cout << "trans pattern incorrect: " << trraw << " , skipping this trange" << std::endl;
                        goto discard;
                    }
                    translated += cons.at (trraw.substr (tpos, pos - tpos)).get_string ();
                }

                size_t col;
                if (auto it = trans.find ("col"); it != trans.end ()) {
                    col = it->second.get_number ().get_int () - 1;
                    if (lines[row].length () < col + origin.length ()) {
                        std::cout << "origin pattern not matched: " << origin << " , skipping this trange" << std::endl;
                        goto discard;
                    }
                } else {
                    col = lines[row].find (origin);
                    if (col == std::string::npos) {
                        std::cout << "origin pattern not found: " << origin << " , skipping this trange" << std::endl;
                        goto discard;
                    }
                }

                tranges[row][col] = {origin.length (), translated};

            discard:
                continue;
            }

            for (size_t row = 0; row < lines.size (); ++row) {
                for (const auto& [col, trange] : tranges[row]) {
                    const auto& [orglen, translated] = trange;

                    lines[row].replace (col, orglen, translated);
                }
            }

            save_lines (file_path, lines);
        }
    } catch (const std::exception& e) {
        std::cerr << "json format error!" << std::endl;
        std::cerr << e.what () << std::endl;
    }

    return 0;
}
