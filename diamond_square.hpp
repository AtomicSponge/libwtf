/*
 * Height Map Generator using Diamond Square
 * By:  Matthew Evans
 * File:  diamond_square.hpp
 * Version:  090820
 *
 * See LICENSE.md for copyright information.
 *
 * Implementation of the diamond square algorithm as a C++ class.
 * Creates a vector of doubles that can then be used as a height map.
 * 
 * https://en.wikipedia.org/wiki/Diamond-square_algorithm
 * 
 * Example:
 * 
 * diamond_square my_map = diamond_square(8, 0.096f);  //  Initialize object
 * my_map.build_map();  //  Generate the map
 * height_map some_map = my_map.get_map();  //  Do something with the map
 * 
 */

#ifndef MTE_DIAMOND_SQUARE_HPP
#define MTE_DIAMOND_SQUARE_HPP

#include <vector>
#include <ctime>
#include <cmath>
#include <cassert>

namespace mte {

//  Define a container for storing height maps
typedef std::vector<double> height_map;

class diamond_square {
    public:
        /*
         * Initialize the diamond square object.
         * 
         * size = Size value to generate map with (2 - 14)
         * offset = Offset value - higher value for more even terrain
         * seed = Seed for random
         */
        diamond_square(
            const std::size_t& size,
            const float& offset,
            const unsigned int& seed
        ) : map_offset(offset), map_seed(seed), map_side(0) {
            initialize(size);
        };

        /*
         * Initialize the diamond square object.  Use time as seed.
         * 
         * size = Size value to generate map with (2 - 14)
         * offset = Offset value - higher value for more even terrain
         */
        diamond_square(
            const std::size_t& size,
            const float& offset
        ) : map_offset(offset), map_seed(std::time(nullptr)), map_side(0) {
            initialize(size);
        };

        ~diamond_square() = default;

        /*
         * Return the height map vector.
         */
        const height_map get_map(void) const { return hmap; };

        /*
         * Return a single value in the height map.
         * Performs assertion on position value.
         * 
         * pos = Position to get value for.
         */
        const double get_value(const std::size_t& pos) const {
            assert(pos < hmap.size());
            return hmap[pos];
        };

        /*
         * Get the side value of the map.
         */
        const int get_side(void) const {
            return map_side;
        };

        /*
         * Get the map's seed.
         */
        const unsigned int get_seed(void) const {
            return map_seed;
        };

        /*
         * Build the height map using the power of diamond square!
         * Call this after declaring the object to build the actual map.
         */
        void build_map(void) {
            std::srand(map_seed);  //  Set seed.

            //  Set the initial values in the four corners of the map.
            //  Also counts as the first square step.
            hmap[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / map_offset;
            hmap[get_side() - 1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / map_offset;
            hmap[(get_side() * get_side()) - get_side()] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / map_offset;
            hmap[(get_side() * get_side()) - 1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / map_offset;

            std::size_t step_size, half_step;  //  For storing our step sizes.
            double cor1, cor2, cor3, cor4, new_value, scale;  //  For storing our results.

            //  Set our step size and start the loop.
            step_size = get_side() - 1;

            //  Diamond square loop.
            while(step_size > 1) {
                scale = map_offset * (double)step_size;  //  Adjust randomness per step.
                half_step = step_size / 2;

                //  Diamond phase.
                for(std::size_t y = 0; y < get_side() - 1; y += step_size) {
                    for(std::size_t x = 0; x < get_side() - 1; x += step_size) {
                        //  Get values from the square step.
                        cor1 = get_map_value(x, y);
                        cor2 = get_map_value(x, y + step_size);
                        cor3 = get_map_value(x + step_size, y);
                        cor4 = get_map_value(x + step_size, y + step_size);

                        //  Calculate the new value based on an average of the read values.
                        //  Also factoring in step scale.
                        new_value = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
                        new_value = (new_value * scale * 2) / scale;
                        new_value = ( cor1 + cor2 + cor3 + cor4 + new_value ) / 5;
                        set_map_value(x + half_step, y + half_step, new_value);
                    }
                }

                //  Square phase.
                for(std::size_t y = 0; y <= get_side() - 1; y += half_step) {
                    for(std::size_t x = (y + half_step) % step_size; x <= get_side() - 1; x += step_size) {
                        //  Get values from the diamond step.
                        cor1 = get_map_value(x, y - half_step);
                        cor2 = get_map_value(x + half_step, y);
                        cor3 = get_map_value(x, y + half_step);
                        cor4 = get_map_value(x - half_step, y);

                        //  Calculate the new value based on an average of the read values.
                        //  Also factoring in step scale.
                        new_value = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
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
         * Define the height map as map_side * map_side
         */
        void initialize(std::size_t size) {
            if(size < min_size) size = min_size;
            if(size > max_size) size = max_size;
            map_side = pow(2, size) + 1;

            hmap.clear();
            hmap.resize((map_side * map_side), 0.0f);
        };

        /*
         * Functions to get/set values in the map vector.
         * Called during the diamond square loop.
         */
        inline const double get_map_value(
            const std::size_t& x,
            const std::size_t& y
        ) const {
            return hmap[(((y + get_side()) % get_side()) * get_side()) + ((x + get_side()) % get_side())];
        };

        inline void set_map_value(
            const std::size_t& x,
            const std::size_t& y,
            const double& new_value
        ) {
            hmap[(((y + get_side()) % get_side()) * get_side()) + ((x + get_side()) % get_side())] = new_value;
        };

        /*
         * Minimum and maximum size of the map.
         * The map side is calculated:  (2 ^ size) + 1
         */
        const std::size_t min_size = 2;
        const std::size_t max_size = 14;

        height_map hmap;            //  Store the height map (vector of doubles)
        int map_side;               //  Used for width and height of the map
        double map_offset;          //  Store the map's offset
        unsigned int map_seed;      //  Seed used for random
};

}  //  end namespace mte

#endif
