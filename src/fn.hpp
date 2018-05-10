//
// Created by morteza on 10-05-2018.
//

#ifndef CRYFA_FN_HPP
#define CRYFA_FN_HPP

/**
 * @brief        Accumulate hop index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 * @param h      hop value
 * @return       A number
 */
template <typename T, typename Iter, typename Hop>
T accum_hops (Iter first, Iter last, T init, Hop h) {
  for (; first < last; first+=h)
    init += *first;
  return init;
};

/**
 * @brief        Accumulate even index values in a range
 * @param first  beginning of the range
 * @param last   end of the range
 * @param init   initial value
 * @return       A number
 */
template <typename T, typename Iter>
T accum_even (Iter first, Iter last, T init) {
  return accum_hops(first, last, init, 2);
};

/**
 * @brief         Accumulate odd index values in a range
 * @param  first  beginning of the range
 * @param  last   end of the range
 * @param  init   initial value
 * @return        A number
 */
template <typename T, typename Iter>
T accum_odd (Iter first, Iter last, T init) {
  return accum_hops(first+1, last, init, 2);
};

/**
 * @brief        Save the contents of a file into a string
 * @param fname  file name
 * @param out    the output string
 */
template <typename Iter>
void file_to_string (const string& fname, Iter out) {
  std::ifstream in(fname);
  for (char c; in.get(c);)
    *out++ += c;
  in.close();
}

#endif //CRYFA_FN_HPP