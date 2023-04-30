#include "utils/range/range.h"
#include "gtest/gtest.h"

TEST(RangeTest, Test1)
{
    Range r(0, 2);

    ASSERT_EQ(r.Contains(0), true);
    ASSERT_EQ(r.Contains(1), true);
    ASSERT_EQ(r.Contains(2), false);
}

TEST(RangeTest, Test2)
{
    Range r1(0, 2);
    Range r2(2, 4);
    Range r3(1, 3);
    Range r4(0, 5);

    ASSERT_EQ(Range::IfIntersect(r1, r1), true);
    ASSERT_EQ(Range::Intersection(r1, r1).GetStart(), 0);
    ASSERT_EQ(Range::Intersection(r1, r1).GetEnd(), 2);
    ASSERT_EQ(Range::IfIntersect(r1, r2), false);
    ASSERT_EQ(Range::IfIntersect(r1, r3), true);
    ASSERT_EQ(Range::Intersection(r1, r3).GetStart(), 1);
    ASSERT_EQ(Range::Intersection(r1, r3).GetEnd(), 2);
    ASSERT_EQ(Range::IfIntersect(r1, r4), true);
    ASSERT_EQ(Range::Intersection(r1, r4).GetStart(), 0);
    ASSERT_EQ(Range::Intersection(r1, r4).GetEnd(), 2);

    ASSERT_EQ(Range::IfIntersect(r2, r1), false);
    ASSERT_EQ(Range::IfIntersect(r2, r2), true);
    ASSERT_EQ(Range::Intersection(r2, r2).GetStart(), 2);
    ASSERT_EQ(Range::Intersection(r2, r2).GetEnd(), 4);
    ASSERT_EQ(Range::IfIntersect(r2, r3), true);
    ASSERT_EQ(Range::Intersection(r2, r3).GetStart(), 2);
    ASSERT_EQ(Range::Intersection(r2, r3).GetEnd(), 3);
    ASSERT_EQ(Range::IfIntersect(r2, r4), true);
    ASSERT_EQ(Range::Intersection(r2, r4).GetStart(), 2);
    ASSERT_EQ(Range::Intersection(r2, r4).GetEnd(), 4);

    ASSERT_EQ(Range::IfIntersect(r3, r1), true);
    ASSERT_EQ(Range::Intersection(r3, r1).GetStart(), 1);
    ASSERT_EQ(Range::Intersection(r3, r1).GetEnd(), 2);
    ASSERT_EQ(Range::IfIntersect(r3, r2), true);
    ASSERT_EQ(Range::Intersection(r3, r2).GetStart(), 2);
    ASSERT_EQ(Range::Intersection(r3, r2).GetEnd(), 3);
    ASSERT_EQ(Range::IfIntersect(r3, r3), true);
    ASSERT_EQ(Range::Intersection(r3, r3).GetStart(), 1);
    ASSERT_EQ(Range::Intersection(r3, r3).GetEnd(), 3);
    ASSERT_EQ(Range::IfIntersect(r3, r4), true);
    ASSERT_EQ(Range::Intersection(r3, r4).GetStart(), 1);
    ASSERT_EQ(Range::Intersection(r3, r4).GetEnd(), 3);

    ASSERT_EQ(Range::IfIntersect(r4, r1), true);
    ASSERT_EQ(Range::Intersection(r4, r1).GetStart(), 0);
    ASSERT_EQ(Range::Intersection(r4, r1).GetEnd(), 2);
    ASSERT_EQ(Range::IfIntersect(r4, r2), true);
    ASSERT_EQ(Range::Intersection(r4, r2).GetStart(), 2);
    ASSERT_EQ(Range::Intersection(r4, r2).GetEnd(), 4);
    ASSERT_EQ(Range::IfIntersect(r4, r3), true);
    ASSERT_EQ(Range::Intersection(r4, r3).GetStart(), 1);
    ASSERT_EQ(Range::Intersection(r4, r3).GetEnd(), 3);
    ASSERT_EQ(Range::IfIntersect(r4, r4), true);
    ASSERT_EQ(Range::Intersection(r4, r4).GetStart(), 0);
    ASSERT_EQ(Range::Intersection(r4, r4).GetEnd(), 5);
}
