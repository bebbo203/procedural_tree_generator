//
// Implementation for Yocto/Extension.
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

#include "yocto_extension.h"

#include <atomic>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <iostream>
#include <yocto/yocto_shape.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_sceneio.h>
#include <yocto/yocto_shape.h>
#include <math.h>
using namespace std::string_literals;
using namespace yocto::math;
using namespace yocto::shape;
using namespace yocto::common;
using namespace yocto::sceneio;

// -----------------------------------------------------------------------------
// MATH FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto::extension {

// import math symbols for use
using math::abs;
using math::acos;
using math::atan2;
using math::clamp;
using math::cos;
using math::exp;
using math::flt_max;
using math::fmod;
using math::fresnel_conductor;
using math::fresnel_dielectric;
using math::identity3x3f;
using math::invalidb3f;
using math::log;
using math::make_rng;
using math::max;
using math::min;
using math::pif;
using math::pow;
using math::sample_discrete_cdf;
using math::sample_discrete_cdf_pdf;
using math::sample_uniform;
using math::sample_uniform_pdf;
using math::sin;
using math::sqrt;
using math::zero2f;
using math::zero2i;
using math::zero3f;
using math::zero3i;
using math::zero4f;
using math::zero4i;

}  // namespace yocto::pathtrace

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR EXTENSION
// -----------------------------------------------------------------------------
namespace yocto::extension 
{
    std::vector<int> get_influence_sphere(vec3f center, std::vector<vec3f>& attractors, float radius)
    {
        auto influencers = std::vector<int>();

        for(int i=0; i<attractors.size(); i++)
        { 
            auto p = attractors[i];

            //std::cout << pow(p.x - center.x, 2.0) + pow(p.y - center.y, 2.0) + pow(p.z - center.z, 2.0) << " | " << distance(center, p) << " ---> " << radius << "\n";
            
            if(distance(center, p) < radius)
            {
            influencers += i;  
            }
        }

        return influencers;
        }

    void make_sphere_position(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
        std::vector<vec3f>& normals, std::vector<vec2f>& texcoords,
        const vec2i& steps, float scale, const vec2f& uvscale, const vec3f& p) 
    {
        auto qquads     = std::vector<vec4i>{};
        auto qpositions = std::vector<vec3f>{};
        auto qnormals   = std::vector<vec3f>{};
        auto qtexcoords = std::vector<vec2f>{};
        make_rect(qquads, qpositions, qnormals, qtexcoords, steps, {1, 1}, {1, 1});
        for (auto i = 0; i < qpositions.size(); i++) 
        {
            auto uv      = qtexcoords[i];
            auto a       = vec2f{2 * pif * uv.x, pif * (1 - uv.y)};
            qpositions[i] = vec3f{yocto::math::cos(a.x) * yocto::math::sin(a.y), yocto::math::sin(a.x) * yocto::math::sin(a.y), yocto::math::cos(a.y)} *
                        scale;
            qnormals[i]   = normalize(qpositions[i]);
            qtexcoords[i] = uv * uvscale;
            qpositions[i] += p;
        }

        merge_quads(quads, positions, normals, texcoords, qquads, qpositions,
            qnormals, qtexcoords);
        }


        // R è di quanto la base deve essere più grande 
    void make_pill_frame(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
            std::vector<vec3f>& normals, std::vector<vec2f>& texcoords,
            const vec3i& steps, const vec2f& scale, const vec3f& uvscale, const frame3f& frame, const float R=0) {
        auto qquads     = std::vector<vec4i>{};
        auto qpositions = std::vector<vec3f>{};
        auto qnormals   = std::vector<vec3f>{};
        auto qtexcoords = std::vector<vec2f>{};
        auto squads     = std::vector<vec4i>{};
        auto spositions = std::vector<vec3f>{};
        auto snormals   = std::vector<vec3f>{};
        auto stexcoords = std::vector<vec2f>{};

        // side
        make_rect(qquads, qpositions, qnormals, qtexcoords, {steps.x, steps.y},
            {1, 1}, {1, 1});
        
        
        //make_sphere_position(squads, spositions, snormals, stexcoords, vec2i{steps.x,steps.y}, scale.x + R, vec2f{uvscale.x,uvscale.y}, vec3f{0,0,0});
        //
        make_sphere_position(squads, spositions, snormals, stexcoords, vec2i{steps.x,steps.y}, scale.x, vec2f{uvscale.x,uvscale.y}, vec3f{0,0,scale.y*2}) ;
        squads.erase(squads.begin() + squads.size()/2 , squads.end());


        for( auto i=0; i< spositions.size(); i++)
        {
            spositions[i] = transform_point(frame, spositions[i]);
            snormals[i] = transform_vector(frame, -snormals[i]);
        }

        for (auto i = 0; i < qpositions.size(); i++) 
        {
            auto uv       = qtexcoords[i];
            auto phi      = 2 * pif * uv.x;
            
            qpositions[i] = {
                yocto::math::cos(phi) * scale.x, yocto::math::sin(phi) * scale.x, (2 * uv.y - 1) * scale.y};

            auto norm_z = yocto::math::abs((qpositions[i].z - scale.y));
            auto s = (R/2 * norm_z) / scale.y;
            
            qpositions[i].x = yocto::math::cos(phi) * (scale.x + s);
            qpositions[i].y = yocto::math::sin(phi) * (scale.x + s);
            

            qnormals[i]   = {-yocto::math::cos(phi), -yocto::math::sin(phi), 0};
            qtexcoords[i] = uv * vec2f{uvscale.x, uvscale.y,} ;

            qpositions[i].z += scale.y;
            qpositions[i] = transform_point(frame, qpositions[i]);
            qnormals[i] = transform_vector(frame, qnormals[i]); 
        }

        
        merge_quads(quads, positions, normals, texcoords, qquads, qpositions,
            qnormals, qtexcoords);
        merge_quads(quads, positions, normals, texcoords, squads, spositions,
            snormals, stexcoords);
    }

