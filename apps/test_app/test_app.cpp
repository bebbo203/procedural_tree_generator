
#include <iostream>
#include <yocto/yocto_shape.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_math.h>
#include <math.h>
using namespace yocto::math;
using namespace yocto::shape;
using namespace yocto::common;



float f(float x)
{
  return  -1 * x * x + 5 * x + 0.3;
}


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


// R è di quanto la base deve essere più grande 
void cylinder_try(std::vector<vec4i>& quads, std::vector<vec3f>& positions,
    std::vector<vec3f>& normals, std::vector<vec2f>& texcoords,
    const vec3i& steps, const vec2f& scale, const vec3f& uvscale, const frame3f& frame, const float R=0) {
  auto qquads     = std::vector<vec4i>{};
  auto qpositions = std::vector<vec3f>{};
  auto qnormals   = std::vector<vec3f>{};
  auto qtexcoords = std::vector<vec2f>{};

  // side
  make_rect(qquads, qpositions, qnormals, qtexcoords, {steps.x, steps.y},
      {1, 1}, {1, 1});
  
  
  for (auto i = 0; i < qpositions.size(); i++) {
    auto uv       = qtexcoords[i];
    auto phi      = 2 * pif * uv.x;
    
    qpositions[i] = {
        yocto::math::cos(phi) * scale.x, yocto::math::sin(phi) * scale.x, (2 * uv.y - 1) * scale.y};

    

    
    auto norm_z = yocto::math::abs((qpositions[i].z - scale.y));
    auto s = (R/2 * norm_z) / scale.y;
    
    qpositions[i].x = yocto::math::cos(phi) * (scale.x + s);
    qpositions[i].y = yocto::math::sin(phi) * (scale.x + s);
    

    qnormals[i]   = {yocto::math::cos(phi), yocto::math::sin(phi), 0};
    qtexcoords[i] = uv * vec2f{uvscale.x, uvscale.y};

    qpositions[i].z += scale.y;
    qpositions[i] = transform_point(frame, qpositions[i]);
    qnormals[i] = transform_vector(frame, qnormals[i]);
  }

  
  merge_quads(quads, positions, normals, texcoords, qquads, qpositions,
      qnormals, qtexcoords);
  
 

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


int main(void)
{
  std::cout << "Hello, yocto!\n";
  std::cout << "On blender, Y forward, Z up.\n";

  std::vector<vec4i> quads; 
  std::vector<vec3f> positions;
  std::vector<vec3f> normals;
  std::vector<vec2f> texcoords;
  std::vector<vec2i> lines;
  std::vector<int> points;
  std::vector<vec3f> colors;
  std::string error;

  rng_state rng = make_rng(54);
  
  
  
 
  

  std::vector<vec3f> cloud;
  auto starting_point = vec3f{0, 0, 0.0};

  
  for(int i=0; i<500; i++)
  {
    auto p = rand3f(rng) * 10 - 5;
    if(2 > pow(p.x - 0 , 2.0) + pow(p.y - 0, 2.0) + pow(p.z - 2, 2.0)  )
    { 
      p.z += 0;
      cloud += p;
    }
    else
      i -= 1;
      
  }



  auto tree_nodes = std::vector<int>();
  auto attractors = std::vector<int>();
  auto dead_attractors = std::vector<int>();
  auto branches = std::vector<int>(200, 0);

  positions += starting_point;
  tree_nodes += 0;

  
  float D = 0.3;
  while(tree_nodes.size() < 400)
  {
    auto nodes_to_be_added = std::vector<int>();
    for(auto node: tree_nodes)
    {
      if(tree_nodes.size() > 1 && node == 0)
        continue;
      auto new_cloud = std::vector<vec3f>();
      auto v = positions[node];

      attractors = get_influence_sphere(v, cloud, 5*D);
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

      positions += v_prime;
      int new_node = positions.size()-1;

      lines += vec2i{node, new_node};
      nodes_to_be_added += new_node;


      auto dead_attractors = get_influence_sphere(v_prime, cloud, 3*D);

      
      for(int i=0; i<cloud.size(); i++)
      {
        auto j = std::find(dead_attractors.begin(), dead_attractors.end(), i);
        if(j == dead_attractors.end())
        {
          new_cloud += cloud[i];
        }
      }

      cloud = std::vector<vec3f>(new_cloud);
      std::reverse(lines.begin(), lines.end());
    }

    if(nodes_to_be_added.empty())
      break;
    else
      nodes_to_be_added += tree_nodes;
      tree_nodes = nodes_to_be_added;
      
    


  }
  
  
 

 
 

  auto new_positions = positions;
  positions.clear();
  quads.clear();
  

  auto width_vector = std::vector<std::vector<float>>(tree_nodes.size());
  for(auto& r: width_vector)
    r = std::vector<float>(tree_nodes.size(), -1);

  for(auto l: lines)
  {
    auto max_width = 30;
    auto t = branches_depth(lines, branches, l);
   
    width_vector[l[0]][l[1]] = t > max_width ? max_width : t;
  }


  //Il cono lo allargo sotto eh
  
  auto k = 0.002f;
  for(auto l: lines)
  {
    auto x = new_positions[l[0]];
    auto x_ = new_positions[l[1]];

   
    

    //auto width = branches_depth(lines, branches, l);
    auto width = width_vector[l[0]][l[1]];
    auto frame = frame_fromz(x, normalize(x_ - x));

    auto base_width = width;
    for(int i=0; i < tree_nodes.size(); i++)
    {
      if(width_vector[i][l[0]] != -1)
      {
        base_width = width_vector[i][l[0]];
        break;
      }
    }

    
    std::cout << width * k << " -> " << base_width * k - width * k << "\n";
     



    cylinder_try(quads, positions,
    normals, texcoords, vec3i{8, 8, 8}, vec2f{k * width, D/2}, vec3f{1.0, 1.0, 1.0}, frame, (base_width - width) * k );
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
    std::cout << "No error"<< "\n";
  else
    std::cout << "Error"<< error <<"\n";

  
}