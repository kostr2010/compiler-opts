
// #include "ir/bb.h"
// #include "ir/graph.h"
// #include "ir/graph_builder.h"
// #include "pass/pass_manager_default_passes.h"

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
//     // std::array<marker::Marker, Pass::PassTraits<PassT>::num_marks::value> arr;
//     // for (int i = 0; i < arr.size(); ++i) {
//     //     arr[i] = std::move(marker::MarkerFactory::AcquireMarker());
//     // }

//     std::vector<marker::Marker> arr;
//     arr.reserve(Pass::PassTraits<PassT>::num_marks::value);

//     for (size_t i = 0; i < Pass::PassTraits<PassT>::num_marks::value; ++i) {
//         arr.emplace_back(marker::MarkerFactory::AcquireMarker());
//     }

//     return arr;
// }

// template <typename PassT>
// using Markers = marker::Marker[Pass::PassTraits<PassT>::num_marks::value];

// void foo(const Pass::Markers<BFS> m)
// {
//     std::cout << m[0].GetSlot() << "\n";
//     std::cout << "**\n";
// }

int main()
{

    // std::array<marker::Marker, Pass::PassTraits<BFS>::num_marks::value> v = {
    //     marker::MarkerFactory::AcquireMarker(), marker::MarkerFactory::AcquireMarker(),
    //     marker::MarkerFactory::AcquireMarker()
    // };

    // Markers markers{ marker::MarkerFactory::AcquireMarker(),
    //                  marker::MarkerFactory::AcquireMarker(),
    //                  marker::MarkerFactory::AcquireMarker() };

    // using Markers = Pass::Markers<BFS>;

    // Markers markers = {
    //     marker::MarkerFactory::AcquireMarker(),
    // };

    // std::cout << "*\n";

    // foo(markers);

    // v.emplace_back(marker::MarkerFactory::AcquireMarker());
    // v.emplace_back(marker::MarkerFactory::AcquireMarker());
    // v.emplace_back(marker::MarkerFactory::AcquireMarker());

    // for (size_t i = 0; i < Pass::PassTraits<BFS>::num_marks::value; ++i) {
    //     auto a = marker::MarkerFactory::AcquireMarker();
    //     v.emplace_back(std::move(a));
    // }

    // auto a1 = GetMarkers<DBE>();
    // std::cout << a1.size() << "\n";
    // auto m1 = marker::MarkerFactory::AcquireMarker();
    // auto m2 = marker::MarkerFactory::AcquireMarker();
    // auto m3 = marker::MarkerFactory::AcquireMarker();
    // auto m4 = marker::MarkerFactory::AcquireMarker();
    // auto m5 = marker::MarkerFactory::AcquireMarker();
    // auto m6 = marker::MarkerFactory::AcquireMarker();
    return 0;
}

#pragma GCC diagnostic pop
