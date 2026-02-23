#include <fstream>
#include <iostream>

#include "json.h"

std::vector<std::string> read_lines (const std::string& path) {
    std::ifstream fin (path);
    std::vector<std::string> lines;

    for (std::string line; !fin.eof (); lines.push_back (std::move (line))) {
        if (!lines.empty ()) lines.back () += '\n';

        std::getline (fin, line);
    }

    return lines;
}
void save_lines (const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream fout (path);
    for (const auto& line : lines) fout << line;
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

            size_t row = 0;
            for (const auto& trans_json : file) {
                const auto& trans = trans_json.get_object ();

                auto origin = trans.at ("org").get_string ();
                if (auto it = trans.find ("row"); it != trans.end ()) {
                    row = it->second.get_number ().get_int () - 1;
                }
                if (row >= lines.size ()) {
                    std::cout << "row out of bounds for file " << file_path << " , skipping this trange" << std::endl;
                    continue;
                }
                auto& line = lines[row];

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

                    pos = trraw.find (')', tpos);
                    if (pos == std::string::npos) {
                        std::cout << "trans pattern incorrect: " << trraw << " , skipping this trange" << std::endl;
                        goto discard;
                    }
                    auto cons_str = trraw.substr (tpos, pos - tpos);
                    if (auto it = cons.find (cons_str); it != cons.end ()) {
                        translated += it->second.get_string ();
                    } else {
                        std::cout << "constant pattern not found: " << cons_str << " , skipping this trange"
                                  << std::endl;
                        goto discard;
                    }
                }

                size_t col;
                if (auto it = trans.find ("col"); it != trans.end ()) {
                    col = it->second.get_number ().get_int () - 1;
                    if (line.compare (col, origin.length (), origin.data ()) != 0) {
                        std::cout << "origin pattern not matched: " << origin << " , skipping this trange" << std::endl;
                        goto discard;
                    }
                } else {
                    col = line.find (origin);
                    if (col == std::string::npos) {
                        std::cout << "origin pattern not found: " << origin << " , skipping this trange" << std::endl;
                        goto discard;
                    } else if (line.find (origin, col + 1) != std::string::npos) {
                        std::cout << "origin pattern appears more than once: " << origin
                                  << " , picking the first pattern" << std::endl;
                    }
                }

                if (auto it = trans.find ("break"); it != trans.end ()) {
                    if (it->second == "begin") {
                        tranges[row][0] = {col + origin.length (), translated};
                    } else if (it->second == "end") {
                        tranges[row][col] = {line.length () - col, translated};
                    } else {
                        tranges[row][0] = {line.length (), translated};
                    }
                } else {
                    tranges[row][col] = {origin.length (), translated};
                }

            discard:
                ++row;
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
