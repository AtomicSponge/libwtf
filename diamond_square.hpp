/*
 * Height Map Generator using Diamond Square
 * By:  Matthew Evans
 * File:  diamond_square.hpp
 * Version:  TESTING
 *
 * See LICENSE.md for copyright information.
 *
 * Implementation of the diamond square algorithm as a C++ class.
 * Creates a vector that can then be used as a height map.
 * 
 * https://en.wikipedia.org/wiki/Diamond-square_algorithm
 * 
 * Example:
 * 
 * diamond_square my_map = diamond_square<long double>(8, 0.096f);  //  Initialize object
 * my_map.build_map();  //  Generate the map
 * height_map some_map = my_map.get_map();  //  Do something with the map
 * 
 */

#ifndef WTF_DIAMOND_SQUARE_HPP
#define WTF_DIAMOND_SQUARE_HPP

#ifndef WTF_DS_MIN_SIZE
#define WTF_DS_MIN_SIZE (2)
#endif

#ifndef WTF_DS_MAX_SIZE
#define WTF_DS_MAX_SIZE (14)
#endif

#include <vector>
#include <ctime>
#include <cmath>
#include <exception>

namespace wtf {

//!  Define a container for storing height maps
template <typename T>
using height_map = std::vector<T>;

template <typename T>
/*!
 * \class diamond_square
 * \brief Create a height map using the diamond square algorithm.
 * \tparam T Height map type - double or long double.
 */
class diamond_square {
    public:
        /*!
         * \brief Initialize the diamond square object.
         * \param size Size value to generate map with (2 - 14)
         * \param offset Offset value - higher value for more even terrain
         * \param seed Seed for random
         */
        diamond_square(
            const std::size_t& size,
            const T& offset,
            const uint32_t& seed
        ) : map_offset(offset), _map_seed(seed), _map_side(0) {
            initialize(size);
        };

        /*!
         * \brief Initialize the diamond square object.  Use time as seed.
         * \param size Size value to generate map with (2 - 14)
         * \param offset Offset value - higher value for more even terrain
         */
        diamond_square(
            const std::size_t& size,
            const T& offset
        ) : map_offset(offset), _map_seed(std::time(nullptr)), _map_side(0) {
            initialize(size);
        };

        diamond_square() = delete;    //!<  Delete default constructor.
        ~diamond_square() = default;  //!<  Default destructor.

        /*!
         * \brief Get the height map vector.
         * \return The height map.
         */
        const height_map operator*() const { return hmap; };

        /*!
         * \brief Get the height map vector.
         * \return The height map.
         */
        const height_map get_map(void) const { return hmap; };

        /*!
         * \brief Get a single value in the height map.
         * \param pos Position to get value for.
         * \return Map value at position.
         */
        const T operator[](const std::size_t& pos) const { get_value(pos); }

        /*!
         * \brief Get a single value in the height map.
         * \param pos Position to get value for.
         * \return Map value at position.
         */
        const T get_value(const std::size_t& pos) const {
            if(pos >= hmap.size()) throw std::out_of_range("Invalid map position.");
            return hmap[pos];
        };

        //!  ...
        static const std::size_t min_size = WTF_DS_MIN_SIZE;
        //!  ...
        static const std::size_t max_size = WTF_DS_MAX_SIZE;
        //!  ...
        const std::size_t& map_side = _map_side;
        //!  ...
        const uint32_t& map_seed = _map_seed;

        /*!
         * \brief Build the height map using the power of diamond square!
         * Call this after declaring the object to build the actual map.
         */
        void build_map(void) {
            std::srand(_map_seed);  //  Set seed.

            //  Set the initial values in the four corners of the map.
            //  Also counts as the first square step.
            hmap[0] = (static_cast<T>(rand()) / static_cast<T>(RAND_MAX)) / map_offset;
            hmap[_map_side - 1] = (static_cast<T>(rand()) / static_cast<T>(RAND_MAX)) / map_offset;
            hmap[(_map_side * _map_side) - _map_side] = (static_cast<T>(rand()) / static_cast<T>(RAND_MAX)) / map_offset;
            hmap[(_map_side * _map_side) - 1] = (static_cast<T>(rand()) / static_cast<T>(RAND_MAX)) / map_offset;

            std::size_t step_size, half_step;  //  For storing our step sizes.
            T cor1, cor2, cor3, cor4, new_value, scale;  //  For storing our results.

            //  Set our step size and start the loop.
            step_size = _map_side - 1;

            //  Diamond square loop.
            while(step_size > 1) {
                scale = map_offset * (T)step_size;  //  Adjust randomness per step.
                half_step = step_size / 2;

                //  Diamond phase.
                for(std::size_t y = 0; y < _map_side - 1; y += step_size) {
                    for(std::size_t x = 0; x < _map_side - 1; x += step_size) {
                        //  Get values from the square step.
                        cor1 = get_map_value(x, y);
                        cor2 = get_map_value(x, y + step_size);
                        cor3 = get_map_value(x + step_size, y);
                        cor4 = get_map_value(x + step_size, y + step_size);

                        //  Calculate the new value based on an average of the read values.
                        //  Also factoring in step scale.
                        new_value = static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
                        new_value = (new_value * scale * 2) / scale;
                        new_value = ( cor1 + cor2 + cor3 + cor4 + new_value ) / 5;
                        set_map_value(x + half_step, y + half_step, new_value);
                    }
                }

                //  Square phase.
                for(std::size_t y = 0; y <= _map_side - 1; y += half_step) {
                    for(std::size_t x = (y + half_step) % step_size; x <= _map_side - 1; x += step_size) {
                        //  Get values from the diamond step.
                        cor1 = get_map_value(x, y - half_step);
                        cor2 = get_map_value(x + half_step, y);
                        cor3 = get_map_value(x, y + half_step);
                        cor4 = get_map_value(x - half_step, y);

                        //  Calculate the new value based on an average of the read values.
                        //  Also factoring in step scale.
                        new_value = static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
                        new_value = (new_value * scale * 2) / scale;
                        new_value = ( cor1 + cor2 + cor3 + cor4 + new_value ) / 5;
                        set_map_value(x, y, new_value);
                    }
                }
                step_size = half_step;
            }  //  End diamond square loop.
        };

    private:
        /*
         * Initialize the object.  Constructors call this.
         * Verifies the passed size value is within range.
         * Define the height map as _map_side * _map_side
         */
        void initialize(std::size_t size) {
            static_assert(std::is_same_v<T, double> || std::is_same_v<T, long double>, "Diamond Square Type must be double or long double");
            if(size < min_size) size = min_size;
            if(size > max_size) size = max_size;
            _map_side = pow(2, size) + 1;

            hmap.clear();
            hmap.resize((_map_side * _map_side), 0.0f);
        };

        /*
         * Functions to get/set values in the map vector.
         * Called during the diamond square loop.
         */
        const T get_map_value(
            const std::size_t& x,
            const std::size_t& y
        ) const {
            return hmap[(((y + _map_side) % _map_side) * _map_side) + ((x + _map_side) % _map_side)];
        };

        void set_map_value(
            const std::size_t& x,
            const std::size_t& y,
            const T& new_value
        ) {
            hmap[(((y + _map_side) % _map_side) * _map_side) + ((x + _map_side) % _map_side)] = new_value;
        };

        height_map hmap;           //  Store the height map (vector of Ts)
        std::size_t _map_side;     //  Used for width and height of the map
        const T map_offset;        //  Store the map's offset
        const uint32_t _map_seed;  //  Seed used for random
};

}  //  end namespace mte

#endif
