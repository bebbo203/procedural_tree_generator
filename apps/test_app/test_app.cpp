
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
  double n = 2.5;

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
    
    //if(2 > distance(p, vec3f{0,0,0})  )
    if(pow(p.x*p.x + p.y*p.y, 0.5) < f(p.z))
    //if(10 > pow(p.x - 0, 2.0) + pow(p.y, 2.0) && p.z > 1 && p.z < 3)
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
  std::vector<vec3f> positions, nodes_positions;
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
  auto initial_length = vec3f{0, 0, 1.4};
  std::vector<vec3f> cloud = attractors_generator(10000, -20, 20 , 1,  f, rng);
  
  nodes_positions += starting_point;
  nodes_positions += initial_length;
  tree_nodes += 0;
  tree_nodes += 1;

  lines += vec2i{0, 1};

  //Tree's nodes generation
  int max_nodes = 700;
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


  while(tree_nodes.size() < max_nodes)
  {
    
   
    auto nodes_to_be_added = std::vector<int>();
    for(auto node: tree_nodes)
    {
      
      if(node == 0 || branches[node] >= 3)
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
    auto max_width = 9999;
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
    auto max_base_width = 999999;

    base_width = (base_width - width);
    //if(base_width > width * max_base_width)
    std::cout << width << " | " << base_width << " | " << max_base_width << "\n"; 
    if(base_width > max_base_width)
      base_width = max_base_width;

    cylinder_try(quads, positions,
    normals, texcoords, vec3i{8, 8, 8}, vec2f{W * width, length/2}, vec3f{0.5, 0.5, 1.0}, frame, base_width * W );

    
  
  
    //Try to add leaves

    auto leaves_density_min = 8;
    auto leaves_density_max = 15;
    auto leaf_size = 0.001;
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
        
        quad_try(quads, positions, normals, texcoords, leaf_size, leaf_frame);
      }


    }

  
  }
  

  if(false)
  {
    //positions.clear();
    quads.clear();
    normals.clear();
    texcoords.clear();
    lines.clear();
    

    //cylinder_try(quads, positions,
    //  normals, texcoords, vec3i{8, 8, 8}, vec2f{0.5, 1}, vec3f{0.5, 0.5, 1.0}, frame_fromz(vec3f{1,1,0}, vec3f{0,0,1}), 1 );
  }
  
  
  std::cout << "Positions: " << positions.size() << "\n";
  std::cout << "Points:    " << points.size() << "\n";
  std::cout << "Lines:     " << lines.size() << "\n";
  std::cout << "Quads:     " << quads.size() << "\n";
  std::cout << "Alive att :  " << cloud.size() << "\n";
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