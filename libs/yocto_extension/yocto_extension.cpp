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
#include <ctime>
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

    std::vector<vec3f> attractors_generator(int points_number, float range_min, float range_max, float z_offset, float f(float), rng_state& rng, int f_selector)
    {
        std::vector<vec3f> cloud;
        
        for(int i=0; i<points_number; i++)
        {
            auto p = (range_max - range_min) * rand3f(rng) + range_min;
            

            if(f_selector == 0)
            {
                if(distance(p, zero3f) < range_max/2)
                {
                    p.z += z_offset;
                    cloud += p;
                }
                else
                    i -= 1; 
            }
            else if (f_selector == 1)
            {
                if(pow(p.x*p.x + p.y*p.y, 0.5) < f(p.z))
                {
                    p.z += z_offset;
                    cloud += p;
                }
                else
                    i -= 1; 
            }
            else if (f_selector == 2)
            {
                if(p.z > 0 && p.z < -sqrt(pow(p.x, 2.0) + pow(p.y, 2.0)) * 2 + z_offset)
                {          
                    p.z += 2;
                    cloud += p;
                    
                }
                else
                    i -= 1; 
            }
            else if (f_selector == 3)
            {
                if(p.z > 0 && p.x > -0.5 && p.x < 0.5)
                {          
                    cloud += p;
                }
                else
                    i -= 1; 
            }
            
        }

        return cloud;
    }

    yocto::image::image<vec3b> load_image_to_texture(std::string& img_path)
    {
        std::string error;
        auto img = yocto::image::image<vec3b>();
        if(!yocto::image::load_image(img_path, img, error))
        {
          std::cout << "\nimage load error: " << error << "\n";
          exit(1);
        }
        return img;
    }

    yocto::image::image<byte> load_scalar_image_to_texture(std::string& img_path)
    {
        std::string error;
        auto img = yocto::image::image<byte>();
        if(!yocto::image::load_image(img_path, img, error))
        {    
            std::cout << "\nimage load error: " << error << "\n";
            exit(1);
        }
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

    void generate_tree(int n_attractors, int attractors_range_min, int attractors_range_max, int attractors_z_offset,
      vec3f tree_starting_point, vec3f trunk_length, int max_nodes, float D, float W, float max_width,
      float max_influence_sphere, float max_killing_radius, vec3f tropism, int leaves_density_min, int leaves_density_max,
      float leaf_size_max, float leaf_size_min, float a_spiral, float k_spiral, float e, float rounds, int leaves_textures_number,
      int single_object, std::string single_mode_texture_path, std::string single_mode_opacity_texture_path,
      std::string multiple_mode_tree_texture_path, std::string multiple_modes_leaf_texture_path, 
      std::string multiple_modes_leaf_opacity_texture_path,  std::string export_name_path, int f_selector)
    {
        
        auto START_TIME = clock();
        std::vector<vec4i> quads, leaf_quads, tree_quads; 
        std::vector<vec3f> positions, nodes_positions, leaf_positions, tree_positions;
        std::vector<vec3f> normals, leaf_normals, tree_normals;
        std::vector<vec2f> texcoords, leaf_texcoords, tree_texcoords;
        std::vector<vec2i> lines;
        std::vector<int> points;
        /*
        // ############# PARAMETERS #############
            // SHAPE PARAMETERS
                // Number of points to be included in the general shape
                int n_attractors = 10000;
                // Dimensions of the cube in which n_points attractors will be generated
                int attractors_range_min = -20;
                int attractors_range_max = 20;
                // Offset in the Z direction of the initial cube of points
                int attractors_z_offset = 8;
            // TREE PARAMETERS
                // Point in wich the tree will start to grow
                vec3f tree_starting_point = vec3f{0, 0, 0};
                // Approximated point in wich the trunk will start to grow branches (trunk length)
                vec3f trunk_length = vec3f{0.0, 0.0, 1};
                // Max number of tree nodes
                int max_nodes = 100; 
                // Length of a generic branch
                float D = 0.2;
                // Min width of a branch
                float W = 0.005;
                // Max width of a branch, 50 is a good value (not related to W, it's just a constant that will be multiplied by W)
                float max_width = 50;
                // Attractor inluence shpere
                float max_influence_sphere = 5*D;
                // Attractor kill distance from a branch
                float max_killing_radius = 3.5*D;
                //Eventual tropism (deformation or gravity) of a branch
                vec3f tropism = vec3f{0, 0, 0};
            // LEAVES PARAMETERS
                // Min number of leaves in a single branch of lenght D
                int leaves_density_min = 8;
                // Max number of leaves in a single branch of length D
                int leaves_density_max = 15;
                // Max size of a leaf
                float leaf_size_max = 0.1;
                // Min size of a leaf
                float leaf_size_min = 0.05;
                // The following are 4 parameters of the logaritmic spiral used for distributing the leaves. 
                float a_spiral = 1;
                float k_spiral = 2;
                float e = 2.7;
                float rounds = 9;
            // Texture parameters
                // Number of differents leaves
                int leaves_textures_number = 2;
                // For the texture, if you are using single object mode, don't worry. 
                // If you are using the multiple object mode, call the leaf texture with a index that start from 0 at the end.
                // For example:
                    // leaf_texture_0.png
                    // leaf_opacity_texture_0.png
                    // leaf_texture_1.png
                    // leaf_opacity_texture_1.png
                // And pass the path WITHOUT numbers and extension (hopefully a png)
                    // leaf_texture_
                    // leaf_opacity_texture_
                // Single object mode texture
                std::string single_mode_texture_path = "resources/exports/tree.png";
                std::string single_mode_opacity_texture_path = "resources/exports/tree_opacity.png";
                // Multiple object mode textures
                std::string multiple_mode_tree_texture_path = "resources/exports/wood.png";
                std::string multiple_modes_leaf_texture_path = "resources/exports/leaf_";
                std::string multiple_modes_leaf_opacity_texture_path = "resources/exports/leaf_opacity_";
                //Remember to have a shapes and textures folder!
                std::string export_name_path = "resources/exports/albero.json";

        // ######################################
        */
        std::string error;
        auto tree_nodes = std::vector<int>();
        
        
        


        //Cloud generation
        rng_state rng = make_rng(54);
        std::cout << "Generating attractors... ";
        std::vector<vec3f> cloud = attractors_generator(n_attractors, attractors_range_min, attractors_range_max , attractors_z_offset,  f, rng, f_selector);
        std::cout << (clock() - START_TIME) / (double)CLOCKS_PER_SEC << "s\n";
        

        nodes_positions += tree_starting_point;
        tree_nodes += 0;

        //Tree's nodes generation
        auto branches = std::vector<int>(max_nodes+2, 0);
        branches[0] += 1;
        
        // A loop that generates the trunk of the tree, approximating the initial value given by the user 
        // (Large approximation to avoid an infinite loop)
        float dist = D * 2;
        while(dist > D)
        {
            auto a = trunk_length;
            auto b = nodes_positions[tree_nodes.size()-1];
            auto dir = normalize(a-b);
            
            auto new_node = b + dir * D;
            
            nodes_positions += new_node;
            tree_nodes += (int)nodes_positions.size()-1;
            branches[nodes_positions.size()-1] += 1;
            lines += {(int)tree_nodes.size()-2, (int)tree_nodes.size()-1};
            dist = distance(new_node, trunk_length);
            
        }

        auto forbidden_node = (int)tree_nodes.size() - 1;
        auto GROW_TIME = clock();
        std::cout << "Growing the tree... ";
        while(tree_nodes.size() < max_nodes)
        {
            
            auto nodes_to_be_added = std::vector<int>();
            for(auto node: tree_nodes)
            {
            
                if(node < forbidden_node || branches[node] >= 3)
                    continue;
                auto new_cloud = std::vector<vec3f>();
                auto v = nodes_positions[node];

                auto attractors = get_influence_sphere(v, cloud, max_influence_sphere);
                
                if(attractors.empty())
                    continue;

                branches[node] += 1;
                auto total_dir = zero3f;
                for(auto a: attractors)
                {
                    auto s = cloud[a];
                    total_dir += normalize(s - v);
                } 
                total_dir = normalize(total_dir + tropism);
                
                auto v_prime = v + total_dir * D;
                
                nodes_positions += v_prime;
                int new_node = nodes_positions.size()-1;

                lines += vec2i{node, new_node};
                nodes_to_be_added += new_node;

                if(nodes_to_be_added.size() + tree_nodes.size() >= max_nodes)
                    break;

                auto dead_attractors = get_influence_sphere(v_prime, cloud, max_killing_radius);

                for(int i=0; i<cloud.size(); i++)
                {
                    auto j = std::find(dead_attractors.begin(), dead_attractors.end(), i);
                    if(j == dead_attractors.end())
                    {
                    new_cloud += cloud[i];
                    }
                }
                
                cloud = new_cloud;
            }

            if(nodes_to_be_added.empty())
                break;
            else
            {
                nodes_to_be_added += tree_nodes;
                tree_nodes = nodes_to_be_added;
            } 
        }
        
        
        //Matrix for keeping track of the width of a segment
        auto width_vector = std::vector<std::vector<float>>(tree_nodes.size());
        for(auto& r: width_vector)
            r = std::vector<float>(tree_nodes.size(), -1);

        for(auto l: lines)
        {
            auto t = branches_depth(lines, branches, l);
        
            width_vector[l[0]][l[1]] = t > max_width ? max_width : t;
        }
        std::cout << (clock() - GROW_TIME) / (double)CLOCKS_PER_SEC << "s\n";
        auto MESH_TIME = clock();
        std::cout << "Building the meshes... ";
        frame3f frame;
        for(auto l: lines)
        {
            auto x = nodes_positions[l[0]];
            auto x_ = nodes_positions[l[1]];

            auto length = distance(x_, x);
            
            auto width = width_vector[l[0]][l[1]];
            frame = frame_fromz(x, normalize(x_ - x));

            auto base_width = width;
            for(int i=0; i < tree_nodes.size(); i++)
            {
                if(width_vector[i][l[0]] != -1)
                {
                    base_width = width_vector[i][l[0]];
                    break;
                }
            }

            //Used to mantain a certain armony within the small branches
            auto max_base_width = width * 3;

            base_width = (base_width - width);

            if(base_width > max_base_width)
            base_width = max_base_width;

            vec2f scale = vec2f{W * width, length/2};
            vec3f uv_scale = vec3f{W*width, length/2, 1};
            uv_scale = {width*pif / 32, 1  ,1};

            make_pill_frame(tree_quads, tree_positions,
            tree_normals, tree_texcoords, vec3i{8, 8, 8}, scale, uv_scale , frame, base_width * W );
        
            // Add leaves to the last branches: the majority of trees have leaves only on
            // branches that don't have sons.
            if(leaves_density_max != 0 && width == 1)
            {
                int how_much = (leaves_density_max -  leaves_density_min) * rand1f(rng) +  leaves_density_min; 
                for(int i=0; i<how_much; i++)
                {
                    auto p = (1 / (float)how_much * (i+1));
                    

                    auto leaf_position = x + p * (x_ - x);
                    
                    
                    p = rounds * pif * p ;
                    float X = a_spiral * pow((double)e, k_spiral*p) * yocto::math::cos(p); 
                    float Y = a_spiral * pow((double)e, k_spiral*p) * yocto::math::sin(p); 
                
                    auto leaf_size = (leaf_size_max - leaf_size_min) * rand1f(rng) + leaf_size_min;
                    
                    frame3f leaf_frame;
                    leaf_frame = frame_fromzx(leaf_position, frame.z, vec3f{X, Y, 0});
                    auto random_rotation = rotation_frame(leaf_frame.x, rand1f(rng));
                    
                    leaf_frame.x = transform_vector(random_rotation, leaf_frame.x);
                    leaf_frame.y = transform_vector(random_rotation, leaf_frame.y);
                    leaf_frame.z = transform_vector(random_rotation, leaf_frame.z);
                    
                    make_quad_frame(leaf_quads, leaf_positions, leaf_normals, leaf_texcoords, leaf_size, leaf_frame);   
                }
            }
        }
        
        
        auto leaf_positions_array = std::vector<std::vector<vec3f>>(leaves_textures_number);
        auto leaf_quads_array = std::vector<std::vector<vec4i>>(leaves_textures_number);
        auto leaf_normals_array = std::vector<std::vector<vec3f>>(leaves_textures_number);
        auto leaf_texcoords_array = std::vector<std::vector<vec2f>>(leaves_textures_number);
        if(single_object)
        {
            //UV coordinates need to be changed: 

            //Tree cordinates change: just divide by 2.
            //Maybe some precision error? With this margin it seems to work fine so...
            float max = 0;
            for(auto& uv: tree_texcoords)
            {
                uv.y /= 2.01;
                uv.y += 1e-3;
                max = uv.x > max ? uv.x : max;
            }

            for(auto& uv: tree_texcoords)
            {
                uv.x /= (max*2.01); 
                uv.x += 1e-3;
            }

            //For the leaves we just divide the texcoords and randomly move the texture along the big texture
            for(auto i=0; i<leaf_texcoords.size(); i+=4)
            {
                auto extraction = rand1f(rng) * (leaves_textures_number-1);
                auto where = round(extraction);
                for(auto j=0; j<4; j++)
                {
                    leaf_texcoords[i+j] /= 2;
                    
                    if(where == 0)
                    {
                        leaf_texcoords[i+j].x += 0.5;
                    }
                    else if (where == 1)
                    {
                        leaf_texcoords[i+j].x += 0.5;
                        leaf_texcoords[i+j].y += 0.5;
                    }
                    else if (where == 2)
                    {
                        leaf_texcoords[i+j].y += 0.5;
                    }
                    
                    
                    
                }
            }
        }
        else
        {
            //Nothing to do for the trunk but the leaves need to be subdivided in others shapes
            
            
            auto quads_counter = 0;
            for(int i=0; i<leaf_positions.size(); i+=4)
            {
                auto extraction = rand1f(rng) * (leaves_textures_number-1);
                auto where = round(extraction);
                
                for(int j=0; j<4; j++)
                {
                    leaf_positions_array[where] += leaf_positions[i+j];
                    leaf_normals_array[where] += leaf_normals[i+j];
                    leaf_texcoords_array[where] += leaf_texcoords[i+j];
                }
                
                int q = leaf_positions_array[where].size();
                leaf_quads_array[where] += vec4i{q-4, q-3, q-2, q-1};
            }

        
        }
        
        
        std::cout << (clock() - MESH_TIME) / (double)CLOCKS_PER_SEC << "s\n";
        std::cout << "Positions: " << tree_positions.size() << "\n";
        std::cout << "Points:    " << points.size() << "\n";
        std::cout << "Lines:     " << lines.size() << "\n";
        std::cout << "Quads:     " << tree_quads.size() << "\n";
        std::cout << "Alive att :  " << cloud.size() << "\n";
        std::cout << "Tree Nodes:  " << tree_nodes.size() << "\n";

        //Scene preparation

        auto final_scene = new model();
        auto SAVE_TIME = clock();
        std::cout << "Preparing and saving the models... ";
        //Here, the output is a scene with multiple objects and a texture file for each object.
        if(single_object == 0)
        {
            //Generate the tree shape
            auto tree_shape = add_shape(final_scene);
            tree_shape -> name = "tree";
            create_shape(tree_shape, tree_quads, tree_positions, tree_normals, tree_texcoords);
            
            //One shape for every set of leaves
            auto leaf_shape_array = std::vector<yocto::sceneio::shape*>(leaves_textures_number);
            for(int i=0; i<leaves_textures_number; i++)
            {
                leaf_shape_array[i] = add_shape(final_scene);
                std::string shape_name = "leaf_shape_"+std::to_string(i);
                leaf_shape_array[i] -> name = shape_name;
                create_shape(leaf_shape_array[i], leaf_quads_array[i], leaf_positions_array[i], leaf_normals_array[i], leaf_texcoords_array[i]);
            }
            


            
            //Load textures
            auto tree_txt = add_texture(final_scene);

            auto tree_img = load_image_to_texture(multiple_mode_tree_texture_path);
            tree_txt -> colorb = tree_img;
            tree_txt -> name = "tree";

            auto leaf_txt_array = std::vector<yocto::sceneio::texture*>(leaves_textures_number*2);
            auto txt_cnt = 0;
            for(int i=0; i<leaves_textures_number; i++)
            {
                leaf_txt_array[txt_cnt] = add_texture(final_scene);
                std::string txt_name = multiple_modes_leaf_texture_path+std::to_string(i)+".png";
                auto leaf_img = load_image_to_texture(txt_name);
                leaf_txt_array[txt_cnt] -> colorb = leaf_img;
                leaf_txt_array[txt_cnt] -> name = "leaf_txt_"+std::to_string(i);

                leaf_txt_array[txt_cnt+1] = add_texture(final_scene);
                std::string txt_opacity_name = multiple_modes_leaf_opacity_texture_path+std::to_string(i)+".png";
                auto leaf_opacity_img = load_scalar_image_to_texture(txt_opacity_name);
                leaf_txt_array[txt_cnt+1] ->scalarb = leaf_opacity_img;
                leaf_txt_array[txt_cnt+1] ->name = "leaf_opacity_txt_"+std::to_string(i);

                txt_cnt += 2;
            }



            //Prepare materials
            auto tree_material = add_material(final_scene);
            tree_material->name = "Wood_material";
            tree_material->roughness = 1;
            tree_material->color_tex = tree_txt;
            tree_material->color = vec3f{1,1,1};
            
            txt_cnt = 0;
            auto leaf_material_array = std::vector<yocto::sceneio::material*>(leaves_textures_number);
            for(int i=0; i<leaves_textures_number; i++)
            {
                leaf_material_array[i] = add_material(final_scene);
                leaf_material_array[i]->name = "Leaf_material_"+std::to_string(i);
                leaf_material_array[i]->roughness = 1;
                leaf_material_array[i]->color_tex = leaf_txt_array[txt_cnt];
                leaf_material_array[i]->opacity_tex = leaf_txt_array[txt_cnt+1];
                leaf_material_array[i]->color = vec3f{1,1,1};
                
                txt_cnt += 2;
            }


            //Prepare objects
            auto tree_obj = add_object(final_scene);
            tree_obj -> name = "Tree_obj";
            tree_obj -> shape = tree_shape;
            tree_obj -> material = tree_material;
            tree_obj -> frame = frame3f{vec3f{0,0,1}, vec3f{1,0,0}, vec3f{0,1,0}, vec3f{0,0,0}};
            
            auto leaf_obj_array = std::vector<yocto::sceneio::object*>(leaves_textures_number);
            for(int i=0; i<leaves_textures_number; i++)
            {
                leaf_obj_array[i] = add_object(final_scene);
                leaf_obj_array[i] -> name = "Leaf_obj_"+std::to_string(i);
                leaf_obj_array[i] -> shape = leaf_shape_array[i];
                
                leaf_obj_array[i] -> material = leaf_material_array[i];
                leaf_obj_array[i] -> frame = frame3f{vec3f{0,0,1}, vec3f{1,0,0}, vec3f{0,1,0}, vec3f{0,0,0}};
            }
        }
        else
        {
            //Here we have a single object with one texture. 
            
            merge_quads(tree_quads, tree_positions, tree_normals, tree_texcoords, leaf_quads, leaf_positions, leaf_normals, leaf_texcoords);

            auto tree_shape = add_shape(final_scene);
            tree_shape -> name = "tree";
            create_shape(tree_shape, tree_quads, tree_positions, tree_normals, tree_texcoords);

            auto tree_txt = add_texture(final_scene);
            
            auto tree_img = load_image_to_texture(single_mode_texture_path);
            tree_txt -> colorb = tree_img;
            tree_txt -> name = "tree";

            auto tree_opacity_txt = add_texture(final_scene);
            auto tree_opacity_img = load_scalar_image_to_texture(single_mode_opacity_texture_path);
            tree_opacity_txt -> scalarb = tree_opacity_img;
            tree_opacity_txt -> name = "tree_opacity";

            auto tree_material = add_material(final_scene);
            tree_material->name = "Tree_material";
            tree_material->roughness = 1;
            tree_material->color_tex = tree_txt;
            tree_material->opacity_tex = tree_opacity_txt;
            
            tree_material->color = vec3f{1,1,1};

            auto tree_obj = add_object(final_scene);
            tree_obj -> name = "Tree_obj";
            tree_obj -> shape = tree_shape;
            tree_obj -> material = tree_material;
            tree_obj -> frame = frame3f{vec3f{0,0,1}, vec3f{1,0,0}, vec3f{0,1,0}, vec3f{0,0,0}};

        }
        

        //Simple camera
        auto cam = add_camera(final_scene);
        cam->frame = frame3f{identity3x3f, vec3f{0,2,15}};
        
        
        
        //Simple environment 
        auto env = add_environment(final_scene);
        env->emission = vec3f{0.4,0.4,0.4};
        
        // If you save in json it will have an opacity texture
        bool ok = save_scene(export_name_path, final_scene, error);
        std::cout << (clock() - SAVE_TIME) / (double)CLOCKS_PER_SEC << "s\n";


        std::cout << "Total elapsed time: " << (clock() - START_TIME) / (double)CLOCKS_PER_SEC << "s\n";
        

        if(error == "")
            std::cout << "No errors!" << "\n";
        else
            std::cout << "Save error: "<< error <<"\n";

    }



}  // namespace yocto::pathtrace
