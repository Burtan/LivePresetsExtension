#include "gtest/gtest.h"
#include "util/util.h"

bool (*cmp)(int&, long&) = [](int& a, long& b) -> bool {
    return a == b;
};

TEST(util_test_addedAndRemoved, utils_test) {
    std::vector<int> saved = {1, 1, 1, 2, 2, 2};
    std::vector<long> current = {1, 2, 3};
    std::vector<int> removed;
    std::vector<long> added;
    std::vector<std::pair<int, long>> updated;

    compareVectors(saved, current, removed, added, updated, *cmp);

    ASSERT_EQ(updated.size(), 2);
    ASSERT_EQ(removed.size(), 4);
    ASSERT_EQ(added.size(), 1);
}

TEST(util_test_added, utils_test) {
    std::vector<int> saved = {1, 2};
    std::vector<long> current = {3, 1, 4, 2, 5, 6};
    std::vector<int> removed;
    std::vector<long> added;
    std::vector<std::pair<int, long>> updated;

    compareVectors(saved, current, removed, added, updated, *cmp);

    ASSERT_EQ(updated.size(), 2);
    ASSERT_EQ(removed.size(), 0);
    ASSERT_EQ(added.size(), 4);
}

TEST(util_test_removed, utils_test) {
    std::vector<int> saved = {1, 1, 2, 2};
    std::vector<long> current = {1, 2};
    std::vector<int> removed;
    std::vector<long> added;
    std::vector<std::pair<int, long>> updated;

    compareVectors(saved, current, removed, added, updated, *cmp);

    ASSERT_EQ(updated.size(), 2);
    ASSERT_EQ(removed.size(), 2);
    ASSERT_EQ(added.size(), 0);
}