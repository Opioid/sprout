 find source/ -iname '*.hpp' -o -iname '*.inl' -o -iname '*.cpp' | xargs clang-format -i -style=file
