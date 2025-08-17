struct stat st;
if (stat(rootPath.c_str(), &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
        // it's a directory
    } else {
        // it's a file
    }
}
