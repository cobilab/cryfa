//
// Created by morteza on 10-05-2018.
//

#ifndef PROJECT_FN_HPP
#define PROJECT_FN_HPP

/**
 * @brief Accumulate even index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 */
template <typename T, typename Iter>
T accum_even (Iter first, Iter last, T init) {
    for (; first < last; first+=2)
        init += *first;
    return init;
};

/**
 * @brief Accumulate odd index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 */
template <typename T, typename Iter>
T accum_odd (Iter first, Iter last, T init) {
    for (++first; first < last; first+=2)
        init += *first;
    return init;
};

/**
 * @brief Accumulate hop index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 * @param h      hop value
 */
template <typename T, typename Iter, typename Hop>
T accum_hops (Iter first, Iter last, T init, Hop h) {
    for (; first < last; first+=h)
        init += *first;
    return init;
};

#endif //PROJECT_FN_HPP