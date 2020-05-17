
#include <iostream>
#include <yocto/yocto_shape.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_math.h>
#include <math.h>
using namespace yocto::math;
using namespace yocto::shape;
using namespace yocto::common;





std::vector<int> get_influence_sphere(vec3f center, std::vector<vec3f>& attractors, float radius)
{
  auto influencers = std::vector<int>();

  for(int i=0; i<attractors.size(); i++)
  { 
    auto p = attractors[i];
    if(radius > pow(p.x - center.x, 2.0)+ pow(p.y - center.y, 2.0) + pow(p.z - center.z, 2.0))
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
  
  
  sphere_try(squads, spositions, snormals, stexcoords, vec2i{steps.x,steps.y}, scale.x + R, vec2f{uvscale.x,uvscale.y}, vec3f{0,0,0});
  squads.erase(squads.begin(), squads.begin() + squads.size()/2);
  sphere_try(squads, spositions, snormals, stexcoords, vec2i{steps.x,steps.y}, scale.x, vec2f{uvscale.x,uvscale.y}, vec3f{0,0,scale.y*2}) ;

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
      {0.5, 0.5}, {1, 0.5}, {1, 1}, {0.5, 1}};
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
  double n = 1;

  if(branches[start_line.y] == 1)
  {
    auto succ = -1;
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
    
    return branches_depth(lines, branches, vec2i{start_line.y, succ});
  }
  else
  {
    while(!actual.empty())
    {
      node = actual[0].y;
      for(int i=0; i<lines.size(); i++)
      {
        if(lines[i].x == node)
        {
          if(branches[lines[i].y] > 0 )
          {
            actual += lines[i];
            acc += pow((float)branches[lines[i].y], n);
          }
        }

    }

    tot_acc += pow(acc, 1/n); 
    acc = 0;
    actual.erase(actual.begin());

    } 
  }
  
  return tot_acc == 0 ? 1 : tot_acc;

}

float f(float x)
{
  return  -1 * x * x + 3 * x + 0.3;
}


std::vector<vec3f> attractors_generator(int points_number, int range_min, int range_max, int z_offset, float f(float), rng_state& rng)
{
  std::vector<vec3f> cloud;
  
  for(int i=0; i<points_number; i++)
  {
    auto p = (range_max - range_min) * rand3f(rng) + range_min;
    
    if(3 > pow(p.x - 0 , 2.0) + pow(p.y - 0, 2.0) + pow(p.z - 2, 2.0)  )
    //if(pow(p.x*p.x + p.y*p.y, 0.5) < f(p.z))
    {
      p.z += z_offset;
      cloud += p;
    }
    else
      i -= 1;   
  }

  return cloud;
}



int main(void)
{
  std::cout << "Hello, yocto!\n";
  std::cout << "On blender, Y forward, Z up.\n";

  std::vector<vec4i> quads; 
  std::vector<vec3f> positions, attractors_postions;
  std::vector<vec3f> normals;
  std::vector<vec2f> texcoords;
  std::vector<vec2i> lines;
  std::vector<int> points;
  std::vector<vec3f> colors;
  std::string error;
  auto tree_nodes = std::vector<int>();
  


  //Cloud generation
  rng_state rng = make_rng(54);
  
  auto starting_point = vec3f{0, 0, 0};
  
  std::vector<vec3f> cloud = attractors_generator(250, -3, 3, 0, f, rng);
  
  attractors_postions += starting_point;
  tree_nodes += 0;

  //Tree's nodes generation
  int max_nodes = 100;
  float D = 0.4;
  float max_influece_sphere = 5*D;
  float max_killing_radius = 3*D;
  
  auto branches = std::vector<int>(max_nodes+1, 0);
  while(tree_nodes.size() < max_nodes)
  {
    
    auto nodes_to_be_added = std::vector<int>();
    for(auto node: tree_nodes)
    {
     
      if(tree_nodes.size() > 1 && node == 0)
        continue;
      auto new_cloud = std::vector<vec3f>();
      auto v = attractors_postions[node];

      auto attractors = get_influence_sphere(v, cloud, max_influece_sphere);
      if(attractors.empty())
        continue;
      
      branches[node] += 1;
      
      auto total_dir = zero3f;
      for(auto a: attractors)
      {
        auto s = cloud[a];
        total_dir += normalize(s - v);
      } 
      total_dir = normalize(total_dir);

      auto v_prime = v + total_dir * D;

      attractors_postions += v_prime;
      int new_node = attractors_postions.size()-1;

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
    auto max_width = 30;
    auto t = branches_depth(lines, branches, l);
   
    width_vector[l[0]][l[1]] = t > max_width ? max_width : t;
  }


  
  
  auto k = 0.002f;
  frame3f frame;
  for(auto l: lines)
  {
    auto x = attractors_postions[l[0]];
    auto x_ = attractors_postions[l[1]];

   
    

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
    auto max_base_width = 5;

    base_width = (base_width - width);
    if(base_width > width * max_base_width)
      base_width = max_base_width*width;

    cylinder_try(quads, positions,
    normals, texcoords, vec3i{8, 8, 8}, vec2f{k * width, D/2}, vec3f{0.5, 0.5, 1.0}, frame, base_width * k );

    
  
  
    //Try to add leaves

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
        auto k = 2;
        auto e = 2.7;
        p = 9 * pif * p ;
        float X = a * pow(e, k*p) * yocto::math::cos(p); 
        float Y = a * pow(e, k*p) * yocto::math::sin(p); 
      

        

        frame3f leaf_frame;
        leaf_frame = frame_fromzx(leaf_position, frame.z, vec3f{X, Y, 0});
        auto random_rotation = rotation_frame(leaf_frame.x, rand1f(rng));
        
        leaf_frame.x = transform_vector(random_rotation, leaf_frame.x);
        leaf_frame.y = transform_vector(random_rotation, leaf_frame.y);
        leaf_frame.z = transform_vector(random_rotation, leaf_frame.z);
        
        //auto leaf_frame = frame_fromzx(leaf_position, i%2==1 ? vec3f{0.5, 0.5,  1} : vec3f{-0.5, -0.5, 1}, x_alternate);
        quad_try(quads, positions, normals, texcoords, leaf_size, leaf_frame);
      }


    }

  
  }
  

  if(false)
  {
    positions.clear();
    quads.clear();
    normals.clear();
    texcoords.clear();

    cylinder_try(quads, positions,
      normals, texcoords, vec3i{8, 8, 8}, vec2f{0.5, 1}, vec3f{0.5, 0.5, 1.0}, frame_fromz(vec3f{1,1,0}, vec3f{0,0,1}), 1 );
  }
  
  
  std::cout << "Positions: " << positions.size() << "\n";
  std::cout << "Points:    " << points.size() << "\n";
  std::cout << "Lines:     " << lines.size() << "\n";
  std::cout << "Quads:     " << quads.size() << "\n";
  std::cout << "Tree Nodes:  " << tree_nodes.size() << "\n";


  
  
  bool ok = save_shape("test.obj",
    points, lines,
    std::vector<vec3i>(), quads,
    positions, normals,
    texcoords, colors,
    std::vector<float>(), error, true,
    false);

  

  if(error == "")
    std::cout << "You're lucky, no errors!" << "\n";
  else
    std::cout << "Error"<< error <<"\n";

  
}