    void make_quad_frame(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
        std::vector<vec3f>& normals, std::vector<vec2f>& texcoords, float scale, const frame3f& frame) 
    {
        auto quad_positions = std::vector<vec3f>{
            {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
        auto quad_normals = std::vector<vec3f>{
            {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
        auto quad_texcoords = std::vector<vec2f>{
            {0, 0}, {1, 0}, {1, 1}, {0, 1}};
            
        auto quad_quads = std::vector<vec4i>{{0, 1, 2, 3}};


        for(auto i=0; i<quad_positions.size(); i++)
        {
            quad_positions[i] *= scale;
            quad_positions[i] = transform_point(frame, quad_positions[i]);
            quad_normals[i] = transform_vector(frame, quad_normals[i]);
            
        }

        merge_quads(quads, positions, normals, texcoords, quad_quads, quad_positions,
            quad_normals, quad_texcoords);
    }



    float branches_depth(std::vector<vec2i> lines, std::vector<int> branches, vec2i start_line)
    {
        float acc = 0;
        auto actual = std::vector<vec2i>();
        actual += start_line;
        int node;
        float tot_acc = 0.0;
        double n = 2.05;

        // If the branch continues without subdividing in little branches 
        // the width is the same as the one of the following branches
        if(branches[start_line.y] == 1)
        {
            auto succ = -1;
            // Find the end of the next branch
            for(int i=0; i<lines.size(); i++)
            {
            if(lines[i].x == start_line.y)
            {
                succ = lines[i].y;
                break;
            }
            }

            if(succ == -1)
            return 1;
            
            // Recursive call to find the width of the succesive branch
            return branches_depth(lines, branches, vec2i{start_line.y, succ});
        }
        else
        {
            while(!actual.empty())
            {
            
            node = actual[0].y;
            // Find all the branches that starts from node 
            for(int i=0; i<lines.size(); i++)
            {
                // If this is the line that starts from node
                if(lines[i].x == node)
                {
                // If this line generates at least one branch
                if(branches[lines[i].y] > 0 )
                {
                    // At the nex iteration continue with this path
                    actual += lines[i];
                    acc += pow((float)branches[lines[i].y], n);
                }
                }

            }

            tot_acc += acc; 
            acc = 0;
            actual.erase(actual.begin());

            } 
        }
        
        // Maybe do I need to n-root when I accumulate?
        tot_acc = pow(tot_acc, 1/n);
        return tot_acc == 0 ? 1 : tot_acc;

    }

    float f(float x)
    {
        return  -1 * x * x + 3 * x + 0.3;
    }

    std::vector<vec3f> attractors_generator(int points_number, float range_min, float range_max, float z_offset, float f(float), rng_state& rng)
    {
        std::vector<vec3f> cloud;
        
        for(int i=0; i<points_number; i++)
        {
            auto p = (range_max - range_min) * rand3f(rng) + range_min;
            
            if(distance(p, vec3f{0,0,0}) < 8)
            //if(pow(p.x*p.x + p.y*p.y, 0.5) < f(p.z))
            //if(10 > pow(p.x - 0, 2.0) + pow(p.y, 2.0) && p.z > 1 && p.z < 8)
            {
            p.z += z_offset;
            cloud += p;
            }
            else
            i -= 1;   
        }

        return cloud;
    }

    yocto::image::image<vec3b> load_image_to_texture(std::string& img_path)
    {
        std::string error;
        auto img = yocto::image::image<vec3b>();
        if(!yocto::image::load_image(img_path, img, error))
            std::cout << "image load error: " << error << "\n";
        return img;
    }

    yocto::image::image<byte> load_scalar_image_to_texture(std::string& img_path)
    {
        std::string error;
        auto img = yocto::image::image<byte>();
        if(!yocto::image::load_image(img_path, img, error))
            std::cout << "image load error: " << error << "\n";
        return img;
    }

    bool create_shape(yocto::sceneio::shape* shape, std::vector<vec4i>& quads, std::vector<vec3f>& positions,
         std::vector<vec3f>& normals, std::vector<vec2f>& texcoords)
    { 
        shape->quads += quads;
        shape->positions += positions;
        shape->normals += normals;
        shape->texcoords += texcoords;

        return true;
    }



}  // namespace yocto::pathtrace
