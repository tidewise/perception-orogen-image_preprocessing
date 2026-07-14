#ifndef image_preprocessing_TYPES_HPP
#define image_preprocessing_TYPES_HPP

#include <cstdint>
#include <vector>

namespace image_preprocessing {

    enum GrayscaleMethod {
        OPENCV,
        SUM
    };

    struct Range3U8 {
        std::vector<uint8_t> min;
        std::vector<uint8_t> max;

        /** Range3U8 is valid if element-wise all min is less or equal max */
        bool valid() const
        {
            if (min.size() != 3 || max.size() != 3) {
                return false;
            }

            return (min[0] <= max[0]) && (min[1] <= max[1]) && (min[2] <= max[2]);
        }
    };
}
#endif