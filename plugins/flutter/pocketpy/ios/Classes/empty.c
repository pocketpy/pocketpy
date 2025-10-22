void py_initialize();

__attribute__((used)) void ensure_no_tree_shaking(void) {
    py_initialize();
}
