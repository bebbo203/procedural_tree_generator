
#include <iostream>
#include <yocto/yocto_shape.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_sceneio.h>
#include <yocto/yocto_shape.h>
#include <math.h>
using namespace yocto::math;
using namespace yocto::shape;
using namespace yocto::common;
using namespace yocto::sceneio;





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

void sphere_try(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
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
void cylinder_try(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
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
  
  
  //sphere_try(squads, spositions, snormals, stexcoords, vec2i{steps.x,steps.y}, scale.x + R, vec2f{uvscale.x,uvscale.y}, vec3f{0,0,0});
  //
  sphere_try(squads, spositions, snormals, stexcoords, vec2i{steps.x,steps.y}, scale.x, vec2f{uvscale.x,uvscale.y}, vec3f{0,0,scale.y*2}) ;
  squads.erase(squads.begin() + squads.size()/2 , squads.end());

  for( auto i=0; i< spositions.size(); i++)
  {
    spositions[i] = transform_point(frame, spositions[i]);
    snormals[i] = transform_vector(frame, snormals[i]);
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

void quad_try(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
    std::vector<vec3f>& normals, std::vector<vec2f>& texcoords, float scale, const frame3f& frame) {
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

void create_shape(shape* shape, std::vector<vec4i>& quads, std::vector<vec3f>& positions,
                  std::vector<vec3f>& normals, std::vector<vec2f>& texcoords)
{
  
  shape->quads += quads;
  shape->positions += positions;
  shape->normals += normals;
  shape->texcoords += texcoords;
  

}

int main(void)
{
  std::cout << "Hello, yocto!\n";
  std::cout << "On blender, Y forward, Z up.\n";

  std::vector<vec4i> quads, leaf_quads, tree_quads; 
  std::vector<vec3f> positions, nodes_positions, leaf_positions, tree_positions;
  std::vector<vec3f> normals, leaf_normals, tree_normals;
  std::vector<vec2f> texcoords, leaf_texcoords, tree_texcoords;
  std::vector<vec2i> lines;
  std::vector<int> points;
  
  std::string error;
  auto tree_nodes = std::vector<int>();
  bool single_object = false;
  


  //Cloud generation
  rng_state rng = make_rng(54);
  
  auto starting_point = vec3f{0, 0, 0};
  auto initial_length = vec3f{0.0, 0.0, 1};
  std::vector<vec3f> cloud = attractors_generator(10000, -20, 20 , 8,  f, rng);
  
  nodes_positions += starting_point;
  //nodes_positions += initial_length;
  tree_nodes += 0;
  //tree_nodes += 1;

  //lines += vec2i{0, 1};

  //Tree's nodes generation
  int max_nodes = 100;
  float D = 0.2;
  float W = 0.005;
  float max_influence_sphere = 5*D;
  float max_killing_radius = 3.5*D;
  vec3f tropism = vec3f{0, 0, 0};
  
  auto branches = std::vector<int>(max_nodes+2, 0);
  branches[0] += 1;
  
  /*
  for(auto& p: cloud)
  {
    if(distance(p, vec3f{0,0,1}) <= 1)
    //if(1 > pow(p.x - 1, 2.0) + pow(p.y - 1, 2.0) + pow(p.z - 0, 2.0))
    {
      positions += p;
      points += (int)positions.size() -1;
    }
  }
  */

  
  // A loop that generates the trunk of the tree, approximating the initial value given by the user 
  // (Large approximation to avoid an infinite loop)
  float dist = 9999;
  while(dist > D/4)
  {
    auto a = initial_length;
    auto b = nodes_positions[tree_nodes.size()-1];
    auto dir = normalize(a-b);
    //std::cout << dir.x << "," << dir.y << "," << dir.z << "\n";
    auto new_node = b + dir * D;
    
    nodes_positions += new_node;
    tree_nodes += (int)nodes_positions.size()-1;
    branches[nodes_positions.size()-1] += 1;
    lines += {(int)tree_nodes.size()-2, (int)tree_nodes.size()-1};
    dist = distance(new_node, initial_length);
    //std::cout << new_node.x << "|" << new_node.y << "|" << new_node.x << " | " << dist << "\n";
  }

  auto forbidden_node = (int)tree_nodes.size() - 1;
  
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
    auto max_width = 50;
    auto t = branches_depth(lines, branches, l);
   
    width_vector[l[0]][l[1]] = t > max_width ? max_width : t;
  }


  
  
  
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


    uv_scale = {width*2*pif / 64, 1  ,1};

    cylinder_try(tree_quads, tree_positions,
    tree_normals, tree_texcoords, vec3i{8, 8, 8}, scale, uv_scale , frame, base_width * W );

    
  
  
    // Add leaves to the last branches: the majority of trees have leaves only on
    // branches that don't have sons.

    auto leaves_density_min = 8;
    auto leaves_density_max = 15;
    auto leaf_size = 0.1;
    if(width == 1)
    {
      int how_much = (leaves_density_max -  leaves_density_min) * rand1f(rng) +  leaves_density_min; 
      for(int i=0; i<how_much; i++)
      {
        auto p = (1 / (float)how_much * (i+1));
        

        auto leaf_position = x + p * (x_ - x);
        
        auto a = 1;
        auto k_ = 2;
        auto e = 2.7;
        p = 9 * pif * p ;
        float X = a * pow(e, k_*p) * yocto::math::cos(p); 
        float Y = a * pow(e, k_*p) * yocto::math::sin(p); 
      

        

        frame3f leaf_frame;
        leaf_frame = frame_fromzx(leaf_position, frame.z, vec3f{X, Y, 0});
        auto random_rotation = rotation_frame(leaf_frame.x, rand1f(rng));
        
        leaf_frame.x = transform_vector(random_rotation, leaf_frame.x);
        leaf_frame.y = transform_vector(random_rotation, leaf_frame.y);
        leaf_frame.z = transform_vector(random_rotation, leaf_frame.z);
        
        quad_try(leaf_quads, leaf_positions, leaf_normals, leaf_texcoords, leaf_size, leaf_frame);
      }


    }

  
  }
  

  if(false)
  {
    positions.clear();
    quads.clear();
    normals.clear();
    texcoords.clear();
    lines.clear();
    

    cylinder_try(quads, positions,
      normals, texcoords, vec3i{8, 8, 8}, vec2f{0.5, 1}, vec3f{0.5, 0.5, 1.0}, frame_fromz(vec3f{1,1,0}, vec3f{0,0,1}), 1 );
  }
  
  
  std::cout << "Positions: " << tree_positions.size() << "\n";
  std::cout << "Points:    " << points.size() << "\n";
  std::cout << "Lines:     " << lines.size() << "\n";
  std::cout << "Quads:     " << tree_quads.size() << "\n";
  std::cout << "Alive att :  " << cloud.size() << "\n";
  std::cout << "Tree Nodes:  " << tree_nodes.size() << "\n";

  //Scene preparation

  auto final_scene = new model();

  //Here, the output is a scene with multiple objects and a texture file for each object.
  if(single_object == false)
  {
    //Generate the shapes (only two)
    auto tree_shape = add_shape(final_scene);
    tree_shape -> name = "tree";
    create_shape(tree_shape, tree_quads, tree_positions, tree_normals, tree_texcoords);
    auto leaf_shape = add_shape(final_scene);
    leaf_shape -> name = "leaves";
    create_shape(leaf_shape, leaf_quads, leaf_positions, leaf_normals, leaf_texcoords);
    
    


    
    //Load textures
    auto tree_txt = add_texture(final_scene);
    std::string name = "resources/exports/tree.png";
    auto tree_img = load_image_to_texture(name);
    tree_txt -> colorb = tree_img;
    tree_txt -> name = "tree.png";

    

    auto leaf_txt = add_texture(final_scene);
    name = "resources/exports/leaf.png";
    auto leaf_img = load_image_to_texture(name);
    leaf_txt -> colorb = leaf_img;
    leaf_txt -> name = "leaf.png";

    auto leaf_opacity_txt = add_texture(final_scene);
    name = "resources/exports/leaf_opacity.png";
    auto leaf_opacity_img = load_scalar_image_to_texture(name);
    leaf_opacity_txt->scalarb = leaf_opacity_img;
    leaf_opacity_txt->name = "leaf_opacity.png";
    


    //Prepare materials
    auto tree_material = add_material(final_scene);
    tree_material->name = "Wood_material";
    tree_material->roughness = 1;
    tree_material->color_tex = tree_txt;
    tree_material->color = vec3f{1,1,1};
    

    auto leaf_material = add_material(final_scene);
    leaf_material->name = "Leaf_material";
    leaf_material->roughness = 1;
    leaf_material->color_tex = leaf_txt;
    leaf_material->opacity_tex = leaf_opacity_txt;



    leaf_material->color = vec3f{1,1,1};
  

    //Prepare objects
    auto tree_obj = add_object(final_scene);
    tree_obj -> name = "Tree_obj";
    tree_obj -> shape = tree_shape;
    tree_obj -> material = tree_material;
    tree_obj -> frame = frame3f{vec3f{0,0,1}, vec3f{1,0,0}, vec3f{0,1,0}, vec3f{0,0,0}};
    

    auto leaf_obj = add_object(final_scene);
    leaf_obj -> name = "Leaf_obj";
    leaf_obj -> shape = leaf_shape;
    leaf_obj -> material = leaf_material;
    leaf_obj -> frame = frame3f{vec3f{0,0,1}, vec3f{1,0,0}, vec3f{0,1,0}, vec3f{0,0,0}};
  }

  //Simple camera
  auto cam = add_camera(final_scene);
  cam->frame = frame_fromz(vec3f{-10,2,0}, vec3f{-5,0.1,0.1});
  
  //Simple environment 
  auto env = add_environment(final_scene);
  env->emission = vec3f{0.4,0.4,0.4};
  
  // If you save in json it will have an opacity texture
  bool ok = save_scene("resources/exports/test.obj", final_scene, error);
  

  

  if(error == "")
    std::cout << "You're lucky, no errors!" << "\n";
  else
    std::cout << "Save error: "<< error <<"\n";

  
}