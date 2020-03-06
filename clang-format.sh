find source/ -iname '*.h' -o -iname '*.hpp' -o -iname '*.inl' -o -iname '*.cpp' | xargs clang-format -i -style=file
