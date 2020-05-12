
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


int main(void)
{
  std::cout << "Hello, yocto!\n";

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
  auto starting_point = vec3f{0.5, 0.5, 0.0};

  
  for(int i=0; i<500; i++)
  {
    auto p = rand3f(rng);
    if(0.25 > pow(p.x - 0.5 , 2.0) + pow(p.y - 0.5, 2.0) + pow(p.z - 0.5, 2.0)  )
    { 
      cloud += p;
    }
    else
      i -= 1;
  }

  auto tree_nodes = std::vector<int>();
  auto attractors = std::vector<int>();
  auto dead_attractors = std::vector<int>();

  positions += starting_point;
  tree_nodes += 0;

  float D = 0.1;
  while(tree_nodes.size() < 50)
  {
    auto nodes_to_be_added = std::vector<int>();
    for(auto node: tree_nodes)
    {
      if(tree_nodes.size() > 1 && node == 0)
        continue;
      auto new_cloud = std::vector<vec3f>();
      auto v = positions[node];

      attractors = get_influence_sphere(v, cloud, 0.3);
      if(attractors.empty())
        continue;
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


      auto dead_attractors = get_influence_sphere(v_prime, cloud, 2*D);

      
      for(int i=0; i<cloud.size(); i++)
      {
        auto j = std::find(dead_attractors.begin(), dead_attractors.end(), i);
        if(j == dead_attractors.end())
        {
          new_cloud += cloud[i];
        }
      }

      cloud = std::vector<vec3f>(new_cloud);
      std::cout << new_cloud.size() << "\n";


    }

    if(nodes_to_be_added.empty())
      break;
    else
      tree_nodes += nodes_to_be_added;
    
    


  }
  
 
  
  
  std::cout << "Positions: " << positions.size() << "\n";
  std::cout << "Points:    " << points.size() << "\n";
  std::cout << "Lines:    " << lines.size() << "\n";
  std::cout << "Tree Nodes:    " << tree_nodes.size() << "\n";


  
  bool ok = save_shape("test.obj",
    points, lines,
    std::vector<vec3i>(),  quads,
    positions, normals,
    texcoords, colors,
    std::vector<float>(), error, true,
    false);

  

  if(error == "")
    std::cout << "No error"<< "\n";
  else
    std::cout << "Error"<< error <<"\n";

  
}