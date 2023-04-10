
// #include "ir/bb.h"
// #include "ir/graph.h"
// #include "ir/graph_builder.h"
#include "ir/default_passes.h"
#include "ir/marker_factory.h"

#include <array>
#include <bitset>
#include <iostream>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

// file for tomfoolery and experiments

// template <typename PassT>
// static auto GetMarkers()
// {
//     // std::array<marking::Marker, Pass::PassTraits<PassT>::num_marks::value> arr;
//     // for (int i = 0; i < arr.size(); ++i) {
//     //     arr[i] = std::move(marking::MarkerFactory::AcquireMarker());
//     // }

//     std::vector<marking::Marker> arr;
//     arr.reserve(Pass::PassTraits<PassT>::num_marks::value);

//     for (size_t i = 0; i < Pass::PassTraits<PassT>::num_marks::value; ++i) {
//         arr.emplace_back(marking::MarkerFactory::AcquireMarker());
//     }

//     return arr;
// }

// template <typename PassT>
// using Markers = marking::Marker[Pass::PassTraits<PassT>::num_marks::value];

// void foo(const Pass::Markers<BFS> m)
// {
//     std::cout << m[0].GetSlot() << "\n";
//     std::cout << "**\n";
// }

int main()
{

    // std::array<marking::Marker, Pass::PassTraits<BFS>::num_marks::value> v = {
    //     marking::MarkerFactory::AcquireMarker(), marking::MarkerFactory::AcquireMarker(),
    //     marking::MarkerFactory::AcquireMarker()
    // };

    // Markers markers{ marking::MarkerFactory::AcquireMarker(),
    //                  marking::MarkerFactory::AcquireMarker(),
    //                  marking::MarkerFactory::AcquireMarker() };

    // using Markers = Pass::Markers<BFS>;

    // Markers markers = {
    //     marking::MarkerFactory::AcquireMarker(),
    // };

    // std::cout << "*\n";

    // foo(markers);

    // v.emplace_back(marking::MarkerFactory::AcquireMarker());
    // v.emplace_back(marking::MarkerFactory::AcquireMarker());
    // v.emplace_back(marking::MarkerFactory::AcquireMarker());

    // for (size_t i = 0; i < Pass::PassTraits<BFS>::num_marks::value; ++i) {
    //     auto a = marking::MarkerFactory::AcquireMarker();
    //     v.emplace_back(std::move(a));
    // }

    // auto a1 = GetMarkers<DBE>();
    // std::cout << a1.size() << "\n";
    // auto m1 = marking::MarkerFactory::AcquireMarker();
    // auto m2 = marking::MarkerFactory::AcquireMarker();
    // auto m3 = marking::MarkerFactory::AcquireMarker();
    // auto m4 = marking::MarkerFactory::AcquireMarker();
    // auto m5 = marking::MarkerFactory::AcquireMarker();
    // auto m6 = marking::MarkerFactory::AcquireMarker();
    return 0;
}

#pragma GCC diagnostic pop
