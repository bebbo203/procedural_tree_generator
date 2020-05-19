
#include <yocto_extension/yocto_extension.h>
#include <yocto/yocto_math.h>
using namespace yocto::math;


int main(void)
{

  std::cout << "Hello, yocto... :(\n";
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
      bool single_object = false;

  // ######################################

  yocto::extension::generate_tree(n_attractors, attractors_range_min, attractors_range_max, attractors_z_offset,
      tree_starting_point, trunk_length, max_nodes, D, W, max_width,
      max_influence_sphere, max_killing_radius, tropism, leaves_density_min, leaves_density_max,
      leaf_size_max, leaf_size_min,a_spiral, k_spiral, e, rounds, leaves_textures_number,
      single_object, single_mode_texture_path, single_mode_opacity_texture_path,
      multiple_mode_tree_texture_path, multiple_modes_leaf_texture_path, 
      multiple_modes_leaf_opacity_texture_path, export_name_path);

  return 0;
}