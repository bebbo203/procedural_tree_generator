//
// # Yocto/Extension: Tiny Yocto/GL extension
//
//

//
// LICENSE:
//
// Copyright (c) 2020 -- 2020 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#ifndef _YOCTO_EXTENSION_H_
#define _YOCTO_EXTENSION_H_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include <yocto/yocto_image.h>
#include <yocto/yocto_math.h>
#include <iostream>
#include <yocto/yocto_shape.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_sceneio.h>
#include <yocto/yocto_shape.h>
#include <math.h>

#include <atomic>
#include <future>
#include <memory>

// -----------------------------------------------------------------------------
// ALIASES
// -----------------------------------------------------------------------------
namespace yocto::extension {

// Namespace aliases
namespace ext = yocto::extension;
namespace img = yocto::image;

// Math defitions
using math::bbox3f;
using math::byte;
using math::frame3f;
using math::identity3x4f;
using math::ray3f;
using math::rng_state;
using math::vec2f;
using math::vec2i;
using math::vec3b;
using math::vec3f;
using math::vec3i;
using math::vec4f;
using math::vec4i;
using math::zero2f;
using math::zero3f;

}  // namespace yocto::pathtrace

// -----------------------------------------------------------------------------
// HIGH LEVEL API
// -----------------------------------------------------------------------------
namespace yocto::extension 
{

    std::vector<int> get_influence_sphere(vec3f center, std::vector<vec3f>& attractors, float radius);

    void make_sphere_position(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
        std::vector<vec3f>& normals, std::vector<vec2f>& texcoords,
        const vec2i& steps, float scale, const vec2f& uvscale, const vec3f& p);
    
    void make_pill_frame(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
        std::vector<vec3f>& normals, std::vector<vec2f>& texcoords,
        const vec3i& steps, const vec2f& scale, const vec3f& uvscale, const frame3f& frame, const float R);
    
    void make_quad_frame(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
        std::vector<vec3f>& normals, std::vector<vec2f>& texcoords, float scale, const frame3f& frame);
    
    float branches_depth(std::vector<vec2i> lines, std::vector<int> branches, vec2i start_line);

    float f(float x);

    std::vector<vec3f> attractors_generator(int points_number, float range_min, float range_max, float z_offset, float f(float), rng_state& rng);

    yocto::image::image<vec3b> load_image_to_texture(std::string& img_path);

    yocto::image::image<byte> load_scalar_image_to_texture(std::string& img_path);

    bool create_shape(yocto::sceneio::shape* shape, std::vector<vec4i>& quads, std::vector<vec3f>& positions,
        std::vector<vec3f>& normals, std::vector<vec2f>& texcoords);

    void generate_tree(int n_attractors, int attractors_range_min, int attractors_range_max, int attractors_z_offset,
      vec3f tree_starting_point, vec3f trunk_length, int max_nodes, float D, float W, float max_width,
      float max_influence_sphere, float max_killing_radius, vec3f tropism, int leaves_density_min, int leaves_density_max,
      float leaf_size_max, float leaf_size_min, float a_spiral, float k_spiral, float e, float rounds, int leaves_textures_number,
      bool single_object, std::string single_mode_texture_path, std::string single_mode_opacity_texture_path,
      std::string multiple_mode_tree_texture_path, std::string multiple_modes_leaf_opacity_texture_path,  std::string export_name_path);
}  // namespace yocto::pathtrace

#endif
