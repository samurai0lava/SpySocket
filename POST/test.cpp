int findClosest(const std::vector<std::string>& locations, const std::string& path) {
    int bestIndex = -1;
    size_t bestLength = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        const std::string& loc = locations[i];

        // Must be a prefix of path
        if (path.compare(0, loc.length(), loc) == 0) {
            // Make sure the next char is '/' or nothing (to avoid /cgi-bin matching /cgi-binary)
            if (path.length() == loc.length() || path[loc.length()] == '/' || loc[loc.length()-1] == '/') {
                if (loc.length() > bestLength) {
                    bestIndex = i;
                    bestLength = loc.length();
                }
            }
        }
    }

    return bestIndex;
}
